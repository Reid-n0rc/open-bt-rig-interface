# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for cat_loopback.py against a simulated device (no hardware, no bleak).

    python3 -m unittest discover -s tools/cat_loopback -p 'test_*.py'
"""

from __future__ import annotations

import asyncio
import contextlib
import io
import os
import random
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cat_loopback as cl  # noqa: E402
from cat_loopback import pc  # noqa: E402


class FakeDevice:
    """A device with a TX->RX jumper on port 0, following SPEC §7: credit
    returned as bytes leave toward the radio, echoed bytes sent as CAT_DATA."""

    def __init__(self, tx_buffer=256, max_payload=256, uart_bytes_per_tick=64,
                 corrupt_at=None, drop=0, extra=b"", ignore_credit=False, proto_minor=None):
        self.tx_buffer = tx_buffer
        self.max_payload = max_payload
        self.uart_bytes_per_tick = uart_bytes_per_tick
        self.corrupt_at, self.drop, self.extra = corrupt_at, drop, extra
        self.ignore_credit = ignore_credit
        self.proto_minor = pc.PROTOCOL_VERSION[1] if proto_minor is None else proto_minor
        self.decoder = pc.StreamDecoder()
        self.host_feed = None
        self.queue = bytearray()   # toward the "radio" (the jumper)
        self.credit = 0
        self.echoed = 0
        self.is_open = False
        self.task = None

    def attach(self, session: cl.LoopbackSession) -> None:
        self.host_feed = session.feed

    async def send_to_host(self, name, token, fields):
        wire = pc.encode(name, token, fields)
        for i in range(0, len(wire), 20):  # small GATT-like chunks
            self.host_feed(wire[i:i + 20])
        await asyncio.sleep(0)

    async def receive(self, wire: bytes) -> None:
        for msg in self.decoder.feed(wire):
            await self.handle(msg)

    async def handle(self, msg):
        name, token, f = msg["message"], msg["token"], msg["fields"]
        major, _minor, patch = pc.PROTOCOL_VERSION
        if name == "HELLO":
            await self.send_to_host("DEVICE_INFO", token, {
                "proto_major": major, "proto_minor": self.proto_minor, "proto_patch": patch,
                "fw_major": 0, "fw_minor": 1, "fw_patch": 0, "variant": "D",
                "hw_revision": "A", "max_payload": self.max_payload, "build": "fake"})
        elif name == "CAPS_GET":
            await self.send_to_host("CAPS", token, {"tlvs": [
                {"tlv": "SERIAL_JACK", "port": 0, "modes": 1, "min_baud": 4800,
                 "max_baud": 115200, "tx_buffer": self.tx_buffer}]})
        elif name == "SERIAL_OPEN":
            self.is_open = bool(f["open"])
            self.credit, self.queue = self.tx_buffer, bytearray()
            if self.is_open and self.task is None:
                self.task = asyncio.ensure_future(self.uart())
            await self.send_to_host("RESULT", token, {"request_type": 0x20, "code": 0})
        elif name == "SERIAL_SET":
            await self.send_to_host("RESULT", token, {"request_type": 0x21, "code": 0})
        elif name == "CAT_DATA":
            data = bytes.fromhex(f["data"])
            take = len(data) if self.ignore_credit else min(len(data), self.credit)
            self.queue += data[:take]
            self.credit -= take
            if take < len(data):
                await self.send_to_host("RESULT", 0, {"request_type": 0x22, "code": 7})

    async def uart(self):
        """Moves bytes through the jumper at a fixed rate and echoes them."""
        while True:
            await asyncio.sleep(0.001)
            if not self.is_open or not self.queue or self.uart_bytes_per_tick == 0:
                continue
            n = min(self.uart_bytes_per_tick, len(self.queue))
            out, self.queue = bytes(self.queue[:n]), self.queue[n:]
            self.credit += n  # the bytes left toward the radio (SPEC §7.4)
            await self.send_to_host("CAT_CREDIT", 0, {"port": 0, "credit": n})
            out = self.mangle(out)
            for i in range(0, len(out), self.max_payload - 1):
                await self.send_to_host("CAT_DATA", 0,
                                        {"port": 0, "data": out[i:i + self.max_payload - 1].hex()})

    def mangle(self, out: bytes) -> bytes:
        start = self.echoed
        self.echoed += len(out)
        b = bytearray(out)
        if self.corrupt_at is not None and start <= self.corrupt_at < self.echoed:
            b[self.corrupt_at - start] ^= 0xFF
        if self.drop and self.echoed >= 1000:
            b, self.drop = b[:-self.drop] if self.drop < len(b) else b"", 0
        if self.extra and self.echoed >= 1000:
            b += self.extra
            self.extra = b""
        return bytes(b)

    def stop(self):
        if self.task:
            self.task.cancel()


def run(device: FakeDevice, size=3000, timeout=5.0, baud=None, close=True):
    async def go():
        session = cl.LoopbackSession(device.receive)
        device.attach(session)
        try:
            await session.start(baud)
            payload = random.Random(1).randbytes(size)
            report = await session.run(payload, timeout)
            if close:
                await session.close()
            return session, report
        finally:
            device.stop()
    return asyncio.run(go())


class LoopbackTests(unittest.TestCase):
    def test_byte_exact_loopback_passes(self):
        session, report = run(FakeDevice(), baud=115200)
        self.assertTrue(report.ok, report.summary())
        self.assertEqual(3000, report.sent)
        self.assertEqual(3000, report.received)
        self.assertGreater(report.credit_messages, 0)
        self.assertIn("PASS", report.summary())

    def test_frames_respect_credit_and_max_payload(self):
        device = FakeDevice(tx_buffer=100, max_payload=64)
        _session, report = run(device, size=1000)
        self.assertTrue(report.ok, report.summary())
        self.assertGreaterEqual(report.frames_sent, 1000 // 63)
        self.assertGreaterEqual(device.credit, 0)  # never driven below zero

    def test_corrupted_byte_fails_with_position(self):
        _s, report = run(FakeDevice(corrupt_at=1234))
        self.assertFalse(report.ok)
        self.assertIn("first difference at byte 1234", report.summary())

    def test_missing_bytes_time_out(self):
        _s, report = run(FakeDevice(drop=5), timeout=0.5)
        self.assertFalse(report.ok)
        self.assertTrue(any("timed out waiting for echo" in e for e in report.errors),
                        report.errors)

    def test_extra_bytes_fail(self):
        # The extras arrive with the last frame, so close the run quickly.
        _s, report = run(FakeDevice(extra=b"XYZ"), size=1100)
        self.assertFalse(report.ok)
        self.assertTrue(any("extra" in e or "difference" in e for e in report.errors),
                        report.errors)

    def test_never_exceeds_credit(self):
        # A device whose radio side takes nothing returns no credit: the
        # session stops at its credit, waits, and then times out.
        device = FakeDevice(uart_bytes_per_tick=0)
        _s, report = run(device, size=600, timeout=0.3, close=False)
        self.assertFalse(report.ok)
        self.assertEqual(256, report.sent)  # exactly the credit, never more

    def test_overflow_result_is_reported(self):
        session = cl.LoopbackSession(lambda _w: asyncio.sleep(0))
        session.feed(pc.encode("RESULT", 0, {"request_type": 0x22, "code": 7}))
        self.assertIn("RESULT code 7 for request type 0x22", session.report.errors[0])

    def test_version_mismatch_is_refused(self):
        device = FakeDevice(proto_minor=pc.PROTOCOL_VERSION[1] + 1)

        async def go():
            session = cl.LoopbackSession(device.receive)
            device.attach(session)
            with self.assertRaises(cl.LoopbackError):
                await session.start(None)
        asyncio.run(go())

    def test_no_reply_times_out(self):
        async def go():
            session = cl.LoopbackSession(lambda _w: asyncio.sleep(0))
            with self.assertRaises(cl.LoopbackError):
                await session.request("PING", {"data": ""}, timeout=0.05)
        asyncio.run(go())

    def test_tokens_wrap_and_skip_zero(self):
        session = cl.LoopbackSession(lambda _w: asyncio.sleep(0))
        tokens = [session._next_token() for _ in range(300)]
        self.assertNotIn(0, tokens)
        self.assertEqual(1, tokens[255])

    def test_args(self):
        args = cl.parse_args(["--bytes", "10", "--baud", "38400"])
        self.assertEqual((10, 38400, 15), (args.bytes, args.baud, args.seed))
        with self.assertRaises(SystemExit), contextlib.redirect_stderr(io.StringIO()):
            cl.parse_args(["--bytes", "0"])


if __name__ == "__main__":
    unittest.main()

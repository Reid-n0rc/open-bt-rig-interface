#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""CAT bridge loopback test for a dev kit (issue #15, protocol/SPEC.md §7).

Wire the SERIAL-jack UART's TX to its RX on the dev kit (see
firmware/platform/esp-idf/BOARD.md), then:

    pip install bleak==0.22.3
    python3 tools/cat_loopback/cat_loopback.py --bytes 4096 --baud 115200

The script connects over Bluetooth LE, starts a session (HELLO, CAPS_GET),
opens port 0, sends random bytes as CAT_DATA within the device's credit
(SPEC §7.4) and checks that exactly the same bytes come back as CAT_DATA.
It fails on any difference, any RESULT error (for example OVERFLOW), or if
the echo hasn't finished by the timeout.

The protocol logic (LoopbackSession) is independent of the transport and is
unit-tested against a simulated device in test_cat_loopback.py; only BleLink
needs `bleak` and hardware.
"""

from __future__ import annotations

import argparse
import asyncio
import os
import random
import sys
import time
from dataclasses import dataclass, field
from typing import Awaitable, Callable

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "protocol"))
import proto_codec as pc  # noqa: E402

SERVICE_UUID = "274b5780-7929-4cf2-8a88-af1180d8fc04"
RX_UUID = "5c287ba9-de76-46de-8ca1-3519ad5fa899"  # host -> device
TX_UUID = "3b7d006a-e34e-448c-a911-7f683a261abf"  # device -> host
HOST_MAX_PAYLOAD = 1024
REPLY_TIMEOUT_S = 2.0
PORT = 0


class LoopbackError(Exception):
    """The loopback failed: a protocol error, a mismatch or a timeout."""


@dataclass
class Report:
    sent: int = 0
    received: int = 0
    frames_sent: int = 0
    frames_received: int = 0
    credit_messages: int = 0
    elapsed_s: float = 0.0
    errors: list[str] = field(default_factory=list)

    @property
    def ok(self) -> bool:
        return not self.errors and self.sent == self.received

    def summary(self) -> str:
        rate = self.received / self.elapsed_s if self.elapsed_s > 0 else 0.0
        lines = [
            f"sent {self.sent} bytes in {self.frames_sent} CAT_DATA frames",
            f"received {self.received} bytes in {self.frames_received} CAT_DATA frames",
            f"{self.credit_messages} CAT_CREDIT messages, {self.elapsed_s:.2f} s, {rate:.0f} B/s",
        ]
        lines += [f"ERROR: {e}" for e in self.errors]
        lines.append("PASS: byte-exact loopback" if self.ok else "FAIL")
        return "\n".join(lines)


SendFn = Callable[[bytes], Awaitable[None]]


class LoopbackSession:
    """Drives one loopback run over any byte-stream link.

    The link calls `feed()` with every chunk the device sends; the session
    calls `send` with whole frames (the link splits them for its transport).
    """

    def __init__(self, send: SendFn) -> None:
        self._send = send
        self._decoder = pc.StreamDecoder()
        self._replies: dict[int, asyncio.Future] = {}
        self._token = 0
        self._credit = 0
        self._credit_event = asyncio.Event()
        self._echo = bytearray()
        self._echo_event = asyncio.Event()
        self.report = Report()
        self.device_max_payload = 256

    # --- incoming -------------------------------------------------------------

    def feed(self, data: bytes) -> None:
        for msg in self._decoder.feed(data):
            self._handle(msg)

    def _handle(self, msg: dict) -> None:
        name, token, f = msg["message"], msg["token"], msg["fields"]
        if token and token in self._replies and not self._replies[token].done():
            self._replies[token].set_result(msg)
            return
        if name == "CAT_DATA" and f["port"] == PORT:
            data = bytes.fromhex(f["data"])
            self._echo += data
            self.report.received += len(data)
            self.report.frames_received += 1
            self._echo_event.set()
        elif name == "CAT_CREDIT" and f["port"] == PORT:
            self._credit += f["credit"]
            self.report.credit_messages += 1
            self._credit_event.set()
        elif name == "RESULT" and f["code"] != 0:
            self.report.errors.append(
                f"RESULT code {f['code']} for request type 0x{f['request_type']:02x}")
            self._credit_event.set()  # wake the sender so it can stop
            self._echo_event.set()

    # --- requests -------------------------------------------------------------

    def _next_token(self) -> int:
        self._token = self._token % 255 + 1
        return self._token

    async def request(self, name: str, fields: dict, timeout: float = REPLY_TIMEOUT_S) -> dict:
        token = self._next_token()
        fut = asyncio.get_running_loop().create_future()
        self._replies[token] = fut
        await self._send(pc.encode(name, token, fields))
        try:
            return await asyncio.wait_for(fut, timeout)
        except asyncio.TimeoutError:
            raise LoopbackError(f"no reply to {name} within {timeout} s") from None
        finally:
            self._replies.pop(token, None)

    async def start(self, baud: int | None) -> None:
        major, minor, patch = pc.PROTOCOL_VERSION
        info = await self.request("HELLO", {"proto_major": major, "proto_minor": minor,
                                            "proto_patch": patch,
                                            "max_payload": HOST_MAX_PAYLOAD, "flags": 0})
        if info["message"] != "DEVICE_INFO":
            raise LoopbackError(f"HELLO answered with {info['message']}")
        f = info["fields"]
        if (f["proto_major"], f["proto_minor"]) != (major, minor):
            raise LoopbackError(f"device protocol {f['proto_major']}.{f['proto_minor']}, "
                                f"this script {major}.{minor}")
        self.device_max_payload = f["max_payload"]

        caps = await self.request("CAPS_GET", {})
        jack = [t for t in caps["fields"]["tlvs"]
                if t["tlv"] == "SERIAL_JACK" and t["port"] == PORT]
        if not jack:
            raise LoopbackError("the device reports no SERIAL_JACK port 0 in CAPS")
        tx_buffer = jack[0]["tx_buffer"]

        await self._expect_ok("SERIAL_OPEN", {"port": PORT, "open": 1})
        self._credit = tx_buffer  # SPEC §7.4: opening resets the credit
        if baud is not None:
            await self._expect_ok("SERIAL_SET", {"port": PORT, "baud": baud, "data_bits": 8,
                                                 "parity": 0, "stop_bits": 0})

    async def _expect_ok(self, name: str, fields: dict) -> None:
        r = await self.request(name, fields)
        if r["message"] != "RESULT" or r["fields"]["code"] != 0:
            raise LoopbackError(f"{name} failed: {r}")

    # --- the loopback ---------------------------------------------------------

    async def run(self, payload: bytes, timeout: float) -> Report:
        """Sends `payload` within the credit and waits for the same bytes back."""
        t0 = time.monotonic()
        deadline = t0 + timeout
        chunk_max = max(1, self.device_max_payload - 1)  # CAT_DATA: port + data
        pos = 0
        while pos < len(payload) and not self.report.errors:
            if self._credit == 0:
                self._credit_event.clear()
                await self._wait(self._credit_event, deadline, "credit")
                continue
            n = min(self._credit, chunk_max, len(payload) - pos)
            await self._send(pc.encode("CAT_DATA", 0,
                                       {"port": PORT, "data": payload[pos:pos + n].hex()}))
            self._credit -= n
            pos += n
            self.report.sent += n
            self.report.frames_sent += 1
        while len(self._echo) < len(payload) and not self.report.errors:
            self._echo_event.clear()
            await self._wait(self._echo_event, deadline, "echo")
        self.report.elapsed_s = time.monotonic() - t0
        self._compare(payload)
        return self.report

    async def _wait(self, event: asyncio.Event, deadline: float, what: str) -> None:
        left = deadline - time.monotonic()
        try:
            await asyncio.wait_for(event.wait(), max(left, 0.0))
        except asyncio.TimeoutError:
            self.report.errors.append(f"timed out waiting for {what} "
                                      f"(sent {self.report.sent}, received {len(self._echo)})")

    def _compare(self, payload: bytes) -> None:
        echo = bytes(self._echo)
        if echo == payload[:len(echo)] and len(echo) == len(payload):
            return
        for i, (a, b) in enumerate(zip(payload, echo)):
            if a != b:
                self.report.errors.append(
                    f"first difference at byte {i}: sent 0x{a:02x}, received 0x{b:02x}")
                return
        if len(echo) > len(payload):
            self.report.errors.append(f"{len(echo) - len(payload)} extra bytes received")
        elif not self.report.errors:
            self.report.errors.append(f"{len(payload) - len(echo)} bytes missing")

    async def close(self) -> None:
        try:
            await self._expect_ok("SERIAL_OPEN", {"port": PORT, "open": 0})
        except LoopbackError as exc:
            self.report.errors.append(f"closing the port: {exc}")


# --- Bluetooth LE (needs bleak and a dev kit) ----------------------------------


class BleLink:
    def __init__(self, address: str | None, name: str | None, scan_timeout: float) -> None:
        self.address, self.name, self.scan_timeout = address, name, scan_timeout
        self.client = None
        self.on_data: Callable[[bytes], None] = lambda _d: None

    async def connect(self) -> None:
        try:
            from bleak import BleakClient, BleakScanner
        except ImportError:
            raise LoopbackError("this needs bleak: pip install bleak==0.22.3") from None
        if self.address:
            device = await BleakScanner.find_device_by_address(self.address, self.scan_timeout)
        else:
            def match(d, adv):
                if SERVICE_UUID not in [u.lower() for u in adv.service_uuids]:
                    return False
                return self.name is None or (d.name or "") == self.name
            device = await BleakScanner.find_device_by_filter(match, self.scan_timeout)
        if device is None:
            raise LoopbackError("no Rig Interface device found (is it advertising?)")
        self.client = BleakClient(device)
        await self.client.connect()
        try:
            await self.client.pair()  # RX/TX need an encrypted, bonded link (SPEC §13.1)
        except NotImplementedError:
            pass  # macOS pairs on first access to an encrypted characteristic
        await self.client.start_notify(TX_UUID, lambda _c, data: self.on_data(bytes(data)))

    async def send(self, wire: bytes) -> None:
        size = max(20, self.client.mtu_size - 3)  # SPEC §13.1: chunks of ATT_MTU - 3
        for i in range(0, len(wire), size):
            await self.client.write_gatt_char(RX_UUID, wire[i:i + size], response=False)

    async def disconnect(self) -> None:
        if self.client is not None and self.client.is_connected:
            await self.client.disconnect()


async def run_ble(args: argparse.Namespace) -> Report:
    link = BleLink(args.address, args.name, args.scan_timeout)
    await asyncio.wait_for(link.connect(), args.scan_timeout + 30)
    session = LoopbackSession(link.send)
    link.on_data = session.feed
    try:
        await session.start(args.baud)
        payload = random.Random(args.seed).randbytes(args.bytes)
        report = await session.run(payload, args.timeout)
        await session.close()
        return report
    finally:
        await link.disconnect()


def parse_args(argv: list[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    p.add_argument("--address", help="BLE address (or macOS UUID) of the device")
    p.add_argument("--name", help="advertised name to pick, if several devices are around")
    p.add_argument("--bytes", type=int, default=4096, help="bytes to send (default 4096)")
    p.add_argument("--baud", type=int, help="SERIAL_SET this baud rate first (8N1)")
    p.add_argument("--seed", type=int, default=15, help="random seed for the payload")
    p.add_argument("--timeout", type=float, default=60.0, help="seconds for the whole run")
    p.add_argument("--scan-timeout", type=float, default=10.0, help="seconds to scan")
    args = p.parse_args(argv)
    if args.bytes < 1:
        p.error("--bytes must be at least 1")
    return args


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    try:
        report = asyncio.run(run_ble(args))
    except (LoopbackError, asyncio.TimeoutError) as exc:
        print(f"FAIL: {exc or 'timed out connecting'}")
        return 1
    print(report.summary())
    return 0 if report.ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

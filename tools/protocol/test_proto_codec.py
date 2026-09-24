# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Round-trip every golden vector in protocol/vectors/ through the reference codec.

    python3 -m unittest discover -s tools/protocol -p 'test_*.py'
"""

from __future__ import annotations

import json
import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import make_vectors  # noqa: E402
import proto_codec as pc  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
VECTORS = ROOT / "protocol" / "vectors"
SPEC = ROOT / "protocol" / "SPEC.md"


def load(name: str) -> dict:
    return json.loads((VECTORS / name).read_text(encoding="utf-8"))


class MessageVectors(unittest.TestCase):
    def setUp(self):
        self.data = load("messages.json")

    def test_version_matches_codec(self):
        self.assertEqual(self.data["protocol_version"], ".".join(map(str, pc.PROTOCOL_VERSION)))

    def test_encode_matches_vector(self):
        for v in self.data["messages"]:
            with self.subTest(v["name"]):
                msg_type, payload = pc.encode_payload(v["message"], v["fields"])
                self.assertEqual(f"0x{msg_type:02x}", v["type"])
                self.assertEqual(payload.hex(), v["payload_hex"])
                frame = pc.build_frame(msg_type, v["token"], payload)
                self.assertEqual(frame.hex(), v["frame_hex"])
                self.assertEqual(pc.encode(v["message"], v["token"], v["fields"]).hex(),
                                 v["wire_hex"])

    def test_decode_matches_vector(self):
        for v in self.data["messages"]:
            with self.subTest(v["name"]):
                msg = pc.decode(bytes.fromhex(v["wire_hex"]))
                self.assertEqual(msg["message"], v["message"])
                self.assertEqual(msg["token"], v["token"])
                self.assertEqual(msg["fields"], v["fields"])

    def test_wire_has_single_delimiter(self):
        for v in self.data["messages"]:
            wire = bytes.fromhex(v["wire_hex"])
            self.assertEqual(wire.count(0), 1, v["name"])
            self.assertEqual(wire[-1], 0, v["name"])

    def test_every_message_has_a_vector(self):
        covered = {v["message"] for v in self.data["messages"]}
        names = {name for name, _d, _s in pc.MESSAGES.values()}
        self.assertEqual(names - covered, set())

    def test_every_config_key_and_tlv_has_a_vector(self):
        keys, tlvs = set(), set()
        for v in self.data["messages"]:
            cfg = v["fields"].get("config")
            if cfg:
                keys.add(cfg["key"])
            for t in v["fields"].get("tlvs", []):
                tlvs.add(t["tlv"])
        self.assertEqual({n for n, _ in pc.CONFIG_KEYS.values()} - keys, set())
        self.assertEqual({n for n, _ in pc.CAP_TLVS.values()} - tlvs, set())

    def test_every_message_is_in_the_spec(self):
        spec = SPEC.read_text(encoding="utf-8")
        for msg_type, (name, _d, _s) in pc.MESSAGES.items():
            with self.subTest(name):
                self.assertRegex(spec, rf"\|\s*`0x{msg_type:02x}`\s*\|\s*`{name}`\s*\|")


class FramingVectors(unittest.TestCase):
    def setUp(self):
        self.data = load("framing.json")

    def test_crc_known_answers(self):
        self.assertEqual(pc.crc16(b"123456789"), 0x29B1)
        for case in self.data["crc16"]:
            data = (case["input_ascii"].encode() if "input_ascii" in case
                    else bytes.fromhex(case["input_hex"]))
            self.assertEqual(f"0x{pc.crc16(data):04x}", case["crc"])

    def test_cobs_round_trip(self):
        for case in self.data["cobs"]:
            raw, enc = bytes.fromhex(case["decoded_hex"]), bytes.fromhex(case["encoded_hex"])
            self.assertEqual(pc.cobs_encode(raw), enc)
            self.assertEqual(pc.cobs_decode(enc), raw)
            self.assertNotIn(0, enc)

    def test_cobs_published_examples(self):
        # Independent of the generator: well-known COBS encodings.
        self.assertEqual(pc.cobs_encode(bytes.fromhex("00")).hex(), "0101")
        self.assertEqual(pc.cobs_encode(bytes.fromhex("11220033")).hex(), "0311220233")
        self.assertEqual(pc.cobs_encode(bytes(range(1, 255))).hex(), "ff" + bytes(range(1, 255)).hex())

    def test_invalid_frames_are_rejected(self):
        for case in self.data["invalid"]:
            with self.subTest(case["name"]):
                with self.assertRaisesRegex(pc.ProtocolError, re.escape(case["error"])):
                    pc.decode(bytes.fromhex(case["wire_hex"]))

    def test_stream_resync(self):
        s = self.data["stream"]
        dec = pc.StreamDecoder()
        wire = bytes.fromhex(s["wire_hex"])
        msgs = []
        for i in range(0, len(wire), 3):  # arbitrary chunk boundaries
            msgs += dec.feed(wire[i:i + 3])
        self.assertEqual([m["message"] for m in msgs], s["messages"])
        self.assertEqual(dec.errors, s["errors"])


class GattVectors(unittest.TestCase):
    def setUp(self):
        self.data = load("gatt.json")

    def test_info(self):
        info = self.data["info"]
        self.assertEqual(pc.encode_info(info["fields"]).hex(), info["value_hex"])
        self.assertEqual(pc.decode_info(bytes.fromhex(info["value_hex"])), info["fields"])

    def test_chunking(self):
        c = self.data["chunking"]
        wire = bytes.fromhex(c["wire_hex"])
        chunks = [bytes.fromhex(h) for h in c["chunks_hex"]]
        self.assertEqual(pc.chunk(wire, c["att_mtu"]), chunks)
        self.assertTrue(all(len(x) <= c["att_mtu"] - 3 for x in chunks))
        dec = pc.StreamDecoder()
        msgs = [m for x in chunks for m in dec.feed(x)]
        self.assertEqual([m["message"] for m in msgs], ["CAPS"])


class CodecBehaviour(unittest.TestCase):
    def test_vectors_are_current(self):
        for fname, data in make_vectors.build().items():
            with self.subTest(fname):
                self.assertEqual((VECTORS / fname).read_text(encoding="utf-8"),
                                 make_vectors.render(data))

    def test_unknown_tlv_prefix_extension(self):
        # A newer minor version may append fields to a known TLV.
        value = bytes([0x01, 6]) + (5).to_bytes(4, "little") + b"\xaa\xbb"
        frame = pc.build_frame(0x04, 0, value)
        msg = pc.decode(pc.cobs_encode(frame) + b"\x00")
        self.assertEqual(msg["fields"]["tlvs"], [{"tlv": "FEATURES", "features": 5}])

    def test_payload_limit(self):
        with self.assertRaises(pc.ProtocolError):
            pc.encode("CAT_DATA", 0, {"port": 0, "data": "41" * pc.MAX_PAYLOAD})
        pc.encode("CAT_DATA", 0, {"port": 0, "data": "41" * (pc.MAX_PAYLOAD - 1)})

    def test_out_of_range_field(self):
        with self.assertRaises(pc.ProtocolError):
            pc.encode("PTT_SET", 0, {"state": 256})

    def test_missing_and_extra_fields(self):
        with self.assertRaises(pc.ProtocolError):
            pc.encode("PTT_SET", 0, {})
        with self.assertRaises(pc.ProtocolError):
            pc.encode("PTT_SET", 0, {"state": 1, "extra": 2})

    def test_directions(self):
        for name, direction, _schema in pc.MESSAGES.values():
            self.assertIn(direction, {"h2d", "d2h", "both"}, name)


if __name__ == "__main__":
    unittest.main()

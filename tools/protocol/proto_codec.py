#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Reference encoder/decoder for the host-device protocol (protocol/SPEC.md).

Python standard library only. It is a readable reference for implementers and
the test oracle for the golden vectors in protocol/vectors/, not a product.

Layers (SPEC.md §3):

    fields (dict)  --encode_payload-->  payload bytes
    type, token, payload  --build_frame-->  frame = type | token | payload | CRC-16
    frame  --cobs_encode + 0x00-->  wire bytes (what GATT, L2CAP CoC and CDC-ACM carry)

    python3 tools/protocol/proto_codec.py decode 0330...00   # wire hex -> JSON
"""

from __future__ import annotations

import json
import struct
import sys

PROTOCOL_VERSION = (0, 1, 0)
MAX_PAYLOAD = 1024  # SPEC.md §12: the most any implementation must accept

# --- CRC-16 and COBS (SPEC.md §3) -------------------------------------------


def crc16(data: bytes) -> int:
    """CRC-16/IBM-3740 (also called CRC-16/CCITT-FALSE): poly 0x1021,
    init 0xFFFF, no reflection, no final XOR. Check value 0x29B1."""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else (crc << 1)
            crc &= 0xFFFF
    return crc


def cobs_encode(data: bytes) -> bytes:
    """Consistent Overhead Byte Stuffing: output contains no 0x00 bytes."""
    out = bytearray()
    block = bytearray()
    full_block_at_end = False
    for byte in data:
        full_block_at_end = False
        if byte == 0:
            out.append(len(block) + 1)
            out += block
            block.clear()
        else:
            block.append(byte)
            if len(block) == 254:
                out.append(255)
                out += block
                block.clear()
                full_block_at_end = True
    if not full_block_at_end:  # a 0xFF block that ends the data needs no final code
        out.append(len(block) + 1)
        out += block
    return bytes(out)


def cobs_decode(data: bytes) -> bytes:
    out = bytearray()
    i = 0
    while i < len(data):
        code = data[i]
        if code == 0:
            raise ProtocolError("COBS: zero byte inside a frame")
        end = i + code
        if end > len(data):
            raise ProtocolError("COBS: block runs past the end of the frame")
        chunk = data[i + 1:end]
        if 0 in chunk:
            raise ProtocolError("COBS: zero byte inside a frame")
        out += chunk
        i = end
        if code != 255 and i < len(data):
            out.append(0)
    return bytes(out)


class ProtocolError(ValueError):
    """A frame or payload that a receiver must reject (SPEC.md §12)."""


# --- Field codecs -------------------------------------------------------------

_INTS = {"u8": "<B", "u16": "<H", "u32": "<I", "u64": "<Q", "i8": "<b", "i16": "<h"}


def _pack_fields(schema, values: dict) -> bytes:
    out = bytearray()
    names = {name for name, _ in schema}
    extra = set(values) - names
    if extra:
        raise ProtocolError(f"unknown fields {sorted(extra)}")
    for name, kind in schema:
        if name not in values:
            raise ProtocolError(f"missing field '{name}'")
        v = values[name]
        if kind in _INTS:
            try:
                out += struct.pack(_INTS[kind], v)
            except struct.error as exc:
                raise ProtocolError(f"field '{name}': {exc}") from None
        elif kind == "char":
            b = v.encode("ascii")
            if len(b) != 1:
                raise ProtocolError(f"field '{name}' must be one ASCII character")
            out += b
        elif kind == "hex":
            out += bytes.fromhex(v)
        elif kind == "utf8":
            out += v.encode("utf-8")
        elif kind == "tlv":
            out += _pack_tlvs(v)
        elif kind == "config":
            out += _pack_config(v)
        else:
            raise AssertionError(kind)
    return bytes(out)


def _unpack_fields(schema, data: bytes, where: str) -> dict:
    values = {}
    pos = 0
    for name, kind in schema:
        if kind in _INTS:
            size = struct.calcsize(_INTS[kind])
            if pos + size > len(data):
                raise ProtocolError(f"{where}: too short for field '{name}'")
            (values[name],) = struct.unpack_from(_INTS[kind], data, pos)
            pos += size
        elif kind == "char":
            if pos + 1 > len(data):
                raise ProtocolError(f"{where}: too short for field '{name}'")
            values[name] = data[pos:pos + 1].decode("ascii")
            pos += 1
        else:  # trailing kinds take the rest of the payload
            rest = data[pos:]
            pos = len(data)
            if kind == "hex":
                values[name] = rest.hex()
            elif kind == "utf8":
                try:
                    values[name] = rest.decode("utf-8")
                except UnicodeDecodeError:
                    raise ProtocolError(f"{where}: field '{name}' is not UTF-8") from None
            elif kind == "tlv":
                values[name] = _unpack_tlvs(rest)
            elif kind == "config":
                values[name] = _unpack_config(rest)
            else:
                raise AssertionError(kind)
    if pos != len(data):
        raise ProtocolError(f"{where}: {len(data) - pos} unexpected trailing bytes")
    return values


# --- Capability TLVs (SPEC.md §5) ---------------------------------------------

CAP_TLVS = {
    0x01: ("FEATURES", [("features", "u32")]),
    0x02: ("SERIAL_JACK", [("port", "u8"), ("modes", "u8"), ("min_baud", "u32"),
                           ("max_baud", "u32"), ("tx_buffer", "u16")]),
    0x03: ("RADIO_USB_SERIAL", [("port", "u8"), ("chip", "u8"), ("vid", "u16"),
                                ("pid", "u16"), ("interface", "u8"), ("tx_buffer", "u16")]),
    0x04: ("AUDIO", [("directions", "u8"), ("codecs", "u8"), ("rates", "u8"), ("paths", "u8"),
                     ("max_frame_samples", "u16"), ("tx_buffer_samples", "u16")]),
    0x05: ("PTT", [("outputs", "u8"), ("keepalive_min_ms", "u16"), ("keepalive_max_ms", "u16"),
                   ("max_tx_min_s", "u32")]),
    0x06: ("TONE", [("max_symbols", "u16"), ("max_tone_index", "u8"), ("shaping", "u8"),
                    ("min_symbol_us", "u32"), ("max_symbol_us", "u32")]),
    0x07: ("BLE_TX_POWER", [("min_dbm", "i8"), ("max_dbm", "i8")]),
    0x08: ("PAIRING", [("triggers", "u8"), ("window_min_s", "u16"), ("window_max_s", "u16"),
                       ("max_bonds", "u8")]),
}
_CAP_BY_NAME = {name: (tag, schema) for tag, (name, schema) in CAP_TLVS.items()}


def _pack_tlvs(items) -> bytes:
    out = bytearray()
    for item in items:
        item = dict(item)
        name = item.pop("tlv")
        if name in _CAP_BY_NAME:
            tag, schema = _CAP_BY_NAME[name]
            value = _pack_fields(schema, item)
        else:  # unknown tag, kept as raw bytes: {"tlv": "0x7f", "hex": "..."}
            tag, value = int(name, 16), bytes.fromhex(item["hex"])
        if len(value) > 255:
            raise ProtocolError("TLV value longer than 255 bytes")
        out += bytes([tag, len(value)]) + value
    return bytes(out)


def _unpack_tlvs(data: bytes) -> list:
    items, pos = [], 0
    while pos < len(data):
        if pos + 2 > len(data):
            raise ProtocolError("CAPS: truncated TLV header")
        tag, length = data[pos], data[pos + 1]
        value = data[pos + 2:pos + 2 + length]
        if len(value) != length:
            raise ProtocolError("CAPS: truncated TLV value")
        pos += 2 + length
        if tag in CAP_TLVS:
            name, schema = CAP_TLVS[tag]
            # A newer minor version may append fields: decode the known prefix.
            known = sum(struct.calcsize(_INTS[k]) for _, k in schema)
            if length < known:
                raise ProtocolError(f"CAPS: TLV {name} too short")
            fields = _unpack_fields(schema, value[:known], f"TLV {name}")
            items.append({"tlv": name, **fields})
        else:
            items.append({"tlv": f"0x{tag:02x}", "hex": value.hex()})
    return items


# --- Configuration keys (SPEC.md §6) -------------------------------------------

CONFIG_KEYS = {
    0x01: ("SERIAL_JACK_MODE", [("mode", "u8")]),
    0x02: ("PTT_TARGETS", [("targets", "u8"), ("usb_port", "u8")]),
    0x03: ("LINE_MAP", [("rts_action", "u8"), ("dtr_action", "u8")]),
    0x04: ("PTT_KEEPALIVE_MS", [("ms", "u16")]),
    0x05: ("MAX_TX_S", [("s", "u32")]),
    0x06: ("AUDIO_PATH", [("path", "u8")]),
    0x07: ("TX_LEVEL", [("centibel", "i16")]),
    0x08: ("RX_ATTENUATOR", [("on", "u8")]),
    0x09: ("RX_GAIN", [("centibel", "i16")]),
    0x0A: ("HOST_MODE", [("mode", "u8")]),
    0x0B: ("BLE_TX_POWER", [("dbm", "i8")]),
    0x0C: ("WIRED_PROFILE", [("profile", "u8")]),
    0x0D: ("SERIAL_DEFAULT", [("baud", "u32"), ("data_bits", "u8"), ("parity", "u8"),
                              ("stop_bits", "u8")]),
    0x0E: ("USB_NET_SUBNET", [("a", "u8"), ("b", "u8"), ("c", "u8"), ("d", "u8")]),
    0x0F: ("PAIRING_WINDOW_S", [("s", "u16")]),
    0x10: ("POWER_DOWN_DELAY_S", [("s", "u16")]),
}
_KEY_BY_NAME = {name: (key, schema) for key, (name, schema) in CONFIG_KEYS.items()}


def _pack_config(v: dict) -> bytes:
    key, schema = _KEY_BY_NAME[v["key"]]
    return bytes([key, v["selector"]]) + _pack_fields(schema, v["value"])


def _unpack_config(data: bytes) -> dict:
    if len(data) < 2:
        raise ProtocolError("config: missing key or selector")
    key, selector = data[0], data[1]
    if key not in CONFIG_KEYS:
        raise ProtocolError(f"config: unknown key 0x{key:02x}")
    name, schema = CONFIG_KEYS[key]
    return {"key": name, "selector": selector,
            "value": _unpack_fields(schema, data[2:], f"config {name}")}


# --- Messages (SPEC.md §4 to §11) ----------------------------------------------
# type: (name, direction, fields). Direction: "h2d", "d2h" or "both".

MESSAGES = {
    # Session and discovery
    0x01: ("HELLO", "h2d", [("proto_major", "u8"), ("proto_minor", "u8"), ("proto_patch", "u8"),
                            ("max_payload", "u16"), ("flags", "u8")]),
    0x02: ("DEVICE_INFO", "d2h", [("proto_major", "u8"), ("proto_minor", "u8"),
                                  ("proto_patch", "u8"), ("fw_major", "u8"), ("fw_minor", "u8"),
                                  ("fw_patch", "u8"), ("variant", "char"),
                                  ("hw_revision", "char"), ("max_payload", "u16"),
                                  ("build", "utf8")]),
    0x03: ("CAPS_GET", "h2d", []),
    0x04: ("CAPS", "d2h", [("tlvs", "tlv")]),
    0x05: ("RESULT", "d2h", [("request_type", "u8"), ("code", "u8")]),
    0x06: ("PING", "h2d", [("data", "hex")]),
    0x07: ("PONG", "d2h", [("data", "hex")]),
    0x08: ("STATUS_GET", "h2d", []),
    0x09: ("STATUS", "d2h", [("host_link", "u8"), ("transport", "u8"), ("phy", "u8"),
                             ("conn_interval_us", "u32"), ("att_mtu", "u16"), ("coc_mtu", "u16"),
                             ("flags", "u16"), ("audio_path", "u8"), ("supply_mv", "u16"),
                             ("frame_errors", "u16"), ("cat_overflows", "u16")]),
    # Configuration
    0x10: ("CONFIG_GET", "h2d", [("key", "u8"), ("selector", "u8")]),
    0x11: ("CONFIG_SET", "h2d", [("flags", "u8"), ("config", "config")]),
    0x12: ("CONFIG", "d2h", [("config", "config")]),
    0x13: ("CONFIG_RESET", "h2d", [("flags", "u8")]),
    # Serial / CAT
    0x20: ("SERIAL_OPEN", "h2d", [("port", "u8"), ("open", "u8")]),
    0x21: ("SERIAL_SET", "h2d", [("port", "u8"), ("baud", "u32"), ("data_bits", "u8"),
                                 ("parity", "u8"), ("stop_bits", "u8")]),
    0x22: ("CAT_DATA", "both", [("port", "u8"), ("data", "hex")]),
    0x23: ("CAT_CREDIT", "d2h", [("port", "u8"), ("credit", "u16")]),
    0x24: ("MODEM_LINES", "h2d", [("port", "u8"), ("lines", "u8")]),
    0x25: ("MODEM_STATUS", "d2h", [("port", "u8"), ("lines", "u8")]),
    # PTT
    0x30: ("PTT_SET", "h2d", [("state", "u8")]),
    0x31: ("PTT_STATUS", "d2h", [("state", "u8"), ("sources", "u8"), ("reason", "u8"),
                                 ("remaining_s", "u32")]),
    0x32: ("KEEPALIVE", "h2d", []),
    # Audio
    0x40: ("AUDIO_START", "h2d", [("direction", "u8"), ("codec", "u8"), ("sample_rate", "u16"),
                                  ("frame_samples", "u16"), ("codec_param", "u16"),
                                  ("start_time_us", "u64")]),
    0x41: ("AUDIO_STOP", "h2d", [("direction", "u8")]),
    0x42: ("AUDIO_FRAME", "both", [("seq", "u16"), ("timestamp", "u32"), ("flags", "u8"),
                                   ("data", "hex")]),
    0x43: ("AUDIO_STATUS", "d2h", [("direction", "u8"), ("state", "u8"), ("fill_samples", "u16"),
                                   ("target_samples", "u16"), ("underruns", "u16"),
                                   ("overruns", "u16"), ("first_sample_time_us", "u64")]),
    # Clock
    0x50: ("TIME_REQ", "h2d", [("host_t1", "u64")]),
    0x51: ("TIME_RESP", "d2h", [("host_t1", "u64"), ("device_t2", "u64"), ("device_t3", "u64")]),
    0x52: ("TIME_SET", "h2d", [("device_time_us", "u64"), ("utc_us", "u64"),
                               ("uncertainty_us", "u32")]),
    # Tone-sequence TX (optional)
    0x60: ("TONE_SETUP", "h2d", [("count", "u16"), ("base_mhz", "u32"), ("spacing_mhz", "u32"),
                                 ("symbol_us", "u32"), ("shaping", "u8"), ("bt_x100", "u8"),
                                 ("amplitude", "u16"), ("ramp_us", "u16"), ("ptt_lead_ms", "u16"),
                                 ("ptt_tail_ms", "u16"), ("flags", "u8")]),
    0x61: ("TONE_DATA", "h2d", [("offset", "u16"), ("tones", "hex")]),
    0x62: ("TONE_START", "h2d", [("start_utc_us", "u64")]),
    0x63: ("TONE_CANCEL", "h2d", []),
    0x64: ("TONE_STATUS", "d2h", [("state", "u8"), ("symbol", "u16"), ("reason", "u8")]),
}
_MSG_BY_NAME = {name: (t, schema) for t, (name, _d, schema) in MESSAGES.items()}

# The GATT Info characteristic (SPEC.md §13.2) is read, not framed.
INFO_SCHEMA = [("proto_major", "u8"), ("proto_minor", "u8"), ("proto_patch", "u8"),
               ("flags", "u8"), ("psm", "u16")]


def encode_payload(name: str, fields: dict) -> tuple[int, bytes]:
    msg_type, schema = _MSG_BY_NAME[name]
    payload = _pack_fields(schema, fields)
    if len(payload) > MAX_PAYLOAD:
        raise ProtocolError("payload longer than MAX_PAYLOAD")
    return msg_type, payload


def decode_payload(msg_type: int, payload: bytes) -> tuple[str, dict]:
    if msg_type not in MESSAGES:
        raise ProtocolError(f"unknown message type 0x{msg_type:02x}")
    name, _direction, schema = MESSAGES[msg_type]
    return name, _unpack_fields(schema, payload, name)


def build_frame(msg_type: int, token: int, payload: bytes) -> bytes:
    body = bytes([msg_type, token]) + payload
    return body + struct.pack("<H", crc16(body))


def parse_frame(frame: bytes) -> tuple[int, int, bytes]:
    if len(frame) < 4:
        raise ProtocolError("frame shorter than 4 bytes")
    body, (crc,) = frame[:-2], struct.unpack("<H", frame[-2:])
    if crc16(body) != crc:
        raise ProtocolError("CRC mismatch")
    if len(body) - 2 > MAX_PAYLOAD:
        raise ProtocolError("payload longer than MAX_PAYLOAD")
    return body[0], body[1], body[2:]


def encode(name: str, token: int, fields: dict) -> bytes:
    """Message -> wire bytes (COBS-encoded frame plus the 0x00 delimiter)."""
    msg_type, payload = encode_payload(name, fields)
    return cobs_encode(build_frame(msg_type, token, payload)) + b"\x00"


def decode(wire: bytes) -> dict:
    """Wire bytes of exactly one frame (with its 0x00 delimiter) -> message."""
    if not wire.endswith(b"\x00") or b"\x00" in wire[:-1]:
        raise ProtocolError("expected exactly one 0x00-delimited frame")
    msg_type, token, payload = parse_frame(cobs_decode(wire[:-1]))
    name, fields = decode_payload(msg_type, payload)
    return {"message": name, "token": token, "fields": fields}


class StreamDecoder:
    """Splits a byte stream (GATT chunks, L2CAP SDUs, CDC-ACM reads) into
    messages. Bad frames are counted and skipped, as SPEC.md §3.4 requires."""

    def __init__(self) -> None:
        self._buf = bytearray()
        self.errors = 0

    def feed(self, data: bytes) -> list[dict]:
        out = []
        for byte in data:
            if byte != 0:
                self._buf.append(byte)
                continue
            raw, self._buf = bytes(self._buf), bytearray()
            if not raw:
                continue  # empty frames are padding / resync
            try:
                out.append(decode(raw + b"\x00"))
            except ProtocolError:
                self.errors += 1
        return out


def encode_info(fields: dict) -> bytes:
    return _pack_fields(INFO_SCHEMA, fields)


def decode_info(data: bytes) -> dict:
    return _unpack_fields(INFO_SCHEMA, data, "Info")


def chunk(wire: bytes, att_mtu: int) -> list[bytes]:
    """Split a byte stream into GATT writes/notifications of at most ATT_MTU - 3."""
    size = att_mtu - 3
    return [wire[i:i + size] for i in range(0, len(wire), size)]


def main(argv: list[str]) -> int:
    if len(argv) == 2 and argv[0] == "decode":
        dec = StreamDecoder()
        msgs = dec.feed(bytes.fromhex(argv[1]))
        print(json.dumps(msgs, indent=2))
        return 1 if dec.errors else 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

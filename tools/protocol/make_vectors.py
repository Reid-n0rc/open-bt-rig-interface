#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Write or check the golden vectors in protocol/vectors/.

    python3 tools/protocol/make_vectors.py           # rewrite the JSON files
    python3 tools/protocol/make_vectors.py --check   # fail if they're out of date

The examples below are the source of the vectors. Changing a message format
changes its vectors, which needs a protocol version bump (AGENTS.md, Protocol).
Values are examples only; none is tied to a particular radio or host app.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

import proto_codec as pc

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "protocol" / "vectors"

# (vector name, message, token, fields, note)
EXAMPLES = [
    ("hello", "HELLO", 1,
     {"proto_major": 0, "proto_minor": 1, "proto_patch": 0, "max_payload": 1024, "flags": 0},
     "Host opens a session and states its protocol version."),
    ("device_info", "DEVICE_INFO", 1,
     {"proto_major": 0, "proto_minor": 1, "proto_patch": 0, "fw_major": 0, "fw_minor": 1,
      "fw_patch": 0, "variant": "R", "hw_revision": "A", "max_payload": 1024,
      "build": "0.1.0+g1a2b3c4"},
     "Reply to HELLO. build is free UTF-8 text."),
    ("caps_get", "CAPS_GET", 2, {}, "Request the capability list."),
    ("caps_full", "CAPS", 2, {"tlvs": [
        {"tlv": "FEATURES", "features": 0x37FFF},
        {"tlv": "SERIAL_JACK", "port": 0, "modes": 0x0F, "min_baud": 4800, "max_baud": 115200,
         "tx_buffer": 512},
        {"tlv": "RADIO_USB_SERIAL", "port": 1, "chip": 1, "vid": 0x10C4, "pid": 0xEA70,
         "interface": 0, "tx_buffer": 512},
        {"tlv": "RADIO_USB_SERIAL", "port": 2, "chip": 1, "vid": 0x10C4, "pid": 0xEA70,
         "interface": 1, "tx_buffer": 512},
        {"tlv": "AUDIO", "directions": 3, "codecs": 1, "rates": 3, "paths": 3,
         "max_frame_samples": 480, "tx_buffer_samples": 2400},
        {"tlv": "PTT", "outputs": 7, "keepalive_min_ms": 500, "keepalive_max_ms": 10000,
         "max_tx_min_s": 10, "max_tx_max_s": 600, "hw_max_tx_s": 600},
        {"tlv": "TONE", "max_symbols": 256, "max_tone_index": 15, "shaping": 3,
         "min_symbol_us": 1000, "max_symbol_us": 2000000},
        {"tlv": "BLE_TX_POWER", "min_dbm": -24, "max_dbm": 9},
        {"tlv": "PAIRING", "triggers": 3, "window_min_s": 30, "window_max_s": 600,
         "max_bonds": 8},
     ]}, "Every TLV. The radio USB entries show a dual-port USB-serial chip (VID/PID are examples)."),
    ("caps_minimal", "CAPS", 0, {"tlvs": [
        {"tlv": "FEATURES", "features": 0x0003},
        {"tlv": "SERIAL_JACK", "port": 0, "modes": 0x01, "min_baud": 4800, "max_baud": 115200,
         "tx_buffer": 256},
        {"tlv": "PTT", "outputs": 1, "keepalive_min_ms": 500, "keepalive_max_ms": 10000,
         "max_tx_min_s": 10, "max_tx_max_s": 600, "hw_max_tx_s": 600},
     ]}, "Unsolicited (token 0) after a change; a device with CAT and PTT only."),
    ("caps_unknown_tlv", "CAPS", 0, {"tlvs": [
        {"tlv": "FEATURES", "features": 0x0001},
        {"tlv": "0x7f", "hex": "0102"},
     ]}, "A TLV tag from a newer minor version: receivers skip it by its length."),
    ("result_ok", "RESULT", 3, {"request_type": 0x11, "code": 0}, "Success for a request with no specific reply."),
    ("result_unsupported", "RESULT", 4, {"request_type": 0x40, "code": 4},
     "AUDIO_START on a device without the feature."),
    ("result_frame_error", "RESULT", 0, {"request_type": 0x00, "code": 10},
     "A frame failed its CRC; token and type unknown."),
    ("ping", "PING", 5, {"data": "deadbeef"}, "Echo request with opaque data."),
    ("pong", "PONG", 5, {"data": "deadbeef"}, "Echo reply."),
    ("status_get", "STATUS_GET", 6, {}, "Request link and device status."),
    ("status_ble_l2cap", "STATUS", 6,
     {"host_link": 1, "transport": 2, "phy": 2, "conn_interval_us": 15000, "att_mtu": 247,
      "coc_mtu": 512, "flags": 0x000B, "audio_path": 2, "supply_mv": 5020,
      "frame_errors": 0, "cat_overflows": 0},
     "Bluetooth, L2CAP CoC, 2M PHY, 15 ms interval; radio USB serial and sound card present; time set."),
    ("status_pairing_open", "STATUS", 0,
     {"host_link": 1, "transport": 1, "phy": 2, "conn_interval_us": 30000, "att_mtu": 185,
      "coc_mtu": 0, "flags": 0x0040, "audio_path": 0, "supply_mv": 0,
      "frame_errors": 0, "cat_overflows": 0},
     "Unsolicited: the pairing window is open (flag bit 6)."),
    ("status_wired", "STATUS", 0,
     {"host_link": 2, "transport": 3, "phy": 0, "conn_interval_us": 0, "att_mtu": 0,
      "coc_mtu": 0, "flags": 0x0004, "audio_path": 1, "supply_mv": 4870,
      "frame_errors": 2, "cat_overflows": 0},
     "Wired CDC-ACM control port; a radio USB device is attached but not usable."),
    ("status_usb_network", "STATUS", 37,
     {"host_link": 2, "transport": 4, "phy": 0, "conn_interval_us": 0, "att_mtu": 0,
      "coc_mtu": 0, "flags": 0x0018, "audio_path": 1, "supply_mv": 5010,
      "frame_errors": 0, "cat_overflows": 0},
     "Wired, TCP over the USB network interface; time set; USB-C source advertises 1.5 A or more."),
    ("config_get_line_map", "CONFIG_GET", 7, {"key": 0x03, "selector": 0},
     "Read the RTS/DTR mapping of port 0 (SERIAL jack)."),
    ("config_set_jack_mode", "CONFIG_SET", 8,
     {"flags": 1, "config": {"key": "SERIAL_JACK_MODE", "selector": 0, "value": {"mode": 2}}},
     "SERIAL jack to CI-V, persisted."),
    ("config_set_ptt_targets", "CONFIG_SET", 9,
     {"flags": 0, "config": {"key": "PTT_TARGETS", "selector": 0,
                             "value": {"targets": 3, "usb_port": 1}}},
     "PTT drives the AUDIO-jack closure and RTS on radio USB port 1."),
    ("config_set_line_map", "CONFIG_SET", 10,
     {"flags": 1, "config": {"key": "LINE_MAP", "selector": 0,
                             "value": {"rts_action": 1, "dtr_action": 0}}},
     "Port 0: RTS keys PTT, DTR ignored."),
    ("config_set_keepalive", "CONFIG_SET", 11,
     {"flags": 1, "config": {"key": "PTT_KEEPALIVE_MS", "selector": 0, "value": {"ms": 3000}}},
     "Keepalive timeout 3 s."),
    ("config_set_max_tx", "CONFIG_SET", 12,
     {"flags": 1, "config": {"key": "MAX_TX_S", "selector": 0, "value": {"s": 180}}},
     "Maximum continuous TX 180 s."),
    ("config_set_audio_path", "CONFIG_SET", 13,
     {"flags": 0, "config": {"key": "AUDIO_PATH", "selector": 0, "value": {"path": 1}}},
     "Force the analog audio path."),
    ("config_set_tx_level", "CONFIG_SET", 14,
     {"flags": 1, "config": {"key": "TX_LEVEL", "selector": 0, "value": {"centibel": -120}}},
     "TX level -12.0 dB."),
    ("config_set_rx_attenuator", "CONFIG_SET", 15,
     {"flags": 1, "config": {"key": "RX_ATTENUATOR", "selector": 0, "value": {"on": 1}}},
     "RX attenuator on."),
    ("config_set_rx_gain", "CONFIG_SET", 16,
     {"flags": 0, "config": {"key": "RX_GAIN", "selector": 0, "value": {"centibel": 35}}},
     "RX gain +3.5 dB."),
    ("config_set_host_mode", "CONFIG_SET", 17,
     {"flags": 1, "config": {"key": "HOST_MODE", "selector": 0, "value": {"mode": 0}}},
     "Host mode automatic."),
    ("config_set_ble_tx_power", "CONFIG_SET", 18,
     {"flags": 1, "config": {"key": "BLE_TX_POWER", "selector": 0, "value": {"dbm": 6}}},
     "BLE TX power 6 dBm (must not exceed the BLE_TX_POWER capability maximum)."),
    ("config_set_wired_profile", "CONFIG_SET", 36,
     {"flags": 1, "config": {"key": "WIRED_PROFILE", "selector": 0, "value": {"profile": 1}}},
     "Wired profile 'serial' (the default): CDC-ACM bridge port for SERIAL-jack radios."),
    ("config_set_wired_profile_network", "CONFIG_SET", 41,
     {"flags": 1, "config": {"key": "WIRED_PROFILE", "selector": 0, "value": {"profile": 0}}},
     "Wired profile 'network' (for iPhone/iPad): USB network interface instead of the serial port."),
    ("config_set_serial_default", "CONFIG_SET", 38,
     {"flags": 1, "config": {"key": "SERIAL_DEFAULT", "selector": 0,
                             "value": {"baud": 9600, "data_bits": 8, "parity": 0, "stop_bits": 0}}},
     "Port 0 line settings applied at SERIAL_OPEN: 9600 8N1 (the default)."),
    ("config_set_usb_net_subnet", "CONFIG_SET", 39,
     {"flags": 1, "config": {"key": "USB_NET_SUBNET", "selector": 0,
                             "value": {"a": 10, "b": 169, "c": 160, "d": 0}}},
     "USB network /30 subnet 10.169.160.0 (the default); octets in address order."),
    ("config_set_pairing_window", "CONFIG_SET", 40,
     {"flags": 1, "config": {"key": "PAIRING_WINDOW_S", "selector": 0, "value": {"s": 120}}},
     "Pairing window length 120 s (the default)."),
    ("config_report", "CONFIG", 7,
     {"config": {"key": "LINE_MAP", "selector": 0, "value": {"rts_action": 1, "dtr_action": 0}}},
     "Reply to CONFIG_GET (also sent with token 0 when a value changes)."),
    ("config_reset", "CONFIG_RESET", 19, {"flags": 1}, "Restore defaults and persist them."),
    ("serial_open", "SERIAL_OPEN", 20, {"port": 0, "open": 1}, "Open port 0; RTS/DTR arming restarts."),
    ("serial_close", "SERIAL_OPEN", 21, {"port": 0, "open": 0}, "Close port 0; its lines drop."),
    ("serial_set", "SERIAL_SET", 22,
     {"port": 0, "baud": 38400, "data_bits": 8, "parity": 0, "stop_bits": 0}, "38400 8N1."),
    ("serial_set_8n2", "SERIAL_SET", 23,
     {"port": 1, "baud": 4800, "data_bits": 8, "parity": 0, "stop_bits": 2}, "4800 8N2 on radio USB port 1."),
    ("cat_data_h2d", "CAT_DATA", 0, {"port": 0, "data": "46413b"},
     "Host to radio: three opaque CAT bytes."),
    ("cat_data_d2h_zeros", "CAT_DATA", 0, {"port": 2, "data": "fefe00e0fd"},
     "Radio to host with 0x00 bytes inside (COBS removes them on the wire)."),
    ("cat_credit", "CAT_CREDIT", 0, {"port": 0, "credit": 256}, "256 more bytes may be sent to port 0."),
    ("modem_lines", "MODEM_LINES", 0, {"port": 0, "lines": 0x02}, "RTS asserted, DTR deasserted."),
    ("modem_status", "MODEM_STATUS", 0, {"port": 1, "lines": 0x03}, "CTS and DSR asserted on radio USB port 1."),
    ("ptt_on", "PTT_SET", 24, {"state": 1}, "Key PTT (re-send, or KEEPALIVE, within the keepalive time)."),
    ("ptt_off", "PTT_SET", 25, {"state": 0}, "Release PTT."),
    ("ptt_status_keyed", "PTT_STATUS", 24, {"state": 1, "sources": 0x01, "reason": 0, "remaining_s": 180},
     "Keyed by the protocol command; 180 s of TX left."),
    ("ptt_status_keepalive_timeout", "PTT_STATUS", 0,
     {"state": 0, "sources": 0x00, "reason": 2, "remaining_s": 0},
     "Unsolicited: PTT dropped because the keepalive expired."),
    ("ptt_status_hw_watchdog", "PTT_STATUS", 0,
     {"state": 0, "sources": 0x00, "reason": 12, "remaining_s": 0},
     "Reported after the fact: the hardware PTT watchdog forced PTT off."),
    ("keepalive", "KEEPALIVE", 0, {}, "Refreshes every protocol PTT source."),
    ("audio_start_rx", "AUDIO_START", 26,
     {"direction": 1, "codec": 0, "sample_rate": 12000, "frame_samples": 120, "codec_param": 0,
      "start_time_us": 0},
     "RX (radio to host) 12 kHz PCM, 10 ms frames, start now."),
    ("audio_start_tx_scheduled", "AUDIO_START", 27,
     {"direction": 2, "codec": 0, "sample_rate": 16000, "frame_samples": 160, "codec_param": 0,
      "start_time_us": 1234567890},
     "TX 16 kHz PCM; sample 0 plays at device time 1234567890 us."),
    ("audio_start_lc3", "AUDIO_START", 28,
     {"direction": 1, "codec": 1, "sample_rate": 16000, "frame_samples": 160, "codec_param": 40,
      "start_time_us": 0},
     "Optional LC3, 10 ms frames of 40 bytes."),
    ("audio_stop", "AUDIO_STOP", 29, {"direction": 1}, "Stop RX audio."),
    ("audio_frame", "AUDIO_FRAME", 0,
     {"seq": 7, "timestamp": 840, "flags": 0, "data": "0000ff7f0180fffe"},
     "Four PCM16LE samples (0, 32767, -32767, -257) at sample index 840."),
    ("audio_frame_gap", "AUDIO_FRAME", 0,
     {"seq": 65535, "timestamp": 4294967200, "flags": 1, "data": "01000200"},
     "Discontinuity flag set; seq and timestamp near their wrap points."),
    ("audio_status", "AUDIO_STATUS", 0,
     {"direction": 2, "state": 2, "fill_samples": 720, "target_samples": 720, "underruns": 0,
      "overruns": 0, "first_sample_time_us": 1234567890},
     "TX running at its target fill."),
    ("time_req", "TIME_REQ", 30, {"host_t1": 1000000}, "Clock-sync probe; host_t1 is opaque to the device."),
    ("time_resp", "TIME_RESP", 30, {"host_t1": 1000000, "device_t2": 55000000, "device_t3": 55000120},
     "Device receive and transmit times, microseconds since boot."),
    ("time_set", "TIME_SET", 31,
     {"device_time_us": 55000060, "utc_us": 1790000000000000, "uncertainty_us": 8000},
     "UTC at a device time, with the host's error estimate."),
    ("tone_setup_ft8_example", "TONE_SETUP", 32,
     {"count": 79, "base_mhz": 1500000, "spacing_mhz": 6250, "symbol_us": 160000, "shaping": 1,
      "bt_x100": 200, "amplitude": 52000, "ramp_us": 5000, "ptt_lead_ms": 50, "ptt_tail_ms": 20,
      "flags": 1},
     "Example only (FT8-like: 8 tones, 6.25 Hz, 160 ms, GFSK BT 2.0; verify against the mode's spec)."),
    ("tone_data", "TONE_DATA", 33, {"offset": 0, "tones": "03010406000502"},
     "First seven tone indices."),
    ("tone_start", "TONE_START", 34, {"start_utc_us": 1790000015000000}, "Start at a UTC time."),
    ("tone_cancel", "TONE_CANCEL", 35, {}, "Cancel a scheduled or running sequence."),
    ("tone_status", "TONE_STATUS", 0, {"state": 4, "symbol": 12, "reason": 0}, "Playing symbol 12."),
]

INVALID = [
    ("bad_crc", "0630010129 2b00", "CRC mismatch"),
    ("unknown_type", None, "unknown message type"),
    ("short_payload", None, "too short"),
    ("trailing_bytes", None, "unexpected trailing bytes"),
    ("unknown_config_key", None, "unknown key"),
    ("zero_in_frame", "063001000129 2a00", "0x00"),
]


def _invalid_wire(name: str, fixed: str | None) -> bytes:
    if fixed:
        return bytes.fromhex(fixed.replace(" ", ""))
    frames = {
        "unknown_type": pc.build_frame(0x7E, 1, b""),
        "short_payload": pc.build_frame(0x30, 1, b""),          # PTT_SET without its state byte
        "trailing_bytes": pc.build_frame(0x32, 1, b"\x01"),     # KEEPALIVE with a stray byte
        "unknown_config_key": pc.build_frame(0x12, 1, b"\xee\x00\x01"),
    }
    return pc.cobs_encode(frames[name]) + b"\x00"


def build() -> dict[str, dict]:
    version = ".".join(map(str, pc.PROTOCOL_VERSION))
    header = {
        "protocol_version": version,
        "spec": "protocol/SPEC.md",
        "generated_by": "tools/protocol/make_vectors.py",
    }
    messages = []
    for name, msg, token, fields, note in EXAMPLES:
        msg_type, payload = pc.encode_payload(msg, fields)
        frame = pc.build_frame(msg_type, token, payload)
        messages.append({
            "name": name,
            "message": msg,
            "type": f"0x{msg_type:02x}",
            "direction": pc.MESSAGES[msg_type][1],
            "token": token,
            "fields": fields,
            "payload_hex": payload.hex(),
            "frame_hex": frame.hex(),
            "wire_hex": (pc.cobs_encode(frame) + b"\x00").hex(),
            "note": note,
        })

    cobs_cases = [b"", b"\x00", b"\x00\x00", b"\x11\x22\x00\x33", b"\x11\x00\x00\x00",
                  bytes(range(1, 255)), bytes(range(0, 255)), bytes(range(1, 256))]
    framing = {
        **header,
        "crc16": [{"input_ascii": "123456789", "crc": "0x29b1"},
                  {"input_hex": "3001", "crc": f"0x{pc.crc16(bytes.fromhex('3001')):04x}"}],
        "cobs": [{"decoded_hex": d.hex(), "encoded_hex": pc.cobs_encode(d).hex()}
                 for d in cobs_cases],
        "invalid": [{"name": n, "wire_hex": _invalid_wire(n, h).hex(), "error": e}
                    for n, h, e in INVALID],
        "stream": _stream_example(),
    }

    info = {"proto_major": 0, "proto_minor": 1, "proto_patch": 0, "flags": 1, "psm": 0x0080}
    info_open = {"proto_major": 0, "proto_minor": 1, "proto_patch": 0, "flags": 3, "psm": 0x0080}
    wire = bytes.fromhex(next(m["wire_hex"] for m in messages if m["name"] == "caps_full"))
    gatt = {
        **header,
        "info": {"fields": info, "value_hex": pc.encode_info(info).hex(),
                 "note": "Info characteristic value; psm 0x0080 is an example, read it from the device."},
        "info_pairing_open": {"fields": info_open, "value_hex": pc.encode_info(info_open).hex(),
                              "note": "Info with flags bit 1 set: the pairing window is open."},
        "chunking": {
            "att_mtu": 23,
            "wire_hex": wire.hex(),
            "chunks_hex": [c.hex() for c in pc.chunk(wire, 23)],
            "note": "The CAPS reply split into 20-byte notifications (ATT_MTU 23, the minimum).",
        },
    }
    return {"messages.json": {**header, "messages": messages},
            "framing.json": framing, "gatt.json": gatt}


def _stream_example() -> dict:
    """Two messages, a leading resync delimiter and a corrupt frame in one stream."""
    a = pc.encode("KEEPALIVE", 0, {})
    b = pc.encode("PTT_SET", 2, {"state": 0})
    corrupt = bytes.fromhex("0630010129ff00")
    stream = b"\x00" + a + corrupt + b
    return {"wire_hex": stream.hex(), "messages": ["KEEPALIVE", "PTT_SET"], "errors": 1,
            "note": "Empty frames are skipped; the corrupt frame is dropped and counted."}


def render(data: dict) -> str:
    return json.dumps(data, indent=2, ensure_ascii=False) + "\n"


def main(argv: list[str]) -> int:
    files = build()
    check = "--check" in argv
    stale = []
    for fname, data in files.items():
        path = OUT / fname
        text = render(data)
        if check:
            if not path.exists() or path.read_text(encoding="utf-8") != text:
                stale.append(fname)
        else:
            OUT.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding="utf-8")
    if stale:
        print("Out of date (run tools/protocol/make_vectors.py):", ", ".join(stale))
        return 1
    print("vectors " + ("up to date" if check else "written") + f": {', '.join(files)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

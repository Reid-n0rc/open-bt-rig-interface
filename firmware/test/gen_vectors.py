#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
"""Turn the golden vectors in protocol/vectors/*.json into a C test table.

    python3 firmware/test/gen_vectors.py --out build/vectors_gen.c

The host build runs this at build time, so the C tests always use the
current JSON. Field kinds come from the reference codec's schemas
(tools/protocol/proto_codec.py). Every message field is set and checked
through the C struct member of the same name (firmware/app/include/proto.h).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "protocol"))

import proto_codec as pc  # noqa: E402

VECTORS = ROOT / "protocol" / "vectors"

SIGNED = {"i8", "i16"}
INTS = {"u8", "u16", "u32", "u64", "i8", "i16"}

# Error text from framing.json "invalid" -> expected C result.
INVALID_EXPECT = {
    "CRC mismatch": "VEC_FRAME_ERROR",
    "0x00": "VEC_FRAME_ERROR",
    "unknown message type": "PROTO_ERR_UNKNOWN_TYPE",
    "too short": "PROTO_ERR_BAD_LENGTH",
    "unexpected trailing bytes": "PROTO_ERR_BAD_LENGTH",
    "unknown key": "PROTO_ERR_BAD_VALUE",
}


class Gen:
    def __init__(self) -> None:
        self.decls: list[str] = []
        self.n = 0

    def arr(self, data: bytes) -> str:
        """Declare a static byte array and return its vbytes_t initializer."""
        self.n += 1
        name = f"b{self.n}"
        body = ", ".join(f"0x{b:02x}" for b in data) or "0"
        self.decls.append(f"static const uint8_t {name}[] = {{{body}}};")
        return f"{{{name}, {len(data)}u}}"

    def raw(self, data: bytes) -> str:
        """Declare a static byte array and return its name."""
        self.n += 1
        name = f"b{self.n}"
        body = ", ".join(f"0x{b:02x}" for b in data) or "0"
        self.decls.append(f"static const uint8_t {name}[] = {{{body}}};")
        return name


def c_int(v: int, kind: str) -> str:
    if kind in SIGNED:
        return f"(int64_t){v}"
    return f"(uint64_t){v}ull"


def field_lines(g: Gen, schema, values: dict, path: str, fill: list[str], check: list[str],
                where: str) -> None:
    """Emit assignments (fill) and assertions (check) for a field list."""
    for name, kind in schema:
        v = values[name]
        member = f"{path}.{name}"
        label = f'"{where}.{name}"'
        if kind in INTS:
            ctype = "int64_t" if kind in SIGNED else "uint64_t"
            suffix = "" if kind in SIGNED else ("ull" if kind == "u64" else "u")
            fill.append(f"    m->{member} = {v}{suffix};")
            check.append(f"    TEST_ASSERT_TRUE_MESSAGE(({ctype})m->{member} == {c_int(v, kind)}, {label});")
        elif kind == "char":
            fill.append(f"    m->{member} = '{v}';")
            check.append(f"    TEST_ASSERT_EQUAL_CHAR_MESSAGE('{v}', m->{member}, {label});")
        elif kind in ("hex", "utf8"):
            data = bytes.fromhex(v) if kind == "hex" else v.encode("utf-8")
            a = g.raw(data)
            fill.append(f"    m->{member}.data = {a};")
            fill.append(f"    m->{member}.len = {len(data)}u;")
            check.append(f"    TEST_ASSERT_EQUAL_UINT16_MESSAGE({len(data)}u, m->{member}.len, {label});")
            if data:
                check.append(f"    TEST_ASSERT_EQUAL_MEMORY_MESSAGE({a}, m->{member}.data, {len(data)}u, {label});")
        elif kind.startswith("b") and kind[1:].isdigit():  # fixed-length bytes
            data = bytes.fromhex(v)
            a = g.raw(data)
            fill.append(f"    memcpy(m->{member}, {a}, {len(data)}u);")
            check.append(f"    TEST_ASSERT_EQUAL_MEMORY_MESSAGE({a}, m->{member}, {len(data)}u, {label});")
        elif kind == "trust":
            fill.append("    size_t pos = 0;")
            fill.append("    proto_trust_record_t r;")
            check.append(f"    proto_bytes_t cur = m->{member};")
            check.append("    proto_trust_record_t r;")
            for i, rec in enumerate(v):
                ident = bytes.fromhex(rec["ident"])
                a = g.raw(ident)
                fill.append(f"    r.kind = {rec['kind']};")
                fill.append(f"    r.slot = {rec['slot']};")
                fill.append(f"    memcpy(r.ident, {a}, {len(ident)}u);")
                fill.append("    TEST_ASSERT_EQUAL_INT(PROTO_OK, proto_trust_put(&r, buf, cap, &pos));")
                check.append(f'    TEST_ASSERT_EQUAL_INT_MESSAGE(1, proto_trust_next(&cur, &r), "{where} record {i}");')
                check.append(f"    TEST_ASSERT_EQUAL_UINT8({rec['kind']}, r.kind);")
                check.append(f"    TEST_ASSERT_EQUAL_UINT8({rec['slot']}, r.slot);")
                check.append(f"    TEST_ASSERT_EQUAL_MEMORY({a}, r.ident, {len(ident)}u);")
            fill.append(f"    m->{member}.data = buf;")
            fill.append(f"    m->{member}.len = (uint16_t)pos;")
            check.append(f'    TEST_ASSERT_EQUAL_INT_MESSAGE(0, proto_trust_next(&cur, &r), "{where}: extra records");')
        elif kind == "config":
            key, schema_v = pc._KEY_BY_NAME[v["key"]]
            fill.append(f"    m->{member}.key = 0x{key:02x};")
            fill.append(f"    m->{member}.selector = {v['selector']};")
            check.append(f"    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x{key:02x}, m->{member}.key, {label});")
            check.append(f"    TEST_ASSERT_EQUAL_UINT8_MESSAGE({v['selector']}, m->{member}.selector, {label});")
            field_lines(g, schema_v, v["value"], f"{member}.value.{v['key'].lower()}", fill, check,
                        f"{where}.{v['key']}")
        elif kind == "tlv":
            tlv_lines(g, v, member, fill, check, where)
        else:
            raise AssertionError(kind)


def tlv_lines(g: Gen, items, member: str, fill: list[str], check: list[str], where: str) -> None:
    fill.append("    size_t pos = 0;")
    fill.append("    proto_tlv_t t;")
    check.append(f"    proto_bytes_t cur = m->{member};")
    check.append("    proto_tlv_t t;")
    for i, item in enumerate(items):
        item = dict(item)
        name = item.pop("tlv")
        fill.append("    memset(&t, 0, sizeof(t));")
        check.append(f'    TEST_ASSERT_EQUAL_INT_MESSAGE(1, proto_tlv_next(&cur, &t), "{where} TLV {i}");')
        if name in pc._CAP_BY_NAME:
            tag, schema = pc._CAP_BY_NAME[name]
            fill.append(f"    t.tag = 0x{tag:02x};")
            fill.append("    t.known = true;")
            check.append(f"    TEST_ASSERT_EQUAL_UINT8(0x{tag:02x}, t.tag);")
            check.append("    TEST_ASSERT_TRUE(t.known);")
            sub_fill: list[str] = []
            sub_check: list[str] = []
            field_lines(g, schema, item, f"v.{name.lower()}", sub_fill, sub_check, f"{where}.{name}")
            fill += [s.replace("m->v.", "t.v.") for s in sub_fill]
            check += [s.replace("m->v.", "t.v.") for s in sub_check]
        else:
            tag = int(name, 16)
            data = bytes.fromhex(item["hex"])
            a = g.raw(data)
            fill.append(f"    t.tag = 0x{tag:02x};")
            fill.append(f"    t.value = {a};")
            fill.append(f"    t.len = {len(data)}u;")
            check.append(f"    TEST_ASSERT_EQUAL_UINT8(0x{tag:02x}, t.tag);")
            check.append("    TEST_ASSERT_FALSE(t.known);")
            check.append(f"    TEST_ASSERT_EQUAL_UINT8({len(data)}u, t.len);")
            check.append(f"    TEST_ASSERT_EQUAL_MEMORY({a}, t.value, {len(data)}u);")
        fill.append("    TEST_ASSERT_EQUAL_INT(PROTO_OK, proto_tlv_put(&t, buf, cap, &pos));")
    fill.append(f"    m->{member}.data = buf;")
    fill.append(f"    m->{member}.len = (uint16_t)pos;")
    check.append(f'    TEST_ASSERT_EQUAL_INT_MESSAGE(0, proto_tlv_next(&cur, &t), "{where}: extra TLVs");')


def gen_messages(g: Gen, data: dict) -> list[str]:
    out: list[str] = []
    rows: list[str] = []
    for v in data["messages"]:
        name = v["name"]
        msg_type, schema = pc._MSG_BY_NAME[v["message"]]
        member = f"u.{v['message'].lower()}"
        fill: list[str] = []
        check: list[str] = []
        if schema:
            field_lines(g, schema, v["fields"], member, fill, check, name)
        out.append(f"static void fill_{name}(proto_msg_t *m, uint8_t *buf, size_t cap)\n{{")
        out.append("    (void)m;\n    (void)buf;\n    (void)cap;")
        out += fill
        out.append("}\n")
        out.append(f"static void check_{name}(const proto_msg_t *m)\n{{")
        out.append("    (void)m;")
        out += check
        out.append("}\n")
        rows.append(
            f'    {{"{name}", 0x{msg_type:02x}, {v["token"]}, '
            f'{g.arr(bytes.fromhex(v["payload_hex"]))}, {g.arr(bytes.fromhex(v["frame_hex"]))}, '
            f'{g.arr(bytes.fromhex(v["wire_hex"]))}, fill_{name}, check_{name}}},'
        )
    out.append("const vec_msg_t vec_messages[] = {")
    out += rows
    out.append("};")
    out.append("const size_t vec_messages_count = sizeof(vec_messages) / sizeof(vec_messages[0]);\n")
    return out


def gen_framing(g: Gen, data: dict) -> list[str]:
    out: list[str] = ["const vec_crc_t vec_crc[] = {"]
    for c in data["crc16"]:
        raw = c["input_ascii"].encode("ascii") if "input_ascii" in c else bytes.fromhex(c["input_hex"])
        out.append(f"    {{{g.arr(raw)}, {c['crc']}}},")
    out.append("};")
    out.append("const size_t vec_crc_count = sizeof(vec_crc) / sizeof(vec_crc[0]);\n")

    out.append("const vec_cobs_t vec_cobs[] = {")
    for c in data["cobs"]:
        out.append(f"    {{{g.arr(bytes.fromhex(c['decoded_hex']))}, {g.arr(bytes.fromhex(c['encoded_hex']))}}},")
    out.append("};")
    out.append("const size_t vec_cobs_count = sizeof(vec_cobs) / sizeof(vec_cobs[0]);\n")

    out.append("const vec_invalid_t vec_invalid[] = {")
    for c in data["invalid"]:
        expect = next((e for k, e in INVALID_EXPECT.items() if k in c["error"]), None)
        if expect is None:
            raise SystemExit(f"gen_vectors: no C expectation for invalid vector error {c['error']!r}")
        out.append(f'    {{"{c["name"]}", {g.arr(bytes.fromhex(c["wire_hex"]))}, {expect}}},')
    out.append("};")
    out.append("const size_t vec_invalid_count = sizeof(vec_invalid) / sizeof(vec_invalid[0]);\n")

    s = data["stream"]
    types = [pc._MSG_BY_NAME[name][0] for name in s["messages"]]
    tarr = g.raw(bytes(types))
    out.append(f"const vec_stream_t vec_stream = {{{g.arr(bytes.fromhex(s['wire_hex']))}, {tarr}, "
               f"{len(types)}u, {s['errors']}u}};\n")
    return out


def gen_gatt(g: Gen, data: dict) -> list[str]:
    out: list[str] = ["const vec_info_t vec_info[] = {"]
    for key in ("info", "info_pairing_open"):
        f = data[key]["fields"]
        out.append(f"    {{{g.arr(bytes.fromhex(data[key]['value_hex']))}, "
                   f"{{{f['proto_major']}, {f['proto_minor']}, {f['proto_patch']}, {f['flags']}, {f['psm']}}}}},")
    out.append("};")
    out.append("const size_t vec_info_count = sizeof(vec_info) / sizeof(vec_info[0]);\n")
    c = data["chunking"]
    chunks = ", ".join(g.arr(bytes.fromhex(h)) for h in c["chunks_hex"])
    out.append(f"static const vbytes_t chunk_list[] = {{{chunks}}};")
    out.append(f"const vec_chunking_t vec_chunking = {{{c['att_mtu']}, "
               f"{g.arr(bytes.fromhex(c['wire_hex']))}, chunk_list, {len(c['chunks_hex'])}u}};\n")
    return out


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out", required=True, type=Path)
    args = ap.parse_args(argv)

    g = Gen()
    body: list[str] = []
    body += gen_framing(g, json.loads((VECTORS / "framing.json").read_text(encoding="utf-8")))
    body += gen_gatt(g, json.loads((VECTORS / "gatt.json").read_text(encoding="utf-8")))
    msgs = json.loads((VECTORS / "messages.json").read_text(encoding="utf-8"))
    version = msgs["protocol_version"]
    body += gen_messages(g, msgs)

    head = [
        "/* Generated by firmware/test/gen_vectors.py from protocol/vectors/ (JSON). Do not edit. */",
        '#include <string.h>',
        '#include "unity.h"',
        '#include "vectors.h"',
        "",
        f'const char vec_protocol_version[] = "{version}";',
        "",
    ]
    text = "\n".join(head + g.decls + [""] + body) + "\n"
    args.out.parent.mkdir(parents=True, exist_ok=True)
    if not args.out.exists() or args.out.read_text(encoding="utf-8") != text:
        args.out.write_text(text, encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

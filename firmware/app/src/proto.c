/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Host-device protocol codec (protocol/SPEC.md 0.1.0). See proto.h.
 */
#include "proto.h"

#include <string.h>

/* --- CRC-16/IBM-3740 and COBS (SPEC §3) ------------------------------------ */

uint16_t proto_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

size_t proto_cobs_encode(const uint8_t *in, size_t len, uint8_t *out, size_t cap)
{
    size_t code_pos = 0; /* where the current block's code byte goes */
    size_t o = 1;        /* next output byte (position 0 is the first code byte) */
    uint8_t code = 1;    /* block length + 1 */
    bool full_block_at_end = false;

    for (size_t i = 0; i < len; i++) {
        full_block_at_end = false;
        if (in[i] != 0) {
            if (o >= cap) {
                return 0;
            }
            out[o++] = in[i];
            code++;
            if (code != 0xFF) {
                continue;
            }
            full_block_at_end = true; /* 254 data bytes: close the block */
        }
        if (code_pos >= cap) {
            return 0;
        }
        out[code_pos] = code;
        code_pos = o++;
        code = 1;
    }
    if (full_block_at_end) {
        /* A 0xFF block that ends the data needs no final code byte. */
        return code_pos;
    }
    if (code_pos >= cap) {
        return 0;
    }
    out[code_pos] = code;
    return o;
}

int proto_cobs_decode(const uint8_t *in, size_t len, uint8_t *out, size_t cap)
{
    size_t i = 0;
    size_t o = 0;
    while (i < len) {
        uint8_t code = in[i];
        if (code == 0) {
            return -1;
        }
        size_t end = i + code;
        if (end > len) {
            return -1;
        }
        for (size_t j = i + 1; j < end; j++) {
            if (in[j] == 0 || o >= cap) {
                return -1;
            }
            out[o++] = in[j];
        }
        i = end;
        if (code != 0xFF && i < len) {
            if (o >= cap) {
                return -1;
            }
            out[o++] = 0;
        }
    }
    return (int)o;
}

/* --- Field tables ------------------------------------------------------------ */

enum field_kind {
    K_U8,
    K_U16,
    K_U32,
    K_U64,
    K_I8,
    K_I16,
    K_CHAR,
    K_B6,     /* 6 bytes */
    K_B16,    /* 16 bytes */
    K_BYTES,  /* rest of the payload */
    K_TLVS,   /* rest of the payload, a TLV list */
    K_TRUST,  /* rest of the payload, 8-byte trust records */
    K_CONFIG, /* rest of the payload: key, selector, value */
};

typedef struct {
    uint8_t kind;
    uint16_t offset;
} field_t;

#define F(type, member, kind) {kind, (uint16_t)offsetof(type, member)}
#define N(arr) (sizeof(arr) / sizeof((arr)[0]))

static size_t kind_size(uint8_t kind)
{
    switch (kind) {
    case K_U8:
    case K_I8:
    case K_CHAR:
        return 1;
    case K_U16:
    case K_I16:
        return 2;
    case K_U32:
        return 4;
    case K_U64:
        return 8;
    case K_B6:
        return 6;
    case K_B16:
        return 16;
    default:
        return 0; /* variable */
    }
}

static bool is_array(uint8_t kind)
{
    return kind == K_B6 || kind == K_B16;
}

static void put_le(uint8_t *p, uint64_t v, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

static uint64_t get_le(const uint8_t *p, size_t n)
{
    uint64_t v = 0;
    for (size_t i = 0; i < n; i++) {
        v |= (uint64_t)p[i] << (8 * i);
    }
    return v;
}

static int encode_config(const proto_config_t *c, uint8_t *out, size_t cap, size_t *pos);
static int decode_config(const uint8_t *in, size_t len, proto_config_t *c);

/* Writes fields from `base`. Returns PROTO_OK, PROTO_ERR_OVERFLOW or
 * PROTO_ERR_BAD_VALUE. */
static int put_fields(const field_t *f, size_t n, const void *base, uint8_t *out, size_t cap,
                      size_t *pos)
{
    const uint8_t *b = (const uint8_t *)base;
    for (size_t i = 0; i < n; i++) {
        const uint8_t *src = b + f[i].offset;
        size_t sz = kind_size(f[i].kind);
        if (is_array(f[i].kind)) {
            if (*pos + sz > cap) {
                return PROTO_ERR_OVERFLOW;
            }
            memcpy(out + *pos, src, sz);
            *pos += sz;
        } else if (sz) {
            uint64_t v;
            switch (f[i].kind) {
            case K_U8: v = *(const uint8_t *)src; break;
            case K_I8: v = (uint8_t) * (const int8_t *)src; break;
            case K_CHAR: v = (uint8_t) * (const char *)src; break;
            case K_U16: v = *(const uint16_t *)(const void *)src; break;
            case K_I16: v = (uint16_t) * (const int16_t *)(const void *)src; break;
            case K_U32: v = *(const uint32_t *)(const void *)src; break;
            default: v = *(const uint64_t *)(const void *)src; break;
            }
            if (*pos + sz > cap) {
                return PROTO_ERR_OVERFLOW;
            }
            put_le(out + *pos, v, sz);
            *pos += sz;
        } else if (f[i].kind == K_BYTES || f[i].kind == K_TLVS || f[i].kind == K_TRUST) {
            const proto_bytes_t *bytes = (const proto_bytes_t *)(const void *)src;
            if (*pos + bytes->len > cap) {
                return PROTO_ERR_OVERFLOW;
            }
            if (bytes->len) {
                memcpy(out + *pos, bytes->data, bytes->len);
            }
            *pos += bytes->len;
        } else { /* K_CONFIG */
            int rc = encode_config((const proto_config_t *)(const void *)src, out, cap, pos);
            if (rc != PROTO_OK) {
                return rc;
            }
        }
    }
    return PROTO_OK;
}

static bool tlvs_valid(const uint8_t *data, size_t len);

/* Reads fields into `base`. `exact`: trailing bytes are an error. Returns
 * PROTO_OK, PROTO_ERR_BAD_LENGTH or PROTO_ERR_BAD_VALUE. */
static int get_fields(const field_t *f, size_t n, void *base, const uint8_t *in, size_t len,
                      bool exact)
{
    uint8_t *b = (uint8_t *)base;
    size_t pos = 0;
    for (size_t i = 0; i < n; i++) {
        uint8_t *dst = b + f[i].offset;
        size_t sz = kind_size(f[i].kind);
        if (is_array(f[i].kind)) {
            if (pos + sz > len) {
                return PROTO_ERR_BAD_LENGTH;
            }
            memcpy(dst, in + pos, sz);
            pos += sz;
        } else if (sz) {
            if (pos + sz > len) {
                return PROTO_ERR_BAD_LENGTH;
            }
            uint64_t v = get_le(in + pos, sz);
            pos += sz;
            switch (f[i].kind) {
            case K_U8: *(uint8_t *)dst = (uint8_t)v; break;
            case K_I8: *(int8_t *)dst = (int8_t)(uint8_t)v; break;
            case K_CHAR: *(char *)dst = (char)(uint8_t)v; break;
            case K_U16: *(uint16_t *)(void *)dst = (uint16_t)v; break;
            case K_I16: *(int16_t *)(void *)dst = (int16_t)(uint16_t)v; break;
            case K_U32: *(uint32_t *)(void *)dst = (uint32_t)v; break;
            default: *(uint64_t *)(void *)dst = v; break;
            }
        } else if (f[i].kind == K_BYTES || f[i].kind == K_TLVS || f[i].kind == K_TRUST) {
            proto_bytes_t *bytes = (proto_bytes_t *)(void *)dst;
            bytes->data = in + pos;
            bytes->len = (uint16_t)(len - pos);
            if (f[i].kind == K_TLVS && !tlvs_valid(bytes->data, bytes->len)) {
                return PROTO_ERR_BAD_VALUE;
            }
            if (f[i].kind == K_TRUST && bytes->len % PROTO_TRUST_RECORD_LEN != 0) {
                return PROTO_ERR_BAD_LENGTH; /* truncated record */
            }
            pos = len;
        } else { /* K_CONFIG */
            int rc = decode_config(in + pos, len - pos, (proto_config_t *)(void *)dst);
            if (rc != PROTO_OK) {
                return rc;
            }
            pos = len;
        }
    }
    if (exact && pos != len) {
        return PROTO_ERR_BAD_LENGTH;
    }
    return PROTO_OK;
}

/* --- Config keys (SPEC §6.1) --- */

#define CV(member, kind) F(proto_config_t, value.member, kind)

static const field_t cfg_serial_jack_mode[] = {CV(serial_jack_mode.mode, K_U8)};
static const field_t cfg_ptt_targets[] = {CV(ptt_targets.targets, K_U8),
                                          CV(ptt_targets.usb_port, K_U8)};
static const field_t cfg_line_map[] = {CV(line_map.rts_action, K_U8),
                                       CV(line_map.dtr_action, K_U8)};
static const field_t cfg_keepalive[] = {CV(ptt_keepalive_ms.ms, K_U16)};
static const field_t cfg_max_tx[] = {CV(max_tx_s.s, K_U32)};
static const field_t cfg_audio_path[] = {CV(audio_path.path, K_U8)};
static const field_t cfg_tx_level[] = {CV(tx_level.centibel, K_I16)};
static const field_t cfg_rx_att[] = {CV(rx_attenuator.on, K_U8)};
static const field_t cfg_rx_gain[] = {CV(rx_gain.centibel, K_I16)};
static const field_t cfg_host_mode[] = {CV(host_mode.mode, K_U8)};
static const field_t cfg_ble_tx_power[] = {CV(ble_tx_power.dbm, K_I8)};
static const field_t cfg_wired_profile[] = {CV(wired_profile.profile, K_U8)};
static const field_t cfg_serial_default[] = {
    CV(serial_default.baud, K_U32), CV(serial_default.data_bits, K_U8),
    CV(serial_default.parity, K_U8), CV(serial_default.stop_bits, K_U8)};
static const field_t cfg_subnet[] = {CV(usb_net_subnet.a, K_U8), CV(usb_net_subnet.b, K_U8),
                                     CV(usb_net_subnet.c, K_U8), CV(usb_net_subnet.d, K_U8)};
static const field_t cfg_pairing_window[] = {CV(pairing_window_s.s, K_U16)};
static const field_t cfg_power_down[] = {CV(power_down_delay_s.s, K_U16)};
static const field_t cfg_wired_port_lock[] = {CV(wired_port_lock.level, K_U8)};

typedef struct {
    const field_t *fields;
    uint8_t n;
} schema_t;

static const schema_t config_schemas[PROTO_KEY_LAST + 1] = {
    [PROTO_KEY_SERIAL_JACK_MODE] = {cfg_serial_jack_mode, N(cfg_serial_jack_mode)},
    [PROTO_KEY_PTT_TARGETS] = {cfg_ptt_targets, N(cfg_ptt_targets)},
    [PROTO_KEY_LINE_MAP] = {cfg_line_map, N(cfg_line_map)},
    [PROTO_KEY_PTT_KEEPALIVE_MS] = {cfg_keepalive, N(cfg_keepalive)},
    [PROTO_KEY_MAX_TX_S] = {cfg_max_tx, N(cfg_max_tx)},
    [PROTO_KEY_AUDIO_PATH] = {cfg_audio_path, N(cfg_audio_path)},
    [PROTO_KEY_TX_LEVEL] = {cfg_tx_level, N(cfg_tx_level)},
    [PROTO_KEY_RX_ATTENUATOR] = {cfg_rx_att, N(cfg_rx_att)},
    [PROTO_KEY_RX_GAIN] = {cfg_rx_gain, N(cfg_rx_gain)},
    [PROTO_KEY_HOST_MODE] = {cfg_host_mode, N(cfg_host_mode)},
    [PROTO_KEY_BLE_TX_POWER] = {cfg_ble_tx_power, N(cfg_ble_tx_power)},
    [PROTO_KEY_WIRED_PROFILE] = {cfg_wired_profile, N(cfg_wired_profile)},
    [PROTO_KEY_SERIAL_DEFAULT] = {cfg_serial_default, N(cfg_serial_default)},
    [PROTO_KEY_USB_NET_SUBNET] = {cfg_subnet, N(cfg_subnet)},
    [PROTO_KEY_PAIRING_WINDOW_S] = {cfg_pairing_window, N(cfg_pairing_window)},
    [PROTO_KEY_POWER_DOWN_DELAY_S] = {cfg_power_down, N(cfg_power_down)},
    [PROTO_KEY_WIRED_PORT_LOCK] = {cfg_wired_port_lock, N(cfg_wired_port_lock)},
};

static const schema_t *config_schema(uint8_t key)
{
    if (key < PROTO_KEY_FIRST || key > PROTO_KEY_LAST) {
        return NULL;
    }
    return &config_schemas[key];
}

int proto_config_value_len(uint8_t key)
{
    const schema_t *s = config_schema(key);
    if (!s) {
        return -1;
    }
    size_t n = 0;
    for (size_t i = 0; i < s->n; i++) {
        n += kind_size(s->fields[i].kind);
    }
    return (int)n;
}

static int encode_config(const proto_config_t *c, uint8_t *out, size_t cap, size_t *pos)
{
    const schema_t *s = config_schema(c->key);
    if (!s) {
        return PROTO_ERR_BAD_VALUE;
    }
    if (*pos + 2 > cap) {
        return PROTO_ERR_OVERFLOW;
    }
    out[(*pos)++] = c->key;
    out[(*pos)++] = c->selector;
    return put_fields(s->fields, s->n, c, out, cap, pos);
}

static int decode_config(const uint8_t *in, size_t len, proto_config_t *c)
{
    if (len < 2) {
        return PROTO_ERR_BAD_LENGTH;
    }
    memset(c, 0, sizeof(*c));
    c->key = in[0];
    c->selector = in[1];
    const schema_t *s = config_schema(c->key);
    if (!s) {
        return PROTO_ERR_BAD_VALUE; /* unknown key */
    }
    return get_fields(s->fields, s->n, c, in + 2, len - 2, true);
}

int proto_config_encode(const proto_config_t *c, uint8_t *out, size_t cap, size_t *pos)
{
    return encode_config(c, out, cap, pos);
}

int proto_config_decode(const uint8_t *in, size_t len, proto_config_t *c)
{
    return decode_config(in, len, c);
}

/* --- CAPS TLVs (SPEC §5.1) --- */

#define TV(member, kind) F(proto_tlv_t, v.member, kind)

static const field_t tlv_features[] = {TV(features.features, K_U32)};
static const field_t tlv_serial_jack[] = {
    TV(serial_jack.port, K_U8), TV(serial_jack.modes, K_U8), TV(serial_jack.min_baud, K_U32),
    TV(serial_jack.max_baud, K_U32), TV(serial_jack.tx_buffer, K_U16)};
static const field_t tlv_radio_usb_serial[] = {
    TV(radio_usb_serial.port, K_U8),      TV(radio_usb_serial.chip, K_U8),
    TV(radio_usb_serial.vid, K_U16),      TV(radio_usb_serial.pid, K_U16),
    TV(radio_usb_serial.interface, K_U8), TV(radio_usb_serial.tx_buffer, K_U16)};
static const field_t tlv_audio[] = {
    TV(audio.directions, K_U8),         TV(audio.codecs, K_U8),
    TV(audio.rates, K_U8),              TV(audio.paths, K_U8),
    TV(audio.max_frame_samples, K_U16), TV(audio.tx_buffer_samples, K_U16)};
static const field_t tlv_ptt[] = {TV(ptt.outputs, K_U8), TV(ptt.keepalive_min_ms, K_U16),
                                  TV(ptt.keepalive_max_ms, K_U16), TV(ptt.max_tx_min_s, K_U32)};
static const field_t tlv_tone[] = {TV(tone.max_symbols, K_U16), TV(tone.max_tone_index, K_U8),
                                   TV(tone.shaping, K_U8), TV(tone.min_symbol_us, K_U32),
                                   TV(tone.max_symbol_us, K_U32)};
static const field_t tlv_ble_tx_power[] = {TV(ble_tx_power.min_dbm, K_I8),
                                           TV(ble_tx_power.max_dbm, K_I8)};
static const field_t tlv_pairing[] = {TV(pairing.triggers, K_U8), TV(pairing.window_min_s, K_U16),
                                      TV(pairing.window_max_s, K_U16),
                                      TV(pairing.max_bonds, K_U8),
                                      TV(pairing.max_wired_hosts, K_U8)};

static const schema_t tlv_schemas[] = {
    [PROTO_TLV_FEATURES] = {tlv_features, N(tlv_features)},
    [PROTO_TLV_SERIAL_JACK] = {tlv_serial_jack, N(tlv_serial_jack)},
    [PROTO_TLV_RADIO_USB_SERIAL] = {tlv_radio_usb_serial, N(tlv_radio_usb_serial)},
    [PROTO_TLV_AUDIO] = {tlv_audio, N(tlv_audio)},
    [PROTO_TLV_PTT] = {tlv_ptt, N(tlv_ptt)},
    [PROTO_TLV_TONE] = {tlv_tone, N(tlv_tone)},
    [PROTO_TLV_BLE_TX_POWER] = {tlv_ble_tx_power, N(tlv_ble_tx_power)},
    [PROTO_TLV_PAIRING] = {tlv_pairing, N(tlv_pairing)},
};

static const schema_t *tlv_schema(uint8_t tag)
{
    if (tag == 0 || tag >= N(tlv_schemas)) {
        return NULL;
    }
    return &tlv_schemas[tag];
}

static size_t schema_size(const schema_t *s)
{
    size_t n = 0;
    for (size_t i = 0; i < s->n; i++) {
        n += kind_size(s->fields[i].kind);
    }
    return n;
}

int proto_tlv_put(const proto_tlv_t *tlv, uint8_t *out, size_t cap, size_t *pos)
{
    const schema_t *s = tlv_schema(tlv->tag);
    size_t start = *pos;
    if (start + 2 > cap) {
        return PROTO_ERR_OVERFLOW;
    }
    size_t p = start + 2;
    if (s && tlv->known) {
        int rc = put_fields(s->fields, s->n, tlv, out, cap, &p);
        if (rc != PROTO_OK) {
            return rc;
        }
    } else {
        if (p + tlv->len > cap) {
            return PROTO_ERR_OVERFLOW;
        }
        if (tlv->len) {
            memcpy(out + p, tlv->value, tlv->len);
        }
        p += tlv->len;
    }
    /* A value is at most 255 bytes: raw values have a u8 length and the
     * known TLVs are all shorter. */
    out[start] = tlv->tag;
    out[start + 1] = (uint8_t)(p - start - 2);
    *pos = p;
    return PROTO_OK;
}

int proto_tlv_next(proto_bytes_t *cursor, proto_tlv_t *out)
{
    if (cursor->len == 0) {
        return 0;
    }
    if (cursor->len < 2) {
        return -1;
    }
    uint8_t tag = cursor->data[0];
    uint8_t len = cursor->data[1];
    if ((size_t)len + 2 > cursor->len) {
        return -1;
    }
    memset(out, 0, sizeof(*out));
    out->tag = tag;
    out->len = len;
    out->value = cursor->data + 2;
    const schema_t *s = tlv_schema(tag);
    if (s) {
        size_t known = schema_size(s);
        if (len < known) {
            return -1;
        }
        /* Decode the known prefix; a newer minor version may append fields.
         * Fixed-size fields only, and `known` bytes are there: can't fail. */
        (void)get_fields(s->fields, s->n, out, out->value, known, true);
        out->known = true;
    }
    cursor->data += 2u + len;
    cursor->len = (uint16_t)(cursor->len - 2u - len);
    return 1;
}

static bool tlvs_valid(const uint8_t *data, size_t len)
{
    proto_bytes_t c = {data, (uint16_t)len};
    proto_tlv_t t;
    int rc;
    while ((rc = proto_tlv_next(&c, &t)) == 1) {
    }
    return rc == 0;
}

/* --- Messages (SPEC §4 to §11, §16) --- */

#define M(member, kind) F(proto_msg_t, u.member, kind)

static const field_t m_hello[] = {M(hello.proto_major, K_U8), M(hello.proto_minor, K_U8),
                                  M(hello.proto_patch, K_U8), M(hello.max_payload, K_U16),
                                  M(hello.flags, K_U8)};
static const field_t m_device_info[] = {
    M(device_info.proto_major, K_U8), M(device_info.proto_minor, K_U8),
    M(device_info.proto_patch, K_U8), M(device_info.fw_major, K_U8),
    M(device_info.fw_minor, K_U8),    M(device_info.fw_patch, K_U8),
    M(device_info.variant, K_CHAR),   M(device_info.hw_revision, K_CHAR),
    M(device_info.max_payload, K_U16), M(device_info.build, K_BYTES)};
static const field_t m_caps[] = {M(caps.tlvs, K_TLVS)};
static const field_t m_result[] = {M(result.request_type, K_U8), M(result.code, K_U8)};
static const field_t m_ping[] = {M(ping.data, K_BYTES)};
static const field_t m_pong[] = {M(pong.data, K_BYTES)};
static const field_t m_status[] = {
    M(status.host_link, K_U8),         M(status.transport, K_U8),
    M(status.phy, K_U8),               M(status.conn_interval_us, K_U32),
    M(status.att_mtu, K_U16),          M(status.coc_mtu, K_U16),
    M(status.flags, K_U16),            M(status.audio_path, K_U8),
    M(status.supply_mv, K_U16),        M(status.frame_errors, K_U16),
    M(status.cat_overflows, K_U16)};
static const field_t m_config_get[] = {M(config_get.key, K_U8), M(config_get.selector, K_U8)};
static const field_t m_config_set[] = {M(config_set.flags, K_U8),
                                       M(config_set.config, K_CONFIG)};
static const field_t m_config[] = {M(config.config, K_CONFIG)};
static const field_t m_config_reset[] = {M(config_reset.flags, K_U8)};
static const field_t m_serial_open[] = {M(serial_open.port, K_U8), M(serial_open.open, K_U8)};
static const field_t m_serial_set[] = {M(serial_set.port, K_U8), M(serial_set.baud, K_U32),
                                       M(serial_set.data_bits, K_U8),
                                       M(serial_set.parity, K_U8),
                                       M(serial_set.stop_bits, K_U8)};
static const field_t m_cat_data[] = {M(cat_data.port, K_U8), M(cat_data.data, K_BYTES)};
static const field_t m_cat_credit[] = {M(cat_credit.port, K_U8), M(cat_credit.credit, K_U16)};
static const field_t m_modem_lines[] = {M(modem_lines.port, K_U8), M(modem_lines.lines, K_U8)};
static const field_t m_modem_status[] = {M(modem_status.port, K_U8),
                                         M(modem_status.lines, K_U8)};
static const field_t m_ptt_set[] = {M(ptt_set.state, K_U8)};
static const field_t m_ptt_status[] = {M(ptt_status.state, K_U8), M(ptt_status.sources, K_U8),
                                       M(ptt_status.reason, K_U8),
                                       M(ptt_status.remaining_s, K_U32)};
static const field_t m_audio_start[] = {
    M(audio_start.direction, K_U8),      M(audio_start.codec, K_U8),
    M(audio_start.sample_rate, K_U16),   M(audio_start.frame_samples, K_U16),
    M(audio_start.codec_param, K_U16),   M(audio_start.start_time_us, K_U64)};
static const field_t m_audio_stop[] = {M(audio_stop.direction, K_U8)};
static const field_t m_audio_frame[] = {M(audio_frame.seq, K_U16),
                                        M(audio_frame.timestamp, K_U32),
                                        M(audio_frame.flags, K_U8),
                                        M(audio_frame.data, K_BYTES)};
static const field_t m_audio_status[] = {
    M(audio_status.direction, K_U8),       M(audio_status.state, K_U8),
    M(audio_status.fill_samples, K_U16),   M(audio_status.target_samples, K_U16),
    M(audio_status.underruns, K_U16),      M(audio_status.overruns, K_U16),
    M(audio_status.first_sample_time_us, K_U64)};
static const field_t m_time_req[] = {M(time_req.host_t1, K_U64)};
static const field_t m_time_resp[] = {M(time_resp.host_t1, K_U64),
                                      M(time_resp.device_t2, K_U64),
                                      M(time_resp.device_t3, K_U64)};
static const field_t m_time_set[] = {M(time_set.device_time_us, K_U64),
                                     M(time_set.utc_us, K_U64),
                                     M(time_set.uncertainty_us, K_U32)};
static const field_t m_tone_setup[] = {
    M(tone_setup.count, K_U16),       M(tone_setup.base_mhz, K_U32),
    M(tone_setup.spacing_mhz, K_U32), M(tone_setup.symbol_us, K_U32),
    M(tone_setup.shaping, K_U8),      M(tone_setup.bt_x100, K_U8),
    M(tone_setup.amplitude, K_U16),   M(tone_setup.ramp_us, K_U16),
    M(tone_setup.ptt_lead_ms, K_U16), M(tone_setup.ptt_tail_ms, K_U16),
    M(tone_setup.flags, K_U8)};
static const field_t m_tone_data[] = {M(tone_data.offset, K_U16), M(tone_data.tones, K_BYTES)};
static const field_t m_tone_start[] = {M(tone_start.start_utc_us, K_U64)};
static const field_t m_tone_status[] = {M(tone_status.state, K_U8),
                                        M(tone_status.symbol, K_U16),
                                        M(tone_status.reason, K_U8)};
static const field_t m_auth[] = {M(auth.host_token, K_B16)};
static const field_t m_auth_status[] = {M(auth_status.state, K_U8), M(auth_status.slot, K_U8)};
static const field_t m_trust_list[] = {M(trust_list.records, K_TRUST)};
static const field_t m_trust_remove[] = {M(trust_remove.kind, K_U8), M(trust_remove.slot, K_U8)};
static const field_t m_factory_reset[] = {M(factory_reset.confirm, K_U32)};

typedef struct {
    uint8_t type;
    uint8_t dir;
    const field_t *fields;
    uint8_t n;
} msg_def_t;

static const msg_def_t messages[] = {
    {PROTO_HELLO, PROTO_H2D, m_hello, N(m_hello)},
    {PROTO_DEVICE_INFO, PROTO_D2H, m_device_info, N(m_device_info)},
    {PROTO_CAPS_GET, PROTO_H2D, NULL, 0},
    {PROTO_CAPS, PROTO_D2H, m_caps, N(m_caps)},
    {PROTO_RESULT, PROTO_D2H, m_result, N(m_result)},
    {PROTO_PING, PROTO_H2D, m_ping, N(m_ping)},
    {PROTO_PONG, PROTO_D2H, m_pong, N(m_pong)},
    {PROTO_STATUS_GET, PROTO_H2D, NULL, 0},
    {PROTO_STATUS, PROTO_D2H, m_status, N(m_status)},
    {PROTO_CONFIG_GET, PROTO_H2D, m_config_get, N(m_config_get)},
    {PROTO_CONFIG_SET, PROTO_H2D, m_config_set, N(m_config_set)},
    {PROTO_CONFIG, PROTO_D2H, m_config, N(m_config)},
    {PROTO_CONFIG_RESET, PROTO_H2D, m_config_reset, N(m_config_reset)},
    {PROTO_SERIAL_OPEN, PROTO_H2D, m_serial_open, N(m_serial_open)},
    {PROTO_SERIAL_SET, PROTO_H2D, m_serial_set, N(m_serial_set)},
    {PROTO_CAT_DATA, PROTO_BOTH, m_cat_data, N(m_cat_data)},
    {PROTO_CAT_CREDIT, PROTO_D2H, m_cat_credit, N(m_cat_credit)},
    {PROTO_MODEM_LINES, PROTO_H2D, m_modem_lines, N(m_modem_lines)},
    {PROTO_MODEM_STATUS, PROTO_D2H, m_modem_status, N(m_modem_status)},
    {PROTO_PTT_SET, PROTO_H2D, m_ptt_set, N(m_ptt_set)},
    {PROTO_PTT_STATUS, PROTO_D2H, m_ptt_status, N(m_ptt_status)},
    {PROTO_KEEPALIVE, PROTO_H2D, NULL, 0},
    {PROTO_AUDIO_START, PROTO_H2D, m_audio_start, N(m_audio_start)},
    {PROTO_AUDIO_STOP, PROTO_H2D, m_audio_stop, N(m_audio_stop)},
    {PROTO_AUDIO_FRAME, PROTO_BOTH, m_audio_frame, N(m_audio_frame)},
    {PROTO_AUDIO_STATUS, PROTO_D2H, m_audio_status, N(m_audio_status)},
    {PROTO_TIME_REQ, PROTO_H2D, m_time_req, N(m_time_req)},
    {PROTO_TIME_RESP, PROTO_D2H, m_time_resp, N(m_time_resp)},
    {PROTO_TIME_SET, PROTO_H2D, m_time_set, N(m_time_set)},
    {PROTO_TONE_SETUP, PROTO_H2D, m_tone_setup, N(m_tone_setup)},
    {PROTO_TONE_DATA, PROTO_H2D, m_tone_data, N(m_tone_data)},
    {PROTO_TONE_START, PROTO_H2D, m_tone_start, N(m_tone_start)},
    {PROTO_TONE_CANCEL, PROTO_H2D, NULL, 0},
    {PROTO_TONE_STATUS, PROTO_D2H, m_tone_status, N(m_tone_status)},
    {PROTO_AUTH, PROTO_H2D, m_auth, N(m_auth)},
    {PROTO_AUTH_STATUS, PROTO_D2H, m_auth_status, N(m_auth_status)},
    {PROTO_TRUST_LIST_GET, PROTO_H2D, NULL, 0},
    {PROTO_TRUST_LIST, PROTO_D2H, m_trust_list, N(m_trust_list)},
    {PROTO_TRUST_REMOVE, PROTO_H2D, m_trust_remove, N(m_trust_remove)},
    {PROTO_FACTORY_RESET, PROTO_H2D, m_factory_reset, N(m_factory_reset)},
};

static const msg_def_t *msg_def(uint8_t type)
{
    for (size_t i = 0; i < N(messages); i++) {
        if (messages[i].type == type) {
            return &messages[i];
        }
    }
    return NULL;
}

bool proto_type_known(uint8_t type)
{
    return msg_def(type) != NULL;
}

int proto_type_dir(uint8_t type)
{
    const msg_def_t *d = msg_def(type);
    return d ? d->dir : 0;
}

int proto_decode_payload(uint8_t type, uint8_t token, const uint8_t *payload, size_t len,
                         proto_msg_t *out)
{
    const msg_def_t *d = msg_def(type);
    memset(out, 0, sizeof(*out));
    out->type = type;
    out->token = token;
    if (!d) {
        return PROTO_ERR_UNKNOWN_TYPE;
    }
    if (len > PROTO_MAX_PAYLOAD) {
        return PROTO_ERR_BAD_LENGTH;
    }
    return get_fields(d->fields, d->n, out, payload, len, true);
}

int proto_encode_payload(const proto_msg_t *msg, uint8_t *out, size_t cap, size_t *len)
{
    const msg_def_t *d = msg_def(msg->type);
    if (!d) {
        return PROTO_ERR_UNKNOWN_TYPE;
    }
    if (cap > PROTO_MAX_PAYLOAD) {
        cap = PROTO_MAX_PAYLOAD;
    }
    *len = 0;
    return put_fields(d->fields, d->n, msg, out, cap, len);
}

size_t proto_encode(const proto_msg_t *msg, uint8_t *wire, size_t cap)
{
    uint8_t frame[PROTO_MAX_FRAME];
    size_t plen = 0;
    if (proto_encode_payload(msg, frame + 2, PROTO_MAX_PAYLOAD, &plen) != PROTO_OK) {
        return 0;
    }
    frame[0] = msg->type;
    frame[1] = msg->token;
    uint16_t crc = proto_crc16(frame, plen + 2);
    frame[plen + 2] = (uint8_t)(crc & 0xFFu);
    frame[plen + 3] = (uint8_t)(crc >> 8);
    if (cap < 1) {
        return 0;
    }
    size_t n = proto_cobs_encode(frame, plen + 4, wire, cap - 1);
    if (n == 0) {
        return 0;
    }
    wire[n] = 0x00;
    return n + 1;
}

/* --- TRUST_LIST records (SPEC §15.3) --- */

int proto_trust_put(const proto_trust_record_t *r, uint8_t *out, size_t cap, size_t *pos)
{
    if (*pos + PROTO_TRUST_RECORD_LEN > cap) {
        return PROTO_ERR_OVERFLOW;
    }
    out[*pos] = r->kind;
    out[*pos + 1] = r->slot;
    memcpy(out + *pos + 2, r->ident, PROTO_TRUST_IDENT_LEN);
    *pos += PROTO_TRUST_RECORD_LEN;
    return PROTO_OK;
}

int proto_trust_next(proto_bytes_t *cursor, proto_trust_record_t *out)
{
    if (cursor->len < PROTO_TRUST_RECORD_LEN) {
        return 0;
    }
    out->kind = cursor->data[0];
    out->slot = cursor->data[1];
    memcpy(out->ident, cursor->data + 2, PROTO_TRUST_IDENT_LEN);
    cursor->data += PROTO_TRUST_RECORD_LEN;
    cursor->len = (uint16_t)(cursor->len - PROTO_TRUST_RECORD_LEN);
    return 1;
}

/* --- Info characteristic --- */

void proto_info_encode(const proto_info_t *info, uint8_t out[PROTO_INFO_LEN])
{
    out[0] = info->proto_major;
    out[1] = info->proto_minor;
    out[2] = info->proto_patch;
    out[3] = info->flags;
    out[4] = (uint8_t)(info->psm & 0xFFu);
    out[5] = (uint8_t)(info->psm >> 8);
}

int proto_info_decode(const uint8_t *data, size_t len, proto_info_t *out)
{
    if (len != PROTO_INFO_LEN) {
        return PROTO_ERR_BAD_LENGTH;
    }
    out->proto_major = data[0];
    out->proto_minor = data[1];
    out->proto_patch = data[2];
    out->flags = data[3];
    out->psm = (uint16_t)(data[4] | (data[5] << 8));
    return PROTO_OK;
}

/* --- Frames and the stream receiver (SPEC §3.4) --- */

int proto_frame_check(const uint8_t *frame, size_t len, uint8_t *type, uint8_t *token,
                      const uint8_t **payload, size_t *payload_len)
{
    if (len < PROTO_FRAME_OVERHEAD || len > PROTO_MAX_FRAME) {
        return PROTO_ERR_FRAME;
    }
    uint16_t crc = (uint16_t)(frame[len - 2] | (frame[len - 1] << 8));
    if (proto_crc16(frame, len - 2) != crc) {
        return PROTO_ERR_FRAME;
    }
    *type = frame[0];
    *token = frame[1];
    *payload = frame + 2;
    *payload_len = len - 4;
    return PROTO_OK;
}

void proto_rx_init(proto_rx_t *rx, proto_frame_cb on_frame, proto_frame_error_cb on_error,
                   void *ctx)
{
    memset(rx, 0, sizeof(*rx));
    rx->max_frame = PROTO_MAX_FRAME;
    rx->on_frame = on_frame;
    rx->on_error = on_error;
    rx->ctx = ctx;
}

void proto_rx_reset(proto_rx_t *rx)
{
    rx->len = 0;
    rx->discarding = false;
}

static void rx_error(proto_rx_t *rx)
{
    rx->errors++;
    if (rx->on_error) {
        rx->on_error(rx->ctx);
    }
}

static void rx_frame_done(proto_rx_t *rx)
{
    if (rx->discarding) {
        rx->discarding = false;
        rx->len = 0;
        rx_error(rx);
        return;
    }
    if (rx->len == 0) {
        return; /* empty frame: padding or a deliberate flush */
    }
    int n = proto_cobs_decode(rx->buf, rx->len, rx->frame, sizeof(rx->frame));
    rx->len = 0;
    if (n < 0 || (size_t)n > rx->max_frame) {
        rx_error(rx);
        return;
    }
    uint8_t type, token;
    const uint8_t *payload;
    size_t plen;
    if (proto_frame_check(rx->frame, (size_t)n, &type, &token, &payload, &plen) != PROTO_OK) {
        rx_error(rx);
        return;
    }
    if (rx->on_frame) {
        rx->on_frame(rx->ctx, type, token, payload, plen);
    }
}

void proto_rx_feed(proto_rx_t *rx, const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        if (b == 0x00) {
            rx_frame_done(rx);
            continue;
        }
        if (rx->discarding) {
            continue;
        }
        if (rx->len >= sizeof(rx->buf)) {
            /* Longer than any valid frame: discard up to the next delimiter. */
            rx->discarding = true;
            rx->len = 0;
            continue;
        }
        rx->buf[rx->len++] = b;
    }
}

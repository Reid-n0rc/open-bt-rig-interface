/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Host-device protocol codec (protocol/SPEC.md 0.1.0).
 *
 *   message struct  --proto_encode-->  wire bytes = COBS(type|token|payload|CRC) 0x00
 *   wire bytes      --proto_rx_feed--> frames (type, token, payload) --proto_decode--> struct
 *
 * Table-driven: each message, config key and CAPS TLV has a field list that
 * mirrors the reference codec (tools/protocol/proto_codec.py). The host tests
 * check every golden vector in protocol/vectors/ against this code.
 *
 * Portable C99, no allocation. Decoded byte fields (proto_bytes_t) point into
 * the buffer that was decoded; they are valid only while that buffer is.
 */
#ifndef PROTO_H
#define PROTO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPEC §4.1 */
#define PROTO_VERSION_MAJOR 0
#define PROTO_VERSION_MINOR 1
#define PROTO_VERSION_PATCH 0

/* SPEC §12.2: every implementation must be able to parse 1024. */
#define PROTO_MAX_PAYLOAD 1024u
#define PROTO_MIN_PAYLOAD_LIMIT 256u
#define PROTO_FRAME_OVERHEAD 4u /* type, token, CRC-16 */
#define PROTO_MAX_FRAME (PROTO_MAX_PAYLOAD + PROTO_FRAME_OVERHEAD)
/* COBS adds one byte per 254 (rounded up) plus one; then the 0x00 delimiter. */
#define PROTO_COBS_MAX(n) ((n) + ((n) / 254u) + 1u)
#define PROTO_MAX_WIRE (PROTO_COBS_MAX(PROTO_MAX_FRAME) + 1u)

#define PROTO_PING_MAX 64u  /* SPEC §4.3 */
#define PROTO_BUILD_MAX 32u /* SPEC §4.2, DEVICE_INFO.build */

/* Message types (SPEC §16). */
enum proto_type {
    PROTO_HELLO = 0x01,
    PROTO_DEVICE_INFO = 0x02,
    PROTO_CAPS_GET = 0x03,
    PROTO_CAPS = 0x04,
    PROTO_RESULT = 0x05,
    PROTO_PING = 0x06,
    PROTO_PONG = 0x07,
    PROTO_STATUS_GET = 0x08,
    PROTO_STATUS = 0x09,
    PROTO_CONFIG_GET = 0x10,
    PROTO_CONFIG_SET = 0x11,
    PROTO_CONFIG = 0x12,
    PROTO_CONFIG_RESET = 0x13,
    PROTO_SERIAL_OPEN = 0x20,
    PROTO_SERIAL_SET = 0x21,
    PROTO_CAT_DATA = 0x22,
    PROTO_CAT_CREDIT = 0x23,
    PROTO_MODEM_LINES = 0x24,
    PROTO_MODEM_STATUS = 0x25,
    PROTO_PTT_SET = 0x30,
    PROTO_PTT_STATUS = 0x31,
    PROTO_KEEPALIVE = 0x32,
    PROTO_AUDIO_START = 0x40,
    PROTO_AUDIO_STOP = 0x41,
    PROTO_AUDIO_FRAME = 0x42,
    PROTO_AUDIO_STATUS = 0x43,
    PROTO_TIME_REQ = 0x50,
    PROTO_TIME_RESP = 0x51,
    PROTO_TIME_SET = 0x52,
    PROTO_TONE_SETUP = 0x60,
    PROTO_TONE_DATA = 0x61,
    PROTO_TONE_START = 0x62,
    PROTO_TONE_CANCEL = 0x63,
    PROTO_TONE_STATUS = 0x64,
    PROTO_AUTH = 0x70,
    PROTO_AUTH_STATUS = 0x71,
    PROTO_TRUST_LIST_GET = 0x72,
    PROTO_TRUST_LIST = 0x73,
    PROTO_TRUST_REMOVE = 0x74,
    PROTO_FACTORY_RESET = 0x75,
};

/* RESULT codes (SPEC §12.1). */
enum proto_code {
    PROTO_OK = 0,
    PROTO_ERR_UNKNOWN_TYPE = 1,
    PROTO_ERR_BAD_LENGTH = 2,
    PROTO_ERR_BAD_VALUE = 3,
    PROTO_ERR_UNSUPPORTED = 4,
    PROTO_ERR_BUSY = 5,
    PROTO_ERR_STATE = 6,
    PROTO_ERR_OVERFLOW = 7,
    PROTO_ERR_VERSION_MISMATCH = 8,
    PROTO_ERR_OUT_OF_RANGE = 9,
    PROTO_ERR_FRAME = 10,
    PROTO_ERR_RATE_LIMITED = 11,
    PROTO_ERR_INTERNAL = 12,
    PROTO_ERR_NOT_AUTHORIZED = 13,
};

/* Message direction. */
enum proto_dir { PROTO_H2D = 1, PROTO_D2H = 2, PROTO_BOTH = 3 };

/* Config keys (SPEC §6.1). */
enum proto_config_key {
    PROTO_KEY_SERIAL_JACK_MODE = 0x01,
    PROTO_KEY_PTT_TARGETS = 0x02,
    PROTO_KEY_LINE_MAP = 0x03,
    PROTO_KEY_PTT_KEEPALIVE_MS = 0x04,
    PROTO_KEY_MAX_TX_S = 0x05,
    PROTO_KEY_AUDIO_PATH = 0x06,
    PROTO_KEY_TX_LEVEL = 0x07,
    PROTO_KEY_RX_ATTENUATOR = 0x08,
    PROTO_KEY_RX_GAIN = 0x09,
    PROTO_KEY_HOST_MODE = 0x0A,
    PROTO_KEY_BLE_TX_POWER = 0x0B,
    PROTO_KEY_WIRED_PROFILE = 0x0C,
    PROTO_KEY_SERIAL_DEFAULT = 0x0D,
    PROTO_KEY_USB_NET_SUBNET = 0x0E,
    PROTO_KEY_PAIRING_WINDOW_S = 0x0F,
    PROTO_KEY_POWER_DOWN_DELAY_S = 0x10,
    PROTO_KEY_WIRED_PORT_LOCK = 0x11,
};
#define PROTO_KEY_FIRST PROTO_KEY_SERIAL_JACK_MODE
#define PROTO_KEY_LAST PROTO_KEY_WIRED_PORT_LOCK

/* CAPS TLV tags (SPEC §5.1). */
enum proto_tlv_tag {
    PROTO_TLV_FEATURES = 0x01,
    PROTO_TLV_SERIAL_JACK = 0x02,
    PROTO_TLV_RADIO_USB_SERIAL = 0x03,
    PROTO_TLV_AUDIO = 0x04,
    PROTO_TLV_PTT = 0x05,
    PROTO_TLV_TONE = 0x06,
    PROTO_TLV_BLE_TX_POWER = 0x07,
    PROTO_TLV_PAIRING = 0x08,
};

/* FEATURES bits (SPEC §5.2). */
enum proto_feature {
    PROTO_F_CAT = 1u << 0,
    PROTO_F_PTT_CLOSURE = 1u << 1,
    PROTO_F_AUDIO_RX = 1u << 2,
    PROTO_F_AUDIO_TX = 1u << 3,
    PROTO_F_AUDIO_LC3 = 1u << 4,
    PROTO_F_CLOCK_SYNC = 1u << 5,
    PROTO_F_TONE_SEQUENCE = 1u << 6,
    PROTO_F_L2CAP_COC = 1u << 7,
    PROTO_F_MODEM_STATUS = 1u << 8,
    PROTO_F_BLE_TX_POWER = 1u << 9,
    PROTO_F_RADIO_USB_HOST = 1u << 10,
    PROTO_F_WIRED_MODE = 1u << 11,
    PROTO_F_RX_ATTENUATOR = 1u << 12,
    PROTO_F_CONFIG_PERSIST = 1u << 13,
    PROTO_F_SCHEDULED_AUDIO = 1u << 14,
    PROTO_F_FIRMWARE_UPDATE = 1u << 15, /* reserved: always 0 in 0.1 (SPEC §15) */
    PROTO_F_USB_NETWORK = 1u << 16,
    PROTO_F_PAIRING_WINDOW = 1u << 17,
};

/* A byte range inside a buffer (not owned). */
typedef struct {
    const uint8_t *data;
    uint16_t len;
} proto_bytes_t;

/* --- Payload structs. Member names match SPEC field names. --- */

typedef struct {
    uint8_t proto_major, proto_minor, proto_patch;
    uint16_t max_payload;
    uint8_t flags;
} proto_hello_t;

typedef struct {
    uint8_t proto_major, proto_minor, proto_patch;
    uint8_t fw_major, fw_minor, fw_patch;
    char variant, hw_revision;
    uint16_t max_payload;
    proto_bytes_t build;
} proto_device_info_t;

typedef struct { proto_bytes_t tlvs; } proto_caps_t;
typedef struct { uint8_t request_type, code; } proto_result_t;
typedef struct { proto_bytes_t data; } proto_ping_t;
typedef proto_ping_t proto_pong_t;

typedef struct {
    uint8_t host_link, transport, phy;
    uint32_t conn_interval_us;
    uint16_t att_mtu, coc_mtu, flags;
    uint8_t audio_path;
    uint16_t supply_mv, frame_errors, cat_overflows;
} proto_status_t;

typedef struct {
    uint8_t key, selector;
    union {
        struct { uint8_t mode; } serial_jack_mode;
        struct { uint8_t targets, usb_port; } ptt_targets;
        struct { uint8_t rts_action, dtr_action; } line_map;
        struct { uint16_t ms; } ptt_keepalive_ms;
        struct { uint32_t s; } max_tx_s;
        struct { uint8_t path; } audio_path;
        struct { int16_t centibel; } tx_level;
        struct { uint8_t on; } rx_attenuator;
        struct { int16_t centibel; } rx_gain;
        struct { uint8_t mode; } host_mode;
        struct { int8_t dbm; } ble_tx_power;
        struct { uint8_t profile; } wired_profile;
        struct { uint32_t baud; uint8_t data_bits, parity, stop_bits; } serial_default;
        struct { uint8_t a, b, c, d; } usb_net_subnet;
        struct { uint16_t s; } pairing_window_s;
        struct { uint16_t s; } power_down_delay_s;
        struct { uint8_t level; } wired_port_lock;
    } value;
} proto_config_t;

typedef struct { uint8_t key, selector; } proto_config_get_t;
typedef struct { uint8_t flags; proto_config_t config; } proto_config_set_t;
typedef struct { proto_config_t config; } proto_config_msg_t;
typedef struct { uint8_t flags; } proto_config_reset_t;

typedef struct { uint8_t port, open; } proto_serial_open_t;
typedef struct {
    uint8_t port;
    uint32_t baud;
    uint8_t data_bits, parity, stop_bits;
} proto_serial_set_t;
typedef struct { uint8_t port; proto_bytes_t data; } proto_cat_data_t;
typedef struct { uint8_t port; uint16_t credit; } proto_cat_credit_t;
typedef struct { uint8_t port, lines; } proto_modem_lines_t;
typedef proto_modem_lines_t proto_modem_status_t;

typedef struct { uint8_t state; } proto_ptt_set_t;
typedef struct {
    uint8_t state, sources, reason;
    uint32_t remaining_s;
} proto_ptt_status_t;

typedef struct {
    uint8_t direction, codec;
    uint16_t sample_rate, frame_samples, codec_param;
    uint64_t start_time_us;
} proto_audio_start_t;
typedef struct { uint8_t direction; } proto_audio_stop_t;
typedef struct {
    uint16_t seq;
    uint32_t timestamp;
    uint8_t flags;
    proto_bytes_t data;
} proto_audio_frame_t;
typedef struct {
    uint8_t direction, state;
    uint16_t fill_samples, target_samples, underruns, overruns;
    uint64_t first_sample_time_us;
} proto_audio_status_t;

typedef struct { uint64_t host_t1; } proto_time_req_t;
typedef struct { uint64_t host_t1, device_t2, device_t3; } proto_time_resp_t;
typedef struct {
    uint64_t device_time_us, utc_us;
    uint32_t uncertainty_us;
} proto_time_set_t;

typedef struct {
    uint16_t count;
    uint32_t base_mhz, spacing_mhz, symbol_us;
    uint8_t shaping, bt_x100;
    uint16_t amplitude, ramp_us, ptt_lead_ms, ptt_tail_ms;
    uint8_t flags;
} proto_tone_setup_t;
typedef struct { uint16_t offset; proto_bytes_t tones; } proto_tone_data_t;
typedef struct { uint64_t start_utc_us; } proto_tone_start_t;
typedef struct {
    uint8_t state;
    uint16_t symbol;
    uint8_t reason;
} proto_tone_status_t;

/* Security (SPEC §15). */
#define PROTO_HOST_TOKEN_LEN 16u
#define PROTO_TRUST_IDENT_LEN 6u
#define PROTO_TRUST_RECORD_LEN 8u
#define PROTO_FACTORY_RESET_CONFIRM 0x54455352u /* "RSET" */
typedef struct { uint8_t host_token[PROTO_HOST_TOKEN_LEN]; } proto_auth_t;
typedef struct { uint8_t state, slot; } proto_auth_status_t;
typedef struct { proto_bytes_t records; } proto_trust_list_t; /* 8-byte records */
typedef struct { uint8_t kind, slot; } proto_trust_remove_t;
typedef struct { uint32_t confirm; } proto_factory_reset_t;
/* One TRUST_LIST record. */
typedef struct {
    uint8_t kind; /* 1 Bluetooth bond, 2 approved wired host */
    uint8_t slot;
    uint8_t ident[PROTO_TRUST_IDENT_LEN];
} proto_trust_record_t;

/* One message. `type` selects the union member (lower-case message name). */
typedef struct {
    uint8_t type;
    uint8_t token;
    union {
        proto_hello_t hello;
        proto_device_info_t device_info;
        proto_caps_t caps;
        proto_result_t result;
        proto_ping_t ping;
        proto_pong_t pong;
        proto_status_t status;
        proto_config_get_t config_get;
        proto_config_set_t config_set;
        proto_config_msg_t config;
        proto_config_reset_t config_reset;
        proto_serial_open_t serial_open;
        proto_serial_set_t serial_set;
        proto_cat_data_t cat_data;
        proto_cat_credit_t cat_credit;
        proto_modem_lines_t modem_lines;
        proto_modem_status_t modem_status;
        proto_ptt_set_t ptt_set;
        proto_ptt_status_t ptt_status;
        proto_audio_start_t audio_start;
        proto_audio_stop_t audio_stop;
        proto_audio_frame_t audio_frame;
        proto_audio_status_t audio_status;
        proto_time_req_t time_req;
        proto_time_resp_t time_resp;
        proto_time_set_t time_set;
        proto_tone_setup_t tone_setup;
        proto_tone_data_t tone_data;
        proto_tone_start_t tone_start;
        proto_tone_status_t tone_status;
        proto_auth_t auth;
        proto_auth_status_t auth_status;
        proto_trust_list_t trust_list;
        proto_trust_remove_t trust_remove;
        proto_factory_reset_t factory_reset;
    } u;
} proto_msg_t;

/* A CAPS TLV (SPEC §5.1). Unknown tags keep only tag/len/value. */
typedef struct {
    uint8_t tag;
    uint8_t len;
    const uint8_t *value; /* raw value bytes (decode) or unknown-tag bytes (encode) */
    bool known;
    union {
        struct { uint32_t features; } features;
        struct {
            uint8_t port, modes;
            uint32_t min_baud, max_baud;
            uint16_t tx_buffer;
        } serial_jack;
        struct {
            uint8_t port, chip;
            uint16_t vid, pid;
            uint8_t interface;
            uint16_t tx_buffer;
        } radio_usb_serial;
        struct {
            uint8_t directions, codecs, rates, paths;
            uint16_t max_frame_samples, tx_buffer_samples;
        } audio;
        struct {
            uint8_t outputs;
            uint16_t keepalive_min_ms, keepalive_max_ms;
            uint32_t max_tx_min_s;
        } ptt;
        struct {
            uint16_t max_symbols;
            uint8_t max_tone_index, shaping;
            uint32_t min_symbol_us, max_symbol_us;
        } tone;
        struct { int8_t min_dbm, max_dbm; } ble_tx_power;
        struct {
            uint8_t triggers;
            uint16_t window_min_s, window_max_s;
            uint8_t max_bonds, max_wired_hosts;
        } pairing;
    } v;
} proto_tlv_t;

/* GATT Info characteristic (SPEC §13.2), readable before pairing. */
typedef struct {
    uint8_t proto_major, proto_minor, proto_patch, flags;
    uint16_t psm;
} proto_info_t;
#define PROTO_INFO_LEN 6u
#define PROTO_INFO_F_L2CAP 0x01u
#define PROTO_INFO_F_PAIRING_OPEN 0x02u

/* --- CRC and COBS (SPEC §3) --- */

/* CRC-16/IBM-3740: poly 0x1021, init 0xFFFF, no reflection, no final XOR. */
uint16_t proto_crc16(const uint8_t *data, size_t len);

/* Returns the encoded length, or 0 if `cap` is too small. No delimiter added. */
size_t proto_cobs_encode(const uint8_t *in, size_t len, uint8_t *out, size_t cap);

/* Returns the decoded length, or -1 on a malformed block or overflow. */
int proto_cobs_decode(const uint8_t *in, size_t len, uint8_t *out, size_t cap);

/* --- Messages --- */

/* True if `type` is a message type defined in this protocol version. */
bool proto_type_known(uint8_t type);
/* PROTO_H2D, PROTO_D2H or PROTO_BOTH; 0 for an unknown type. */
int proto_type_dir(uint8_t type);

/* Payload <-> struct. Return PROTO_OK or a proto_code (UNKNOWN_TYPE,
 * BAD_LENGTH, BAD_VALUE for an unknown config key or a malformed TLV list). */
int proto_decode_payload(uint8_t type, uint8_t token, const uint8_t *payload, size_t len,
                         proto_msg_t *out);
int proto_encode_payload(const proto_msg_t *msg, uint8_t *out, size_t cap, size_t *len);

/* Struct -> wire bytes (COBS frame plus the 0x00 delimiter). Returns the
 * number of bytes written, or 0 on error (bad fields or `cap` too small). */
size_t proto_encode(const proto_msg_t *msg, uint8_t *wire, size_t cap);

/* --- CAPS TLVs --- */

/* Appends one TLV at out[*pos]. Returns PROTO_OK or PROTO_ERR_OVERFLOW. */
int proto_tlv_put(const proto_tlv_t *tlv, uint8_t *out, size_t cap, size_t *pos);

/* Reads the next TLV from *cursor and advances it. Returns 1 for a TLV,
 * 0 at the end, or -1 for a truncated or too-short TLV. A known tag longer
 * than expected is accepted (newer minor version, SPEC §5). */
int proto_tlv_next(proto_bytes_t *cursor, proto_tlv_t *out);

/* --- TRUST_LIST records --- */

/* Appends one record. Returns PROTO_OK or PROTO_ERR_OVERFLOW. */
int proto_trust_put(const proto_trust_record_t *r, uint8_t *out, size_t cap, size_t *pos);
/* Reads the next record: 1, 0 at the end (a decoded list is always whole). */
int proto_trust_next(proto_bytes_t *cursor, proto_trust_record_t *out);

/* --- Config values --- */

/* Length in bytes of a config key's value (after key and selector), or -1
 * for an unknown key. */
int proto_config_value_len(uint8_t key);

/* A whole config record (key, selector, value), as in CONFIG and CONFIG_SET.
 * Decoding needs the exact length. Return PROTO_OK or a proto_code. */
int proto_config_encode(const proto_config_t *c, uint8_t *out, size_t cap, size_t *pos);
int proto_config_decode(const uint8_t *in, size_t len, proto_config_t *c);

/* --- Info characteristic --- */
void proto_info_encode(const proto_info_t *info, uint8_t out[PROTO_INFO_LEN]);
int proto_info_decode(const uint8_t *data, size_t len, proto_info_t *out);

/* --- Frame parsing --- */

/* Checks a COBS-decoded frame (length and CRC). On success sets type, token
 * and the payload range and returns PROTO_OK; otherwise PROTO_ERR_FRAME. */
int proto_frame_check(const uint8_t *frame, size_t len, uint8_t *type, uint8_t *token,
                      const uint8_t **payload, size_t *payload_len);

/* --- Stream receiver (SPEC §3.4) --- */

typedef void (*proto_frame_cb)(void *ctx, uint8_t type, uint8_t token, const uint8_t *payload,
                               size_t len);
typedef void (*proto_frame_error_cb)(void *ctx);

typedef struct {
    uint8_t buf[PROTO_MAX_WIRE];   /* bytes of the current (COBS) frame */
    uint8_t frame[PROTO_MAX_FRAME]; /* COBS-decoded frame, valid during callbacks */
    size_t len;
    size_t max_frame; /* max_payload + 4 accepted from this peer */
    bool discarding;  /* frame too long: skip to the next delimiter */
    uint32_t errors;  /* frames dropped (COBS, length, CRC) */
    proto_frame_cb on_frame;
    proto_frame_error_cb on_error;
    void *ctx;
} proto_rx_t;

void proto_rx_init(proto_rx_t *rx, proto_frame_cb on_frame, proto_frame_error_cb on_error,
                   void *ctx);
/* Drops any partial frame (for example when a transport closes). */
void proto_rx_reset(proto_rx_t *rx);
void proto_rx_feed(proto_rx_t *rx, const uint8_t *data, size_t len);

/* GATT write/notification size for an ATT_MTU (SPEC §13.1). */
static inline size_t proto_gatt_chunk_size(uint16_t att_mtu)
{
    return att_mtu > 3u ? (size_t)att_mtu - 3u : 0u;
}

#ifdef __cplusplus
}
#endif

#endif /* PROTO_H */

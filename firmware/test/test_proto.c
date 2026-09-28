/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Protocol codec against every golden vector in protocol/vectors/.
 */
#include <stdio.h>
#include <string.h>

#include "proto.h"
#include "unity.h"
#include "vectors.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* --- Receiver capture --- */

typedef struct {
    uint8_t type[64], token[64];
    uint8_t payload[64][PROTO_MAX_PAYLOAD];
    size_t len[64];
    size_t n;
    unsigned errors;
} capture_t;

static void cap_frame(void *ctx, uint8_t type, uint8_t token, const uint8_t *payload, size_t len)
{
    capture_t *c = (capture_t *)ctx;
    TEST_ASSERT_TRUE(c->n < 64);
    c->type[c->n] = type;
    c->token[c->n] = token;
    memcpy(c->payload[c->n], payload, len);
    c->len[c->n] = len;
    c->n++;
}

static void cap_error(void *ctx)
{
    ((capture_t *)ctx)->errors++;
}

static capture_t cap;
static proto_rx_t rx;

static void rx_start(void)
{
    memset(&cap, 0, sizeof(cap));
    proto_rx_init(&rx, cap_frame, cap_error, &cap);
}

/* --- CRC and COBS --- */

static void test_version_matches_vectors(void)
{
    char v[16];
    snprintf(v, sizeof(v), "%d.%d.%d", PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR,
             PROTO_VERSION_PATCH);
    TEST_ASSERT_EQUAL_STRING(vec_protocol_version, v);
}

static void test_crc16_vectors(void)
{
    TEST_ASSERT_TRUE(vec_crc_count > 0);
    for (size_t i = 0; i < vec_crc_count; i++) {
        TEST_ASSERT_EQUAL_HEX16(vec_crc[i].crc, proto_crc16(vec_crc[i].input.p, vec_crc[i].input.n));
    }
}

static void test_cobs_vectors(void)
{
    uint8_t buf[600];
    TEST_ASSERT_TRUE(vec_cobs_count > 0);
    for (size_t i = 0; i < vec_cobs_count; i++) {
        const vec_cobs_t *c = &vec_cobs[i];
        size_t n = proto_cobs_encode(c->decoded.p, c->decoded.n, buf, sizeof(buf));
        TEST_ASSERT_EQUAL_size_t(c->encoded.n, n);
        TEST_ASSERT_EQUAL_MEMORY(c->encoded.p, buf, n);
        int d = proto_cobs_decode(c->encoded.p, c->encoded.n, buf, sizeof(buf));
        TEST_ASSERT_EQUAL_INT((int)c->decoded.n, d);
        if (d > 0) {
            TEST_ASSERT_EQUAL_MEMORY(c->decoded.p, buf, (size_t)d);
        }
    }
}

static void test_cobs_rejects_bad_input(void)
{
    uint8_t buf[16];
    const uint8_t zero_code[] = {0x00, 0x11};
    const uint8_t runs_past[] = {0x05, 0x11, 0x22};
    const uint8_t zero_inside[] = {0x03, 0x11, 0x00};
    TEST_ASSERT_EQUAL_INT(-1, proto_cobs_decode(zero_code, sizeof(zero_code), buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_INT(-1, proto_cobs_decode(runs_past, sizeof(runs_past), buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_INT(-1, proto_cobs_decode(zero_inside, sizeof(zero_inside), buf, sizeof(buf)));
    /* Output buffer too small. */
    const uint8_t ok[] = {0x04, 0x11, 0x22, 0x33};
    TEST_ASSERT_EQUAL_INT(-1, proto_cobs_decode(ok, sizeof(ok), buf, 2));
    TEST_ASSERT_EQUAL_size_t(0, proto_cobs_encode(ok, sizeof(ok), buf, 3));
    TEST_ASSERT_EQUAL_size_t(0, proto_cobs_encode(ok, 0, buf, 0));
}

static void test_cobs_round_trip_all_lengths(void)
{
    static uint8_t in[1100], enc[1200], dec[1100];
    for (size_t len = 0; len < sizeof(in); len += 37) {
        for (size_t i = 0; i < len; i++) {
            in[i] = (uint8_t)((i * 7u) % 5u == 0 ? 0 : i);
        }
        size_t n = proto_cobs_encode(in, len, enc, sizeof(enc));
        TEST_ASSERT_TRUE(n > 0);
        TEST_ASSERT_TRUE(n <= PROTO_COBS_MAX(len));
        TEST_ASSERT_NULL(memchr(enc, 0, n));
        TEST_ASSERT_EQUAL_INT((int)len, proto_cobs_decode(enc, n, dec, sizeof(dec)));
        if (len) {
            TEST_ASSERT_EQUAL_MEMORY(in, dec, len);
        }
    }
}

/* --- Messages --- */

static void test_every_message_encodes_to_its_vector(void)
{
    static uint8_t buf[PROTO_MAX_PAYLOAD], wire[PROTO_MAX_WIRE], payload[PROTO_MAX_PAYLOAD];
    TEST_ASSERT_TRUE(vec_messages_count > 0);
    for (size_t i = 0; i < vec_messages_count; i++) {
        const vec_msg_t *v = &vec_messages[i];
        proto_msg_t m;
        memset(&m, 0, sizeof(m));
        m.type = v->type;
        m.token = v->token;
        v->fill(&m, buf, sizeof(buf));

        size_t plen = 0;
        TEST_ASSERT_EQUAL_INT_MESSAGE(PROTO_OK, proto_encode_payload(&m, payload, sizeof(payload), &plen),
                                      v->name);
        TEST_ASSERT_EQUAL_size_t_MESSAGE(v->payload.n, plen, v->name);
        if (plen) {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(v->payload.p, payload, plen, v->name);
        }

        size_t n = proto_encode(&m, wire, sizeof(wire));
        TEST_ASSERT_EQUAL_size_t_MESSAGE(v->wire.n, n, v->name);
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(v->wire.p, wire, n, v->name);
    }
}

static void test_every_message_decodes_from_its_vector(void)
{
    for (size_t i = 0; i < vec_messages_count; i++) {
        const vec_msg_t *v = &vec_messages[i];
        rx_start();
        proto_rx_feed(&rx, v->wire.p, v->wire.n);
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cap.errors, v->name);
        TEST_ASSERT_EQUAL_size_t_MESSAGE(1, cap.n, v->name);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(v->type, cap.type[0], v->name);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(v->token, cap.token[0], v->name);
        TEST_ASSERT_EQUAL_size_t_MESSAGE(v->payload.n, cap.len[0], v->name);
        if (cap.len[0]) {
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(v->payload.p, cap.payload[0], cap.len[0], v->name);
        }

        /* The frame between COBS and the payload matches too. */
        uint8_t frame[PROTO_MAX_FRAME];
        int fl = proto_cobs_decode(v->wire.p, v->wire.n - 1, frame, sizeof(frame));
        TEST_ASSERT_EQUAL_INT_MESSAGE((int)v->frame.n, fl, v->name);
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(v->frame.p, frame, v->frame.n, v->name);

        proto_msg_t m;
        TEST_ASSERT_EQUAL_INT_MESSAGE(PROTO_OK,
                                      proto_decode_payload(cap.type[0], cap.token[0], cap.payload[0],
                                                           cap.len[0], &m),
                                      v->name);
        TEST_ASSERT_EQUAL_UINT8(v->type, m.type);
        TEST_ASSERT_EQUAL_UINT8(v->token, m.token);
        v->check(&m);
    }
}

static void test_every_message_type_has_a_vector(void)
{
    for (unsigned t = 1; t < 0xE0; t++) {
        if (!proto_type_known((uint8_t)t)) {
            continue;
        }
        bool found = false;
        for (size_t i = 0; i < vec_messages_count; i++) {
            found = found || vec_messages[i].type == t;
        }
        char msg[32];
        snprintf(msg, sizeof(msg), "type 0x%02x", t);
        TEST_ASSERT_TRUE_MESSAGE(found, msg);
    }
}

static void test_every_config_key_has_a_vector(void)
{
    for (unsigned key = PROTO_KEY_FIRST; key <= PROTO_KEY_LAST; key++) {
        bool found = false;
        for (size_t i = 0; i < vec_messages_count; i++) {
            const vec_msg_t *v = &vec_messages[i];
            if (v->type == PROTO_CONFIG_SET || v->type == PROTO_CONFIG) {
                found = found || v->payload.p[v->type == PROTO_CONFIG_SET ? 1 : 0] == key;
            }
        }
        char msg[32];
        snprintf(msg, sizeof(msg), "config key 0x%02x", key);
        TEST_ASSERT_TRUE_MESSAGE(found, msg);
    }
}

static void test_invalid_frames(void)
{
    TEST_ASSERT_TRUE(vec_invalid_count > 0);
    for (size_t i = 0; i < vec_invalid_count; i++) {
        const vec_invalid_t *v = &vec_invalid[i];
        rx_start();
        proto_rx_feed(&rx, v->wire.p, v->wire.n);
        if (v->expect == VEC_FRAME_ERROR) {
            TEST_ASSERT_TRUE_MESSAGE(cap.errors >= 1, v->name);
            TEST_ASSERT_EQUAL_size_t_MESSAGE(0, cap.n, v->name);
            continue;
        }
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cap.errors, v->name);
        TEST_ASSERT_EQUAL_size_t_MESSAGE(1, cap.n, v->name);
        proto_msg_t m;
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            v->expect, proto_decode_payload(cap.type[0], cap.token[0], cap.payload[0], cap.len[0], &m),
            v->name);
    }
}

static void test_stream_with_resync(void)
{
    rx_start();
    proto_rx_feed(&rx, vec_stream.wire.p, vec_stream.wire.n);
    TEST_ASSERT_EQUAL_UINT(vec_stream.errors, cap.errors);
    TEST_ASSERT_EQUAL_size_t(vec_stream.ntypes, cap.n);
    for (size_t i = 0; i < cap.n; i++) {
        TEST_ASSERT_EQUAL_UINT8(vec_stream.types[i], cap.type[i]);
    }
}

static void test_stream_byte_by_byte(void)
{
    rx_start();
    for (size_t i = 0; i < vec_stream.wire.n; i++) {
        proto_rx_feed(&rx, &vec_stream.wire.p[i], 1);
    }
    TEST_ASSERT_EQUAL_UINT(vec_stream.errors, cap.errors);
    TEST_ASSERT_EQUAL_size_t(vec_stream.ntypes, cap.n);
}

static void test_oversized_frame_is_discarded(void)
{
    static uint8_t junk[PROTO_MAX_WIRE + 50];
    memset(junk, 0x11, sizeof(junk));
    rx_start();
    proto_rx_feed(&rx, junk, sizeof(junk));
    const uint8_t delim = 0;
    proto_rx_feed(&rx, &delim, 1);
    TEST_ASSERT_EQUAL_UINT(1, cap.errors);
    /* The receiver resynchronizes on the next frame. */
    proto_rx_feed(&rx, vec_messages[0].wire.p, vec_messages[0].wire.n);
    TEST_ASSERT_EQUAL_size_t(1, cap.n);
}

static void test_short_frame_and_reset(void)
{
    const uint8_t short_frame[] = {0x03, 0x30, 0x01, 0x00}; /* 2 bytes after COBS */
    rx_start();
    proto_rx_feed(&rx, short_frame, sizeof(short_frame));
    TEST_ASSERT_EQUAL_UINT(1, cap.errors);
    /* A partial frame is dropped by proto_rx_reset(). */
    proto_rx_feed(&rx, vec_messages[0].wire.p, vec_messages[0].wire.n - 3);
    proto_rx_reset(&rx);
    proto_rx_feed(&rx, vec_messages[0].wire.p, vec_messages[0].wire.n);
    TEST_ASSERT_EQUAL_size_t(1, cap.n);
    TEST_ASSERT_EQUAL_UINT(1, cap.errors);
}

static void test_encode_rejects_unknown_type_and_small_buffer(void)
{
    uint8_t wire[8];
    proto_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = 0x7E;
    TEST_ASSERT_EQUAL_size_t(0, proto_encode(&m, wire, sizeof(wire)));
    m.type = PROTO_STATUS; /* 16-byte payload doesn't fit in 8 bytes */
    TEST_ASSERT_EQUAL_size_t(0, proto_encode(&m, wire, sizeof(wire)));
    TEST_ASSERT_EQUAL_INT(0, proto_type_dir(0x7E));
    TEST_ASSERT_EQUAL_INT(PROTO_H2D, proto_type_dir(PROTO_PTT_SET));
    TEST_ASSERT_EQUAL_INT(PROTO_D2H, proto_type_dir(PROTO_PTT_STATUS));
    TEST_ASSERT_EQUAL_INT(PROTO_BOTH, proto_type_dir(PROTO_CAT_DATA));
}

static void test_encode_and_decode_limits(void)
{
    static uint8_t big[PROTO_MAX_PAYLOAD + 1], out[PROTO_MAX_WIRE + 8];
    proto_msg_t m;
    /* A payload longer than 1024 bytes is BAD_LENGTH. */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_LENGTH, proto_decode_payload(PROTO_PING, 0, big, sizeof(big), &m));
    /* Byte field overflow. */
    memset(&m, 0, sizeof(m));
    m.type = PROTO_PING;
    m.u.ping.data.data = big;
    m.u.ping.data.len = 4;
    size_t len;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_encode_payload(&m, out, 2, &len));
    /* A caller buffer larger than the protocol maximum is capped. */
    m.u.ping.data.len = PROTO_MAX_PAYLOAD + 1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_encode_payload(&m, out, sizeof(out), &len));
    /* A config message with an unknown key doesn't encode. */
    memset(&m, 0, sizeof(m));
    m.type = PROTO_CONFIG;
    m.u.config.config.key = 0x7F;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, proto_encode_payload(&m, out, sizeof(out), &len));
    /* No room for anything. */
    m.type = PROTO_KEEPALIVE;
    TEST_ASSERT_EQUAL_size_t(0, proto_encode(&m, out, 0));
    TEST_ASSERT_EQUAL_size_t(6, proto_encode(&m, out, 6)); /* code + 4-byte frame + delimiter */
    /* COBS: no room for the zero a block implies. */
    const uint8_t two_blocks[] = {0x02, 0x11, 0x01};
    uint8_t one[1];
    TEST_ASSERT_EQUAL_INT(-1, proto_cobs_decode(two_blocks, sizeof(two_blocks), one, sizeof(one)));
    /* Raw TLV overflow. */
    proto_tlv_t t;
    memset(&t, 0, sizeof(t));
    t.tag = 0x7F;
    t.value = big;
    t.len = 10;
    size_t pos = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_tlv_put(&t, out, 8, &pos));
}

static void test_config_record_errors(void)
{
    proto_config_t c;
    const uint8_t too_short[] = {0x05};
    const uint8_t bad_len[] = {0x05, 0x00, 0x2c}; /* MAX_TX_S needs 4 value bytes */
    const uint8_t unknown[] = {0xEE, 0x00, 0x01};
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_LENGTH, proto_config_decode(too_short, sizeof(too_short), &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_LENGTH, proto_config_decode(bad_len, sizeof(bad_len), &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, proto_config_decode(unknown, sizeof(unknown), &c));
    TEST_ASSERT_EQUAL_INT(-1, proto_config_value_len(0));
    TEST_ASSERT_EQUAL_INT(4, proto_config_value_len(PROTO_KEY_MAX_TX_S));
    uint8_t out[4];
    size_t pos = 0;
    memset(&c, 0, sizeof(c));
    c.key = 0xEE;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, proto_config_encode(&c, out, sizeof(out), &pos));
    c.key = PROTO_KEY_MAX_TX_S;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_config_encode(&c, out, sizeof(out), &pos));
}

/* --- TLVs --- */

static void test_tlv_newer_minor_appends_fields(void)
{
    /* A FEATURES TLV with 2 extra bytes: the known prefix is read. */
    const uint8_t data[] = {0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0xAA, 0xBB};
    proto_bytes_t cur = {data, sizeof(data)};
    proto_tlv_t t;
    TEST_ASSERT_EQUAL_INT(1, proto_tlv_next(&cur, &t));
    TEST_ASSERT_TRUE(t.known);
    TEST_ASSERT_EQUAL_UINT32(1, t.v.features.features);
    TEST_ASSERT_EQUAL_INT(0, proto_tlv_next(&cur, &t));
}

static void test_tlv_errors(void)
{
    proto_tlv_t t;
    const uint8_t truncated_header[] = {0x01};
    const uint8_t truncated_value[] = {0x01, 0x04, 0x00};
    const uint8_t too_short_known[] = {0x01, 0x02, 0x00, 0x00};
    proto_bytes_t c1 = {truncated_header, sizeof(truncated_header)};
    proto_bytes_t c2 = {truncated_value, sizeof(truncated_value)};
    proto_bytes_t c3 = {too_short_known, sizeof(too_short_known)};
    TEST_ASSERT_EQUAL_INT(-1, proto_tlv_next(&c1, &t));
    TEST_ASSERT_EQUAL_INT(-1, proto_tlv_next(&c2, &t));
    TEST_ASSERT_EQUAL_INT(-1, proto_tlv_next(&c3, &t));
    /* A CAPS payload with a bad TLV list is BAD_VALUE. */
    proto_msg_t m;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE,
                          proto_decode_payload(PROTO_CAPS, 0, truncated_value, sizeof(truncated_value), &m));
    /* proto_tlv_put overflow. */
    uint8_t out[3];
    size_t pos = 0;
    memset(&t, 0, sizeof(t));
    t.tag = PROTO_TLV_FEATURES;
    t.known = true;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_tlv_put(&t, out, sizeof(out), &pos));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, proto_tlv_put(&t, out, 1, &pos));
}

/* --- GATT --- */

static void test_info_vectors(void)
{
    for (size_t i = 0; i < vec_info_count; i++) {
        uint8_t out[PROTO_INFO_LEN];
        proto_info_encode(&vec_info[i].fields, out);
        TEST_ASSERT_EQUAL_MEMORY(vec_info[i].value.p, out, PROTO_INFO_LEN);
        proto_info_t d;
        TEST_ASSERT_EQUAL_INT(PROTO_OK, proto_info_decode(vec_info[i].value.p, vec_info[i].value.n, &d));
        TEST_ASSERT_EQUAL_UINT8(vec_info[i].fields.flags, d.flags);
        TEST_ASSERT_EQUAL_UINT16(vec_info[i].fields.psm, d.psm);
        TEST_ASSERT_EQUAL_UINT8(vec_info[i].fields.proto_minor, d.proto_minor);
    }
    proto_info_t d;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_LENGTH, proto_info_decode(vec_info[0].value.p, 5, &d));
}

static void test_gatt_chunking(void)
{
    const vec_chunking_t *c = &vec_chunking;
    size_t size = proto_gatt_chunk_size(c->att_mtu);
    TEST_ASSERT_EQUAL_size_t(20, size);
    TEST_ASSERT_EQUAL_size_t(0, proto_gatt_chunk_size(3));
    size_t off = 0;
    rx_start();
    for (size_t i = 0; i < c->nchunks; i++) {
        size_t n = c->wire.n - off < size ? c->wire.n - off : size;
        TEST_ASSERT_EQUAL_size_t(c->chunks[i].n, n);
        TEST_ASSERT_EQUAL_MEMORY(c->chunks[i].p, c->wire.p + off, n);
        proto_rx_feed(&rx, c->chunks[i].p, c->chunks[i].n); /* chunk boundaries carry no meaning */
        off += n;
    }
    TEST_ASSERT_EQUAL_size_t(c->wire.n, off);
    TEST_ASSERT_EQUAL_size_t(1, cap.n);
    TEST_ASSERT_EQUAL_UINT8(PROTO_CAPS, cap.type[0]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_version_matches_vectors);
    RUN_TEST(test_crc16_vectors);
    RUN_TEST(test_cobs_vectors);
    RUN_TEST(test_cobs_rejects_bad_input);
    RUN_TEST(test_cobs_round_trip_all_lengths);
    RUN_TEST(test_every_message_encodes_to_its_vector);
    RUN_TEST(test_every_message_decodes_from_its_vector);
    RUN_TEST(test_every_message_type_has_a_vector);
    RUN_TEST(test_every_config_key_has_a_vector);
    RUN_TEST(test_invalid_frames);
    RUN_TEST(test_stream_with_resync);
    RUN_TEST(test_stream_byte_by_byte);
    RUN_TEST(test_oversized_frame_is_discarded);
    RUN_TEST(test_short_frame_and_reset);
    RUN_TEST(test_encode_rejects_unknown_type_and_small_buffer);
    RUN_TEST(test_encode_and_decode_limits);
    RUN_TEST(test_config_record_errors);
    RUN_TEST(test_tlv_newer_minor_appends_fields);
    RUN_TEST(test_tlv_errors);
    RUN_TEST(test_info_vectors);
    RUN_TEST(test_gatt_chunking);
    return UNITY_END();
}

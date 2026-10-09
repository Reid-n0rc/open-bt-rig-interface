/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * The device core: sessions and dispatch (SPEC §4, §8.7, §12, §13.5, §14).
 */
#include <string.h>

#include "app.h"
#include "ble_power.h"
#include "fake_hal.h"
#include "fake_security.h"
#include "fw_version.h"
#include "unity.h"

static const board_t board = {
    .variant = 'D',
    .hw_revision = 'A',
    .features = PROTO_F_CAT | PROTO_F_PTT_CLOSURE | PROTO_F_CLOCK_SYNC | PROTO_F_BLE_TX_POWER |
                PROTO_F_CONFIG_PERSIST | PROTO_F_PAIRING_WINDOW | PROTO_F_FIRMWARE_UPDATE,
    .serial_modes = 0x01,
    .serial_min_baud = 4800,
    .serial_max_baud = 115200,
    .cat_tx_buffer = 256,
    .ptt_outputs = PTT_TARGET_CLOSURE,
    .pairing_triggers = BOARD_PAIR_POWER_ON | BOARD_PAIR_BUTTON,
    .max_bonds = 4,
    .max_wired_hosts = 2,
    .l2cap_psm = 0,
};

static app_t app;
static uint64_t now_us;
static proto_msg_t got[FAKE_MSG_MAX];
static size_t ngot;

#define GATT HAL_TRANSPORT_GATT

void setUp(void)
{
    fake_reset();
    fake_sec.approved = false;
    fake_sec.approve_result = PROTO_ERR_UNSUPPORTED; /* as the #14 stubs */
    fake_sec.approve_calls = 0;
    fake_sec.factory_reset_result = PROTO_ERR_UNSUPPORTED;
    now_us = 1000000;
    fake.time_us = now_us;
    app_init(&app, &board, HAL_LINK_BLUETOOTH, PTT_R_BOOT, now_us);
}

void tearDown(void)
{
}

static void advance_ms(uint64_t ms)
{
    now_us += ms * 1000u;
    fake.time_us = now_us;
    app_tick(&app, now_us);
}

static void send_msg(uint8_t transport, const proto_msg_t *m)
{
    uint8_t wire[PROTO_MAX_WIRE];
    size_t n = proto_encode(m, wire, sizeof(wire));
    TEST_ASSERT_TRUE(n > 0);
    app_rx(&app, transport, wire, n, now_us);
}

static size_t receive(uint8_t transport)
{
    ngot = fake_sent(transport, got, FAKE_MSG_MAX);
    return ngot;
}

/* The last message of a type among those received. */
static const proto_msg_t *find(uint8_t type)
{
    for (size_t i = ngot; i > 0; i--) {
        if (got[i - 1].type == type) {
            return &got[i - 1];
        }
    }
    return NULL;
}

static void hello(uint8_t transport, uint8_t minor)
{
    proto_msg_t m = {.type = PROTO_HELLO, .token = 1};
    m.u.hello.proto_major = 0;
    m.u.hello.proto_minor = minor;
    m.u.hello.max_payload = 1024;
    send_msg(transport, &m);
}

static void open_session(void)
{
    hello(GATT, PROTO_VERSION_MINOR);
    receive(GATT);
    TEST_ASSERT_NOT_NULL(find(PROTO_DEVICE_INFO));
}

static void simple(uint8_t type, uint8_t token)
{
    proto_msg_t m = {.type = type, .token = token};
    send_msg(GATT, &m);
}

static const proto_result_t *result(void)
{
    const proto_msg_t *r = find(PROTO_RESULT);
    TEST_ASSERT_NOT_NULL(r);
    return &r->u.result;
}

static void send_ptt(uint8_t state, uint8_t token)
{
    proto_msg_t m = {.type = PROTO_PTT_SET, .token = token};
    m.u.ptt_set.state = state;
    send_msg(GATT, &m);
}

/* --- Session start (SPEC §4.2) --- */

static void test_hello_answers_device_info(void)
{
    hello(GATT, PROTO_VERSION_MINOR);
    receive(GATT);
    const proto_msg_t *d = find(PROTO_DEVICE_INFO);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_EQUAL_UINT8(1, d->token);
    TEST_ASSERT_EQUAL_UINT8(PROTO_VERSION_MAJOR, d->u.device_info.proto_major);
    TEST_ASSERT_EQUAL_UINT8(PROTO_VERSION_MINOR, d->u.device_info.proto_minor);
    TEST_ASSERT_EQUAL_UINT8(FW_VERSION_MAJOR, d->u.device_info.fw_major);
    TEST_ASSERT_EQUAL_UINT8(FW_VERSION_MINOR, d->u.device_info.fw_minor);
    TEST_ASSERT_EQUAL_UINT8(FW_VERSION_PATCH, d->u.device_info.fw_patch);
    TEST_ASSERT_EQUAL_CHAR('D', d->u.device_info.variant);
    TEST_ASSERT_EQUAL_CHAR('A', d->u.device_info.hw_revision);
    TEST_ASSERT_EQUAL_UINT16(PROTO_MAX_PAYLOAD, d->u.device_info.max_payload);
    TEST_ASSERT_TRUE(d->u.device_info.build.len <= PROTO_BUILD_MAX);
    TEST_ASSERT_EQUAL_MEMORY(FW_VERSION_BUILD, d->u.device_info.build.data,
                             d->u.device_info.build.len);
    TEST_ASSERT_EQUAL_UINT8(GATT, app.active);
}

static void test_boot_status_reported_to_first_session(void)
{
    hello(GATT, PROTO_VERSION_MINOR);
    receive(GATT);
    const proto_msg_t *s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(0, s->token);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_BOOT, s->u.ptt_status.reason);
    TEST_ASSERT_EQUAL_UINT8(0, s->u.ptt_status.state);
    /* Only once. */
    hello(GATT, PROTO_VERSION_MINOR);
    receive(GATT);
    TEST_ASSERT_NULL(find(PROTO_PTT_STATUS));
}

static void test_watchdog_reset_reported_to_next_session(void)
{
    fake_reset();
    app_init(&app, &board, HAL_LINK_BLUETOOTH, app_boot_reason(APP_RESET_WATCHDOG), now_us);
    TEST_ASSERT_FALSE(fake.closure);
    advance_ms(5000); /* no session yet */
    hello(GATT, PROTO_VERSION_MINOR);
    receive(GATT);
    const proto_msg_t *s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_WATCHDOG, s->u.ptt_status.reason);
}

static void test_pending_watchdog_not_hidden_by_later_status(void)
{
    fake_reset();
    app_init(&app, &board, HAL_LINK_WIRED, app_boot_reason(APP_RESET_WATCHDOG), now_us);
    /* Wired serial profile: native lines key PTT with no protocol session. */
    app_native_lines(&app, 0, 0, now_us);
    app_native_lines(&app, 0, PTT_LINE_RTS, now_us);
    TEST_ASSERT_TRUE(fake.closure);
    app_native_lines(&app, 0, 0, now_us);
    hello(HAL_TRANSPORT_TCP, PROTO_VERSION_MINOR);
    receive(HAL_TRANSPORT_TCP);
    const proto_msg_t *s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_WATCHDOG, s->u.ptt_status.reason);
}

static void test_version_mismatch(void)
{
    hello(GATT, PROTO_VERSION_MINOR + 1);
    receive(GATT);
    TEST_ASSERT_NOT_NULL(find(PROTO_DEVICE_INFO)); /* always answers HELLO */
    simple(PROTO_CAPS_GET, 2);
    receive(GATT);
    TEST_ASSERT_NOT_NULL(find(PROTO_CAPS));
    simple(PROTO_STATUS_GET, 3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_VERSION_MISMATCH, result()->code);
    send_ptt(1, 4);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_VERSION_MISMATCH, result()->code);
    TEST_ASSERT_FALSE(fake.closure);
}

static void test_requests_before_hello_are_ignored(void)
{
    send_ptt(1, 5);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
}

static void test_hello_errors_answered_on_that_transport(void)
{
    const uint8_t bad_hello[] = {0x01, 0x07, 0x00}; /* payload too short */
    uint8_t frame[16], wire[32];
    memcpy(frame, bad_hello, 3);
    uint16_t crc = proto_crc16(frame, 3);
    frame[3] = (uint8_t)crc;
    frame[4] = (uint8_t)(crc >> 8);
    size_t n = proto_cobs_encode(frame, 5, wire, sizeof(wire));
    wire[n++] = 0;
    app_rx(&app, GATT, wire, n, now_us);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_LENGTH, result()->code);
    TEST_ASSERT_EQUAL_UINT8(0, app.active);
}

/* --- Replies (SPEC §4.3) --- */

static void test_caps(void)
{
    open_session();
    simple(PROTO_CAPS_GET, 9);
    receive(GATT);
    const proto_msg_t *c = find(PROTO_CAPS);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_UINT8(9, c->token);
    proto_bytes_t cur = c->u.caps.tlvs;
    proto_tlv_t t;
    bool seen_ptt = false, seen_ble = false, seen_pairing = false;
    while (proto_tlv_next(&cur, &t) == 1) {
        if (t.tag == PROTO_TLV_FEATURES) {
            TEST_ASSERT_EQUAL_UINT32(0, t.v.features.features & PROTO_F_FIRMWARE_UPDATE);
            TEST_ASSERT_TRUE(t.v.features.features & PROTO_F_CAT);
        } else if (t.tag == PROTO_TLV_PTT) {
            seen_ptt = true;
            TEST_ASSERT_EQUAL_UINT16(500, t.v.ptt.keepalive_min_ms);
            TEST_ASSERT_EQUAL_UINT16(10000, t.v.ptt.keepalive_max_ms);
            TEST_ASSERT_EQUAL_UINT32(10, t.v.ptt.max_tx_min_s);
        } else if (t.tag == PROTO_TLV_BLE_TX_POWER) {
            seen_ble = true;
            TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, t.v.ble_tx_power.max_dbm);
            TEST_ASSERT_EQUAL_INT8(-24, t.v.ble_tx_power.min_dbm);
        } else if (t.tag == PROTO_TLV_PAIRING) {
            seen_pairing = true;
            TEST_ASSERT_EQUAL_UINT8(BOARD_PAIR_POWER_ON | BOARD_PAIR_BUTTON, t.v.pairing.triggers);
            TEST_ASSERT_EQUAL_UINT16(30, t.v.pairing.window_min_s);
            TEST_ASSERT_EQUAL_UINT16(600, t.v.pairing.window_max_s);
            TEST_ASSERT_EQUAL_UINT8(4, t.v.pairing.max_bonds);
            TEST_ASSERT_EQUAL_UINT8(2, t.v.pairing.max_wired_hosts);
        }
    }
    TEST_ASSERT_TRUE(seen_ptt && seen_ble && seen_pairing);
}

static void test_ping_status_and_unknown(void)
{
    open_session();
    proto_msg_t m = {.type = PROTO_PING, .token = 6};
    const uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
    m.u.ping.data.data = data;
    m.u.ping.data.len = 4;
    send_msg(GATT, &m);
    receive(GATT);
    const proto_msg_t *p = find(PROTO_PONG);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL_UINT16(4, p->u.pong.data.len);
    TEST_ASSERT_EQUAL_MEMORY(data, p->u.pong.data.data, 4);

    static uint8_t big[65];
    m.u.ping.data.data = big;
    m.u.ping.data.len = 65;
    send_msg(GATT, &m);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_LENGTH, result()->code);

    fake.link.phy = 2;
    fake.link.att_mtu = 247;
    fake.link.flags = 0xFFFF; /* only hardware bits pass */
    simple(PROTO_STATUS_GET, 7);
    receive(GATT);
    const proto_msg_t *s = find(PROTO_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(GATT, s->u.status.transport);
    TEST_ASSERT_EQUAL_UINT16(247, s->u.status.att_mtu);
    TEST_ASSERT_EQUAL_UINT16(0x0017 | 0x0040, s->u.status.flags); /* pairing window open */

    /* A device -> host type sent to the device, and an undefined type. */
    simple(PROTO_PTT_STATUS, 8);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNKNOWN_TYPE, result()->code);
    TEST_ASSERT_EQUAL_UINT8(PROTO_PTT_STATUS, result()->request_type);
    /* Type 0x7E, token 10. */
    uint8_t frame[4] = {0x7e, 0x0a, 0, 0};
    uint16_t crc = proto_crc16(frame, 2);
    frame[2] = (uint8_t)crc;
    frame[3] = (uint8_t)(crc >> 8);
    uint8_t wire[8];
    size_t n = proto_cobs_encode(frame, 4, wire, sizeof(wire));
    wire[n++] = 0;
    app_rx(&app, GATT, wire, n, now_us);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNKNOWN_TYPE, result()->code);
    TEST_ASSERT_EQUAL_UINT8(10, find(PROTO_RESULT)->token);
}

static void test_token_zero_gets_no_reply_on_success(void)
{
    open_session();
    simple(PROTO_KEEPALIVE, 0);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
    simple(PROTO_KEEPALIVE, 12);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
    TEST_ASSERT_EQUAL_UINT8(12, find(PROTO_RESULT)->token);
    /* Errors are still reported with token 0. */
    simple(PROTO_TONE_CANCEL, 0);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    TEST_ASSERT_EQUAL_UINT8(0, find(PROTO_RESULT)->token);
}

static void test_frame_errors_rate_limited(void)
{
    open_session();
    const uint8_t bad[] = {0x06, 0x30, 0x01, 0x01, 0x29, 0x2b, 0x00}; /* bad CRC */
    app_rx(&app, GATT, bad, sizeof(bad), now_us);
    app_rx(&app, GATT, bad, sizeof(bad), now_us);
    receive(GATT);
    TEST_ASSERT_EQUAL_size_t(1, ngot);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_FRAME, result()->code);
    TEST_ASSERT_EQUAL_UINT8(0, result()->request_type);
    advance_ms(1000);
    app_rx(&app, GATT, bad, sizeof(bad), now_us);
    receive(GATT);
    TEST_ASSERT_EQUAL_size_t(1, ngot);
    TEST_ASSERT_EQUAL_UINT16(3, app.frame_errors);
}

/* --- PTT through the protocol --- */

static void test_ptt_set_reply_and_keepalive_timeout(void)
{
    open_session();
    send_ptt(1, 24);
    receive(GATT);
    const proto_msg_t *s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(24, s->token);
    TEST_ASSERT_EQUAL_UINT8(1, s->u.ptt_status.state);
    TEST_ASSERT_EQUAL_UINT32(300, s->u.ptt_status.remaining_s);
    TEST_ASSERT_EQUAL_size_t(1, ngot); /* exactly one reply, no duplicate */
    TEST_ASSERT_TRUE(fake.closure);

    for (int i = 0; i < 60; i++) {
        simple(PROTO_KEEPALIVE, 0);
        advance_ms(1000);
    }
    TEST_ASSERT_TRUE(fake.closure);
    advance_ms(3000); /* host hangs */
    TEST_ASSERT_FALSE(fake.closure);
    receive(GATT);
    s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(0, s->token);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, s->u.ptt_status.reason);

    send_ptt(2, 25);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_VALUE, result()->code);
}

static void test_ptt_set_token_zero(void)
{
    open_session();
    send_ptt(1, 0);
    receive(GATT);
    TEST_ASSERT_NOT_NULL(find(PROTO_PTT_STATUS)); /* a change: reported with token 0 */
    send_ptt(1, 0);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT)); /* no change, token 0: nothing */
}

static void test_link_loss_turns_ptt_off(void)
{
    open_session();
    send_ptt(1, 1);
    TEST_ASSERT_TRUE(fake.closure);
    app_transport_closed(&app, GATT, now_us);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_EQUAL_UINT8(0, app.active);
    /* Messages after the link loss are ignored until the next HELLO. */
    send_ptt(1, 2);
    TEST_ASSERT_FALSE(fake.closure);
    app_transport_closed(&app, HAL_TRANSPORT_L2CAP, now_us); /* not the active one: no effect */
    app_transport_closed(&app, 0, now_us);
}

static void test_new_hello_restarts_session(void)
{
    open_session();
    send_ptt(1, 1);
    receive(GATT);
    TEST_ASSERT_TRUE(fake.closure);
    /* HELLO on L2CAP takes over; PTT goes off first (LINK_LOST on the old one). */
    hello(HAL_TRANSPORT_L2CAP, PROTO_VERSION_MINOR);
    TEST_ASSERT_FALSE(fake.closure);
    receive(GATT);
    const proto_msg_t *s = find(PROTO_PTT_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LINK_LOST, s->u.ptt_status.reason);
    receive(HAL_TRANSPORT_L2CAP);
    TEST_ASSERT_NOT_NULL(find(PROTO_DEVICE_INFO));
    TEST_ASSERT_EQUAL_UINT8(HAL_TRANSPORT_L2CAP, app.active);
    /* The old transport is no longer served. */
    send_ptt(1, 3);
    TEST_ASSERT_FALSE(fake.closure);
}

static void test_serial_port_and_modem_lines(void)
{
    open_session();
    proto_msg_t m = {.type = PROTO_MODEM_LINES, .token = 0};
    m.u.modem_lines.port = 0;
    m.u.modem_lines.lines = PTT_LINE_RTS;
    send_msg(GATT, &m);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_STATE, result()->code); /* port not open */

    proto_msg_t o = {.type = PROTO_SERIAL_OPEN, .token = 20};
    o.u.serial_open.port = 0;
    o.u.serial_open.open = 1;
    send_msg(GATT, &o);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);

    m.u.modem_lines.lines = 0;
    send_msg(GATT, &m);
    m.u.modem_lines.lines = PTT_LINE_RTS;
    send_msg(GATT, &m);
    TEST_ASSERT_TRUE(fake.closure);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_LINE, find(PROTO_PTT_STATUS)->u.ptt_status.sources);

    proto_msg_t d = {.type = PROTO_CAT_DATA, .token = 0};
    const uint8_t cat[] = "FA;";
    d.u.cat_data.port = 0;
    d.u.cat_data.data.data = cat;
    d.u.cat_data.data.len = 3;
    send_msg(GATT, &d);
    TEST_ASSERT_EQUAL_MEMORY("FA;", fake.serial_out, 3);
    receive(GATT);
    const proto_msg_t *cr = find(PROTO_CAT_CREDIT);
    TEST_ASSERT_NOT_NULL(cr);
    TEST_ASSERT_EQUAL_UINT16(3, cr->u.cat_credit.credit);

    proto_msg_t s = {.type = PROTO_SERIAL_SET, .token = 22};
    s.u.serial_set.port = 0;
    s.u.serial_set.baud = 38400;
    s.u.serial_set.data_bits = 8;
    send_msg(GATT, &s);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
    TEST_ASSERT_EQUAL_UINT32(38400, fake.serial_cfg[0].baud);

    /* Radio -> host bytes: batched for CAT_BATCH_US (SPEC §7.3). */
    const uint8_t reply[] = "FA00014074000;";
    app_cat_from_radio(&app, 0, reply, 5, now_us);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
    TEST_ASSERT_EQUAL_UINT64(now_us + CAT_BATCH_US, app_next_deadline_us(&app, now_us));
    advance_ms(1);
    app_cat_from_radio(&app, 0, reply + 5, sizeof(reply) - 6, now_us);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
    advance_ms(1);
    receive(GATT);
    const proto_msg_t *cd = find(PROTO_CAT_DATA);
    TEST_ASSERT_NOT_NULL(cd);
    TEST_ASSERT_EQUAL_UINT16(sizeof(reply) - 1, cd->u.cat_data.data.len);
    TEST_ASSERT_EQUAL_MEMORY(reply, cd->u.cat_data.data.data, sizeof(reply) - 1);
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, app_next_deadline_us(&app, now_us));
    app_cat_from_radio(&app, 1, reply, 3, now_us); /* port not open: dropped */
    advance_ms(5);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));

    /* Session end closes the port and drops PTT. */
    app_transport_closed(&app, GATT, now_us);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_FALSE(fake.serial_open[0]);
}

static void test_cat_from_radio_chunks_to_peer_max_payload(void)
{
    proto_msg_t m = {.type = PROTO_HELLO, .token = 1};
    m.u.hello.proto_minor = PROTO_VERSION_MINOR;
    m.u.hello.max_payload = 100; /* below 256: raised to 256 */
    send_msg(GATT, &m);
    TEST_ASSERT_EQUAL_UINT16(256, app.peer_max_payload);
    proto_msg_t o = {.type = PROTO_SERIAL_OPEN, .token = 0};
    o.u.serial_open.open = 1;
    send_msg(GATT, &o);
    receive(GATT);
    static uint8_t data[600];
    memset(data, 0x55, sizeof(data));
    app_cat_from_radio(&app, 0, data, sizeof(data), now_us);
    receive(GATT); /* full frames go at once */
    TEST_ASSERT_EQUAL_size_t(2, ngot);
    TEST_ASSERT_EQUAL_UINT16(255, got[0].u.cat_data.data.len);
    TEST_ASSERT_EQUAL_UINT16(255, got[1].u.cat_data.data.len);
    advance_ms(2); /* the rest when the batching window ends */
    receive(GATT);
    TEST_ASSERT_EQUAL_size_t(1, ngot);
    TEST_ASSERT_EQUAL_UINT16(90, got[0].u.cat_data.data.len);
}

static void open_port0(void)
{
    open_session();
    proto_msg_t o = {.type = PROTO_SERIAL_OPEN, .token = 0};
    o.u.serial_open.port = 0;
    o.u.serial_open.open = 1;
    send_msg(GATT, &o);
    receive(GATT);
}

static void send_cat(const uint8_t *data, uint16_t len)
{
    proto_msg_t d = {.type = PROTO_CAT_DATA, .token = 0};
    d.u.cat_data.port = 0;
    d.u.cat_data.data.data = data;
    d.u.cat_data.data.len = len;
    send_msg(GATT, &d);
}

static uint32_t credit_received(void)
{
    uint32_t total = 0;
    for (size_t i = 0; i < ngot; i++) {
        if (got[i].type == PROTO_CAT_CREDIT) {
            TEST_ASSERT_EQUAL_UINT8(0, got[i].u.cat_credit.port);
            total += got[i].u.cat_credit.credit;
        }
    }
    return total;
}

/* A slow radio side: bytes within the credit are kept and sent as the UART
 * takes them, and the credit comes back only for bytes that left (§7.4). */
static void test_cat_credit_returned_as_bytes_leave(void)
{
    open_port0();
    static uint8_t data[256];
    for (size_t i = 0; i < sizeof(data); i++) {
        data[i] = (uint8_t)(i ^ 0xA5u);
    }
    fake.serial_accept = 100;
    send_cat(data, 200);
    receive(GATT);
    TEST_ASSERT_NULL(find(PROTO_RESULT)); /* no OVERFLOW: within the credit */
    TEST_ASSERT_EQUAL_UINT32(100, credit_received());
    TEST_ASSERT_EQUAL_size_t(100, fake.serial_out_len);
    TEST_ASSERT_EQUAL_UINT64(now_us + CAT_PUMP_US, app_next_deadline_us(&app, now_us));
    advance_ms(1);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT32(100, credit_received());
    TEST_ASSERT_EQUAL_size_t(200, fake.serial_out_len);
    TEST_ASSERT_EQUAL_MEMORY(data, fake.serial_out, 200); /* byte-exact, in order */
    advance_ms(1);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT)); /* nothing more to return */
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, app_next_deadline_us(&app, now_us));
    TEST_ASSERT_EQUAL_UINT16(0, app.cat.overflows);
}

static void test_cat_over_credit_reports_overflow(void)
{
    open_port0();
    static uint8_t data[300];
    memset(data, 0x3B, sizeof(data));
    fake.serial_accept = 0; /* radio side stalled */
    send_cat(data, 250);
    receive(GATT);
    TEST_ASSERT_EQUAL_size_t(0, ngot);
    send_cat(data, 10); /* 6 fit in the 256-byte credit, 4 don't */
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_OVERFLOW, result()->code);
    TEST_ASSERT_EQUAL_UINT16(1, app.cat.overflows);
    fake.serial_accept = (size_t)-1;
    advance_ms(1);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT32(256, credit_received());
    TEST_ASSERT_EQUAL_size_t(256, fake.serial_out_len);
}

static void test_cat_session_end_discards_queues(void)
{
    open_port0();
    const uint8_t data[] = "FA;";
    fake.serial_accept = 0;
    send_cat(data, 3);
    app_cat_from_radio(&app, 0, data, 3, now_us);
    app_transport_closed(&app, GATT, now_us);
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, app_next_deadline_us(&app, now_us));
    fake.serial_accept = (size_t)-1;
    advance_ms(5);
    TEST_ASSERT_EQUAL_size_t(0, fake.serial_out_len);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
    /* No session: the radio's bytes go nowhere. */
    app_cat_from_radio(&app, 0, data, 3, now_us);
    TEST_ASSERT_EQUAL_UINT16(0, app.cat.port[0].rxq_len);
}

/* --- Configuration (SPEC §6) --- */

static void config_set(uint8_t flags, uint8_t key, uint8_t selector, uint8_t token,
                       void (*fill)(proto_config_t *))
{
    proto_msg_t m = {.type = PROTO_CONFIG_SET, .token = token};
    m.u.config_set.flags = flags;
    m.u.config_set.config.key = key;
    m.u.config_set.config.selector = selector;
    fill(&m.u.config_set.config);
    send_msg(GATT, &m);
}

static void fill_ble_3(proto_config_t *c)
{
    c->value.ble_tx_power.dbm = 3;
}
static void fill_ble_20(proto_config_t *c)
{
    c->value.ble_tx_power.dbm = 20;
}
static void fill_targets_rts(proto_config_t *c)
{
    c->value.ptt_targets.targets = PTT_TARGET_CLOSURE;
    c->value.ptt_targets.usb_port = 0;
}
static void fill_line_map_dtr(proto_config_t *c)
{
    c->value.line_map.rts_action = PTT_ACT_PTT;
    c->value.line_map.dtr_action = PTT_ACT_PTT;
}
static void fill_max_tx_0(proto_config_t *c)
{
    c->value.max_tx_s.s = 0;
}
static void fill_jack_mode_0(proto_config_t *c)
{
    c->value.serial_jack_mode.mode = 0;
}
static void fill_host_mode_1(proto_config_t *c)
{
    c->value.host_mode.mode = 1;
}

static void test_config_get_set_and_ble_power(void)
{
    open_session();
    proto_msg_t g = {.type = PROTO_CONFIG_GET, .token = 7};
    g.u.config_get.key = PROTO_KEY_MAX_TX_S;
    send_msg(GATT, &g);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT32(300, find(PROTO_CONFIG)->u.config.config.value.max_tx_s.s);
    g.u.config_get.key = 0x7F;
    send_msg(GATT, &g);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_VALUE, result()->code);

    config_set(0, PROTO_KEY_BLE_TX_POWER, 0, 18, fill_ble_3);
    receive(GATT);
    TEST_ASSERT_EQUAL_INT8(3, find(PROTO_CONFIG)->u.config.config.value.ble_tx_power.dbm);
    TEST_ASSERT_EQUAL_INT8(3, fake.ble_dbm);
    config_set(0, PROTO_KEY_BLE_TX_POWER, 0, 19, fill_ble_20);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_OUT_OF_RANGE, result()->code);
    TEST_ASSERT_EQUAL_INT8(3, fake.ble_dbm); /* unchanged: nothing raises the power */

    /* Token 0: no reply on success. */
    config_set(0, PROTO_KEY_BLE_TX_POWER, 0, 0, fill_ble_3);
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
}

static void test_config_persist_and_rate_limit(void)
{
    open_session();
    config_set(1, PROTO_KEY_MAX_TX_S, 0, 10, fill_max_tx_0);
    receive(GATT);
    TEST_ASSERT_NOT_NULL(find(PROTO_CONFIG));
    TEST_ASSERT_EQUAL_UINT(1, fake.saves);
    config_set(1, PROTO_KEY_BLE_TX_POWER, 0, 11, fill_ble_3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_RATE_LIMITED, result()->code);
    TEST_ASSERT_EQUAL_UINT(1, fake.saves);
    advance_ms(1000);
    config_set(1, PROTO_KEY_BLE_TX_POWER, 0, 12, fill_ble_3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT(2, fake.saves);
    /* A storage failure is INTERNAL. */
    advance_ms(1000);
    fake.save_result = -1;
    config_set(1, PROTO_KEY_BLE_TX_POWER, 0, 13, fill_ble_3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_INTERNAL, result()->code);

    /* The stored configuration is loaded at the next boot. */
    fake.save_result = 0;
    advance_ms(1000);
    config_set(1, PROTO_KEY_MAX_TX_S, 0, 14, fill_max_tx_0);
    app_init(&app, &board, HAL_LINK_BLUETOOTH, PTT_R_BOOT, now_us);
    TEST_ASSERT_EQUAL_UINT32(0, app.cfg.max_tx_s);
    TEST_ASSERT_EQUAL_INT8(3, app.cfg.ble_tx_power);
    TEST_ASSERT_EQUAL_INT8(3, fake.ble_dbm);
    TEST_ASSERT_EQUAL_UINT32(0, app.ptt.cfg.max_tx_s);
}

static void test_corrupt_storage_uses_defaults(void)
{
    memset(fake.stored, 0xA5, 100);
    fake.stored_len = 100;
    app_init(&app, &board, HAL_LINK_BLUETOOTH, PTT_R_BOOT, now_us);
    TEST_ASSERT_EQUAL_UINT32(300, app.cfg.max_tx_s);
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, fake.ble_dbm);
}

static void test_config_reset(void)
{
    open_session();
    config_set(0, PROTO_KEY_BLE_TX_POWER, 0, 1, fill_ble_3);
    proto_msg_t r = {.type = PROTO_CONFIG_RESET, .token = 19};
    r.u.config_reset.flags = 1;
    send_msg(GATT, &r);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, fake.ble_dbm);
    TEST_ASSERT_EQUAL_UINT(1, fake.saves);
    send_msg(GATT, &r); /* second persisted write within a second */
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_RATE_LIMITED, result()->code);
    r.u.config_reset.flags = 0;
    send_msg(GATT, &r);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
}

static void test_ptt_config_change_while_keyed_releases(void)
{
    open_session();
    send_ptt(1, 1);
    TEST_ASSERT_TRUE(fake.closure);
    config_set(0, PROTO_KEY_LINE_MAP, 0, 2, fill_line_map_dtr);
    TEST_ASSERT_FALSE(fake.closure);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MODE_CHANGE, find(PROTO_PTT_STATUS)->u.ptt_status.reason);
    TEST_ASSERT_NOT_NULL(find(PROTO_CONFIG));
    send_ptt(1, 3);
    config_set(0, PROTO_KEY_PTT_TARGETS, 0, 4, fill_targets_rts);
    TEST_ASSERT_TRUE(fake.closure); /* same targets: no change, stays keyed */
}

static void test_non_ptt_config_never_asserts(void)
{
    open_session();
    unsigned on_before = fake.closure_on_count;
    config_set(0, PROTO_KEY_SERIAL_JACK_MODE, 0, 1, fill_jack_mode_0);
    config_set(0, PROTO_KEY_HOST_MODE, 0, 2, fill_host_mode_1);
    config_set(0, PROTO_KEY_LINE_MAP, 0, 3, fill_line_map_dtr);
    TEST_ASSERT_EQUAL_UINT(on_before, fake.closure_on_count);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_TRUE(fake.jack_mode_sets >= 2);
}

/* --- Other messages --- */

static void test_time_audio_tone(void)
{
    open_session();
    proto_msg_t t = {.type = PROTO_TIME_REQ, .token = 30};
    t.u.time_req.host_t1 = 1000000;
    fake.time_us = now_us + 120;
    send_msg(GATT, &t);
    receive(GATT);
    const proto_msg_t *r = find(PROTO_TIME_RESP);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL_UINT64(1000000, r->u.time_resp.host_t1);
    TEST_ASSERT_EQUAL_UINT64(now_us, r->u.time_resp.device_t2);
    TEST_ASSERT_EQUAL_UINT64(now_us + 120, r->u.time_resp.device_t3);

    proto_msg_t s = {.type = PROTO_TIME_SET, .token = 31};
    s.u.time_set.device_time_us = now_us - 10;
    s.u.time_set.utc_us = 1790000000000000ull;
    send_msg(GATT, &s);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
    simple(PROTO_STATUS_GET, 32);
    receive(GATT);
    TEST_ASSERT_TRUE(find(PROTO_STATUS)->u.status.flags & (1u << 3)); /* UTC fresh */

    proto_msg_t a = {.type = PROTO_AUDIO_START, .token = 26};
    a.u.audio_start.direction = 1;
    send_msg(GATT, &a);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    a.type = PROTO_AUDIO_STOP;
    send_msg(GATT, &a);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    proto_msg_t f = {.type = PROTO_AUDIO_FRAME, .token = 0};
    send_msg(GATT, &f);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    const uint8_t tones[] = {1, 2};
    proto_msg_t d = {.type = PROTO_TONE_DATA, .token = 33};
    d.u.tone_data.tones.data = tones;
    d.u.tone_data.tones.len = 2;
    send_msg(GATT, &d);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    simple(PROTO_TONE_SETUP, 34);
    simple(PROTO_TONE_START, 35);
    receive(GATT);
    TEST_ASSERT_EQUAL_size_t(2, ngot);
}

/* --- Wired mode (SPEC §14) --- */

static void auth(uint8_t transport, uint8_t token)
{
    proto_msg_t m = {.type = PROTO_AUTH, .token = token};
    memset(m.u.auth.host_token, 0x5A, sizeof(m.u.auth.host_token));
    send_msg(transport, &m);
}

/* --- Security (SPEC §15); the stores are #64 and fail closed --- */

static void test_wired_host_needs_approval(void)
{
    fake_reset();
    app_init(&app, &board, HAL_LINK_WIRED, PTT_R_BOOT, now_us);
    TEST_ASSERT_TRUE(app_pairing_open(&app)); /* power-on opens the window in wired mode too */
    hello(HAL_TRANSPORT_TCP, PROTO_VERSION_MINOR);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_FALSE(app.authorized);
    /* Only HELLO, CAPS_GET, PING and AUTH work. */
    proto_msg_t k = {.type = PROTO_PTT_SET, .token = 5};
    k.u.ptt_set.state = 1;
    send_msg(HAL_TRANSPORT_TCP, &k);
    TEST_ASSERT_FALSE(fake.closure);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_NOT_AUTHORIZED, result()->code);
    proto_msg_t c = {.type = PROTO_CAPS_GET, .token = 6};
    send_msg(HAL_TRANSPORT_TCP, &c);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_NOT_NULL(find(PROTO_CAPS));
    /* AUTH: the host store is #64, so no host is approved yet. */
    auth(HAL_TRANSPORT_TCP, 7);
    receive(HAL_TRANSPORT_TCP);
    const proto_msg_t *s = find(PROTO_AUTH_STATUS);
    TEST_ASSERT_NOT_NULL(s);
    TEST_ASSERT_EQUAL_UINT8(1, s->u.auth_status.state);
    TEST_ASSERT_EQUAL_UINT8(0xFF, s->u.auth_status.slot);
    TEST_ASSERT_FALSE(app.authorized);
    advance_ms(120000); /* window closed */
    auth(HAL_TRANSPORT_TCP, 8);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(1, find(PROTO_AUTH_STATUS)->u.auth_status.state);
}

static void test_bluetooth_host_is_authorized_by_its_bond(void)
{
    open_session();
    TEST_ASSERT_TRUE(app.authorized);
    auth(GATT, 3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(0, find(PROTO_AUTH_STATUS)->u.auth_status.state);
}

static void test_trust_list_and_factory_reset(void)
{
    open_session();
    simple(PROTO_TRUST_LIST_GET, 1);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code); /* #64 */
    proto_msg_t r = {.type = PROTO_TRUST_REMOVE, .token = 2};
    r.u.trust_remove.kind = 1;
    send_msg(GATT, &r);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code);
    proto_msg_t f = {.type = PROTO_FACTORY_RESET, .token = 3};
    f.u.factory_reset.confirm = 0x12345678;
    send_msg(GATT, &f);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_VALUE, result()->code);
    f.u.factory_reset.confirm = PROTO_FACTORY_RESET_CONFIRM;
    send_msg(GATT, &f);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, result()->code); /* #64 */
    fake_sec.factory_reset_result = PROTO_OK;
    send_msg(GATT, &f);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
}

static void test_wired_host_approval_paths(void)
{
    fake_reset();
    app_init(&app, &board, HAL_LINK_WIRED, PTT_R_BOOT, now_us);
    hello(HAL_TRANSPORT_TCP, PROTO_VERSION_MINOR);
    /* No free slot. */
    fake_sec.approve_result = PROTO_ERR_OVERFLOW;
    auth(HAL_TRANSPORT_TCP, 1);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(2, find(PROTO_AUTH_STATUS)->u.auth_status.state);
    TEST_ASSERT_FALSE(app.authorized);
    /* Approved inside the window: the window closes. */
    fake_sec.approve_result = PROTO_OK;
    auth(HAL_TRANSPORT_TCP, 2);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(0, find(PROTO_AUTH_STATUS)->u.auth_status.state);
    TEST_ASSERT_TRUE(app.authorized);
    TEST_ASSERT_FALSE(app_pairing_open(&app));
    /* A new HELLO needs a new AUTH; a known host is approved without the window. */
    hello(HAL_TRANSPORT_TCP, PROTO_VERSION_MINOR);
    TEST_ASSERT_FALSE(app.authorized);
    fake_sec.approved = true;
    unsigned calls = fake_sec.approve_calls;
    auth(HAL_TRANSPORT_TCP, 3);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(0, find(PROTO_AUTH_STATUS)->u.auth_status.state);
    TEST_ASSERT_TRUE(app.authorized);
    TEST_ASSERT_EQUAL_UINT(calls, fake_sec.approve_calls);
}

static void fill_port_lock(proto_config_t *c, uint8_t level)
{
    c->value.wired_port_lock.level = level;
}
static void fill_lock_1(proto_config_t *c)
{
    fill_port_lock(c, 1);
}
static void fill_lock_3(proto_config_t *c)
{
    fill_port_lock(c, 3);
}
static void fill_lock_4(proto_config_t *c)
{
    fill_port_lock(c, 4);
}

static void test_wired_port_lock(void)
{
    /* Wired serial profile: the bridge port's RTS keys PTT with no session. */
    app_host_link_changed(&app, HAL_LINK_WIRED, now_us);
    app_native_lines(&app, 0, 0, now_us);
    app_native_lines(&app, 0, PTT_LINE_RTS, now_us);
    TEST_ASSERT_TRUE(fake.closure);
    /* An authorized (here: Bluetooth-style) session sets the lock. */
    app.host_link = HAL_LINK_BLUETOOTH;
    open_session();
    config_set(0, PROTO_KEY_WIRED_PORT_LOCK, 0, 1, fill_lock_4);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_BAD_VALUE, result()->code);
    config_set(0, PROTO_KEY_WIRED_PORT_LOCK, 0, 2, fill_lock_3);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_OUT_OF_RANGE, result()->code); /* no radio-port disconnect */
    TEST_ASSERT_TRUE(fake.closure);
    config_set(0, PROTO_KEY_WIRED_PORT_LOCK, 0, 3, fill_lock_1);
    TEST_ASSERT_FALSE(fake.closure); /* the native line's PTT is released first */
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(1, find(PROTO_CONFIG)->u.config.config.value.wired_port_lock.level);
    /* Native lines no longer key. */
    app.host_link = HAL_LINK_WIRED;
    app_native_lines(&app, 0, 0, now_us);
    app_native_lines(&app, 0, PTT_LINE_RTS, now_us);
    TEST_ASSERT_FALSE(fake.closure);
}

static void test_wired_mode_rules(void)
{
    fake_reset();
    app_init(&app, &board, HAL_LINK_WIRED, PTT_R_BOOT, now_us);
    hello(HAL_TRANSPORT_CONTROL, PROTO_VERSION_MINOR);
    receive(HAL_TRANSPORT_CONTROL);
    app.authorized = true; /* as if approved (the host store is #64) */
    /* Audio messages: UNSUPPORTED (wired audio is USB Audio Class). */
    proto_msg_t a = {.type = PROTO_AUDIO_START, .token = 1};
    send_msg(HAL_TRANSPORT_CONTROL, &a);
    proto_msg_t f = {.type = PROTO_AUDIO_FRAME, .token = 0};
    send_msg(HAL_TRANSPORT_CONTROL, &f);
    /* Serial messages: only over the USB network in the network profile. */
    proto_msg_t o = {.type = PROTO_SERIAL_OPEN, .token = 2};
    o.u.serial_open.open = 1;
    send_msg(HAL_TRANSPORT_CONTROL, &o);
    receive(HAL_TRANSPORT_CONTROL);
    TEST_ASSERT_EQUAL_size_t(3, ngot);
    for (size_t i = 0; i < ngot; i++) {
        TEST_ASSERT_EQUAL_UINT8(PROTO_ERR_UNSUPPORTED, got[i].u.result.code);
    }
    app.cfg.wired_profile = 0;
    hello(HAL_TRANSPORT_TCP, PROTO_VERSION_MINOR);
    app.authorized = true;
    send_msg(HAL_TRANSPORT_TCP, &o);
    receive(HAL_TRANSPORT_TCP);
    TEST_ASSERT_EQUAL_UINT8(PROTO_OK, result()->code);
    /* USB reset ends the wired session. */
    proto_msg_t k = {.type = PROTO_PTT_SET, .token = 5};
    k.u.ptt_set.state = 1;
    send_msg(HAL_TRANSPORT_TCP, &k);
    TEST_ASSERT_TRUE(fake.closure);
    app_usb_reset(&app, now_us);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_EQUAL_UINT8(0, app.active);
}

static void test_native_lines_only_in_wired_mode(void)
{
    app_native_lines(&app, PTT_PORT_CONTROL, 0, now_us);
    app_native_lines(&app, PTT_PORT_CONTROL, PTT_LINE_RTS, now_us);
    TEST_ASSERT_FALSE(fake.closure); /* Bluetooth mode: ignored */
    app_host_link_changed(&app, HAL_LINK_WIRED, now_us);
    app_native_lines(&app, 3, PTT_LINE_RTS, now_us); /* not a device serial port */
    app_native_lines(&app, PTT_PORT_CONTROL, 0, now_us);
    app_native_lines(&app, PTT_PORT_CONTROL, PTT_LINE_RTS, now_us);
    TEST_ASSERT_TRUE(fake.closure);
    advance_ms(10000); /* no keepalive needed */
    TEST_ASSERT_TRUE(fake.closure);
    app_usb_reset(&app, now_us); /* unplug / suspend / reset */
    TEST_ASSERT_FALSE(fake.closure);
}

static void test_host_link_change_turns_ptt_off(void)
{
    open_session();
    send_ptt(1, 1);
    TEST_ASSERT_TRUE(fake.closure);
    app_pairing_button(&app, now_us);
    TEST_ASSERT_TRUE(app_pairing_open(&app));
    app_host_link_changed(&app, HAL_LINK_WIRED, now_us);
    TEST_ASSERT_FALSE(fake.closure);
    TEST_ASSERT_EQUAL_UINT8(0, app.active);
    TEST_ASSERT_FALSE(app_pairing_open(&app)); /* a host-mode change closes the window */
    app_pairing_button(&app, now_us);          /* the button opens it in wired mode too */
    TEST_ASSERT_TRUE(app_pairing_open(&app));
}

static void test_fault_turns_ptt_off(void)
{
    open_session();
    send_ptt(1, 1);
    app_fault(&app, now_us);
    TEST_ASSERT_FALSE(fake.closure);
    receive(GATT);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, find(PROTO_PTT_STATUS)->u.ptt_status.reason);
}

/* --- Pairing window (SPEC §13.5) --- */

static void test_pairing_window_local_only(void)
{
    uint8_t info[PROTO_INFO_LEN];
    TEST_ASSERT_TRUE(app_pairing_open(&app)); /* opened by power-on */
    app_info(&app, info);
    TEST_ASSERT_EQUAL_HEX8(PROTO_INFO_F_PAIRING_OPEN, info[3]);
    advance_ms(120000);
    TEST_ASSERT_FALSE(app_pairing_open(&app));
    app_info(&app, info);
    TEST_ASSERT_EQUAL_HEX8(0, info[3]);

    /* No protocol message opens it: send every type with junk payloads. */
    open_session();
    for (unsigned type = 1; type < 0x100; type++) {
        uint8_t frame[8] = {(uint8_t)type, 0x01, 0x01, 0x01};
        uint16_t crc = proto_crc16(frame, 4);
        frame[4] = (uint8_t)crc;
        frame[5] = (uint8_t)(crc >> 8);
        uint8_t wire[16];
        size_t n = proto_cobs_encode(frame, 6, wire, sizeof(wire));
        wire[n++] = 0;
        app_rx(&app, GATT, wire, n, now_us);
        TEST_ASSERT_FALSE(app_pairing_open(&app));
    }
    fake_clear_tx();

    app_pairing_button(&app, now_us);
    TEST_ASSERT_TRUE(app_pairing_open(&app));
    app_bond_added(&app);
    TEST_ASSERT_FALSE(app_pairing_open(&app));
}

static void test_info_and_led(void)
{
    uint8_t info[PROTO_INFO_LEN];
    board_t coc = board;
    coc.l2cap_psm = 0x0080;
    app.board = &coc;
    app_info(&app, info);
    TEST_ASSERT_EQUAL_HEX8(PROTO_INFO_F_L2CAP | PROTO_INFO_F_PAIRING_OPEN, info[3]);
    TEST_ASSERT_EQUAL_HEX8(0x80, info[4]);
    app.board = &board;

    /* Pairing window: 4 Hz. */
    TEST_ASSERT_TRUE(app_led(&app, 0));
    TEST_ASSERT_FALSE(app_led(&app, 125000));
    advance_ms(120000);
    /* Idle: 1 Hz. */
    TEST_ASSERT_TRUE(app_led(&app, 0));
    TEST_ASSERT_TRUE(app_led(&app, 499000));
    TEST_ASSERT_FALSE(app_led(&app, 500000));
    /* Session: steady on. */
    open_session();
    TEST_ASSERT_TRUE(app_led(&app, 500000));
}

static void test_max_payload_clamped_and_other_transport_errors(void)
{
    proto_msg_t m = {.type = PROTO_HELLO, .token = 1};
    m.u.hello.proto_minor = PROTO_VERSION_MINOR;
    m.u.hello.max_payload = 4096;
    send_msg(GATT, &m);
    TEST_ASSERT_EQUAL_UINT16(PROTO_MAX_PAYLOAD, app.peer_max_payload);
    receive(GATT);
    /* A frame error on a transport without the session: counted, not answered. */
    const uint8_t bad[] = {0x06, 0x30, 0x01, 0x01, 0x29, 0x2b, 0x00};
    app_rx(&app, HAL_TRANSPORT_L2CAP, bad, sizeof(bad), now_us);
    TEST_ASSERT_EQUAL_UINT16(1, app.frame_errors);
    TEST_ASSERT_EQUAL_size_t(0, receive(HAL_TRANSPORT_L2CAP));
    TEST_ASSERT_EQUAL_size_t(0, receive(GATT));
}

static void test_bad_transport_numbers_ignored(void)
{
    const uint8_t zero = 0;
    app_rx(&app, 0, &zero, 1, now_us);
    app_rx(&app, HAL_TRANSPORT_MAX + 1, &zero, 1, now_us);
    app_transport_closed(&app, HAL_TRANSPORT_MAX + 1, now_us);
    TEST_ASSERT_EQUAL_UINT8(0, app.active);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hello_answers_device_info);
    RUN_TEST(test_boot_status_reported_to_first_session);
    RUN_TEST(test_watchdog_reset_reported_to_next_session);
    RUN_TEST(test_pending_watchdog_not_hidden_by_later_status);
    RUN_TEST(test_version_mismatch);
    RUN_TEST(test_requests_before_hello_are_ignored);
    RUN_TEST(test_hello_errors_answered_on_that_transport);
    RUN_TEST(test_caps);
    RUN_TEST(test_ping_status_and_unknown);
    RUN_TEST(test_token_zero_gets_no_reply_on_success);
    RUN_TEST(test_frame_errors_rate_limited);
    RUN_TEST(test_ptt_set_reply_and_keepalive_timeout);
    RUN_TEST(test_ptt_set_token_zero);
    RUN_TEST(test_link_loss_turns_ptt_off);
    RUN_TEST(test_new_hello_restarts_session);
    RUN_TEST(test_serial_port_and_modem_lines);
    RUN_TEST(test_cat_from_radio_chunks_to_peer_max_payload);
    RUN_TEST(test_cat_credit_returned_as_bytes_leave);
    RUN_TEST(test_cat_over_credit_reports_overflow);
    RUN_TEST(test_cat_session_end_discards_queues);
    RUN_TEST(test_config_get_set_and_ble_power);
    RUN_TEST(test_config_persist_and_rate_limit);
    RUN_TEST(test_corrupt_storage_uses_defaults);
    RUN_TEST(test_config_reset);
    RUN_TEST(test_ptt_config_change_while_keyed_releases);
    RUN_TEST(test_non_ptt_config_never_asserts);
    RUN_TEST(test_time_audio_tone);
    RUN_TEST(test_wired_host_needs_approval);
    RUN_TEST(test_bluetooth_host_is_authorized_by_its_bond);
    RUN_TEST(test_trust_list_and_factory_reset);
    RUN_TEST(test_wired_host_approval_paths);
    RUN_TEST(test_wired_port_lock);
    RUN_TEST(test_wired_mode_rules);
    RUN_TEST(test_native_lines_only_in_wired_mode);
    RUN_TEST(test_host_link_change_turns_ptt_off);
    RUN_TEST(test_fault_turns_ptt_off);
    RUN_TEST(test_pairing_window_local_only);
    RUN_TEST(test_info_and_led);
    RUN_TEST(test_max_payload_clamped_and_other_transport_errors);
    RUN_TEST(test_bad_transport_numbers_ignored);
    return UNITY_END();
}

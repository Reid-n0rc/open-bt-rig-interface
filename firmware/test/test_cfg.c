/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Configuration keys (SPEC §6.1): defaults, ranges and storage.
 */
#include <string.h>

#include "ble_power.h"
#include "cfg.h"
#include "unity.h"

static const board_t board = {
    .variant = 'D',
    .hw_revision = 'A',
    .features = PROTO_F_CAT | PROTO_F_PTT_CLOSURE | PROTO_F_BLE_TX_POWER,
    .serial_modes = 0x01,
    .serial_min_baud = 4800,
    .serial_max_baud = 115200,
    .cat_tx_buffer = 256,
    .ptt_outputs = PTT_TARGET_CLOSURE | PTT_TARGET_RTS | PTT_TARGET_DTR,
    .pairing_triggers = BOARD_PAIR_POWER_ON,
    .max_bonds = 4,
};

static cfg_t cfg;

void setUp(void)
{
    cfg_defaults(&cfg, &board);
}

void tearDown(void)
{
}

static proto_config_t rec(uint8_t key, uint8_t selector)
{
    proto_config_t c;
    memset(&c, 0, sizeof(c));
    c.key = key;
    c.selector = selector;
    return c;
}

static void test_defaults_match_spec(void)
{
    proto_config_t c;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_SERIAL_JACK_MODE, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(0, c.value.serial_jack_mode.mode);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_PTT_TARGETS, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(0x01, c.value.ptt_targets.targets);
    TEST_ASSERT_EQUAL_UINT8(0, c.value.ptt_targets.usb_port);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_LINE_MAP, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PTT, c.value.line_map.rts_action);
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_IGNORE, c.value.line_map.dtr_action);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_LINE_MAP, PTT_PORT_CONTROL, &c));
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PTT, c.value.line_map.rts_action);
    for (uint8_t port = 1; port <= 4; port++) {
        TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_LINE_MAP, port, &c));
        TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PASS, c.value.line_map.rts_action);
        TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PASS, c.value.line_map.dtr_action);
    }
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_PTT_KEEPALIVE_MS, 0, &c));
    TEST_ASSERT_EQUAL_UINT16(3000, c.value.ptt_keepalive_ms.ms);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_MAX_TX_S, 0, &c));
    TEST_ASSERT_EQUAL_UINT32(300, c.value.max_tx_s.s);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_AUDIO_PATH, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(0, c.value.audio_path.path);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_TX_LEVEL, 0, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_RX_ATTENUATOR, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(0, c.value.rx_attenuator.on);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_RX_GAIN, 0, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_HOST_MODE, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(0, c.value.host_mode.mode);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_BLE_TX_POWER, 0, &c));
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, c.value.ble_tx_power.dbm); /* default: the cap */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_WIRED_PROFILE, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(1, c.value.wired_profile.profile); /* serial */
    for (uint8_t port = 0; port <= 4; port++) {
        TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_SERIAL_DEFAULT, port, &c));
        TEST_ASSERT_EQUAL_UINT32(9600, c.value.serial_default.baud);
        TEST_ASSERT_EQUAL_UINT8(8, c.value.serial_default.data_bits);
        TEST_ASSERT_EQUAL_UINT8(0, c.value.serial_default.parity);
        TEST_ASSERT_EQUAL_UINT8(0, c.value.serial_default.stop_bits);
    }
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_USB_NET_SUBNET, 0, &c));
    TEST_ASSERT_EQUAL_UINT8(10, c.value.usb_net_subnet.a);
    TEST_ASSERT_EQUAL_UINT8(169, c.value.usb_net_subnet.b);
    TEST_ASSERT_EQUAL_UINT8(160, c.value.usb_net_subnet.c);
    TEST_ASSERT_EQUAL_UINT8(0, c.value.usb_net_subnet.d);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_PAIRING_WINDOW_S, 0, &c));
    TEST_ASSERT_EQUAL_UINT16(120, c.value.pairing_window_s.s);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_POWER_DOWN_DELAY_S, 0, &c));
    TEST_ASSERT_EQUAL_UINT16(30, c.value.power_down_delay_s.s);
}

static void test_unknown_key_and_selector(void)
{
    proto_config_t c;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_get(&cfg, 0x00, 0, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_get(&cfg, 0x7F, 0, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_get(&cfg, PROTO_KEY_MAX_TX_S, 1, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_get(&cfg, PROTO_KEY_LINE_MAP, 5, &c));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_get(&cfg, PROTO_KEY_SERIAL_DEFAULT, 5, &c));
    c = rec(0x7F, 0);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
}

static void test_max_tx_range(void)
{
    proto_config_t c = rec(PROTO_KEY_MAX_TX_S, 0);
    c.value.max_tx_s.s = 9;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_UINT32(300, cfg.max_tx_s); /* unchanged on error */
    c.value.max_tx_s.s = 10;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.max_tx_s.s = 0xFFFFFFFFu; /* no upper bound */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.max_tx_s.s = 0; /* 0 = the timer is off */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    ptt_config_t pc;
    cfg_to_ptt(&cfg, &board, &pc);
    TEST_ASSERT_EQUAL_UINT32(PTT_MAX_TX_DISABLED, pc.max_tx_s);
}

static void test_keepalive_range(void)
{
    proto_config_t c = rec(PROTO_KEY_PTT_KEEPALIVE_MS, 0);
    c.value.ptt_keepalive_ms.ms = 499;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ptt_keepalive_ms.ms = 10001;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ptt_keepalive_ms.ms = 0; /* the keepalive can't be switched off */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ptt_keepalive_ms.ms = 500;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.ptt_keepalive_ms.ms = 10000;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
}

static void test_ble_tx_power_can_only_lower(void)
{
    proto_config_t c = rec(PROTO_KEY_BLE_TX_POWER, 0);
    c.value.ble_tx_power.dbm = BLE_TX_POWER_CAP_DBM + 1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ble_tx_power.dbm = 20;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ble_tx_power.dbm = 127;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.ble_tx_power.dbm = BLE_TX_POWER_MIN_DBM - 1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, cfg.ble_tx_power);
    c.value.ble_tx_power.dbm = 2; /* rounds down to a controller step */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_INT8(0, cfg.ble_tx_power);
    c.value.ble_tx_power.dbm = BLE_TX_POWER_MIN_DBM;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_MIN_DBM, cfg.ble_tx_power);
}

static void test_ptt_targets(void)
{
    proto_config_t c = rec(PROTO_KEY_PTT_TARGETS, 0);
    c.value.ptt_targets.targets = PTT_TARGET_RTS;
    c.value.ptt_targets.usb_port = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.ptt_targets.usb_port = 5;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.ptt_targets.targets = PTT_TARGET_CLOSURE;
    c.value.ptt_targets.usb_port = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.ptt_targets.targets = PTT_TARGET_CLOSURE | PTT_TARGET_DTR | 0xF0; /* reserved bits */
    c.value.ptt_targets.usb_port = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_UINT8(PTT_TARGET_CLOSURE | PTT_TARGET_DTR, cfg.ptt_targets);
    board_t closure_only = board;
    closure_only.ptt_outputs = PTT_TARGET_CLOSURE;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, cfg_set(&cfg, &closure_only, &c));
}

static void test_line_map(void)
{
    proto_config_t c = rec(PROTO_KEY_LINE_MAP, 0);
    c.value.line_map.rts_action = PTT_ACT_PASS; /* not on port 0 */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c = rec(PROTO_KEY_LINE_MAP, PTT_PORT_CONTROL);
    c.value.line_map.dtr_action = PTT_ACT_PASS; /* nor on 0x0F */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.line_map.dtr_action = 3;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c = rec(PROTO_KEY_LINE_MAP, 3);
    c.value.line_map.rts_action = PTT_ACT_PTT;
    c.value.line_map.dtr_action = PTT_ACT_PASS;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PTT, cfg.rts_action[3]);
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PASS, cfg.dtr_action[3]);
    c = rec(PROTO_KEY_LINE_MAP, 6);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
}

static void test_enums(void)
{
    proto_config_t c = rec(PROTO_KEY_SERIAL_JACK_MODE, 0);
    c.value.serial_jack_mode.mode = 4;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.serial_jack_mode.mode = 2; /* listed, but this board has mode 0 only */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, cfg_set(&cfg, &board, &c));
    c.value.serial_jack_mode.mode = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_AUDIO_PATH, 0);
    c.value.audio_path.path = 3;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.audio_path.path = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_HOST_MODE, 0);
    c.value.host_mode.mode = 3;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.host_mode.mode = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_WIRED_PROFILE, 0);
    c.value.wired_profile.profile = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.wired_profile.profile = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_RX_ATTENUATOR, 0);
    c.value.rx_attenuator.on = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.rx_attenuator.on = 1; /* the board has no attenuator */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, cfg_set(&cfg, &board, &c));
    c.value.rx_attenuator.on = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_TX_LEVEL, 0);
    c.value.tx_level.centibel = 1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.tx_level.centibel = -120;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_RX_GAIN, 0);
    c.value.rx_gain.centibel = 35;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
}

static void test_serial_default(void)
{
    proto_config_t c = rec(PROTO_KEY_SERIAL_DEFAULT, 0);
    c.value.serial_default.baud = 38400;
    c.value.serial_default.data_bits = 8;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.serial_default.baud = 4799;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.serial_default.baud = 115201;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.serial_default.baud = 9600;
    c.value.serial_default.data_bits = 6;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.serial_default.data_bits = 7;
    c.value.serial_default.parity = 5;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.serial_default.parity = 2;
    c.value.serial_default.stop_bits = 3;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    /* A radio port: its chip checks the baud rate when the port opens. */
    c = rec(PROTO_KEY_SERIAL_DEFAULT, 2);
    c.value.serial_default.baud = 921600;
    c.value.serial_default.data_bits = 8;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.serial_default.baud = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
}

static void test_usb_net_subnet(void)
{
    proto_config_t c = rec(PROTO_KEY_USB_NET_SUBNET, 0);
    const uint8_t good[][4] = {{10, 0, 0, 0}, {172, 16, 5, 4}, {172, 31, 0, 252}, {192, 168, 1, 8}};
    const uint8_t bad[][4] = {{8, 8, 8, 0}, {172, 32, 0, 0}, {192, 169, 0, 0}, {10, 0, 0, 2}};
    for (size_t i = 0; i < 4; i++) {
        c.value.usb_net_subnet.a = good[i][0];
        c.value.usb_net_subnet.b = good[i][1];
        c.value.usb_net_subnet.c = good[i][2];
        c.value.usb_net_subnet.d = good[i][3];
        TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
        c.value.usb_net_subnet.a = bad[i][0];
        c.value.usb_net_subnet.b = bad[i][1];
        c.value.usb_net_subnet.c = bad[i][2];
        c.value.usb_net_subnet.d = bad[i][3];
        TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    }
}

static void test_pairing_window_and_power_down(void)
{
    proto_config_t c = rec(PROTO_KEY_PAIRING_WINDOW_S, 0);
    c.value.pairing_window_s.s = 29;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.pairing_window_s.s = 601;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.pairing_window_s.s = 30;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    c = rec(PROTO_KEY_POWER_DOWN_DELAY_S, 0);
    c.value.power_down_delay_s.s = 0; /* never */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c.value.power_down_delay_s.s = 4;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.power_down_delay_s.s = 3601;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    c.value.power_down_delay_s.s = 3600;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
}

static void test_wired_port_lock(void)
{
    proto_config_t c = rec(PROTO_KEY_WIRED_PORT_LOCK, 0);
    proto_config_t g;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_get(&cfg, PROTO_KEY_WIRED_PORT_LOCK, 0, &g));
    TEST_ASSERT_EQUAL_UINT8(0, g.value.wired_port_lock.level); /* open */
    c.value.wired_port_lock.level = 4;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cfg_set(&cfg, &board, &c));
    c.value.wired_port_lock.level = 3;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cfg_set(&cfg, &board, &c));
    board_t iso = board;
    iso.radio_port_isolation = true;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &iso, &c));
    ptt_config_t pc;
    cfg_to_ptt(&cfg, &board, &pc);
    TEST_ASSERT_TRUE(pc.native_locked);
    c.value.wired_port_lock.level = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    cfg_to_ptt(&cfg, &board, &pc);
    TEST_ASSERT_FALSE(pc.native_locked);
}

static void test_storage_round_trip(void)
{
    proto_config_t c = rec(PROTO_KEY_MAX_TX_S, 0);
    c.value.max_tx_s.s = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c = rec(PROTO_KEY_LINE_MAP, 2);
    c.value.line_map.rts_action = PTT_ACT_PTT;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));
    c = rec(PROTO_KEY_BLE_TX_POWER, 0);
    c.value.ble_tx_power.dbm = -12;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cfg_set(&cfg, &board, &c));

    uint8_t blob[CFG_BLOB_MAX];
    size_t n = cfg_serialize(&cfg, blob, sizeof(blob));
    TEST_ASSERT_TRUE(n > 0);
    cfg_t loaded;
    TEST_ASSERT_EQUAL_INT(0, cfg_deserialize(&loaded, &board, blob, n));
    TEST_ASSERT_EQUAL_MEMORY(&cfg, &loaded, sizeof(cfg));
    TEST_ASSERT_EQUAL_size_t(0, cfg_serialize(&cfg, blob, 10)); /* too small */
    TEST_ASSERT_EQUAL_size_t(0, cfg_serialize(&cfg, blob, 5));
    /* A record count larger than the records present. */
    n = cfg_serialize(&cfg, blob, sizeof(blob));
    blob[3] = 200;
    uint16_t crc = proto_crc16(blob, n - 2);
    blob[n - 2] = (uint8_t)crc;
    blob[n - 1] = (uint8_t)(crc >> 8);
    TEST_ASSERT_EQUAL_INT(-1, cfg_deserialize(&loaded, &board, blob, n));
}

static void test_storage_rejects_corruption(void)
{
    uint8_t blob[CFG_BLOB_MAX];
    size_t n = cfg_serialize(&cfg, blob, sizeof(blob));
    cfg_t loaded;
    blob[10] ^= 0x40;
    TEST_ASSERT_EQUAL_INT(-1, cfg_deserialize(&loaded, &board, blob, n));
    cfg_t defaults;
    cfg_defaults(&defaults, &board);
    TEST_ASSERT_EQUAL_MEMORY(&defaults, &loaded, sizeof(loaded));
    TEST_ASSERT_EQUAL_INT(-1, cfg_deserialize(&loaded, &board, blob, 3));
    blob[0] = 'X';
    TEST_ASSERT_EQUAL_INT(-1, cfg_deserialize(&loaded, &board, blob, n));
}

/* A stored value that is out of range now (for example a BLE TX power above a
 * lowered cap) keeps its default: the blob can't bypass cfg_set(). */
static void test_storage_rechecks_every_value(void)
{
    uint8_t blob[CFG_BLOB_MAX];
    size_t n = cfg_serialize(&cfg, blob, sizeof(blob));
    /* Find the BLE_TX_POWER record and raise it to +20 dBm, then fix the CRC. */
    size_t pos = 4;
    while (pos < n - 2 && blob[pos] != PROTO_KEY_BLE_TX_POWER) {
        pos += 2u + (size_t)proto_config_value_len(blob[pos]);
    }
    TEST_ASSERT_EQUAL_UINT8(PROTO_KEY_BLE_TX_POWER, blob[pos]);
    blob[pos + 2] = 20;
    uint16_t crc = proto_crc16(blob, n - 2);
    blob[n - 2] = (uint8_t)crc;
    blob[n - 1] = (uint8_t)(crc >> 8);
    cfg_t loaded;
    TEST_ASSERT_EQUAL_INT(1, cfg_deserialize(&loaded, &board, blob, n));
    TEST_ASSERT_EQUAL_INT8(BLE_TX_POWER_CAP_DBM, loaded.ble_tx_power);
}

static void test_storage_stops_at_unknown_key(void)
{
    uint8_t blob[CFG_BLOB_MAX];
    size_t n = cfg_serialize(&cfg, blob, sizeof(blob));
    blob[4] = 0x7E; /* first record: a key from newer firmware */
    uint16_t crc = proto_crc16(blob, n - 2);
    blob[n - 2] = (uint8_t)crc;
    blob[n - 1] = (uint8_t)(crc >> 8);
    cfg_t loaded;
    TEST_ASSERT_EQUAL_INT(1, cfg_deserialize(&loaded, &board, blob, n));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_defaults_match_spec);
    RUN_TEST(test_unknown_key_and_selector);
    RUN_TEST(test_max_tx_range);
    RUN_TEST(test_keepalive_range);
    RUN_TEST(test_ble_tx_power_can_only_lower);
    RUN_TEST(test_ptt_targets);
    RUN_TEST(test_line_map);
    RUN_TEST(test_enums);
    RUN_TEST(test_serial_default);
    RUN_TEST(test_usb_net_subnet);
    RUN_TEST(test_pairing_window_and_power_down);
    RUN_TEST(test_wired_port_lock);
    RUN_TEST(test_storage_round_trip);
    RUN_TEST(test_storage_rejects_corruption);
    RUN_TEST(test_storage_rechecks_every_value);
    RUN_TEST(test_storage_stops_at_unknown_key);
    return UNITY_END();
}

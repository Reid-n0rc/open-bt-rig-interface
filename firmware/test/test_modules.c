/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Smaller modules: BLE TX power cap, pairing window, clock sync, CAT bridge
 * skeleton, audio and security stubs, reset-reason mapping.
 */
#include <string.h>

#include "app.h"
#include "audio.h"
#include "ble_power.h"
#include "cat.h"
#include "clock_sync.h"
#include "fake_hal.h"
#include "pairing.h"
#include "security.h"
#include "unity.h"

static const board_t board = {
    .variant = 'D',
    .hw_revision = 'A',
    .features = PROTO_F_CAT | PROTO_F_PTT_CLOSURE,
    .serial_modes = 0x01,
    .serial_min_baud = 4800,
    .serial_max_baud = 115200,
    .cat_tx_buffer = 8,
    .ptt_outputs = PTT_TARGET_CLOSURE,
};

void setUp(void)
{
    fake_reset();
}

void tearDown(void)
{
}

/* --- BLE TX power cap (SPEC §6.3, REQ-REG-002/-008) --- */

static void test_ble_cap_is_within_fcc_and_eu_limits(void)
{
    TEST_ASSERT_TRUE(BLE_TX_POWER_CAP_DBM * 100 <= BLE_TX_POWER_FCC_MAX_CDBM);
    TEST_ASSERT_TRUE(BLE_TX_POWER_CAP_DBM * 100 + BLE_ANTENNA_GAIN_CDBI <=
                     BLE_TX_POWER_EU_MAX_EIRP_CDBM);
    TEST_ASSERT_EQUAL_INT(6, BLE_TX_POWER_CAP_DBM);
    TEST_ASSERT_EQUAL_INT(10, BLE_DBM_TO_LEVEL(BLE_TX_POWER_CAP_DBM)); /* ESP_PWR_LVL_P6 */
}

static void test_ble_step_never_exceeds_cap(void)
{
    for (int dbm = -128; dbm <= 127; dbm++) {
        int8_t s = ble_power_step_dbm(dbm);
        TEST_ASSERT_TRUE(s <= BLE_TX_POWER_CAP_DBM);
        TEST_ASSERT_TRUE(s >= BLE_TX_POWER_MIN_DBM);
        TEST_ASSERT_EQUAL_INT(0, (s - BLE_TX_POWER_MIN_DBM) % BLE_TX_POWER_STEP_DB);
        if (dbm >= BLE_TX_POWER_MIN_DBM) {
            TEST_ASSERT_TRUE(s <= dbm); /* rounds down, never up */
        }
    }
    TEST_ASSERT_EQUAL_INT8(0, ble_power_step_dbm(2));
    TEST_ASSERT_EQUAL_INT8(3, ble_power_step_dbm(3));
    TEST_ASSERT_EQUAL_INT8(6, ble_power_step_dbm(20));
}

/* --- Pairing window (SPEC §13.5) --- */

static void test_pairing_window(void)
{
    pairing_t pw;
    pairing_init(&pw);
    TEST_ASSERT_FALSE(pairing_is_open(&pw));
    pairing_open_local(&pw, 120, 1000);
    TEST_ASSERT_TRUE(pairing_is_open(&pw));
    pairing_tick(&pw, 120999);
    TEST_ASSERT_TRUE(pairing_is_open(&pw));
    pairing_tick(&pw, 121000);
    TEST_ASSERT_FALSE(pairing_is_open(&pw));

    pairing_open_local(&pw, 30, 0);
    pairing_bond_added(&pw); /* the first new bond closes it */
    TEST_ASSERT_FALSE(pairing_is_open(&pw));

    pairing_open_local(&pw, 30, 0);
    pairing_close(&pw); /* a host-mode change closes it */
    TEST_ASSERT_FALSE(pairing_is_open(&pw));
}

/* --- Clock sync (SPEC §10) --- */

static void test_clock_sync(void)
{
    clock_sync_t c;
    clock_sync_init(&c);
    proto_time_req_t req = {1000000};
    proto_time_resp_t resp;
    clock_sync_resp(&req, 55000000, 55000120, &resp);
    TEST_ASSERT_EQUAL_UINT64(1000000, resp.host_t1);
    TEST_ASSERT_EQUAL_UINT64(55000000, resp.device_t2);
    TEST_ASSERT_EQUAL_UINT64(55000120, resp.device_t3);

    uint64_t dev;
    TEST_ASSERT_FALSE(clock_sync_fresh(&c, 60000000));
    TEST_ASSERT_FALSE(clock_sync_utc_to_device(&c, 1, 60000000, &dev));

    proto_time_set_t set = {55000060, 1790000000000000ull, 8000};
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, clock_sync_set(&c, &set, 50000000)); /* future */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, clock_sync_set(&c, &set, 60000000));
    TEST_ASSERT_TRUE(clock_sync_fresh(&c, 60000000));
    TEST_ASSERT_TRUE(clock_sync_utc_to_device(&c, 1790000015000000ull, 60000000, &dev));
    TEST_ASSERT_EQUAL_UINT64(55000060ull + 15000000ull, dev);
    TEST_ASSERT_FALSE(clock_sync_utc_to_device(&c, 1, 60000000, &dev)); /* before the mapping */
    /* Stale after 10 minutes. */
    TEST_ASSERT_FALSE(clock_sync_fresh(&c, 60000000 + CLOCK_FRESH_US));
}

/* --- CAT bridge skeleton (SPEC §7) --- */

static ptt_t ptt;
static cfg_t cfg;

static void cat_setup(cat_t *c)
{
    ptt_config_t pc;
    ptt_config_defaults(&pc);
    ptt_init(&ptt, &pc, NULL, 0, PTT_R_BOOT);
    cfg_defaults(&cfg, &board);
    cat_init(c);
}

static void test_cat_ports_and_open(void)
{
    cat_t c;
    cat_setup(&c);
    TEST_ASSERT_TRUE(cat_port_exists(&c, 0));
    TEST_ASSERT_FALSE(cat_port_exists(&c, 1)); /* no radio USB-serial yet (#43) */
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, cat_open(&c, &ptt, &board, &cfg, 1, 1, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_open(&c, &ptt, &board, &cfg, 9, 1, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_open(&c, &ptt, &board, &cfg, 0, 2, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 0, 1, 0));
    TEST_ASSERT_TRUE(cat_is_open(&c, 0));
    TEST_ASSERT_TRUE(fake.serial_open[0]);
    TEST_ASSERT_EQUAL_UINT32(9600, fake.serial_cfg[0].baud); /* SERIAL_DEFAULT */
    TEST_ASSERT_EQUAL_UINT16(8, c.port[0].credit);
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 0, 0, 0));
    TEST_ASSERT_FALSE(cat_is_open(&c, 0));
    TEST_ASSERT_FALSE(fake.serial_open[0]);
    /* The HAL refusing the format. */
    fake.serial_open_result = PROTO_ERR_UNSUPPORTED;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, cat_open(&c, &ptt, &board, &cfg, 0, 1, 0));
    TEST_ASSERT_FALSE(cat_is_open(&c, 0));
    /* A radio port once enumerated. */
    c.radio_ports_present = 1u << 2;
    fake.serial_open_result = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 2, 1, 0));
}

static void test_cat_set(void)
{
    cat_t c;
    cat_setup(&c);
    proto_serial_set_t s = {0, 38400, 8, 0, 0};
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_STATE, cat_set(&c, &board, &s)); /* not open */
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 0, 1, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_set(&c, &board, &s));
    TEST_ASSERT_EQUAL_UINT32(38400, fake.serial_cfg[0].baud);
    s.baud = 1200;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cat_set(&c, &board, &s));
    s.baud = 9600;
    s.data_bits = 5;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_set(&c, &board, &s));
    s.data_bits = 8;
    s.stop_bits = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_set(&c, &board, &s));
    s.port = 9;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_set(&c, &board, &s));
    /* A radio port with baud 0. */
    c.radio_ports_present = 1u << 1;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 1, 1, 0));
    proto_serial_set_t r = {1, 0, 8, 0, 0};
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OUT_OF_RANGE, cat_set(&c, &board, &r));
}

static void test_cat_credit_and_overflow(void)
{
    cat_t c;
    cat_setup(&c);
    uint16_t written;
    const uint8_t data[] = "FA;FB;IF;TX;";
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_STATE, cat_from_host(&c, 0, data, 3, &written));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 0, 1, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_LENGTH, cat_from_host(&c, 0, data, 0, &written));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_from_host(&c, 0, data, 3, &written));
    TEST_ASSERT_EQUAL_UINT16(3, written);
    TEST_ASSERT_EQUAL_MEMORY("FA;", fake.serial_out, 3); /* bytes pass unchanged */
    /* The radio side is slow: only 2 of 6 bytes leave now. */
    fake.serial_accept = 2;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, cat_from_host(&c, 0, data + 3, 6, &written));
    TEST_ASSERT_EQUAL_UINT16(2, written);
    TEST_ASSERT_EQUAL_UINT16(4, c.port[0].credit);
    TEST_ASSERT_EQUAL_UINT16(1, c.overflows);
    /* More than the credit: the excess is dropped. */
    fake.serial_accept = (size_t)-1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_OVERFLOW, cat_from_host(&c, 0, data, 12, &written));
    TEST_ASSERT_EQUAL_UINT16(4, written);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_from_host(&c, 9, data, 1, &written));
}

static void test_cat_modem_lines_and_close_all(void)
{
    cat_t c;
    cat_setup(&c);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_STATE, cat_modem_lines(&c, &ptt, 0, 0, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_open(&c, &ptt, &board, &cfg, 0, 1, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_modem_lines(&c, &ptt, 0, 0, 0));
    TEST_ASSERT_EQUAL_INT(PROTO_OK, cat_modem_lines(&c, &ptt, 0, PTT_LINE_RTS, 0));
    TEST_ASSERT_TRUE(ptt.on);
    cat_close_all(&c, &ptt, 0);
    TEST_ASSERT_FALSE(ptt.on);
    TEST_ASSERT_FALSE(cat_is_open(&c, 0));
    TEST_ASSERT_EQUAL_UINT16(0, c.port[0].credit);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_BAD_VALUE, cat_modem_lines(&c, &ptt, 9, 0, 0));
}

/* --- Audio hooks (stubs until #16) --- */

static void test_audio_stubs(void)
{
    audio_t a;
    audio_init(&a);
    proto_audio_start_t req = {1, 0, 12000, 120, 0, 0};
    proto_audio_status_t st;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_start(&a, &board, &req, &st));
    TEST_ASSERT_EQUAL_UINT8(0, st.state);
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_stop(&a, &board, 1, &st));
    proto_audio_frame_t f;
    memset(&f, 0, sizeof(f));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_frame(&a, &board, &f));
    board_t with_audio = board;
    with_audio.features |= PROTO_F_AUDIO_RX;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_start(&a, &with_audio, &req, &st));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_stop(&a, &with_audio, 1, &st));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, audio_frame(&a, &with_audio, &f));
    a.running = true;
    audio_session_end(&a);
    TEST_ASSERT_FALSE(a.running);
}

/* --- Security hooks (#64): fail closed --- */

static void test_security_stubs_fail_closed(void)
{
    sec_host_id_t h;
    memset(&h, 0xAB, sizeof(h));
    TEST_ASSERT_FALSE(sec_wired_host_approved(&h));
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, sec_wired_host_approve(&h, true));
    TEST_ASSERT_FALSE(sec_wired_host_approved(&h));
    TEST_ASSERT_EQUAL_UINT(0, sec_bond_count());
    fake.secure_erase_result = -1;
    TEST_ASSERT_EQUAL_INT(PROTO_ERR_UNSUPPORTED, sec_factory_reset());
    fake.secure_erase_result = 0;
    TEST_ASSERT_EQUAL_INT(PROTO_OK, sec_factory_reset());
}

/* --- Reset reason -> first PTT_STATUS reason (SPEC §8.5, §8.6) --- */

static void test_boot_reason_mapping(void)
{
    TEST_ASSERT_EQUAL_UINT8(PTT_R_BOOT, app_boot_reason(APP_RESET_POWER_ON));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_WATCHDOG, app_boot_reason(APP_RESET_WATCHDOG));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, app_boot_reason(APP_RESET_BROWNOUT));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, app_boot_reason(APP_RESET_PANIC));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, app_boot_reason(APP_RESET_OTHER));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_ble_cap_is_within_fcc_and_eu_limits);
    RUN_TEST(test_ble_step_never_exceeds_cap);
    RUN_TEST(test_pairing_window);
    RUN_TEST(test_clock_sync);
    RUN_TEST(test_cat_ports_and_open);
    RUN_TEST(test_cat_set);
    RUN_TEST(test_cat_credit_and_overflow);
    RUN_TEST(test_cat_modem_lines_and_close_all);
    RUN_TEST(test_audio_stubs);
    RUN_TEST(test_security_stubs_fail_closed);
    RUN_TEST(test_boot_reason_mapping);
    return UNITY_END();
}

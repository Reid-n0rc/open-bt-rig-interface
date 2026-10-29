/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Fake HAL for the host tests. See fake_hal.h.
 */
#include "fake_hal.h"

#include <string.h>

fake_hal_t fake;

void fake_reset(void)
{
    memset(&fake, 0, sizeof(fake));
    fake.serial_accept = (size_t)-1;
    fake.link.host_link = HAL_LINK_BLUETOOTH;
}

void fake_clear_tx(void)
{
    memset(fake.tx_len, 0, sizeof(fake.tx_len));
}

/* Decoded frames live in this buffer until the next fake_sent() call. */
static uint8_t frames[FAKE_MSG_MAX][PROTO_MAX_FRAME];

size_t fake_sent(uint8_t transport, proto_msg_t *out, size_t max)
{
    size_t n = 0;
    const uint8_t *p = fake.tx[transport];
    size_t len = fake.tx_len[transport];
    size_t start = 0;
    for (size_t i = 0; i < len && n < max && n < FAKE_MSG_MAX; i++) {
        if (p[i] != 0) {
            continue;
        }
        int flen = proto_cobs_decode(p + start, i - start, frames[n], PROTO_MAX_FRAME);
        start = i + 1;
        uint8_t type, token;
        const uint8_t *payload;
        size_t plen;
        if (flen < 0 ||
            proto_frame_check(frames[n], (size_t)flen, &type, &token, &payload, &plen) != PROTO_OK ||
            proto_decode_payload(type, token, payload, plen, &out[n]) != PROTO_OK) {
            memset(&out[n], 0, sizeof(out[n])); /* type 0 marks a bad frame */
        }
        n++;
    }
    fake.tx_len[transport] = 0;
    return n;
}

/* --- hal.h --- */

uint64_t hal_time_us(void)
{
    return fake.time_us;
}

void hal_ptt_closure_set(bool on)
{
    if (on && !fake.closure) {
        fake.closure_on_count++;
    }
    fake.closure = on;
    fake.closure_writes++;
}

void hal_radio_lines_set(uint8_t port, uint8_t lines)
{
    if (port < 5) {
        fake.radio_lines[port] = lines;
    }
}

void hal_status_led_set(bool on)
{
    fake.led = on;
}

int hal_serial_open(uint8_t port, const hal_uart_cfg_t *cfg)
{
    if (fake.serial_open_result != 0) {
        return fake.serial_open_result;
    }
    fake.serial_open[port] = true;
    fake.serial_cfg[port] = *cfg;
    return 0;
}

void hal_serial_close(uint8_t port)
{
    fake.serial_open[port] = false;
}

size_t hal_serial_write(uint8_t port, const uint8_t *data, size_t len)
{
    (void)port;
    size_t n = len < fake.serial_accept ? len : fake.serial_accept;
    if (fake.serial_out_len + n > sizeof(fake.serial_out)) {
        n = sizeof(fake.serial_out) - fake.serial_out_len;
    }
    memcpy(fake.serial_out + fake.serial_out_len, data, n);
    fake.serial_out_len += n;
    return n;
}

void hal_serial_jack_mode(uint8_t mode)
{
    fake.jack_mode = mode;
    fake.jack_mode_sets++;
}

int hal_audio_open(uint32_t sample_rate_hz)
{
    (void)sample_rate_hz;
    return PROTO_ERR_UNSUPPORTED;
}

void hal_audio_close(void)
{
}

void hal_ble_tx_power_set(int8_t dbm)
{
    fake.ble_dbm = dbm;
    fake.ble_sets++;
}

void hal_transport_send(uint8_t transport, const uint8_t *data, size_t len)
{
    if (transport > HAL_TRANSPORT_MAX || fake.tx_len[transport] + len > FAKE_TX_MAX) {
        return;
    }
    memcpy(fake.tx[transport] + fake.tx_len[transport], data, len);
    fake.tx_len[transport] += len;
}

void hal_link_status(hal_link_status_t *out)
{
    *out = fake.link;
}

size_t hal_config_load(uint8_t *buf, size_t cap)
{
    size_t n = fake.stored_len < cap ? fake.stored_len : cap;
    memcpy(buf, fake.stored, n);
    return n;
}

int hal_secure_get(const char *key, uint8_t *buf, size_t cap, size_t *len)
{
    (void)key;
    (void)buf;
    (void)cap;
    *len = 0;
    return -1;
}

int hal_secure_set(const char *key, const uint8_t *buf, size_t len)
{
    (void)key;
    (void)buf;
    (void)len;
    return -1;
}

int hal_secure_erase_all(void)
{
    return fake.secure_erase_result;
}

int hal_config_save(const uint8_t *buf, size_t len)
{
    if (fake.save_result != 0) {
        return fake.save_result;
    }
    if (len > sizeof(fake.stored)) {
        return -1;
    }
    memcpy(fake.stored, buf, len);
    fake.stored_len = len;
    fake.saves++;
    return 0;
}

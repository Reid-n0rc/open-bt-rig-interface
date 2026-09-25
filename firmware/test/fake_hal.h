/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Fake HAL for the host tests: records every output and lets a test set time
 * and inputs.
 */
#ifndef FAKE_HAL_H
#define FAKE_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal.h"
#include "proto.h"

#define FAKE_TX_MAX 8192u
#define FAKE_MSG_MAX 64u

typedef struct {
    uint64_t time_us;
    bool closure;
    unsigned closure_writes;
    unsigned closure_on_count; /* how many times the closure was switched on */
    uint8_t radio_lines[5];
    bool led;
    int8_t ble_dbm;
    unsigned ble_sets;
    uint8_t jack_mode;
    unsigned jack_mode_sets;

    bool serial_open[5];
    hal_uart_cfg_t serial_cfg[5];
    int serial_open_result; /* returned by hal_serial_open */
    uint8_t serial_out[1024];
    size_t serial_out_len;
    size_t serial_accept; /* max bytes hal_serial_write accepts per call */

    uint8_t tx[HAL_TRANSPORT_MAX + 1][FAKE_TX_MAX]; /* bytes sent per transport */
    size_t tx_len[HAL_TRANSPORT_MAX + 1];

    hal_link_status_t link;

    uint8_t stored[256];
    size_t stored_len;
    unsigned saves;
    int save_result;
    int secure_erase_result;
} fake_hal_t;

extern fake_hal_t fake;

void fake_reset(void);
/* Decodes every frame sent on a transport since the last call (or reset). */
size_t fake_sent(uint8_t transport, proto_msg_t *out, size_t max);
/* Discards what was sent. */
void fake_clear_tx(void);

#endif /* FAKE_HAL_H */

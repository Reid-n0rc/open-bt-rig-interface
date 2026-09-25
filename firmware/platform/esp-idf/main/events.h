/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Events from the Bluetooth host task and drivers to the app task. One
 * queue keeps stream bytes and link events in order, and only the app task
 * touches the app state.
 */
#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EVT_DATA_MAX 244u /* ATT_MTU 247 - 3 */

typedef enum {
    EVT_RX,               /* stream bytes from a transport */
    EVT_TRANSPORT_CLOSED, /* link lost, unsubscribed, ... */
    EVT_BOND_ADDED,       /* a new BLE bond was stored */
    EVT_PAIR_BUTTON,      /* the pairing button was pressed */
    EVT_SERIAL_RX,        /* bytes from the radio on a serial port */
} evt_type_t;

typedef struct {
    uint8_t type;      /* evt_type_t */
    uint8_t transport; /* HAL_TRANSPORT_* or the serial port */
    uint16_t len;
    uint8_t data[EVT_DATA_MAX];
} evt_t;

/* Posts an event; copies up to EVT_DATA_MAX bytes per event (longer data is
 * split). Returns false if the queue is full (the bytes are lost and counted
 * as a frame error by the receiver when the frame fails its CRC). */
bool evt_post(evt_type_t type, uint8_t transport, const uint8_t *data, size_t len);

#endif /* EVENTS_H */

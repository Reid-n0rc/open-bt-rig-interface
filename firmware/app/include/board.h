/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * What the hardware can do. The platform fills this in; the app reports it
 * in DEVICE_INFO and CAPS (SPEC §4.2, §5) and uses it to check config values.
 */
#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PAIRING.triggers bits (SPEC §5.1). */
#define BOARD_PAIR_POWER_ON 0x01u
#define BOARD_PAIR_BUTTON 0x02u

typedef struct {
    char variant;     /* DEVICE_INFO.variant: 'R', 'M', or 'D' for a dev kit */
    char hw_revision; /* DEVICE_INFO.hw_revision: 'A', ... */
    uint32_t features; /* FEATURES bits (proto.h PROTO_F_*) */

    /* SERIAL_JACK TLV */
    uint8_t serial_modes; /* bit n = SERIAL-jack mode n */
    uint32_t serial_min_baud, serial_max_baud;
    uint16_t cat_tx_buffer; /* initial CAT credit per port */

    /* PTT TLV */
    uint8_t ptt_outputs; /* bit 0 closure, bit 1 RTS, bit 2 DTR on a radio port */

    /* PAIRING TLV */
    uint8_t pairing_triggers;
    uint8_t max_bonds;
    uint8_t max_wired_hosts; /* approved wired hosts it can store (#64) */

    /* WIRED_PORT_LOCK level 3 needs a radio-port switch that can disconnect. */
    bool radio_port_isolation;

    /* Info characteristic */
    uint16_t l2cap_psm; /* 0 = no L2CAP CoC */
} board_t;

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */

/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * CAT bridge (SPEC §7): serial ports, host credit and RTS/DTR. The bytes
 * pass unchanged (REQ-CAT-001); nothing here parses CAT. This is the
 * skeleton: port 0 (SERIAL jack) goes to hal_serial_*; radio USB-serial
 * ports 1-4 arrive with the USB host issue (#43). Batching and the full
 * credit scheme are #15.
 */
#ifndef CAT_H
#define CAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "cfg.h"
#include "ptt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAT_PORTS 5u /* 0 SERIAL jack, 1-4 radio USB-serial */

typedef struct {
    bool open;
    uint16_t credit; /* bytes the host may still send */
} cat_port_t;

typedef struct {
    cat_port_t port[CAT_PORTS];
    uint8_t radio_ports_present; /* bit n = radio port n enumerated (#43) */
    uint16_t overflows;          /* STATUS.cat_overflows */
} cat_t;

void cat_init(cat_t *c);
bool cat_port_exists(const cat_t *c, uint8_t port);
bool cat_is_open(const cat_t *c, uint8_t port);

/* SERIAL_OPEN. Returns a PROTO code. */
int cat_open(cat_t *c, ptt_t *ptt, const board_t *board, const cfg_t *cfg, uint8_t port,
             uint8_t open, uint64_t now_ms);
/* SERIAL_SET. */
int cat_set(cat_t *c, const board_t *board, const proto_serial_set_t *s);
/* CAT_DATA from the host. Writes what fits in the credit; `*written` is the
 * number of bytes that left toward the radio (credit to return). Returns
 * PROTO_OK, PROTO_ERR_OVERFLOW (credit exceeded, excess dropped) or an error. */
int cat_from_host(cat_t *c, uint8_t port, const uint8_t *data, size_t len, uint16_t *written);
/* MODEM_LINES. */
int cat_modem_lines(cat_t *c, ptt_t *ptt, uint8_t port, uint8_t lines, uint64_t now_ms);
/* Session end: closes every port (their lines deassert) and forgets credit. */
void cat_close_all(cat_t *c, ptt_t *ptt, uint64_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* CAT_H */

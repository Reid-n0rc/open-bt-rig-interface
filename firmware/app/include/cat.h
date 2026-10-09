/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * CAT bridge (SPEC §7): serial ports, host credit and RTS/DTR. The bytes
 * pass unchanged (REQ-CAT-001); nothing here parses CAT. Port 0 (SERIAL
 * jack) goes to hal_serial_*; radio USB-serial ports 1-4 arrive with the USB
 * host issue (#43).
 *
 * Host -> radio: CAT_DATA fills a per-port queue within the host's credit;
 * cat_pump() hands it to the HAL as the HAL accepts bytes, and only the bytes
 * that left come back as CAT_CREDIT (§7.4).
 * Radio -> host: bytes are batched per port and sent when a frame is full or
 * CAT_BATCH_US after the first byte (§7.3). If the batch buffer fills, the
 * newest bytes are dropped and counted (STATUS.cat_overflows).
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

/* Host -> radio queue per port. A port's credit (board->cat_tx_buffer) is
 * capped to this. */
#define CAT_TXQ_MAX 1024u
/* Radio -> host batch buffer per port: one largest frame (max_payload 1024,
 * SPEC §12.2) plus what can arrive while it is being sent. */
#define CAT_RXQ_MAX 2048u
/* Radio -> host batching window (SPEC §7.3). */
#define CAT_BATCH_US 2000u
/* How soon a non-empty host -> radio queue is pumped again. */
#define CAT_PUMP_US 1000u

typedef struct {
    bool open;
    uint16_t credit; /* bytes the host may still send */

    uint8_t txq[CAT_TXQ_MAX]; /* toward the radio, not yet accepted by the HAL */
    uint16_t txq_len;

    uint8_t rxq[CAT_RXQ_MAX]; /* from the radio, not yet sent to the host */
    uint16_t rxq_len;
    uint64_t rx_first_us; /* arrival of the oldest unsent byte */
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
/* CAT_DATA from the host: queues what fits in the credit. Returns PROTO_OK,
 * PROTO_ERR_OVERFLOW (credit exceeded, the excess dropped) or an error. */
int cat_from_host(cat_t *c, uint8_t port, const uint8_t *data, size_t len);
/* Hands queued bytes to the HAL. Returns how many left toward the radio:
 * the credit to return to the host in CAT_CREDIT. */
uint16_t cat_pump(cat_t *c, uint8_t port);
/* MODEM_LINES. */
int cat_modem_lines(cat_t *c, ptt_t *ptt, uint8_t port, uint8_t lines, uint64_t now_ms);
/* Session end: closes every port (their lines deassert), forgets credit and
 * discards unsent bytes in both directions. */
void cat_close_all(cat_t *c, ptt_t *ptt, uint64_t now_ms);

/* Bytes from the radio on an open port. Returns how many were dropped
 * because the batch buffer was full (0 normally). */
size_t cat_from_radio(cat_t *c, uint8_t port, const uint8_t *data, size_t len, uint64_t now_us);
/* The next batch due for the host on a port: copies up to `chunk_max`
 * bytes into `out` and returns the count, or 0 if nothing is due yet. A
 * batch is due when it fills `chunk_max` or CAT_BATCH_US after its first
 * byte. */
size_t cat_rx_next(cat_t *c, uint8_t port, size_t chunk_max, uint64_t now_us, uint8_t *out);
/* When cat_pump() or cat_rx_next() next has work: an absolute time in
 * microseconds, or UINT64_MAX if nothing is waiting. */
uint64_t cat_next_deadline_us(const cat_t *c, uint64_t now_us);

#ifdef __cplusplus
}
#endif

#endif /* CAT_H */

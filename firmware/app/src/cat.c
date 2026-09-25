/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * CAT bridge skeleton (SPEC §7). See cat.h.
 */
#include "cat.h"

#include <string.h>

#include "hal.h"

void cat_init(cat_t *c)
{
    memset(c, 0, sizeof(*c));
}

bool cat_port_exists(const cat_t *c, uint8_t port)
{
    if (port == 0) {
        return true;
    }
    return port <= 4 && (c->radio_ports_present & (1u << port));
}

bool cat_is_open(const cat_t *c, uint8_t port)
{
    return port < CAT_PORTS && c->port[port].open;
}

static int port_check(const cat_t *c, uint8_t port)
{
    if (port >= CAT_PORTS) {
        return PROTO_ERR_BAD_VALUE;
    }
    if (!cat_port_exists(c, port)) {
        return PROTO_ERR_UNSUPPORTED; /* radio port not enumerated */
    }
    return PROTO_OK;
}

int cat_open(cat_t *c, ptt_t *ptt, const board_t *board, const cfg_t *cfg, uint8_t port,
             uint8_t open, uint64_t now_ms)
{
    int rc = port_check(c, port);
    if (rc != PROTO_OK) {
        return rc;
    }
    if (open > 1) {
        return PROTO_ERR_BAD_VALUE;
    }
    if (!open) {
        if (c->port[port].open) {
            hal_serial_close(port);
        }
        c->port[port].open = false;
        c->port[port].credit = 0;
        ptt_port_closed(ptt, port, now_ms); /* lines deassert, sources release */
        return PROTO_OK;
    }
    const cfg_serial_t *d = &cfg->serial_default[port];
    hal_uart_cfg_t u = {d->baud, d->data_bits, d->parity, d->stop_bits};
    rc = hal_serial_open(port, &u);
    if (rc != PROTO_OK) {
        return rc;
    }
    c->port[port].open = true;
    c->port[port].credit = board->cat_tx_buffer; /* SPEC §7.2 */
    ptt_port_opened(ptt, port, now_ms);          /* restarts RTS/DTR arming (§8.4) */
    return PROTO_OK;
}

int cat_set(cat_t *c, const board_t *board, const proto_serial_set_t *s)
{
    int rc = port_check(c, s->port);
    if (rc != PROTO_OK) {
        return rc;
    }
    if (!c->port[s->port].open) {
        return PROTO_ERR_STATE;
    }
    if ((s->data_bits != 7 && s->data_bits != 8) || s->parity > 4 || s->stop_bits > 2) {
        return PROTO_ERR_BAD_VALUE;
    }
    if (s->port == 0 && (s->baud < board->serial_min_baud || s->baud > board->serial_max_baud)) {
        return PROTO_ERR_OUT_OF_RANGE;
    }
    if (s->baud == 0) {
        return PROTO_ERR_OUT_OF_RANGE;
    }
    hal_uart_cfg_t u = {s->baud, s->data_bits, s->parity, s->stop_bits};
    return hal_serial_open(s->port, &u);
}

int cat_from_host(cat_t *c, uint8_t port, const uint8_t *data, size_t len, uint16_t *written)
{
    *written = 0;
    int rc = port_check(c, port);
    if (rc != PROTO_OK) {
        return rc;
    }
    if (!c->port[port].open) {
        return PROTO_ERR_STATE;
    }
    if (len == 0) {
        return PROTO_ERR_BAD_LENGTH; /* 1 to max_payload - 1 bytes */
    }
    size_t take = len <= c->port[port].credit ? len : c->port[port].credit;
    c->port[port].credit = (uint16_t)(c->port[port].credit - take);
    size_t out = take ? hal_serial_write(port, data, take) : 0;
    /* Bytes that left toward the radio come back as credit (SPEC §7.4). */
    c->port[port].credit = (uint16_t)(c->port[port].credit + out);
    *written = (uint16_t)out;
    if (take < len || out < take) {
        c->overflows = c->overflows < 0xFFFFu ? (uint16_t)(c->overflows + 1u) : 0xFFFFu;
        return PROTO_ERR_OVERFLOW;
    }
    return PROTO_OK;
}

int cat_modem_lines(cat_t *c, ptt_t *ptt, uint8_t port, uint8_t lines, uint64_t now_ms)
{
    int rc = port_check(c, port);
    if (rc != PROTO_OK) {
        return rc;
    }
    if (!c->port[port].open) {
        return PROTO_ERR_STATE;
    }
    ptt_lines(ptt, port, lines, PTT_ORIGIN_PROTOCOL, now_ms);
    return PROTO_OK;
}

void cat_close_all(cat_t *c, ptt_t *ptt, uint64_t now_ms)
{
    for (uint8_t port = 0; port < CAT_PORTS; port++) {
        if (c->port[port].open) {
            hal_serial_close(port);
            ptt_port_closed(ptt, port, now_ms);
        }
        c->port[port].open = false;
        c->port[port].credit = 0;
    }
}

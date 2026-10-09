/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * CAT bridge (SPEC §7). See cat.h.
 */
#include "cat.h"

#include <string.h>

#include "hal.h"

static void count_overflow(cat_t *c)
{
    if (c->overflows < 0xFFFFu) {
        c->overflows++;
    }
}

/* Forgets credit and unsent bytes in both directions (SPEC §7.2). */
static void port_reset(cat_port_t *p)
{
    p->open = false;
    p->credit = 0;
    p->txq_len = 0;
    p->rxq_len = 0;
    p->rx_first_us = 0;
}

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
        port_reset(&c->port[port]);
        ptt_port_closed(ptt, port, now_ms); /* lines deassert, sources release */
        return PROTO_OK;
    }
    const cfg_serial_t *d = &cfg->serial_default[port];
    hal_uart_cfg_t u = {d->baud, d->data_bits, d->parity, d->stop_bits};
    rc = hal_serial_open(port, &u);
    if (rc != PROTO_OK) {
        return rc;
    }
    /* Opening again restarts the port: fresh credit, nothing queued (§7.2). */
    port_reset(&c->port[port]);
    c->port[port].open = true;
    c->port[port].credit =
        board->cat_tx_buffer <= CAT_TXQ_MAX ? board->cat_tx_buffer : (uint16_t)CAT_TXQ_MAX;
    ptt_port_opened(ptt, port, now_ms); /* restarts RTS/DTR arming (§8.4) */
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

int cat_from_host(cat_t *c, uint8_t port, const uint8_t *data, size_t len)
{
    int rc = port_check(c, port);
    if (rc != PROTO_OK) {
        return rc;
    }
    cat_port_t *p = &c->port[port];
    if (!p->open) {
        return PROTO_ERR_STATE;
    }
    if (len == 0) {
        return PROTO_ERR_BAD_LENGTH; /* 1 to max_payload - 1 bytes */
    }
    /* credit + queued never exceeds the queue, so the credit always fits. */
    size_t take = len <= p->credit ? len : p->credit;
    memcpy(p->txq + p->txq_len, data, take);
    p->txq_len = (uint16_t)(p->txq_len + take);
    p->credit = (uint16_t)(p->credit - take);
    if (take < len) {
        count_overflow(c);
        return PROTO_ERR_OVERFLOW; /* the host exceeded its credit */
    }
    return PROTO_OK;
}

uint16_t cat_pump(cat_t *c, uint8_t port)
{
    if (port >= CAT_PORTS) {
        return 0;
    }
    cat_port_t *p = &c->port[port];
    if (!p->open || p->txq_len == 0) {
        return 0;
    }
    size_t out = hal_serial_write(port, p->txq, p->txq_len);
    if (out > p->txq_len) {
        out = p->txq_len; /* a HAL can't accept more than offered */
    }
    memmove(p->txq, p->txq + out, p->txq_len - out);
    p->txq_len = (uint16_t)(p->txq_len - out);
    /* Bytes that left toward the radio come back as credit (§7.4). */
    p->credit = (uint16_t)(p->credit + out);
    return (uint16_t)out;
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
        port_reset(&c->port[port]);
    }
}

size_t cat_from_radio(cat_t *c, uint8_t port, const uint8_t *data, size_t len, uint64_t now_us)
{
    if (port >= CAT_PORTS || !c->port[port].open || len == 0) {
        return 0;
    }
    cat_port_t *p = &c->port[port];
    size_t room = CAT_RXQ_MAX - p->rxq_len;
    size_t take = len <= room ? len : room;
    if (p->rxq_len == 0 && take) {
        p->rx_first_us = now_us;
    }
    memcpy(p->rxq + p->rxq_len, data, take);
    p->rxq_len = (uint16_t)(p->rxq_len + take);
    if (take < len) {
        count_overflow(c); /* the newest bytes are dropped (§7.4) */
    }
    return len - take;
}

size_t cat_rx_next(cat_t *c, uint8_t port, size_t chunk_max, uint64_t now_us, uint8_t *out)
{
    if (port >= CAT_PORTS || chunk_max == 0) {
        return 0;
    }
    cat_port_t *p = &c->port[port];
    if (!p->open || p->rxq_len == 0) {
        return 0;
    }
    bool full = p->rxq_len >= chunk_max;
    bool aged = now_us - p->rx_first_us >= CAT_BATCH_US;
    if (!full && !aged) {
        return 0;
    }
    size_t n = full ? chunk_max : p->rxq_len;
    memcpy(out, p->rxq, n);
    memmove(p->rxq, p->rxq + n, p->rxq_len - n);
    p->rxq_len = (uint16_t)(p->rxq_len - n);
    /* What remains arrived after the oldest byte just sent: keeping that
     * time can only send it sooner, never later than CAT_BATCH_US. */
    return n;
}

uint64_t cat_next_deadline_us(const cat_t *c, uint64_t now_us)
{
    uint64_t next = UINT64_MAX;
    for (uint8_t port = 0; port < CAT_PORTS; port++) {
        const cat_port_t *p = &c->port[port];
        if (!p->open) {
            continue;
        }
        if (p->rxq_len) {
            uint64_t due = p->rx_first_us + CAT_BATCH_US;
            if (due < next) {
                next = due;
            }
        }
        if (p->txq_len && now_us + CAT_PUMP_US < next) {
            next = now_us + CAT_PUMP_US;
        }
    }
    return next;
}

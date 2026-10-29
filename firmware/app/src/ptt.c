/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * PTT controller (protocol/SPEC.md §8). See ptt.h.
 *
 * Every event runs the same steps: change the inputs (requests, line
 * arming, fail-safe flags), then update() recomputes the sources, drives the
 * outputs, restarts the timers and reports PTT_STATUS if anything changed.
 */
#include "ptt.h"

#include <string.h>

#define REASON_AUTO 0xFFu /* RELEASED if PTT went off, NONE otherwise */

/* Sources that drive the PTT_TARGETS outputs. A pass-through line is a
 * "possible PTT" (SPEC §8.3): it needs the keepalive, counts toward max TX
 * and drops at session end, but it reaches the radio only through its own
 * line, never through the AUDIO-jack closure. */
#define DRIVING_SOURCES (PTT_SRC_SET | PTT_SRC_LINE | PTT_SRC_TONE | PTT_SRC_NATIVE_LINE)

static const uint8_t line_bit[2] = {PTT_LINE_DTR, PTT_LINE_RTS};

int ptt_port_index(uint8_t port)
{
    if (port <= 4) {
        return port;
    }
    if (port == PTT_PORT_CONTROL) {
        return 5;
    }
    return -1;
}

static bool is_radio_port(int idx)
{
    return idx >= 1 && idx <= 4;
}

void ptt_config_defaults(ptt_config_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->keepalive_ms = PTT_KEEPALIVE_DEFAULT_MS;
    cfg->max_tx_s = PTT_MAX_TX_DEFAULT_S;
    cfg->targets = PTT_TARGET_CLOSURE; /* closure only (0x01, 0) */
    cfg->usb_port = 0;
    /* Port 0 and 0x0F: RTS -> PTT, DTR ignored. Ports 1-4: both pass-through. */
    cfg->rts_action[0] = PTT_ACT_PTT;
    cfg->dtr_action[0] = PTT_ACT_IGNORE;
    for (int i = 1; i <= 4; i++) {
        cfg->rts_action[i] = PTT_ACT_PASS;
        cfg->dtr_action[i] = PTT_ACT_PASS;
    }
    cfg->rts_action[5] = PTT_ACT_PTT;
    cfg->dtr_action[5] = PTT_ACT_IGNORE;
    ptt_config_sanitize(cfg);
}

bool ptt_config_sanitize(ptt_config_t *cfg)
{
    ptt_config_t before = *cfg;

    /* The keepalive can't be switched off. */
    if (cfg->keepalive_ms < PTT_KEEPALIVE_MIN_MS || cfg->keepalive_ms > PTT_KEEPALIVE_MAX_MS) {
        cfg->keepalive_ms = PTT_KEEPALIVE_DEFAULT_MS;
    }
    /* Max TX: 0 (off) or at least 10 s, no upper bound. */
    if (cfg->max_tx_s != PTT_MAX_TX_DISABLED && cfg->max_tx_s < PTT_MAX_TX_MIN_S) {
        cfg->max_tx_s = PTT_MAX_TX_DEFAULT_S;
    }
    cfg->targets &= (PTT_TARGET_CLOSURE | PTT_TARGET_RTS | PTT_TARGET_DTR);
    if (cfg->targets & (PTT_TARGET_RTS | PTT_TARGET_DTR)) {
        if (cfg->usb_port < 1 || cfg->usb_port > 4) {
            cfg->targets &= PTT_TARGET_CLOSURE;
            cfg->usb_port = 0;
        }
    } else {
        cfg->usb_port = 0;
    }
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        uint8_t *acts[2] = {&cfg->dtr_action[i], &cfg->rts_action[i]};
        for (int j = 0; j < 2; j++) {
            if (*acts[j] > PTT_ACT_PASS || (*acts[j] == PTT_ACT_PASS && !is_radio_port(i))) {
                *acts[j] = PTT_ACT_IGNORE;
            }
        }
    }
    return memcmp(&before, cfg, sizeof(before)) != 0;
}

static uint8_t action(const ptt_t *p, int idx, int j)
{
    if (p->cfg.native_locked && p->port[idx].origin == PTT_ORIGIN_NATIVE) {
        return PTT_ACT_IGNORE; /* WIRED_PORT_LOCK: LINE_MAP ignored on native ports */
    }
    return j == 0 ? p->cfg.dtr_action[idx] : p->cfg.rts_action[idx];
}

/* Pass-through lines that reach the radio's chip on radio port `idx`. */
static uint8_t pass_out(const ptt_t *p, int idx)
{
    uint8_t out = 0;
    if (!is_radio_port(idx) || p->port[idx].pass_dropped || p->locked_out) {
        return 0;
    }
    for (int j = 0; j < 2; j++) {
        if (action(p, idx, j) == PTT_ACT_PASS && (p->port[idx].host_lines & line_bit[j])) {
            out |= line_bit[j];
        }
    }
    return out;
}

static uint8_t compute_sources(const ptt_t *p)
{
    uint8_t s = 0;
    if (p->set_active) {
        s |= PTT_SRC_SET;
    }
    if (p->tone_active) {
        s |= PTT_SRC_TONE;
    }
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        for (int j = 0; j < 2; j++) {
            if (action(p, i, j) == PTT_ACT_PTT && p->port[i].arm[j] == PTT_KEYING) {
                s |= p->port[i].origin == PTT_ORIGIN_NATIVE ? PTT_SRC_NATIVE_LINE : PTT_SRC_LINE;
            }
        }
        if (pass_out(p, i)) {
            s |= PTT_SRC_PASSTHROUGH;
        }
    }
    return s;
}

static void apply_outputs(ptt_t *p, bool force)
{
    bool drive = (p->sources & DRIVING_SOURCES) != 0;
    bool closure = drive && (p->cfg.targets & PTT_TARGET_CLOSURE);
    if (force || closure != p->closure_out) {
        p->closure_out = closure;
        if (p->ops.set_closure) {
            p->ops.set_closure(p->ops.ctx, closure);
        }
    }
    for (int port = 1; port <= 4; port++) {
        uint8_t lines = pass_out(p, port);
        if (drive && p->cfg.usb_port == port) {
            if (p->cfg.targets & PTT_TARGET_RTS) {
                lines |= PTT_LINE_RTS;
            }
            if (p->cfg.targets & PTT_TARGET_DTR) {
                lines |= PTT_LINE_DTR;
            }
        }
        if (force || lines != p->radio_out[port]) {
            p->radio_out[port] = lines;
            if (p->ops.set_radio_lines) {
                p->ops.set_radio_lines(p->ops.ctx, (uint8_t)port, lines);
            }
        }
    }
}

/* Every host request that could key is released (SPEC §8.5 lockout). */
static bool all_requests_released(const ptt_t *p)
{
    if (p->set_requested || p->tone_requested) {
        return false;
    }
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        for (int j = 0; j < 2; j++) {
            if (action(p, i, j) != PTT_ACT_IGNORE && (p->port[i].host_lines & line_bit[j])) {
                return false;
            }
        }
    }
    return true;
}

static uint32_t remaining_s(const ptt_t *p, uint64_t now_ms)
{
    if (!p->on || p->cfg.max_tx_s == PTT_MAX_TX_DISABLED) {
        return 0;
    }
    uint64_t end = p->tx_start_ms + (uint64_t)p->cfg.max_tx_s * 1000u;
    if (now_ms >= end) {
        return 0;
    }
    return (uint32_t)((end - now_ms + 999u) / 1000u);
}

void ptt_status(const ptt_t *p, uint64_t now_ms, ptt_status_t *st)
{
    st->state = p->on ? 1 : 0;
    st->sources = p->sources;
    st->reason = p->last_reason;
    st->remaining_s = remaining_s(p, now_ms);
}

static void emit(ptt_t *p, uint64_t now_ms, uint8_t reason)
{
    ptt_status_t st;
    p->last_reason = reason;
    ptt_status(p, now_ms, &st);
    if (p->suppress_notify) {
        p->changed = true;
    } else if (p->ops.status) {
        p->ops.status(p->ops.ctx, &st);
    }
}

/* Recomputes everything after an event. `reason` applies if PTT state or
 * sources changed; `info` (NOT_ARMED, LOCKED_OUT) is reported otherwise;
 * `force` reports `reason` even without a change. */
static void update_ex(ptt_t *p, uint64_t now_ms, uint8_t reason, uint8_t info, bool force)
{
    uint8_t prev_sources = p->sources;
    bool prev_on = p->on;

    if (p->locked_out && all_requests_released(p)) {
        p->locked_out = false;
    }
    p->sources = compute_sources(p);
    p->on = p->sources != 0;
    if (p->on && !prev_on) {
        p->tx_start_ms = now_ms; /* max TX counts from logical PTT on (§8.5) */
    }
    if ((p->sources & PTT_KEEPALIVE_SOURCES) && !(prev_sources & PTT_KEEPALIVE_SOURCES)) {
        p->keepalive_ms = now_ms; /* keepalive window starts with the first such source */
    }
    apply_outputs(p, false);

    bool changed = p->on != prev_on || p->sources != prev_sources;
    if (reason == REASON_AUTO) {
        reason = p->on ? PTT_R_NONE : (prev_on ? PTT_R_RELEASED : PTT_R_NONE);
    }
    if (changed || force) {
        emit(p, now_ms, reason);
    } else if (info != PTT_R_NONE) {
        emit(p, now_ms, info);
    }
    uint8_t dropped = (uint8_t)(prev_sources & (uint8_t)~p->sources);
    if (dropped && p->ops.sources_dropped) {
        p->ops.sources_dropped(p->ops.ctx, dropped, p->last_reason);
    }
}

static void update(ptt_t *p, uint64_t now_ms, uint8_t reason, uint8_t info)
{
    update_ex(p, now_ms, reason, info, false);
}

static void block_port(ptt_t *p, int idx)
{
    p->port[idx].arm[0] = PTT_BLOCKED;
    p->port[idx].arm[1] = PTT_BLOCKED;
}

void ptt_init(ptt_t *p, const ptt_config_t *cfg, const ptt_ops_t *ops, uint64_t now_ms,
              uint8_t boot_reason)
{
    memset(p, 0, sizeof(*p));
    p->cfg = *cfg;
    ptt_config_sanitize(&p->cfg);
    if (ops) {
        p->ops = *ops;
    }
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        block_port(p, i);
    }
    p->keepalive_ms = now_ms;
    apply_outputs(p, true); /* drive every output off */
    update_ex(p, now_ms, boot_reason, PTT_R_NONE, true);
}

bool ptt_set(ptt_t *p, bool state, uint64_t now_ms, ptt_status_t *st)
{
    uint8_t info = PTT_R_NONE;
    p->suppress_notify = true;
    p->changed = false;
    if (state) {
        p->keepalive_ms = now_ms;
        p->set_requested = true;
        if (p->locked_out) {
            info = PTT_R_LOCKED_OUT;
        } else {
            p->set_active = true;
        }
    } else {
        p->set_requested = false;
        p->set_active = false;
    }
    update(p, now_ms, REASON_AUTO, info);
    p->suppress_notify = false;
    ptt_status(p, now_ms, st);
    return p->changed;
}

void ptt_keepalive(ptt_t *p, uint64_t now_ms)
{
    p->keepalive_ms = now_ms;
}

void ptt_lines(ptt_t *p, uint8_t port, uint8_t lines, uint8_t origin, uint64_t now_ms)
{
    int idx = ptt_port_index(port);
    if (idx < 0) {
        return;
    }
    ptt_port_t *pp = &p->port[idx];
    uint8_t old = pp->host_lines;
    uint8_t now_lines = lines & (PTT_LINE_DTR | PTT_LINE_RTS);
    uint8_t rises = (uint8_t)(now_lines & (uint8_t)~old);
    uint8_t info = PTT_R_NONE;
    uint8_t reason = REASON_AUTO;

    if (origin == PTT_ORIGIN_PROTOCOL) {
        p->keepalive_ms = now_ms; /* MODEM_LINES refreshes the keepalive (§8.2) */
    }
    pp->origin = origin;
    pp->pass_dropped = false; /* the host restates its lines */

    for (int j = 0; j < 2; j++) {
        uint8_t bit = line_bit[j];
        uint8_t act = action(p, idx, j);
        bool level = (now_lines & bit) != 0;
        if (act == PTT_ACT_PTT) {
            uint8_t *arm = &pp->arm[j];
            switch (*arm) {
            case PTT_BLOCKED:
                if (!level) {
                    *arm = PTT_ARMED; /* seen deasserted */
                } else if (rises & bit) {
                    info = p->locked_out ? PTT_R_LOCKED_OUT : PTT_R_NOT_ARMED;
                }
                break;
            case PTT_ARMED:
                if (rises & bit) {
                    if (rises == (PTT_LINE_DTR | PTT_LINE_RTS)) {
                        *arm = PTT_BLOCKED; /* both rose together: a port open */
                        info = PTT_R_NOT_ARMED;
                    } else if (p->locked_out) {
                        info = PTT_R_LOCKED_OUT;
                    } else {
                        *arm = PTT_KEYING;
                    }
                }
                break;
            default: /* PTT_KEYING */
                if (!level) {
                    *arm = PTT_ARMED;
                }
                break;
            }
        } else if (act == PTT_ACT_PASS && is_radio_port(idx)) {
            if ((rises & bit) && p->locked_out) {
                info = PTT_R_LOCKED_OUT;
            }
        }
    }
    pp->host_lines = now_lines;

    /* Both native lines dropping at once is a CDC-ACM port close (§8.6). */
    if (origin == PTT_ORIGIN_NATIVE && old == (PTT_LINE_DTR | PTT_LINE_RTS) && now_lines == 0) {
        block_port(p, idx);
        reason = PTT_R_PORT_CLOSED;
    }
    update(p, now_ms, reason, info);
}

static void reset_port(ptt_t *p, int idx, uint8_t origin)
{
    p->port[idx].host_lines = 0;
    p->port[idx].origin = origin;
    p->port[idx].pass_dropped = false;
    block_port(p, idx);
}

void ptt_port_opened(ptt_t *p, uint8_t port, uint64_t now_ms)
{
    int idx = ptt_port_index(port);
    if (idx < 0) {
        return;
    }
    reset_port(p, idx, PTT_ORIGIN_PROTOCOL);
    update(p, now_ms, PTT_R_PORT_CLOSED, PTT_R_NONE);
}

void ptt_port_closed(ptt_t *p, uint8_t port, uint64_t now_ms)
{
    int idx = ptt_port_index(port);
    if (idx < 0) {
        return;
    }
    reset_port(p, idx, p->port[idx].origin);
    update(p, now_ms, PTT_R_PORT_CLOSED, PTT_R_NONE);
}

void ptt_native_reset(ptt_t *p, uint64_t now_ms)
{
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        if (p->port[i].origin == PTT_ORIGIN_NATIVE) {
            reset_port(p, i, PTT_ORIGIN_NATIVE);
        }
    }
    update(p, now_ms, PTT_R_LINK_LOST, PTT_R_NONE);
}

bool ptt_tone(ptt_t *p, bool key, uint64_t now_ms)
{
    uint8_t info = PTT_R_NONE;
    if (key) {
        p->tone_requested = true;
        if (p->locked_out) {
            info = PTT_R_LOCKED_OUT;
        } else {
            p->tone_active = true;
        }
    } else {
        p->tone_requested = false;
        p->tone_active = false;
    }
    update(p, now_ms, REASON_AUTO, info);
    return p->tone_active;
}

void ptt_tone_done(ptt_t *p, uint64_t now_ms)
{
    p->tone_requested = false;
    p->tone_active = false;
    update(p, now_ms, p->sources == PTT_SRC_TONE ? PTT_R_SEQUENCE_DONE : REASON_AUTO,
           PTT_R_NONE);
}

/* Releases every source and blocks every line. Native line levels are kept:
 * they are the USB host's real state, and a BLOCKED line must be seen
 * deasserted before it can key again. */
static void release_all(ptt_t *p)
{
    p->set_requested = false;
    p->set_active = false;
    p->tone_requested = false;
    p->tone_active = false;
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        if (p->port[i].origin == PTT_ORIGIN_PROTOCOL) {
            p->port[i].host_lines = 0; /* §8.7: every port closes */
        }
        p->port[i].pass_dropped = false;
        block_port(p, i);
    }
    p->locked_out = false;
}

void ptt_all_off(ptt_t *p, uint8_t reason, uint64_t now_ms)
{
    release_all(p);
    update(p, now_ms, reason, PTT_R_NONE);
}

void ptt_configure(ptt_t *p, const ptt_config_t *cfg, uint64_t now_ms)
{
    ptt_config_t next = *cfg;
    ptt_config_sanitize(&next);
    bool targets_changed = next.targets != p->cfg.targets || next.usb_port != p->cfg.usb_port;
    bool map_changed[PTT_NUM_PORTS];
    bool any_map_changed = false;
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        map_changed[i] = next.rts_action[i] != p->cfg.rts_action[i] ||
                         next.dtr_action[i] != p->cfg.dtr_action[i] ||
                         (next.native_locked != p->cfg.native_locked &&
                          p->port[i].origin == PTT_ORIGIN_NATIVE);
        any_map_changed = any_map_changed || map_changed[i];
    }

    if (p->on && (targets_changed || any_map_changed)) {
        /* Release PTT first (§8.5). Requests stay as they are for the lockout. */
        p->set_active = false;
        p->set_requested = false;
        p->tone_active = false;
        p->tone_requested = false;
        for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
            block_port(p, i);
            p->port[i].pass_dropped = true;
        }
        update(p, now_ms, PTT_R_MODE_CHANGE, PTT_R_NONE);
    }
    for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
        if (map_changed[i]) {
            /* A newly mapped line must be seen deasserted first, and a newly
             * passed-through line waits for the host's next line update. */
            block_port(p, i);
            p->port[i].pass_dropped = true;
        }
    }
    p->cfg = next;
    update(p, now_ms, PTT_R_MODE_CHANGE, PTT_R_NONE);
}

void ptt_tick(ptt_t *p, uint64_t now_ms)
{
    if (p->on && p->cfg.max_tx_s != PTT_MAX_TX_DISABLED &&
        now_ms - p->tx_start_ms >= (uint64_t)p->cfg.max_tx_s * 1000u) {
        /* Max TX (§8.5): release every source, block every line, lock out. */
        p->set_active = false;
        p->tone_active = false;
        p->tone_requested = false; /* the device cancels its own sequence */
        for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
            block_port(p, i);
            p->port[i].pass_dropped = true;
        }
        p->locked_out = true;
        update(p, now_ms, PTT_R_MAX_TX, PTT_R_NONE);
        return;
    }
    if ((p->sources & PTT_KEEPALIVE_SOURCES) &&
        now_ms - p->keepalive_ms >= (uint64_t)p->cfg.keepalive_ms) {
        /* Keepalive timeout (§8.2): release sources 0, 1, 2 and 4. */
        p->set_active = false;
        p->set_requested = false;
        p->tone_active = false;
        p->tone_requested = false;
        for (int i = 0; i < (int)PTT_NUM_PORTS; i++) {
            if (p->port[i].origin == PTT_ORIGIN_PROTOCOL) {
                block_port(p, i);
                p->port[i].pass_dropped = true;
            }
        }
        update(p, now_ms, PTT_R_KEEPALIVE_TIMEOUT, PTT_R_NONE);
    }
}

uint8_t ptt_arm_state(const ptt_t *p, uint8_t port, uint8_t line)
{
    int idx = ptt_port_index(port);
    if (idx < 0) {
        return PTT_BLOCKED;
    }
    return p->port[idx].arm[line == PTT_LINE_DTR ? 0 : 1];
}

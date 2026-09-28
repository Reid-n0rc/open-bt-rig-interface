/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * PTT controller: every path of protocol/SPEC.md §8 (REQ-PTT-005 to -010).
 */
#include <string.h>

#include "ptt.h"
#include "unity.h"

/* --- Captured outputs --- */

static struct {
    bool closure;
    unsigned closure_writes;
    uint8_t radio[5];
    ptt_status_t status[64];
    unsigned nstatus;
    uint8_t dropped;
    uint8_t dropped_reason;
} out;

static void set_closure(void *ctx, bool on)
{
    (void)ctx;
    out.closure = on;
    out.closure_writes++;
}

static void set_radio_lines(void *ctx, uint8_t port, uint8_t lines)
{
    (void)ctx;
    TEST_ASSERT_TRUE(port >= 1 && port <= 4);
    out.radio[port] = lines;
}

static void on_status(void *ctx, const ptt_status_t *st)
{
    (void)ctx;
    TEST_ASSERT_TRUE(out.nstatus < 64);
    out.status[out.nstatus++] = *st;
}

static void on_dropped(void *ctx, uint8_t sources, uint8_t reason)
{
    (void)ctx;
    out.dropped |= sources;
    out.dropped_reason = reason;
}

static const ptt_ops_t ops = {set_closure, set_radio_lines, on_status, on_dropped, NULL};

static ptt_t p;
static ptt_config_t cfg;
static uint64_t now;

static const ptt_status_t *last(void)
{
    TEST_ASSERT_TRUE(out.nstatus > 0);
    return &out.status[out.nstatus - 1];
}

static void start(void)
{
    memset(&out, 0, sizeof(out));
    out.closure = true; /* prove init drives it off */
    for (int i = 1; i <= 4; i++) {
        out.radio[i] = 0xFF;
    }
    now = 1000;
    ptt_init(&p, &cfg, &ops, now, PTT_R_BOOT);
    out.nstatus = 0;
}

void setUp(void)
{
    ptt_config_defaults(&cfg);
    start();
}

void tearDown(void)
{
}

static bool key(void)
{
    ptt_status_t st;
    return ptt_set(&p, true, now, &st) && st.state == 1;
}

static void run_until(uint64_t t, uint64_t step)
{
    while (now < t) {
        now += step;
        ptt_tick(&p, now);
    }
}

/* Runs to time t, sending KEEPALIVE every second. */
static void run_with_keepalive(uint64_t t)
{
    while (now < t) {
        now += 50;
        if (now % 1000 == 0) {
            ptt_keepalive(&p, now);
        }
        ptt_tick(&p, now);
    }
}

/* --- Boot and reset (REQ-PTT-005) --- */

static void test_boot_drives_outputs_off_and_reports_boot(void)
{
    memset(&out, 0, sizeof(out));
    out.closure = true;
    for (int i = 1; i <= 4; i++) {
        out.radio[i] = 0xFF;
    }
    ptt_init(&p, &cfg, &ops, 0, PTT_R_BOOT);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_TRUE(out.closure_writes >= 1);
    for (int i = 1; i <= 4; i++) {
        TEST_ASSERT_EQUAL_UINT8(0, out.radio[i]);
    }
    TEST_ASSERT_EQUAL_UINT(1, out.nstatus);
    TEST_ASSERT_EQUAL_UINT8(0, last()->state);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_BOOT, last()->reason);
    for (uint8_t port = 0; port <= 4; port++) {
        TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, port, PTT_LINE_RTS));
        TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, port, PTT_LINE_DTR));
    }
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, PTT_PORT_CONTROL, PTT_LINE_RTS));
}

static void test_watchdog_reset_reports_watchdog(void)
{
    memset(&out, 0, sizeof(out));
    ptt_init(&p, &cfg, &ops, 0, PTT_R_WATCHDOG);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_WATCHDOG, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(0, last()->state);

    memset(&out, 0, sizeof(out));
    ptt_init(&p, &cfg, &ops, 0, PTT_R_FAULT);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, last()->reason);
}

static void test_init_sanitizes_config(void)
{
    ptt_config_t bad = cfg;
    bad.keepalive_ms = 0;
    bad.max_tx_s = 3;
    ptt_init(&p, &bad, &ops, 0, PTT_R_BOOT);
    TEST_ASSERT_EQUAL_UINT16(PTT_KEEPALIVE_DEFAULT_MS, p.cfg.keepalive_ms);
    TEST_ASSERT_EQUAL_UINT32(PTT_MAX_TX_DEFAULT_S, p.cfg.max_tx_s);
    ptt_init(&p, &bad, NULL, 0, PTT_R_BOOT); /* no ops: must not crash */
}

/* --- PTT_SET (§8.1) --- */

static void test_ptt_set_keys_and_releases(void)
{
    ptt_status_t st;
    TEST_ASSERT_TRUE(ptt_set(&p, true, now, &st));
    TEST_ASSERT_EQUAL_UINT8(1, st.state);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_SET, st.sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_NONE, st.reason);
    TEST_ASSERT_EQUAL_UINT32(300, st.remaining_s);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus); /* the caller replies; no duplicate notification */

    TEST_ASSERT_FALSE(ptt_set(&p, true, now, &st)); /* no change */
    TEST_ASSERT_TRUE(st.state);

    now += 500;
    TEST_ASSERT_TRUE(ptt_set(&p, false, now, &st));
    TEST_ASSERT_EQUAL_UINT8(0, st.state);
    TEST_ASSERT_EQUAL_UINT8(0, st.sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_RELEASED, st.reason);
    TEST_ASSERT_EQUAL_UINT32(0, st.remaining_s);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_SET, out.dropped);
}

static void test_remaining_counts_down(void)
{
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 10500);
    ptt_status_t st;
    ptt_status(&p, now, &st);
    TEST_ASSERT_EQUAL_UINT32(290, st.remaining_s); /* 289.5 s left, rounded up */
    ptt_status(&p, now + 290000, &st); /* at the limit, before the next tick */
    TEST_ASSERT_EQUAL_UINT32(0, st.remaining_s);
}

/* --- Keepalive (§8.2) --- */

static void test_keepalive_timeout_releases(void)
{
    TEST_ASSERT_TRUE(key());
    run_until(now + 2950, 50);
    TEST_ASSERT_TRUE(out.closure);
    run_until(now + 50, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, last()->state);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_SET, out.dropped);
    /* A KEEPALIVE after the timeout doesn't re-key. */
    ptt_keepalive(&p, now);
    ptt_tick(&p, now + 10);
    TEST_ASSERT_FALSE(out.closure);
}

static void test_keepalive_messages_hold_ptt(void)
{
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 60000);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus);
}

static void test_repeated_ptt_set_counts_as_keepalive(void)
{
    ptt_status_t st;
    TEST_ASSERT_TRUE(key());
    for (int i = 0; i < 10; i++) {
        run_until(now + 2000, 50);
        (void)ptt_set(&p, true, now, &st);
    }
    TEST_ASSERT_TRUE(out.closure);
}

static void test_modem_lines_count_as_keepalive(void)
{
    TEST_ASSERT_TRUE(key());
    for (int i = 0; i < 10; i++) {
        run_until(now + 2000, 50);
        ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    }
    TEST_ASSERT_TRUE(out.closure);
}

static void test_ptt_set_zero_is_not_a_keepalive(void)
{
    ptt_status_t st;
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);            /* arm RTS */
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now); /* key with RTS */
    TEST_ASSERT_TRUE(out.closure);
    run_until(now + 2000, 50);
    (void)ptt_set(&p, false, now, &st);
    run_until(now + 1000, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
}

static void test_keepalive_window_starts_with_first_source(void)
{
    /* Nothing keyed for 20 s, then PTT_SET: the timeout counts from now. */
    run_until(now + 20000, 50);
    TEST_ASSERT_TRUE(key());
    run_until(now + 2900, 50);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_keepalive_timeout_keeps_native_lines(void)
{
    /* Wired CDC-ACM line (no keepalive) plus PTT_SET (keepalive). */
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_TRUE(key());
    run_until(now + 3000, 50);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(1, last()->state);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_NATIVE_LINE, last()->sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_KEYING, ptt_arm_state(&p, PTT_PORT_CONTROL, PTT_LINE_RTS));
}

static void test_keepalive_timeout_blocks_protocol_lines(void)
{
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_LINE, p.sources);
    run_until(now + 3000, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    /* The host comes back with RTS still high: that doesn't key. */
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    /* Drop and raise again: keys. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
}

/* --- Arming (§8.4) --- */

static void test_line_rise_from_blocked_does_not_key(void)
{
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_NOT_ARMED, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(0, last()->state);
}

static void test_line_seen_low_then_rising_alone_keys(void)
{
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_ARMED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_KEYING, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_LINE, last()->sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_NONE, last()->reason);
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_ARMED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_RELEASED, last()->reason);
}

static void test_both_lines_rising_is_a_port_open(void)
{
    /* Linux raises DTR and RTS together when a port opens. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_NOT_ARMED, last()->reason);
    /* Software that keys with RTS drops it first, then raises it. */
    ptt_lines(&p, 0, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_ARMED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    ptt_lines(&p, 0, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_both_rising_with_dtr_mapped_too(void)
{
    cfg.dtr_action[0] = PTT_ACT_PTT;
    start();
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_DTR));
    /* DTR alone keys once armed. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_ignored_line_never_keys(void)
{
    /* Default: DTR ignored on port 0. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus);
}

static void test_serial_open_blocks_and_releases(void)
{
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
    ptt_port_opened(&p, 0, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_PORT_CLOSED, last()->reason);
    /* After the open, a lone rise without first seeing the line low doesn't key. */
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
}

static void test_port_close_releases(void)
{
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    ptt_port_closed(&p, 0, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_PORT_CLOSED, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    TEST_ASSERT_EQUAL_UINT8(0, p.port[0].host_lines);
}

static void test_invalid_port_is_ignored(void)
{
    ptt_lines(&p, 7, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    ptt_port_opened(&p, 7, now);
    ptt_port_closed(&p, 9, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 7, PTT_LINE_RTS));
    TEST_ASSERT_EQUAL_INT(-1, ptt_port_index(5));
    TEST_ASSERT_EQUAL_INT(5, ptt_port_index(PTT_PORT_CONTROL));
}

/* --- Wired native lines (§14.3) --- */

static void test_native_line_needs_no_keepalive(void)
{
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_NATIVE_LINE, last()->sources);
    run_until(now + 20000, 50);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_native_port_open_does_not_key(void)
{
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_FALSE(out.closure);
}

static void test_native_close_drop_is_port_closed(void)
{
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_TRUE(out.closure);
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now); /* both drop at close */
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_PORT_CLOSED, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, PTT_PORT_CONTROL, PTT_LINE_RTS));
}

static void test_native_lock_ignores_native_lines(void)
{
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_TRUE(out.closure);
    ptt_config_t next = cfg;
    next.native_locked = true;
    ptt_configure(&p, &next, now);
    TEST_ASSERT_FALSE(out.closure); /* released first (SPEC §15.2) */
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MODE_CHANGE, last()->reason);
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_FALSE(out.closure);
    /* Protocol lines still work. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_usb_reset_releases_native_lines(void)
{
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    ptt_native_reset(&p, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LINK_LOST, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, PTT_PORT_CONTROL, PTT_LINE_RTS));
}

/* --- Pass-through (§8.3) --- */

static void test_passthrough_follows_host_and_needs_keepalive(void)
{
    ptt_lines(&p, 1, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_DTR, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_PASSTHROUGH, last()->sources);
    TEST_ASSERT_EQUAL_UINT8(1, last()->state);
    /* A pass-through line is a possible PTT, but it doesn't drive the closure. */
    TEST_ASSERT_FALSE(out.closure);
    run_until(now + 3000, 50);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
    /* The host restates its lines: they pass through again. */
    ptt_lines(&p, 1, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_DTR, out.radio[1]);
}

static void test_passthrough_not_armed_at_port_open(void)
{
    /* Pass-through shows what a direct cable would: both lines rise. */
    ptt_lines(&p, 2, PTT_LINE_DTR | PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_DTR | PTT_LINE_RTS, out.radio[2]);
    ptt_lines(&p, 2, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[2]);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_RELEASED, last()->reason);
}

static void test_passthrough_counts_toward_max_tx(void)
{
    cfg.max_tx_s = 10;
    start();
    ptt_lines(&p, 1, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    run_with_keepalive(now + 10000);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MAX_TX, last()->reason);
    /* Locked out: asserting again is refused. */
    ptt_lines(&p, 1, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LOCKED_OUT, last()->reason);
    /* Release everything: the lockout clears and the lines pass again. */
    ptt_lines(&p, 1, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(p.locked_out);
    ptt_lines(&p, 1, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_RTS, out.radio[1]);
}

static void test_radio_ports_default_to_passthrough(void)
{
    /* SPEC §8.3 (approved 2026-09-28): radio ports 1-4 pass both lines
     * through by default, and that never keys the AUDIO-jack closure. */
    ptt_config_t d;
    ptt_config_defaults(&d);
    for (int i = 1; i <= 4; i++) {
        TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PASS, d.rts_action[i]);
        TEST_ASSERT_EQUAL_UINT8(PTT_ACT_PASS, d.dtr_action[i]);
    }
    for (uint8_t port = 1; port <= 4; port++) {
        ptt_lines(&p, port, 0, PTT_ORIGIN_PROTOCOL, now);
        ptt_lines(&p, port, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
        TEST_ASSERT_EQUAL_UINT8(PTT_LINE_RTS, out.radio[port]); /* forwarded to the chip */
        TEST_ASSERT_FALSE(out.closure);                         /* the closure stays open */
        TEST_ASSERT_TRUE(p.sources & PTT_SRC_PASSTHROUGH);
        ptt_lines(&p, port, 0, PTT_ORIGIN_PROTOCOL, now);
    }
}

static void test_radio_port_action_ptt_keys_the_closure(void)
{
    /* Action 1 on a radio port uses the device's own hardware PTT. */
    cfg.rts_action[2] = PTT_ACT_PTT;
    start();
    ptt_lines(&p, 2, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_ARMED, ptt_arm_state(&p, 2, PTT_LINE_RTS));
    ptt_lines(&p, 2, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[2]); /* not forwarded to the chip */
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_LINE, last()->sources);
    /* The arming rules apply: both lines rising together don't key. */
    ptt_lines(&p, 2, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    ptt_lines(&p, 2, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    /* DTR stays pass-through on the same port. */
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_DTR, out.radio[2]);
    /* The keepalive still applies. */
    ptt_lines(&p, 2, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 2, PTT_LINE_RTS | PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
    run_until(now + 3000, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
}

static void test_passthrough_not_allowed_on_port_zero(void)
{
    cfg.rts_action[0] = PTT_ACT_PASS;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_IGNORE, cfg.rts_action[0]);
}

/* --- Outputs (PTT_TARGETS) --- */

static void test_targets_rts_on_radio_port(void)
{
    cfg.targets = PTT_TARGET_RTS;
    cfg.usb_port = 2;
    start();
    TEST_ASSERT_TRUE(key());
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_RTS, out.radio[2]);
    ptt_status_t st;
    (void)ptt_set(&p, false, now, &st);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[2]);
}

static void test_targets_closure_and_dtr(void)
{
    cfg.targets = PTT_TARGET_CLOSURE | PTT_TARGET_DTR;
    cfg.usb_port = 4;
    start();
    TEST_ASSERT_TRUE(key());
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_LINE_DTR, out.radio[4]);
}

static void test_targets_without_port_fall_back_to_closure(void)
{
    cfg.targets = PTT_TARGET_RTS | PTT_TARGET_CLOSURE;
    cfg.usb_port = 0;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT8(PTT_TARGET_CLOSURE, cfg.targets);
    cfg.targets = PTT_TARGET_CLOSURE | 0x80;
    cfg.usb_port = 3;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT8(PTT_TARGET_CLOSURE, cfg.targets);
    TEST_ASSERT_EQUAL_UINT8(0, cfg.usb_port);
}

/* --- Max TX (§8.5) --- */

static void test_max_tx_releases_and_locks_out(void)
{
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 299950);
    TEST_ASSERT_TRUE(out.closure);
    run_with_keepalive(now + 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MAX_TX, last()->reason);
    TEST_ASSERT_TRUE(p.locked_out);

    ptt_status_t st;
    TEST_ASSERT_TRUE(ptt_set(&p, true, now, &st)); /* key attempt: reported */
    TEST_ASSERT_EQUAL_UINT8(0, st.state);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LOCKED_OUT, st.reason);
    TEST_ASSERT_FALSE(out.closure);

    (void)ptt_set(&p, false, now, &st); /* every source released */
    TEST_ASSERT_FALSE(p.locked_out);
    TEST_ASSERT_TRUE(key());
    TEST_ASSERT_TRUE(out.closure);
}

static void test_max_tx_lockout_waits_for_lines(void)
{
    cfg.max_tx_s = 10;
    start();
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    run_with_keepalive(now + 10000);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_TRUE(p.locked_out);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
    /* PTT_SET is refused while RTS is still held. */
    ptt_status_t st;
    (void)ptt_set(&p, true, now, &st);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LOCKED_OUT, st.reason);
    (void)ptt_set(&p, false, now, &st);
    TEST_ASSERT_TRUE(p.locked_out);
    /* A native line rising elsewhere is refused too. */
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LOCKED_OUT, last()->reason);
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    /* RTS drops: the lockout clears and the line re-arms. */
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(p.locked_out);
    TEST_ASSERT_EQUAL_UINT8(PTT_ARMED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
}

static void test_max_tx_counts_from_first_source(void)
{
    cfg.max_tx_s = 20;
    start();
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 15000);
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now); /* a second source */
    run_with_keepalive(now + 5000);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MAX_TX, last()->reason);
}

static void test_max_tx_long_value(void)
{
    cfg.max_tx_s = 100000; /* no upper bound */
    start();
    TEST_ASSERT_TRUE(key());
    ptt_status_t st;
    ptt_status(&p, now, &st);
    TEST_ASSERT_EQUAL_UINT32(100000, st.remaining_s);
    run_with_keepalive(now + 3600000);
    TEST_ASSERT_TRUE(out.closure);
}

static void test_max_tx_disabled_keeps_other_fail_safes(void)
{
    cfg.max_tx_s = PTT_MAX_TX_DISABLED;
    start();
    TEST_ASSERT_EQUAL_UINT32(0, p.cfg.max_tx_s);
    TEST_ASSERT_TRUE(key());
    ptt_status_t st;
    ptt_status(&p, now, &st);
    TEST_ASSERT_EQUAL_UINT32(0, st.remaining_s);
    run_with_keepalive(now + 2ull * 3600ull * 1000ull); /* two hours */
    TEST_ASSERT_TRUE(out.closure);
    /* The keepalive still drops PTT. */
    run_until(now + 3000, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
    /* Disconnect still drops PTT. */
    TEST_ASSERT_TRUE(key());
    ptt_all_off(&p, PTT_R_LINK_LOST, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LINK_LOST, last()->reason);
    /* So does a native line's port close. */
    ptt_lines(&p, PTT_PORT_CONTROL, 0, PTT_ORIGIN_NATIVE, now);
    ptt_lines(&p, PTT_PORT_CONTROL, PTT_LINE_RTS, PTT_ORIGIN_NATIVE, now);
    ptt_native_reset(&p, now);
    TEST_ASSERT_FALSE(out.closure);
}

static void test_max_tx_sanitize(void)
{
    cfg.max_tx_s = 9;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT32(PTT_MAX_TX_DEFAULT_S, cfg.max_tx_s);
    cfg.max_tx_s = 0;
    TEST_ASSERT_FALSE(ptt_config_sanitize(&cfg));
    cfg.max_tx_s = 10;
    TEST_ASSERT_FALSE(ptt_config_sanitize(&cfg));
    cfg.keepalive_ms = 10001;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT16(PTT_KEEPALIVE_DEFAULT_MS, cfg.keepalive_ms);
    cfg.rts_action[3] = 7;
    TEST_ASSERT_TRUE(ptt_config_sanitize(&cfg));
    TEST_ASSERT_EQUAL_UINT8(PTT_ACT_IGNORE, cfg.rts_action[3]);
}

/* --- Session end, mode change, faults (§8.5, §8.7) --- */

static void test_session_end_turns_everything_off(void)
{
    TEST_ASSERT_TRUE(key());
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 1, PTT_LINE_DTR, PTT_ORIGIN_PROTOCOL, now);
    ptt_all_off(&p, PTT_R_LINK_LOST, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(0, last()->state);
    TEST_ASSERT_EQUAL_UINT8(0, last()->sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LINK_LOST, last()->reason);
    for (uint8_t port = 0; port <= 4; port++) {
        TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, port, PTT_LINE_RTS));
        TEST_ASSERT_EQUAL_UINT8(0, p.port[port].host_lines);
    }
}

static void test_session_end_clears_lockout(void)
{
    cfg.max_tx_s = 10;
    start();
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 10000);
    TEST_ASSERT_TRUE(p.locked_out);
    ptt_all_off(&p, PTT_R_LINK_LOST, now);
    TEST_ASSERT_FALSE(p.locked_out);
    TEST_ASSERT_TRUE(key());
}

static void test_mode_change_and_fault(void)
{
    TEST_ASSERT_TRUE(key());
    ptt_all_off(&p, PTT_R_MODE_CHANGE, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MODE_CHANGE, last()->reason);
    TEST_ASSERT_TRUE(key());
    ptt_all_off(&p, PTT_R_FAULT, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_FAULT, last()->reason);
    /* Off already: nothing to report. */
    unsigned n = out.nstatus;
    ptt_all_off(&p, PTT_R_LINK_LOST, now);
    TEST_ASSERT_EQUAL_UINT(n, out.nstatus);
}

/* --- Configuration changes never assert PTT (§8.5, REQ-PTT-009) --- */

static void test_target_change_while_keyed_releases(void)
{
    TEST_ASSERT_TRUE(key());
    ptt_config_t next = cfg;
    next.targets = PTT_TARGET_RTS;
    next.usb_port = 1;
    ptt_configure(&p, &next, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MODE_CHANGE, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(0, p.sources);
}

static void test_line_map_change_while_keyed_releases(void)
{
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_TRUE(out.closure);
    ptt_config_t next = cfg;
    next.dtr_action[0] = PTT_ACT_PTT;
    ptt_configure(&p, &next, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MODE_CHANGE, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
}

static void test_line_map_change_never_asserts(void)
{
    /* The host holds RTS high on port 0 (ignored) and on port 1 (ignored). */
    cfg.rts_action[0] = PTT_ACT_IGNORE;
    cfg.rts_action[1] = PTT_ACT_IGNORE;
    start();
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 1, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    ptt_config_t next = cfg;
    next.rts_action[0] = PTT_ACT_PTT;  /* now mapped to PTT */
    next.rts_action[1] = PTT_ACT_PASS; /* now passed through */
    ptt_configure(&p, &next, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[1]);
    TEST_ASSERT_EQUAL_UINT8(0, p.sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_BLOCKED, ptt_arm_state(&p, 0, PTT_LINE_RTS));
}

static void test_target_change_never_asserts(void)
{
    ptt_config_t next = cfg;
    next.targets = PTT_TARGET_CLOSURE | PTT_TARGET_RTS | PTT_TARGET_DTR;
    next.usb_port = 3;
    ptt_configure(&p, &next, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(0, out.radio[3]);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus);
}

static void test_timer_change_keeps_ptt(void)
{
    TEST_ASSERT_TRUE(key());
    ptt_config_t next = cfg;
    next.keepalive_ms = 5000;
    next.max_tx_s = 600;
    ptt_configure(&p, &next, now);
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT(0, out.nstatus);
    /* Shortening max TX below the elapsed time ends TX at the next tick. */
    run_with_keepalive(now + 20000);
    next.max_tx_s = 10;
    ptt_configure(&p, &next, now);
    ptt_tick(&p, now + 1);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_MAX_TX, last()->reason);
}

/* --- Tone sequence source (§11) --- */

static void test_tone_source(void)
{
    TEST_ASSERT_TRUE(ptt_tone(&p, true, now));
    TEST_ASSERT_TRUE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_TONE, last()->sources);
    ptt_tone_done(&p, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_SEQUENCE_DONE, last()->reason);
    TEST_ASSERT_TRUE(ptt_tone(&p, true, now));
    TEST_ASSERT_FALSE(ptt_tone(&p, false, now));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_RELEASED, last()->reason);
}

static void test_tone_needs_keepalive(void)
{
    TEST_ASSERT_TRUE(ptt_tone(&p, true, now));
    run_until(now + 3000, 50);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, last()->reason);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_TONE, out.dropped);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_KEEPALIVE_TIMEOUT, out.dropped_reason);
}

static void test_tone_refused_in_lockout(void)
{
    cfg.max_tx_s = 10;
    start();
    TEST_ASSERT_TRUE(ptt_tone(&p, true, now));
    run_with_keepalive(now + 10000);
    TEST_ASSERT_FALSE(out.closure);
    /* The device cancelled its own sequence, so the lockout is already clear. */
    TEST_ASSERT_FALSE(p.locked_out);
    TEST_ASSERT_TRUE(key());
    run_with_keepalive(now + 10000);
    TEST_ASSERT_TRUE(p.locked_out);
    TEST_ASSERT_FALSE(ptt_tone(&p, true, now));
    TEST_ASSERT_EQUAL_UINT8(PTT_R_LOCKED_OUT, last()->reason);
}

/* --- Several sources --- */

static void test_two_sources(void)
{
    TEST_ASSERT_TRUE(key());
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    ptt_lines(&p, 0, PTT_LINE_RTS, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_SET | PTT_SRC_LINE, last()->sources);
    ptt_status_t st;
    (void)ptt_set(&p, false, now, &st);
    TEST_ASSERT_EQUAL_UINT8(1, st.state);
    TEST_ASSERT_EQUAL_UINT8(PTT_SRC_LINE, st.sources);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_NONE, st.reason);
    TEST_ASSERT_TRUE(out.closure);
    ptt_lines(&p, 0, 0, PTT_ORIGIN_PROTOCOL, now);
    TEST_ASSERT_FALSE(out.closure);
    TEST_ASSERT_EQUAL_UINT8(PTT_R_RELEASED, last()->reason);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_boot_drives_outputs_off_and_reports_boot);
    RUN_TEST(test_watchdog_reset_reports_watchdog);
    RUN_TEST(test_init_sanitizes_config);
    RUN_TEST(test_ptt_set_keys_and_releases);
    RUN_TEST(test_remaining_counts_down);
    RUN_TEST(test_keepalive_timeout_releases);
    RUN_TEST(test_keepalive_messages_hold_ptt);
    RUN_TEST(test_repeated_ptt_set_counts_as_keepalive);
    RUN_TEST(test_modem_lines_count_as_keepalive);
    RUN_TEST(test_ptt_set_zero_is_not_a_keepalive);
    RUN_TEST(test_keepalive_window_starts_with_first_source);
    RUN_TEST(test_keepalive_timeout_keeps_native_lines);
    RUN_TEST(test_keepalive_timeout_blocks_protocol_lines);
    RUN_TEST(test_line_rise_from_blocked_does_not_key);
    RUN_TEST(test_line_seen_low_then_rising_alone_keys);
    RUN_TEST(test_both_lines_rising_is_a_port_open);
    RUN_TEST(test_both_rising_with_dtr_mapped_too);
    RUN_TEST(test_ignored_line_never_keys);
    RUN_TEST(test_serial_open_blocks_and_releases);
    RUN_TEST(test_port_close_releases);
    RUN_TEST(test_invalid_port_is_ignored);
    RUN_TEST(test_native_line_needs_no_keepalive);
    RUN_TEST(test_native_port_open_does_not_key);
    RUN_TEST(test_native_close_drop_is_port_closed);
    RUN_TEST(test_native_lock_ignores_native_lines);
    RUN_TEST(test_usb_reset_releases_native_lines);
    RUN_TEST(test_passthrough_follows_host_and_needs_keepalive);
    RUN_TEST(test_passthrough_not_armed_at_port_open);
    RUN_TEST(test_passthrough_counts_toward_max_tx);
    RUN_TEST(test_radio_ports_default_to_passthrough);
    RUN_TEST(test_radio_port_action_ptt_keys_the_closure);
    RUN_TEST(test_passthrough_not_allowed_on_port_zero);
    RUN_TEST(test_targets_rts_on_radio_port);
    RUN_TEST(test_targets_closure_and_dtr);
    RUN_TEST(test_targets_without_port_fall_back_to_closure);
    RUN_TEST(test_max_tx_releases_and_locks_out);
    RUN_TEST(test_max_tx_lockout_waits_for_lines);
    RUN_TEST(test_max_tx_counts_from_first_source);
    RUN_TEST(test_max_tx_long_value);
    RUN_TEST(test_max_tx_disabled_keeps_other_fail_safes);
    RUN_TEST(test_max_tx_sanitize);
    RUN_TEST(test_session_end_turns_everything_off);
    RUN_TEST(test_session_end_clears_lockout);
    RUN_TEST(test_mode_change_and_fault);
    RUN_TEST(test_target_change_while_keyed_releases);
    RUN_TEST(test_line_map_change_while_keyed_releases);
    RUN_TEST(test_line_map_change_never_asserts);
    RUN_TEST(test_target_change_never_asserts);
    RUN_TEST(test_timer_change_keeps_ptt);
    RUN_TEST(test_tone_source);
    RUN_TEST(test_tone_needs_keepalive);
    RUN_TEST(test_tone_refused_in_lockout);
    RUN_TEST(test_two_sources);
    return UNITY_END();
}

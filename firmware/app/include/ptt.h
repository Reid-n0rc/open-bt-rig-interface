/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * PTT controller: protocol/SPEC.md §8 (approved by the maintainer on
 * 2026-09-24, amended 2026-09-25) and docs/architecture.md §5.
 *
 * One logical PTT, on while any source is active and no fail-safe blocks it.
 * Pure logic: time comes in as milliseconds of device time, and the outputs
 * go out through ptt_ops_t, so the host tests drive every path.
 *
 * Changes to this module affect the PTT fail-safes and need the maintainer's
 * explicit approval in the PR (GOVERNANCE.md, Safety and compliance).
 */
#ifndef PTT_H
#define PTT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PTT_STATUS.sources bits (SPEC §8.1). */
enum ptt_source {
    PTT_SRC_SET = 1u << 0,         /* PTT_SET state 1 */
    PTT_SRC_LINE = 1u << 1,        /* RTS/DTR mapped to PTT, sent in MODEM_LINES */
    PTT_SRC_TONE = 1u << 2,        /* tone sequence with device-keyed PTT (§11) */
    PTT_SRC_NATIVE_LINE = 1u << 3, /* native RTS/DTR on a wired CDC-ACM port (§14) */
    PTT_SRC_PASSTHROUGH = 1u << 4, /* asserted pass-through line on a radio port (§8.3) */
};
/* Sources that need the keepalive (§8.2): all except native wired lines. */
#define PTT_KEEPALIVE_SOURCES (PTT_SRC_SET | PTT_SRC_LINE | PTT_SRC_TONE | PTT_SRC_PASSTHROUGH)

/* PTT_STATUS.reason (SPEC §8.6). */
enum ptt_reason {
    PTT_R_NONE = 0,
    PTT_R_RELEASED = 1,
    PTT_R_KEEPALIVE_TIMEOUT = 2,
    PTT_R_MAX_TX = 3,
    PTT_R_LINK_LOST = 4,
    PTT_R_PORT_CLOSED = 5,
    PTT_R_MODE_CHANGE = 6,
    PTT_R_NOT_ARMED = 7,
    PTT_R_LOCKED_OUT = 8,
    PTT_R_FAULT = 9,
    PTT_R_SEQUENCE_DONE = 10,
    PTT_R_BOOT = 11,
    PTT_R_WATCHDOG = 12, /* the internal watchdog reset the device (§8.5) */
};

/* MODEM_LINES.lines bits (SPEC §7.1). */
#define PTT_LINE_DTR 0x01u
#define PTT_LINE_RTS 0x02u

/* PTT_TARGETS.targets bits (SPEC §6.1). */
#define PTT_TARGET_CLOSURE 0x01u
#define PTT_TARGET_RTS 0x02u
#define PTT_TARGET_DTR 0x04u

/* LINE_MAP actions (SPEC §8.3). */
enum ptt_action { PTT_ACT_IGNORE = 0, PTT_ACT_PTT = 1, PTT_ACT_PASS = 2 };

/* Per-line arming state (SPEC §8.4). */
enum ptt_arm { PTT_BLOCKED = 0, PTT_ARMED = 1, PTT_KEYING = 2 };

/* Where a port's line state comes from. */
enum ptt_origin {
    PTT_ORIGIN_PROTOCOL = 0, /* MODEM_LINES (Bluetooth or the USB network) */
    PTT_ORIGIN_NATIVE = 1,   /* CDC-ACM SET_CONTROL_LINE_STATE (wired) */
};

/* Ports: 0 = SERIAL jack, 1-4 = radio USB-serial, 0x0F = wired control port. */
#define PTT_PORT_CONTROL 0x0Fu
#define PTT_NUM_PORTS 6u /* indexes 0-4, and 5 for port 0x0F */

/* Limits from SPEC §6.1 (the PTT TLV reports them). */
#define PTT_KEEPALIVE_MIN_MS 500u
#define PTT_KEEPALIVE_MAX_MS 10000u
#define PTT_KEEPALIVE_DEFAULT_MS 3000u
#define PTT_MAX_TX_MIN_S 10u      /* no upper bound (maintainer, 2026-09-25) */
#define PTT_MAX_TX_DEFAULT_S 300u
#define PTT_MAX_TX_DISABLED 0u    /* MAX_TX_S 0: the max-TX timer is off */

typedef struct {
    uint16_t keepalive_ms;
    uint32_t max_tx_s;    /* 0 = off, else >= PTT_MAX_TX_MIN_S */
    uint8_t targets;      /* PTT_TARGET_* */
    uint8_t usb_port;     /* 1-4 when targets has RTS/DTR, else 0 */
    uint8_t rts_action[PTT_NUM_PORTS];
    uint8_t dtr_action[PTT_NUM_PORTS];
    bool native_locked; /* WIRED_PORT_LOCK >= 1: native lines never key (SPEC §15.2) */
} ptt_config_t;

typedef struct {
    uint8_t state;   /* logical PTT */
    uint8_t sources; /* PTT_SRC_* */
    uint8_t reason;  /* PTT_R_* */
    uint32_t remaining_s; /* max-TX time left; 0 when off or when the timer is off */
} ptt_status_t;

typedef struct {
    void (*set_closure)(void *ctx, bool on);                          /* AUDIO-jack closure */
    void (*set_radio_lines)(void *ctx, uint8_t port, uint8_t lines);  /* port 1-4, PTT_LINE_* */
    void (*status)(void *ctx, const ptt_status_t *st);                /* notification (token 0) */
    void (*sources_dropped)(void *ctx, uint8_t sources, uint8_t reason); /* e.g. cancel a tone */
    void *ctx;
} ptt_ops_t;

typedef struct {
    uint8_t host_lines;  /* last line state from the host (PTT_LINE_*) */
    uint8_t origin;      /* ptt_origin of the last update */
    uint8_t arm[2];      /* [0] DTR, [1] RTS: ptt_arm */
    bool pass_dropped;   /* pass-through held off until the next update */
} ptt_port_t;

typedef struct {
    ptt_config_t cfg;
    ptt_ops_t ops;
    ptt_port_t port[PTT_NUM_PORTS];
    bool set_requested; /* host's PTT_SET state */
    bool set_active;
    bool tone_requested;
    bool tone_active;
    bool locked_out; /* after MAX_TX until every source is released (§8.5) */
    bool on;
    uint8_t sources;
    uint64_t tx_start_ms;
    uint64_t keepalive_ms; /* time of the last keepalive refresh */
    bool closure_out;
    uint8_t radio_out[5]; /* index 1-4 */
    bool suppress_notify;
    bool changed;
    uint8_t last_reason;
} ptt_t;

/* Default configuration (SPEC §6.1 defaults). */
void ptt_config_defaults(ptt_config_t *cfg);
/* Puts every out-of-range value back to its default. Returns true if it
 * changed anything. */
bool ptt_config_sanitize(ptt_config_t *cfg);
/* Maps port 0-4 / 0x0F to an index, or -1. */
int ptt_port_index(uint8_t port);

/* Power-on or reset: outputs off, every line BLOCKED, and a status with
 * `boot_reason` (PTT_R_BOOT, PTT_R_FAULT or PTT_R_WATCHDOG). */
void ptt_init(ptt_t *p, const ptt_config_t *cfg, const ptt_ops_t *ops, uint64_t now_ms,
              uint8_t boot_reason);

/* PTT_SET. Refreshes the keepalive when state is 1. Fills *st with the
 * status for the reply and returns true if PTT state or sources changed.
 * Doesn't call ops.status (the caller replies). */
bool ptt_set(ptt_t *p, bool state, uint64_t now_ms, ptt_status_t *st);

/* KEEPALIVE. */
void ptt_keepalive(ptt_t *p, uint64_t now_ms);

/* A line update: MODEM_LINES (origin PROTOCOL, refreshes the keepalive) or
 * a native CDC-ACM line change (origin NATIVE). */
void ptt_lines(ptt_t *p, uint8_t port, uint8_t lines, uint8_t origin, uint64_t now_ms);

/* SERIAL_OPEN on a port: its lines restart BLOCKED and deasserted. */
void ptt_port_opened(ptt_t *p, uint8_t port, uint64_t now_ms);
/* A port closed: deasserts its lines and releases its sources. */
void ptt_port_closed(ptt_t *p, uint8_t port, uint64_t now_ms);
/* USB reset, configure, suspend or unplug in wired mode: native lines drop. */
void ptt_native_reset(ptt_t *p, uint64_t now_ms);

/* Tone sequence keys or releases PTT (source bit 2). Returns true if keyed. */
bool ptt_tone(ptt_t *p, bool key, uint64_t now_ms);
/* Tone sequence finished and released PTT (reason SEQUENCE_DONE). */
void ptt_tone_done(ptt_t *p, uint64_t now_ms);

/* Everything off, every line BLOCKED, lockout cleared. For session end
 * (PTT_R_LINK_LOST), host-mode change (PTT_R_MODE_CHANGE) and internal faults
 * (PTT_R_FAULT). */
void ptt_all_off(ptt_t *p, uint8_t reason, uint64_t now_ms);

/* New configuration. A change to the targets or a line map releases PTT
 * first; no configuration change ever asserts PTT. */
void ptt_configure(ptt_t *p, const ptt_config_t *cfg, uint64_t now_ms);

/* Runs the keepalive and max-TX timers. Call at least every 50 ms. The
 * keepalive always runs; the max-TX timer runs unless MAX_TX_S is 0. */
void ptt_tick(ptt_t *p, uint64_t now_ms);

/* Current status (reason = the last reason reported). */
void ptt_status(const ptt_t *p, uint64_t now_ms, ptt_status_t *st);

/* Line arming state of a port's line (PTT_LINE_DTR or PTT_LINE_RTS). */
uint8_t ptt_arm_state(const ptt_t *p, uint8_t port, uint8_t line);

#ifdef __cplusplus
}
#endif

#endif /* PTT_H */

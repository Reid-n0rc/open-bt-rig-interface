/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * The device core: sessions (SPEC §4), message dispatch and the modules
 * behind it. The platform feeds it transport bytes and events from one task
 * and calls app_tick() at least every 50 ms; the app answers through hal.h.
 */
#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "audio.h"
#include "board.h"
#include "cat.h"
#include "cfg.h"
#include "clock_sync.h"
#include "hal.h"
#include "pairing.h"
#include "proto.h"
#include "ptt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const board_t *board;
    cfg_t cfg;
    ptt_t ptt;
    pairing_t pairing;
    clock_sync_t clock;
    cat_t cat;
    audio_t audio;

    proto_rx_t rx[HAL_TRANSPORT_MAX + 1]; /* one receiver per transport */
    uint8_t rx_transport;                /* transport being fed */
    uint8_t host_link;                   /* HAL_LINK_* */
    uint8_t active;                      /* transport of the session, 0 = none */
    bool authorized;                     /* bonded (BLE) or approved (wired, #64) */
    bool version_ok;
    uint16_t peer_max_payload;

    uint16_t frame_errors;
    bool frame_error_reported;
    uint64_t frame_error_report_ms;
    bool persisted;
    uint64_t persist_ms;

    bool status_pending; /* a PTT_STATUS to report once a session starts */
    ptt_status_t pending;

    uint64_t now_us; /* time of the event being handled */
} app_t;

/* Why the device last reset, as the platform reports it (for example from
 * esp_reset_reason() on ESP-IDF). */
typedef enum {
    APP_RESET_POWER_ON = 0, /* power-on, reset pin, software restart */
    APP_RESET_WATCHDOG,     /* task, interrupt or RTC watchdog */
    APP_RESET_BROWNOUT,
    APP_RESET_PANIC, /* exception, CPU lock-up */
    APP_RESET_OTHER,
} app_reset_t;

/* The first PTT_STATUS reason after a reset (SPEC §8.6): WATCHDOG after a
 * watchdog reset, FAULT after a brownout or panic, BOOT otherwise. */
uint8_t app_boot_reason(app_reset_t reset);

/* Power-on. `boot_reason` comes from app_boot_reason(); it is reported to
 * the first session (SPEC §8.5). */
void app_init(app_t *app, const board_t *board, uint8_t host_link, uint8_t boot_reason,
              uint64_t now_us);

/* Stream bytes from a transport. */
void app_rx(app_t *app, uint8_t transport, const uint8_t *data, size_t len, uint64_t now_us);
/* A transport closed (link lost, unsubscribed, TCP closed, ...). */
void app_transport_closed(app_t *app, uint8_t transport, uint64_t now_us);
/* Timers: PTT keepalive and max TX, the pairing window. */
void app_tick(app_t *app, uint64_t now_us);

/* Wired mode: native CDC-ACM line state on a device serial port (port 0 =
 * SERIAL-jack bridge, 0x0F = control port). */
void app_native_lines(app_t *app, uint8_t port, uint8_t lines, uint64_t now_us);
/* Wired mode: USB reset, configure, suspend or unplug. */
void app_usb_reset(app_t *app, uint64_t now_us);
/* Host-mode switch (Bluetooth <-> wired). */
void app_host_link_changed(app_t *app, uint8_t host_link, uint64_t now_us);
/* An internal fault (for example a failed output). */
void app_fault(app_t *app, uint64_t now_us);

/* The pairing button (a local action). */
void app_pairing_button(app_t *app, uint64_t now_us);
/* A new BLE bond was stored. */
void app_bond_added(app_t *app);
bool app_pairing_open(const app_t *app);

/* Bytes from the radio on a serial port (sent as CAT_DATA when open). */
void app_cat_from_radio(app_t *app, uint8_t port, const uint8_t *data, size_t len);

/* The GATT Info characteristic value (SPEC §13.2). */
void app_info(const app_t *app, uint8_t out[PROTO_INFO_LEN]);

/* Status LED pattern: on/off at `now_us`. Pairing window: 4 Hz; session:
 * steady on; otherwise 1 Hz. */
bool app_led(const app_t *app, uint64_t now_us);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */

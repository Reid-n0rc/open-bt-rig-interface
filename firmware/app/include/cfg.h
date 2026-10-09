/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Device configuration: every key of SPEC §6.1, with its default and range.
 *
 * Every write goes through cfg_set(), including values loaded from storage,
 * so a corrupt or out-of-range stored value falls back to its default and
 * can never raise the BLE TX power above the cap.
 */
#ifndef CFG_H
#define CFG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "proto.h"
#include "ptt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_SERIAL_PORTS 5u /* SERIAL_DEFAULT selector 0-4 */

/* SPEC §6.1 defaults and ranges. */
#define CFG_PAIRING_WINDOW_MIN_S 30u
#define CFG_PAIRING_WINDOW_MAX_S 600u
#define CFG_PAIRING_WINDOW_DEFAULT_S 120u
#define CFG_SERIAL_DEFAULT_BAUD 9600u
#define CFG_POWER_DOWN_MIN_S 5u
#define CFG_POWER_DOWN_MAX_S 3600u
#define CFG_POWER_DOWN_DEFAULT_S 30u /* 0 = never */
#define CFG_WIRED_PORT_LOCK_MAX 3u
/* TX_LEVEL and RX_GAIN defaults are set by the audio issue (#16); 0 until then. */
#define CFG_TX_LEVEL_DEFAULT 0
#define CFG_RX_GAIN_DEFAULT 0

typedef struct {
    uint32_t baud;
    uint8_t data_bits, parity, stop_bits;
} cfg_serial_t;

typedef struct {
    uint8_t serial_jack_mode;
    uint8_t ptt_targets, ptt_usb_port;
    uint8_t rts_action[PTT_NUM_PORTS], dtr_action[PTT_NUM_PORTS];
    uint16_t ptt_keepalive_ms;
    uint32_t max_tx_s; /* 0 = max-TX timer off */
    uint8_t audio_path;
    int16_t tx_level;
    uint8_t rx_attenuator;
    int16_t rx_gain;
    uint8_t host_mode;
    int8_t ble_tx_power;
    uint8_t wired_profile;
    cfg_serial_t serial_default[CFG_SERIAL_PORTS];
    uint8_t usb_net_subnet[4];
    uint16_t pairing_window_s;
    uint16_t power_down_delay_s; /* used by power management (#10, #11) */
    uint8_t wired_port_lock;     /* 0-3 (SPEC §15.2); USB side is #44 */
} cfg_t;

void cfg_defaults(cfg_t *cfg, const board_t *board);

/* CONFIG_GET: fills `out` for a key and selector. Returns PROTO_OK or
 * PROTO_ERR_BAD_VALUE (unknown key or selector). */
int cfg_get(const cfg_t *cfg, uint8_t key, uint8_t selector, proto_config_t *out);

/* CONFIG_SET: checks and applies one value. Returns PROTO_OK or the
 * RESULT code (BAD_VALUE, OUT_OF_RANGE, UNSUPPORTED). Nothing changes on error. */
int cfg_set(cfg_t *cfg, const board_t *board, const proto_config_t *in);

/* The PTT controller's view of the configuration. */
void cfg_to_ptt(const cfg_t *cfg, const board_t *board, ptt_config_t *out);

/* Storage format: a small header, one record per key/selector (the CONFIG
 * encoding), and a CRC-16. Returns the length, or 0 if `cap` is too small. */
#define CFG_BLOB_MAX 192u
size_t cfg_serialize(const cfg_t *cfg, uint8_t *out, size_t cap);
/* Loads a stored blob on top of the defaults. Records that fail cfg_set()
 * keep their default. Returns the number of records rejected, or -1 if the
 * blob itself is invalid (then `cfg` holds the defaults). */
int cfg_deserialize(cfg_t *cfg, const board_t *board, const uint8_t *in, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CFG_H */

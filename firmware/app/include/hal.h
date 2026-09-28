/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Hardware-abstraction interface: the only way firmware/app reaches the
 * hardware. firmware/platform/<sdk>/ implements it for a target, and
 * firmware/test/fake_hal.c implements it for the host tests.
 *
 * The app calls these from one task (the app task); implementations don't
 * need to be re-entrant with respect to each other.
 */
#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Time --- */

/* Device time: microseconds since boot, monotonic (SPEC §2). */
uint64_t hal_time_us(void);

/* --- PTT outputs (the hardware keeps them off unless driven, REQ-PTT-008) --- */

/* AUDIO-jack PTT closure. */
void hal_ptt_closure_set(bool on);
/* RTS/DTR toward a radio USB-serial port 1-4 (bit 0 DTR, bit 1 RTS). */
void hal_radio_lines_set(uint8_t port, uint8_t lines);

/* --- Status LED --- */
void hal_status_led_set(bool on);

/* --- Serial ports --- */

typedef struct {
    uint32_t baud;
    uint8_t data_bits; /* 7 or 8 */
    uint8_t parity;    /* 0 none, 1 odd, 2 even, 3 mark, 4 space */
    uint8_t stop_bits; /* 0 one, 1 one and a half, 2 two */
} hal_uart_cfg_t;

/* Opens or reconfigures port 0 (SERIAL jack) or a radio port 1-4. Returns
 * 0, or a PROTO_ERR_* code (for example UNSUPPORTED for a line format the
 * hardware can't do). */
int hal_serial_open(uint8_t port, const hal_uart_cfg_t *cfg);
void hal_serial_close(uint8_t port);
/* Queues bytes toward the radio. Returns how many were accepted. */
size_t hal_serial_write(uint8_t port, const uint8_t *data, size_t len);
/* SERIAL-jack mode switches (SPEC §6.2). Must never assert PTT. */
void hal_serial_jack_mode(uint8_t mode);

/* --- Audio (I2S codec), hooks for #16 --- */
int hal_audio_open(uint32_t sample_rate_hz);
void hal_audio_close(void);

/* --- Bluetooth LE --- */

/* Applies a BLE TX power already limited by ble_power_step_dbm(). */
void hal_ble_tx_power_set(int8_t dbm);

/* --- Host link --- */

/* Transports (STATUS.transport, SPEC §16.1). */
#define HAL_TRANSPORT_GATT 1u
#define HAL_TRANSPORT_L2CAP 2u
#define HAL_TRANSPORT_CONTROL 3u /* wired CDC-ACM control port */
#define HAL_TRANSPORT_TCP 4u     /* wired USB network */
#define HAL_TRANSPORT_MAX 4u

/* STATUS.host_link */
#define HAL_LINK_BLUETOOTH 1u
#define HAL_LINK_WIRED 2u

/* Sends stream bytes (already framed) on a transport. */
void hal_transport_send(uint8_t transport, const uint8_t *data, size_t len);

typedef struct {
    uint8_t host_link;
    uint8_t transport;
    uint8_t phy; /* 0 n/a, 1 1M, 2 2M, 3 Coded */
    uint32_t conn_interval_us;
    uint16_t att_mtu, coc_mtu;
    uint16_t flags; /* STATUS.flags bits 0, 1, 2 and 4 (hardware state) */
    uint8_t audio_path;
    uint16_t supply_mv;
} hal_link_status_t;

void hal_link_status(hal_link_status_t *out);

/* --- Configuration storage --- */

/* Loads the stored configuration blob. Returns the length, or 0 if none. */
size_t hal_config_load(uint8_t *buf, size_t cap);
/* Stores the blob. Returns 0 on success. */
int hal_config_save(const uint8_t *buf, size_t len);

/* --- Secure storage (hooks for #64; encrypted NVS with flash encryption) --- */

/* Reads/writes a secret (bond keys, host approvals). Return 0 on success,
 * nonzero if absent or unsupported. Not implemented before #64. */
int hal_secure_get(const char *key, uint8_t *buf, size_t cap, size_t *len);
int hal_secure_set(const char *key, const uint8_t *buf, size_t len);
/* Erases every secret and bond (factory reset). */
int hal_secure_erase_all(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */

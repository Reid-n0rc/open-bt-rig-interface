/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * BLE TX power cap (SPEC §6.3, docs/compliance/fcc.md §1.3, and the EU
 * analysis of #59 / PR #62, docs/compliance/eu.md §3.1, ADR-0009, REQ-REG-008).
 *
 * One cap for all markets: the lower of
 *   - FCC: the ESP32-S3-MINI-1 grant (FCC ID 2AC7Z-ESPS3MINI1) covers BLE at
 *     10.3 dBm conducted at most;
 *   - EU: Espressif's EU-type examination certificate 0370-RED-4972 lists BLE
 *     at 9.96 dBm e.i.r.p.; staying at or below it keeps the device inside the
 *     module's EU test and under EN 300 328's 10 dBm e.i.r.p. threshold.
 *     Conducted limit = 9.96 dBm - antenna gain.
 * The PCB antenna's gain isn't published. The only gain Espressif states is
 * 2.33 dBi, the antenna used to certify the -1U variant (module datasheet
 * v1.7 §10); it is used here as the working figure (verify, #59, against
 * Espressif's EN 300 328 test report).
 *
 * The ESP32-S3 controller sets TX power in 3 dB steps from -24 dBm (ESP-IDF
 * v6.0.3 esp_bt.h, esp_power_level_t; the applied power may be 0-2 dB below
 * the request). The cap is the highest step at or below the limit: +6 dBm
 * (ESP_PWR_LVL_P6). The protocol can only lower the power; nothing can raise
 * it above the cap.
 */
#ifndef BLE_POWER_H
#define BLE_POWER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Limits in hundredths of a dB. */
#define BLE_TX_POWER_FCC_MAX_CDBM 1030     /* 10.3 dBm conducted (FCC grant) */
#define BLE_TX_POWER_EU_MAX_EIRP_CDBM 996  /* 9.96 dBm e.i.r.p. (EU certificate) */
#define BLE_ANTENNA_GAIN_CDBI 233          /* 2.33 dBi, working figure (verify) */
#define BLE_TX_POWER_EU_MAX_CDBM (BLE_TX_POWER_EU_MAX_EIRP_CDBM - BLE_ANTENNA_GAIN_CDBI)
#define BLE_TX_POWER_LIMIT_CDBM                                                                    \
    (BLE_TX_POWER_FCC_MAX_CDBM < BLE_TX_POWER_EU_MAX_CDBM ? BLE_TX_POWER_FCC_MAX_CDBM              \
                                                          : BLE_TX_POWER_EU_MAX_CDBM)

/* Controller steps: -24 dBm to +20 dBm in 3 dB steps. */
#define BLE_TX_POWER_STEP_DB 3
#define BLE_TX_POWER_MIN_DBM (-24)

/* The firmware cap, for all markets. */
#define BLE_TX_POWER_CAP_DBM 6

/* ESP-IDF esp_power_level_t index for a supported step (N24 = 0 ... P6 = 10). */
#define BLE_DBM_TO_LEVEL(dbm) (((dbm) - BLE_TX_POWER_MIN_DBM) / BLE_TX_POWER_STEP_DB)

_Static_assert(BLE_TX_POWER_CAP_DBM * 100 <= BLE_TX_POWER_FCC_MAX_CDBM,
               "BLE TX power cap exceeds the FCC grant (10.3 dBm conducted)");
_Static_assert(BLE_TX_POWER_CAP_DBM * 100 + BLE_ANTENNA_GAIN_CDBI <= BLE_TX_POWER_EU_MAX_EIRP_CDBM,
               "BLE TX power cap exceeds the module's EU e.i.r.p. (9.96 dBm)");
_Static_assert((BLE_TX_POWER_CAP_DBM + BLE_TX_POWER_STEP_DB) * 100 > BLE_TX_POWER_LIMIT_CDBM,
               "BLE TX power cap is not the highest step at or below the limit");
_Static_assert((BLE_TX_POWER_CAP_DBM - BLE_TX_POWER_MIN_DBM) % BLE_TX_POWER_STEP_DB == 0,
               "BLE TX power cap is not a controller step");

/* Rounds a requested power down to a controller step and clamps it to
 * [BLE_TX_POWER_MIN_DBM, BLE_TX_POWER_CAP_DBM]. The result never exceeds the cap. */
int8_t ble_power_step_dbm(int dbm);

#ifdef __cplusplus
}
#endif

#endif /* BLE_POWER_H */

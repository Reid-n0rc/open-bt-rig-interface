/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * BLE TX power cap (SPEC §6.3). See ble_power.h.
 */
#include "ble_power.h"

int8_t ble_power_step_dbm(int dbm)
{
    if (dbm > BLE_TX_POWER_CAP_DBM) {
        dbm = BLE_TX_POWER_CAP_DBM;
    }
    if (dbm < BLE_TX_POWER_MIN_DBM) {
        dbm = BLE_TX_POWER_MIN_DBM;
    }
    /* Round down to a controller step, so the result never exceeds the request. */
    int steps = (dbm - BLE_TX_POWER_MIN_DBM) / BLE_TX_POWER_STEP_DB;
    return (int8_t)(BLE_TX_POWER_MIN_DBM + steps * BLE_TX_POWER_STEP_DB);
}

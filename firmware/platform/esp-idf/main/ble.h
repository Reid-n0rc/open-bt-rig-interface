/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Bluetooth LE: NimBLE host, the Rig Interface GATT service (SPEC §13) and
 * the TX power cap (SPEC §6.3).
 */
#ifndef BLE_H
#define BLE_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "proto.h"

/* Starts NimBLE, registers the service and starts advertising. */
esp_err_t ble_start(const char *device_name);

/* Values the app task publishes for the Bluetooth host task. */
void ble_publish(const uint8_t info[PROTO_INFO_LEN], bool pairing_open);

#endif /* BLE_H */

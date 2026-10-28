/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Radio USB port in Bluetooth mode (#43): the ESP32-S3 is USB host to the
 * radio (hubs included) through esp-usb, and protocol ports 1-4 reach the
 * radio's USB-serial chips through its CP210x, FTDI, CH34x and CDC-ACM
 * drivers.
 *
 * Devices that enumerate or go away are posted to the app task
 * (EVT_RADIO_USB_ATTACH / EVT_RADIO_USB_DETACH); the app decides which
 * interfaces are ports (usb_radio.h). The radio_usb_serial_* and
 * radio_usb_lines_set calls come from the app task through hal.h; a worker
 * task does the USB transfers, whose timeouts are longer than the task
 * watchdog's. Only an open waits for it, and for at most 300 ms.
 */
#ifndef RADIO_USB_H
#define RADIO_USB_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "hal.h"
#include "usb_radio.h"

/* Starts the USB host. `map` is the app's port table; it is read only from
 * the app task, inside the calls below. */
esp_err_t radio_usb_start(const usb_radio_t *map);

/* Ports 1-4, as hal_serial_open / _close / _write and hal_radio_lines_set. */
int radio_usb_serial_open(uint8_t port, const hal_uart_cfg_t *cfg);
void radio_usb_serial_close(uint8_t port);
size_t radio_usb_serial_write(uint8_t port, const uint8_t *data, size_t len);
void radio_usb_lines_set(uint8_t port, uint8_t lines);

#endif /* RADIO_USB_H */

/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Pin map for the Espressif ESP32-S3-DevKitM-1 (ESP32-S3-MINI-1). Shared by
 * the app and the bootloader hook, so the PTT pin is defined once.
 * See ../BOARD.md for the wiring, the reasons for each choice and the
 * sources. The custom board (#9, #21) gets its own header.
 */
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

/* PTT output, active high. GPIO4: no pull-up at reset (ESP32-S3 datasheet
 * Table 2-1), not a strapping pin, and its only power-up glitch is low-level
 * (Table 2-2), so an external pull-down keeps it off from power-on. */
#define BOARD_PTT_GPIO 4

/* Status LED: the DevKitM-1's addressable RGB LED (WS2812-type) on GPIO48. */
#define BOARD_LED_GPIO 48
#define BOARD_LED_WS2812 1

/* Pairing button: the Boot button (GPIO0, low when pressed). Read only after
 * boot; holding it at reset selects the ROM download mode. */
#define BOARD_PAIR_BUTTON_GPIO 0

/* SERIAL-jack UART (3.3 V logic on the dev kit; no level shifting). */
#define BOARD_SERIAL_UART 1
#define BOARD_SERIAL_TX_GPIO 17
#define BOARD_SERIAL_RX_GPIO 18

/* I2S to an audio codec (#16; nothing is fitted on the dev kit). */
#define BOARD_I2S_BCLK_GPIO 5
#define BOARD_I2S_WS_GPIO 6
#define BOARD_I2S_DOUT_GPIO 7
#define BOARD_I2S_DIN_GPIO 8

/* DEVICE_INFO: 'D' marks a development kit, not a product variant. */
#define BOARD_VARIANT 'D'
#define BOARD_HW_REVISION 'A'

#endif /* BOARD_PINS_H */

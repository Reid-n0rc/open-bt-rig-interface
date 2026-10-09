/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Platform setup that isn't part of hal.h.
 */
#ifndef HAL_ESP_H
#define HAL_ESP_H

/* Drives the PTT output low and keeps it an output. Call first in app_main. */
void hal_esp_ptt_init(void);
void hal_esp_button_init(void);
void hal_esp_serial_init(void);

#endif /* HAL_ESP_H */

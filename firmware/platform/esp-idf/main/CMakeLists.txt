# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

idf_component_register(
    SRCS "main.c" "hal_esp.c" "ble.c" "status_led.c"
    INCLUDE_DIRS "." "../board"
    REQUIRES rig_app bt nvs_flash esp_timer esp_driver_gpio esp_driver_uart esp_driver_rmt
             esp_driver_i2s esp_app_format)

/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Second-stage bootloader hook: drive the PTT output low as early as
 * software can, before the bootloader initializes anything else (ESP-IDF
 * v6.0.3, API guide "Bootloader", section "Custom Bootloader", and the
 * example custom_bootloader/bootloader_hooks).
 *
 * This is the third layer. Before it runs, the pin is at its reset state
 * (input disabled, no pull, ESP32-S3 datasheet Table 2-1) and the board's
 * external pull-down holds PTT off (REQ-PTT-008); that covers the ROM boot
 * and any time the firmware isn't running. The app drives it low again first
 * thing in app_main().
 */
#include <stdint.h>

#include "board_pins.h"
#include "esp_rom_gpio.h"
#include "hal/gpio_ll.h"
#include "soc/gpio_sig_map.h"

/* Referenced so the linker keeps this file (see the ESP-IDF example). */
void bootloader_hooks_include(void)
{
}

void bootloader_before_init(void)
{
    const uint32_t pin = BOARD_PTT_GPIO;
    gpio_ll_set_level(&GPIO, pin, 0); /* output latch low before enabling */
    esp_rom_gpio_pad_select_gpio(pin);
    esp_rom_gpio_connect_out_signal(pin, SIG_GPIO_OUT_IDX, false, false);
    gpio_ll_pulldown_en(&GPIO, pin);
    gpio_ll_output_enable(&GPIO, pin);
}

void bootloader_after_init(void)
{
}

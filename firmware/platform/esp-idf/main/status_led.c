/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Status LED. On the DevKitM-1 it is an addressable RGB LED (WS2812-type)
 * on GPIO48, driven with the RMT peripheral's bytes encoder (ESP-IDF v6.0.3
 * RMT driver). A plain LED on a custom board is a GPIO.
 *
 * WS2812 timing at 10 MHz (0.1 us per tick): a 0 bit is 0.3 us high and
 * 0.9 us low, a 1 bit 0.9 us high and 0.3 us low; bytes are sent G, R, B,
 * most significant bit first. The line idles low between updates, which is
 * the reset code (at least 50 us).
 */
#include "status_led.h"

#include <stdbool.h>

#include "board_pins.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal.h"

#if BOARD_LED_WS2812
#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"

#define RMT_RESOLUTION_HZ 10000000u

static rmt_channel_handle_t s_chan;
static rmt_encoder_handle_t s_enc;
#endif

static const char *TAG = "led";
static bool s_ready;
static bool s_on;

esp_err_t status_led_init(void)
{
#if BOARD_LED_WS2812
    rmt_tx_channel_config_t cfg = {
        .gpio_num = BOARD_LED_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION_HZ,
        .mem_block_symbols = 48,
        .trans_queue_depth = 2,
    };
    esp_err_t err = rmt_new_tx_channel(&cfg, &s_chan);
    if (err != ESP_OK) {
        return err;
    }
    rmt_bytes_encoder_config_t enc = {
        .bit0 = {.level0 = 1, .duration0 = 3, .level1 = 0, .duration1 = 9},
        .bit1 = {.level0 = 1, .duration0 = 9, .level1 = 0, .duration1 = 3},
        .flags.msb_first = 1,
    };
    err = rmt_new_bytes_encoder(&enc, &s_enc);
    if (err == ESP_OK) {
        err = rmt_enable(s_chan);
    }
    if (err != ESP_OK) {
        return err;
    }
#else
    gpio_config_t io = {
        .pin_bit_mask = 1ull << BOARD_LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t err = gpio_config(&io);
    if (err != ESP_OK) {
        return err;
    }
#endif
    s_ready = true;
    ESP_LOGI(TAG, "status LED on GPIO%d", BOARD_LED_GPIO);
    return ESP_OK;
}

void hal_status_led_set(bool on)
{
    if (!s_ready || on == s_on) {
        return;
    }
    s_on = on;
#if BOARD_LED_WS2812
    /* Dim blue when on: G, R, B. */
    static const uint8_t on_grb[3] = {0x00, 0x00, 0x10};
    static const uint8_t off_grb[3] = {0x00, 0x00, 0x00};
    rmt_transmit_config_t tx = {.loop_count = 0};
    (void)rmt_transmit(s_chan, s_enc, on ? on_grb : off_grb, 3, &tx);
#else
    gpio_set_level(BOARD_LED_GPIO, on ? 1 : 0);
#endif
}

/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * hal.h on ESP-IDF v6.0.3 (ESP32-S3): GPIO, UART, I2S, esp_timer and NVS.
 * The Bluetooth parts (transport, link status, TX power) are in ble.c and
 * the status LED in status_led.c.
 */
#include <string.h>

#include "board_pins.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal.h"
#include "hal_esp.h"
#include "nvs.h"
#include "proto.h"

static const char *TAG = "hal";

/* --- Time --- */

uint64_t hal_time_us(void)
{
    return (uint64_t)esp_timer_get_time();
}

/* --- PTT --- */

void hal_esp_ptt_init(void)
{
    /* Low first, then output: the pin never goes high on the way. */
    gpio_set_level(BOARD_PTT_GPIO, 0);
    gpio_config_t io = {
        .pin_bit_mask = 1ull << BOARD_PTT_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    gpio_set_level(BOARD_PTT_GPIO, 0);
}

void hal_ptt_closure_set(bool on)
{
    gpio_set_level(BOARD_PTT_GPIO, on ? 1 : 0);
}

void hal_radio_lines_set(uint8_t port, uint8_t lines)
{
    /* Radio USB-serial ports arrive with the USB host issue (#43). None
     * exists before then, so nothing can be asserted here. */
    (void)port;
    (void)lines;
}

/* --- Pairing button --- */

static void button_task(void *arg)
{
    (void)arg;
    int last = 1;
    for (;;) {
        int level = gpio_get_level(BOARD_PAIR_BUTTON_GPIO);
        if (level == 0 && last == 1) {
            (void)evt_post(EVT_PAIR_BUTTON, 0, NULL, 0);
        }
        last = level;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void hal_esp_button_init(void)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ull << BOARD_PAIR_BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));
    xTaskCreate(button_task, "button", 2048, NULL, 2, NULL);
}

/* --- SERIAL-jack UART (port 0) --- */

/* The driver is installed while port 0 is open and deleted when it closes,
 * which discards bytes still in its TX ring buffer (SPEC §7.2): a command
 * the host queued must not reach the radio after the session ended. The
 * lock keeps the RX task out of the driver while it is installed or deleted;
 * reads, writes, open and close otherwise run in their own tasks as before
 * (writes, open and close all in the app task). */
static bool s_uart_installed;
static SemaphoreHandle_t s_uart_lock;

static void uart_rx_task(void *arg)
{
    (void)arg;
    static uint8_t buf[EVT_DATA_MAX];
    for (;;) {
        xSemaphoreTake(s_uart_lock, portMAX_DELAY);
        int n = 0;
        bool installed = s_uart_installed;
        if (installed) {
            /* Reads return after a 1 ms gap; the app batches into frames (SPEC §7.3). */
            n = uart_read_bytes(BOARD_SERIAL_UART, buf, sizeof(buf), pdMS_TO_TICKS(1));
        }
        xSemaphoreGive(s_uart_lock);
        if (n > 0) {
            (void)evt_post(EVT_SERIAL_RX, 0, buf, (size_t)n);
        } else if (!installed) {
            vTaskDelay(pdMS_TO_TICKS(20));
        } else if (n < 0) {
            vTaskDelay(pdMS_TO_TICKS(1)); /* a driver error: don't spin */
        }
    }
}

void hal_esp_serial_init(void)
{
    s_uart_lock = xSemaphoreCreateMutex();
    configASSERT(s_uart_lock);
    xTaskCreate(uart_rx_task, "uart_rx", 3072, NULL, 5, NULL);
}

int hal_serial_open(uint8_t port, const hal_uart_cfg_t *cfg)
{
    if (port != 0) {
        return PROTO_ERR_UNSUPPORTED; /* radio USB-serial: #43 */
    }
    uart_config_t u = {
        .baud_rate = (int)cfg->baud,
        .data_bits = cfg->data_bits == 7 ? UART_DATA_7_BITS : UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    switch (cfg->parity) {
    case 0: u.parity = UART_PARITY_DISABLE; break;
    case 1: u.parity = UART_PARITY_ODD; break;
    case 2: u.parity = UART_PARITY_EVEN; break;
    default: return PROTO_ERR_UNSUPPORTED; /* mark/space: not in the ESP32-S3 UART */
    }
    switch (cfg->stop_bits) {
    case 0: u.stop_bits = UART_STOP_BITS_1; break;
    case 1: u.stop_bits = UART_STOP_BITS_1_5; break;
    default: u.stop_bits = UART_STOP_BITS_2; break;
    }
    int rc = PROTO_OK;
    xSemaphoreTake(s_uart_lock, portMAX_DELAY);
    if (!s_uart_installed) {
        if (uart_driver_install(BOARD_SERIAL_UART, 1024, 1024, 0, NULL, 0) == ESP_OK) {
            s_uart_installed = true;
        } else {
            rc = PROTO_ERR_INTERNAL;
        }
    }
    if (rc == PROTO_OK &&
        (uart_param_config(BOARD_SERIAL_UART, &u) != ESP_OK ||
         uart_set_pin(BOARD_SERIAL_UART, BOARD_SERIAL_TX_GPIO, BOARD_SERIAL_RX_GPIO,
                      UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)) {
        rc = PROTO_ERR_INTERNAL;
    }
    xSemaphoreGive(s_uart_lock);
    return rc;
}

void hal_serial_close(uint8_t port)
{
    if (port != 0) {
        return;
    }
    xSemaphoreTake(s_uart_lock, portMAX_DELAY);
    if (s_uart_installed) {
        /* Deleting the driver discards both ring buffers (SPEC §7.2). */
        (void)uart_driver_delete(BOARD_SERIAL_UART);
        s_uart_installed = false;
    }
    xSemaphoreGive(s_uart_lock);
}

size_t hal_serial_write(uint8_t port, const uint8_t *data, size_t len)
{
    if (port != 0 || !s_uart_installed) {
        return 0;
    }
    /* Only what fits in the driver's TX ring buffer, so this never blocks;
     * the rest stays queued in the app and is offered again (SPEC §7.4). */
    size_t room = 0;
    if (uart_get_tx_buffer_free_size(BOARD_SERIAL_UART, &room) != ESP_OK || room == 0) {
        return 0;
    }
    size_t n = len < room ? len : room;
    int w = uart_write_bytes(BOARD_SERIAL_UART, (const char *)data, n);
    return w > 0 ? (size_t)w : 0;
}

void hal_serial_jack_mode(uint8_t mode)
{
    /* The mode switches are on the custom board (#9, #15). Changing them must
     * never assert PTT; the dev kit has none. */
    ESP_LOGI(TAG, "SERIAL-jack mode %u (no switches on the dev kit)", mode);
}

/* --- Audio (I2S), hooks for #16 --- */

static i2s_chan_handle_t s_i2s_tx, s_i2s_rx;

int hal_audio_open(uint32_t sample_rate_hz)
{
    if (s_i2s_tx) {
        return PROTO_ERR_BUSY;
    }
    i2s_chan_config_t chan = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    if (i2s_new_channel(&chan, &s_i2s_tx, &s_i2s_rx) != ESP_OK) {
        return PROTO_ERR_INTERNAL;
    }
    i2s_std_config_t std = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate_hz),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = BOARD_I2S_BCLK_GPIO,
                .ws = BOARD_I2S_WS_GPIO,
                .dout = BOARD_I2S_DOUT_GPIO,
                .din = BOARD_I2S_DIN_GPIO,
            },
    };
    if (i2s_channel_init_std_mode(s_i2s_tx, &std) != ESP_OK ||
        i2s_channel_init_std_mode(s_i2s_rx, &std) != ESP_OK) {
        hal_audio_close();
        return PROTO_ERR_INTERNAL;
    }
    return PROTO_OK;
}

void hal_audio_close(void)
{
    if (s_i2s_tx) {
        (void)i2s_del_channel(s_i2s_tx);
        s_i2s_tx = NULL;
    }
    if (s_i2s_rx) {
        (void)i2s_del_channel(s_i2s_rx);
        s_i2s_rx = NULL;
    }
}

/* --- Configuration storage (NVS) --- */

#define NVS_NS "rig"
#define NVS_KEY_CFG "cfg"

size_t hal_config_load(uint8_t *buf, size_t cap)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) {
        return 0;
    }
    size_t len = cap;
    esp_err_t err = nvs_get_blob(h, NVS_KEY_CFG, buf, &len);
    nvs_close(h);
    return err == ESP_OK ? len : 0;
}

int hal_config_save(const uint8_t *buf, size_t len)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) {
        return -1;
    }
    esp_err_t err = nvs_set_blob(h, NVS_KEY_CFG, buf, len);
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    return err == ESP_OK ? 0 : -1;
}

/* --- Secure storage: hooks for #64 (encrypted NVS, flash encryption) --- */

int hal_secure_get(const char *key, uint8_t *buf, size_t cap, size_t *len)
{
    (void)key;
    (void)buf;
    (void)cap;
    *len = 0;
    return -1;
}

int hal_secure_set(const char *key, const uint8_t *buf, size_t len)
{
    (void)key;
    (void)buf;
    (void)len;
    return -1;
}

int hal_secure_erase_all(void)
{
    return -1;
}

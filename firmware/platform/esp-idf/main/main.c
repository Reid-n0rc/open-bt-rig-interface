/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * ESP32-S3 firmware entry point. Order matters:
 *   1. PTT output low (already low from the bootloader hook; the board's
 *      pull-down covered the ROM boot).
 *   2. Why did we reset? A watchdog reset is reported to the next session.
 *   3. The app core, then the drivers and Bluetooth.
 * The app task owns the app state and feeds the task watchdog; the
 * watchdogs themselves are set in sdkconfig.defaults and never disabled.
 */
#include <inttypes.h>
#include <string.h>

#include "app.h"
#include "ble.h"
#include "board_pins.h"
#include "esp_app_desc.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "fw_version.h"
#include "hal_esp.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "status_led.h"

/* The watchdogs must stay on and must reset the chip (maintainer decision,
 * 2026-09-25; SPEC §8.5). */
#if !CONFIG_ESP_TASK_WDT_EN || !CONFIG_ESP_TASK_WDT_INIT || !CONFIG_ESP_TASK_WDT_PANIC
#error "The task watchdog must be enabled at startup and must panic (reset) on timeout"
#endif
#if !CONFIG_ESP_INT_WDT
#error "The interrupt watchdog must be enabled"
#endif
#if !CONFIG_BOOTLOADER_WDT_ENABLE
#error "The RTC watchdog must cover the boot"
#endif
#if CONFIG_ESP_TASK_WDT_TIMEOUT_S > 5
#error "Task watchdog timeout must be a few seconds at most"
#endif

static const char *TAG = "main";

static const board_t s_board = {
    .variant = BOARD_VARIANT,
    .hw_revision = BOARD_HW_REVISION,
    .features = PROTO_F_CAT | PROTO_F_PTT_CLOSURE | PROTO_F_CLOCK_SYNC | PROTO_F_BLE_TX_POWER |
                PROTO_F_CONFIG_PERSIST | PROTO_F_PAIRING_WINDOW,
    .serial_modes = 0x01, /* 3.3 V logic only on the dev kit */
    .serial_min_baud = 4800,
    .serial_max_baud = 115200,
    .cat_tx_buffer = 256,
    .ptt_outputs = 0x01, /* AUDIO-jack closure (a GPIO on the dev kit) */
    .pairing_triggers = BOARD_PAIR_POWER_ON | BOARD_PAIR_BUTTON,
    .max_bonds = CONFIG_BT_NIMBLE_MAX_BONDS,
    .max_wired_hosts = 0,          /* the wired-host store is #64 */
    .radio_port_isolation = false, /* no radio USB port on the dev kit */
    .l2cap_psm = 0,
};

static app_t s_app;
static QueueHandle_t s_queue;

bool evt_post(evt_type_t type, uint8_t transport, const uint8_t *data, size_t len)
{
    bool ok = true;
    do {
        evt_t e;
        size_t n = len < EVT_DATA_MAX ? len : EVT_DATA_MAX;
        e.type = (uint8_t)type;
        e.transport = transport;
        e.len = (uint16_t)n;
        if (n) {
            memcpy(e.data, data, n);
        }
        ok = xQueueSend(s_queue, &e, pdMS_TO_TICKS(20)) == pdTRUE && ok;
        data += n;
        len -= n;
    } while (len);
    return ok;
}

static app_reset_t reset_cause(void)
{
    switch (esp_reset_reason()) {
    case ESP_RST_POWERON:
    case ESP_RST_EXT:
    case ESP_RST_SW:
    case ESP_RST_USB:
    case ESP_RST_JTAG:
        return APP_RESET_POWER_ON;
    case ESP_RST_TASK_WDT:
    case ESP_RST_INT_WDT:
    case ESP_RST_WDT:
        return APP_RESET_WATCHDOG;
    case ESP_RST_BROWNOUT:
    case ESP_RST_PWR_GLITCH:
        return APP_RESET_BROWNOUT;
    case ESP_RST_PANIC:
    case ESP_RST_CPU_LOCKUP:
        return APP_RESET_PANIC;
    default:
        return APP_RESET_OTHER;
    }
}

static void handle(const evt_t *e)
{
    uint64_t now = hal_time_us();
    switch (e->type) {
    case EVT_RX:
        app_rx(&s_app, e->transport, e->data, e->len, now);
        break;
    case EVT_TRANSPORT_CLOSED:
        app_transport_closed(&s_app, e->transport, now);
        break;
    case EVT_BOND_ADDED:
        app_bond_added(&s_app);
        break;
    case EVT_PAIR_BUTTON:
        app_pairing_button(&s_app, now);
        ESP_LOGI(TAG, "pairing window open");
        break;
    case EVT_SERIAL_RX:
        app_cat_from_radio(&s_app, e->transport, e->data, e->len);
        break;
    default:
        break;
    }
}

void app_main(void)
{
    hal_esp_ptt_init(); /* 1: PTT off before anything else */

    app_reset_t cause = reset_cause(); /* 2 */

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    s_queue = xQueueCreate(16, sizeof(evt_t));
    configASSERT(s_queue);

    /* 3: the core. It drives every PTT output off again and reports the
     * reset reason to the first session. */
    app_init(&s_app, &s_board, HAL_LINK_BLUETOOTH, app_boot_reason(cause), hal_time_us());

    const esp_app_desc_t *desc = esp_app_get_description();
    ESP_LOGI(TAG, "rig-interface firmware %s (fw %d.%d.%d, protocol %d.%d.%d, ESP-IDF %s)",
             desc->version, FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH,
             PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR, PROTO_VERSION_PATCH, esp_get_idf_version());
    ESP_LOGI(TAG, "reset cause %d, PTT on GPIO%d", (int)cause, BOARD_PTT_GPIO);

    if (status_led_init() != ESP_OK) {
        ESP_LOGW(TAG, "status LED unavailable");
    }
    hal_esp_button_init();
    hal_esp_serial_init();
    uint8_t info[PROTO_INFO_LEN];
    app_info(&s_app, info);
    ble_publish(info, app_pairing_open(&s_app));
    ESP_ERROR_CHECK(ble_start("Rig Interface"));

    /* The app task is this one: it must run at least every 50 ms for the PTT
     * timers, and it feeds the task watchdog. */
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    for (;;) {
        evt_t e;
        if (xQueueReceive(s_queue, &e, pdMS_TO_TICKS(10)) == pdTRUE) {
            handle(&e);
            while (xQueueReceive(s_queue, &e, 0) == pdTRUE) {
                handle(&e);
            }
        }
        uint64_t now = hal_time_us();
        app_tick(&s_app, now);
        hal_status_led_set(app_led(&s_app, now));
        app_info(&s_app, info);
        ble_publish(info, app_pairing_open(&s_app));
        esp_task_wdt_reset();
    }
}

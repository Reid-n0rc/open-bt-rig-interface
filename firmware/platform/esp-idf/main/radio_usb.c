/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Radio USB port in Bluetooth mode (#43). See radio_usb.h.
 *
 * Tasks:
 *   usb_lib    the USB Host Library's events (hubs included).
 *   usb_client a host client that opens every new device to read its
 *              descriptors and keeps it open until it goes, so it hears
 *              DEV_GONE; both are posted to the app task.
 *   usb_serial the worker: opens, closes and configures ports 1-4, sets
 *              their RTS/DTR, and drains their TX buffers.
 * The esp-usb CDC-ACM driver runs its own task and calls data_cb with the
 * radio's bytes.
 */
#include "radio_usb.h"

#include <inttypes.h>
#include <stdatomic.h>
#include <string.h>

#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"
#include "proto.h"
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"
#include "usb/vcp_ch34x.h"
#include "usb/vcp_cp210x.h"
#include "usb/vcp_ftdi.h"

static const char *TAG = "radio_usb";

_Static_assert(sizeof(usb_radio_desc_t) <= EVT_DATA_MAX, "a device summary fits one event");

/* SERIAL_OPEN / SERIAL_SET wait this long for the worker. A USB control
 * transfer can take up to 5 s to time out; the app task must not. */
#define OPEN_WAIT_MS 300u
#define TX_BUF 512u   /* per port, like the SERIAL jack's UART ring buffer */
#define TX_CHUNK 64u  /* one full-speed bulk packet */
#define TX_TIMEOUT_MS 50u
#define MAX_DEVICES USB_RADIO_MAX_DEVICES

/* FTDI SIO_SET_MODEM_CTRL: request 1, wValue = mask << 8 | state, bit 0 DTR,
 * bit 1 RTS (Linux ftdi_sio.h, facts only; THIRD_PARTY.md). esp-usb's FTDI
 * driver sends request 2 with other values, so the lines are set here. */
#define FTDI_REQ_MODEM_CTRL 0x01u
#define FTDI_MODEM_MASK_DTR_RTS 0x0300u

static const usb_radio_t *s_map;

/* --- Worker --- */

typedef enum { CMD_OPEN, CMD_CLOSE, CMD_LINES } cmd_type_t;

typedef struct {
    uint8_t type; /* cmd_type_t */
    uint8_t port;
    uint8_t lines;
    uint32_t seq;           /* CMD_OPEN: matches its answer */
    usb_radio_port_t where; /* CMD_OPEN */
    hal_uart_cfg_t cfg;     /* CMD_OPEN */
} cmd_t;

/* Worker-owned, except `open` (the app task's view: set on a successful
 * open, cleared on close) and `gone` (set by the CDC driver's task). */
typedef struct {
    cdc_acm_dev_hdl_t hdl;
    uint8_t chip, interface;
    StreamBufferHandle_t tx;
    atomic_bool open, gone;
} port_t;

static port_t s_port[USB_RADIO_PORTS + 1];
static QueueHandle_t s_cmds;
typedef struct {
    uint32_t seq;
    int rc;
} open_result_t;

static QueueHandle_t s_open_result; /* open_result_t, one per CMD_OPEN */
static uint32_t s_open_seq;         /* app task only */

static bool data_cb(const uint8_t *data, size_t len, void *arg)
{
    (void)evt_post(EVT_SERIAL_RX, (uint8_t)(uintptr_t)arg, data, len);
    return true;
}

static void event_cb(const cdc_acm_host_dev_event_data_t *event, void *arg)
{
    uint8_t port = (uint8_t)(uintptr_t)arg;
    if (event->type == CDC_ACM_HOST_DEVICE_DISCONNECTED) {
        /* The handle is closed by the worker when the app closes the port,
         * after usb_client posts the detach. */
        atomic_store(&s_port[port].gone, true);
    }
}

static esp_err_t lines_apply(const port_t *p, uint8_t lines)
{
    bool dtr = (lines & 0x01u) != 0, rts = (lines & 0x02u) != 0;
    if (p->chip == USB_CHIP_FTDI) {
        uint16_t v = (uint16_t)(FTDI_MODEM_MASK_DTR_RTS | (dtr ? 0x01u : 0u) | (rts ? 0x02u : 0u));
        return cdc_acm_host_send_custom_request(
            p->hdl, USB_BM_REQUEST_TYPE_TYPE_VENDOR | USB_BM_REQUEST_TYPE_DIR_OUT,
            FTDI_REQ_MODEM_CTRL, v, (uint16_t)(p->interface + 1u), 0, NULL);
    }
    /* CP210x SET_MHS, CH34x MODEM_OUT, CDC SET_CONTROL_LINE_STATE. */
    return cdc_acm_host_set_control_line_state(p->hdl, dtr, rts);
}

static esp_err_t line_coding_apply(const port_t *p, const hal_uart_cfg_t *cfg)
{
    cdc_acm_line_coding_t lc = {
        .dwDTERate = cfg->baud,
        .bCharFormat = cfg->stop_bits, /* 0 one, 1 one and a half, 2 two: as CDC */
        .bParityType = cfg->parity,    /* 0 none, 1 odd, 2 even, 3 mark, 4 space: as CDC */
        .bDataBits = cfg->data_bits,
    };
    return cdc_acm_host_line_coding_set(p->hdl, &lc);
}

static void port_close(uint8_t port)
{
    port_t *p = &s_port[port];
    if (p->hdl) {
        if (!atomic_load(&p->gone)) {
            (void)lines_apply(p, 0); /* closing deasserts RTS and DTR (SPEC §7.2) */
        }
        (void)cdc_acm_host_close(p->hdl);
        p->hdl = NULL;
    }
    xStreamBufferReset(p->tx); /* unsent bytes are discarded (§7.2) */
}

static int port_open(const cmd_t *c)
{
    port_t *p = &s_port[c->port];
    if (p->hdl && p->chip == c->where.chip && p->interface == c->where.interface &&
        !atomic_load(&p->gone)) {
        /* SERIAL_SET on an open port: new line settings (REQ-CAT-004). */
        return line_coding_apply(p, &c->cfg) == ESP_OK ? PROTO_OK : PROTO_ERR_UNSUPPORTED;
    }
    port_close(c->port);
    atomic_store(&p->gone, false);

    cdc_acm_host_device_config_t dc = {
        .connection_timeout_ms = 100, /* it has enumerated already */
        .out_buffer_size = TX_CHUNK,
        .in_buffer_size = 0, /* the IN endpoint's packet size */
        .event_cb = event_cb,
        .data_cb = data_cb,
        .user_arg = (void *)(uintptr_t)c->port,
    };
    const usb_radio_port_t *w = &c->where;
    esp_err_t err;
    switch (w->chip) {
    case USB_CHIP_CP210X:
        err = cp210x_vcp_open(w->pid, w->interface, &dc, &p->hdl);
        break;
    case USB_CHIP_FTDI:
        err = ftdi_vcp_open(w->pid, w->interface, &dc, &p->hdl);
        break;
    case USB_CHIP_CH34X:
        err = ch34x_vcp_open(w->pid, w->interface, &dc, &p->hdl);
        break;
    case USB_CHIP_CDC_ACM: {
        cdc_acm_host_open_config_t oc = {
            .vid = w->vid,
            .pid = w->pid,
            .interface_idx = w->interface,
            .dev_addr = w->dev, /* the device handle is its USB address */
            .connection_timeout_ms = dc.connection_timeout_ms,
            .out_buffer_size = dc.out_buffer_size,
            .in_buffer_size = dc.in_buffer_size,
            .event_cb = dc.event_cb,
            .data_cb = dc.data_cb,
            .user_arg = dc.user_arg,
        };
        err = cdc_acm_host_open_v2(&oc, &p->hdl);
        break;
    }
    default:
        err = ESP_ERR_NOT_SUPPORTED;
        break;
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "port %u: open failed: %s", c->port, esp_err_to_name(err));
        p->hdl = NULL;
        return PROTO_ERR_UNSUPPORTED;
    }
    p->chip = w->chip;
    p->interface = w->interface;
    /* RTS and DTR inactive on open, whatever the chip did (ADR-0003 §8):
     * the PTT controller drives them from here on. */
    if (lines_apply(p, 0) != ESP_OK || line_coding_apply(p, &c->cfg) != ESP_OK) {
        ESP_LOGW(TAG, "port %u: setup failed", c->port);
        port_close(c->port);
        return PROTO_ERR_UNSUPPORTED;
    }
    xStreamBufferReset(p->tx);
    ESP_LOGI(TAG, "port %u: chip %u %04x:%04x interface %u at %" PRIu32 " baud", c->port,
             w->chip, w->vid, w->pid, w->interface, c->cfg.baud);
    return PROTO_OK;
}

static void drain_tx(void)
{
    static uint8_t chunk[TX_CHUNK];
    for (uint8_t port = 1; port <= USB_RADIO_PORTS; port++) {
        port_t *p = &s_port[port];
        if (!p->hdl || atomic_load(&p->gone)) {
            continue;
        }
        size_t n = xStreamBufferReceive(p->tx, chunk, sizeof(chunk), 0);
        if (n && cdc_acm_host_data_tx_blocking(p->hdl, chunk, n, TX_TIMEOUT_MS) != ESP_OK) {
            ESP_LOGW(TAG, "port %u: %u bytes not sent", port, (unsigned)n);
        }
    }
}

static void serial_task(void *arg)
{
    (void)arg;
    for (;;) {
        cmd_t c;
        if (xQueueReceive(s_cmds, &c, pdMS_TO_TICKS(1)) == pdTRUE) {
            switch (c.type) {
            case CMD_OPEN: {
                open_result_t r = {c.seq, port_open(&c)};
                (void)xQueueOverwrite(s_open_result, &r);
                break;
            }
            case CMD_CLOSE:
                port_close(c.port);
                break;
            case CMD_LINES:
                if (s_port[c.port].hdl && !atomic_load(&s_port[c.port].gone) &&
                    lines_apply(&s_port[c.port], c.lines) != ESP_OK) {
                    ESP_LOGW(TAG, "port %u: RTS/DTR not set", c.port);
                }
                break;
            default:
                break;
            }
        }
        drain_tx();
    }
}

/* --- App-task side (hal.h) --- */

int radio_usb_serial_open(uint8_t port, const hal_uart_cfg_t *cfg)
{
    const usb_radio_port_t *w = s_map ? usb_radio_port(s_map, port) : NULL;
    if (!w || !s_cmds) {
        return PROTO_ERR_UNSUPPORTED;
    }
    cmd_t c = {.type = CMD_OPEN, .port = port, .seq = ++s_open_seq, .where = *w, .cfg = *cfg};
    TickType_t until = xTaskGetTickCount() + pdMS_TO_TICKS(OPEN_WAIT_MS);
    if (xQueueSend(s_cmds, &c, pdMS_TO_TICKS(OPEN_WAIT_MS)) == pdTRUE) {
        open_result_t r;
        TickType_t now;
        /* Skips a late answer to an earlier open that timed out. */
        while ((now = xTaskGetTickCount()) < until &&
               xQueueReceive(s_open_result, &r, until - now) == pdTRUE) {
            if (r.seq == c.seq) {
                atomic_store(&s_port[port].open, r.rc == PROTO_OK);
                return r.rc;
            }
        }
    }
    /* Gave up; if the open still succeeds, this close undoes it. */
    radio_usb_serial_close(port);
    return PROTO_ERR_BUSY;
}

void radio_usb_serial_close(uint8_t port)
{
    if (port < 1 || port > USB_RADIO_PORTS || !s_cmds) {
        return;
    }
    atomic_store(&s_port[port].open, false);
    cmd_t c = {.type = CMD_CLOSE, .port = port};
    (void)xQueueSend(s_cmds, &c, portMAX_DELAY); /* must not be lost */
}

size_t radio_usb_serial_write(uint8_t port, const uint8_t *data, size_t len)
{
    if (port < 1 || port > USB_RADIO_PORTS || !atomic_load(&s_port[port].open)) {
        return 0;
    }
    /* What fits now; the app keeps the rest and offers it again (§7.4). */
    return xStreamBufferSend(s_port[port].tx, data, len, 0);
}

void radio_usb_lines_set(uint8_t port, uint8_t lines)
{
    if (port < 1 || port > USB_RADIO_PORTS || !s_cmds) {
        return;
    }
    cmd_t c = {.type = CMD_LINES, .port = port, .lines = lines};
    /* Must not be lost: a PTT-off is one of these (SPEC §8.5). */
    (void)xQueueSend(s_cmds, &c, portMAX_DELAY);
}

/* --- USB host and devices --- */

static usb_host_client_handle_t s_client;
static struct {
    uint8_t addr;
    usb_device_handle_t hdl;
} s_dev[MAX_DEVICES];
static uint8_t s_new[MAX_DEVICES]; /* addresses waiting to be opened */
static size_t s_new_n;
static usb_device_handle_t s_gone[MAX_DEVICES];
static size_t s_gone_n;

static void post_until_queued(evt_type_t type, uint8_t dev, const void *data, size_t len)
{
    /* Attach and detach must reach the app, or a port outlives its chip. */
    while (!evt_post(type, dev, data, len)) {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static void client_cb(const usb_host_client_event_msg_t *msg, void *arg)
{
    (void)arg;
    if (msg->event == USB_HOST_CLIENT_EVENT_NEW_DEV && s_new_n < MAX_DEVICES) {
        s_new[s_new_n++] = msg->new_dev.address;
    } else if (msg->event == USB_HOST_CLIENT_EVENT_DEV_GONE && s_gone_n < MAX_DEVICES) {
        s_gone[s_gone_n++] = msg->dev_gone.dev_hdl;
    }
}

static void device_new(uint8_t addr)
{
    int slot = -1;
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!s_dev[i].hdl) {
            slot = i;
            break;
        }
    }
    usb_device_handle_t hdl;
    if (slot < 0 || usb_host_device_open(s_client, addr, &hdl) != ESP_OK) {
        ESP_LOGW(TAG, "device %u: not opened", addr);
        return;
    }
    const usb_device_desc_t *dd;
    const usb_config_desc_t *cd;
    if (usb_host_get_device_descriptor(hdl, &dd) != ESP_OK ||
        usb_host_get_active_config_descriptor(hdl, &cd) != ESP_OK) {
        (void)usb_host_device_close(s_client, hdl);
        return;
    }
    if (dd->bDeviceClass == USB_CLASS_HUB) {
        /* The radio's own hub: the hub driver handles it, its devices
         * enumerate on their own. */
        (void)usb_host_device_close(s_client, hdl);
        return;
    }
    usb_radio_desc_t d = {.vid = dd->idVendor, .pid = dd->idProduct};
    int offset = 0;
    const usb_standard_desc_t *cur = (const usb_standard_desc_t *)cd;
    while ((cur = usb_parse_next_descriptor_of_type(cur, cd->wTotalLength,
                                                    USB_B_DESCRIPTOR_TYPE_INTERFACE, &offset))) {
        const usb_intf_desc_t *i = (const usb_intf_desc_t *)cur;
        if (i->bAlternateSetting != 0 || d.n_ifaces >= USB_RADIO_MAX_IFACES) {
            continue;
        }
        d.iface[d.n_ifaces++] = (usb_radio_iface_t){i->bInterfaceNumber, i->bInterfaceClass,
                                                     i->bInterfaceSubClass,
                                                     i->bInterfaceProtocol};
    }
    s_dev[slot].addr = addr;
    s_dev[slot].hdl = hdl;
    ESP_LOGI(TAG, "device %u: %04x:%04x, %u interfaces", addr, d.vid, d.pid, d.n_ifaces);
    post_until_queued(EVT_RADIO_USB_ATTACH, addr, &d, sizeof(d));
}

static void device_gone(usb_device_handle_t hdl)
{
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (s_dev[i].hdl == hdl) {
            uint8_t addr = s_dev[i].addr;
            (void)usb_host_device_close(s_client, hdl);
            s_dev[i].hdl = NULL;
            s_dev[i].addr = 0;
            ESP_LOGI(TAG, "device %u: gone", addr);
            post_until_queued(EVT_RADIO_USB_DETACH, addr, NULL, 0);
            return;
        }
    }
}

static void client_task(void *arg)
{
    (void)arg;
    usb_host_client_config_t cfg = {
        .is_synchronous = false,
        .max_num_event_msg = 8,
        .async = {.client_event_callback = client_cb, .callback_arg = NULL},
    };
    ESP_ERROR_CHECK(usb_host_client_register(&cfg, &s_client));
    for (;;) {
        (void)usb_host_client_handle_events(s_client, portMAX_DELAY);
        for (size_t i = 0; i < s_gone_n; i++) {
            device_gone(s_gone[i]);
        }
        s_gone_n = 0;
        for (size_t i = 0; i < s_new_n; i++) {
            device_new(s_new[i]);
        }
        s_new_n = 0;
    }
}

static void usb_lib_task(void *arg)
{
    TaskHandle_t starter = (TaskHandle_t)arg;
    usb_host_config_t hc = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LOWMED,
    };
    ESP_ERROR_CHECK(usb_host_install(&hc));
    xTaskNotifyGive(starter);
    for (;;) {
        uint32_t flags = 0;
        (void)usb_host_lib_handle_events(portMAX_DELAY, &flags);
        if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            (void)usb_host_device_free_all();
        }
    }
}

esp_err_t radio_usb_start(const usb_radio_t *map)
{
    s_map = map;
    for (uint8_t port = 1; port <= USB_RADIO_PORTS; port++) {
        s_port[port].tx = xStreamBufferCreate(TX_BUF, 1);
        if (!s_port[port].tx) {
            return ESP_ERR_NO_MEM;
        }
    }
    s_open_result = xQueueCreate(1, sizeof(open_result_t));
    s_cmds = xQueueCreate(16, sizeof(cmd_t));
    if (!s_cmds || !s_open_result) {
        return ESP_ERR_NO_MEM;
    }
    if (xTaskCreate(usb_lib_task, "usb_lib", 4096, xTaskGetCurrentTaskHandle(), 10, NULL) !=
        pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    esp_err_t err = cdc_acm_host_install(NULL);
    if (err != ESP_OK) {
        return err;
    }
    if (xTaskCreate(serial_task, "usb_serial", 4096, NULL, 6, NULL) != pdPASS ||
        xTaskCreate(client_task, "usb_client", 4096, NULL, 5, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

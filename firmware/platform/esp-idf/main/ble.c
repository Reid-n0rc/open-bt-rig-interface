/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Bluetooth LE on NimBLE (ESP-IDF v6.0.3). NimBLE, not Bluedroid, because
 * the L2CAP CoC transport needs it (SPEC §13.3). Flow follows ESP-IDF's
 * examples/bluetooth/nimble/bleprph.
 *
 *  - GATT service and characteristics with the SPEC §13.1 UUIDs. RX and TX
 *    need an encrypted link with a bonded host; Info is readable by anyone.
 *  - Pairing: LE Secure Connections, "Just Works", bonding only while the
 *    pairing window is open (SPEC §13.5). With the window closed, a new host
 *    can connect and read Info, but its link is dropped if it pairs, and RX
 *    refuses an unbonded writer. (Refusing the pairing request itself, and
 *    the rest of the security design, is #64.)
 *  - TX power: capped at BLE_TX_POWER_CAP_DBM (ble_power.h).
 *  - L2CAP CoC (SPEC §13.3) comes later; Info reports it absent.
 */
#include "ble.h"

#include <string.h>

#include "ble_power.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "sdkconfig.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/* The cap must map to the ESP-IDF level it names, and the controller's
 * default level (used before the first esp_ble_tx_power_set) must not be
 * above it. */
_Static_assert(BLE_DBM_TO_LEVEL(BLE_TX_POWER_CAP_DBM) == ESP_PWR_LVL_P6,
               "BLE TX power cap must be ESP_PWR_LVL_P6 (+6 dBm)");
_Static_assert(CONFIG_BT_CTRL_DFT_TX_POWER_LEVEL_EFF <= ESP_PWR_LVL_P6,
               "sdkconfig: the default BLE TX power level is above the cap");

/* Declared by the NimBLE store config component (see bleprph). */
void ble_store_config_init(void);

static const char *TAG = "ble";

/* SPEC §13.1 UUIDs, little-endian byte order for BLE_UUID128_INIT. */
/* 274b5780-7929-4cf2-8a88-af1180d8fc04 */
static const ble_uuid128_t svc_uuid = BLE_UUID128_INIT(
    0x04, 0xfc, 0xd8, 0x80, 0x11, 0xaf, 0x88, 0x8a, 0xf2, 0x4c, 0x29, 0x79, 0x80, 0x57, 0x4b, 0x27);
/* 5c287ba9-de76-46de-8ca1-3519ad5fa899 */
static const ble_uuid128_t rx_uuid = BLE_UUID128_INIT(
    0x99, 0xa8, 0x5f, 0xad, 0x19, 0x35, 0xa1, 0x8c, 0xde, 0x46, 0x76, 0xde, 0xa9, 0x7b, 0x28, 0x5c);
/* 3b7d006a-e34e-448c-a911-7f683a261abf */
static const ble_uuid128_t tx_uuid = BLE_UUID128_INIT(
    0xbf, 0x1a, 0x26, 0x3a, 0x68, 0x7f, 0x11, 0xa9, 0x8c, 0x44, 0x4e, 0xe3, 0x6a, 0x00, 0x7d, 0x3b);
/* 9e6aea0a-b69f-4c23-9d32-ff0aa98bc000 */
static const ble_uuid128_t info_uuid = BLE_UUID128_INIT(
    0x00, 0xc0, 0x8b, 0xa9, 0x0a, 0xff, 0x32, 0x9d, 0x23, 0x4c, 0x9f, 0xb6, 0x0a, 0xea, 0x6a, 0x9e);

static uint16_t s_tx_handle;
static uint16_t s_conn = BLE_HS_CONN_HANDLE_NONE;
static volatile bool s_tx_subscribed;
static uint8_t s_own_addr_type;

static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static uint8_t s_info[PROTO_INFO_LEN];
static volatile bool s_pairing_open;

static void advertise(void);

void ble_publish(const uint8_t info[PROTO_INFO_LEN], bool pairing_open)
{
    taskENTER_CRITICAL(&s_lock);
    memcpy(s_info, info, PROTO_INFO_LEN);
    s_pairing_open = pairing_open;
    taskEXIT_CRITICAL(&s_lock);
}

static bool conn_bonded(uint16_t conn)
{
    struct ble_gap_conn_desc desc;
    return ble_gap_conn_find(conn, &desc) == 0 && desc.sec_state.encrypted &&
           desc.sec_state.bonded;
}

/* --- GATT --- */

static int rx_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)attr;
    (void)arg;
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) {
        return BLE_ATT_ERR_UNLIKELY;
    }
    if (!conn_bonded(conn)) {
        return BLE_ATT_ERR_INSUFFICIENT_AUTHEN; /* SPEC §13.5 */
    }
    static uint8_t buf[512];
    uint16_t len = 0;
    if (ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf), &len) != 0) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    (void)evt_post(EVT_RX, HAL_TRANSPORT_GATT, buf, len);
    return 0;
}

static int tx_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn;
    (void)attr;
    (void)ctxt;
    (void)arg;
    return BLE_ATT_ERR_READ_NOT_PERMITTED; /* notify only */
}

static int info_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn;
    (void)attr;
    (void)arg;
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR) {
        return BLE_ATT_ERR_UNLIKELY;
    }
    uint8_t v[PROTO_INFO_LEN];
    taskENTER_CRITICAL(&s_lock);
    memcpy(v, s_info, sizeof(v));
    taskEXIT_CRITICAL(&s_lock);
    return os_mbuf_append(ctxt->om, v, sizeof(v)) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &svc_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &rx_uuid.u,
                    .access_cb = rx_access,
                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP |
                             BLE_GATT_CHR_F_WRITE_ENC,
                },
                {
                    .uuid = &tx_uuid.u,
                    .access_cb = tx_access,
                    .val_handle = &s_tx_handle,
                    .flags = BLE_GATT_CHR_F_NOTIFY,
                },
                {
                    .uuid = &info_uuid.u,
                    .access_cb = info_access,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0},
            },
    },
    {0},
};

/* Stream bytes toward the host: TX notifications of at most ATT_MTU - 3. */
void hal_transport_send(uint8_t transport, const uint8_t *data, size_t len)
{
    if (transport != HAL_TRANSPORT_GATT) {
        return; /* L2CAP CoC and the wired transports come later */
    }
    uint16_t conn = s_conn;
    if (conn == BLE_HS_CONN_HANDLE_NONE || !s_tx_subscribed || !conn_bonded(conn)) {
        return;
    }
    size_t chunk = proto_gatt_chunk_size(ble_att_mtu(conn));
    while (len && chunk) {
        size_t n = len < chunk ? len : chunk;
        struct os_mbuf *om = ble_hs_mbuf_from_flat(data, (uint16_t)n);
        if (!om) {
            ESP_LOGW(TAG, "no mbuf: %u bytes dropped", (unsigned)len);
            return;
        }
        if (ble_gatts_notify_custom(conn, s_tx_handle, om) != 0) {
            ESP_LOGW(TAG, "notify failed");
            return;
        }
        data += n;
        len -= n;
    }
}

void hal_link_status(hal_link_status_t *out)
{
    memset(out, 0, sizeof(*out));
    out->host_link = HAL_LINK_BLUETOOTH;
    uint16_t conn = s_conn;
    if (conn == BLE_HS_CONN_HANDLE_NONE) {
        return;
    }
    struct ble_gap_conn_desc desc;
    if (ble_gap_conn_find(conn, &desc) == 0) {
        out->conn_interval_us = (uint32_t)desc.conn_itvl * 1250u; /* 1.25 ms units */
    }
    uint8_t tx_phy, rx_phy;
    if (ble_gap_read_le_phy(conn, &tx_phy, &rx_phy) == 0) {
        out->phy = tx_phy; /* 1 1M, 2 2M, 3 Coded: same values as STATUS.phy */
    }
    out->att_mtu = ble_att_mtu(conn);
}

static volatile int8_t s_tx_dbm = BLE_TX_POWER_CAP_DBM;
static volatile bool s_synced;

static void apply_tx_power(void)
{
    int8_t capped = ble_power_step_dbm(s_tx_dbm); /* never above the cap */
    esp_power_level_t level = (esp_power_level_t)BLE_DBM_TO_LEVEL(capped);
    esp_err_t e1 = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, level);
    esp_err_t e2 = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, level);
    if (e1 != ESP_OK || e2 != ESP_OK) {
        ESP_LOGW(TAG, "TX power set failed (%d, %d)", e1, e2);
    }
    /* Read back: the controller must never run above the cap. */
    esp_power_level_t adv = esp_ble_tx_power_get(ESP_BLE_PWR_TYPE_ADV);
    if (adv != ESP_PWR_LVL_INVALID && adv > ESP_PWR_LVL_P6) {
        ESP_LOGE(TAG, "TX power above the cap; restarting");
        abort();
    }
    ESP_LOGI(TAG, "BLE TX power %d dBm (cap %d dBm)", capped, BLE_TX_POWER_CAP_DBM);
}

/* The controller runs at its default level (the cap, sdkconfig) until the
 * host has synced; the requested level is applied then. */
void hal_ble_tx_power_set(int8_t dbm)
{
    s_tx_dbm = ble_power_step_dbm(dbm);
    if (s_synced) {
        apply_tx_power();
    }
}

/* --- GAP --- */

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    struct ble_gap_conn_desc desc;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status != 0) {
            advertise();
            return 0;
        }
        s_conn = event->connect.conn_handle;
        s_tx_subscribed = false;
        /* New bonds only while the pairing window is open (SPEC §13.5). */
        ble_hs_cfg.sm_bonding = s_pairing_open ? 1 : 0;
        ESP_LOGI(TAG, "connected; new bonds %s", s_pairing_open ? "allowed" : "refused");
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnected, reason 0x%x", event->disconnect.reason);
        s_conn = BLE_HS_CONN_HANDLE_NONE;
        s_tx_subscribed = false;
        (void)evt_post(EVT_TRANSPORT_CLOSED, HAL_TRANSPORT_GATT, NULL, 0);
        advertise();
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        if (event->enc_change.status == 0 && ble_gap_conn_find(event->enc_change.conn_handle, &desc) == 0 &&
            !desc.sec_state.bonded) {
            /* Encrypted but not bonded: outside the pairing window. */
            ESP_LOGW(TAG, "unbonded host: disconnecting");
            (void)ble_gap_terminate(event->enc_change.conn_handle, BLE_ERR_AUTH_FAIL);
        }
        return 0;

    case BLE_GAP_EVENT_PARING_COMPLETE:
        if (event->pairing_complete.status == 0 &&
            ble_gap_conn_find(event->pairing_complete.conn_handle, &desc) == 0 &&
            desc.sec_state.bonded) {
            if (s_pairing_open) {
                (void)evt_post(EVT_BOND_ADDED, 0, NULL, 0); /* the first new bond closes it */
            } else {
                (void)ble_store_util_delete_peer(&desc.peer_id_addr);
                (void)ble_gap_terminate(event->pairing_complete.conn_handle, BLE_ERR_AUTH_FAIL);
            }
        }
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* A bonded host lost its keys: re-pair only inside the window. */
        if (!s_pairing_open) {
            return BLE_GAP_REPEAT_PAIRING_IGNORE;
        }
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0) {
            (void)ble_store_util_delete_peer(&desc.peer_id_addr);
        }
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.attr_handle == s_tx_handle) {
            s_tx_subscribed = event->subscribe.cur_notify;
            if (!s_tx_subscribed) {
                (void)evt_post(EVT_TRANSPORT_CLOSED, HAL_TRANSPORT_GATT, NULL, 0);
            }
        }
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "ATT MTU %u", event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        advertise();
        return 0;

    default:
        return 0;
    }
}

static void advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128 = &svc_uuid; /* hosts can filter scans by it (SPEC §13.1) */
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "adv fields: %d", rc);
        return;
    }
    struct ble_hs_adv_fields rsp;
    memset(&rsp, 0, sizeof(rsp));
    const char *name = ble_svc_gap_device_name();
    rsp.name = (const uint8_t *)name;
    rsp.name_len = (uint8_t)strlen(name);
    rsp.name_is_complete = 1;
    rc = ble_gap_adv_rsp_set_fields(&rsp);
    if (rc != 0) {
        ESP_LOGE(TAG, "scan response: %d", rc);
        return;
    }
    struct ble_gap_adv_params params;
    memset(&params, 0, sizeof(params));
    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &params, gap_event, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "advertising: %d", rc);
    }
}

static void on_sync(void)
{
    int rc = ble_hs_util_ensure_addr(0);
    if (rc == 0) {
        rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "address: %d", rc);
        return;
    }
    s_synced = true;
    apply_tx_power();
    advertise();
    ESP_LOGI(TAG, "advertising");
}

static void on_reset(int reason)
{
    ESP_LOGE(TAG, "host reset, reason %d", reason);
}

static void host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_start(const char *device_name)
{
    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) {
        return err;
    }
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; /* full: drop the oldest bond */
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;            /* "Just Works" */
    ble_hs_cfg.sm_sc = 1;                                  /* LE Secure Connections */
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_bonding = 0; /* set per connection from the pairing window */
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_gatts_count_cfg(gatt_svcs);
    if (rc == 0) {
        rc = ble_gatts_add_svcs(gatt_svcs);
    }
    if (rc == 0) {
        rc = ble_svc_gap_device_name_set(device_name);
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "GATT setup: %d", rc);
        return ESP_FAIL;
    }
    ble_store_config_init();
    nimble_port_freertos_init(host_task);
    return ESP_OK;
}

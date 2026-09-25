/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * The device core: sessions and message dispatch (SPEC §4). See app.h.
 */
#include "app.h"

#include <string.h>

#include "ble_power.h"
#include "fw_version.h"
#include "security.h"

#define US_PER_MS 1000u

static uint64_t ms(uint64_t us)
{
    return us / US_PER_MS;
}

/* --- Sending --- */

static void send_on(app_t *app, uint8_t transport, const proto_msg_t *m)
{
    uint8_t wire[PROTO_MAX_WIRE];
    size_t n = proto_encode(m, wire, sizeof(wire));
    if (n && transport) {
        hal_transport_send(transport, wire, n);
    }
    (void)app;
}

static void send(app_t *app, const proto_msg_t *m)
{
    send_on(app, app->active, m);
}

static void send_result_on(app_t *app, uint8_t transport, uint8_t type, uint8_t token,
                           uint8_t code)
{
    proto_msg_t m = {.type = PROTO_RESULT, .token = token};
    m.u.result.request_type = type;
    m.u.result.code = code;
    send_on(app, transport, &m);
}

/* The one reply to a request (SPEC §4.3): RESULT unless a specific reply was
 * sent. Token 0 gets no reply on success; errors are always reported. */
static void reply_result(app_t *app, uint8_t type, uint8_t token, int code)
{
    if (code == PROTO_OK && token == 0) {
        return;
    }
    send_result_on(app, app->active, type, token, (uint8_t)code);
}

static void send_ptt_status(app_t *app, uint8_t token, const ptt_status_t *st)
{
    proto_msg_t m = {.type = PROTO_PTT_STATUS, .token = token};
    m.u.ptt_status.state = st->state;
    m.u.ptt_status.sources = st->sources;
    m.u.ptt_status.reason = st->reason;
    m.u.ptt_status.remaining_s = st->remaining_s;
    send(app, &m);
}

/* --- PTT controller glue --- */

static void ptt_closure_cb(void *ctx, bool on)
{
    (void)ctx;
    hal_ptt_closure_set(on);
}

static void ptt_radio_lines_cb(void *ctx, uint8_t port, uint8_t lines)
{
    (void)ctx;
    hal_radio_lines_set(port, lines);
}

static bool reason_sticky(uint8_t reason)
{
    return reason == PTT_R_WATCHDOG || reason == PTT_R_FAULT || reason == PTT_R_MAX_TX;
}

/* PTT_STATUS notifications: on the session's transport, or kept until the
 * next session starts, so a watchdog reset reaches the next session (§8.5).
 * A later routine status doesn't hide a pending WATCHDOG, FAULT or MAX_TX. */
static void ptt_status_cb(void *ctx, const ptt_status_t *st)
{
    app_t *app = (app_t *)ctx;
    if (app->active) {
        send_ptt_status(app, 0, st);
        return;
    }
    if (!app->status_pending || !reason_sticky(app->pending.reason) || reason_sticky(st->reason)) {
        app->pending = *st;
        app->status_pending = true;
    }
}

static void ptt_sources_dropped_cb(void *ctx, uint8_t sources, uint8_t reason)
{
    /* A device-keyed tone sequence would be cancelled here (#17). */
    (void)ctx;
    (void)sources;
    (void)reason;
}

static void apply_ptt_config(app_t *app)
{
    ptt_config_t pc;
    cfg_to_ptt(&app->cfg, app->board, &pc);
    ptt_configure(&app->ptt, &pc, ms(app->now_us));
}

/* --- Sessions (SPEC §4.2, §8.7) --- */

static void session_end(app_t *app, uint8_t reason)
{
    if (!app->active) {
        return;
    }
    uint64_t now = ms(app->now_us);
    ptt_all_off(&app->ptt, reason, now); /* PTT off, every line BLOCKED */
    audio_session_end(&app->audio);      /* stop audio */
    cat_close_all(&app->cat, &app->ptt, now); /* close ports, forget credit */
    /* A tone sequence would be cancelled here (#17). Config and the UTC
     * mapping are kept. */
    proto_rx_reset(&app->rx[app->active]);
    app->active = 0;
    app->version_ok = false;
}

static void handle_hello(app_t *app, uint8_t transport, const proto_msg_t *m)
{
    const proto_hello_t *h = &m->u.hello;
    session_end(app, PTT_R_LINK_LOST); /* a new HELLO restarts the session */

    app->active = transport;
    /* A Bluetooth host is authorized by its bond (the BLE layer delivers RX
     * only from bonded, encrypted links); a wired host needs AUTH (§15.2). */
    app->authorized = transport == HAL_TRANSPORT_GATT || transport == HAL_TRANSPORT_L2CAP;
    if (PROTO_VERSION_MAJOR == 0) {
        app->version_ok = h->proto_major == 0 && h->proto_minor == PROTO_VERSION_MINOR;
    } else {
        app->version_ok = h->proto_major == PROTO_VERSION_MAJOR;
    }
    uint16_t mp = h->max_payload;
    if (mp < PROTO_MIN_PAYLOAD_LIMIT) {
        mp = PROTO_MIN_PAYLOAD_LIMIT;
    }
    if (mp > PROTO_MAX_PAYLOAD) {
        mp = PROTO_MAX_PAYLOAD;
    }
    app->peer_max_payload = mp;
    app->frame_error_reported = false;

    proto_msg_t r = {.type = PROTO_DEVICE_INFO, .token = m->token};
    proto_device_info_t *d = &r.u.device_info;
    d->proto_major = PROTO_VERSION_MAJOR;
    d->proto_minor = PROTO_VERSION_MINOR;
    d->proto_patch = PROTO_VERSION_PATCH;
    d->fw_major = FW_VERSION_MAJOR;
    d->fw_minor = FW_VERSION_MINOR;
    d->fw_patch = FW_VERSION_PATCH;
    d->variant = app->board->variant;
    d->hw_revision = app->board->hw_revision;
    d->max_payload = PROTO_MAX_PAYLOAD;
    size_t blen = strlen(FW_VERSION_BUILD);
    d->build.data = (const uint8_t *)FW_VERSION_BUILD;
    d->build.len = (uint16_t)(blen > PROTO_BUILD_MAX ? PROTO_BUILD_MAX : blen);
    send(app, &r);

    if (app->status_pending) {
        app->status_pending = false;
        send_ptt_status(app, 0, &app->pending);
    }
}

/* --- CAPS (SPEC §5) --- */

static void send_caps(app_t *app, uint8_t token)
{
    const board_t *b = app->board;
    uint8_t buf[PROTO_MAX_PAYLOAD];
    size_t pos = 0;
    proto_tlv_t t;

    memset(&t, 0, sizeof(t));
    t.known = true;
    t.tag = PROTO_TLV_FEATURES;
    t.v.features.features = b->features & ~(uint32_t)PROTO_F_FIRMWARE_UPDATE;
    (void)proto_tlv_put(&t, buf, sizeof(buf), &pos);

    if (b->features & PROTO_F_CAT) {
        memset(&t, 0, sizeof(t));
        t.known = true;
        t.tag = PROTO_TLV_SERIAL_JACK;
        t.v.serial_jack.port = 0;
        t.v.serial_jack.modes = b->serial_modes;
        t.v.serial_jack.min_baud = b->serial_min_baud;
        t.v.serial_jack.max_baud = b->serial_max_baud;
        t.v.serial_jack.tx_buffer = b->cat_tx_buffer;
        (void)proto_tlv_put(&t, buf, sizeof(buf), &pos);
    }

    memset(&t, 0, sizeof(t));
    t.known = true;
    t.tag = PROTO_TLV_PTT;
    t.v.ptt.outputs = b->ptt_outputs;
    t.v.ptt.keepalive_min_ms = PTT_KEEPALIVE_MIN_MS;
    t.v.ptt.keepalive_max_ms = PTT_KEEPALIVE_MAX_MS;
    t.v.ptt.max_tx_min_s = PTT_MAX_TX_MIN_S;
    (void)proto_tlv_put(&t, buf, sizeof(buf), &pos);

    if (b->features & PROTO_F_BLE_TX_POWER) {
        memset(&t, 0, sizeof(t));
        t.known = true;
        t.tag = PROTO_TLV_BLE_TX_POWER;
        t.v.ble_tx_power.min_dbm = BLE_TX_POWER_MIN_DBM;
        t.v.ble_tx_power.max_dbm = BLE_TX_POWER_CAP_DBM;
        (void)proto_tlv_put(&t, buf, sizeof(buf), &pos);
    }
    if (b->features & PROTO_F_PAIRING_WINDOW) {
        memset(&t, 0, sizeof(t));
        t.known = true;
        t.tag = PROTO_TLV_PAIRING;
        t.v.pairing.triggers = b->pairing_triggers;
        t.v.pairing.window_min_s = CFG_PAIRING_WINDOW_MIN_S;
        t.v.pairing.window_max_s = CFG_PAIRING_WINDOW_MAX_S;
        t.v.pairing.max_bonds = b->max_bonds;
        t.v.pairing.max_wired_hosts = b->max_wired_hosts;
        (void)proto_tlv_put(&t, buf, sizeof(buf), &pos);
    }

    proto_msg_t r = {.type = PROTO_CAPS, .token = token};
    r.u.caps.tlvs.data = buf;
    r.u.caps.tlvs.len = (uint16_t)pos;
    send(app, &r);
}

/* --- STATUS (SPEC §16.1) --- */

#define STATUS_F_UTC_FRESH (1u << 3)
#define STATUS_F_AUDIO_RUNNING (1u << 5)
#define STATUS_F_PAIRING_OPEN (1u << 6)
#define STATUS_F_HW_MASK ((1u << 0) | (1u << 1) | (1u << 2) | (1u << 4))

static void send_status(app_t *app, uint8_t token)
{
    hal_link_status_t link;
    memset(&link, 0, sizeof(link));
    hal_link_status(&link);

    proto_msg_t r = {.type = PROTO_STATUS, .token = token};
    proto_status_t *s = &r.u.status;
    s->host_link = link.host_link;
    s->transport = app->active;
    s->phy = link.phy;
    s->conn_interval_us = link.conn_interval_us;
    s->att_mtu = link.att_mtu;
    s->coc_mtu = link.coc_mtu;
    s->flags = (uint16_t)(link.flags & STATUS_F_HW_MASK);
    if (clock_sync_fresh(&app->clock, app->now_us)) {
        s->flags |= STATUS_F_UTC_FRESH;
    }
    if (app->audio.running) {
        s->flags |= STATUS_F_AUDIO_RUNNING;
    }
    if (pairing_is_open(&app->pairing)) {
        s->flags |= STATUS_F_PAIRING_OPEN;
    }
    s->audio_path = link.audio_path;
    s->supply_mv = link.supply_mv;
    s->frame_errors = app->frame_errors;
    s->cat_overflows = app->cat.overflows;
    send(app, &r);
}

/* --- Configuration (SPEC §6) --- */

static bool persist_allowed(app_t *app)
{
    uint64_t now = ms(app->now_us);
    if (app->persisted && now - app->persist_ms < 1000u) {
        return false; /* at most one persisted write per second (§6, §12.2) */
    }
    return true;
}

static int persist(app_t *app)
{
    uint8_t blob[CFG_BLOB_MAX];
    size_t n = cfg_serialize(&app->cfg, blob, sizeof(blob));
    app->persisted = true;
    app->persist_ms = ms(app->now_us);
    if (n == 0 || hal_config_save(blob, n) != 0) {
        return PROTO_ERR_INTERNAL;
    }
    return PROTO_OK;
}

static bool is_ptt_key(uint8_t key)
{
    return key == PROTO_KEY_PTT_TARGETS || key == PROTO_KEY_LINE_MAP ||
           key == PROTO_KEY_PTT_KEEPALIVE_MS || key == PROTO_KEY_MAX_TX_S ||
           key == PROTO_KEY_WIRED_PORT_LOCK;
}

static void apply_side_effects(app_t *app, uint8_t key)
{
    if (is_ptt_key(key)) {
        apply_ptt_config(app); /* never asserts PTT; a map or target change releases it */
    } else if (key == PROTO_KEY_SERIAL_JACK_MODE) {
        hal_serial_jack_mode(app->cfg.serial_jack_mode); /* never asserts PTT (§6.2) */
    } else if (key == PROTO_KEY_BLE_TX_POWER) {
        hal_ble_tx_power_set(ble_power_step_dbm(app->cfg.ble_tx_power));
    }
    /* HOST_MODE, WIRED_PROFILE and USB_NET_SUBNET take effect at the next
     * mode evaluation or enumeration (#44); PAIRING_WINDOW_S the next time
     * the window opens; audio keys with #16. */
}

static void handle_config_set(app_t *app, const proto_msg_t *m)
{
    const proto_config_set_t *cs = &m->u.config_set;
    bool want_persist = (cs->flags & 0x01u) != 0;
    if (want_persist && !persist_allowed(app)) {
        reply_result(app, m->type, m->token, PROTO_ERR_RATE_LIMITED);
        return;
    }
    int rc = cfg_set(&app->cfg, app->board, &cs->config);
    if (rc != PROTO_OK) {
        reply_result(app, m->type, m->token, rc);
        return;
    }
    apply_side_effects(app, cs->config.key);
    if (want_persist && persist(app) != PROTO_OK) {
        reply_result(app, m->type, m->token, PROTO_ERR_INTERNAL);
        return;
    }
    if (m->token) {
        proto_msg_t r = {.type = PROTO_CONFIG, .token = m->token};
        (void)cfg_get(&app->cfg, cs->config.key, cs->config.selector, &r.u.config.config);
        send(app, &r);
    }
}

static void handle_config_reset(app_t *app, const proto_msg_t *m)
{
    bool want_persist = (m->u.config_reset.flags & 0x01u) != 0;
    if (want_persist && !persist_allowed(app)) {
        reply_result(app, m->type, m->token, PROTO_ERR_RATE_LIMITED);
        return;
    }
    cfg_defaults(&app->cfg, app->board);
    apply_ptt_config(app);
    hal_serial_jack_mode(app->cfg.serial_jack_mode);
    hal_ble_tx_power_set(ble_power_step_dbm(app->cfg.ble_tx_power));
    int rc = want_persist ? persist(app) : PROTO_OK;
    reply_result(app, m->type, m->token, rc);
}

/* --- Security (SPEC §15). The stores behind it are #64. --- */

#define AUTH_APPROVED 0u
#define AUTH_NOT_APPROVED 1u
#define AUTH_REFUSED 2u
#define AUTH_NO_SLOT 0xFFu

static void handle_auth(app_t *app, const proto_msg_t *m)
{
    proto_msg_t r = {.type = PROTO_AUTH_STATUS, .token = m->token};
    r.u.auth_status.state = AUTH_NOT_APPROVED;
    r.u.auth_status.slot = AUTH_NO_SLOT;
    sec_host_id_t id;
    memcpy(id.id, m->u.auth.host_token, sizeof(id.id));

    if (app->active == HAL_TRANSPORT_GATT || app->active == HAL_TRANSPORT_L2CAP) {
        r.u.auth_status.state = AUTH_APPROVED; /* the bond already authorizes */
    } else if (sec_wired_host_approved(&id)) {
        app->authorized = true;
        r.u.auth_status.state = AUTH_APPROVED;
    } else if (pairing_is_open(&app->pairing)) {
        /* Only the local action that opened the window approves a new host. */
        int rc = sec_wired_host_approve(&id, true);
        if (rc == PROTO_OK) {
            app->authorized = true;
            pairing_bond_added(&app->pairing); /* the approval closes the window */
            r.u.auth_status.state = AUTH_APPROVED;
        } else if (rc == PROTO_ERR_OVERFLOW) {
            r.u.auth_status.state = AUTH_REFUSED; /* no free slot */
        }
    }
    send(app, &r);
}

/* --- Wired-mode availability (SPEC §14) --- */

static bool serial_allowed(const app_t *app, uint8_t port)
{
    if (app->host_link != HAL_LINK_WIRED) {
        return true;
    }
    /* Only port 0, only over the USB network in the network profile. */
    return port == 0 && app->cfg.wired_profile == 0 && app->active == HAL_TRANSPORT_TCP;
}

/* --- Dispatch --- */

static void dispatch(app_t *app, const proto_msg_t *m)
{
    uint64_t now = ms(app->now_us);
    int rc = PROTO_OK;

    if (!app->version_ok && m->type != PROTO_CAPS_GET && m->type != PROTO_PING) {
        reply_result(app, m->type, m->token, PROTO_ERR_VERSION_MISMATCH);
        return;
    }
    if (!app->authorized && m->type != PROTO_CAPS_GET && m->type != PROTO_PING &&
        m->type != PROTO_AUTH) {
        reply_result(app, m->type, m->token, PROTO_ERR_NOT_AUTHORIZED); /* §15.2 */
        return;
    }
    if (proto_type_dir(m->type) == PROTO_D2H) {
        /* A device -> host message sent to the device. */
        reply_result(app, m->type, m->token, PROTO_ERR_UNKNOWN_TYPE);
        return;
    }

    switch (m->type) {
    case PROTO_CAPS_GET:
        send_caps(app, m->token);
        return;
    case PROTO_PING: {
        if (m->u.ping.data.len > PROTO_PING_MAX) {
            rc = PROTO_ERR_BAD_LENGTH;
            break;
        }
        proto_msg_t r = {.type = PROTO_PONG, .token = m->token};
        r.u.pong.data = m->u.ping.data;
        send(app, &r);
        return;
    }
    case PROTO_STATUS_GET:
        send_status(app, m->token);
        return;
    case PROTO_CONFIG_GET: {
        proto_msg_t r = {.type = PROTO_CONFIG, .token = m->token};
        rc = cfg_get(&app->cfg, m->u.config_get.key, m->u.config_get.selector, &r.u.config.config);
        if (rc == PROTO_OK) {
            send(app, &r);
            return;
        }
        break;
    }
    case PROTO_CONFIG_SET:
        handle_config_set(app, m);
        return;
    case PROTO_CONFIG_RESET:
        handle_config_reset(app, m);
        return;
    case PROTO_SERIAL_OPEN:
        rc = serial_allowed(app, m->u.serial_open.port)
                 ? cat_open(&app->cat, &app->ptt, app->board, &app->cfg, m->u.serial_open.port,
                            m->u.serial_open.open, now)
                 : PROTO_ERR_UNSUPPORTED;
        break;
    case PROTO_SERIAL_SET:
        rc = serial_allowed(app, m->u.serial_set.port)
                 ? cat_set(&app->cat, app->board, &m->u.serial_set)
                 : PROTO_ERR_UNSUPPORTED;
        break;
    case PROTO_CAT_DATA: {
        uint16_t written = 0;
        uint8_t port = m->u.cat_data.port;
        rc = serial_allowed(app, port)
                 ? cat_from_host(&app->cat, port, m->u.cat_data.data.data, m->u.cat_data.data.len,
                                 &written)
                 : PROTO_ERR_UNSUPPORTED;
        if (written) {
            proto_msg_t r = {.type = PROTO_CAT_CREDIT, .token = 0};
            r.u.cat_credit.port = port;
            r.u.cat_credit.credit = written;
            send(app, &r);
        }
        break;
    }
    case PROTO_MODEM_LINES:
        rc = serial_allowed(app, m->u.modem_lines.port)
                 ? cat_modem_lines(&app->cat, &app->ptt, m->u.modem_lines.port,
                                   m->u.modem_lines.lines, now)
                 : PROTO_ERR_UNSUPPORTED;
        break;
    case PROTO_PTT_SET: {
        if (m->u.ptt_set.state > 1) {
            rc = PROTO_ERR_BAD_VALUE;
            break;
        }
        ptt_status_t st;
        bool changed = ptt_set(&app->ptt, m->u.ptt_set.state == 1, now, &st);
        if (m->token || changed) {
            send_ptt_status(app, m->token, &st); /* the reply, or a change with token 0 */
        }
        return;
    }
    case PROTO_KEEPALIVE:
        ptt_keepalive(&app->ptt, now);
        break;
    case PROTO_AUDIO_START:
    case PROTO_AUDIO_STOP: {
        proto_msg_t r = {.type = PROTO_AUDIO_STATUS, .token = m->token};
        if (app->host_link == HAL_LINK_WIRED) {
            rc = PROTO_ERR_UNSUPPORTED; /* wired audio is USB Audio Class */
            break;
        }
        rc = m->type == PROTO_AUDIO_START
                 ? audio_start(&app->audio, app->board, &m->u.audio_start, &r.u.audio_status)
                 : audio_stop(&app->audio, app->board, m->u.audio_stop.direction,
                              &r.u.audio_status);
        if (rc == PROTO_OK) {
            send(app, &r);
            return;
        }
        break;
    }
    case PROTO_AUDIO_FRAME:
        rc = app->host_link == HAL_LINK_WIRED ? PROTO_ERR_UNSUPPORTED
                                              : audio_frame(&app->audio, app->board,
                                                            &m->u.audio_frame);
        break;
    case PROTO_TIME_REQ: {
        proto_msg_t r = {.type = PROTO_TIME_RESP, .token = m->token};
        clock_sync_resp(&m->u.time_req, app->now_us, hal_time_us(), &r.u.time_resp);
        send(app, &r);
        return;
    }
    case PROTO_TIME_SET:
        rc = clock_sync_set(&app->clock, &m->u.time_set, app->now_us);
        break;
    case PROTO_TONE_SETUP:
    case PROTO_TONE_DATA:
    case PROTO_TONE_START:
    case PROTO_TONE_CANCEL:
        rc = PROTO_ERR_UNSUPPORTED; /* optional module, #17 */
        break;
    case PROTO_AUTH:
        handle_auth(app, m);
        return;
    case PROTO_TRUST_LIST_GET:
    case PROTO_TRUST_REMOVE:
        rc = PROTO_ERR_UNSUPPORTED; /* the bond and host stores are #64 */
        break;
    case PROTO_FACTORY_RESET:
        rc = m->u.factory_reset.confirm == PROTO_FACTORY_RESET_CONFIRM ? sec_factory_reset()
                                                                       : PROTO_ERR_BAD_VALUE;
        break;
    default:
        rc = PROTO_ERR_UNKNOWN_TYPE;
        break;
    }
    reply_result(app, m->type, m->token, rc);
}

static void on_frame(void *ctx, uint8_t type, uint8_t token, const uint8_t *payload, size_t len)
{
    app_t *app = (app_t *)ctx;
    uint8_t transport = app->rx_transport;
    proto_msg_t m;
    int rc = proto_decode_payload(type, token, payload, len, &m);

    if (type == PROTO_HELLO) {
        if (rc == PROTO_OK) {
            handle_hello(app, transport, &m);
        } else {
            send_result_on(app, transport, type, token, (uint8_t)rc);
        }
        return;
    }
    if (transport != app->active) {
        return; /* no session on this transport: only HELLO is handled */
    }
    if (rc != PROTO_OK) {
        reply_result(app, type, token, rc);
        return;
    }
    dispatch(app, &m);
}

static void on_frame_error(void *ctx)
{
    app_t *app = (app_t *)ctx;
    uint64_t now = ms(app->now_us);
    if (app->frame_errors < 0xFFFFu) {
        app->frame_errors++;
    }
    if (app->rx_transport != app->active || !app->active) {
        return;
    }
    /* RESULT FRAME_ERROR, token 0, at most once per second (SPEC §3.4). */
    if (!app->frame_error_reported || now - app->frame_error_report_ms >= 1000u) {
        app->frame_error_reported = true;
        app->frame_error_report_ms = now;
        send_result_on(app, app->active, 0, 0, PROTO_ERR_FRAME);
    }
}

/* --- Public API --- */

uint8_t app_boot_reason(app_reset_t reset)
{
    switch (reset) {
    case APP_RESET_POWER_ON:
        return PTT_R_BOOT;
    case APP_RESET_WATCHDOG:
        return PTT_R_WATCHDOG;
    default: /* brownout, panic, anything else */
        return PTT_R_FAULT;
    }
}

void app_init(app_t *app, const board_t *board, uint8_t host_link, uint8_t boot_reason,
              uint64_t now_us)
{
    memset(app, 0, sizeof(*app));
    app->board = board;
    app->host_link = host_link;
    app->now_us = now_us;

    /* PTT first: every output off before anything else runs. */
    cfg_defaults(&app->cfg, board);
    ptt_config_t pc;
    cfg_to_ptt(&app->cfg, board, &pc);
    ptt_ops_t ops = {ptt_closure_cb, ptt_radio_lines_cb, ptt_status_cb, ptt_sources_dropped_cb,
                     app};
    ptt_init(&app->ptt, &pc, &ops, ms(now_us), boot_reason);

    /* Stored configuration, checked key by key. */
    uint8_t blob[CFG_BLOB_MAX];
    size_t n = hal_config_load(blob, sizeof(blob));
    if (n) {
        (void)cfg_deserialize(&app->cfg, board, blob, n);
        apply_ptt_config(app);
    }
    hal_serial_jack_mode(app->cfg.serial_jack_mode);
    hal_ble_tx_power_set(ble_power_step_dbm(app->cfg.ble_tx_power));

    pairing_init(&app->pairing);
    clock_sync_init(&app->clock);
    cat_init(&app->cat);
    audio_init(&app->audio);
    for (unsigned i = 0; i <= HAL_TRANSPORT_MAX; i++) {
        proto_rx_init(&app->rx[i], on_frame, on_frame_error, app);
    }
    /* Power-on opens the pairing window (SPEC §13.5). */
    if (board->pairing_triggers & BOARD_PAIR_POWER_ON) {
        pairing_open_local(&app->pairing, app->cfg.pairing_window_s, ms(now_us));
    }
}

void app_rx(app_t *app, uint8_t transport, const uint8_t *data, size_t len, uint64_t now_us)
{
    if (transport == 0 || transport > HAL_TRANSPORT_MAX) {
        return;
    }
    app->now_us = now_us;
    app->rx_transport = transport;
    proto_rx_feed(&app->rx[transport], data, len);
    app->rx_transport = 0;
}

void app_transport_closed(app_t *app, uint8_t transport, uint64_t now_us)
{
    if (transport == 0 || transport > HAL_TRANSPORT_MAX) {
        return;
    }
    app->now_us = now_us;
    proto_rx_reset(&app->rx[transport]);
    if (transport == app->active) {
        session_end(app, PTT_R_LINK_LOST); /* loss of the active transport (§8.5) */
    }
}

void app_tick(app_t *app, uint64_t now_us)
{
    app->now_us = now_us;
    ptt_tick(&app->ptt, ms(now_us));
    pairing_tick(&app->pairing, ms(now_us));
}

void app_native_lines(app_t *app, uint8_t port, uint8_t lines, uint64_t now_us)
{
    app->now_us = now_us;
    if (app->host_link != HAL_LINK_WIRED || (port != 0 && port != PTT_PORT_CONTROL)) {
        return;
    }
    ptt_lines(&app->ptt, port, lines, PTT_ORIGIN_NATIVE, ms(now_us));
}

void app_usb_reset(app_t *app, uint64_t now_us)
{
    app->now_us = now_us;
    if (app->active == HAL_TRANSPORT_CONTROL || app->active == HAL_TRANSPORT_TCP) {
        session_end(app, PTT_R_LINK_LOST); /* a wired session ends on USB reset (§14) */
    }
    ptt_native_reset(&app->ptt, ms(now_us));
}

void app_host_link_changed(app_t *app, uint8_t host_link, uint64_t now_us)
{
    app->now_us = now_us;
    session_end(app, PTT_R_MODE_CHANGE);
    ptt_all_off(&app->ptt, PTT_R_MODE_CHANGE, ms(now_us)); /* REQ-PTT-005 */
    app->host_link = host_link;
    pairing_close(&app->pairing); /* a host-mode change closes the window (§13.5) */
}

void app_fault(app_t *app, uint64_t now_us)
{
    app->now_us = now_us;
    ptt_all_off(&app->ptt, PTT_R_FAULT, ms(now_us));
}

void app_pairing_button(app_t *app, uint64_t now_us)
{
    app->now_us = now_us;
    /* The same window allows new bonds and approves wired hosts (§15.2). */
    pairing_open_local(&app->pairing, app->cfg.pairing_window_s, ms(now_us));
}

void app_bond_added(app_t *app)
{
    pairing_bond_added(&app->pairing);
}

bool app_pairing_open(const app_t *app)
{
    return pairing_is_open(&app->pairing);
}

void app_cat_from_radio(app_t *app, uint8_t port, const uint8_t *data, size_t len)
{
    if (!app->active || !cat_is_open(&app->cat, port)) {
        return;
    }
    size_t chunk_max = (size_t)app->peer_max_payload - 1u; /* CAT_DATA: port + data */
    while (len) {
        size_t n = len < chunk_max ? len : chunk_max;
        proto_msg_t m = {.type = PROTO_CAT_DATA, .token = 0};
        m.u.cat_data.port = port;
        m.u.cat_data.data.data = data;
        m.u.cat_data.data.len = (uint16_t)n;
        send(app, &m);
        data += n;
        len -= n;
    }
}

void app_info(const app_t *app, uint8_t out[PROTO_INFO_LEN])
{
    proto_info_t info = {PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR, PROTO_VERSION_PATCH, 0,
                         app->board->l2cap_psm};
    if (app->board->l2cap_psm) {
        info.flags |= PROTO_INFO_F_L2CAP;
    }
    if (pairing_is_open(&app->pairing)) {
        info.flags |= PROTO_INFO_F_PAIRING_OPEN;
    }
    proto_info_encode(&info, out);
}

bool app_led(const app_t *app, uint64_t now_us)
{
    uint64_t t = ms(now_us);
    if (pairing_is_open(&app->pairing)) {
        return (t / 125u) % 2u == 0; /* 4 Hz */
    }
    if (app->active) {
        return true;
    }
    return (t / 500u) % 2u == 0; /* 1 Hz */
}

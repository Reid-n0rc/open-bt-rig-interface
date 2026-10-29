/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Device configuration (SPEC §6). See cfg.h.
 */
#include "cfg.h"

#include <string.h>

#include "ble_power.h"

#define BLOB_MAGIC0 'C'
#define BLOB_MAGIC1 'F'
#define BLOB_VERSION 1u

void cfg_defaults(cfg_t *cfg, const board_t *board)
{
    (void)board; /* every default is board-independent so far */
    ptt_config_t p;
    ptt_config_defaults(&p);

    memset(cfg, 0, sizeof(*cfg));
    cfg->serial_jack_mode = 0; /* 3.3 V logic */
    cfg->ptt_targets = p.targets;
    cfg->ptt_usb_port = p.usb_port;
    memcpy(cfg->rts_action, p.rts_action, sizeof(cfg->rts_action));
    memcpy(cfg->dtr_action, p.dtr_action, sizeof(cfg->dtr_action));
    cfg->ptt_keepalive_ms = p.keepalive_ms;
    cfg->max_tx_s = p.max_tx_s;
    cfg->audio_path = 0;
    cfg->tx_level = CFG_TX_LEVEL_DEFAULT;
    cfg->rx_attenuator = 0;
    cfg->rx_gain = CFG_RX_GAIN_DEFAULT;
    cfg->host_mode = 0;
    cfg->ble_tx_power = BLE_TX_POWER_CAP_DBM; /* default: the cap */
    cfg->wired_profile = 1;                   /* serial */
    for (unsigned i = 0; i < CFG_SERIAL_PORTS; i++) {
        cfg->serial_default[i].baud = CFG_SERIAL_DEFAULT_BAUD; /* 9600 8N1 */
        cfg->serial_default[i].data_bits = 8;
        cfg->serial_default[i].parity = 0;
        cfg->serial_default[i].stop_bits = 0;
    }
    cfg->usb_net_subnet[0] = 10; /* 10.169.160.0/30 */
    cfg->usb_net_subnet[1] = 169;
    cfg->usb_net_subnet[2] = 160;
    cfg->usb_net_subnet[3] = 0;
    cfg->pairing_window_s = CFG_PAIRING_WINDOW_DEFAULT_S;
    cfg->power_down_delay_s = CFG_POWER_DOWN_DEFAULT_S;
    cfg->wired_port_lock = 0; /* open */
}

static bool selector_ok(uint8_t key, uint8_t selector)
{
    switch (key) {
    case PROTO_KEY_LINE_MAP:
        return ptt_port_index(selector) >= 0;
    case PROTO_KEY_SERIAL_DEFAULT:
        return selector < CFG_SERIAL_PORTS;
    default:
        return selector == 0;
    }
}

int cfg_get(const cfg_t *cfg, uint8_t key, uint8_t selector, proto_config_t *out)
{
    memset(out, 0, sizeof(*out));
    out->key = key;
    out->selector = selector;
    if (proto_config_value_len(key) < 0 || !selector_ok(key, selector)) {
        return PROTO_ERR_BAD_VALUE;
    }
    switch (key) {
    case PROTO_KEY_SERIAL_JACK_MODE:
        out->value.serial_jack_mode.mode = cfg->serial_jack_mode;
        break;
    case PROTO_KEY_PTT_TARGETS:
        out->value.ptt_targets.targets = cfg->ptt_targets;
        out->value.ptt_targets.usb_port = cfg->ptt_usb_port;
        break;
    case PROTO_KEY_LINE_MAP: {
        int idx = ptt_port_index(selector);
        out->value.line_map.rts_action = cfg->rts_action[idx];
        out->value.line_map.dtr_action = cfg->dtr_action[idx];
        break;
    }
    case PROTO_KEY_PTT_KEEPALIVE_MS:
        out->value.ptt_keepalive_ms.ms = cfg->ptt_keepalive_ms;
        break;
    case PROTO_KEY_MAX_TX_S:
        out->value.max_tx_s.s = cfg->max_tx_s;
        break;
    case PROTO_KEY_AUDIO_PATH:
        out->value.audio_path.path = cfg->audio_path;
        break;
    case PROTO_KEY_TX_LEVEL:
        out->value.tx_level.centibel = cfg->tx_level;
        break;
    case PROTO_KEY_RX_ATTENUATOR:
        out->value.rx_attenuator.on = cfg->rx_attenuator;
        break;
    case PROTO_KEY_RX_GAIN:
        out->value.rx_gain.centibel = cfg->rx_gain;
        break;
    case PROTO_KEY_HOST_MODE:
        out->value.host_mode.mode = cfg->host_mode;
        break;
    case PROTO_KEY_BLE_TX_POWER:
        out->value.ble_tx_power.dbm = cfg->ble_tx_power;
        break;
    case PROTO_KEY_WIRED_PROFILE:
        out->value.wired_profile.profile = cfg->wired_profile;
        break;
    case PROTO_KEY_SERIAL_DEFAULT: {
        const cfg_serial_t *s = &cfg->serial_default[selector];
        out->value.serial_default.baud = s->baud;
        out->value.serial_default.data_bits = s->data_bits;
        out->value.serial_default.parity = s->parity;
        out->value.serial_default.stop_bits = s->stop_bits;
        break;
    }
    case PROTO_KEY_USB_NET_SUBNET:
        out->value.usb_net_subnet.a = cfg->usb_net_subnet[0];
        out->value.usb_net_subnet.b = cfg->usb_net_subnet[1];
        out->value.usb_net_subnet.c = cfg->usb_net_subnet[2];
        out->value.usb_net_subnet.d = cfg->usb_net_subnet[3];
        break;
    case PROTO_KEY_PAIRING_WINDOW_S:
        out->value.pairing_window_s.s = cfg->pairing_window_s;
        break;
    case PROTO_KEY_POWER_DOWN_DELAY_S:
        out->value.power_down_delay_s.s = cfg->power_down_delay_s;
        break;
    default: /* PROTO_KEY_WIRED_PORT_LOCK */
        out->value.wired_port_lock.level = cfg->wired_port_lock;
        break;
    }
    return PROTO_OK;
}

static bool private_subnet(uint8_t a, uint8_t b)
{
    return a == 10 || (a == 172 && (b & 0xF0u) == 16) || (a == 192 && b == 168);
}

int cfg_set(cfg_t *cfg, const board_t *board, const proto_config_t *in)
{
    if (proto_config_value_len(in->key) < 0 || !selector_ok(in->key, in->selector)) {
        return PROTO_ERR_BAD_VALUE;
    }
    switch (in->key) {
    case PROTO_KEY_SERIAL_JACK_MODE: {
        uint8_t mode = in->value.serial_jack_mode.mode;
        if (mode > 3) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (board && !(board->serial_modes & (1u << mode))) {
            return PROTO_ERR_UNSUPPORTED;
        }
        cfg->serial_jack_mode = mode;
        return PROTO_OK;
    }
    case PROTO_KEY_PTT_TARGETS: {
        /* Reserved bits are ignored (SPEC §2). */
        uint8_t targets = in->value.ptt_targets.targets &
                          (PTT_TARGET_CLOSURE | PTT_TARGET_RTS | PTT_TARGET_DTR);
        uint8_t port = in->value.ptt_targets.usb_port;
        if (targets & (PTT_TARGET_RTS | PTT_TARGET_DTR)) {
            if (port < 1 || port > 4) {
                return PROTO_ERR_BAD_VALUE;
            }
        } else if (port != 0) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (board && (targets & (uint8_t)~board->ptt_outputs)) {
            return PROTO_ERR_UNSUPPORTED;
        }
        cfg->ptt_targets = targets;
        cfg->ptt_usb_port = port;
        return PROTO_OK;
    }
    case PROTO_KEY_LINE_MAP: {
        int idx = ptt_port_index(in->selector);
        uint8_t rts = in->value.line_map.rts_action;
        uint8_t dtr = in->value.line_map.dtr_action;
        bool radio = idx >= 1 && idx <= 4;
        if (rts > PTT_ACT_PASS || dtr > PTT_ACT_PASS) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (!radio && (rts == PTT_ACT_PASS || dtr == PTT_ACT_PASS)) {
            return PROTO_ERR_BAD_VALUE; /* action 2 only on ports 1-4 (SPEC §8.3) */
        }
        cfg->rts_action[idx] = rts;
        cfg->dtr_action[idx] = dtr;
        return PROTO_OK;
    }
    case PROTO_KEY_PTT_KEEPALIVE_MS: {
        uint16_t ms = in->value.ptt_keepalive_ms.ms;
        if (ms < PTT_KEEPALIVE_MIN_MS || ms > PTT_KEEPALIVE_MAX_MS) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->ptt_keepalive_ms = ms;
        return PROTO_OK;
    }
    case PROTO_KEY_MAX_TX_S: {
        uint32_t s = in->value.max_tx_s.s;
        /* 0 switches the max-TX timer off (maintainer, 2026-09-25); otherwise
         * at least 10 s, with no upper bound. The keepalive, disconnect and
         * watchdog fail-safes stay on whatever this is. */
        if (s != PTT_MAX_TX_DISABLED && s < PTT_MAX_TX_MIN_S) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->max_tx_s = s;
        return PROTO_OK;
    }
    case PROTO_KEY_AUDIO_PATH:
        if (in->value.audio_path.path > 2) {
            return PROTO_ERR_BAD_VALUE;
        }
        cfg->audio_path = in->value.audio_path.path;
        return PROTO_OK;
    case PROTO_KEY_TX_LEVEL:
        if (in->value.tx_level.centibel > 0) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->tx_level = in->value.tx_level.centibel;
        return PROTO_OK;
    case PROTO_KEY_RX_ATTENUATOR:
        if (in->value.rx_attenuator.on > 1) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (board && !(board->features & PROTO_F_RX_ATTENUATOR) && in->value.rx_attenuator.on) {
            return PROTO_ERR_UNSUPPORTED;
        }
        cfg->rx_attenuator = in->value.rx_attenuator.on;
        return PROTO_OK;
    case PROTO_KEY_RX_GAIN:
        cfg->rx_gain = in->value.rx_gain.centibel;
        return PROTO_OK;
    case PROTO_KEY_HOST_MODE:
        if (in->value.host_mode.mode > 2) {
            return PROTO_ERR_BAD_VALUE;
        }
        cfg->host_mode = in->value.host_mode.mode;
        return PROTO_OK;
    case PROTO_KEY_BLE_TX_POWER: {
        int dbm = in->value.ble_tx_power.dbm;
        /* The protocol can only lower the power (SPEC §6.3). */
        if (dbm < BLE_TX_POWER_MIN_DBM || dbm > BLE_TX_POWER_CAP_DBM) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->ble_tx_power = ble_power_step_dbm(dbm); /* the value in effect */
        return PROTO_OK;
    }
    case PROTO_KEY_WIRED_PROFILE:
        if (in->value.wired_profile.profile > 1) {
            return PROTO_ERR_BAD_VALUE;
        }
        cfg->wired_profile = in->value.wired_profile.profile;
        return PROTO_OK;
    case PROTO_KEY_SERIAL_DEFAULT: {
        const uint8_t port = in->selector;
        uint32_t baud = in->value.serial_default.baud;
        if (in->value.serial_default.data_bits != 7 && in->value.serial_default.data_bits != 8) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (in->value.serial_default.parity > 4 || in->value.serial_default.stop_bits > 2) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (baud == 0) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        /* Port 0 has the SERIAL_JACK TLV's range; a radio port's chip checks
         * its own range when the port opens. */
        if (port == 0 && board && (baud < board->serial_min_baud || baud > board->serial_max_baud)) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->serial_default[port].baud = baud;
        cfg->serial_default[port].data_bits = in->value.serial_default.data_bits;
        cfg->serial_default[port].parity = in->value.serial_default.parity;
        cfg->serial_default[port].stop_bits = in->value.serial_default.stop_bits;
        return PROTO_OK;
    }
    case PROTO_KEY_USB_NET_SUBNET: {
        uint8_t a = in->value.usb_net_subnet.a, b = in->value.usb_net_subnet.b;
        uint8_t c = in->value.usb_net_subnet.c, d = in->value.usb_net_subnet.d;
        if (!private_subnet(a, b) || (d % 4u) != 0) {
            return PROTO_ERR_BAD_VALUE;
        }
        cfg->usb_net_subnet[0] = a;
        cfg->usb_net_subnet[1] = b;
        cfg->usb_net_subnet[2] = c;
        cfg->usb_net_subnet[3] = d;
        return PROTO_OK;
    }
    case PROTO_KEY_PAIRING_WINDOW_S: {
        uint16_t s = in->value.pairing_window_s.s;
        if (s < CFG_PAIRING_WINDOW_MIN_S || s > CFG_PAIRING_WINDOW_MAX_S) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->pairing_window_s = s;
        return PROTO_OK;
    }
    case PROTO_KEY_POWER_DOWN_DELAY_S: {
        uint16_t s = in->value.power_down_delay_s.s;
        /* 0 = never; otherwise 5-3600 s (SPEC §6.1). */
        if (s != 0 && (s < CFG_POWER_DOWN_MIN_S || s > CFG_POWER_DOWN_MAX_S)) {
            return PROTO_ERR_OUT_OF_RANGE;
        }
        cfg->power_down_delay_s = s;
        return PROTO_OK;
    }
    default: { /* PROTO_KEY_WIRED_PORT_LOCK (SPEC §15.2) */
        uint8_t level = in->value.wired_port_lock.level;
        if (level > CFG_WIRED_PORT_LOCK_MAX) {
            return PROTO_ERR_BAD_VALUE;
        }
        if (level == CFG_WIRED_PORT_LOCK_MAX && !(board && board->radio_port_isolation)) {
            return PROTO_ERR_OUT_OF_RANGE; /* no disconnect state on the radio port switch */
        }
        cfg->wired_port_lock = level;
        return PROTO_OK;
    }
    }
}

void cfg_to_ptt(const cfg_t *cfg, const board_t *board, ptt_config_t *out)
{
    (void)board;
    memset(out, 0, sizeof(*out));
    out->keepalive_ms = cfg->ptt_keepalive_ms;
    out->max_tx_s = cfg->max_tx_s;
    out->targets = cfg->ptt_targets;
    out->usb_port = cfg->ptt_usb_port;
    memcpy(out->rts_action, cfg->rts_action, sizeof(out->rts_action));
    memcpy(out->dtr_action, cfg->dtr_action, sizeof(out->dtr_action));
    /* WIRED_PORT_LOCK 1 or higher: native RTS/DTR never key PTT (SPEC §15.2). */
    out->native_locked = cfg->wired_port_lock >= 1;
    ptt_config_sanitize(out);
}

/* Every (key, selector) that exists, in storage order. */
static size_t records(uint8_t keys[], uint8_t selectors[], size_t cap)
{
    size_t n = 0;
    for (uint8_t key = PROTO_KEY_FIRST; key <= PROTO_KEY_LAST; key++) {
        if (key == PROTO_KEY_LINE_MAP) {
            static const uint8_t ports[] = {0, 1, 2, 3, 4, PTT_PORT_CONTROL};
            for (size_t i = 0; i < sizeof(ports) && n < cap; i++) {
                keys[n] = key;
                selectors[n++] = ports[i];
            }
        } else if (key == PROTO_KEY_SERIAL_DEFAULT) {
            for (uint8_t s = 0; s < CFG_SERIAL_PORTS && n < cap; s++) {
                keys[n] = key;
                selectors[n++] = s;
            }
        } else if (n < cap) {
            keys[n] = key;
            selectors[n++] = 0;
        }
    }
    return n;
}

size_t cfg_serialize(const cfg_t *cfg, uint8_t *out, size_t cap)
{
    uint8_t keys[32], sels[32];
    size_t n = records(keys, sels, sizeof(keys));
    size_t pos = 4;
    if (cap < 6) {
        return 0;
    }
    out[0] = BLOB_MAGIC0;
    out[1] = BLOB_MAGIC1;
    out[2] = BLOB_VERSION;
    out[3] = (uint8_t)n;
    for (size_t i = 0; i < n; i++) {
        proto_config_t c;
        (void)cfg_get(cfg, keys[i], sels[i], &c);
        if (proto_config_encode(&c, out, cap - 2, &pos) != PROTO_OK) {
            return 0;
        }
    }
    uint16_t crc = proto_crc16(out, pos);
    out[pos++] = (uint8_t)(crc & 0xFFu);
    out[pos++] = (uint8_t)(crc >> 8);
    return pos;
}

int cfg_deserialize(cfg_t *cfg, const board_t *board, const uint8_t *in, size_t len)
{
    cfg_defaults(cfg, board);
    if (len < 6 || in[0] != BLOB_MAGIC0 || in[1] != BLOB_MAGIC1 || in[2] != BLOB_VERSION) {
        return -1;
    }
    uint16_t crc = (uint16_t)(in[len - 2] | (in[len - 1] << 8));
    if (proto_crc16(in, len - 2) != crc) {
        return -1;
    }
    size_t pos = 4, end = len - 2;
    int rejected = 0;
    for (unsigned i = 0; i < in[3]; i++) {
        if (end - pos < 2) {
            return -1;
        }
        int vlen = proto_config_value_len(in[pos]);
        if (vlen < 0 || end - pos < 2u + (size_t)vlen) {
            /* Unknown key (newer firmware wrote it) or truncated: stop here. */
            rejected++;
            break;
        }
        proto_config_t c;
        if (proto_config_decode(in + pos, 2u + (size_t)vlen, &c) != PROTO_OK ||
            cfg_set(cfg, board, &c) != PROTO_OK) {
            rejected++;
        }
        pos += 2u + (size_t)vlen;
    }
    return rejected;
}

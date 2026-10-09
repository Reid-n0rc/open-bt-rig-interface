/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * BLE pairing window (SPEC §13.5). See pairing.h.
 */
#include "pairing.h"

void pairing_init(pairing_t *pw)
{
    pw->open = false;
    pw->close_at_ms = 0;
}

void pairing_open_local(pairing_t *pw, uint16_t window_s, uint64_t now_ms)
{
    pw->open = true;
    pw->close_at_ms = now_ms + (uint64_t)window_s * 1000u;
}

void pairing_bond_added(pairing_t *pw)
{
    pw->open = false;
}

void pairing_close(pairing_t *pw)
{
    pw->open = false;
}

void pairing_tick(pairing_t *pw, uint64_t now_ms)
{
    if (pw->open && now_ms >= pw->close_at_ms) {
        pw->open = false;
    }
}

bool pairing_is_open(const pairing_t *pw)
{
    return pw->open;
}

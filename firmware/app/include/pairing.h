/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * BLE pairing window (SPEC §13.5). New bonds are accepted only while it is
 * open. Only a local action opens it (power-on or a pairing button); no
 * protocol message can, over any transport. It closes after
 * PAIRING_WINDOW_S, after the first new bond or wired-host approval, or
 * when the host mode changes. The same window approves wired hosts (§15.2).
 */
#ifndef PAIRING_H
#define PAIRING_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool open;
    uint64_t close_at_ms;
} pairing_t;

void pairing_init(pairing_t *pw);
/* A local action (power-on or the pairing button). Not reachable from the
 * protocol. Opening again restarts the window. */
void pairing_open_local(pairing_t *pw, uint16_t window_s, uint64_t now_ms);
/* A new bond or wired-host approval was stored: the window closes. */
void pairing_bond_added(pairing_t *pw);
/* The host mode changed: the window closes. */
void pairing_close(pairing_t *pw);
void pairing_tick(pairing_t *pw, uint64_t now_ms);
/* True while new bonds may be accepted. */
bool pairing_is_open(const pairing_t *pw);

#ifdef __cplusplus
}
#endif

#endif /* PAIRING_H */

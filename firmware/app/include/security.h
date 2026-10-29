/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Hooks for the security design (Cyber Resilience Act level), implemented in
 * #64: wired host approval (granted by the pairing-window local action), the
 * BLE bond store, and secure (encrypted) storage. #14 only defines the
 * interfaces; every hook here refuses until #64 fills it in, so nothing is
 * approved by accident.
 */
#ifndef SECURITY_H
#define SECURITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* An approved wired host (for example a hash of what identifies it). */
#define SEC_HOST_ID_LEN 16u
typedef struct {
    uint8_t id[SEC_HOST_ID_LEN];
} sec_host_id_t;

/* True if a wired host was approved by a local action. Stub: false. */
bool sec_wired_host_approved(const sec_host_id_t *host);
/* Records a wired host approval (only while the pairing window is open).
 * Returns a PROTO code; stub: PROTO_ERR_UNSUPPORTED. */
int sec_wired_host_approve(const sec_host_id_t *host, bool pairing_window_open);
/* Number of stored BLE bonds (the platform's bond store). Stub: 0. */
unsigned sec_bond_count(void);
/* Factory reset: erase bonds, approvals and secrets. Returns a PROTO code;
 * stub: PROTO_ERR_UNSUPPORTED. */
int sec_factory_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SECURITY_H */

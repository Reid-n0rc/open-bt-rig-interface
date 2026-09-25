/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Controllable security hooks for test_app, so the app's AUTH handling can
 * be tested before the real stores exist (#64). It replaces
 * firmware/app/src/security.c in that test only (the linker then doesn't
 * pull security.o from the library); test_modules tests the real stubs.
 */
#include "fake_security.h"

#include "proto.h"

fake_security_t fake_sec;

bool sec_wired_host_approved(const sec_host_id_t *host)
{
    (void)host;
    return fake_sec.approved;
}

int sec_wired_host_approve(const sec_host_id_t *host, bool pairing_window_open)
{
    (void)host;
    fake_sec.approve_calls++;
    if (!pairing_window_open) {
        return PROTO_ERR_STATE;
    }
    return fake_sec.approve_result;
}

unsigned sec_bond_count(void)
{
    return 0;
}

int sec_factory_reset(void)
{
    return fake_sec.factory_reset_result;
}

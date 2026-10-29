/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Security hooks: stubs until #64. See security.h. They fail closed.
 */
#include "security.h"

#include "hal.h"
#include "proto.h"

bool sec_wired_host_approved(const sec_host_id_t *host)
{
    (void)host;
    return false;
}

int sec_wired_host_approve(const sec_host_id_t *host, bool pairing_window_open)
{
    (void)host;
    (void)pairing_window_open;
    return PROTO_ERR_UNSUPPORTED;
}

unsigned sec_bond_count(void)
{
    return 0;
}

int sec_factory_reset(void)
{
    return hal_secure_erase_all() == 0 ? PROTO_OK : PROTO_ERR_UNSUPPORTED;
}

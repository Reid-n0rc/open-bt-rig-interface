/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#ifndef FAKE_SECURITY_H
#define FAKE_SECURITY_H

#include "security.h"

typedef struct {
    bool approved;            /* sec_wired_host_approved() */
    int approve_result;       /* sec_wired_host_approve() inside the window */
    unsigned approve_calls;
    int factory_reset_result; /* sec_factory_reset() */
} fake_security_t;

extern fake_security_t fake_sec;

#endif /* FAKE_SECURITY_H */

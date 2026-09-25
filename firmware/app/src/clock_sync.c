/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Clock sync (SPEC §10). See clock_sync.h.
 */
#include "clock_sync.h"

#include <string.h>

void clock_sync_init(clock_sync_t *c)
{
    memset(c, 0, sizeof(*c));
}

void clock_sync_resp(const proto_time_req_t *req, uint64_t t2, uint64_t t3,
                     proto_time_resp_t *resp)
{
    resp->host_t1 = req->host_t1;
    resp->device_t2 = t2;
    resp->device_t3 = t3;
}

int clock_sync_set(clock_sync_t *c, const proto_time_set_t *set, uint64_t now_us)
{
    if (set->device_time_us > now_us) {
        return PROTO_ERR_BAD_VALUE;
    }
    c->mapped = true;
    c->device_time_us = set->device_time_us;
    c->utc_us = set->utc_us;
    c->uncertainty_us = set->uncertainty_us;
    c->set_at_us = now_us;
    return PROTO_OK;
}

bool clock_sync_fresh(const clock_sync_t *c, uint64_t now_us)
{
    return c->mapped && now_us - c->set_at_us < CLOCK_FRESH_US;
}

bool clock_sync_utc_to_device(const clock_sync_t *c, uint64_t utc_us, uint64_t now_us,
                              uint64_t *device_us)
{
    if (!clock_sync_fresh(c, now_us) || utc_us < c->utc_us) {
        return false;
    }
    *device_us = c->device_time_us + (utc_us - c->utc_us);
    return true;
}

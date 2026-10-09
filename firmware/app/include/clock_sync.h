/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Clock sync (SPEC §10). Device time is microseconds since boot and is never
 * set by the host; TIME_SET only records a device-time <-> UTC mapping,
 * which survives sessions and is stale after 10 minutes (SPEC §10.3, verify).
 * Scheduled audio (#16) and tone sequences (#17) use it.
 */
#ifndef CLOCK_SYNC_H
#define CLOCK_SYNC_H

#include <stdbool.h>
#include <stdint.h>

#include "proto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLOCK_FRESH_US (10ull * 60ull * 1000000ull)

typedef struct {
    bool mapped;
    uint64_t device_time_us; /* device time of the mapping point */
    uint64_t utc_us;         /* UTC at that point */
    uint32_t uncertainty_us;
    uint64_t set_at_us; /* device time the mapping arrived */
} clock_sync_t;

void clock_sync_init(clock_sync_t *c);
/* TIME_REQ received at device time t2; t3 is the device time just before the
 * reply is sent (SPEC §10.1). */
void clock_sync_resp(const proto_time_req_t *req, uint64_t t2, uint64_t t3,
                     proto_time_resp_t *resp);
/* TIME_SET. Returns PROTO_OK, or PROTO_ERR_BAD_VALUE for a mapping point in
 * the future. */
int clock_sync_set(clock_sync_t *c, const proto_time_set_t *set, uint64_t now_us);
/* STATUS.flags bit 3. */
bool clock_sync_fresh(const clock_sync_t *c, uint64_t now_us);
/* UTC -> device time. Returns false without a fresh mapping (STATE_ERROR). */
bool clock_sync_utc_to_device(const clock_sync_t *c, uint64_t utc_us, uint64_t now_us,
                              uint64_t *device_us);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_SYNC_H */

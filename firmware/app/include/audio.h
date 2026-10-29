/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Audio pipeline hooks (SPEC §9). The pipeline itself (jitter buffer, rate
 * matching, codec, radio USB sound card) is #16 and #43. Until then the
 * device doesn't report AUDIO_RX/AUDIO_TX, and every audio message gets
 * UNSUPPORTED (SPEC §5.2).
 */
#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "proto.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool running;
    uint8_t direction; /* 1 RX, 2 TX */
} audio_t;

void audio_init(audio_t *a);
/* AUDIO_START. Fills *st for the reply. Returns a PROTO code. */
int audio_start(audio_t *a, const board_t *board, const proto_audio_start_t *req,
                proto_audio_status_t *st);
/* AUDIO_STOP. */
int audio_stop(audio_t *a, const board_t *board, uint8_t direction, proto_audio_status_t *st);
/* AUDIO_FRAME from the host (TX). */
int audio_frame(audio_t *a, const board_t *board, const proto_audio_frame_t *f);
/* Session end (SPEC §8.7): stop audio. */
void audio_session_end(audio_t *a);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H */

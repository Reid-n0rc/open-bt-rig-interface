/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Controllable audio hooks for test_app, so the app's AUDIO_START success
 * path and the STATUS audio flag can be tested before the pipeline exists
 * (#16). It replaces firmware/app/src/audio.c in that test only (the linker
 * then doesn't pull audio.o from the library); test_modules tests the real
 * stubs.
 */
#include "fake_audio.h"

#include <string.h>

#include "proto.h"

fake_audio_t fake_aud = {.start_result = PROTO_ERR_UNSUPPORTED};

void audio_init(audio_t *a)
{
    memset(a, 0, sizeof(*a));
}

static void fill_status(const audio_t *a, uint8_t direction, proto_audio_status_t *st)
{
    memset(st, 0, sizeof(*st));
    st->direction = direction;
    st->state = (a->running && a->direction == direction) ? 2 : 0;
}

int audio_start(audio_t *a, const board_t *board, const proto_audio_start_t *req,
                proto_audio_status_t *st)
{
    (void)board;
    if (fake_aud.start_result == PROTO_OK) {
        a->running = true;
        a->direction = req->direction;
    }
    fill_status(a, req->direction, st);
    return fake_aud.start_result;
}

int audio_stop(audio_t *a, const board_t *board, uint8_t direction, proto_audio_status_t *st)
{
    (void)board;
    fill_status(a, direction, st);
    return PROTO_ERR_UNSUPPORTED;
}

int audio_frame(audio_t *a, const board_t *board, const proto_audio_frame_t *f)
{
    (void)a;
    (void)board;
    (void)f;
    return PROTO_ERR_UNSUPPORTED;
}

void audio_session_end(audio_t *a)
{
    a->running = false;
}

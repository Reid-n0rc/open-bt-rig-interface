/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Audio pipeline hooks: stubs until #16. See audio.h.
 */
#include "audio.h"

#include <string.h>

#include "hal.h"

#define AUDIO_FEATURES (PROTO_F_AUDIO_RX | PROTO_F_AUDIO_TX)

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
    fill_status(a, req->direction, st);
    if (!(board->features & AUDIO_FEATURES)) {
        return PROTO_ERR_UNSUPPORTED;
    }
    /* #16: check the format against the AUDIO TLV, then hal_audio_open(). */
    return PROTO_ERR_UNSUPPORTED;
}

int audio_stop(audio_t *a, const board_t *board, uint8_t direction, proto_audio_status_t *st)
{
    fill_status(a, direction, st);
    if (!(board->features & AUDIO_FEATURES)) {
        return PROTO_ERR_UNSUPPORTED;
    }
    return PROTO_ERR_UNSUPPORTED;
}

int audio_frame(audio_t *a, const board_t *board, const proto_audio_frame_t *f)
{
    (void)a;
    (void)f;
    if (!(board->features & AUDIO_FEATURES)) {
        return PROTO_ERR_UNSUPPORTED;
    }
    return PROTO_ERR_UNSUPPORTED;
}

void audio_session_end(audio_t *a)
{
    if (a->running) {
        hal_audio_close();
    }
    a->running = false;
}

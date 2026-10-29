/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#ifndef FAKE_AUDIO_H
#define FAKE_AUDIO_H

#include "audio.h"

typedef struct {
    int start_result; /* audio_start() on a board with audio */
} fake_audio_t;

extern fake_audio_t fake_aud;

#endif /* FAKE_AUDIO_H */

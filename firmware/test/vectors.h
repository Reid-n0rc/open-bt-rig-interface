/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * The golden vectors as C tables. vectors_gen.c is generated at build time
 * by gen_vectors.py from the JSON files in protocol/vectors/.
 */
#ifndef VECTORS_H
#define VECTORS_H

#include <stddef.h>
#include <stdint.h>

#include "proto.h"

typedef struct {
    const uint8_t *p;
    size_t n;
} vbytes_t;

/* messages.json */
typedef struct {
    const char *name;
    uint8_t type;
    uint8_t token;
    vbytes_t payload, frame, wire;
    /* Sets every field of the message (TLVs are built in buf). */
    void (*fill)(proto_msg_t *m, uint8_t *buf, size_t cap);
    /* Asserts every field of a decoded message. */
    void (*check)(const proto_msg_t *m);
} vec_msg_t;

/* framing.json */
typedef struct {
    vbytes_t input;
    uint16_t crc;
} vec_crc_t;

typedef struct {
    vbytes_t decoded, encoded;
} vec_cobs_t;

#define VEC_FRAME_ERROR (-1) /* dropped by the receiver (COBS, length or CRC) */
typedef struct {
    const char *name;
    vbytes_t wire;
    int expect; /* VEC_FRAME_ERROR or the PROTO_ERR_* the device answers */
} vec_invalid_t;

typedef struct {
    vbytes_t wire;
    const uint8_t *types; /* message types in order */
    size_t ntypes;
    unsigned errors;
} vec_stream_t;

/* gatt.json */
typedef struct {
    vbytes_t value;
    proto_info_t fields;
} vec_info_t;

typedef struct {
    uint16_t att_mtu;
    vbytes_t wire;
    const vbytes_t *chunks;
    size_t nchunks;
} vec_chunking_t;

extern const char vec_protocol_version[];
extern const vec_msg_t vec_messages[];
extern const size_t vec_messages_count;
extern const vec_crc_t vec_crc[];
extern const size_t vec_crc_count;
extern const vec_cobs_t vec_cobs[];
extern const size_t vec_cobs_count;
extern const vec_invalid_t vec_invalid[];
extern const size_t vec_invalid_count;
extern const vec_stream_t vec_stream;
extern const vec_info_t vec_info[];
extern const size_t vec_info_count;
extern const vec_chunking_t vec_chunking;

#endif /* VECTORS_H */

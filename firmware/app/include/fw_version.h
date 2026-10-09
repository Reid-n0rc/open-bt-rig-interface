/*
 * SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * Firmware version, embedded at build time from the `fw-v<semver>` git tags
 * by firmware/cmake/fw_version.cmake (reported in DEVICE_INFO, SPEC §4.2).
 */
#ifndef FW_VERSION_H
#define FW_VERSION_H

#ifndef FW_VERSION_MAJOR
#define FW_VERSION_MAJOR 0
#endif
#ifndef FW_VERSION_MINOR
#define FW_VERSION_MINOR 0
#endif
#ifndef FW_VERSION_PATCH
#define FW_VERSION_PATCH 0
#endif
/* Free text for DEVICE_INFO.build, at most 32 bytes, e.g. "0.1.0+3.g1a2b3c4". */
#ifndef FW_VERSION_BUILD
#define FW_VERSION_BUILD "0.0.0+unknown"
#endif

#endif /* FW_VERSION_H */

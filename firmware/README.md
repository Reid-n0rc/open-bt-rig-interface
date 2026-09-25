<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# firmware

Device firmware. License: PolyForm-Noncommercial-1.0.0 (non-commercial; see
[`COMMERCIAL.md`](../COMMERCIAL.md)). Third-party components keep their own
licenses ([`THIRD_PARTY.md`](../THIRD_PARTY.md)).

| Folder | What |
|---|---|
| [`app/`](app/) | Portable core in C11, no SDK includes: the protocol codec, the PTT controller, configuration, sessions and dispatch, and interfaces for what later issues fill in |
| [`test/`](test/) | Host build (CMake + Unity) of `app/` against a fake HAL, with the tests |
| [`platform/esp-idf/`](platform/esp-idf/) | ESP32-S3 (ESP-IDF v6.0.3): the HAL, NimBLE, the bootloader hook, the dev-kit pin map |
| [`cmake/`](cmake/) | The firmware version from `fw-v*` tags, shared by both builds |

The design follows [`protocol/SPEC.md`](../protocol/SPEC.md) (ADR-0007) and
[ADR-0008](../docs/decisions/ADR-0008-host-links-esp32-s3.md): Bluetooth LE
and wired USB-C, no Bluetooth Classic.

## Build and test

```sh
# Host tests (any OS with CMake >= 3.16, a C11 compiler, Python 3 and git)
cmake -S firmware/test -B build/fw-test
cmake --build build/fw-test
ctest --test-dir build/fw-test --output-on-failure

# Target (ESP-IDF v6.0.3 exported, or its Docker image)
cd firmware/platform/esp-idf && idf.py set-target esp32s3 && idf.py build
idf.py -p <UART port> flash monitor
```

Details: [`test/README.md`](test/README.md) and
[`platform/esp-idf/README.md`](platform/esp-idf/README.md). CI runs both as
the `Firmware host tests` and `Firmware target build (ESP-IDF)` jobs.

## The PTT fail-safes

[`app/src/ptt.c`](app/src/ptt.c) implements SPEC §8 (approved by the
maintainer on 2026-09-24, amended 2026-09-25): the sources and their bits,
the keepalive, per-line RTS/DTR arming (BLOCKED, ARMED, KEYING; both lines
rising together is a port open), the max-TX timer with lockout (default
300 s, no upper bound, 0 disables it), every PTT-off event with its reason
code, and configuration changes that never assert PTT. Every path has a host
test in [`test/test_ptt.c`](test/test_ptt.c). Changes to it need the
maintainer's explicit approval ([`GOVERNANCE.md`](../GOVERNANCE.md)).

Interpretations the SPEC leaves open, chosen as the safer reading:

- **Pass-through lines don't drive the PTT targets.** An asserted
  pass-through line (§8.3) counts as a PTT source for the keepalive, max TX,
  session end and `PTT_STATUS`, but it reaches the radio only through its own
  line, never through the AUDIO-jack closure. Otherwise a program opening a
  radio's USB-serial port (DTR and RTS rise) would key the closure.
- **A keepalive timeout blocks the protocol's lines** (not the native wired
  lines, which need no keepalive), and holds pass-through lines low until the
  host's next `MODEM_LINES`.
- **Both native lines dropping together** on a wired CDC-ACM port is treated
  as a port close (reason `PORT_CLOSED`, lines BLOCKED).
- `NOT_ARMED` is reported for a rise on a BLOCKED line and for both lines
  rising together; `LOCKED_OUT` for any key attempt during the lockout.
- A request that arrives on a transport without the session is ignored
  (except `HELLO`), and a device → host message type sent to the device gets
  `UNKNOWN_TYPE`.

## Versioning

The firmware version comes from git tags `fw-v<major>.<minor>.<patch>`
([`AGENTS.md`](../AGENTS.md#releases-and-tags)).
[`cmake/fw_version.cmake`](cmake/fw_version.cmake) runs
`git describe --tags --long --dirty --match "fw-v*"` at configure time and
passes the result to both builds:

- `FW_VERSION_MAJOR`, `_MINOR`, `_PATCH` go into `DEVICE_INFO.fw_*`;
- `FW_VERSION_BUILD` goes into `DEVICE_INFO.build` (at most 32 bytes), for
  example `0.1.0` on the tag itself, `0.1.0+5.g1a2b3c4` five commits later,
  `+….dirty` with local changes, and `0.0.0+g1a2b3c4` before the first tag;
- the ESP-IDF build also sets `PROJECT_VER`, so the image's app description
  carries the same string.

Set `FW_VERSION=x.y.z` (CMake cache or environment) to build without git.
Only the maintainer tags releases ([`GOVERNANCE.md`](../GOVERNANCE.md)); a
firmware release needs passing tests and a note of the hardware revisions it
supports. CI builds `fw-*` tag pushes like any other.

## Firmware updates: proposal (not implemented)

ADR-0007 deferred the OTA transport to #14; SPEC §15.4 reserves message
types 0xE0–0xEF and `FEATURES` bit 15, and says only signed images are
accepted. The security design is to the **Cyber Resilience Act level**
(#64; EU analysis in #59, PR #62, ADR-0009), so **signed images and secure
boot are required**, not optional. This is a proposal for #64 and a later
ADR; nothing here implements it.

**Image security (required):**

- **Secure Boot v2** on the ESP32-S3 (RSA-3072, RSA-PSS): the ROM verifies
  the bootloader and the bootloader verifies every app image on every boot
  and on each OTA update; an image that fails falls back to another valid
  one ([Secure Boot v2](../docs/references/index.md#esp-idf-secure-boot-v2)).
  "Signed app verification without hardware secure boot" exists but doesn't
  protect against someone who can write the flash, so it isn't enough here.
- **Flash encryption**, with the **UART ROM download mode set to Secure
  Download Mode** or disabled
  ([flash encryption](../docs/references/index.md#esp-idf-flash-encryption)).
- **Rollback and anti-rollback:** the app confirms itself after a self-test
  (`esp_ota_mark_app_valid_cancel_rollback`), otherwise the bootloader
  returns to the previous image; a security version in eFuse blocks
  downgrades to images with known vulnerabilities
  ([OTA](../docs/references/index.md#esp-idf-ota)).
- Two OTA app partitions (the -N8 has 8 MB of flash) plus `otadata`.
- Signing keys stay off the build machines (an offline or HSM-backed key,
  used only by the maintainer's release step); CI builds unsigned images for
  testing.

**Transports:**

- **Bluetooth LE:** a firmware-update extension in message types 0xE0–0xEF
  (a minor protocol version, `FEATURES` bit 15), available only to an
  authorized (bonded) host: start (size, version, image hash), chunks with
  offsets and credit flow control like `CAT_DATA`, finish (the device checks
  the signature before switching), and status. PTT is forced off and stays
  off for the whole update, and the session ends before the restart.
- **Wired:** USB DFU from the app (TinyUSB's DFU class), writing the same
  signed image into the OTA partition. The ROM's own USB DFU and serial
  download are **disabled by Secure Boot or flash encryption**
  ([DFU](../docs/references/index.md#esp-idf-dfu)), so they can't be the
  wired path in production. Or the same protocol extension over the USB
  network. Either way only an approved wired host (SPEC §15.2) may start it.

**Process items the CRA brings (#64):** a vulnerability-handling and
disclosure process, security updates for a stated support period, update
notes, and keeping the ESP-IDF release within Espressif's support period
(30 months per minor release,
[versions](../docs/references/index.md#esp-idf-versions)).

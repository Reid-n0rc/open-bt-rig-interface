<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# firmware/platform

One subfolder per SDK. Each implements the hardware-abstraction interface
([`../app/include/hal.h`](../app/include/hal.h)) used by `../app/`.

- [`esp-idf/`](esp-idf/): ESP32-S3-MINI-1 on ESP-IDF v6.0.3
  ([ADR-0008](../../docs/decisions/ADR-0008-host-links-esp32-s3.md)).

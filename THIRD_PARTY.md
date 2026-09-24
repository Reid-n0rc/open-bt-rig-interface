<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Third-party material and notices

This file is the **single ledger** of material from other projects that is
included in, adapted into, or bundled with this repository and its releases.
Any license that requires a copyright notice, license text or attribution is
satisfied from here.

The repository's own licensing is described in [`LICENSE`](LICENSE) and
[`REUSE.toml`](REUSE.toml). Third-party material **keeps its original license**
and is never relicensed under this project's licenses.

## When an entry is required

Add an entry, in the same PR, whenever you:

- copy or adapt source code, headers or build scripts (firmware, tools, CI);
- copy symbols, footprints or 3D models into `hardware/lib/` (including items
  taken from the KiCad libraries; see below);
- copy text, tables, figures or example circuits from datasheets, application
  notes, reference designs or other projects;
- add a firmware dependency whose code ends up in released binaries
  (SDK components, libraries), even when it isn't committed here;
- ship generated artifacts (fab outputs, STL/3MF, firmware images) that contain
  any of the above.

No entry is needed for facts learned from a source (a pinout, a voltage limit,
a protocol behavior) when nothing is copied. Cite such sources in the relevant
issue, ADR or doc instead.

## Rules

1. **Check the license in the source's actual LICENSE file** (and in file
   headers, which take precedence). Don't rely on a badge, an API summary or
   an earlier issue.
2. **Allowed to copy:** permissive licenses (MIT, BSD-2/3-Clause, ISC,
   Apache-2.0, Zlib, Unlicense/CC0) and CC-BY-4.0 / CC-BY-SA-4.0 for library
   or data files, with their obligations met.
3. **Never copy:** GPL, LGPL, AGPL, "no license", "all rights reserved", or
   terms limited to specific vendors' products (unless this project uses that
   vendor, and the restriction is recorded here). Such sources may be used for
   **facts only**. List them under [Facts only](#facts-only-never-copied).
4. **Keep notices intact:** leave original copyright/license headers in copied
   files, and add the license text to [`LICENSES/`](LICENSES/) under its SPDX
   ID.
5. **REUSE:** give third-party files their real SPDX copyright and license,
   either with file headers or with a path-specific `[[annotations]]` block in
   `REUSE.toml` that takes precedence over the project defaults.
   `uvx --from 'reuse[charset-normalizer]' reuse lint` must pass.
6. **Pin versions:** record the upstream commit or release tag, so the notice
   matches exactly what was used.
7. **Release notices:** every firmware release and every hardware release
   bundle must include a `THIRD_PARTY_NOTICES` file generated from the
   "Currently included" table (full license texts plus copyright lines).

## Entry format

| Field | Content |
|---|---|
| Name | Project or file name |
| Source | URL + pinned commit/tag |
| License | SPDX ID (verified from its LICENSE file) |
| Copyright | Exact copyright line(s) to reproduce |
| Used in | Paths in this repo, or "firmware binary" |
| Modifications | None / summary of changes |
| Obligations | For example "keep notice + license text in source and binary distributions" |
| Added in | PR / issue number |

## Currently included

None yet. The files in `LICENSES/` are license texts (taken from SPDX
license-list-data) and aren't third-party works that need an entry.

| Name | Source | License | Copyright | Used in | Modifications | Obligations | Added in |
|---|---|---|---|---|---|---|---|
| — | | | | | | | |

## Known candidates (not yet included)

Sources already identified for upcoming work. Each license below was read from
the project's LICENSE file on 2026-09-24. **Re-check it when you actually pull
the material in**, and move the row to "Currently included".

| Name | Source | License | Copyright (from LICENSE) | Likely use | Obligations if used |
|---|---|---|---|---|---|
| ft8_lib | https://github.com/kgoba/ft8_lib | MIT | Copyright (c) 2018 Kārlis Goba | Optional on-device FT8/FT4 tone synthesis (#17) | Keep notice + MIT text in source and binaries |
| ESP32_ft8_lib | https://github.com/guido57/ESP32_ft8_lib | MIT | Copyright (c) 2018 Kārlis Goba (as stated in its LICENSE) | Reference/port of ft8_lib to ESP32 (#17) | Same as ft8_lib; also credit the port's author if any of its code is used |
| ESP32_BleSerial | https://github.com/avinabmalla/ESP32_BleSerial | MIT | Copyright (c) 2022 Avinab Malla | BLE-to-UART bridge reference (#15) | Keep notice + MIT text |
| esp32-ble-uart-mx | https://github.com/olegv142/esp32-ble-uart-mx | Unlicense | Public domain dedication | BLE-to-UART reference (#15) | None required; credit as a courtesy |
| ESP-IDF | https://github.com/espressif/esp-idf | Apache-2.0 | Copyright (C) Espressif Systems | Firmware SDK if an Espressif module is selected (#7, #14) | Apache-2.0 text in releases; **its bundled components have other licenses** (Newlib/Picolibc BSD, FreeRTOS MIT, lwIP BSD, …) per its [COPYRIGHT](https://github.com/espressif/esp-idf/blob/master/docs/en/COPYRIGHT.rst) document; the firmware release notices must cover every component actually linked |
| ESP-ADF | https://github.com/espressif/esp-adf | "ESPRESSIF MIT License" (not SPDX MIT) | Copyright (c) 2018 Espressif Systems (Shanghai) | Audio framework, only if an Espressif module is selected | The grant covers **use on Espressif products only**; record that restriction here if it's used |
| KiCad libraries | https://gitlab.com/kicad/libraries | CC-BY-SA-4.0 + design exception | KiCad library contributors | Symbols/footprints/3D models | See the KiCad section below |

The other candidate firmware SDKs from the module decision (#7), such as
Infineon's or Microchip's, must be checked against rules 1–3 before selection.
Their license terms are part of the decision.

## KiCad library material

The KiCad libraries are CC-BY-SA-4.0 with an exception: designs that *use* the
library data (schematics, boards and generated fab files) are **not** bound by
it, and need no attribution
([license](https://gitlab.com/kicad/libraries/kicad-symbols/-/blob/master/LICENSE.md)).

However, **copying library items into `hardware/lib/`** redistributes part of
the library, so those copied files stay **CC-BY-SA-4.0**. Therefore:

- keep their attribution;
- give them a `REUSE.toml` annotation with `CC-BY-SA-4.0` and add
  `LICENSES/CC-BY-SA-4.0.txt`;
- list each copied item (or library file) under "Currently included".

Prefer referencing the stock libraries, and only copy an item when it has to
be modified or pinned. Vendor models and footprints from third-party
download sites often carry their own terms. Check them the same way, and
don't commit any that forbid redistribution.

## Facts only (never copied)

Sources consulted for facts, whose licenses don't allow copying into this
repository. Licenses were verified from each LICENSE file on 2026-09-24.

| Name | Source | License | Why listed |
|---|---|---|---|
| Digirig hardware | https://github.com/softcomplex/digirig | GPL-3.0 | Wired-interface reference; no schematics or layout copied |
| Mobilinkd TNC3 firmware | https://github.com/mobilinkd/tnc3-firmware | GPL-3.0 | BLE + audio interface reference |
| arduino-audio-tools | https://github.com/pschatzmann/arduino-audio-tools | GPL-3.0 | Microcontroller audio streaming reference |
| Linux kernel USB network drivers (`drivers/net/usb/usbnet.c`, `cdc_ncm.c`, commit 038d61fd6422) | https://github.com/torvalds/linux | GPL-2.0 | CDC-NCM host behavior and interface naming (`usbN` / `ethN`) for the protocol's USB network transport (#13); nothing copied |

Add more rows as sources are consulted. Record unlicensed sources here too,
marked "no license".

## Standards and datasheets

Standards (for example ISO 16750-2, ISO 7637-2, CISPR 25), datasheets,
application notes and module integration guides are cited for their
requirements and values in the relevant docs and ADRs. Quoting or reproducing
figures, tables or reference circuits from them requires an entry here and
must comply with the publisher's terms. Many don't allow redistribution, so
summarize and cite instead.

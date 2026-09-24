<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Radio module confirmation: ESP32-S3-MINI-1

Issue: [#7](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/7).
Researched 2026-09-24. Checks the module chosen in
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md) (proposed): sourcing,
lifecycle, the FCC grant, and the fallback module. The dual-mode candidate study
is already recorded in [`core-devices.md`](core-devices.md) §1.2 and isn't
repeated here. Integration rules and the host labeling text are in
[`../compliance/fcc.md`](../compliance/fcc.md).

## Result

**The findings support ADR-0008.** No finding overturns it, so there is no
ADR-0001. Moving ADR-0008 to *accepted* is the maintainer's call.

Findings that constrain the design:

1. **The grant certifies Bluetooth LE at 0.0107 W (10.3 dBm) conducted**, while
   the datasheet allows up to +20 dBm. The firmware must cap the BLE TX power at
   the certified level (§3.2).
2. **The grant is for mobile use only** (at least 20 cm from people). A portable
   or body-worn use needs a separate approval (§3.2).
3. **Only the -N8 ordering code is named in the FCC exhibit.** Grant coverage of
   -N4R2 is **(verify)** (§3.3). Revision A should use **-N8**.
4. **The second-distributor check isn't done.** Digi-Key and Mouser block
   scripted lookups, so only LCSC figures are recorded. The two-distributor rule
   in [`AGENTS.md`](../../AGENTS.md#parts-and-sourcing) is still open (§2).

## 1. Module

| Item | ESP32-S3-MINI-1-N8 (primary) | Source |
|---|---|---|
| SoC | ESP32-S3FN8 (8 MB in-package flash); -N4R2 uses ESP32-S3FH4R2 (4 MB flash, 2 MB PSRAM) | [Espressif modules page](../references/index.md#espressif-s3-modules) |
| Antenna | On-board PCB antenna (-1U: connector for an external antenna) | [Datasheet v1.7](../references/index.md#esp32s3-mini1-ds) §1.2 |
| Size | 15.4 × 20.5 × 2.4 mm | Datasheet §1.2 |
| Temperature | −40 to +85 °C, all ordering codes | Datasheet §1.2 |
| BLE TX power range | −24.0 to +20.0 dBm (software set) | Datasheet Table 7-7 |
| BLE TX peak current | 340 mA at +20 dBm, 204 mA at +9 dBm, 189 mA at 0 dBm (3.3 V, 25 °C) | Datasheet Table 6-5 |
| FCC ID / ISED | 2AC7Z-ESPS3MINI1 / 21098-ESPS3MINI1 | §3 |

On -N4R2, IO26 connects to the in-package PSRAM and isn't free for other uses
(datasheet Table 3-1, note b).

## 2. Sourcing

Checked 2026-09-24, USD, unit price. "1000" is the price break that covers
1000 pieces (LCSC: the 650+ break).

| Part | Distributor | 1 | 100 | 1000 | Stock | Lead time | Source |
|---|---|---|---|---|---|---|---|
| ESP32-S3-MINI-1-N8 | LCSC (C2913206) | 4.7577 | 3.3801 | 3.2403 | 7,313 | not shown | [LCSC](../references/index.md#lcsc-esp32s3-mini1-n8) |
| ESP32-S3-MINI-1-N8 | Digi-Key | | | | | | not readable (bot protection) |
| ESP32-S3-MINI-1-N8 | Mouser | | | | | | not readable (bot protection) |
| ESP32-S3-MINI-1-N4R2 | LCSC (C3013941) | 4.9938 | 3.6223 | 3.4533 | 199 | not shown | [LCSC](../references/index.md#lcsc-esp32s3-mini1-n4r2) |
| ESP32-S3-MINI-1-N4R2 | Digi-Key | | | | | | not readable (bot protection) |
| ESP32-S3-MINI-1-N4R2 | Mouser | | | | | | not readable (bot protection) |
| Raytac MDBT50Q-P1MV2 | LCSC (C5119772) | 7.521 | 7.3309 (10+) | | 0 (out of stock) | not shown | [LCSC](../references/index.md#lcsc-mdbt50q-p1mv2), reference prices only |
| Raytac MDBT50Q | Digi-Key | | | | | | not readable (bot protection) |
| Raytac MDBT50Q | Mouser | | | | | | not readable (bot protection) |

- **Digi-Key and Mouser** product and search pages returned HTTP 403 or a
  bot challenge to every tool available (2026-09-24), so those cells are
  blank. They need a check in a browser, or through the distributors' APIs.
- Espressif's ESP32-S3 modules page lists Digi-Key and Mouser as buying
  channels for -N8 and Digi-Key for -N4R2
  ([Espressif](../references/index.md#espressif-s3-modules)). That shows the
  channels exist. It doesn't show stock.
- The LCSC N8 price at qty 100 moved from $3.34 (ADR-0008, same day, earlier
  lookup) to $3.3801; stock is unchanged at 7,313.
- -N4R2 stock at LCSC is thin (199), another reason to use -N8.

### Lifecycle

- **ESP32-S3 series:** Espressif commits to supply for **at least 12 years
  from 2021-01-01** ([longevity commitment](../references/index.md#espressif-longevity),
  last updated 2025-05-20). Per the same page, an NRND status doesn't change
  that commitment.
- **Module status:** datasheet v1.7 (2026-03-02) carries no NRND or EOL
  watermark. Espressif adds one when every variant it covers is NRND or EOL
  (datasheet, "Datasheet Versioning"). The modules page shows no EOL mark on
  either MINI-1 variant, while it does mark some other ESP32-S3 variants EOL.
- **Distributor lifecycle fields** (Digi-Key "Product Status", Mouser
  "Lifecycle") are unread, for the reason above.
- **MDBT50Q:** Raytac lifecycle status **unknown** (not checked).

## 3. FCC grant review

### 3.1 2AC7Z-ESPS3MINI1 (ESP32-S3-MINI-1)

From the grant as published by Espressif
([grant](../references/index.md#esp32s3-mini1-fcc-grant)). The FCC EAS and
the fcc.report / fccid.io mirrors block scripted access, so the filing list is
recorded as a manual-fetch reference
([filings](../references/index.md#fcc-2ac7z-esps3mini1-filings)).

| Field | Value |
|---|---|
| Grantee | Espressif Systems (Shanghai) Co., Ltd |
| Certified by | Nemko North America (TCB) |
| Grant date | 2022-02-28 (application 2022-02-27) |
| Equipment class | Digital Transmission System (DTS) |
| Description | 2.4GHz Wi-Fi & BT IoT Module |
| Rule parts | 15C; the module manual names 15.247 and 15.209 |
| Frequency, output power (conducted) | 2402.0–2480.0 MHz: 0.0107 W (Bluetooth LE); 2412.0–2462.0 MHz: 0.3365 W (Wi-Fi) |
| Modular type | **Single modular** |
| Antenna | On-board PCB antenna, 3.96 dBi (module manual) **(verify against the FCC exhibit)** |

Grant notes, in summary:

- Single modular approval; the listed power is conducted; 20 MHz and 40 MHz
  Wi-Fi bandwidth modes.
- The integrator must make sure the final product complies with the FCC rules,
  by technical assessment or evaluation including the transmitter's
  operation, following [KDB 996369](../references/index.md#fcc-kdb-996369).
- **Separate approval is required for all other operating configurations,
  including portable configurations (47 CFR 2.1093) and different antenna
  configurations.**

The FCC user-manual exhibit (v0.6, 2022-02-24) was read from a mirror
([manual](../references/index.md#esp32s3-mini1-fcc-manual)), because the FCC copy
couldn't be opened. It says:

- the module is limited to Part 15 Subpart C 15.247 and 15.209;
- the host label must say "Contains FCC ID: 2AC7Z-ESPS3MINI1";
- the antenna must stay at least 20 cm from the user's body, and the module may
  not be co-located with another transmitter;
- the host maker must test the final product for any other requirements
  (Part 15 Subpart B) with the module installed.

The 2022 grant is the only FCC document read. Later permissive changes on
the FCC side are **unknown**, since the EAS couldn't be queried.

### 3.2 What the grant means for the design

- **BLE TX power cap.** The grant (and the ISED certificate) lists BLE at
  0.0107 W conducted, which is 10.3 dBm. The datasheet allows +20 dBm. Setting
  a power above the certified one is outside the grant, so **the firmware caps
  BLE TX power at or below 10.3 dBm** (the nearest ESP-IDF setting at or below
  it, e.g. +9 dBm **(verify the ESP-IDF levels)**). This also lowers the BLE TX
  peak from 340 mA (+20 dBm) to about 204 mA (+9 dBm, datasheet Table 6-5).
  The 340 mA figure in [`constraints.md`](../requirements/constraints.md) §3.4
  stays as the conservative budget.
- **Mobile use only.** The 20 cm separation fits a device on a desk, shelf or
  vehicle mount next to the radio. The user manual must state it. A handheld
  or body-worn version would need its own RF exposure evaluation and approval.
- **No co-located transmitters.** The device must not add another radio that
  transmits at the same time. Wi-Fi stays off (it isn't used, per
  [`constraints.md`](../requirements/constraints.md) §2).
- **Antenna.** Only the on-board PCB antenna (-1 ordering codes) is covered
  without further approval. A -1U with an external antenna counts as a different
  antenna configuration under the grant note. Datasheet v1.7 §10.2 names a
  2.33 dBi certification antenna for -1U while the FCC manual lists 4.54 dBi
  for the connector variant. That conflict doesn't matter while revision A uses
  -1, but **-1U needs its own check** before use.

### 3.3 Which ordering codes the grant covers

- The grant names no model numbers. The ISED certificate names HVIN
  "ESP32-S3-MINI-1".
- The FCC user manual v0.6 lists **ESP32-S3-MINI-1-N8** and
  ESP32-S3-MINI-1U-N8 only.
- -N4R2 uses a different SoC (ESP32-S3FH4R2, with PSRAM). Whether it is covered
  by 2AC7Z-ESPS3MINI1 is **(verify)**: read the FCC ID on a -N4R2 module's
  shield, or ask Espressif. Until then revision A uses **-N8**.

### 3.4 ISED (for the record)

[ISED certificate](../references/index.md#esp32s3-mini1-ised-cert):
**21098-ESPS3MINI1**, HVIN/PMN ESP32-S3-MINI-1, class I permissive change
approved 2024-08-16 (Sporton, FCB), RSS-247 issue 3. Same powers as the FCC
grant (BLE 2402–2480 MHz, 0.0107 W, emission 2M01F1D), PCB antenna, single
modular approval, at least 20 cm from all persons, and co-location evaluated
under the ISED multi-transmitter procedures. ISED and CE are optional and
later ([`constraints.md`](../requirements/constraints.md) §4).

### 3.5 SH6MDBT50Q (Raytac MDBT50Q, fallback)

From the grant reproduced in the Raytac approval sheet, version L, §9.2
([datasheet](../references/index.md#raytac-mdbt50q-ds)):

| Field | Value |
|---|---|
| Certified by | Telefication B.V. (TCB) |
| Grant date | 2018-07-26 |
| Equipment class | Digital Transmission System |
| Modular type | Single modular |
| Frequency, output power (conducted) | 2402.0–2480.0 MHz: 0.0066 W; 2405.0–2480.0 MHz: 0.0066 W |
| Grant note | Modular approval; **portable device**; installed only by OEM integrators, who must pass on the RF exposure operating conditions |
| Host label (approval sheet §9.11.1) | "Contain FCC ID: SH6MDBT50Q" |
| ISED | 8017A-MDBT50Q |

Unlike the ESP32-S3-MINI-1, the MDBT50Q grant covers portable use.

## 4. Risks

| Risk | Effect | Mitigation |
|---|---|---|
| Single vendor (Espressif) | A supply problem stops the build | 12-year longevity commitment; MDBT50Q fallback, at the cost of a USB host controller and a USB-audio host driver (ADR-0008) |
| Second distributor unchecked | AGENTS.md sourcing rule not yet met | Check Digi-Key and Mouser in a browser before the BOM is frozen |
| -N4R2 grant coverage unconfirmed | PSRAM variant may not carry the FCC ID | Use -N8; confirm before any switch |
| BLE TX power above the grant | Product outside its FCC authorization | Firmware cap at 10.3 dBm or lower, with a test (§3.2) |
| 20 cm mobile-only grant | Handheld or body-worn use not covered | State it in the user manual; keep the device off-body |
| Antenna detuned by board or enclosure | Short range, BLE audio dropouts | Board-edge placement and keep-out, 15 mm enclosure clearance, range test in the final enclosure ([`fcc.md`](../compliance/fcc.md) §1) |
| 4-layer recommendation | Espressif recommends a 4-layer board ([hardware design guidelines](../references/index.md#esp32s3-hw-design) §1.4); the project prefers 2 ([`pcb-fabrication.md`](../requirements/pcb-fabrication.md) §2) | Decide in the layout issue; module integration is already a listed reason for 4 layers |
| Shared USB PHY | USB download and logs are lost while the USB port is a host or standard device | UART0 header plus strapping-pin access ([`fcc.md`](../compliance/fcc.md) §1.5) |
| FCC database inaccessible to tools | Later grant changes unseen | Open the filing list in a browser and save it to the reference cache |

## 5. Open items

- **Digi-Key and Mouser** price, stock, lead time and lifecycle for all three
  parts (a person with a browser, or API access).
- **-N4R2** coverage by 2AC7Z-ESPS3MINI1.
- **Antenna gain** 3.96 dBi, and any later permissive change, checked against
  the FCC exhibits.
- **MDBT50Q** lifecycle status at Raytac.

<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog

Hardware (`hw-*`), firmware (`fw-*`) and protocol (`proto-*`) are versioned
and tagged independently (see `AGENTS.md`).

## Unreleased

- Radio interface circuits (`docs/research/radio-interface-circuits.md`,
  ADR-0003 proposed): RS-232-tolerant SERIAL-jack switching (MAX14778,
  TRS3221E, CI-V open drain), fail-safe PTT (AQY212EH PhotoMOS gated by a
  brownout supervisor; firmware lock-ups caught by the ESP32-S3's internal
  watchdogs, no external PTT timer), RTS/DTR
  mapping, isolation from 3.3 V with a 2.304 MHz band-clean push-pull
  supply, clock and harmonic audit (HF, 6 m, 2 m, 70 cm), USB routing, esp-usb
  driver support (CP2105 supported), per-radio cable table and dated LCSC
  sourcing with RoHS. The device no longer supplies VBUS to the radio
  (maintainer decision): constraints §3.1/§3.4/§6/§7 and REQ-RIF-007
  (withdrawn), REQ-RIF-011 and REQ-PTT-011 updated (#9).
- Variant R power (`docs/research/power-radio-usbc.md`, ADR-0005 proposed):
  input-only power from the radio's accessory DC pin or USB-C (sink only, no
  VBUS to the radio); a 0.25 A fuse, Schottky and TVS on the DC input; a
  TPS2121 priority mux feeding the shared ADR-0004 core (one LMR43620MC3RPERQ1
  3.3 V buck synchronized to 2.304 MHz, clear of the HF amateur bands); a power
  budget per mode (wired mode about 292 mA on a 500 mA USB-C port); a table of
  which radios can power variant R; radio-on sense and auto power up/down;
  dated LCSC prices with RoHS status, a power-section BOM cost, and EU EMC
  targets (#11).
- ADR-0002 (proposed): TI TLV320AIC3104 audio codec (alternates TAC5112,
  TLV320AIC3204) and Bourns SM-LP-5001 isolation transformers, with the
  codec clocked from the 2.304 MHz buck-sync oscillator, 3.0 V / 1.8 V LDO
  supplies, level plan, RF hardening and RoHS/REACH status
  (`docs/research/audio-codec.md`) (#8).
- Variant M automotive 12 V power front end (`docs/research/power-automotive.md`)
  and ADR-0004 (proposed): ISO 16750-2:2023 / ISO 7637-2:2011 levels confirmed
  (jump start now 26 V), LM74800-Q1 load-dump cut-off with a 150 V FET and TVS
  stack, CMC + pi filter, two LMR43620-Q1 bucks synchronized at 2.304 MHz to keep
  harmonics out of the HF amateur bands, brownout forcing PTT off, and ≤ 7 µA
  off-state drain. The device supplies no power to the radio (ADR-0003).
  `constraints.md` §3.2/§3.4 and `pcb-fabrication.md` §6.3 updated (#10).
- Per-radio power and interface table (`docs/research/radio-interfaces.md`):
  DC outputs and limits, USB port, chip and audio, CAT levels, PTT and audio
  levels for 13 HF/multiband radios plus generic interfaces, cited from the
  manufacturers' manuals, with cable mappings to the AUDIO and SERIAL jacks and
  a "needs measurement" list. Adds a `manual` kind to the reference library (#5).
- Protocol 0.1.0 draft (`protocol/SPEC.md`), system architecture
  (`docs/architecture.md`) and ADR-0007 (proposed). One COBS-framed,
  CRC-checked message stream over BLE GATT, L2CAP CoC, and, in wired mode, TCP
  over a new USB network interface (CDC-NCM, for iPhone/iPad) and the CDC-ACM
  control port. Covers capability discovery, CAT with credit flow control,
  PTT with keepalive, a user-configurable max TX (default 5 min, can be
  disabled; `AGENTS.md`, `CONTRIBUTING.md`, constraints §6 updated) and RTS/DTR arming, BLE audio framing, clock sync,
  optional tone-sequence TX, a capped BLE TX power, a BLE pairing window,
  watchdog-reset reporting (`WATCHDOG`), configurable defaults
  (serial defaults, USB network subnet, pairing window, power-down delay),
  security to the Cyber Resilience Act level (LE Secure Connections bonding,
  wired-host approval with `AUTH`, trusted-host list and removal, factory
  reset, signed updates, plain USB ports open with a `WIRED_PORT_LOCK`
  setting; #64), and new GATT UUIDs.
  Golden vectors in `protocol/vectors/`, a reference codec in
  `tools/protocol/`, and a `Protocol vectors` CI job. The USB endpoint budget
  changes the wired USB functions per radio type (`constraints.md` §2,
  REQ-HOST-003, -010, -013 to -017, REQ-PTT-002, -007, -011, REQ-PWR-018, REQ-FW-005) (#13).
- Host compatibility research (`docs/research/host-compatibility.md`): wired
  USB-C (CDC-ACM, RTS/DTR, UAC1 vs UAC2, voice processing, port power) and
  Bluetooth LE (2M PHY, DLE, MTU, intervals, L2CAP CoC vs GATT, throughput
  budget) on iOS/iPadOS, macOS, Android, Windows and Linux. Recommends UAC1 for
  #44 and NimBLE with L2CAP CoC plus a GATT fallback (#6).
- Radio module confirmation (`docs/research/module-selection.md`): the
  ESP32-S3-MINI-1 FCC grant (2AC7Z-ESPS3MINI1: single modular, BLE certified at
  10.3 dBm conducted, 20 cm mobile use), ISED ID, lifecycle and dated LCSC
  sourcing; supports ADR-0008. New `docs/compliance/fcc.md`: antenna keep-out
  and board integration rules, host label text, user manual statements and a
  Part 15B SDoC checklist. Revision A uses the -N8 ordering code; ADR-0008
  accepted by the maintainer (#7).
- Requirements specification (`docs/requirements/requirements.md`): numbered,
  testable `REQ-<area>-NNN` requirements for both host links (Bluetooth LE and
  wired USB-C), CAT, PTT fail-safes, audio, radio interfaces, isolation, power,
  regulatory, EMC, firmware, mechanical and manufacturing, with a traceability
  table and a coverage checklist against `constraints.md` (#4).
- CI checks (`.github/workflows/checks.yml`): `REUSE lint`, `KiCad version
  consistency` (every KiCad file and the docs against `KICAD_VERSION`, which
  now also records the symbol-library format) and `Silkscreen revision check`
  (title-block revision, `${REVISION}`/`${ISSUE_DATE}`, required markings,
  `hw-*` tags), with unit tests in `tools/kicad_ci/` (#3).
- PCB fabrication and passive-component rules (`docs/requirements/pcb-fabrication.md`):
  JLCPCB standard process, 2 layers preferred, 0402 resistors, MLCC with a 2×
  voltage rule and DC-bias check (#49).
- Core device shortlist for revision A (`docs/research/core-devices.md`):
  candidate key parts per block, with dated module price and stock (#42).
- ADR-0008 (proposed): Bluetooth LE and wired USB-C host links on the
  ESP32-S3-MINI-1, with one radio side for both: radios with USB serial + USB
  audio, USB serial + analog audio, or RS-232 / 3.3 V logic / CI-V + analog
  audio. In wired mode an on-board hub shows the radio's own USB chips to the
  computer. `constraints.md` and `README.md` updated to match (#42).
- Radio-side connector spec (`docs/requirements/radio-connectors.md`): AUDIO and
  SERIAL 3.5 mm TRRS jacks compatible with existing cables, firmware-selected
  RS-232-tolerant serial modes, a radio USB port and a USB-C port (#42).
- Reference library: `docs/references/` manifest and index, and
  `tools/refs/refs.py` to fetch local copies into a gitignored cache (#42).
- `SECURITY.md`: report security and safety problems privately through
  GitHub private vulnerability reporting (now enabled), linked from
  `GOVERNANCE.md` and the issue chooser (#47).
- Relicensed the hardware (CC-BY-NC-SA-4.0) and firmware
  (PolyForm-Noncommercial-1.0.0) as non-commercial, with commercial licenses
  available ([`COMMERCIAL.md`](COMMERCIAL.md)). Protocol, tools and CI stay MIT,
  and docs stay CC-BY-4.0. Contributor terms were added to
  [`CONTRIBUTING.md`](CONTRIBUTING.md). Earlier copies keep their original
  terms (CERN-OHL-P-2.0 / MIT).
- Repository bootstrap: agent docs, REUSE licensing (CERN-OHL-P-2.0 / MIT /
  CC-BY-4.0), design constraints, directory skeleton, templates.
- CI: required ERC/DRC merge gate (`KiCad ERC/DRC gate`): ERC on schematic
  changes, DRC on PCB changes, in the pinned `kicad/kicad` image (#26).
- `THIRD_PARTY.md`: a single ledger for third-party material and the notices it
  requires (#31).
- Signed commits are now preferred but not required: the branch ruleset no
  longer enforces signatures, and the docs and templates were updated to match (#35).
- Roadmap for revision A (`docs/roadmap.md`): phases, dependency graph, exit
  criteria, and merge gates (ERC for schematic changes, DRC for PCB changes).
- `GOVERNANCE.md`: roles, decision-making, merge and release authority, and why
  contributor terms are needed under dual licensing (#25).
- `CONTRIBUTING.md` expanded into a full contributor guide: ways to contribute,
  hardware/firmware/docs expectations, safety, third-party material, AI-assisted
  contributions and conduct (#32).
- Developer guide (`docs/developer-guide.md`): setup, workflow walkthrough, local
  checks and recipes (#24).

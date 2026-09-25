<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog

Hardware (`hw-*`), firmware (`fw-*`) and protocol (`proto-*`) are versioned
and tagged independently (see `AGENTS.md`).

## Unreleased

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

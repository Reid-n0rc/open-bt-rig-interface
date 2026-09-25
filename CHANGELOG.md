<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog

Hardware (`hw-*`), firmware (`fw-*`) and protocol (`proto-*`) are versioned
and tagged independently (see `AGENTS.md`).

## Unreleased

- EU compliance (`docs/compliance/eu.md`, ADR-0009, proposed): EU conformity is
  now required for every variant. Applicability per variant of the RED
  (safety, EMC, EN 300 328; cybersecurity under Delegated Regulation 2022/30,
  repealed from 2027-12-11), the Cyber Resilience Act, RoHS, REACH Art. 33,
  WEEE, GPSR and UN R10, with OJ references for the harmonised standards, the
  Module A route, declaration and technical file contents, markings, and who
  holds manufacturer obligations. BLE TX power capped at the module's EU-tested
  9.96 dBm e.i.r.p.; `constraints.md` §4 changed and REQ-REG-007 to -014 and
  REQ-EMC-008, -009 added (REQ-REG-006 withdrawn); 32 references added (#59).
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

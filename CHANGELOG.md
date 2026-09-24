<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog

Hardware (`hw-*`), firmware (`fw-*`) and protocol (`proto-*`) are versioned
and tagged independently (see `AGENTS.md`).

## Unreleased

- Core device shortlist for revision A (`docs/research/core-devices.md`):
  candidate key parts per block, and radio-module candidates checked against
  the hard constraints (#42).
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

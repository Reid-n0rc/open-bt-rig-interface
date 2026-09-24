<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Changelog

Hardware (`hw-*`), firmware (`fw-*`) and protocol (`proto-*`) are versioned
and tagged independently (see `AGENTS.md`).

## Unreleased

- Repository bootstrap: agent docs, REUSE licensing (CERN-OHL-P-2.0 / MIT /
  CC-BY-4.0), design constraints, directory skeleton, templates.
- `THIRD_PARTY.md`: a single ledger for third-party material and the notices it
  requires (#31).
- Signed commits are now preferred but not required: the branch ruleset no
  longer enforces signatures, and the docs and templates were updated to match (#35).
- Roadmap for revision A (`docs/roadmap.md`): phases, dependency graph, exit
  criteria, and merge gates (ERC for schematic changes, DRC for PCB changes).

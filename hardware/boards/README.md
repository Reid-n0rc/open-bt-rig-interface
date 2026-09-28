<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-NC-SA-4.0
-->

# hardware/boards

KiCad projects, one folder per board and revision: `hardware/boards/<board>/rev<X>/`. Revision A is one board, `interface/revA/`, built as KiCad 10 design variants `R` and `M` (docs/decisions/ADR-0006-variants-and-board-strategy.md); a future variant that needs its own copper gets its own board folder (for example `interface-U/`) with its own design variant. Each holds the `.kicad_pro`, `.kicad_sch`, `.kicad_pcb`, project library tables and any `.kicad_jobset`. Create and edit them only through the Konnect MCP tools, with KiCad >= the version in `KICAD_VERSION`. The title-block revision must match the `rev<X>` folder, and the front silkscreen shows `${VARIANT}`, `${REVISION}` and `${ISSUE_DATE}`. Fabrication outputs are exported per variant (`--variant R`, `--variant M`), never from the default variant. Fabrication outputs are CI-generated and never committed. License: CC-BY-NC-SA-4.0 (non-commercial; see COMMERCIAL.md).

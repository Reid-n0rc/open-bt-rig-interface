<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-NC-SA-4.0
-->

# hardware/boards

KiCad projects, one folder per board and revision: `hardware/boards/<board>/rev<X>/` (for example `core-R/revA/`). Each holds the `.kicad_pro`, `.kicad_sch`, `.kicad_pcb`, project library tables and any `.kicad_jobset`. Create and edit them only through the Konnect MCP tools, with KiCad >= the version in `KICAD_VERSION`. The title-block revision must match the `rev<X>` folder, and the silkscreen shows `${REVISION}`. Fabrication outputs are CI-generated and never committed. License: CC-BY-NC-SA-4.0 (non-commercial; see COMMERCIAL.md).

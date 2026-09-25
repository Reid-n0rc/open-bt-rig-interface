<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-NC-SA-4.0
-->

# hardware/enclosure

3D-printed enclosure: parametric CAD source per variant, `R/` (PETG, -20 to +60 °C) and `M/` (ASA, -40 to +85 °C, vehicle mounting and strain relief). Both fit the same board (`hardware/boards/interface/`); use the variant's STEP export (`kicad-cli pcb export step --variant <variant>`) as the fit reference (docs/decisions/ADR-0006-variants-and-board-strategy.md). Respect the radio module's antenna keep-out area (no metal), and leave room for the "Contains FCC ID" label. STL/3MF exports are generated and attached to releases, never committed. License: CC-BY-NC-SA-4.0 (non-commercial; see COMMERCIAL.md).

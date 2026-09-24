<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CERN-OHL-P-2.0
-->

# hardware/lib

Project-local KiCad libraries shared by all boards: `symbols/` (`.kicad_sym`), `footprints/` (`<lib>.pretty/`), `3dmodels/` (STEP). Reference them from board library tables using `${KIPRJMOD}`-relative paths. Record the source and license of any imported third-party symbol, footprint or model in [`THIRD_PARTY.md`](../../THIRD_PARTY.md). Items copied from the KiCad libraries stay CC-BY-SA-4.0. License: CERN-OHL-P-2.0 unless noted.

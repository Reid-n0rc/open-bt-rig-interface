// SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
// SPDX-License-Identifier: CC-BY-NC-SA-4.0
//
// Enclosure for variant R: powered from the radio's accessory DC or USB-C,
// -20 to +60 C (ADR-0006). Print in PETG (../README.md).
// CI exports every part in export_parts (tools/enclosure/export_enclosures.py),
// setting "part" with -D.

variant = "R";
material = "PETG";
fit_clear = 0.3;              // lid lip to base wall, per side
vehicle_mount = false;
flange_edges = ["W", "E"];    // only used when vehicle_mount is true
strain_relief_edges = ["S", "W"];
vent_edges = ["E"];

export_parts = ["base", "lid"];
part = "assembly";            // "base" | "lid" | "assembly" | "fit"

include <../common/board.scad>
include <../common/print.scad>
include <../common/enclosure.scad>

enclosure_part(part);

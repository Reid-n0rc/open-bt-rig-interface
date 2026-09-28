// SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
// SPDX-License-Identifier: CC-BY-NC-SA-4.0
//
// Enclosure for variant M: mobile 12 V, -40 to +85 C, parked-car heat
// (ADR-0006). Print in ASA (or ABS), never PLA or PETG (../README.md).
// Vehicle mounting flanges are on by default; set vehicle_mount = false for a
// desk build. CI exports every part in export_parts
// (tools/enclosure/export_enclosures.py), setting "part" with -D.

variant = "M";
material = "ASA";
fit_clear = 0.35;             // lid lip to base wall, per side (ASA shrinks more)
vehicle_mount = true;
flange_edges = ["W", "E"];
strain_relief_edges = ["S", "W"];   // W: the locking DC input cable
vent_edges = ["E"];

export_parts = ["base", "lid"];
part = "assembly";            // "base" | "lid" | "assembly" | "fit"

include <../common/board.scad>
include <../common/print.scad>
include <../common/enclosure.scad>

enclosure_part(part);

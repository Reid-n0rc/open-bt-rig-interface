// SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
// SPDX-License-Identifier: CC-BY-NC-SA-4.0
//
// FDM print and fit parameters shared by both variants (ADR-0010).
// Material-dependent values (fit_clear) are set in each variant file.
// Tuned for a 0.4 mm nozzle and 0.2 mm layers; see ../README.md.

// --- Walls -------------------------------------------------------------------
wall_t = 2.4;           // side walls: 6 perimeters of 0.4 mm
floor_t = 2.4;          // base floor
lid_t = 2.0;            // lid top
min_wall = 1.2;         // thinnest printed wall anywhere (3 perimeters)
ant_zone_wall_t = 1.6;  // walls and lid facing the antenna clearance zone are
                        // thinned to this (issue #19: no thick walls there)
corner_r = 4;           // outside corner radius

// --- Fit and tolerances --------------------------------------------------------
board_gap = 0.5;        // board edge to inner wall, each side
hole_clear = 0.3;       // added to printed hole diameters (FDM holes print small)
tongue_gap = 0.25;      // side gap between a lid tongue and its base notch
lip_h = 3;              // lid lip depth into the base
lip_t = 1.6;            // lid lip thickness
post_gap = 0.1;         // lid post end to board top (the screw closes it)

// --- Fasteners: M3 from below, through base boss and PCB into the lid post -----
boss_d = 8;             // base boss under each mounting hole
screw_clear_d = 3.4;    // M3 clearance
screw_head_d = 6.0;     // M3 socket/pan head (5.5) + clearance
screw_head_h = 3.2;     // counterbore depth from the underside
post_d = 7;             // lid post
insert_d = 4.0;         // bore for an M3 heat-set insert (use 2.5 for a self-tapping screw)
insert_depth = 6;       // bore depth in the post

// --- Status LED light pipe ------------------------------------------------------
led_pipe_d = 3.0;       // 3 mm light-pipe rod, or fill with clear filament
led_pipe_wall = 1.2;
led_gap = 1.0;          // pipe end to LED top

// --- Venting (vertical slots with a 45 degree peak: prints without support) -------
vent_w = 2.0;
vent_pitch = 5;

// --- Label recess (fcc.md section 2; CE/WEEE marks, variant and revision) --------
// On the outside of a side wall, outside the antenna clearance. 40 x 18 mm holds:
//   "Contains FCC ID: 2AC7Z-ESPS3MINI1"
//   "open-bt-rig-interface <variant> Rev <rev>"
//   CE mark (>= 5 mm high, eu.md section 12) beside the WEEE bin with its date bar
// Use a non-metallic (paper or polyester) label.
label_size = [40, 18];
label_depth = 0.4;
label_edge = "W";
label_pos = 50;         // centre along the edge (board coordinates)

// --- Strain relief: tie-wrap tabs at floor level outside the cable walls ---------
sr_len = 14;            // tab reach from the wall
sr_t = 3;               // tab thickness
tie_slot = [5.2, 2.4];  // [along the cable, across]: fits a 4.8 mm cable tie

// --- Vehicle mounting flanges (variant M) ---------------------------------------
flange_len = 14;
flange_t = 4;
flange_slot_d = 4.5;    // M4 / #8 screw
flange_slot_len = 8;

$fn = 48;

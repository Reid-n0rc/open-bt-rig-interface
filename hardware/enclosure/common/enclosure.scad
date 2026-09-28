// SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
// SPDX-License-Identifier: CC-BY-NC-SA-4.0
//
// Parametric two-part enclosure (base + lid) for the interface board
// (ADR-0006, ADR-0010). Included by R/enclosure_R.scad and M/enclosure_M.scad
// after board.scad and print.scad. Plain OpenSCAD, no libraries.
//
// Geometry is driven entirely by board.scad, print.scad and the variant file:
//   - The cavity holds the board (plus board_gap) AND the antenna clearance
//     box (antenna region + ant_clearance on every side, fcc.md 1.2).
//   - The base splits from the lid above the highest connector opening; the
//     openings are notches open at the top of the base wall, closed by
//     tongues hanging from the lid. Neither part needs support.
//   - Asserts stop the export if a parameter change breaks a rule (metal
//     screws or plastic posts inside the antenna clearance, label over an
//     opening, and so on).
//
// Parts: "base", "lid" (both in print orientation), "assembly" (preview),
// "fit" (intersection of the enclosure with the board model: must be empty).

// ============================================================================
// Derived dimensions
// ============================================================================

ant_x0 = ant_region[0][0];
ant_y0 = ant_region[0][1];
ant_x1 = ant_x0 + ant_region[1][0];
ant_y1 = ant_y0 + ant_region[1][1];
ck = ant_clearance;

// Antenna clearance box in x/y (the z extent follows below).
akx0 = ant_x0 - ck;
akx1 = ant_x1 + ck;
aky0 = ant_y0 - ck;
aky1 = ant_y1 + ck;

// Cavity (inside of the walls), board coordinates.
cav_x0 = min(-board_gap, akx0);
cav_x1 = max(board_w + board_gap, akx1);
cav_y0 = min(-board_gap, aky0);
cav_y1 = max(board_l + board_gap, aky1);
inner_r = max(corner_r - wall_t, 0.5);

// Heights (z = 0 is the underside of the base).
standoff_h = max(bottom_parts_h + 1, ck - board_t - ant_region_z[0]);
board_bot_z = floor_t + standoff_h;
board_top_z = board_bot_z + board_t;
cav_top_z = board_top_z + max(top_parts_h + 1, ant_region_z[1] + ck);
top_z = cav_top_z + lid_t;

// Connector record accessors.
function c_name(c) = c[0];
function c_edge(c) = c[1];
function c_pos(c) = c[2];
function c_zc(c) = board_top_z + c[3];
function c_w(c) = c[5][0];
function c_h(c) = c[4] == "circle" ? c[5][0] : c[5][1];
function c_r(c) = c[4] == "circle" ? c[5][0] / 2 : min(c[5][2], c_h(c) / 2 - 0.01);
function c_body(c) = c[6];
function c_overhang(c) = c[7];
function c_tie(c) = c[8];

// The base/lid split sits above every opening, leaving room for the lid lip.
open_top_max = max([for (c = connectors) c_zc(c) + c_h(c) / 2]);
open_bot_min = min([for (c = connectors) c_zc(c) - c_h(c) / 2]);
split_z = open_top_max + lip_h + 1;

// Vents sit below the board and below every opening.
vent_z0 = floor_t + 2;
vent_z1 = min(board_bot_z - 1, open_bot_min - 2);

// Label band on its wall.
plate_top = max(sr_t, vehicle_mount ? flange_t : 0);
label_z0 = max(floor_t, plate_top) + 2;
label_z1 = label_z0 + label_size[1];

// Edge geometry. Local wall coordinates: u along the edge (board x for S/N,
// board y for W/E), v = world z, w = outward from the plane.
function cav_plane(e) = e == "S" ? cav_y0 : e == "N" ? cav_y1 : e == "W" ? cav_x0 : cav_x1;
function board_plane(e) = e == "S" ? 0 : e == "N" ? board_l : e == "W" ? 0 : board_w;
function edge_span(e) = (e == "S" || e == "N") ? [cav_x0, cav_x1] : [cav_y0, cav_y1];
function on_edge(e) = [for (c = connectors) if (c_edge(c) == e) c];
function has(list, x) = len([for (i = list) if (i == x) 1]) > 0;

// World rectangle [x0, y0, x1, y1] of a band outside (w > 0) a plane.
function wrect(e, plane, u0, u1, w0, w1) =
    e == "S" ? [u0, plane - w1, u1, plane - w0] :
    e == "N" ? [u0, plane + w0, u1, plane + w1] :
    e == "W" ? [plane - w1, u0, plane - w0, u1] :
               [plane + w0, u0, plane + w1, u1];

// Distance from a point to the antenna clearance box in x/y (0 = inside).
function d_ant(p) = let(dx = max(akx0 - p[0], 0, p[0] - akx1),
                        dy = max(aky0 - p[1], 0, p[1] - aky1)) sqrt(dx * dx + dy * dy);

// Which cavity walls the clearance box reaches (these get thinned).
ant_edges = [for (e = ["S", "N", "W", "E"])
    if ((e == "S" && aky0 <= cav_y0) || (e == "N" && aky1 >= cav_y1) ||
        (e == "W" && akx0 <= cav_x0) || (e == "E" && akx1 >= cav_x1)) e];

// Standard screw length for the M3 through-bolt (head seat to insert end).
screw_len_raw = board_top_z + insert_depth - 0.2 - screw_head_h;
screw_len = max([for (l = [6, 8, 10, 12, 14, 16, 18, 20, 25, 30, 35, 40, 45, 50]) if (l <= screw_len_raw) l]);
screw_engagement = screw_head_h + screw_len - board_top_z;

// ============================================================================
// Checks (an assert failure stops the CI export)
// ============================================================================

valid_parts = ["base", "lid", "assembly", "fit"];
assert(has(valid_parts, part), str("unknown part '", part, "'"));
assert(variant == "R" || variant == "M", str("unknown variant ", variant));
assert(ant_clearance >= 15, "fcc.md 1.2 needs >= 15 mm antenna clearance");
assert(min(wall_t, floor_t, lid_t, ant_zone_wall_t, lip_t, led_pipe_wall) >= min_wall,
       "a wall is thinner than min_wall");
assert(screw_engagement >= 4, str("M3 screw engages only ", screw_engagement, " mm in the lid post"));
assert(split_z + 1 <= cav_top_z, "connectors too tall: base/lid split is above the cavity top");
for (c = connectors) {
    assert(has(["S", "N", "W", "E"], c_edge(c)), str(c_name(c), ": bad edge"));
    assert(has(["rect", "circle"], c[4]), str(c_name(c), ": bad shape"));
    assert(c_pos(c) - c_w(c) / 2 >= edge_span(c_edge(c))[0] + inner_r &&
           c_pos(c) + c_w(c) / 2 <= edge_span(c_edge(c))[1] - inner_r,
           str(c_name(c), ": opening runs into a corner"));
    assert(c_body(c)[1] <= c_w(c) && c_body(c)[2] <= c_h(c),
           str(c_name(c), ": body bigger than its opening"));
}
for (h = mount_holes) {
    assert(h[0] > 0 && h[0] < board_w && h[1] > 0 && h[1] < board_l, "mounting hole off the board");
    // Screws and heat-set inserts are metal: keep them out of the clearance box.
    assert(d_ant(h) >= max(boss_d, post_d) / 2,
           str("mounting hole ", h, " is inside the antenna clearance (no metal, fcc.md 1.1)"));
}
assert(d_ant([led[0], led[1]]) >= (led_pipe_d + 2 * led_pipe_wall) / 2,
       "LED light pipe is inside the antenna clearance");
// Label: inside its wall, clear of openings, vents and the base/lid split.
assert(label_pos - label_size[0] / 2 >= edge_span(label_edge)[0] + inner_r &&
       label_pos + label_size[0] / 2 <= edge_span(label_edge)[1] - inner_r,
       "label recess runs into a corner");
assert(label_z1 <= split_z - 1, "label recess crosses the base/lid split");
assert(!has(vent_edges, label_edge), "label wall must not carry vents");
for (c = on_edge(label_edge))
    assert(c_pos(c) + c_w(c) / 2 + 1 <= label_pos - label_size[0] / 2 ||
           c_pos(c) - c_w(c) / 2 - 1 >= label_pos + label_size[0] / 2,
           str("label recess overlaps the ", c_name(c), " opening"));

echo(str("enclosure ", variant, " (", material, "): outside ",
         cav_x1 - cav_x0 + 2 * wall_t, " x ", cav_y1 - cav_y0 + 2 * wall_t, " x ", top_z,
         " mm; board top at z = ", board_top_z, "; split at z = ", split_z,
         "; screws 4 x M3 x ", screw_len, " + M3 heat-set inserts"));

// ============================================================================
// 2D helpers
// ============================================================================

module rrect(x0, y0, x1, y1, r) {
    translate([x0 + r, y0 + r]) offset(r = r) square([x1 - x0 - 2 * r, y1 - y0 - 2 * r]);
}

module cavity_2d() { rrect(cav_x0, cav_y0, cav_x1, cav_y1, inner_r); }

module crect(w, h, r) {
    if (r > 0) offset(r = r) square([w - 2 * r, h - 2 * r], center = true);
    else square([w, h], center = true);
}

module opening_2d(c) {
    translate([c_pos(c), c_zc(c)])
        if (c[4] == "circle") circle(d = c_w(c));
        else crect(c_w(c), c_h(c), c_r(c));
}

// Base notch: the opening plus a slot up to the top of the base wall.
module notch_2d(c) {
    opening_2d(c);
    translate([c_pos(c) - c_w(c) / 2, c_zc(c)]) square([c_w(c), split_z - c_zc(c) + 1]);
}

// Lid tongue: fills the slot above the opening.
module tongue_2d(c) {
    difference() {
        translate([c_pos(c) - c_w(c) / 2 + tongue_gap, c_zc(c)])
            square([c_w(c) - 2 * tongue_gap, split_z - c_zc(c) + 0.01]);
        opening_2d(c);
    }
}

// Vertical vent slot with a 45 degree peak.
module vent_2d(u) {
    polygon([[u - vent_w / 2, vent_z0], [u + vent_w / 2, vent_z0],
             [u + vent_w / 2, vent_z1 - vent_w / 2], [u, vent_z1],
             [u - vent_w / 2, vent_z1 - vent_w / 2]]);
}

// ============================================================================
// 3D helpers
// ============================================================================

// Map local wall coordinates (x = u, y = v = world z, z = w outward) to the world.
module at_edge(e, plane) {
    if (e == "S") multmatrix([[1, 0, 0, 0], [0, 0, -1, plane], [0, 1, 0, 0], [0, 0, 0, 1]]) children();
    else if (e == "N") multmatrix([[1, 0, 0, 0], [0, 0, 1, plane], [0, 1, 0, 0], [0, 0, 0, 1]]) children();
    else if (e == "W") multmatrix([[0, 0, -1, plane], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]]) children();
    else multmatrix([[0, 0, 1, plane], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]]) children();
}

// Extrude a wall-plane profile (u, z) through a wall, from w0 to w1.
module through_wall(e, w0, w1) {
    at_edge(e, cav_plane(e)) translate([0, 0, w0]) linear_extrude(w1 - w0) children();
}

module wbox(r, z0, z1) {
    translate([r[0], r[1], z0]) cube([r[2] - r[0], r[3] - r[1], z1 - z0]);
}

module outer_shell(z0, z1) {
    translate([0, 0, z0]) linear_extrude(z1 - z0) offset(r = wall_t) cavity_2d();
}

module cavity_solid(z0, z1) {
    translate([0, 0, z0]) linear_extrude(z1 - z0) cavity_2d();
}

// Floor-level plate outside a wall, spanning [u0, u1].
module edge_plate(e, u0, u1, reach, t) {
    wbox(wrect(e, cav_plane(e), u0, u1, wall_t - 0.01, wall_t + reach), 0, t);
}

// Rounded slot through a floor-level plate, long axis outward from the wall.
module plate_slot(e, u, w_mid, width, len, t) {
    r = wrect(e, cav_plane(e), u - width / 2, u + width / 2, w_mid - len / 2, w_mid + len / 2);
    rr = min(width, len) / 2 - 0.01;
    translate([0, 0, -1]) linear_extrude(t + 2) rrect(r[0], r[1], r[2], r[3], rr);
}

function tie_span(e) = let(cs = [for (c = on_edge(e)) if (c_tie(c)) c])
    len(cs) == 0 ? [] :
    [min([for (c = cs) c_pos(c) - c_w(c) / 2]) - tie_slot[1] - 4,
     max([for (c = cs) c_pos(c) + c_w(c) / 2]) + tie_slot[1] + 4];

// ============================================================================
// Base
// ============================================================================

module base() {
    difference() {
        union() {
            difference() {
                outer_shell(0, split_z);
                cavity_solid(floor_t, split_z + 1);
            }
            // Bosses under the mounting holes.
            for (h = mount_holes)
                translate([h[0], h[1], floor_t - 0.01]) cylinder(d = boss_d, h = standoff_h + 0.01);
            // Strain-relief tabs.
            for (e = strain_relief_edges) {
                s = tie_span(e);
                if (len(s) == 2) edge_plate(e, max(s[0], edge_span(e)[0]), min(s[1], edge_span(e)[1]), sr_len, sr_t);
            }
            // Vehicle mounting flanges (variant M).
            if (vehicle_mount)
                for (e = flange_edges)
                    edge_plate(e, edge_span(e)[0] + inner_r, edge_span(e)[1] - inner_r, flange_len, flange_t);
        }
        // Connector notches.
        for (c = connectors) through_wall(c_edge(c), -0.01, wall_t + 1) notch_2d(c);
        // Screw holes and head counterbores.
        for (h = mount_holes) {
            translate([h[0], h[1], -1]) cylinder(d = screw_clear_d + hole_clear, h = board_bot_z + 2);
            translate([h[0], h[1], -1]) cylinder(d = screw_head_d + hole_clear, h = screw_head_h + 1);
        }
        // Vents, skipping the bosses.
        if (vent_z1 - vent_z0 >= 4)
            for (e = vent_edges) {
                sp = edge_span(e);
                n = floor((sp[1] - sp[0] - 2 * inner_r - 8) / vent_pitch);
                us = [for (i = [0 : n - 1]) (sp[0] + sp[1]) / 2 + (i - (n - 1) / 2) * vent_pitch];
                through_wall(e, -0.01, wall_t + 1)
                    for (u = us)
                        if (len([for (h = mount_holes)
                                 if (abs(u - ((e == "S" || e == "N") ? h[0] : h[1])) < boss_d / 2 + vent_w)
                                     1]) == 0)
                            vent_2d(u);
            }
        // Label recess on the outside of its wall.
        through_wall(label_edge, wall_t - label_depth, wall_t + 1)
            translate([label_pos - label_size[0] / 2, label_z0]) square(label_size);
        // Thin the walls the antenna clearance box reaches (inner pocket,
        // stopping below the lid lip seat).
        for (e = ant_edges) {
            sp = (e == "S" || e == "N") ? [max(akx0, cav_x0 + inner_r), min(akx1, cav_x1 - inner_r)]
                                        : [max(aky0, cav_y0 + inner_r), min(aky1, cav_y1 - inner_r)];
            through_wall(e, -0.01, wall_t - ant_zone_wall_t)
                translate([sp[0], floor_t]) square([sp[1] - sp[0], split_z - lip_h - 1 - floor_t]);
        }
        // Tie-wrap slots either side of each strain-relieved cable.
        for (e = strain_relief_edges)
            for (c = on_edge(e))
                if (c_tie(c))
                    for (s = [-1, 1])
                        plate_slot(e, c_pos(c) + s * (c_w(c) / 2 + tie_slot[1] / 2 + 1.5),
                                   wall_t + sr_len / 2, tie_slot[1], tie_slot[0], sr_t);
        // Vehicle flange slots, near the flange ends.
        if (vehicle_mount)
            for (e = flange_edges) {
                sp = edge_span(e);
                for (u = [sp[0] + inner_r + 7, sp[1] - inner_r - 7])
                    plate_slot(e, u, wall_t + flange_len / 2 + 1, flange_slot_d, flange_slot_d + flange_slot_len, flange_t);
            }
    }
}

// ============================================================================
// Lid (modelled in place; lid_printable() flips it for printing)
// ============================================================================

module lid_in_place() {
    difference() {
        union() {
            difference() {
                outer_shell(split_z, top_z);
                cavity_solid(split_z - 1, cav_top_z);
            }
            // Lip into the base.
            translate([0, 0, split_z - lip_h]) linear_extrude(lip_h + 0.01) difference() {
                cavity_2d_in(fit_clear);
                cavity_2d_in(fit_clear + lip_t);
            }
            // Tongues closing the connector notches.
            for (c = connectors) through_wall(c_edge(c), 0, wall_t) tongue_2d(c);
            // Posts clamping the board at each mounting hole.
            for (h = mount_holes)
                translate([h[0], h[1], board_top_z + post_gap]) cylinder(d = post_d, h = cav_top_z - board_top_z - post_gap + 0.01);
            // Light-pipe tube over the LED.
            translate([led[0], led[1], board_top_z + led[2] + led_gap])
                cylinder(d = led_pipe_d + hole_clear + 2 * led_pipe_wall, h = cav_top_z - board_top_z - led[2] - led_gap + 0.01);
        }
        // Heat-set insert bores.
        for (h = mount_holes)
            translate([h[0], h[1], board_top_z]) cylinder(d = insert_d, h = insert_depth);
        // Light-pipe hole through the lid.
        translate([led[0], led[1], board_top_z]) cylinder(d = led_pipe_d + hole_clear, h = top_z);
        // Thin the lid over the antenna clearance box (inner pocket).
        translate([0, 0, cav_top_z - 0.01]) linear_extrude(lid_t - ant_zone_wall_t + 0.01)
            intersection() {
                cavity_2d();
                translate([akx0, aky0]) square([akx1 - akx0, aky1 - aky0]);
            }
    }
}

module cavity_2d_in(d) { offset(delta = -d) cavity_2d(); }

module lid_printable() {
    translate([0, 0, top_z]) rotate([180, 0, 0]) lid_in_place();
}

// ============================================================================
// Board model for the fit check
// ============================================================================

module board_2d() { rrect(0, 0, board_w, board_l, board_corner_r); }

module placeholder_board() {
    eps = 0.02;   // contact faces (bosses, post ends) are allowed to touch
    difference() {
        union() {
            // PCB.
            translate([0, 0, board_bot_z + eps]) linear_extrude(board_t - 2 * eps) board_2d();
            // Connector bodies from the board edge inwards (plus any overhang).
            for (c = connectors) {
                b = c_body(c);
                at_edge(c_edge(c), board_plane(c_edge(c)))
                    translate([c_pos(c) - b[1] / 2, c_zc(c) - b[2] / 2, -(b[0] - c_overhang(c))])
                        cube([b[1], b[2], b[0]]);
            }
            // Radio module (15.4 x 20.5 x module height, antenna end on ant_region).
            translate([ant_x0, ant_y1 - 20.5, board_top_z]) cube([ant_x1 - ant_x0, 20.5, ant_region_z[1]]);
            // Generic top-side and bottom-side part envelopes, clear of the
            // posts, bosses and light pipe.
            translate([0, 0, board_top_z]) linear_extrude(top_parts_h - eps) difference() {
                offset(delta = -1) board_2d();
                for (h = mount_holes) translate(h) circle(d = post_d + 1);
                translate([led[0], led[1]]) circle(d = led_pipe_d + hole_clear + 2 * led_pipe_wall + 1);
            }
            translate([0, 0, board_bot_z - bottom_parts_h]) linear_extrude(bottom_parts_h) difference() {
                offset(delta = -1) board_2d();
                for (h = mount_holes) translate(h) circle(d = boss_d + 1);
            }
        }
        for (h = mount_holes) translate([h[0], h[1], -1]) cylinder(d = mount_hole_d, h = top_z + 2);
    }
}

module board_model() {
    if (pcb_model != "") translate([0, 0, board_bot_z] + pcb_model_offset) import(pcb_model);
    else placeholder_board();
}

// ============================================================================
// Entry point
// ============================================================================

module enclosure_part(p) {
    if (p == "base") base();
    else if (p == "lid") lid_printable();
    else if (p == "assembly") {
        color("SteelBlue", 0.9) base();
        color("LightSteelBlue", 0.6) lid_in_place();
        color("DarkGreen") board_model();
        // Antenna clearance box, for looking at only.
        %translate([akx0, aky0, board_top_z + ant_region_z[0] - ck])
            cube([akx1 - akx0, aky1 - aky0, ant_region_z[1] - ant_region_z[0] + 2 * ck]);
    }
    else if (p == "fit") intersection() {
        union() { base(); lid_in_place(); }
        board_model();
    }
}

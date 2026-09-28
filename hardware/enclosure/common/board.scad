// SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
// SPDX-License-Identifier: CC-BY-NC-SA-4.0
//
// Board parameters for the enclosure (ADR-0010). This is the ONLY file that
// follows the KiCad board (hardware/boards/interface/revA/, #21). Both
// variants (R and M) share one board outline and connector positions
// (ADR-0006), so both enclosures read this one file.
//
// !! Every value marked PLACEHOLDER is a guess made before the KiCad board
// !! existed. Replace it with the real value from the board (#21 / first
// !! layout) and delete the PLACEHOLDER tag. Values without the tag come
// !! from a cited project document.
//
// Units: mm. Board coordinates: origin at the lower-left corner of the board
// outline, seen from the top (component) side; +x to the right, +y towards
// the antenna edge; z is measured up from the board's top face. In KiCad,
// set the drill/place origin at that corner so exported coordinates match
// (KiCad's y axis points down: y_here = board_l - y_kicad if the origin is
// the upper-left corner instead).

// --- Outline --------------------------------------------------------------
board_w = 66;          // PLACEHOLDER: ADR-0006 area estimate, ~66 x 66 mm
board_l = 66;          // PLACEHOLDER
board_t = 1.6;         // PLACEHOLDER: JLCPCB standard thickness (pcb-fabrication.md)
board_corner_r = 1;    // PLACEHOLDER

// --- Mounting holes: [x, y] centres ----------------------------------------
mount_holes = [        // PLACEHOLDER: M3 at 3.5 mm from each corner
    [3.5, 3.5], [board_w - 3.5, 3.5],
    [3.5, board_l - 3.5], [board_w - 3.5, board_l - 3.5]
];
mount_hole_d = 3.2;    // PLACEHOLDER: M3 clearance hole in the PCB

// --- Component envelope -----------------------------------------------------
top_parts_h = 12;      // PLACEHOLDER: tallest top-side part above the board top
                       //   (audio transformers are 7.5 mm, ADR-0006 input table)
bottom_parts_h = 2.5;  // PLACEHOLDER: through-hole leads below the board bottom

// --- Radio module antenna (docs/compliance/fcc.md section 1) ---------------
// The module's "Antenna Area" as a rectangle on the board: [[x, y], [size_x, size_y]].
// Width = module width 15.4 mm (land pattern, fcc.md 1.1); depth 6 mm
// (antenna region, Fig. 26 per fcc.md 1.1). Position is a PLACEHOLDER: the
// module sits on the +y edge with its antenna end at the board edge.
ant_region = [[25.3, board_l - 6], [15.4, 6]];   // PLACEHOLDER position
// Height of the antenna region above the board top: [bottom, top].
// Conservative: the full module height (2.4 mm, ESP32-S3-MINI-1 datasheet,
// verify against the #21 footprint's 3D model).
ant_region_z = [0, 2.4];
// Clearance kept free around the antenna inside the housing, in all
// directions (fcc.md 1.2, from the ESP32-S3 hardware design guidelines).
// Not a placeholder: this is the compliance floor. Don't reduce it without
// an RF test in the final enclosure (#17, #20) and an ADR.
ant_clearance = 15;

// --- Edge connectors ---------------------------------------------------------
// One row per connector that needs an enclosure opening:
// [ name, edge ("S", "N", "W", "E"), position along the edge (x for S/N,
//   y for W/E, opening centre), opening centre height above the board top,
//   opening shape ("rect" or "circle"), opening size ([w, h, corner_r] or [d]),
//   body [depth into the board, width, height] for the fit check,
//   body overhang past the board edge, strain relief (true = tie point) ]
// Openings are sized for the mating plug's overmould, not the receptacle.
// Radio side (USB-A, AUDIO, SERIAL) on S; host USB-C on E; the power-entry
// strip (ADR-0006 decision 5: locking 3-pin DC connector) on W; antenna on N.
connectors = [
    // PLACEHOLDER: USB-A receptacle, radio USB (radio-connectors.md)
    ["radio_usb", "S", 16, 3.5, "rect",   [17, 9, 1],   [14, 14.5, 7],  0,   true],
    // PLACEHOLDER: 3.5 mm TRRS AUDIO jack (radio-connectors.md)
    ["audio",     "S", 36, 3.0, "circle", [8],          [12, 6.5, 6],   0,   true],
    // PLACEHOLDER: 3.5 mm TRRS SERIAL jack (radio-connectors.md)
    ["serial",    "S", 52, 3.0, "circle", [8],          [12, 6.5, 6],   0,   true],
    // PLACEHOLDER: USB-C receptacle, host link / R power fallback
    ["usb_c",     "E", 33, 1.6, "rect",   [13, 7.5, 3], [7.5, 9, 3.3],  0.5, false],
    // PLACEHOLDER: locking 3-pin DC input (ADR-0006 decision 5; part: #21)
    ["dc_in",     "W", 20, 5.0, "rect",   [14, 12, 1],  [12, 11, 10],   0,   true]
];

// --- Status LED --------------------------------------------------------------
led = [5, 50, 1.0];    // PLACEHOLDER: [x, y, LED top above the board top]

// --- Fit check model -----------------------------------------------------------
// Path (relative to the variant .scad file) of an STL of the assembled
// board, exported per variant with
//   kicad-cli pcb export stl --variant <R|M> ... (see hardware/enclosure/README.md)
// "" uses the placeholder board model built from the values above.
pcb_model = "";
pcb_model_offset = [0, 0, 0];   // shift of that STL into board coordinates

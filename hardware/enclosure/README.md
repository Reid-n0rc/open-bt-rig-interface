<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-NC-SA-4.0
-->

# hardware/enclosure

3D-printed two-part enclosure (base + lid) for the interface board, one build
per variant: `R/` (PETG, −20 to +60 °C) and `M/` (ASA, −40 to +85 °C, vehicle
mounting). Both fit the same board (`hardware/boards/interface/`,
[ADR-0006](../../docs/decisions/ADR-0006-variants-and-board-strategy.md)).
The source is plain OpenSCAD 2021.01 with no libraries
([ADR-0010](../../docs/decisions/ADR-0010-enclosure-cad.md)).

License: CC-BY-NC-SA-4.0 (non-commercial; see `COMMERCIAL.md`).

> **Placeholder geometry.** The KiCad board (#21) doesn't exist yet. Board
> outline, mounting holes, component heights, connector positions and the LED
> position in `common/board.scad` are **placeholders**, each tagged
> `PLACEHOLDER`. Don't print for a real board until #21 has replaced them.

## Files

| File | What it holds |
|---|---|
| `common/board.scad` | **The one file that follows the KiCad board.** Outline, mounting holes, part heights, antenna region and clearance, connector openings, LED, fit-check model |
| `common/print.scad` | Walls, FDM tolerances, fasteners, light pipe, vents, label, strain relief, flanges |
| `common/enclosure.scad` | The model and its design-rule asserts |
| `R/enclosure_R.scad`, `M/enclosure_M.scad` | Variant settings and the parts to export |

## Updating from the board (#21)

1. Put the board's drill/place origin on the lower-left corner of the outline.
2. Copy the outline size, the mounting holes, the tallest part heights, the
   module's antenna area, each edge connector's position and height, and the
   LED position into `common/board.scad`; remove each `PLACEHOLDER` tag.
3. Export the assembled board per variant and point `pcb_model` at it:

   ```sh
   kicad-cli pcb export stl --variant R --no-dnp --drill-origin \
     -o hardware/enclosure/R/board-R.stl hardware/boards/interface/revA/interface.kicad_pcb
   ```

   KiCad puts the board's bottom face at z = 0 (check it and set
   `pcb_model_offset` if not). The STL is generated: don't commit it (it is
   gitignored); CI will need the same export step once the board exists.
4. Run the export (below). The fit check must pass.

## Build

```sh
python3 tools/enclosure/export_enclosures.py --list        # variants and parts
python3 tools/enclosure/export_enclosures.py               # fit check + STL/3MF into build/enclosure/
openscad -D 'part="assembly"' hardware/enclosure/M/enclosure_M.scad   # preview in the GUI
```

`part` is `base`, `lid` (both in print orientation), `assembly` (preview, with
the antenna clearance box shown transparent) or `fit` (enclosure ∩ board:
must be empty). The `Enclosure export` CI job runs the same script and uploads
the files as the `enclosure-exports` artifact. STL/3MF files are release
artifacts: never commit them.

## Design features

| Feature | How |
|---|---|
| Antenna keep-out ([`fcc.md`](../../docs/compliance/fcc.md) §1.1–1.2) | The cavity contains the antenna region plus **15 mm on every side, above and below**. Walls and the lid facing that zone are thinned to 1.6 mm. Asserts forbid screws, inserts, posts or the light pipe inside it and a clearance under 15 mm. No metal there, ever: no metal- or carbon-filled filament, conductive paint or metal labels |
| PCB mounting | Bosses under each mounting hole; 4 × M3 screws from below (counterbored) through boss and PCB into M3 heat-set inserts in lid posts, which clamp the board. The build prints the screw length (M3 × 20 with the placeholders) |
| Connector openings | USB-C (host), radio USB-A, AUDIO and SERIAL 3.5 mm jacks, locking DC input: notches open at the top of the base wall, sized for the mating plug; tongues on the lid close them above the plug |
| Strain relief | Tie-wrap tabs at floor level outside the cable walls, with a pair of slots beside each cable (4.8 mm ties) |
| LED | 3 mm light-pipe tube from the lid down to 1 mm above the LED; fit a 3 mm light-pipe rod or fill with clear filament |
| Venting | Vertical slots with 45° peaks in one side wall, below the board |
| Label | 40 × 18 × 0.4 mm recess on a side wall, outside the antenna zone: "Contains FCC ID: 2AC7Z-ESPS3MINI1", model, variant and revision, CE mark (≥ 5 mm high) and the WEEE bin with date bar ([`eu.md`](../../docs/compliance/eu.md) §12). Use a paper or polyester label, not a metal one |
| Vehicle mounting (M) | Flanges along two sides with slots for M4 / #8 screws. The base floor is ≥ 15 mm below the antenna, so mounting on a metal surface keeps the clearance. Set `vehicle_mount = false` for a desk build |

## Printing

- **Orientation:** base on its floor; lid upside down (top on the bed). Both
  print **without support**: openings are open-topped notches, vents have
  pointed tops, posts and tongues point up on the lid.
- **Walls:** side walls 2.4 mm, floor 2.4 mm, lid 2.0 mm, thinnest wall
  1.2 mm (3 perimeters at 0.4 mm). Use enough perimeters that walls print
  solid (6 at 0.4 mm), 0.2 mm layers, 30–40 % infill.
- **Tolerances:** board-to-wall gap 0.5 mm, lid lip 0.30 mm (PETG) / 0.35 mm
  (ASA) per side, holes +0.3 mm, tongue gap 0.25 mm. Adjust in
  `common/print.scad` or the variant file for your printer.
- **Heat-set inserts:** M3, 4.0 mm bore, 6 mm deep. For self-tapping screws
  instead, set `insert_d = 2.5`.

| | R | M |
|---|---|---|
| Material | **PETG** | **ASA** (or ABS). **Not PETG or PLA**: they soften in a parked car ([constraints §11](../../docs/requirements/constraints.md#11-environmental-and-mechanical)) |
| Typical nozzle / bed | 230–250 °C / 70–85 °C | 240–260 °C / 90–110 °C |
| Cooling | Low to moderate | Off or minimal; enclosed printer to avoid warping |
| Notes | — | Heat-soak test (REQ-ENV-003) in #20 |

**PLA is not recommended** for either variant. Temperatures are starting
points; follow the filament maker's data sheet.

Print and fit on real printers is #20.

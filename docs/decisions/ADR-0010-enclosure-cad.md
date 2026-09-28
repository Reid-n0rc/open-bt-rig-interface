<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0010: Enclosure CAD in plain OpenSCAD, driven by one board parameter file

- **Status:** proposed
- **Date:** 2026-09-28
- **Issue:** #19

## Context

Each variant gets a 3D-printed enclosure: `hardware/enclosure/R/` (PETG) and
`hardware/enclosure/M/` (ASA), both built on the same board outline and
connector positions ([ADR-0006](ADR-0006-variants-and-board-strategy.md)
decision 7). The source must be parametric and kept in the repo, and CI must
export STL/3MF for the releases without committing them
([constraints §11](../requirements/constraints.md#11-environmental-and-mechanical);
REQ-MECH-001 to -005 in [`requirements.md`](../requirements/requirements.md)).

The enclosure has to honour:

- the antenna rules in [`fcc.md`](../compliance/fcc.md) §1.1–1.2: at least
  **15 mm of clearance around the PCB antenna in all directions** inside the
  housing, and no metal (screws, inserts, metal labels, filled filament) there;
- label space on the outside for `Contains FCC ID: 2AC7Z-ESPS3MINI1`
  ([`fcc.md`](../compliance/fcc.md) §2), the CE mark (at least 5 mm high) and
  the WEEE bin with its date bar ([`eu.md`](../compliance/eu.md) §12), and the
  variant and revision;
- ADR-0006's power-entry strip along one edge with a locking 3-pin connector
  (strain relief, vehicle mounting on M).

When this was written the KiCad board (#21) did not exist yet, so the tool
must also make it cheap to drop in the real outline, holes and connector
positions later.

The issue asked for CERN-OHL-P-2.0 on the enclosure sources. That is outdated:
[`REUSE.toml`](../../REUSE.toml) and [`AGENTS.md`](../../AGENTS.md#licensing-reuse)
put everything under `hardware/`, enclosure CAD included, under
**CC-BY-NC-SA-4.0**. This ADR follows `REUSE.toml`.

## Options considered

| Criterion | OpenSCAD (plain, no libraries) | FreeCAD (Part Design, Spreadsheet parameters) |
|---|---|---|
| Parametric edits from board dimensions | Every dimension is a variable in a text file; the whole model is recomputed from them. Rules can be written as `assert()`s that stop the build | Spreadsheet-driven constraints work, but a changed outline can break sketch references (topological naming, improved but not gone in 1.0) |
| STEP import of the PCB | **No STEP import.** Imports STL/3MF/OFF/AMF. KiCad 10 exports the assembled board as STL per variant (`kicad-cli pcb export stl --variant …`), which OpenSCAD imports for the fit check | Native STEP import; the KiCad STEP drops straight in |
| Headless STL/3MF export in CI | `openscad -o x.stl` / `-o x.3mf`, one command, no display needed; Ubuntu 24.04 ships 2021.01 with 3MF support ([package](../references/index.md#ubuntu-noble-openscad)) | `freecadcmd` plus a Python script; the document must be recomputed and exported per body; heavier image |
| Text diffs | The source is the text; reviews read like code | `.FCStd` is a zip of XML with generated shape data: not reviewable in a PR |
| Tool license | GPL-2.0 with a CGAL linking exception ([COPYING](../references/index.md#openscad-license)). Using it doesn't bind our files or its output ([GPL FAQ](../references/index.md#gpl-faq-output)); nothing of it is bundled | LGPL-2.0 ([LICENSE](../references/index.md#freecad-license)); same conclusion |
| Libraries | None needed. BOSL2 (BSD-2-Clause, [LICENSE](../references/index.md#bosl2-license)) would be license-compatible but isn't used, so nothing goes in `THIRD_PARTY.md` | None needed |
| Fillets and chamfers | Limited (offset/hull); fine for a printed box | Full B-rep features |

## Decision

1. **Plain OpenSCAD, no libraries.** It wins on the three criteria that matter
   for this repo (text diffs, trivial headless export, rules checked in CI).
   The one gap, STEP import, is covered by KiCad's STL export for the fit check.
   If a future enclosure needs real fillets or STEP-accurate fits that
   OpenSCAD can't do, a later ADR can move to FreeCAD or build123d.
2. **Pinned version:** OpenSCAD **2021.01**, the Ubuntu 24.04 package
   `2021.01-6build4`, installed by the `Enclosure export` job. The export script
   refuses any other version (`OPENSCAD_VERSION`). Bump both together.
3. **Layout** (`hardware/enclosure/`):

   | File | Role |
   |---|---|
   | `common/board.scad` | **The only file that follows the KiCad board**: outline, mounting holes, component heights, antenna region, connector openings, LED. Every guessed value is tagged `PLACEHOLDER` until #21 fills it in |
   | `common/print.scad` | Walls, tolerances, fasteners, vents, label, strain relief, flanges |
   | `common/enclosure.scad` | The model and its asserts |
   | `R/enclosure_R.scad`, `M/enclosure_M.scad` | Variant settings (material, fit clearance, vehicle mounting) and the part list to export |

4. **Antenna clearance is built in, not drawn.** The cavity is sized to contain
   the antenna region grown by `ant_clearance` (15 mm, the fcc.md floor) on every
   side, including above and below, and the walls and lid that face that box are
   thinned. Asserts fail the build if a screw, insert, post or the light pipe
   lands inside it, or if `ant_clearance` drops below 15 mm.
5. **Fit check in CI.** Part `fit` is the intersection of the enclosure with the
   board model; it must be empty. Until #21 exists the board model is a
   placeholder built from `board.scad`; afterwards it's the variant's KiCad STL
   (`pcb_model`). The check already caught a USB-A body hitting a lid post in the
   placeholder layout.
6. **Outputs:** `tools/enclosure/export_enclosures.py` exports each part of
   each variant as STL and 3MF into `build/enclosure/` (gitignored); CI uploads
   them as the `enclosure-exports` artifact. Attaching them to `hw-<variant>-…`
   releases is part of the release workflow.

## Consequences

- #21 updates `common/board.scad` from the board (and sets `pcb_model` to a
  per-variant STL export) and removes the `PLACEHOLDER` tags. No other file
  should need to change for a new outline or connector position.
- **Size:** with the placeholder board (66 × 66 mm) the enclosure is about
  72 × 86 × 37 mm. Most of that height and the 15 mm extension on the antenna
  side come from the 15 mm clearance "in all directions": the board sits about
  15 mm above the floor. REQ-MECH-008's size target is still the maintainer's.
  If an RF test in the enclosure (#17, #20) shows plastic closer than 15 mm is
  harmless, the clearance could apply to metal only, which would shrink the box;
  that needs a new decision, not a parameter edit.
- The issue's "screenshot of the clearance check in CAD" can only be real once
  the board exists; the placeholder check runs in CI now.
- Print and fit on real printers is #20 (human task).
- The `Enclosure export` check always reports, so the maintainer can mark it
  required.

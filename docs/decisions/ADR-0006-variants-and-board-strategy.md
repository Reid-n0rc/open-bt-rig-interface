<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0006: One board with KiCad 10 design variants R and M; folder layout, tags and silkscreen variant marking

- **Status:** proposed
- **Date:** 2026-09-25
- **Issue:** #12

## Context

Revision A has two builds ([constraints §3, §11, §13](../requirements/constraints.md#13-tooling-and-documentation);
REQ-GEN-006, REQ-ISO-002 in [`requirements.md`](../requirements/requirements.md)):

- **R:** powered from the radio's accessory DC or USB-C; −20 to +60 °C; PETG enclosure.
- **M:** mobile/automotive 12 V; −40 to +85 °C; ASA enclosure.

This ADR decides whether they are separate boards or one board with fitting
options, and sets the folder layout, the release tags, the silkscreen variant
marking and which enclosure goes with which build. The decision must land
before the KiCad project is set up (#21).

The inputs are the open research PRs:

| Input | What it fixes for this decision |
|---|---|
| #5 radio table ([PR #56](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/56)) | Which radios can power R from accessory DC; the rest use USB-C |
| #8 audio, ADR-0002 ([PR #55](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/55)) | TLV320AIC3104 codec on both variants. Bourns SM-LP-5001E transformers (12.8 × 9.0 mm, 7.5 mm high) **fitted on M, DNP with 0 Ω bypass on R**; "the board carries both footprints, so either build uses one layout" |
| #9 interface circuits, ADR-0003 ([PR #61](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/61)) | Same SERIAL, PTT, USB hub and switch circuits on both. Jack isolation (ISO7721, ISO1540, jack-domain supply) fitted on M, DNP with 0 Ω links on R. USB-C receptacle TYPE-C-31-M-12 is −30 to +80 °C: fine for R, **not for M** |
| #10 variant M power, ADR-0004 ([PR #57](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/57)) | M input chain: fuse, TPSMB82A + SMBJ33CA TVS stack, 250 V C_in, LM74800-Q1 with two FETs, VS clamp, CMC, pi filter, 100 µF electrolytic, TPS3710, SENSE/HOLD network. Owns the **shared regulator core** (one LMR43620MC3RPERQ1, 3.3 V) and the **2.304 MHz clock** (18.432 MHz ÷ 8). Layout: input zones in order along one edge |
| #11 variant R power, ADR-0005 ([PR #60](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/60)) | R input chain: 0.25 A fuse, B5819W, SMBJ15A, TPS2121 mux (radio DC priority, USB-C fallback), pi filter; then the ADR-0004 core. Leaves the radio DC connector to this ADR |
| #59 EU compliance (in progress, [issue](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59)) | EU conformity is required for the product |

Maintainer decisions this ADR respects:

- The **core is identical across variants**: the 3.3 V buck synced to
  2.304 MHz, the clock, the codec, the ESP32-S3-MINI-1-N8, the hub and USB
  switches, and the SERIAL/AUDIO circuits.
- The variants differ in the **power input chain** (M automotive, R radio DC +
  USB-C mux) and in the **isolation fitting** (transformers and isolators fitted
  on M; DNP with 0 Ω bypass on R).
- The optimization goal is **spurious emissions and cost**. 2-layer boards,
  JLCPCB standard process ([`pcb-fabrication.md`](../requirements/pcb-fabrication.md)).

### Verify first: KiCad 10 design variants

The issue required confirming KiCad 10's design variants before relying on
them. Result: **supported as assumed**, with one limit (ERC and DRC have no
variant option, which suits this design).

| Capability | Source | Local check (kicad-cli 10.0.6, scratch project outside the repo) |
|---|---|---|
| Design variants exist in KiCad 10 | [KiCad 10.0.0 release announcement](../references/index.md#kicad-10-0-0-release) ("Schematic Editing: Variants") | — |
| Fitted/DNP per variant; per-variant exclusion from BOM, position files and simulation; **field overrides** (Value, Footprint, a manufacturer part number field); sheet-level DNP for a whole hierarchical sheet; the Reference can't be overridden | [Schematic Editor manual, "Design variants"](../references/index.md#kicad10-eeschema-manual) | A KiCad-written board stores `(variants (variant (name …)))` at board level and `(variant (name …) (dnp yes))` or field overrides in each footprint |
| **All variants share one copper, netlist and board outline** ("If your product configurations require different copper or different board shapes, you need separate board designs") | Schematic Editor manual, "Design variants" | — |
| Variant-aware BOM | Manual, "BOM export with variants"; [CLI reference](../references/index.md#kicad10-cli-manual) | `sch export bom --variant` gave a different BOM from the default variant |
| Variant-aware position (pick-and-place), Gerber, PDF, STEP, ODB++, IPC-2581 exports | [PCB Editor manual, "Design variants"](../references/index.md#kicad10-pcbnew-manual); CLI reference | `pcb export pos --variant … --exclude-dnp` dropped that variant's DNP parts; `--variant` is accepted by `pcb export gerbers/pdf/step/ipc2581/odb/pos` and `sch export bom/pdf/netlist` |
| `${VARIANT}` and `${VARIANT_DESC}` text variables in title blocks and text items, resolved on the board too | Both manuals | A front-silkscreen text `…[${VARIANT}]…` plotted as `[Variant A]` with `--variant "Variant A"` and as `[]` with the default variant. The variant's silkscreen Gerber differed from the default one |
| ERC/DRC per variant | — | **Not available:** `sch erc` and `pcb drc` in 10.0.6 take no `--variant`. They check the base design, which contains every footprint of every variant |
| DNP and solder paste | — | **The paste layer is identical** for the default and a variant with DNP parts: variant exports don't remove paste from DNP pads |
| Jobsets per variant | KiCad source has a `variant` parameter on the export jobs (facts only) | Not tested; **(verify in #21)**, otherwise the release workflow calls `kicad-cli` directly |

The manual also says sheet files "can be shared between multiple projects",
but that this "is not recommended due to path portability concerns and the
risk of unintentionally changing other projects while editing a shared
sheet"; design blocks are the supported way to copy circuits between projects
([Schematic Editor manual](../references/index.md#kicad10-eeschema-manual),
"Hierarchical schematics", "Design blocks"). That weighs on option A.

### Size estimate

**These are rough estimates**, not a layout. They take the package sizes
stated in the inputs (module land pattern 15.4 × 20.5 mm per
[`fcc.md`](../compliance/fcc.md) §1.1, codec VQFN-32 5 × 5 mm, transformers
12.8 × 9.0 mm, LMR43620 2 × 2 mm, LM74800-Q1 WSON-12, SMB TVS, 1206/1210
passives) and allow for the passives around each part, 2-layer routing and
through-hole connector bodies. Board edge margins, mounting holes and the
antenna clearance add about 25 %.

| Block | Estimated area (mm²) |
|---|---|
| ESP32-S3-MINI-1 and its edge/antenna clearance | ≈ 500 |
| Codec, its two LDOs, level and filter networks | ≈ 250 |
| AUDIO transformers and their 0 Ω bypass footprints | ≈ 300 |
| SERIAL switching (MAX14778, TRS3221E, TCA9534, CI-V, ring-2 3.3 V output switch and limiter, protection) | ≈ 400 |
| Jack isolation (ISO7721, ISO1540, jack-domain supply, 0 Ω links, barrier) | ≈ 300 |
| PTT closure and gating (AQY212EH; TPS3839 supervisor if kept). The TPL5111 hardware timer and TPS3430 watchdog are removed from #9 (maintainer, 2026-09-25) and not counted | ≈ 80 |
| USB-C, hub with crystal, two switches, ESD | ≈ 300 |
| Radio-side connectors (USB-A, two 3.5 mm TRRS) | ≈ 450 |
| Shared regulator core and 2.304 MHz clock | ≈ 250 |
| **Shared core subtotal** | **≈ 2,830** |
| R input chain (ADR-0005) | ≈ 150 |
| M input chain (ADR-0004, with ≥ 0.5 mm clearance on > 50 V nets) | ≈ 450 |
| DC input connector (locking) | ≈ 100 |

| Build | Parts area | With ≈ 25 % margin | Square equivalent |
|---|---|---|---|
| A: R-only board | ≈ 3,080 | ≈ 3,850 | ≈ 62 × 62 mm |
| A: M-only board | ≈ 3,380 | ≈ 4,230 | ≈ 65 × 65 mm |
| **B: one board, both chains** | ≈ 3,530 | ≈ 4,410 | **≈ 66 × 66 mm** |
| C: core board + board-to-board connector | ≈ 2,910 | ≈ 3,640 | ≈ 60 × 60 mm, plus a daughterboard of ≈ 20 × 22 mm (R) or ≈ 25 × 33 mm (M) |

Every option stays well inside JLCPCB's ≤ 100 × 100 mm price bracket
(pcb-fabrication §1). Option B costs R about 4 mm more on each side than
a dedicated R board. The enclosure size target is still **TBD by the
maintainer** (constraints §11), so this can't yet fail it. Height is set by
the 7.5 mm transformers and the through-hole jacks in every option; option C
adds a stacking height or a second board footprint.

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **A. Separate board per variant, sharing the core sheets** | Smallest board per variant; R carries no automotive copper; revisions can move independently | Two layouts of an identical core: KiCad shares schematic sheets between projects only with a warning, and **not the layout**, so the EMC-critical core (buck hot loop, clock, antenna, USB) is laid out twice and can drift. Needs a new CI check that the shared sheets or copies stay identical. Two ERC/DRC runs, two design reviews, two EMC pre-scans of the core | [Schematic Editor manual](../references/index.md#kicad10-eeschema-manual) (sharing sheets, design blocks) |
| **B. One board with KiCad 10 design variants R and M** | One layout, one netlist, one ERC/DRC run and one review cover both builds. The core, clock and harmonic audit (ADR-0004) are measured once. Fitting per variant is what ADR-0002 and ADR-0003 already assume. Variant-aware BOM, position, Gerber and STEP exports and `${VARIANT}` on the silkscreen are native | R carries the unpopulated M chain (≈ +14 % area) and M the R chain. A copper change for one variant is a new revision for both. The paste layer isn't trimmed for DNP parts. ERC/DRC can't check per variant | [Schematic](../references/index.md#kicad10-eeschema-manual) and [PCB](../references/index.md#kicad10-pcbnew-manual) manuals, [CLI](../references/index.md#kicad10-cli-manual), [release notes](../references/index.md#kicad-10-0-0-release); local checks above |
| C. Core board plus power/connector daughterboards | Core laid out once; each input chain on its own small board | Two or three PCBs and an assembly step to join them per unit (not JLCPCB turnkey). A board-to-board connector at the power entry adds loop area and a vibration-sensitive joint on M (REQ-MECH-006). More height or footprint in the enclosure. Cross-board netlist not checked by KiCad; three CI projects; tags must name board combinations | ADR-0004 §12 layout guidance (input zones along one edge) |

### Comparison by criterion

| Criterion | A | B | C |
|---|---|---|---|
| Board size vs enclosure | Best per variant (≈ 62 / 65 mm square) | ≈ 66 mm square for both; within any plausible target (TBD) | Smallest core, but two boards to house |
| BOM cost | Same parts | Same parts; DNP parts cost nothing. 0 Ω links are cents. Slightly larger PCB, same price bracket | + connector pair, second PCB, joining labor |
| EMC | Core laid out twice; each pre-scan covers one board | Core laid out and measured once. Unused chain copper is isolated at both ends (Decision 5) | Extra connector at the power entry |
| Manufacturing | Two PCB designs, two assembly setups | One PCB design; per-variant BOM and CPL; the bare boards differ only in the silkscreen variant text | Several PCBs per unit, manual joining |
| CI (ERC/DRC, silkscreen) | Two projects; a new sheet-identity check | One project; ERC/DRC once over the superset; silkscreen check needs the variant-aware tag match (done in this PR) | Three projects; no cross-board check |
| Release and tags | One tag per board, as documented | Two tags per commit (`hw-R-…`, `hw-M-…`), exports with `--variant` | Tags must name a core + daughterboard pair |

## Decision

**Option B: one board with KiCad 10 design variants.**

1. **Variants in revision A:** exactly two design variants, named **`R`** and
   **`M`** (case-sensitive; the name is used in the tag, the silkscreen and the
   `--variant` exports). Suggested descriptions for `${VARIANT_DESC}`: "radio
   DC or USB-C powered" and "mobile 12 V".
   - The **default variant** (`<Default>`) is the design view with every
     footprint fitted. It is what ERC, DRC and the design review check. It is
     **never exported for fabrication or released**; its silkscreen prints an
     empty variant, which makes an accidental default build visible.
   - **U (future note only):** a smaller USB-host-only build is not in revision
     A. If it can be built by fitting the same board, it becomes a third design
     variant `U`. If it needs its own copper or outline, it is a separate board
     (`hardware/boards/interface-U/rev<X>/`, below) with one design variant `U`.

2. **Folder layout:**

   ```text
   hardware/boards/interface/revA/
     interface.kicad_pro      design variants R and M are defined in the schematic
     interface.kicad_sch      root sheet, plus one sheet per block (below)
     interface.kicad_pcb
     interface.kicad_dru, sym-lib-table, fp-lib-table, interface.kicad_jobset
   hardware/enclosure/R/      PETG enclosure for variant R
   hardware/enclosure/M/      ASA enclosure for variant M
   ```

   Suggested sheet split for #21, so that sheet-level DNP does the variant
   work: shared core sheets (module, USB, audio, SERIAL, PTT, isolation,
   regulator and clock), plus **`power_in_R`** and **`power_in_M`**. Each power
   sheet is DNP as a whole in the other variant. The isolation parts and their
   0 Ω links are marked per symbol.

3. **Fitting per variant:**

   | Block | R | M |
   |---|---|---|
   | Shared core (module, codec, hub, switches, SERIAL/AUDIO circuits, PTT, buck, clock) | Fitted | Fitted |
   | DC input connector (one footprint, Decision 5) | Fitted | Fitted |
   | `power_in_R`: 0.25 A fuse, B5819W, SMBJ15A, TPS2121, pi filter (ADR-0005) | Fitted | DNP |
   | `power_in_M`: 2 A fuse, TVS stack, C_in, LM74800-Q1, Q1/Q2, VS clamp, CMC, pi filter, 100 µF, TPS3710, SENSE/HOLD (ADR-0004) | DNP | Fitted |
   | Output link from each input chain to the shared buck input | R link fitted | M link fitted |
   | AUDIO transformers (ADR-0002) | DNP | Fitted |
   | 0 Ω audio bypass (ADR-0002) | Fitted | DNP |
   | ISO7721, ISO1540, jack-domain supply (ADR-0003) | DNP | Fitted |
   | 0 Ω links replacing them (ADR-0003) | Fitted | DNP |

   Field overrides are used only for parts that differ between variants but
   share a footprint (for example the USB-C receptacle, Decision 5). The core
   is never overridden.

4. **Tags:** `hw-<variant>-rev<X>-v<major>.<minor>[.<patch>][-<pre>]`, the
   form `tools/kicad_ci/check_silkscreen.py` accepts. The first releases are
   **`hw-R-revA-v1.0`** and **`hw-M-revA-v1.0`**. Release candidates are
   `hw-R-revA-v1.0-rc.1`.
   - `<variant>` is a design variant declared in the board, spelled exactly as
     in KiCad. `rev<X>` is the board folder and the title-block revision.
   - A **copper change** is always a new revision folder (`revB`), for both
     variants at once; its tags restart at `v1.0`.
   - A **BOM, fitting or value change** on the same copper bumps the minor
     version of the affected variant only (`hw-M-revA-v1.1`).
   - A **patch** (`v1.0.1`) regenerates outputs or notes with the same copper
     and BOM.
   - Both variants may be tagged on the same commit. Each release carries only
     its own variant's outputs, exported with `--variant <variant>`.

5. **Shared footprint area for the input chains:** the M-only chain **does not
   overlap** the R chain's footprints. The two topologies have different part
   counts, overlapping courtyards would need DRC exclusions, and the M chain's
   high-voltage clearance can't share pads with R. Instead both chains share
   **one power-entry strip along one board edge**, in the order ADR-0004 §12
   asks for:
   - **One DC input connector footprint for both variants:** locking,
     vibration-rated, 3 positions (DC+, ground, SENSE for M's ignition or
     radio-on input; unused on R), rated −40 to +85 °C. This covers REQ-MECH-006
     for M and ADR-0005's "locking 2-pin preferred" for R. Part and pinout: #21,
     with two sources.
   - **Each chain starts at its own fuse, placed at the connector.** The fuse is
     the variant selector, so the unused chain's input stub is only the pad
     spacing.
   - **Each chain ends in a 0 Ω output link** at the buck input capacitors, fitted
     only in its variant, so the unused chain's copper is disconnected at both
     ends.
   - The **buck, its hot loop and the clock** are shared and sit right after the
     join. Whether R's pi filter can share footprints with M's pi filter
     through value overrides is for #21, with the #10/#11 values.
   - The **USB-C receptacle** is one footprint. Prefer one part rated −40 to
     +85 °C for both variants; if it costs more, R can take the TYPE-C-31-M-12
     through a field override, provided the footprints match **(verify in #21)**.

6. **Silkscreen:** the front silkscreen shows the variant with the KiCad text
   variable **`${VARIANT}`**, next to `${REVISION}` and `${ISSUE_DATE}`, for
   example `open-bt-rig-interface ${VARIANT}` and `Rev ${REVISION} ${ISSUE_DATE}`
   (the back carries the designer credit and license mark as before). KiCad
   fills in `${VARIANT}` from the variant chosen at export, so the R and M
   Gerbers print "R" and "M". The silkscreen answers REQ-ISO-002's "how a user
   can tell which build they have": each fitting that changes isolation is its
   own variant name.
   - **CI change (made in this PR):** `check_silkscreen.py` now requires every
     board to declare at least one design variant with a tag-safe name
     (letters, digits, inner hyphens), requires `${VARIANT}` on the front
     silkscreen, and matches a `hw-<variant>-rev<X>-…` tag to a board whose
     folder is `rev<X>` and which **declares** that design variant. It no
     longer matches by folder name (`<variant>` or `*-<variant>`): with one
     board for two variants, the folder name can't carry the variant.

7. **Enclosures:** one enclosure design per variant, `hardware/enclosure/R/`
   (PETG, −20 to +60 °C) and `hardware/enclosure/M/` (ASA, −40 to +85 °C, vehicle
   mounting and strain relief for the locking connector). Both are built on the
   same board outline and connector positions; each uses its variant's STEP
   export (`kicad-cli pcb export step --variant <variant>`, which leaves out
   that variant's DNP parts) as the fit reference. Enclosure STL/3MF files are
   attached to the matching `hw-<variant>-…` release.

Why B: the maintainer fixed an identical core, and the variants differ only in
a small input chain and in isolation fitting that ADR-0002 and ADR-0003 already
drew as one layout. B is the only option where the EMC-critical core (buck hot
loop, 2.304 MHz clock, antenna clearance, USB) is laid out and measured once,
which is the project's main optimization goal. Its costs (≈ 14 % more area on
R, shared revisions, untrimmed paste) are small, and KiCad 10 supports every
output the release needs per variant.

## Consequences

- **#21 (KiCad setup)** creates one project, `hardware/boards/interface/revA/`,
  with design variants `R` and `M`, the sheet split and fitting table above,
  and the silkscreen texts of Decision 6. Variants are edited in the schematic
  only; the PCB receives them through "Update PCB from Schematic".
- **CI:**
  - Done in this PR: the variant-aware silkscreen and tag check, with tests.
  - `select_projects.py` and `run_checks.py` need no change: one project is
    one matrix entry, and ERC/DRC check the superset of every variant's parts.
  - Follow-up (release workflow, Phase 4): on a `hw-<variant>-…` tag, export
    BOM, position, Gerber/drill, PDF and STEP with `--variant <variant>` (or a
    per-variant jobset if 10.0.6 supports it, **verify in #21**), and never
    from the default variant.
  - **Verify in #21:** that `pcb drc --schematic-parity` reports no parity
    errors from variant data.
- **Manufacturing:** the paste layer keeps paste on DNP pads (checked on
  10.0.6). Leftover solder on unfitted pads is expected. Whether to trim the
  paste per variant, or rely on the assembler's stencil, is a #21 /
  manufacturing follow-up **(verify with JLCPCB)**. The bare R and M PCBs differ
  only in the silkscreen variant text, so they are ordered separately.
- **Revision coupling:** a copper fix needed by one variant starts a new
  revision for both. BOM-only changes stay per variant (minor version).
- **Temperature:** every shared part meets −40 to +85 °C (as ADR-0002 to
  ADR-0004 already require). The R-only input parts need only −20 to +60 °C,
  since they are never fitted on M.
- **EU (#59):** both variants share one layout, so pre-compliance data on the
  shared blocks applies to both. Whether R and M need separate technical files
  and declarations is for #59. Nothing here blocks later CE or ISED work
  (REQ-REG-006).
- **Docs updated in this PR:** `AGENTS.md`, `README.md`, `docs/roadmap.md`,
  `docs/developer-guide.md`, `constraints.md` §13, and the READMEs in
  `hardware/boards/` and `hardware/enclosure/`.

### Open questions for the maintainer (not decided here, except where marked closed)

1. **Variant R isolation in wired mode.** [Constraints §6](../requirements/constraints.md#6-safety-and-fail-safe)
   requires jack isolation "on every variant whenever the USB-C data link is
   used", but R's default fitting (ADR-0002, ADR-0003) leaves it out. Options
   carried from ADR-0002: amend §6, or fit isolation on R builds meant for wired
   use, or leave it here. This board strategy makes the second option a pure
   fitting choice: a third design variant (for example `R-ISO`, which the tag
   check accepts) with M's isolation fitting and R's power input, at no layout
   cost.
2. **ADuM4160 radio-USB isolator.** It needs 4.5–5.5 V on both sides, and the
   board has no 5 V rail (#9, radio-interface-circuits §8.6): a 3.3 → 5 V
   step-up plus a separate isolated 5 V for the radio side, on top of the
   isolator itself (about $7.40 at LCSC, quantity 100+, per that study). Decide whether revision A reserves footprints for it (they
   would take area on both variants) or accepts the ground path through the
   radio's USB cable on M.
3. **SERIAL-jack ring-2 3.3 V output: closed.** The maintainer decided to
   **keep it** (2026-09-25): about 20 mA, current-limited, off by default,
   recorded in ADR-0003 ([PR #61](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/61)).
   It is part of the shared core, fitted on both variants, and its switch,
   current limiter and Schottky are counted in the SERIAL block estimate.
4. Also noted: ADR-0004 left open whether M may run from USB-C alone (bench
   use). The TPS2121 can't simply be fitted on M: its 24 V absolute maximum is
   below M's 37–40 V cut-off on the protected rail.

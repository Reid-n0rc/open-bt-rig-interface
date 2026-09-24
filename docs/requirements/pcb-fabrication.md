<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# PCB fabrication and passive components

Issue: [#49](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/49).
These rules apply to every board revision. They target **JLCPCB's standard
(low-cost) PCB and assembly process**, so the design stays inside the cheapest
price tier. Where a rule here is stricter than the fab's minimum, the stricter
rule applies; it leaves margin for yield. Deviations need a note in the
schematic or layout and a reason in the PR.

JLCPCB capability figures below were read from its
[PCB capabilities page](https://jlcpcb.com/capabilities/pcb-capabilities) on
2026-09-24. Re-check them when #21 sets up the KiCad design rules.

## 1. Board

| Item | Project rule | JLCPCB standard capability |
|---|---|---|
| Layers | **2 layers preferred; 4 layers acceptable** (see §2) | 1–2 and 4 layers |
| Material / thickness | FR-4, **1.6 mm** | 0.4–2.0 mm |
| Copper | **1 oz** outer (and 0.5 oz inner on 4 layers, per the standard stackup) | 1 oz default; 2 oz costs more |
| Surface finish | **Lead-free HASL** (RoHS). ENIG only if an assembly-yield problem on fine-pitch pads calls for it | HASL (leaded / lead-free), ENIG, OSP |
| Solder mask / silkscreen | Green / white | Other colors may cost more |
| Size | As small as the electrical and enclosure requirements allow; keep ≤ 100 × 100 mm to stay in the lowest price bracket **(verify current pricing)** | Min 3 × 3 mm |
| Not used | Blind/buried vias, via-in-pad, castellated holes, edge plating, impedance control on 2 layers, V-cut panels | Available at extra cost |

## 2. Two layers or four

Start with **2 layers**. Move to 4 layers (JLCPCB's standard 4-layer stackup
**JLC04161H-7628**, 1.6 mm, which supports impedance-controlled 90 Ω
differential pairs) only when one of these can't be met on 2 layers, and record
the reason in the board's design notes:

1. **USB high-speed pairs.** The wired-mode USB hub (#9) runs at 480 Mbit/s
   between the USB-C port and the hub. A 90 Ω differential pair is impractical
   on a 1.6 mm 2-layer board. Options on 2 layers: keep the high-speed run very
   short (hub next to the USB-C connector), or use a thinner board (0.8–1.0 mm)
   so a 90 Ω pair is a practical width **(verify with JLCPCB's impedance
   calculator)**. Full-speed links (12 Mbit/s: the radio port and the
   ESP32-S3) don't need controlled impedance.
2. **EMC next to HF transmitters** ([constraints §5](constraints.md#5-rf-environment)).
   A solid ground reference under every signal is required. If the 2-layer
   routing breaks the bottom ground pour under the switching regulator, audio
   path or USB, go to 4 layers.
3. **Module integration.** Follow the ESP32-S3 hardware design guidelines for
   the ground under the module and the antenna keep-out, which applies to
   **all layers**.
4. **Routing density** that would otherwise need vias smaller than §3 allows.

## 3. Design rules

Project rules, with margin inside the JLCPCB standard minimums:

| Rule | Project value | JLCPCB minimum (1 oz) |
|---|---|---|
| Track width / clearance | **0.15 / 0.15 mm**; 0.127 mm allowed locally for fine-pitch escape | 2 layers 0.10 / 0.10 mm; 4 layers 0.09 / 0.09 mm |
| Via | **0.3 mm drill / 0.6 mm diameter** | Smaller vias are possible but cost more |
| Annular ring (PTH) | ≥ 0.15 mm | 0.18 mm recommended minimum (2 layers) |
| Pad-to-track clearance | ≥ 0.15 mm | 0.1 mm |
| Hole to hole (edge to edge) | ≥ 0.5 mm | 0.2 mm (vias), 0.45 mm (pads) |
| PTH / NPTH hole | ≥ 0.3 mm / ≥ 0.5 mm | 0.15 mm / 0.5 mm |
| Copper to board edge | **≥ 0.3 mm** (routed edge) | 0.2 mm |
| Solder-mask dam | ≥ 0.1 mm | 0.1 mm |
| Silkscreen line / text height | **≥ 0.15 mm / ≥ 1.0 mm** (applies to the required silkscreen text: name, variant, `${REVISION}`, date, designer, license) | 0.15 mm / 1.0 mm |

High-current and high-voltage nets (DC input, VBUS, the automotive front end)
get widths and clearances from their current and voltage, not from the
defaults above: size traces with an IPC-2221 calculator for ≤ 10 °C rise, and
keep ≥ 0.5 mm clearance on nets that can see more than 50 V (variant M input
before the surge stopper).

## 4. Assembly

- **Top-side assembly only**, JLCPCB standard PCBA.
- **Prefer JLCPCB Basic parts.** Extended parts add a per-part loading fee
  **(verify current fee)**, so use one only when no Basic part meets the
  electrical requirement, and consolidate values (for example one 100 nF part
  everywhere) to keep the number of unique extended parts low.
- Every part still needs two distributor sources and an active lifecycle
  ([constraints §12](constraints.md#12-manufacturing-and-sourcing)); a Basic
  part is not exempt.
- Prefer SMD connectors where mechanically sound; through-hole parts (jacks,
  USB-A, DC input) add hand or wave soldering cost. Keep through-hole parts
  on one side.

## 5. Resistors

- **Default: 0402, thick film, ±1 %** (JLCPCB Basic, 62.5 mW, 50 V working
  voltage; for example 10 kΩ `0402WGF1002TCE`, C25744).
- **Go larger only when needed**, and only as large as needed:
  - **Power:** dissipation ≤ 50 % of the rated power at 70 °C. 0402 ≤ 31 mW,
    0603 (100 mW) ≤ 50 mW, 0805 (125 mW) ≤ 62 mW, 1206 (250 mW) ≤ 125 mW.
  - **Voltage:** the working voltage across the resistor ≤ 50 % of its rated
    working voltage (0402 50 V, 0603 75 V, 0805 150 V, 1206 200 V for the Basic
    thick-film series). This matters on the variant M input, RS-232 lines and
    high-voltage dividers.
  - **Surge and pulse:** resistors that take transients (series resistors on
    the jacks, automotive input, ESD paths) use a pulse-withstanding
    (anti-surge) series in the size its pulse rating requires.
- Thin film or ±0.1 % only where accuracy needs it (for example audio gain or
  reference dividers); state why in the schematic.

## 6. Capacitors

### 6.1 Type

- **MLCC by default.** Other types only where an MLCC can't do the job, with a
  reason in the schematic:
  - bulk or damping capacitance at a hot-plugged DC input (a ceramic-only input
    on a long cable can ring to about twice the supply at plug-in; add an
    electrolytic or an RC damper, or show it isn't needed);
  - values or voltages no reasonably sized MLCC reaches.
- **Case size: the cheapest that meets the electrical requirement**, including
  the effective capacitance after DC bias (§6.3). Small cases lose the most
  capacitance under bias, so the cheapest *adequate* part is often one size up.

### 6.2 Dielectric

| Use | Dielectric |
|---|---|
| Decoupling, bulk, charge pumps, regulator input/output | X7R preferred; X5R allowed (rated to +85 °C, the variant M maximum ambient, so prefer X7R there) |
| Audio signal path filters, crystal load caps, timing, RF matching | **C0G/NP0** (no DC-bias or voltage-coefficient distortion) |
| Audio coupling capacitors too large for C0G | X7R, rated well above the signal and bias voltage; keep AC swing small relative to the rating |
| Never | Y5V, Z5U |

Class II ceramics (X7R/X5R) are also microphonic. Keep them out of the
high-gain audio input path where a C0G or a different topology works.

### 6.3 Voltage rating and DC bias

**Rule: a DC capacitor's rated voltage is at least 2× the nominal operating
voltage of its node, and never below the node's worst-case transient.**
Then check the **effective capacitance at the operating DC bias and
temperature** against the value the circuit needs, using the manufacturer's
DC-bias curve (for example Samsung's or Murata's characterization data). The
2× rule reduces DC-bias loss but doesn't remove it.

| Node (nominal) | Worst case to consider | Minimum rating | Typical choice (JLCPCB Basic, 2026-09-24) |
|---|---|---|---|
| 3.3 V logic / module / codec | Regulator overshoot | **10 V** (2 × 3.3 = 6.6 V) | 100 nF 16 V X7R 0402 (C1525); 10 µF 10 V X5R 0603 (C19702). **Not** 10 µF 6.3 V 0402: below 2×. |
| 1.8 V or lower core rails | — | 6.3 V | 100 nF 16 V X7R 0402 (C1525) |
| 5 V (USB VBUS, VBUS to the radio) | USB hot-plug ringing | **16 V** (2 × 5 = 10 V; 16 V at connectors for the ringing) | 100 nF 50 V X7R 0402 (C307331); 10 µF 25 V X5R 0603/0805 (C96446, C15850) |
| RS-232 charge pump (about ±5.5 V) | Datasheet recommendation | 16 V, or the datasheet's value if higher | 100 nF 50 V X7R 0402 (C307331) |
| Analog-switch supplies (up to ±15 V) | Supply tolerance | **35 V** (use 50 V) | 100 nF 50 V X7R 0402 (C307331); 1 µF 50 V X7R 0805 (C28323) |
| Radio accessory DC (13.8 V, 11–15 V) | TVS clamp voltage | **50 V** (2 × 13.8 = 27.6 V, plus clamp) | 100 nF 50 V X7R 0402/0603; 1 µF 50 V X7R 0805 (C28323); 10 µF 50 V X5R 1206 (C13585) |
| Automotive 12 V, after the surge stopper (variant M) | Stopper's clamped output | **50 V** or 2× the clamped output, whichever is higher **(verify in #10)** | As above |
| Automotive 12 V, before the surge stopper | Suppressed load dump ≈ 35 V, jump start 24 V, pulses per [constraints §3.2](constraints.md#32-automotive-12-v-input-variant-m) | **100 V** | 100 nF 100 V X7R 0805 (C28233); soft-termination parts where the board can flex **(verify availability)** |

Notes:

- Rails not in the table follow the same rule: 2× nominal, rounded up to the
  next standard rating (6.3, 10, 16, 25, 35, 50, 100 V), and at least the
  worst-case transient.
- For AC-coupled nodes, the rating applies to the DC bias plus the peak signal.
- Large MLCCs (1206 and up) crack under board flex: keep them away from board
  edges, mounting holes and connectors, and orient them parallel to the
  nearest edge.

## 7. Where these rules are applied

- KiCad board setup and custom design rules (`.kicad_dru`) for every board are
  created from §1 and §3 in #21, through Konnect.
- Schematic reviews check §5 and §6 for every resistor and capacitor; the
  design review (Konnect review workflow) checks the whole document.

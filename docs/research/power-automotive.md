<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Variant M: automotive 12 V power front end

Issue: [#10](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/10).
Researched 2026-09-24. Decision record:
[ADR-0004](../decisions/ADR-0004-power-automotive.md) (proposed).

Scope: the power input of variant M, from the vehicle 12 V lead to the 5 V and
3.3 V rails, including transient protection, EMI filtering, regulation,
brownout handling and power-down. Variant R power is #11; schematic and layout
come later. Requirements are cited by
[`constraints.md`](../requirements/constraints.md) section and by
[`requirements.md`](../requirements/requirements.md) ID: REQ-PWR-003, -005,
-010 to -017, REQ-EMC-004 to -007 and REQ-ENV-002. The design follows
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md) (ESP32-S3-MINI-1,
Bluetooth LE and wired USB-C, USB hub).

**The device supplies no power to the radio.** The radio port's USB VBUS is
blocked in hardware (maintainer decision 2026-09-24; circuit in ADR-0003,
[#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9)). The device
only takes power in and never sources power out of any port; USB-C is a sink.

Values marked **(verify)** are not confirmed by a primary source or need a
bench test. Calculations are reproducible from the numbers given here.

## Summary

- **Test levels confirmed** from app notes that reproduce ISO 16750-2:2023 and
  ISO 7637-2:2011, plus the official ISO previews for editions and clause
  numbers (§1). One target changes: the **jump start is 26 V for 60 s** in
  ISO 16750-2:2023 (24 V in the 2012 edition).
- **Protection:** 2 A 63 V fuse → anti-series TVS stack (TPSMB82A +
  SMBJ33CA-HE3, breakdown ≥ 107 V at −40 °C) → **LM74800-Q1** ideal-diode
  controller with back-to-back N-MOSFETs in **common-source** topology (150 V
  blocking FET), with overvoltage **cut-off at about 38.5 V**. The TVS absorbs
  no load-dump energy; unsuppressed load dump (101 V) is blocked by the FET (§4).
- **Filter:** Murata DLW5BTM common-mode choke + 2.2 µH / 2.2 µF pi filter,
  corner about 78 kHz, about 59 dB ideal attenuation at the switching
  frequency (§5).
- **Regulation (single stage):** one **LMR43620MC3RPERQ1** converts the
  protected input straight to 3.3 V, in FPWM, **synchronized to 2.304 MHz**
  (an 18.432 MHz oscillator divided by 8) so that no harmonic from 160 m to
  10 m lands inside a US amateur band (worst margin 88 kHz; §6). There is no
  5 V rail. Sync holds up to 19.1 V input (tON-MIN 75 ns max) and down to
  about 4.2 V (tOFF-MIN), so it covers the 4.5 V cold crank.
- **Cold crank:** no buck-boost. The 3.3 V rail stays regulated down to about
  3.5 V input; a hardware brownout detector forces PTT off at 7.0 V; hold-up
  is ≥ 747 µs from 7.0 V at worst-case load (§7, §8).
- **Power-down:** ignition / radio-on sense, USB-C host and a firmware hold
  line drive the LM74800-Q1 enable; auto power-down 30 s (configurable) after
  the radio or ignition turns off. **Off-state drain ≤ 7 µA** at 25 °C
  (limit 1 mA; §9).
- **Power budget:** about 1.1 W typical and **2.1 W worst case** input, inside
  the 1.5 W typical / 3 W peak target of constraints §3.4 (REQ-PWR-003) (§2).
- **Cost:** power-section main parts about **$10.95** at LCSC (qty 100), down
  from about $23.56 for the two-buck version (§10). AEC-Q parts are kept only
  in the input protection chain and where they cost about the same.
- **EU:** all chosen parts are RoHS per the distributor listings. The
  protection and filter also cover EN 301 489-1's vehicle clauses (§11a,
  [#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59)).

## 1. Test levels

### 1.1 Sources and editions

The standards were not read in full. Levels come from manufacturer app notes
that reproduce them; editions and clause numbers come from the official
previews. Where two sources disagree, both are shown.

- **ISO 16750-2:2023** is the **fifth edition** (2023-07). It cancels and
  replaces the **fourth edition, ISO 16750-2:2012** (foreword,
  [official preview](../references/index.md#iso-16750-2-2023-preview)).
  Earlier editions: 2003, 2006, 2010 ([Volta seminar](../references/index.md#volta-iso-16750-2-seminar)).
  So "2010 vs 2023" in the issue is really 2010 (3rd) → 2012 (4th) → 2023 (5th).
- Clauses used here (2023 contents): 4.2 DC supply voltage, 4.3.1 long-term
  overvoltage (4.3.1.2 jump start), 4.3.2 transient overvoltage, 4.6.3 starting
  profile, 4.6.4 load dump, 4.7 reversed voltage.
- **ISO 7637-2:2011** is the **third edition**; it replaces ISO 7637-2:2004
  and its 2008 amendment, and **no longer specifies pulses 4, 5a and 5b**,
  "which are now specified in ISO 16750-2 and ISO 21848" (foreword,
  [official preview](../references/index.md#iso-7637-2-2011-preview)). Pulse
  5a/5b in the issue therefore means ISO 16750-2 load dump test A/B
  ([TI SNOAAA1](../references/index.md#ti-snoaaa1)).
- Level sources: [Microchip DS00006186A](../references/index.md#mchp-ds00006186)
  (Sept 2025, cites ISO 16750-2:2023 and ISO 7637-2:2011);
  [onsemi TND6424](../references/index.md#onsemi-tnd6424) (April 2023, ISO
  7637-2:2011 Annex A levels); cross-checked with
  [Nexperia AN50007](../references/index.md#nexperia-an50007),
  [TI SNOAAA1](../references/index.md#ti-snoaaa1),
  [TI SNOAA13](../references/index.md#ti-snoaa13),
  [Littelfuse load dump note](../references/index.md#littelfuse-tvs-load-dump-an)
  and the [Volta seminar slides](../references/index.md#volta-iso-16750-2-seminar)
  (test-equipment distributor, Italian).

### 1.2 Design levels (12 V system)

| Condition | Level designed for | Standard, clause | Source | vs constraints §3.2 |
|---|---|---|---|---|
| Supply range | Code C: 9–16 V (codes A 6–16, B 8–16, D 10.5–16 V) | ISO 16750-2:2023, 4.2, Table 3 | ISO preview | Matches 9–16 V |
| Test voltage UA | 14 V (ISO 16750-2); 13.5 ± 0.5 V (ISO 7637-2) | 16750-2 via Microchip Table 1-3; 7637-2:2011, 4.2, Table 1 | Microchip, ISO preview | — |
| Long-term overvoltage | 18 V for 60 min at elevated temperature | 16750-2:2023, 4.3.1.1 | Volta **(verify)** | Not listed; add |
| **Jump start** | **26 V for 60 s**, rise/fall ≤ 10 ms, functional status A | 16750-2:2023, 4.3.1.2 (Table 5) | Volta; Microchip Fig. 1 shows 26 V | **Was 24 V** (TI SNOAA13, written in 2019 under the 2012 edition, uses 24 V, 60 s) |
| Transient overvoltage (new in 2023) | 18 V for 400 ms, 1 ms edges, 5 times | 16750-2:2023, 4.3.2 (Table 6) | Volta **(verify)** | Not listed; add |
| Cold crank, "normal" | US1 = 4.5 V for 15 ms, then US = 6.5 V for 10 s | 16750-2:2023, 4.6.3 | Microchip Table 2-1 | Matches "about 6 V, 4.5 V desirable" |
| Cold crank, "severe" | US1 = 3 V, US = 5 V for 1 s | 16750-2:2023, 4.6.3 | Microchip Table 2-1 | Below target; brown out safely |
| Warm crank | US1 = 8 V, US = 10.5 V | 16750-2:2023, 4.6.3 | Microchip Table 2-2 | — |
| **Load dump test A** (unsuppressed, old 5a) | Us 79–101 V, Ri 0.5–4 Ω, td 40–400 ms, tr 10 (−5) ms, **10 pulses at 1 min** | 16750-2, 4.6.4 | Microchip Table 2-4; TI SNOAAA1; Littelfuse | Matches |
| **Load dump test B** (centrally suppressed, old 5b) | US* 27 / 30 / 32 / **35 V** (severity 1–4), same Ri and td, 5 pulses | 16750-2, 4.6.4 | Microchip Table 2-5 | Matches (35 V) |
| Reversed voltage | −14 V for 60 s | 16750-2, 4.7 | TI SNOAA13 (UA −14 V, Ri 0.01 Ω, 60 s); Microchip Fig. 1 | Matches. 2023 test cases 1/2 **(verify)** |
| Pulse 1 | **−150 V** (level IV; level III −112 V), Ri 10 Ω, td 2 ms, 500 pulses, 0.5 s | ISO 7637-2:2011, Annex A, Table A.1 | onsemi TND6424; Nexperia (−75 to −150 V, 10 Ω, 2 ms) | Matches |
| Pulse 2a | **+112 V** (III +55 V), Ri 2 Ω, td 50 µs, 500 pulses | 7637-2:2011, Table A.1 | onsemi | Matches |
| Pulse 2b | +10 V, 10 pulses | 7637-2:2011, Table A.1 | onsemi | — (covered by 26 V jump-start rating) |
| Pulse 3a / 3b | **−220 V / +150 V** (III −165 / +112 V), Ri 50 Ω, 150 ns bursts, 1 h | 7637-2:2011, Table A.1 | onsemi | Matches |
| ESD | ±15 kV air, ISO 10605 | ISO 10605 | Not verified | Keep as target **(verify)** |

Pairing rule for test A: unless agreed otherwise, the upper voltage goes with
the upper Ri and the lower voltage with the lower Ri
([Microchip](../references/index.md#mchp-ds00006186), note to Table 2-4). The
worst corners are therefore 101 V / 4 Ω and 79 V / 0.5 Ω.

### 1.3 Edition differences that matter here

- **ISO 7637-2:2004 → ISO 16750-2:2010:** load dump moved to ISO 16750-2. The
  old pulse 5 was a single pulse with Us 65–87 V; since 2010 it is 10 pulses at
  1-minute intervals with Us 79–101 V, and US* for test B is defined rather than
  left to the user ([Littelfuse](../references/index.md#littelfuse-tvs-load-dump-an), Table 1).
  A TVS sized for the old single pulse is undersized.
- **ISO 16750-2:2012 → 2023** (foreword list and Volta slides): operating
  modes introduced for all electrical tests; jump start test redefined (26 V,
  at room temperature and Tmin, functional status A); new transient overvoltage
  test (18 V, 400 ms); micro-interruptions added to supply drops; starting
  profile severity levels explained (warm crank, cold crank good battery, cold
  crank aged battery); reversed voltage test specified in more detail. Load
  dump test A/B parameters are the same as in the 2012 edition as reproduced by
  the sources above; the 2023 edition adds a more detailed description of the
  load dump origin (Annex B).
- **Verify-first result:** the levels in constraints §3.2 are confirmed except
  the jump start (26 V, not 24 V; REQ-PWR-014 still says 24 V). ESD (ISO 10605)
  was not checked.

## 2. Loads and power budget

Placeholders are marked with the issue that settles them. Every load runs
from 3.3 V; variant M has no 5 V rail.

| Load (3.3 V) | Typical | Worst case | Basis |
|---|---|---|---|
| ESP32-S3-MINI-1 | 0.10 A | **0.34 A** (BLE TX +20 dBm, 100 % duty) | [datasheet](../references/index.md#esp32s3-mini1-ds) Table 6-5; typical per constraints §3.4 (#7) |
| USB hub (USB2422), wired mode | 0 (Bluetooth mode) | 0.05 A | Placeholder (#9) |
| Audio codec incl. its core/analog regulators | 0.05 A | 0.05 A | Placeholder (#8) |
| Oscillator and dividers, supervisor, USB switches, LEDs, PTT PhotoMOS LED, isolator side 1 | 0.02 A | 0.02 A | Allowance; oscillator ≤ 5 mA ([YXC](../references/index.md#yxc-yso110tr-ds)) |
| Isolated jack-side supply (RS-232, analog switches, isolator side 2), from 3.3 V | 0.11 A | 0.11 A | Placeholder: 0.25 W out at 70 % (#9) |
| **3.3 V total** | **0.28 A (0.92 W)** | **0.57 A (1.88 W)** | |
| **12 V input** (buck at 89.5 % typ / 92 % worst, plus 30 mW protection and filter) | **≈ 1.1 W** | **≈ 2.1 W** | Efficiency from the datasheet curve (§6.4) **(verify)** |

- **No radio load.** The radio port's VBUS is blocked (ADR-0003, #9), so no
  VBUS switch or radio current appears here. The radio USB isolator
  (ADuM4160 fitting option, constraints §6) needs only a small radio-side
  supply, which belongs to the isolated supply (#9).
- **The isolated supply runs from 3.3 V.** An SN6505B-class driver accepts
  2.25–5.5 V ([datasheet](../references/index.md#ti-sn6505-q1-ds)); #9 owns
  the details.
- Input current at 2.1 W: 0.15 A at 13.5 V, 0.23 A at 9 V, 0.46 A at 4.5 V.
- Inside constraints §3.4 / REQ-PWR-003 (about 1.5 W typical, 3 W peak).
- **USB-C in variant M** is a sink-only data link (wired mode). The device
  never sources power on it. In variant M its VBUS is used to detect a host and
  as a wake source (§9). Whether variant M can also run from USB-C VBUS alone
  (for bench use) belongs to #11's source-mux design.

## 3. Architecture

```text
12 V lead ─ F1 ─┬─ D3+D4 TVS stack ─┬─ Q1 (150 V) ═╤═ Q2 (100 V) ─ CMC ─ C1 ─ L1 ─┬─ C2 + C_bulk ─┬─ U2 LMR43620MC3 ─ 3.3 V ─┬─ ESP32-S3, hub, codec (#8)
  (2 A)         │  C_in 100 nF 250 V│  HGATE        │  DGATE (ideal diode)           │  (hold-up)    │  (single stage, FPWM)     └─ isolated supply (jack side, #9)
                │                   └── LM74800-Q1 (common source, OV cut-off 38.5 V) │              │
IGN / radio-on ─┴─ sense network ────── EN/UVLO ◄── USB-C VBUS, firmware HOLD       │              │
                                                                                    TPS3710 brownout (7.0 V) ─► PTT enable gate
                              18.432 MHz oscillator ─► ÷8 (3 × SN74LVC1G80) ─► 2.304 MHz ─► MODE/SYNC of U2
```

## 4. Protection chain

### 4.1 Fuse

- **Littelfuse 0466002.NRHF** (466 series, 1206, 2 A fast-acting, **63 V**
  rating, 50 A interrupting at 63 V per its table;
  [datasheet](../references/index.md#littelfuse-0466-ds)). Not automotive
  qualified; at $0.055 it is a third of the AEC-Q200 437A
  ([0437002.WRA](../references/index.md#littelfuse-437-ds), $0.16), and a
  fuse's job here (clearing a board fault) doesn't depend on AEC-Q.
  63 V covers the jump start (26 V) with more than 2× margin.
- 2 A against a worst-case input of 0.46 A at 4.5 V; temperature rerating at
  85 °C per the 466 curve **(verify)**.
- The harness also carries an in-line blade fuse at the battery or fuse-box
  tap (user documentation). It protects the wire; F1 protects the board.
- PTC fuses were not chosen: automotive PTCs are rated well below the 101 V
  load dump and trip slowly.

### 4.2 Reverse polarity: options

| Option | For | Against | Source |
|---|---|---|---|
| Series Schottky | Cheapest | 0.3–0.5 V drop hurts cold crank; dissipation | — |
| P-channel MOSFET | Cheap, simple | Drop and gate clamping at high Vin; no reverse-current blocking on drops | — |
| **LM74700-Q1** ideal diode + N-FET | 20 mV drop, fast reverse blocking (< 0.75 µs), 1 µA shutdown, 80 µA on | No overvoltage cut-off; 65 V max, needs a load-dump solution beside it | [datasheet](../references/index.md#ti-lm74700-q1-ds) |
| LM74502-Q1 + back-to-back N-FETs | Reverse polarity + OV cut-off, 1 µA shutdown, 45 µA on | **No reverse-current blocking** (datasheet), so hold-up capacitance would drain back into a dipping input | [datasheet](../references/index.md#ti-lm74502-q1-ds) |
| LM749x0-Q1 | Ideal diode + OV + current sense, 6 µA sleep | 24-pin QFN, more than needed | [datasheet](../references/index.md#ti-lm749x0-q1-ds) |
| **LM74800-Q1** + back-to-back N-FETs | Ideal diode (10.5 mV regulation, −4.5 mV reverse threshold, 0.5 µs) + OV cut-off; 2.87 µA typ / 5 µA max shutdown; **common-source topology survives unsuppressed load dump** with a clamped VS pin | 12-pin WSON; OV cut-off only (no clamp mode) | [datasheet](../references/index.md#ti-lm7480-q1-ds) §10.3 |

### 4.3 Load dump and surge: options

| Option | For | Against | Source |
|---|---|---|---|
| TVS only, SMBJ/SMCJ class | Cheap, passive | Test A energy is **47–173 J per pulse** (table below) against about 0.87 J for a 600 W SMBJ; fails test A. Clamp of about 45–53 V also exceeds the buck's 42 V absolute maximum | [Vishay SMBJ](../references/index.md#vishay-smbj-ds) |
| TVS only, load-dump class (SM8S / SLD8S) | Passive, survives test A at higher Ri | Littelfuse SLD33-018 handles 38.4 A for 10 × 400 ms pulses; test A needs 58 A at 79 V / 0.5 Ω, so **two in series** are needed; clamp 50 V still exceeds 42 V; large DO-218 package | [Littelfuse](../references/index.md#littelfuse-tvs-load-dump-an) Tables 3–4 |
| Surge stopper, linear clamp (LTC4380, LM5060-Q1 with external clamp) | Keeps running through test A | MOSFET dissipates (Vin − Vclamp) × I for up to 400 ms × 10 pulses; SOA design; LTC4380 facts **not verified** (analog.com blocked scripted access) | [LTC4380](../references/index.md#adi-ltc4380-ds), [LM5060](../references/index.md#ti-lm5060-ds) (5.5–65 V, < 15 µA disabled) |
| **LM74800-Q1 common source, OV cut-off** | Q1 simply turns off: **no energy absorbed** in FET or TVS during test A; one IC also does reverse polarity and reverse-current blocking | Device browns out for the length of a test A pulse (functional status C); needs a 150 V FET and a VS clamp | [LM7480-Q1](../references/index.md#ti-lm7480-q1-ds) §10.3 (TI's 200 V / 24 V example, scaled to 12 V here) |

**TVS energy per test A pulse** (exponential decay from Us to UA = 14 V with
τ = td / ln 10, td = 400 ms, clamp at Vc; E = Vc · ∫(V − Vc) dt / Ri):

| Clamp | 101 V, 4 Ω | 79 V, 0.5 Ω |
|---|---|---|
| SMBJ33CA class, Vc ≈ 45 V | Ipk 14.0 A, **46.9 J** | Ipk 68.0 A, **172.7 J** |
| SLD33-018 class, Vc = 50 V | Ipk 12.8 A, 41.8 J | Ipk 58.0 A, 134.3 J (> 38.4 A rating) |
| **Chosen stack, VBR ≥ 106.9 V** | **0 J** (no conduction) | **0 J** |

Test B (US* ≤ 35 V) gives **0 J** in the chosen stack as well.

### 4.4 Chosen chain and component stress

- **LM74800-Q1** ([datasheet](../references/index.md#ti-lm7480-q1-ds)), AEC-Q100
  grade 1 (−40 to +125 °C ambient), 3–65 V operating, A pin to −65 V (absolute maximum). Common-source
  topology per its §10.3: **Q1** (HGATE, input side) blocks overvoltage,
  **Q2** (DGATE) is the ideal diode.
- **VS clamp:** 10 kΩ (1206, pulse-rated) + 56 V zener (AEC-Q101, SMA) keep VS
  within its 65 V operating limit. At 101 V: resistor 0.20 W, zener 0.25 W for
  up to 400 ms. TI's example uses a 60 V zener; a 62 V BZG03C62 is 58–66 V,
  above the 65 V operating limit, hence 56 V **(verify part)**.
- **OV cut-off at 38.5 V nominal:** threshold 1.195–1.267 V ±1 % resistors gives
  about 37.0–40.0 V. That passes test B (35 V) and the jump start (26 V) and
  stays under the buck's 42 V absolute maximum. Turn-off deglitch is 3.98–5.4 µs.
- **Q1: onsemi FDN86246**, 150 V BVDSS, 261 mΩ max at 10 V, ±20 V VGS,
  IDSS ≤ 1 µA at 120 V, TJ −55 to +150 °C, SOT-23
  ([datasheet](../references/index.md#onsemi-fdn86246-ds)). Worst VDS: pulse
  2a/3b input, clamped by the TVS stack at ≤ about 127 V (sum of VBR max) plus
  dynamic rise; ≥ 15 % margin. In this topology Q1 only switches and never
  absorbs load-dump energy, so SOA isn't involved and a commercial part is
  enough; it replaces a $5.91 AEC-Q101 PowerDI part
  ([DMTH15H017SPSWQ](../references/index.md#diodes-dmth15h017spswq-ds)).
- **Q2: Infineon IRLML0100**, 100 V, 220 mΩ at 10 V, ±16 V VGS, TJ −55 to
  +150 °C, SOT-23 ([datasheet](../references/index.md#infineon-irlml0100-ds)).
  During pulse 1 it blocks the hold-up voltage plus the negative clamp:
  16 V + 44 V + TVS forward drop ≈ 64 V, under 100 V. The LM74800-Q1 charge
  pump turns off at 14.1 V max (VCAP − VS), inside ±16 V VGS.
- **FET dissipation:** (0.26 + 0.22) Ω × 0.46 A² ≈ 0.10 W at 4.5 V input
  (cold crank, short), 11 mW at 13.5 V.
- **TVS stack** (anti-series, battery to ground):
  - **D3: Littelfuse TPSMB82A**, unidirectional, VRWM 70.1 V, VBR 77.9 V min,
    Vc 113 V at 5.4 A, αT 0.105 %/°C, AEC-Q101, TJ −65 to +175 °C
    ([datasheet](../references/index.md#littelfuse-tpsmb-ds)).
  - **D4: Vishay SMBJ33CA-HE3**, bidirectional, VRWM 33 V, VBR 36.7 V min,
    Vc 53.3 V at 11.3 A, αT 0.100 %/°C, AEC-Q101, TJ −55 to +150 °C
    ([datasheet](../references/index.md#vishay-smbj-ds)).
  - Positive breakdown = VBR(D3) + VBR(D4) ≥ 114.6 V at 25 °C and
    **≥ 106.9 V at −40 °C** (VBR(T) = VBR(25 °C)·(1 + αT·(T − 25))): above
    test A's 101 V by 5.9 V.
  - Negative breakdown = VBR(D4) + VF(D3): above the −14 V reverse battery, so
    no conduction for 60 s. It clamps pulse 1 at about −44 V; TI measured
    SMBJ33CA at −44 V with 12 A ([LM7480-Q1](../references/index.md#ti-lm7480-q1-ds) §11.2),
    within the LM74800-Q1's −65 V.
- **C_in: 100 nF 250 V X7R 1206, soft termination** (YAGEO AS1206KKX7RYBB104,
  AEC-Q200; [datasheet](../references/index.md#yageo-as-ds)) at the
  connector, before Q1. The soft termination is kept: a flex crack in this
  capacitor would short the battery lead at the connector, and it costs
  $0.075 more than a plain 1206.
- **Where AEC-Q is kept in the chain:** the TVS pair (D3, D4), which takes
  every transient (the HE3 D4 costs the same as the commercial SMBJ33CA:
  $0.137 vs $0.131; D3 TPSMB82A is $0.075 more than the electrically
  equivalent commercial SMBJ70A), C_in (above), and U1 (the LM74800-Q1 is the
  only stocked version). Everything else in the chain is commercial, rated
  −40 °C or lower to at least +85 °C.

### 4.5 Pulse by pulse

| Pulse | What happens | Energy / stress | Expected status |
|---|---|---|---|
| Pulse 1, −150 V, 10 Ω, 2 ms | D4 clamps at ≈ −44 V; Q2 blocks within 0.5 µs; hold-up keeps the rails | Ipk 10.6 A, **0.20 J per pulse** vs ≈ 0.87 J for a 600 W 10/1000 µs rating; 500 pulses at 0.5 s = 0.40 W average vs 5 W PD (SMBJ) | A |
| Pulse 2a, +112 V, 2 Ω, 50 µs | Stack barely conducts (2.6 A, 0.2 mJ at −40 °C); OV cut-off opens Q1 within ≈ 5 µs | Output rise during the deglitch ≈ 2.6 V into the hold-up capacitance | A |
| Pulse 3a / 3b, −220 / +150 V, 50 Ω, 150 ns | C_in absorbs: +150 V into 100 nF raises the node by 4.4 V; D4 takes 3a (3.6 A, 6 µJ) | Negligible | A |
| Transient 18 V, 400 ms; long-term 18 V | Below OV cut-off; U2 on-time 79.6 ns ≥ 75 ns, so it stays synchronized | — | A |
| Jump start 26 V, 60 s | Below OV cut-off; U2 on-time would be 55 ns, so it folds back to about 1.7–1.95 MHz for the event (§6.3) | Harmonics move for up to 60 s | A |
| Test B, 35 V, ≤ 400 ms | Below OV cut-off; the stack doesn't conduct; U2 folds back to about 1.3–1.45 MHz | 0 J | A |
| Test A, ≤ 101 V, ≤ 400 ms, 10 × | Q1 opens at 37–40 V; hold-up carries ≈ 3 ms, then brownout forces PTT off and the device resets | 0 J in TVS and FETs; VS clamp 0.45 W total for ≤ 400 ms | C |
| Reverse −14 V, 60 s | Q1 body diode conducts, Q2 blocks 14 V | LM74800-Q1 reverse leakage 19 µA typ | A (off) |
| ESD ±15 kV air (330 pF) | 4.95 µC into C_in raises it ≈ 50 V; the stack clamps | **(verify)** with ISO 10605 network | A |

## 5. Input filter (CMC + pi) for CISPR 25

Target: CISPR 25 Class 3 or better (constraints §3.2); designed against
**Class 5**, the strictest: conducted-voltage limits of 70/57/50 dBµV (LW,
PK/QP/AVG), **54/41/34 dBµV (MW 0.53–1.8 MHz)**, 53/40/33 dBµV (SW 5.9–6.2 MHz),
44/31/24 dBµV (CB 26–28 MHz and VHF 30–54 MHz), 38/25/18 dBµV (VHF
68–87 MHz, FM 76–108 MHz) ([TI SLYY136](../references/index.md#ti-slyy136),
Figure 1, from CISPR 25:2016).

2.304 MHz itself falls between CISPR 25 bands, but it is the largest
component, and the vehicle's 12 V wiring also feeds the radio. It is
therefore filtered as if the MW limit applied.

**Differential-mode estimate** (re-checked for the single stage: U2 at
0.57 A, 14 V → 3.3 V, D = 0.236):

- Fundamental of the input current pulse train:
  I1 = (2/π)·Iout·sin(πD) = **0.24 A peak** (the same as the two-stage
  design, because the lower duty cycle offsets the higher output current).
- Buck-side input capacitance 2 × 2.2 µF 100 V (≈ 1.9 µF each at 14 V bias),
  ESR ≈ 5 mΩ: |Z| = 18.9 mΩ → V1 = 4.6 mV peak = **70.2 dBµV rms**.
- Target 28 dBµV (Class 5 MW average 34 dBµV − 6 dB margin): **≥ 42.2 dB**
  of attenuation needed.
- **L1 = 2.2 µH** (TDK TFM252012ALMA2R2MTAA, 2.6 A, 75 mΩ, −55 to +150 °C;
  [datasheet](../references/index.md#tdk-tfm252012alma-ds)); the same part is
  the buck inductor (§6.3). XL = 31.8 Ω at 2.304 MHz. **C1 = 2.2 µF 100 V X7R
  1210** (PSA FS32X225K101EGG; [datasheet](../references/index.md#psa-fs32-ds)),
  XC = 36 mΩ: **≈ 59 dB** ideal, about 17 dB over the need before parasitics.
- **Corner frequency:** f0 = 1 / (2π√(L1·C1)) = **78 kHz** (L1 with C1), or
  55 kHz with the buck-side 3.8 µF; both are ≥ 30× below fsw.
- **Damping and stability:** filter Z0 = √(L1 / C_buck) = 0.76 Ω. The
  **100 µF 50 V electrolytic** (Huawei VD1H101MF105000CE0, −55 to +105 °C,
  5000 h at 105 °C; [datasheet](../references/index.md#huawei-vd-ds)) at the
  buck side damps the resonance with its ESR and doubles as hold-up (§8).
  The buck's negative input resistance, Vin² / Pin, is 39.2 Ω at 9 V and
  9.8 Ω at 4.5 V, well above Z0 (Middlebrook criterion met).
- **Common mode:** **Murata DLW5BTM142TQ2L**, 1400 Ω at 100 MHz, 2 A, 56 mΩ,
  100 V rated, −40 to +105 °C
  ([datasheet](../references/index.md#murata-dlw5btm-ds)), placed after Q2
  where the voltage is ≤ 40 V. It targets the 26–108 MHz bands, where the
  switch-node edges and harmonics 12 and up (27.6 MHz and above) couple as
  common mode. Its attenuation depends on layout and the ground reference; it
  must be verified with a pre-compliance scan **(verify)**.
- Filter placement: after the protection so that its capacitors see ≤ 40 V
  (§11); the protection parts are passive in normal operation.

## 6. Regulation

### 6.1 Buck candidates

| Part | Input | fsw / sync | Min on-time | AEC-Q100 | Notes | Source |
|---|---|---|---|---|---|---|
| **TI LMR43620-Q1** (MC variants: MODE/SYNC, no spread spectrum) | 3.0–36 V, 42 V abs max | Fixed 2.2 MHz (2.1–2.3 MHz); **sync 0.2–2.5 MHz**; FPWM in sync; MSC variants add spread spectrum, active only when free-running | 65 typ / 75 ns max | Grade 1 (−40 to +125 °C TA) | 2 A, 2 × 2 mm HotRod; 1.2–1.6 µA non-switching IQ; RθJA 84.4 °C/W (JEDEC), 50 °C/W on the EVM | [datasheet](../references/index.md#ti-lmr436x0-q1-ds) |
| TI LM62440-Q1 / LMQ62440-Q1 | 3–36 V, 42 V load dump | 2.1 MHz / 400 kHz; sync 0.2–**2.2 MHz** | 55 / 70 ns | Grade 1 | 4 A; LMQ has internal input caps (CISPR 25 class 5) | [LM62440](../references/index.md#ti-lm62440-q1-ds), [LMQ62440](../references/index.md#ti-lmq62440-q1-ds) |
| TI LMR33630-Q1 | 3.8–36 V | Fixed 400 kHz / 1.4 / 2.1 MHz, **no sync** | 68 ns | Grade 1 | 2.1 MHz × 14 = 29.4 MHz, inside 10 m: **rejected** | [datasheet](../references/index.md#ti-lmr33630-q1-ds) |
| ADI LT8609S, MAX20404 (Silent Switcher / spread spectrum) | — | — | — | — | **Not evaluated**: analog.com blocked scripted access on 2026-09-24. Worth a look in #11 | — |

**Choice: one LMR43620MC3RPERQ1** (maintainer decision 2026-09-24): the
LMR436x0 family is the only candidate whose sync range reaches the band-clean
2.304 MHz (§6.2), and the MC3 variant is 3.3 V fixed, MODE/SYNC, without
spread spectrum. The LM6x440 family is the fallback at 2.150 MHz with a
smaller margin.

**Cheaper drop-in check (maintainer rule, 2026-09-25):** a cheaper drop-in
is used only if it is automotive-grade, "since this might get connected to
a car battery".
- The LMR43610-Q1 (1 A) shares the LMR43620-Q1's datasheet, package and
  pinout, and has the same 0.2–2.5 MHz sync range and 75 ns / 85 ns timing
  ([datasheet](../references/index.md#ti-lmr436x0-q1-ds), Device Comparison
  Table).
- Its AEC-Q100 orderables are MSC3, MSC5, RS3Q and RS5Q (TI product page,
  2026-09-25). **There is no MC3 variant** (MODE/SYNC, no spread spectrum).
  The nearest, **LMR43610MSC3RPERQ1**, adds spread spectrum while
  free-running.
- It is also **not cheaper**: TI.com $2.850 (100–249, 0 in stock) against
  $2.494 for the MC3-Q1 (3,500 in stock). At LCSC it is $4.3244 (0 in stock)
  against $3.9856 (C5219290 vs C41658611, 2026-09-25).
- **Decision: keep LMR43620MC3RPERQ1 for both variants.**
- The commercial **LMR43610MB3RPER** (1 A, 3.3 V, MODE/SYNC, no spread
  spectrum, $1.40 at LCSC; [datasheet](../references/index.md#ti-lmr436x0-ds))
  was **not adopted** because it is not AEC-Q100 qualified.

### 6.2 Switching frequency against the HF amateur bands

US band edges, 47 CFR 97.301 (Amateur Extra, ITU Region 2)
([eCFR](../references/index.md#ecfr-47-97-301)): 160 m 1.800–2.000,
80/75 m 3.500–4.000, 40 m 7.000–7.300, 30 m 10.100–10.150, 20 m 14.000–14.350,
17 m 18.068–18.168, 15 m 21.000–21.450, 12 m 24.890–24.990, 10 m
28.000–29.700, 6 m 50–54 MHz. For 60 m the whole 5.3305–5.4064 MHz envelope
of 97.303(h) is avoided ([eCFR](../references/index.md#ecfr-47-97-303)).

Method: for each fsw, check every harmonic n·fsw up to 30 MHz, widened by the
clock tolerance, against every band. Findings (1 kHz scan, 0.3–3.0 MHz):

- **No harmonic comb misses every band unless it is locked to an accurate
  clock.** With ±1 % tolerance (a good RT-set oscillator is worse), only
  2.308–2.310 and 2.731–2.738 MHz survive, with 4–11 kHz margin. With ±3 %,
  nothing does.
- **Common defaults fail:** 2.1 MHz × 14 = 29.4 MHz and 2.2 MHz × 13 =
  28.6 MHz (both in 10 m); 2.0 MHz is the 160 m band edge.
- Windows at ±0.1 % tolerance: 2.148–2.151, 2.287–2.331, 2.478–2.486,
  2.502–2.522, 2.706–2.762, 2.780–2.797, 2.973–2.997 MHz. Below 2.2 MHz (the
  LM6x440 sync limit) only **2.150 MHz** works, with a 50 kHz margin.
- **Chosen: 2.304 MHz** (= 48 kHz × 48), locked to a crystal-grade clock:

| n | f (MHz) | Nearest band | Distance to edge |
|---|---|---|---|
| 1 | 2.304 | 160 m | 304 kHz |
| 2 | 4.608 | 80/75 m | 608 kHz |
| **3** | **6.912** | **40 m** | **88 kHz (worst)** |
| 4 | 9.216 | 30 m | 884 kHz |
| 5 | 11.520 | 30 m | 1370 kHz |
| 6 | 13.824 | 20 m | 176 kHz |
| 7 | 16.128 | 20 m | 1778 kHz |
| 8 | 18.432 | 17 m | 264 kHz |
| 9 | 20.736 | 15 m | 264 kHz |
| 10 | 23.040 | 15 m | 1590 kHz |
| 11 | 25.344 | 12 m | 354 kHz |
| 12 | 27.648 | 10 m | 352 kHz |
| 13 | 29.952 | 10 m | 252 kHz |

- The margin stays ≥ 53 kHz up to ±0.5 % clock error, so any crystal or MEMS
  oscillator (±20–50 ppm) is ample.
- **6 m can't be avoided** with fsw below 4 MHz (the band is 4 MHz wide):
  harmonics 22 and 23 (50.688 and 52.992 MHz) fall inside. They are handled by
  edge control, the CMC, layout and shielding, and must be measured
  **(verify)**.
- **Spread spectrum off:** it lowers CISPR 25 peak readings, but at harmonic n
  it smears energy over n × the spread, straight into the bands. The
  LMR436x0-Q1 disables it while synchronized. **FPWM always:** PFM light-load
  mode wanders in frequency; the MODE/SYNC variants run FPWM when synchronized.
- **Clock source (cheapest that meets ±50 ppm at −40 to +85 °C):** a stock
  **18.432 MHz** CMOS oscillator, **YXC OT322518.432MJBA4SL** (YSO110TR
  series: ±10 ppm at 25 °C, ±20 ppm over −40 to +85 °C, ±3 ppm/year, ≤ 5 mA,
  3 ms start-up; [datasheet](../references/index.md#yxc-yso110tr-ds)), divided
  by 8 with three **SN74LVC1G80** flip-flops (Q̅ to D, ÷2 each; −40 to
  +125 °C; [datasheet](../references/index.md#ti-sn74lvc1g80-ds)). Total
  about **$0.86** at LCSC, against about $3 for a programmed SiT8924B
  ([datasheet](../references/index.md#sitime-sit8924b-ds)). No 2.304 MHz
  (or 4.608/9.216 MHz, ÷2/÷4) oscillator was stocked at LCSC on 2026-09-24.
  The divider's intermediate clocks (18.432, 9.216 and 4.608 MHz) are
  multiples of 2.304 MHz, so they add no new spectral lines.
- **Shared with the audio codec (ADR-0002, PR #55).** The same 2.304 MHz
  clock is the TLV320AIC3104's MCLK: codec PLL P = 3, R = 8, J = 16, D = 0
  gives exactly 48 kHz, with the codec as I2S master. The clock therefore
  has up to three loads: U2 MODE/SYNC, codec MCLK, and the #9
  isolated-supply clock if used.
  - **Accuracy:** ±20 ppm over −40 to +85 °C (plus ±3 ppm/year), meeting
    ADR-0002's ±20 ppm assumption and constraints §8's ±50 ppm.
  - **Jitter:** oscillator phase jitter 0.7 ps max (12 kHz–20 MHz,
    [YXC](../references/index.md#yxc-yso110tr-ds)). The three LVC flip-flops
    add a little; their additive jitter isn't specified **(verify on the
    bench)**. It is far below what a PLL-fed audio MCLK tolerates.
  - **Levels:** SN74LVC1G80 at 3.3 V drives rail to rail (±32 mA). The codec
    MCLK input needs VIH ≥ 0.7 × IOVDD and VIL ≤ 0.3 × IOVDD at IOVDD = 3.3 V
    ([TLV320AIC3104](../references/index.md#ti-tlv320aic3104-ds)), and the
    MODE/SYNC pin needs ≥ 1.6 V high and ≤ 1 V low.
  - **Fanout:** the last flip-flop drives the three loads as a star, with a
    33 Ω series resistor at the driver for each trace (source termination;
    each trace a few cm, over ground). At 2.304 MHz three CMOS inputs
    (≈ 5–10 pF each) draw well under 1 mA. A 74LVC1G34 buffer per branch
    ($0.05–0.10) is the option if the codec branch needs isolation from the
    buck's SYNC trace **(decide at layout)**.
- **Shared clock recommendation (for ADR-0002, ADR-0004 and ADR-0005).**
  One 2.304 MHz net feeds every load: buck MODE/SYNC in both variants, the
  codec MCLK through its PLL (P = 3, R = 8, J = 16, D = 0 → 48 kHz, as
  ADR-0002 has it) and the #9 isolated-supply clock. The source is an
  **18.432 MHz XO ÷ 8**. Options compared (LCSC, 2026-09-24):

| | (a) 2.304 MHz oscillator direct | (b1) 18.432 MHz XO ÷ 8, codec on 2.304 MHz via PLL | (b2) as b1, codec MCLK on 18.432 MHz without PLL |
|---|---|---|---|
| Parts | Programmable MEMS, e.g. SiT8008BI at 2.304 MHz ([datasheet](../references/index.md#sitime-sit8008-ds)); YXC also offers YSO110TR at any 1–125 MHz frequency on request | YXC OT322518.432MJBA4SL + 3 × SN74LVC1G80 (PR #60 uses an SN74HC161 counter instead) | Same parts |
| Cost | ≈ $0.43 (qty 200) to $1.11 (qty 1) for SiT8008BI listings at LCSC (4.608 MHz version); **the 2.304 MHz part isn't stocked**, so price and lead time **(verify)** | **≈ $0.86** (LVC ÷ 8), or ≈ $0.67 with the HC161 | Same |
| Accuracy, −40 to +85 °C | ±20 or ±25 ppm (industrial grade) | ±20 ppm (±10 ppm at 25 °C) | Same |
| Jitter | 0.5 ps typ / 0.9 ps max RMS phase jitter (spec at 75 MHz) | 0.7 ps max phase jitter (12 kHz–20 MHz), plus unspecified flip-flop additive jitter **(verify)** | Oscillator only on the codec path |
| Drive, 3 loads | One LVCMOS output, 15 pF load spec; series-R star | LVC1G80 ±32 mA; series-R star | Two nets to route |
| Spurious | Only the 2.304 MHz comb | Adds a short 18.432 MHz net: fundamental 264 kHz above 17 m (18.168 MHz edge); 3rd harmonic 55.296 MHz, above 6 m; **8th harmonic 147.456 MHz inside 2 m** (144–148 MHz). The 2.304 MHz comb itself has its 63rd and 64th harmonics (145.152 and 147.456 MHz) in 2 m, so 2 m isn't clean either way; the 18.432 MHz net concentrates energy there | Worst: the 18.432 MHz net runs to the codec |
| Codec clock | PLL from 2.304 MHz | PLL from 2.304 MHz | Non-PLL: fS = MCLK / (128 × Q) = 18.432 MHz / (128 × 3) = **48 kHz exactly**, Q = 2–17 ([TLV320AIC3104](../references/index.md#ti-tlv320aic3104-ds), eq. 1) |

  **Recommendation: (b1).**
  - It is stocked, costs about $0.86, and meets ±20 ppm and the codec
    jitter needs.
  - The codec stays on 2.304 MHz via its PLL, as in ADR-0002. That exact
    48 kHz derivation is cited in ADR-0002 (PR #55) and was not checked here.
  - Keep the 18.432 MHz net under 5 mm, from the oscillator straight into
    the first flip-flop, over solid ground.
  - Use SN74LVC1G80 rather than an HC161: the LVC parts are specified far
    above 18.432 MHz at 3.3 V, while the HC161's margin at 3.3 V is
    **(verify)** (PR #60 flags it too).
  - **Revisit (a)** if a 2.304 MHz MEMS part is orderable at or below the
    (b1) cost. It removes the 18.432 MHz net and three parts.
  - (b2) is rejected for emissions.
- The ESP32-S3 can't make 2.304 MHz by integer division of its 80 MHz or
  40 MHz clocks. Its nearest, 80 MHz / 35 = 2.2857 MHz, puts harmonic 13
  only 14 kHz above 10 m, and a fractional LEDC divider would add jitter spurs.
- **Before sync is present** (the first milliseconds, before the 3.3 V rail
  powers the oscillator, which then needs up to 3 ms to start), the MC3
  (no spread spectrum) free-runs at a **fixed 2.2 MHz**, specified as 2.1–2.3 MHz
  ([datasheet](../references/index.md#ti-lmr436x0-q1-ds), FSW(2p2MHz)).
  Against the harmonic table, a nominal 2.2 MHz puts only harmonic 13
  (28.6 MHz) inside a band (10 m). Across the full 2.1–2.3 MHz tolerance,
  harmonics 8 (17 m), 10 (15 m), 11 (12 m) and 13 (10 m) can fall in bands.
  This lasts only until the oscillator starts, before any receive or
  transmit. Behavior if the clock is lost later **(verify)**; presumably the same fallback.
- The **isolated jack-side supply (#9, from 3.3 V) must follow the same rule.** The
  SN6505B-Q1 accepts an external clock of 100–1600 kHz and divides it by 2
  ([datasheet](../references/index.md#ti-sn6505-q1-ds)). Its comb would then
  be 2.304 / k MHz, and the extra harmonics land in bands. Options for #9:
  heavy filtering and shielding of an SN6505B-Q1 stage, or an isolated
  topology switching at 2.304 MHz itself (for example a synchronized
  Fly-Buck; suitability of the LMR436x0-Q1 **(verify)**).

### 6.3 The 3.3 V stage

**U2: LMR43620MC3RPERQ1**, protected input → 3.3 V, FPWM, synchronized at
2.304 MHz ([datasheet](../references/index.md#ti-lmr436x0-q1-ds): tON-MIN
65 ns typ / 75 ns max, tOFF-MIN 60 ns typ / 85 ns max, tON-MAX 6–13 µs in
dropout).

- **Upper limit of synchronized operation:** tON = D / fsw ≥ tON-MIN gives
  Vin ≤ 3.3 V / (75 ns × 2.304 MHz) = **19.1 V** with the maximum tON-MIN
  (22.0 V with the typical 65 ns). That covers the 9–16 V range and the 18 V
  long-term and transient overvoltage tests (tON = 79.6 ns at 18 V).
- **Above that, the part lowers its frequency** to hold regulation (datasheet
  §7.1: "the switching frequency is reduced automatically"): about
  1.69–1.95 MHz during the **26 V jump start** (up to 60 s) and 1.26–1.45 MHz
  during the **35 V test B** clamp (≤ 400 ms). The harmonics move for the
  duration, and a few may cross a band. That is accepted: both are rare
  events while the engine is being started or the alternator misbehaves.
  Whether the part stays phase-related to SYNC while folded back **(verify)**.
- **Lower limit of synchronized operation:** maximum duty
  1 − tOFF-MIN × fsw = 1 − 85 ns × 2.304 MHz = **0.804**, so Vin ≥ 3.3 / 0.804
  = **4.10 V** plus IR drops (≈ 0.1 V at 0.57 A through the FETs, fuse, CMC
  and L1), **≈ 4.2 V at the connector**. The normal cold crank (4.5 V) stays
  synchronized.
- **Below that, dropout:** VDROP1 = 0.2 V typ (3.3 V, 1 A, output ≥ 95 %,
  with frequency foldback) keeps the rail at ≥ 3.14 V down to about **3.5 V
  input**. VDROP = 0.7 V typ keeps FSW ≥ 1.85 MHz down to about 4.0 V.
- **Inductor:** 2.2 µH per the datasheet's Table 8-2 (3.3 V, 2.2 MHz), the same
  TDK TFM252012ALMA2R2MTAA as L1 (2.6 A, 75 mΩ). Ripple at 2.304 MHz:
  0.41 A pp at 9 V, 0.52 A pp at 16 V; peak 0.83 A at 0.57 A load, well under
  its current rating. A cheaper Sunlord SWPA4030S2R2NT (2.95 A, 30 mΩ,
  $0.048) is a candidate once its temperature range is confirmed **(verify)**.
- **Output capacitors:** 2 × 22 µF (datasheet Table 8-2), **Samwha
  CS3216X7R226K160NRI**, 16 V X7R 1206 ([datasheet](../references/index.md#samwha-cs-ds)),
  rated ≥ 2 × 3.3 V per pcb-fabrication §6.3 (10 V minimum) with DC-bias
  headroom. Ripple ≈ 1.4 mV pp with 20 µF effective **(verify DC bias)**.
- **Input capacitors:** C2 (2 × 2.2 µF 100 V, ≈ 3.8 µF effective) plus
  100 nF 100 V at the VIN pin (datasheet: 4.7 µF + 100 nF).
- **No 5 V rail, no second buck.** No VBUS switch to the radio either; the
  radio port's VBUS is blocked in hardware (ADR-0003, #9).
- **Codec analog rail (ADR-0002 / #8):** an **LP5907-3.0** LDO from the 3.3 V
  rail ([LP5907](../references/index.md#ti-lp5907-q1-ds): 250 mV max dropout
  at 250 mA, 120 mV typical). It needs the 3.3 V rail at **≥ about 3.1 V**.
  Check against the figures above:
  - Synchronized (input ≥ ≈ 4.2 V, including the whole normal cold crank):
    the rail is regulated at 3.27–3.33 V (±1 %, datasheet), giving the LDO
    ≥ 270 mV of headroom against its 250 mV worst-case dropout. At the
    codec's ≈ 50 mA the real dropout is a fraction of that.
  - In dropout (input ≈ 4.2 V down to ≈ 3.5 V), the rail stays ≥ 3.135 V
    (VDROP1 spec, ≥ 95 % at 1 A; the load here is 0.57 A), so ≥ 3.1 V holds.
  - Below about 3.5 V input (severe crank only) the codec rail drops. PTT was
    forced off at 7.0 V long before (§8).
- The **ESP32-S3 3.3 V** comes straight from U2 with the decoupling and
  ferrite in Espressif's hardware design guidelines.

### 6.4 Efficiency and thermal

- **Limit:** LMR436x0-Q1 operating junction temperature −40 to **+150 °C**
  ([datasheet](../references/index.md#ti-lmr436x0-q1-ds), Recommended
  Operating Conditions). Heat is acceptable as long as TJ stays below that.
- **Efficiency** read from the datasheet's Figure 8-4 (LMR43620MSC3, 3.3 V
  fixed, 2.2 MHz FPWM, VIN = 12 V, 25 °C): about 79 % at 0.1 A, 87 % at
  0.2 A, 90 % at 0.3 A, 92 % at 0.5–1 A. At 2.304 MHz and 85 °C it will be a
  little lower **(verify)**. FPWM costs about 0.1 W at no load (30 % at
  10 mA), the price of a fixed frequency.
- Thermal resistance 50 °C/W (EVM) to 84.4 °C/W (JEDEC):

| Case | 3.3 V load | η (curve) | Input | U2 loss | TJ at 85 °C ambient |
|---|---|---|---|---|---|
| Worst case (§2 method) | 0.57 A, 1.88 W | 92 % | 2.07 W | 0.16 W | **≤ about 99 °C** |
| Typical (§2) | 0.28 A, 0.92 W | 89.5 % | 1.06 W | 0.11 W | ≤ about 94 °C |
| Typical, BLE TX capped at the FCC-grant 10.3 dBm (#7): ESP32-S3 ≈ 0.2 A (204 mA at +9 dBm, [datasheet](../references/index.md#esp32s3-mini1-ds) Table 6-5) | 0.38 A, 1.26 W | 91 % | 1.42 W | 0.13 W | ≤ about 96 °C |

- Every case is at least 51 °C under the 150 °C limit on 2 layers.
- Q1/Q2 conduction: 0.10 W at 4.5 V input (short cold-crank dip), 11 mW at
  13.5 V. L1, CMC and F1 together < 20 mW.
- The electrolytic's 5000 h at 105 °C becomes roughly 20,000 h at 85 °C
  (doubling per 10 °C rule of thumb) **(verify** against the series data).

## 7. Cold-crank decision

**Decision: no buck-boost. Accept brownout with PTT forced off.**

- Normal cold crank (4.5 V for 15 ms, then 6.5 V) keeps the 3.3 V rail
  regulated and synchronized (≈ 4.2 V needed, §6.3), meeting the "4.5 V
  desirable" target of constraints §3.2 (REQ-PWR-011). PTT is off below 7.0 V
  anyway (§8). Severe crank (3 V) resets the device; PTT is off by the
  hardware default.
- A buck-boost would only add logic uptime between about 3.5 V and 3 V. It
  would cost a second converter, a second frequency to place, and money. The
  radio itself generally isn't specified to operate that low; most mobile HF
  rigs are specified around 13.8 V ± 15 % **(verify per radio, #5)**.

## 8. Brownout detector, PTT fail-safe and hold-up

- **TPS3710** (commercial TPS3710DSER, −40 to +125 °C; the -Q1 version
  has the same electrical specification: 1.8–18 V supply, 400 mV reference
  ±1 % over temperature, 5.5 µA, open-drain, tpd(HL) 18 µs;
  [TPS3710](../references/index.md#ti-tps3710-ds),
  [TPS3710-Q1](../references/index.md#ti-tps3710-q1-ds); buy whichever is
  cheaper at order time) senses the protected
  rail (after Q2) through **330 kΩ / 20 kΩ → 7.00 V falling**. That draws
  40 µA at 14 V, only while powered. SENSE sees 2.29 V at 40 V, under its 7 V
  absolute maximum. It runs from the 3.3 V rail and works down to 1.8 V.
- Its output pulls the **PTT enable** low. PTT enable is ANDed with the
  watchdog gate from #9, so the PhotoMOS LED drive stops even if the MCU
  hasn't reacted. The same signal interrupts the MCU, which also drops PTT in
  firmware and logs the event. Firmware can also inhibit PTT at a higher,
  configurable threshold (for example below 9 V, via ADC).
- Because the PhotoMOS is off unless driven (constraints §6), losing 3.3 V
  also opens PTT. The detector's job is to open it **cleanly before** the
  logic enters an undefined state.
- **Threshold revisited for the single stage:** the logic now runs down to
  about 3.5 V input, so a lower threshold would be possible (6.0 V gives
  477 µs of hold-up). **7.0 V is kept.** It sits above the 6.5 V plateau of
  the normal cold crank, so PTT stays off through the whole crank. The radio
  itself isn't specified that low (#5). And it gives the longest margin
  before logic dropout.
- **Hold-up** (100 µF − 20 % + 3 × 1.9 µF ≈ 85.7 µF; t = C·(V1² − V2²) / 2P;
  dropout at 3.6 V input):

| From → to | 2.1 W (worst) | 1.1 W (typical) | Meaning |
|---|---|---|---|
| 7.0 V → 3.6 V | **747 µs** | 1.46 ms | Time from PTT-off to logic dropout; detector path ≈ 20 µs |
| 14 V → 7.0 V | 3.0 ms | 6.0 ms | Supply interruptions shorter than this don't disturb PTT |

- Q2 blocks reverse current within 0.5 µs, so a dipping or shorted input
  doesn't drain the hold-up capacitance.

## 9. Ignition / radio-on sense, auto power-down, off-state drain

- **Wake and keep-alive** all drive the LM74800-Q1 **EN/UVLO** pin through
  small Schottky diodes (diode-OR), with a pull-down:
  - **SENSE input** (ignition, or a radio's switched accessory DC where
    available, per #5): 100 kΩ series (1206, ≥ 200 V, pulse-rated) into
    22 kΩ to ground, clamped by a 3.3 V zener. It enables at about 7.1 V and
    survives 101 V (≈ 1 mA into the clamp, 0.09 W in the resistor) and −150 V.
    The same node goes to an ESP32-S3 GPIO.
  - **USB-C host VBUS** (wired mode): divider to about 1.6 V.
  - **Firmware HOLD** (3.3 V GPIO, pulled down, low at reset).
  - Optional push-button. For installs without an ignition wire, tie SENSE
    to the battery lead (then the device stays on).
- **Power-down:** SENSE falls → firmware drops PTT, finishes, waits the
  power-down delay (**30 s after the radio or ignition turns off, default;
  configurable**, maintainer decision 2026-09-25), releases HOLD → EN low →
  Q1/Q2 off → all rails
  collapse. A hung MCU is reset by the watchdog, and its HOLD pin defaults low,
  so it can't keep the device on. Firmware may also power down on low battery
  (for example < 11.5 V for 60 s with SENSE off; threshold TBD).
- **Off-state drain** at 12.6 V, EN low:

| Contributor | Current |
|---|---|
| LM74800-Q1 shutdown, I(GND) incl. VS | 2.87 µA typ, **5 µA max** (TJ −40 to +125 °C) |
| TVS stack leakage (IR ≤ 1 µA at VRWM; here at 12.6 V against 103 V VRWM) | ≤ 1 µA |
| Q1 leakage (IDSS ≤ 1 µA at 120 V, 25 °C) | ≤ 1 µA |
| SENSE, VBUS, HOLD networks, OV divider (disconnected by the SW pin), TPS3710, oscillator | 0 (unpowered) |
| **Total** | **≤ 7 µA at 25 °C**, over 140× below the 1 mA limit. FET and TVS leakage at 85 °C **(verify)** |

## 10. Parts, qualification, price and stock

Prices in USD, checked 2026-09-24. LCSC: price at the tier containing
quantity 100. TI.com: 100–249 tier. Digi-Key and Mouser **blocked automated
lookups** on 2026-09-24, so those cells are blank; they must be filled before
ordering. The TI parts are listed as **Active** on ti.com; lifecycle for other
parts is **(verify)**. Stock figures are data only; the maintainer does not
treat low stock as a concern (2026-09-24). **RoHS:** every part below is
listed as RoHS-compliant by LCSC. **REACH SVHC:** only the YXC datasheet shows
a REACH mark; the rest **(verify)** against the manufacturers' declarations
(#59).

| Ref | Part | Qualification / temp (datasheet) | RoHS | LCSC (# / price / stock) | TI.com (price / stock) | Digi-Key / Mouser |
|---|---|---|---|---|---|---|
| F1 | Littelfuse 0466002.NRHF (2 A 63 V) | Commercial | Yes | C3105 / 0.0552 / 42,640 | — | blank |
| D3 | Littelfuse TPSMB82A | AEC-Q101; TJ −65 to +175 °C | Yes | C3704846 / 0.1833 / 2,980 | — | blank |
| D4 | Vishay SMBJ33CAHE3_B/H | AEC-Q101 (HE3); TJ −55 to +150 °C | Yes | C20037758 / 0.1367 / 9,185 | — | blank |
| C_in | YAGEO AS1206KKX7RYBB104 (100 nF 250 V, soft termination) | AEC-Q200 | Yes | C3881218 / 0.1200 / 5,000 | — | blank |
| U1 | TI LM74800QDRRRQ1 | AEC-Q100 grade 1 | Yes | C3215600 / 1.8652 / 2,276 | 2.074 / 28,516 | blank |
| Q1 | onsemi FDN86246 (150 V) | Commercial; TJ −55 to +150 °C | Yes | C891118 / 0.4917 / 1,941 | — | blank |
| Q2 | Infineon IRLML0100TRPBF (100 V) | Commercial; TJ −55 to +150 °C | Yes | C53658 / 0.3923 / 7,680 | — | blank |
| FL1 | Murata DLW5BTM142TQ2L | Commercial; −40 to +105 °C | Yes | C341531 / 0.4549 / 746 | — | blank |
| L1, L2 | TDK TFM252012ALMA2R2MTAA (2.2 µH) | AEC-Q200; −55 to +150 °C | Yes | C404804 / 0.2020 / 4,300 | — | blank |
| C1, C2 (×2) | PSA FS32X225K101EGG (2.2 µF 100 V X7R 1210) | Commercial; X7R | Yes | C153036 / 0.0720 / 346,540 | — | blank |
| C_bulk | Huawei VD1H101MF105000CE0 (100 µF 50 V) | Commercial; −55 to +105 °C | Yes | C189260 / 0.1846 / 8,840 | — | blank |
| U2 | TI LMR43620MC3RPERQ1 | AEC-Q100 grade 1 | Yes | C41658611 / 3.9856 / 10 | 2.494 / 3,500 | blank |
| U2 (not adopted) | TI LMR43610MB3RPER (1 A, commercial): not AEC-Q100 | Commercial; TJ −40 to +150 °C | Yes | C5899189 / 1.4045 / 665 | 1.655 / 18,000 | blank |
| U2 (checked) | TI LMR43610MSC3RPERQ1 (1 A, AEC-Q100, spread spectrum when free-running): not cheaper | AEC-Q100 grade 1 | Yes | C5219290 / 4.3244 / 0 | 2.850 / 0 | blank |
| C_out (×2) | Samwha CS3216X7R226K160NRI (22 µF 16 V X7R 1206) | Commercial; X7R | Yes | C5252682 / 0.1186 / 18,980 | — | blank |
| Y1 | YXC OT322518.432MJBA4SL (18.432 MHz, ±20 ppm) | Commercial; −40 to +85 °C | Yes (REACH mark) | C2831385 / 0.2858 / 1,149 | — | blank |
| U7–U9 | TI SN74LVC1G80DBVR (÷8) | Commercial; −40 to +125 °C | Yes | C42879 / 0.1899 / 5,415 | 0.129 / 285,101 | blank |
| U5 | TI TPS3710DSER | Commercial; −40 to +125 °C | Yes | C702154 / 1.4398 / 2,989 | 1.325 / 192,360 | blank |
| U5 alt | TI TPS3710QDSERQ1 | AEC-Q100 grade 1 | Yes | C2863756 / 1.0411 / 0 | 1.563 / 73 | blank |
| (U6) | TI SN6505BQDBVRQ1 (isolated supply, #9) | AEC-Q100 grade 1 | Yes | C1849490 / 0.7522 / 4,745 | 1.820 / 18,670 | blank |

**Power-section cost (main parts above, LCSC, qty 100; minor resistors,
zeners, diodes and small capacitors excluded in both):**

| Version | Main parts | Total |
|---|---|---|
| Before (two AEC-Q bucks, 5 V + 3.3 V, SiT8924B, AEC-Q parts throughout, LP5907-Q1) | 18 lines | **≈ $23.56** |
| After (single stage, commercial where allowed, 18.432 MHz ÷ 8) | 17 lines | **≈ $10.95** |

The biggest savings: Q1 ($5.91 → $0.49), the second buck and its passives
(≈ $4.5), the oscillator (≈ $2.96 → $0.86) and the 5 V-rail LDO. None of the
parts above is a JLCPCB Basic part, so each adds an extended-part fee
([JLCPCB FAQ](../references/index.md#jlcpcb-pcba-faqs)). Earlier AEC-Q
choices kept as alternates: 0437002.WRA (F1), DMTH15H017SPSWQ (Q1),
[SQSA80ENW](../references/index.md#vishay-sqsa80enw-ds) (Q2),
[ACM70V](../references/index.md#tdk-acm70v-ds) (FL1),
[CGA6N3X7R2A225K](../references/index.md#tdk-cga-ds) (C1/C2) and
[EEE-FK1H101P](../references/index.md#panasonic-fk-ds) (C_bulk).

## 11. Capacitor voltage ratings

Per [pcb-fabrication.md §6.3](../requirements/pcb-fabrication.md#63-voltage-rating-and-dc-bias)
(≥ 2× nominal and ≥ worst-case transient):

| Node | Worst case | Rating used |
|---|---|---|
| Connector to Q1 (before protection) | Test A 101 V; stack clamp up to ≈ 127 V on fast pulses | **250 V** (C_in); 2 × 101 V = 202 V |
| After Q2 (filter, hold-up, U2 input) | OV cut-off 37–40 V | **100 V** MLCC (2 × 40 V = 80 V → next standard 100 V); 50 V electrolytic (the 2× rule is for MLCC) |
| 3.3 V rail | Regulator overshoot | 10 V minimum; 16 V used on the 22 µF output capacitors for DC-bias headroom |

The DC-bias loss of the 100 V 1210 X7R at 14 V is small; 1.9 µF effective was
assumed in §5 **(verify** against the TDK DC-bias curve).

## 11a. EU requirements (#59)

The product must meet EU requirements (maintainer, 2026-09-24,
[#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59)).

- **RoHS:** every chosen part is RoHS-compliant per its distributor listing
  (§10). **REACH SVHC:** **(verify)** per part against the manufacturers'
  declarations (#59).
- **EMC for radio equipment used in vehicles:** ETSI **EN 301 489-1
  V2.2.3** ([ETSI](../references/index.md#etsi-en-301-489-1)); the
  radio-specific part for 2.4 GHz Bluetooth LE is EN 301 489-17 **(verify
  version, #59)**. For vehicle-use equipment on the 12 V DC input:
  - **Clause 9.6, transients and surges in the vehicular environment:** test
    method per ISO 7637-2 (**2004** edition, reference [8], not updated on
    purpose), pulses 1, 2a, 2b, 3a, 3b and 4 at **immunity test level III**.
    Pulses 1/2a/2b/4 are applied 10 times each, 3a/3b for 20 minutes each.
    The design targets the 2011 edition's level IV (−150 / +112 / −220 /
    +150 V, §1.2). That is at least as severe as the 2004 level III values
    TI lists for 12 V (−100 / +50 / −150 / +100 V;
    [TI TIDUB49](../references/index.md#ti-tidub49); level mapping
    **(verify)**). Pulse 2b (+10 V) is covered by the 26 V jump-start
    rating. Pulse 4 (starting profile) is covered by operation down to about
    3.5 V (§6.3); its 2004 level III amplitude **(verify)**. Performance
    criteria: continuous phenomena for 3a/3b, transient phenomena for
    1/2a/2b/4; the link may drop and re-establish.
  - **Clause 8.3, conducted emissions on the DC power port:** for vehicle
    equipment measured with the CISPR 25 artificial network, 150 kHz–30 MHz.
    The limits are 79/66 dBµV (QP/AV, 0.15–0.5 MHz) and **73/60 dBµV
    (0.5–30 MHz)**. The filter's 28 dBµV design target (§5) is more than
    30 dB under that.
  - Fast transients (EN 61000-4-4, 0.5 kV on DC ports) and whether they apply
    to vehicle-use equipment **(verify, #59)**. C_in, the TVS stack and the
    CMC are the first line.
- **Automotive EMC approval:** whether UN ECE Regulation No. 10 (E-marking)
  applies is #59's question.

## 12. Layout guidance (for later KiCad work)

- **Zones in order along one edge:** connector → F1/TVS/C_in → Q1/Q2/U1 →
  CMC → pi filter → bucks. Keep the unfiltered input copper short and away
  from everything else. Nothing noisy may route under or beside the input
  zone, or it couples straight past the filter.
- **Hot loop:** the buck's input capacitors (100 nF plus 2.2 µF)
  directly at VIN/PGND on the same layer, as in the LMR436x0-Q1 layout
  example; switch node as small as possible; inductor next to the SW pin;
  no vias in the hot loop.
- **Ground:** a solid bottom-layer ground under the whole power section;
  stitch the top pours around the buck with vias. Don't split ground. Return
  the TVS stack directly to the connector ground with a short, wide path.
- **High voltage:** ≥ 0.5 mm clearance on nets that can exceed 50 V
  (connector to Q1, the VS clamp); trace widths per IPC-2221 for 1 A at
  ≤ 10 °C rise (pcb-fabrication §3).
- **Sync clock:** oscillator and ÷8 flip-flops next to U2, short trace to
  MODE/SYNC over ground, away from the audio and RF sections. A series
  resistor (22–47 Ω) slows edges.
- **Separation from the radio side:** the power section sits away from the
  AUDIO/SERIAL jacks and the antenna keep-out of the ESP32-S3-MINI-1. The
  isolated supply's transformer straddles the isolation gap; keep the
  isolation barrier clear.
- **Thermal:** normal ground pour and thermal vias under U2 (§6.4).
- **Shielding:** reserve footprint space for a board-level shield can over
  the buck and the oscillator, fitted only if the 6 m / VHF scan needs it.
- **Test points:** protected rail, 3.3 V, SYNC, PTT enable, brownout
  output, for the pulse and EMC tests.

## 13. Parts common with variant R (#11)

- **Shared 3.3 V regulator core:** variant R reuses this exact core: the
  LMR43620MC3RPERQ1 with its 2.2 µH
  inductor and output capacitors, locked to the same 18.432 MHz ÷ 8 =
  2.304 MHz clock. The harmonic analysis (§6.2) then covers both variants.
  The radio accessory input of variant R (11–15 V) is inside the synchronized
  range (≤ 19.1 V); a USB-C 5 V input is too (≥ 4.2 V). PR #60 (#11,
  ADR-0005) already uses the same LMR43620MC3RPERQ1 core and the 18.432 MHz
  ÷ 8 clock.
- **Same TVS and ideal diode:** SMBJ33CA-HE3 and LM74700-Q1 or LM74800-Q1
  (common drain, 60–80 V FETs, no 150 V FET needed without unsuppressed load
  dump) for the accessory input's reverse-polarity and TVS protection.
- **Same TPS3710** brownout detector. The codec rail is #8's decision for
  both. Neither variant has a VBUS switch to the radio (ADR-0003, #9).
- **Variant M only:** the 150 V Q1, TPSMB82A, OV cut-off, CMC and the
  ignition/auto power-down network.

## 14. 24 V trucks (future)

Not supported in revision A (issue default). A 24 V variant would face test A
at 151–202 V with Ri 1–8 Ω, 100–350 ms
([Microchip](../references/index.md#mchp-ds00006186) Table 2-4), a 58 V
suppressed load dump, and a −26 V reverse battery. The LM74800-Q1
common-source design scales to it (TI's §10.3 example is exactly 200 V / 24 V,
with a 200 V Q1), but the buck would need a 60–65 V input part and new TVS
values. Record it as a separate variant if there is demand.

## 15. Open items

- **Bench tests (human-task, later):** ISO 7637-2 pulses 1/2a/3a/3b at
  level IV; ISO 16750-2 test A (both corners) and test B; jump start 26 V;
  cold crank normal and severe (confirm the logic survives 4.5 V); ESD
  ISO 10605; CISPR 25 conducted and radiated pre-scan; receiver noise-floor
  check on every band 160 m–6 m with the radio next to the device.
- **(verify):** ISO 10605 levels; 2023 reversed-voltage test cases;
  long-term and transient overvoltage values (from one secondary source);
  U2 efficiency at 2.304 MHz and 85 °C; fold-back behaviour while
  synchronized; MLCC DC bias; TVS and FET leakage at 85 °C; 56 V zener part;
  electrolytic life at 85 °C; Sunlord inductor temperature range; REACH SVHC
  per part; EN 301 489 pulse 4 level and EFT applicability (#59);
  PhotoMOS turn-off time (#9).
- **REQ-PWR-014** says 24 V; ISO 16750-2:2023 says 26 V. #4's owner should
  update it to 26 V.
- **Decisions for other issues:** isolated jack-side supply (from 3.3 V) and
  its frequency (#9); the codec analog rail and the hub and codec currents
  (#8, #9); UN ECE R10 and REACH (#59).
- **Buck part:** LMR43620MC3RPERQ1 kept for both variants; no cheaper
  automotive-grade drop-in exists (§6.1).

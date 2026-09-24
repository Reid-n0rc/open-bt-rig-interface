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
- **Protection:** 2 A AEC-Q200 fuse → anti-series TVS stack (TPSMB82A +
  SMBJ33CA-HE3, breakdown ≥ 107 V at −40 °C) → **LM74800-Q1** ideal-diode
  controller with back-to-back N-MOSFETs in **common-source** topology (150 V
  blocking FET), with overvoltage **cut-off at about 38.5 V**. The TVS absorbs
  no load-dump energy; unsuppressed load dump (101 V) is blocked by the FET (§4).
- **Filter:** TDK ACM70V common-mode choke + 2.2 µH / 2.2 µF pi filter,
  corner about 78 kHz, about 59 dB ideal attenuation at the switching
  frequency (§5).
- **Regulation:** two **LMR43620-Q1** bucks (12 V → 5 V, then 5 V → 3.3 V),
  **synchronized to a 2.304 MHz crystal-grade clock** so that no harmonic from
  160 m to 10 m lands inside a US amateur band (worst margin 88 kHz; §6).
  The usual 2.1 MHz and 2.2 MHz defaults put a harmonic inside 10 m.
- **Cold crank:** no buck-boost. Logic keeps running to about 3.8 V input; a
  hardware brownout detector forces PTT off at 7.0 V; hold-up is ≥ 618 µs from
  7.0 V at worst-case load (§7, §8).
- **Power-down:** ignition / radio-on sense, USB-C host and a firmware hold
  line drive the LM74800-Q1 enable. **Off-state drain ≤ 7 µA** at 25 °C
  (limit 1 mA; §9).
- **Power budget:** about 1.2 W typical and **2.4 W worst case** input, inside
  the 1.5 W typical / 3 W peak target of constraints §3.4 (REQ-PWR-003) (§2).

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

Placeholders are marked with the issue that settles them.

| Rail | Load | Typical | Worst case | Basis |
|---|---|---|---|---|
| 3.3 V | ESP32-S3-MINI-1 | 0.10 A | **0.34 A** (BLE TX +20 dBm) | [datasheet](../references/index.md#esp32s3-mini1-ds); typical per constraints §3.4 (#7) |
| 3.3 V | USB hub (USB2422), wired mode | 0 (Bluetooth mode) | 0.05 A | Placeholder (#9) |
| 3.3 V | Audio codec incl. its 1.8 V core LDO | 0.05 A | 0.05 A | Placeholder (#8) |
| 3.3 V | Oscillator, supervisor, USB switches, LEDs, PTT PhotoMOS LED, isolator side 1 | 0.02 A | 0.02 A | Allowance; SiT8924B ≤ 4.8 mA ([datasheet](../references/index.md#sitime-sit8924b-ds)) |
| **3.3 V total** | | **0.17 A (0.56 W)** | **0.46 A (1.52 W)** | |
| 5 V | Isolated jack-side supply (RS-232, analog switches, isolator side 2) | 0.07 A | 0.07 A | Placeholder: 0.25 W out at 70 % (#9) |
| 5 V | Buck B input (3.3 V rail at 88 %) | 0.13 A | 0.35 A | Efficiency **(verify)** |
| **5 V total** | | **0.20 A (1.0 W)** | **0.42 A (2.1 W)** | |
| 12 V in | Buck A at 88 %, protection and filter losses | **≈ 1.2 W** | **≈ 2.4 W** | Efficiency **(verify)** |

- **No radio load.** The radio port's VBUS is blocked (ADR-0003, #9), so no
  VBUS switch or radio current appears here. The radio USB isolator
  (ADuM4160 fitting option, constraints §6) needs only a small radio-side
  supply, which belongs to the isolated supply (#9).
- Input current at 2.4 W: 0.18 A at 13.5 V, 0.27 A at 9 V, 0.40 A at 6 V.
- Inside constraints §3.4 / REQ-PWR-003 (about 1.5 W typical, 3 W peak).
- **USB-C in variant M** is a sink-only data link (wired mode). The device
  never sources power on it. In variant M its VBUS is used to detect a host and
  as a wake source (§9). Whether variant M can also run from USB-C VBUS alone
  (for bench use) belongs to #11's source-mux design. The 5 V budget (0.42 A
  worst case) would fit a 500 mA USB-C default, but that isn't designed here.

## 3. Architecture

```text
12 V lead ─ F1 ─┬─ D3+D4 TVS stack ─┬─ Q1 (150 V) ═╤═ Q2 (80 V) ─ CMC ─ C1 ─ L1 ─┬─ C2 + C_bulk ─┬─ Buck A 5 V ─┬─ codec LDO (LP5907-Q1)
  (2 A)         │  C_in 100 nF 250 V│  HGATE        │  DGATE (ideal diode)          │  (hold-up)    │  LMR43620-Q1 ├─ isolated supply (jack side, #9)
                │                   └── LM74800-Q1 (common source, OV cut-off 38.5 V)│              │              └─ Buck B 3.3 V ─ ESP32-S3, hub, codec (LDO)
IGN / radio-on ─┴─ sense network ────── EN/UVLO ◄── USB-C VBUS, firmware HOLD     │              │                 LMR43620-Q1
                                                                                  TPS3710-Q1 brownout (7.0 V) ─► PTT enable gate
                                                  2.304 MHz AEC-Q100 oscillator ─► SYNC of Buck A and Buck B
```

## 4. Protection chain

### 4.1 Fuse

- **Littelfuse 0437002.WRA** (437A series, 1206, 2 A fast-acting, **63 V**,
  AEC-Q200; [datasheet](../references/index.md#littelfuse-437-ds)); interrupting
  rating 50 A at 63 V per the LCSC listing **(verify** the table grouping for 2 A).
  63 V covers the jump start (26 V) with more than 2× margin.
- 2 A against a worst-case input of 0.40 A at 6 V: the datasheet recommends
  continuous operation at ≤ 80 % of rating; temperature derating at 85 °C
  **(verify** against the 437A derating curve).
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
| TVS only, SMBJ/SMCJ class | Cheap, passive | Test A energy is **47–173 J per pulse** (table below) against about 0.87 J for a 600 W SMBJ; fails test A. Clamp of about 45–53 V also exceeds the bucks' 42 V absolute maximum | [Vishay SMBJ](../references/index.md#vishay-smbj-ds) |
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
  stays under the bucks' 42 V absolute maximum. Turn-off deglitch is 3.98–5.4 µs.
- **Q1: Diodes DMTH15H017SPSWQ**, 150 V, AEC-Q101, TJ −55 to +175 °C, IDSS
  ≤ 1 µA at 120 V ([datasheet](../references/index.md#diodes-dmth15h017spswq-ds)).
  Worst VDS: pulse 2a/3b input, clamped by the TVS stack at ≤ about 127 V
  (sum of VBR max) plus dynamic rise; ≥ 15 % margin. Q1 only switches, so
  RDS(on) and SOA are not critical.
- **Q2: Vishay SQSA80ENW**, 80 V, AEC-Q101, TJ −55 to +175 °C, 21 mΩ
  ([datasheet](../references/index.md#vishay-sqsa80enw-ds)). During pulse 1
  it blocks the hold-up voltage plus the negative clamp: 16 V + 44 V + TVS
  forward drop ≈ 64 V, under 80 V. (TI's 12 V example uses 60 V with a 44 V
  clamp, which leaves no margin for a 16 V output.)
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
  connector, before Q1.

### 4.5 Pulse by pulse

| Pulse | What happens | Energy / stress | Expected status |
|---|---|---|---|
| Pulse 1, −150 V, 10 Ω, 2 ms | D4 clamps at ≈ −44 V; Q2 blocks within 0.5 µs; hold-up keeps the rails | Ipk 10.6 A, **0.20 J per pulse** vs ≈ 0.87 J for a 600 W 10/1000 µs rating; 500 pulses at 0.5 s = 0.40 W average vs 5 W PD (SMBJ) | A |
| Pulse 2a, +112 V, 2 Ω, 50 µs | Stack barely conducts (2.6 A, 0.2 mJ at −40 °C); OV cut-off opens Q1 within ≈ 5 µs | Output rise during the deglitch ≈ 2.6 V into the hold-up capacitance | A |
| Pulse 3a / 3b, −220 / +150 V, 50 Ω, 150 ns | C_in absorbs: +150 V into 100 nF raises the node by 4.4 V; D4 takes 3a (3.6 A, 6 µJ) | Negligible | A |
| Jump start 26 V, 60 s; transient 18 V | Below OV cut-off; Buck A on-time still above its minimum at 26 V | — | A |
| Test B, 35 V, ≤ 400 ms | Below OV cut-off; the stack doesn't conduct | 0 J; Buck A briefly below minimum on-time (frequency foldback, §6.3) | A |
| Test A, ≤ 101 V, ≤ 400 ms, 10 × | Q1 opens at 37–40 V; hold-up carries ≈ 2.6 ms, then brownout forces PTT off and the device resets | 0 J in TVS and FETs; VS clamp 0.45 W total for ≤ 400 ms | C |
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

**Differential-mode estimate** (Buck A at 0.42 A, 14 V → 5 V, D = 0.357):

- Fundamental of the input current pulse train:
  I1 = (2/π)·Iout·sin(πD) = **0.24 A peak**.
- Buck-side input capacitance 2 × 2.2 µF 100 V (≈ 1.9 µF each at 14 V bias),
  ESR ≈ 5 mΩ: |Z| = 18.9 mΩ → V1 = 4.5 mV peak = **70.1 dBµV rms**.
- Target 28 dBµV (Class 5 MW average 34 dBµV − 6 dB margin): **≥ 42.1 dB**
  of attenuation needed.
- **L1 = 2.2 µH** (TDK TFM252012ALMA2R2MTAA, AEC-Q200, 2.6 A, 75 mΩ,
  −55 to +150 °C; [datasheet](../references/index.md#tdk-tfm252012alma-ds)):
  XL = 31.8 Ω at 2.304 MHz. **C1 = 2.2 µF 100 V X7R 1210** (TDK
  CGA6N3X7R2A225K, AEC-Q200; [catalog](../references/index.md#tdk-cga-ds)), XC = 36 mΩ:
  **≈ 59 dB** ideal, about 17 dB over the need before parasitics.
- **Corner frequency:** f0 = 1 / (2π√(L1·C1)) = **78 kHz** (L1 with C1), or
  55 kHz with the buck-side 3.8 µF; both are ≥ 30× below fsw.
- **Damping and stability:** filter Z0 = √(L1 / C_buck) = 0.76 Ω. The
  **100 µF 50 V electrolytic** (Panasonic EEE-FK1H101P, AEC-Q200,
  −55 to +105 °C; [datasheet](../references/index.md#panasonic-fk-ds)) at the
  buck side damps the resonance with its ESR and doubles as hold-up (§8).
  The buck's negative input resistance, Vin² / Pin, is 33.8 Ω at 9 V and
  15.0 Ω at 6 V, well above Z0 (Middlebrook criterion met).
- **Common mode:** **TDK ACM70V-701-2PL-TL00**, 700 Ω at 100 MHz, 4 A at
  125 °C, 80 V rated, AEC-Q200, −40 to +125 °C
  ([datasheet](../references/index.md#tdk-acm70v-ds)), placed after Q2 where
  the voltage is ≤ 40 V. It targets the 26–108 MHz bands, where the
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

**Choice: LMR43620-Q1 for both bucks.** It is the only candidate whose sync
range reaches the band-clean 2.304 MHz (§6.2). The LM6x440 family is the
fallback at 2.150 MHz with a smaller margin.

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
- **Clock source:** **SiTime SiT8924B**, AEC-Q100 (grade 1 −40 to +125 °C
  option), any frequency 1–110 MHz, ±20 ppm option, ≤ 4.8 mA
  ([datasheet](../references/index.md#sitime-sit8924b-ds)), ordered at
  2.304000 MHz. Alternate: a standard 18.432 MHz oscillator divided by 8.
  The ESP32-S3 can't make 2.304 MHz by integer division of its 80 MHz or
  40 MHz clocks; a fractional LEDC divider would add jitter spurs.
- **Before sync is present** (the first milliseconds, before the 3.3 V rail
  powers the oscillator), the MC variants (no spread spectrum) free-run at a
  **fixed 2.2 MHz**, specified as 2.1–2.3 MHz
  ([datasheet](../references/index.md#ti-lmr436x0-q1-ds), FSW(2p2MHz)).
  Against the harmonic table, a nominal 2.2 MHz puts only harmonic 13
  (28.6 MHz) inside a band (10 m). Across the full 2.1–2.3 MHz tolerance,
  harmonics 8 (17 m), 10 (15 m), 11 (12 m) and 13 (10 m) can fall in bands.
  This lasts only until the oscillator starts, before any receive or
  transmit. Behavior if the clock is lost later **(verify)**; presumably the same fallback.
- The **isolated jack-side supply (#9) must follow the same rule.** The
  SN6505B-Q1 accepts an external clock of 100–1600 kHz and divides it by 2
  ([datasheet](../references/index.md#ti-sn6505-q1-ds)). Its comb would then
  be 2.304 / k MHz, and the extra harmonics land in bands. Options for #9:
  heavy filtering and shielding of an SN6505B-Q1 stage, or an isolated
  topology switching at 2.304 MHz itself (for example a synchronized
  Fly-Buck; suitability of the LMR436x0-Q1 **(verify)**).

### 6.3 Rails

- **Buck A: 12 V → 5.0 V, LMR43620MC5RPERQ1** (MODE/SYNC, 5 V fixed, no
  spread spectrum; alternate LMR43620MSC5RPERQ1). At 2.304 MHz the on-time is
  136 ns at 16 V, 121 ns at 18 V and 83 ns at 26 V, all above the 75 ns
  maximum tON-MIN. At 35 V (test B) it is 62 ns: the part folds back in
  frequency for the duration, so harmonics move for ≤ 400 ms. The maximum duty
  cycle from tOFF-MIN (85 ns) is 0.80, so 5 V regulates down to about 6.2 V in;
  below that the part runs in dropout (tON-MAX 6–13 µs, output ≈ Vin − 0.2 V).
- **Buck B: 5 V → 3.3 V, LMR43620MC3RPERQ1** (MODE/SYNC, 3.3 V fixed, no
  spread spectrum; alternate LMR43620MSC3RPERQ1), on the same clock. Its input
  never sees automotive transients, so no foldback (D = 0.66). It keeps
  3.3 V until the 5 V rail falls to about 3.5 V (input ≈ 3.7–3.8 V).
- **No VBUS switch to the radio.** The radio port's VBUS is blocked in
  hardware; see ADR-0003 (#9).
- **Buck A sizing:** 0.42 A worst case uses about a fifth of the
  LMR43620-Q1's 2 A. The pin-compatible **LMR43610MSC5RPERQ1** (1 A;
  TI.com 2.850 USD at 100–249, 3,843 in stock, 2026-09-24) would do. The 2 A
  part is kept so that both bucks share one part family, and for headroom
  if #9's isolated supply grows.
- **Codec analog rail: LP5907-Q1** 3.3 V LDO from the 5 V rail (AEC-Q100
  grade 1, 2.2–5.5 V input, 250 mA, < 6.5 µV rms, 82 dB PSRR at 1 kHz, 120 mV
  typical dropout; [datasheet](../references/index.md#ti-lp5907-q1-ds)). #8
  decides. The shortlist's [TPS7A20](../references/index.md#ti-tps7a20-ds) has no -Q1 version at TI.
- The **ESP32-S3 3.3 V** comes straight from Buck B with the decoupling and
  ferrite in Espressif's hardware design guidelines.

### 6.4 Thermal

- **Limit:** LMR436x0-Q1 operating junction temperature −40 to **+150 °C**
  ([datasheet](../references/index.md#ti-lmr436x0-q1-ds), Recommended
  Operating Conditions). Heat is acceptable as long as TJ stays below that.
- Buck A at worst case (P_out 2.1 W, the §2 method): loss ≈ 0.28 W at 88 %.
  ΔT = 14 °C (50 °C/W EVM) to 24 °C (84.4 °C/W JEDEC), so **TJ ≤ about
  109 °C at 85 °C ambient**, 41 °C under the limit.
- **Typical load** (the §2 typical case, ≈ 1.2 W input, 5 V out ≈ 1.0 W):
  Buck A loss ≈ 0.14 W → ΔT 7–11 °C → **TJ ≤ about 96 °C** at 85 °C ambient.
- **Typical with BLE TX capped at the FCC-grant power** (10.3 dBm, #7). The
  datasheet gives 204 mA at +9 dBm and 340 mA at +20 dBm (100 % duty,
  [datasheet](../references/index.md#esp32s3-mini1-ds) Table 6-5), so about
  0.2 A. The 3.3 V rail is then ≈ 0.27 A (0.9 W), 5 V out ≈ 1.4 W, input
  ≈ 1.6 W. Buck A loss ≈ 0.19 W → ΔT 9–16 °C → **TJ ≤ about 101 °C**; Buck B
  loss ≈ 0.12 W → ΔT 6–10 °C.
- Buck B at worst case: ≈ 0.21 W → ≤ 18 °C rise.
- Q1/Q2 conduction: (22 + 21) mΩ × 0.27 A² ≈ 3 mW. L1, CMC and F1 together
  < 20 mW.
- The electrolytic's life at 105 °C is 2000–5000 h depending on size; at
  85 °C ambient expect roughly 4× that **(verify** against the series table;
  a hybrid polymer EEH-ZA gives 10,000 h at 105 °C).

## 7. Cold-crank decision

**Decision: no buck-boost. Accept brownout with PTT forced off.**

- Normal cold crank (4.5 V for 15 ms, then 6.5 V) keeps the 3.3 V logic
  alive: Buck A drops out to Vin − 0.2 V and Buck B needs about 3.5 V. PTT is
  off below 7.0 V anyway (§8). Severe crank (3 V) resets the device; PTT is off
  by the hardware default.
- A buck-boost would only add logic uptime between about 3.8 V and 3 V. It
  would cost a second converter, a second frequency to place, and money. The
  radio itself generally isn't specified to operate that low; most mobile HF
  rigs are specified around 13.8 V ± 15 % **(verify per radio, #5)**.

## 8. Brownout detector, PTT fail-safe and hold-up

- **TPS3710-Q1** (AEC-Q100 grade 1, 1.8–18 V supply, 400 mV reference ±1 %
  over temperature, 5.5 µA, open-drain; tpd(HL) 18 µs;
  [datasheet](../references/index.md#ti-tps3710-q1-ds)) senses the protected
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
- **Hold-up** (100 µF − 20 % + 3 × 1.9 µF ≈ 85.7 µF; t = C·(V1² − V2²) / 2P):

| From → to | 2.4 W (worst) | 1.2 W (typical) | Meaning |
|---|---|---|---|
| 7.0 V → 3.8 V | **618 µs** | 1.28 ms | Time from PTT-off to logic dropout; detector path ≈ 20 µs |
| 14 V → 7.0 V | 2.6 ms | 5.4 ms | Supply interruptions shorter than this don't disturb PTT |

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
- **Power-down:** SENSE falls → firmware drops PTT, finishes, waits a
  configurable delay, releases HOLD → EN low → Q1/Q2 off → all rails
  collapse. A hung MCU is reset by the watchdog, and its HOLD pin defaults low,
  so it can't keep the device on. Firmware may also power down on low battery
  (for example < 11.5 V for 60 s with SENSE off; threshold TBD).
- **Off-state drain** at 12.6 V, EN low:

| Contributor | Current |
|---|---|
| LM74800-Q1 shutdown, I(GND) incl. VS | 2.87 µA typ, **5 µA max** (TJ −40 to +125 °C) |
| TVS stack leakage (IR ≤ 1 µA at VRWM; here at 12.6 V against 103 V VRWM) | ≤ 1 µA |
| Q1 leakage (IDSS ≤ 1 µA at 120 V, 25 °C) | ≤ 1 µA |
| SENSE, VBUS, HOLD networks, OV divider (disconnected by the SW pin), TPS3710-Q1, buck dividers | 0 (unpowered) |
| **Total** | **≤ 7 µA at 25 °C**, over 140× below the 1 mA limit. FET and TVS leakage at 85 °C **(verify)** |

## 10. Parts, qualification, price and stock

Prices in USD, checked 2026-09-24. LCSC: price at the tier containing
quantity 100. TI.com: 100–249 tier. Digi-Key and Mouser **blocked automated
lookups** on 2026-09-24, so those cells are blank; they must be filled before
ordering. The TI parts are listed as **Active** on ti.com; lifecycle for other
parts is **(verify)**.

| Ref | Part | Grade / temp (datasheet) | LCSC (# / price / stock) | TI.com (price / stock) | Digi-Key / Mouser |
|---|---|---|---|---|---|
| F1 | Littelfuse 0437002.WRA | AEC-Q200; 63 V | C720199 / 0.1622 / 5,130 | — | blank |
| D3 | Littelfuse TPSMB82A | AEC-Q101; TJ −65 to +175 °C | C3704846 / 0.1833 / 2,980 | — | blank |
| D4 | Vishay SMBJ33CAHE3_B/H | AEC-Q101 (HE3); TJ −55 to +150 °C | C20037758 / 0.1367 / 9,185 | — | blank |
| C_in | YAGEO AS1206KKX7RYBB104 (100 nF 250 V) | AEC-Q200 | C3881218 / 0.1200 / 5,000 | — | blank |
| U1 | TI LM74800QDRRRQ1 | AEC-Q100 grade 1 | C3215600 / 1.8652 / 2,276 | 2.074 / 28,516 | blank |
| Q1 | Diodes DMTH15H017SPSWQ-13 | AEC-Q101; TJ −55 to +175 °C | C19950019 / 5.9072 / 10 | — | blank |
| Q2 | Vishay SQSA80ENW-T1_GE3 | AEC-Q101; TJ −55 to +175 °C | C511563 / 0.6198 / 1,476 | — | blank |
| FL1 | TDK ACM70V-701-2PL-TL00 | AEC-Q200; −40 to +125 °C | C76582 / 0.5310 / 6,692 | — | blank |
| L1 | TDK TFM252012ALMA2R2MTAA | AEC-Q200; −55 to +150 °C | C404804 / 0.2020 / 4,300 | — | blank |
| C1, C2 | TDK CGA6N3X7R2A225KT0Y0U (2.2 µF 100 V) | AEC-Q200; X7R −55 to +125 °C | C342652 / 0.1978 / 15,660 | — | blank |
| C_bulk | Panasonic EEE-FK1H101P (100 µF 50 V) | AEC-Q200; −55 to +105 °C | C178548 / 0.3593 / 8,402 | — | blank |
| U2, U3 | TI LMR43620MC5RPERQ1 / MC3RPERQ1 (MODE/SYNC, no spread spectrum) | AEC-Q100 grade 1 | C32594901 / 3.7734 / 5; C41658611 / — / 10 | 2.944 / 3,103; 2.494 / 3,500 | blank |
| U2, U3 alt | TI LMR43620MSC5RPERQ1 / MSC3RPERQ1 (adds spread spectrum) | AEC-Q100 grade 1 | C6979910 / 4.457 / 2; C3190193 / 4.8281 / 36 | 2.944 / 1,686; 2.944 / 3,000 | blank |
| Y1 | SiTime SiT8924B at 2.304 MHz (programmed) | AEC-Q100 grade 1 option | Not stocked at 2.304 MHz (8 MHz version C401144: 2.9584 / 24) | — | blank (programmable at distributors) |
| U5 | TI TPS3710QDSERQ1 | AEC-Q100 grade 1 | C2863756 / 1.0411 / 0 | 1.563 / 73 | blank |
| U6 | TI LP5907QMFX-3.3Q1 | AEC-Q100 | C130005 / 0.4393 / 7,268 | 0.460 / 16,881 | blank |
| (U7) | TI SN6505BQDBVRQ1 (isolated supply, #9) | AEC-Q100 grade 1 | C1849490 / 0.7522 / 4,745 | 1.820 / 18,670 | blank |

Stock figures are recorded as data only; the maintainer does not treat low
stock as a concern (2026-09-24). None of the parts above is
a JLCPCB Basic part, so each adds an extended-part fee
([JLCPCB FAQ](../references/index.md#jlcpcb-pcba-faqs)). Alternates to check
in schematic work: Yageo AC1210KKX7R0BB225 (C1/C2), Murata PLT5BPH5013R1SNL
(FL1), TDK CLF5030NIT-2R2N-D (L1), Diodes DMN15H310SK3Q (Q1, **(verify)**
grade). Minor parts (VS clamp zener, SENSE network resistors and zener,
Schottky OR diodes, dividers) get AEC-Q parts at schematic capture.

## 11. Capacitor voltage ratings

Per [pcb-fabrication.md §6.3](../requirements/pcb-fabrication.md#63-voltage-rating-and-dc-bias)
(≥ 2× nominal and ≥ worst-case transient):

| Node | Worst case | Rating used |
|---|---|---|
| Connector to Q1 (before protection) | Test A 101 V; stack clamp up to ≈ 127 V on fast pulses | **250 V** (C_in); 2 × 101 V = 202 V |
| After Q2 (filter, hold-up, Buck A input) | OV cut-off 37–40 V | **100 V** MLCC (2 × 40 V = 80 V → next standard 100 V); 50 V electrolytic (the 2× rule is for MLCC) |
| 5 V rail | Regulator overshoot | 16 V (2 × 5 V = 10 V → next standard rating 16 V) |
| 3.3 V rail | Regulator overshoot | 10 V |

The DC-bias loss of the 100 V 1210 X7R at 14 V is small; 1.9 µF effective was
assumed in §5 **(verify** against the TDK DC-bias curve).

## 12. Layout guidance (for later KiCad work)

- **Zones in order along one edge:** connector → F1/TVS/C_in → Q1/Q2/U1 →
  CMC → pi filter → bucks. Keep the unfiltered input copper short and away
  from everything else. Nothing noisy may route under or beside the input
  zone, or it couples straight past the filter.
- **Hot loops:** each buck's input capacitors (100 nF 0402 plus 2.2 µF)
  directly at VIN/PGND on the same layer, as in the LMR436x0-Q1 layout
  example; switch node as small as possible; inductor next to the SW pin;
  no vias in the hot loop.
- **Ground:** a solid bottom-layer ground under the whole power section;
  stitch the top pours around the bucks with vias. Don't split ground. Return
  the TVS stack directly to the connector ground with a short, wide path.
- **High voltage:** ≥ 0.5 mm clearance on nets that can exceed 50 V
  (connector to Q1, the VS clamp); trace widths per IPC-2221 for 1 A at
  ≤ 10 °C rise (pcb-fabrication §3).
- **Sync clock:** short trace from the oscillator to both MODE/SYNC pins,
  over ground, away from the audio and RF sections. A series resistor
  (22–47 Ω) slows edges.
- **Separation from the radio side:** the power section sits away from the
  AUDIO/SERIAL jacks and the antenna keep-out of the ESP32-S3-MINI-1. The
  isolated supply's transformer straddles the isolation gap; keep the
  isolation barrier clear.
- **Thermal:** a large copper area and thermal vias under Buck A (§6.4).
- **Shielding:** reserve footprint space for a board-level shield can over
  the two bucks and the oscillator, fitted only if the 6 m / VHF scan needs it.
- **Test points:** protected rail, 5 V, 3.3 V, SYNC, PTT enable, brownout
  output, for the pulse and EMC tests.

## 13. Parts common with variant R (#11)

- **Same buck family, same clock:** LMR43620-Q1 for 5 V and 3.3 V, locked to
  the same 2.304 MHz oscillator. The harmonic analysis (§6.2) then covers
  both variants. The radio accessory input of variant R (11–15 V) is well
  inside the LMR43620-Q1 range.
- **Same TVS and ideal diode:** SMBJ33CA-HE3 and LM74700-Q1 or LM74800-Q1
  (common drain, 60–80 V FETs, no 150 V FET needed without unsuppressed load
  dump) for the accessory input's reverse-polarity and TVS protection.
- **Same TPS3710-Q1** brownout detector and **LP5907-Q1** codec LDO. Neither
  variant has a VBUS switch to the radio (ADR-0003, #9).
- **Variant M only:** the 150 V Q1, TPSMB82A, OV cut-off, CMC and the
  ignition/auto power-down network.

## 14. 24 V trucks (future)

Not supported in revision A (issue default). A 24 V variant would face test A
at 151–202 V with Ri 1–8 Ω, 100–350 ms
([Microchip](../references/index.md#mchp-ds00006186) Table 2-4), a 58 V
suppressed load dump, and a −26 V reverse battery. The LM74800-Q1
common-source design scales to it (TI's §10.3 example is exactly 200 V / 24 V,
with a 200 V Q1), but the bucks would need a 60–65 V input part and new TVS
values. Record it as a separate variant if there is demand.

## 15. Open items

- **Bench tests (human-task, later):** ISO 7637-2 pulses 1/2a/3a/3b at
  level IV; ISO 16750-2 test A (both corners) and test B; jump start 26 V;
  cold crank normal and severe (confirm the logic survives 4.5 V); ESD
  ISO 10605; CISPR 25 conducted and radiated pre-scan; receiver noise-floor
  check on every band 160 m–6 m with the radio next to the device.
- **(verify):** ISO 10605 levels; 2023 reversed-voltage test cases;
  long-term and transient overvoltage values (from one secondary source);
  buck efficiencies and TJ on 2 layers; MLCC DC bias; TVS and FET leakage at
  85 °C; 56 V zener part; electrolytic life at 85 °C;
  PhotoMOS turn-off time (#9).
- **REQ-PWR-014** says 24 V; ISO 16750-2:2023 says 26 V. #4's owner should
  update it to 26 V.
- **Decisions for other issues:** isolated jack-side supply frequency (#9);
  the hub and codec currents (#8, #9); whether variant R adopts the
  2.304 MHz clock (#11).

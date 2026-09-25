<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0004: Variant M 12 V power front end: LM74800-Q1 load-dump cut-off, single-stage 3.3 V buck synchronized at 2.304 MHz, brownout with PTT forced off

- **Status:** proposed
- **Date:** 2026-09-25
- **Issue:** #10

## Context

Variant M runs from a vehicle's 12 V system and sits next to an HF receiver.
It has to survive the vehicle transients, not raise the receiver's noise
floor, keep PTT off whenever power is marginal, and draw almost nothing when
the vehicle is off. Constraints: [`constraints.md`](../requirements/constraints.md)
§3.2 (automotive input), §3.4 (power budget), §5 (RF environment), §6
(PTT fail-safe, isolation), §8 (clock accuracy), §11 (−40 to +85 °C), §12 and
[`pcb-fabrication.md`](../requirements/pcb-fabrication.md) (JLCPCB, 2 layers,
MLCC 2× rule). Requirements: REQ-PWR-003, -005, -010 to -017, REQ-EMC-004 to
-007 and REQ-ENV-002 ([`requirements.md`](../requirements/requirements.md)).
Loads follow [ADR-0008](ADR-0008-host-links-esp32-s3.md): ESP32-S3-MINI-1
(0.34 A peak), USB hub, codec and the isolated jack-side supply, all on 3.3 V.
**The device supplies no power to the radio.** The radio port's VBUS is
blocked in hardware (maintainer decision 2026-09-24; ADR-0003,
[#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9)), and the
device never sources power out of any port (USB-C is a sink).

Maintainer direction (2026-09-24):
- Optimize for spurious emissions and cost.
- AEC-Q is not required, except where it costs about the same or matters in
  the input protection chain.
- The product must meet EU requirements
  ([#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59)).

The test levels were confirmed from app notes that reproduce ISO 16750-2:2023
and ISO 7637-2:2011, plus the official ISO previews for editions and clauses
([ISO 16750-2:2023](../references/index.md#iso-16750-2-2023-preview),
[ISO 7637-2:2011](../references/index.md#iso-7637-2-2011-preview);
[research §1](../research/power-automotive.md#1-test-levels)). Design
levels:
- load dump test A 79–101 V (Ri 0.5–4 Ω, ≤ 400 ms, 10 pulses) and test B
  ≤ 35 V ([Microchip DS00006186A](../references/index.md#mchp-ds00006186));
- ISO 7637-2:2011 level IV pulses 1 / 2a / 3a / 3b = −150 / +112 / −220 /
  +150 V ([onsemi TND6424](../references/index.md#onsemi-tnd6424));
- reverse −14 V for 60 s;
- **jump start 26 V for 60 s in the 2023 edition** (24 V before);
- cold crank to 4.5 V (normal) or 3 V (severe).

Full analysis, calculations, prices and stock:
[`docs/research/power-automotive.md`](../research/power-automotive.md).

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| A. Reverse diode + TVS only (SMBJ/SMCJ) | Cheapest, passive | Test A puts 47–173 J per pulse into the TVS (a 600 W SMBJ takes about 0.9 J); its clamp exceeds the buck's 42 V absolute maximum | [Vishay SMBJ](../references/index.md#vishay-smbj-ds), research §4.3 |
| B. Load-dump TVS (SLD/SM8S class) + ideal diode | Passive load-dump handling | Needs two series devices for the 79 V / 0.5 Ω corner (58 A vs 38.4 A rating); 50 V clamp still over 42 V; large package | [Littelfuse load-dump note](../references/index.md#littelfuse-tvs-load-dump-an) |
| C. Linear surge stopper (LTC4380, LM5060-Q1 class) | Keeps running through test A | MOSFET SOA design for 10 × 400 ms pulses; LTC4380 datasheet not verified (blocked); separate reverse protection | [LTC4380](../references/index.md#adi-ltc4380-ds), [LM5060](../references/index.md#ti-lm5060-ds) |
| **D. LM74800-Q1 common source (150 V blocking FET, OV cut-off) + TVS stack with breakdown above 101 V** | One IC does reverse polarity, reverse-current blocking and load-dump cut-off; **0 J** in the TVS and FETs during test A and B; 5 µA max shutdown; the FETs only switch, so small commercial SOT-23 parts do | Resets during a test A pulse (functional status C); needs a 150 V FET and a VS clamp | [LM7480-Q1](../references/index.md#ti-lm7480-q1-ds) §10.3, [TPSMB](../references/index.md#littelfuse-tpsmb-ds) |
| Two bucks (12 → 5 V → 3.3 V) | Keeps logic further from input transients | Nothing needs 5 V any more; two converters to place and filter; about $4.5 more | research §6.3, §10 |
| **One buck, protected input → 3.3 V** (LMR43620MC3RPERQ1) | One converter and one hot loop; synchronized from 4.2 V to 19.1 V input; regulated to ≈ 3.5 V; 92 % at full load | Frequency folds back above 19.1 V (26 V jump start, 35 V test B) | [LMR436x0-Q1](../references/index.md#ti-lmr436x0-q1-ds) |
| Buck at a default frequency (2.1 / 2.2 MHz, spread spectrum) | Standard, no clock part | 2.1 MHz × 14 and 2.2 MHz × 13 land inside 10 m; spread spectrum smears harmonics into the bands | [47 CFR 97.301](../references/index.md#ecfr-47-97-301), research §6.2 |
| **Buck synchronized at 2.304 MHz** from 18.432 MHz ÷ 8 | Every harmonic from 160 m to 10 m at least 88 kHz outside the US bands; the clock also serves the codec (ADR-0002); about $0.86 | Three small logic parts; before sync the buck free-runs at 2.2 MHz for a few ms | [YXC](../references/index.md#yxc-yso110tr-ds), [SN74LVC1G80](../references/index.md#ti-sn74lvc1g80-ds) |
| Buck-boost for cold crank | Logic survives the severe 3 V crank | Second converter and frequency, cost; the radio itself isn't specified that low **(verify, #5)** | research §7 |
| **Brownout with PTT forced off** | Simple; logic and codec supply survive the normal crank; PTT is off below 7 V by hardware | Device resets in a severe crank | [TPS3710](../references/index.md#ti-tps3710-ds) |

## Decision

1. **Protection (option D):**
   - Input side: a 2 A 63 V fuse (Littelfuse 0466002.NRHF), then the
     anti-series TVS stack **TPSMB82A + SMBJ33CA-HE3** (positive breakdown
     ≥ 106.9 V at −40 °C, negative clamp ≈ −44 V) and 100 nF 250 V
     soft-termination at the connector.
   - Controller: **LM74800-Q1** in common-source topology.
     - **Q1:** 150 V, [FDN86246](../references/index.md#onsemi-fdn86246-ds).
     - **Q2:** 100 V, [IRLML0100](../references/index.md#infineon-irlml0100-ds).
     - VS clamped by 10 kΩ + 56 V zener.
     - **Overvoltage cut-off at 38.5 V nominal (37–40 V).**
   - **AEC-Q parts kept** only in this chain: the TVS pair (it takes every
     transient; the HE3 costs about the same as commercial), the
     soft-termination C_in (a flex crack would short the battery lead), and
     the LM74800-Q1 (the only stocked version). Everything else is commercial,
     rated for −40 to +85 °C or wider.
2. **Filter:** a Murata DLW5BTM142TQ2L common-mode choke, then a
   2.2 µF / 2.2 µH / 2.2 µF pi filter (corner ≈ 78 kHz, ≈ 59 dB ideal at fsw
   against 42 dB needed), with a 100 µF 50 V electrolytic for damping and
   hold-up. Designed against CISPR 25 Class 5
   ([TI SLYY136](../references/index.md#ti-slyy136)); it also covers
   EN 301 489-1's DC-port conducted limits (73/60 dBµV) by a wide margin.
3. **Regulation, the shared 3.3 V regulator core:**
   - **One LMR43620MC3RPERQ1** (3.3 V fixed, MODE/SYNC, no spread spectrum)
     takes the protected input directly to 3.3 V, in FPWM, with 2.2 µH and
     2 × 22 µF. There is no 5 V rail.
   - It is synchronized from 4.2 V to 19.1 V input and regulated to about
     3.5 V.
   - Above 19.1 V it folds back in frequency (≈ 1.7–1.95 MHz at the 26 V jump
     start, ≈ 1.3–1.45 MHz at the 35 V test B).
   - Before the clock starts it free-runs at a fixed 2.2 MHz (2.1–2.3 MHz)
     for a few ms.
   - Worst-case load is 0.57 A (2.1 W input), typical about 1.1 W.
   - TJ is ≤ about 99 °C at 85 °C ambient against the 150 °C limit.
   - **Variant R (#11) reuses this exact core** (regulator, inductor, output
     capacitors and clock).
   - Cheaper drop-ins must be automotive-grade (maintainer, 2026-09-25).
     The only AEC-Q100 1 A drop-in, LMR43610MSC3RPERQ1, has no
     no-spread-spectrum variant and is not cheaper ($2.85 vs $2.494 at
     TI.com, 2026-09-25), so **LMR43620MC3RPERQ1 is kept for both variants**.
   - The commercial LMR43610MB3RPER was not adopted: it is not AEC-Q100.
4. **Clock, shared with ADR-0002 and ADR-0005 (the shared-clock
   recommendation, research §6.2):**
   - Source: an 18.432 MHz CMOS oscillator (YXC OT322518.432MJBA4SL:
     ±20 ppm over −40 to +85 °C, 0.7 ps phase jitter max), divided by 8
     with three SN74LVC1G80 flip-flops, giving **2.304 MHz**.
   - It drives up to three loads through 33 Ω series resistors in a star:
     the buck's MODE/SYNC, the TLV320AIC3104 MCLK (PLL to exactly 48 kHz,
     ADR-0002, PR #55) and the #9 isolated-supply clock if used.
   - The codec input levels (VIH ≥ 0.7 × IOVDD at 3.3 V) are met.
   - The codec takes 2.304 MHz through its PLL, not 18.432 MHz directly,
     so the 18.432 MHz net stays under 5 mm. Its 8th harmonic (147.456 MHz)
     falls in 2 m.
   - A direct 2.304 MHz MEMS oscillator is the drop-in upgrade if it can be
     bought at or below the ≈ $0.86 cost.
5. **Codec analog supply:** an LP5907-3.0 from the 3.3 V rail (ADR-0002 /
   #8). It needs the rail at ≥ about 3.1 V.
   - The rail is 3.27–3.33 V whenever the buck is synchronized (input
     ≥ 4.2 V, including the whole normal cold crank).
   - It stays ≥ 3.135 V in dropout down to about 3.5 V input.
6. **Cold crank and brownout:** no buck-boost; brown out safely.
   - A **TPS3710** (commercial, or -Q1 if cheaper at order time) on the
     protected rail pulls **PTT enable** low below **7.0 V**. That threshold
     was kept after rechecking it for the wider working range: it sits above
     the 6.5 V crank plateau.
   - That gives ≥ 747 µs before logic dropout at worst-case load.
   - Supply interruptions up to 3 ms (at 2.1 W) are ridden through.
7. **Power-down:** ignition or radio-on SENSE, USB-C host VBUS and a firmware
   HOLD line are diode-ORed into the LM74800-Q1 enable. Maintainer decisions
   (2026-09-25):
   - **Auto power-down 30 s after the radio or ignition turns off**
     (default, configurable).
   - **0 = never**, at the user's choice. Variant M then exceeds the < 1 mA
     off-state target.
   - The device **stays awake while a USB host is connected** on USB-C.
   - Off-state drain when powered down is **≤ 7 µA** at 25 °C (limit 1 mA).
   - There is no external hardware PTT timer; lock-up protection is the
     ESP32-S3's internal watchdog.
8. **24 V trucks:** not in revision A; the same topology scales (research §14).

Why:
- Option D is the only one that handles unsuppressed load dump without
  absorbing its energy anywhere. It is also the lowest-part-count reverse
  protection with the fast reverse-current blocking that the hold-up and PTT
  fail-safe depend on.
- The synchronized 2.304 MHz clock is the only way found to keep every
  switching harmonic out of the HF amateur bands, which constraints §5
  requires.
- A single 3.3 V stage removes a converter, a frequency source and cost
  (power-section main parts about $23.56 → $10.95 at LCSC, qty 100).

## Consequences

- **constraints.md §3.2:** jump start is now 26 V for 60 s (ISO 16750-2:2023),
  with the editions and research doc cited; REQ-PWR-014 should follow (26 V).
  **§3.4:** the radio VBUS row is removed, since the device supplies no power
  to the radio (ADR-0003, #9). Variant M's 2.1 W worst case meets the 3 W peak
  target (REQ-PWR-003). **pcb-fabrication.md §6.3:** capacitors before the
  protection are rated 250 V, after it 100 V.
- **Variant R (#11)** reuses the shared 3.3 V core and clock. It also reuses
  the SMBJ33CA-HE3, the LM74700-Q1 or LM74800-Q1 (common drain, no 150 V FET)
  and the TPS3710.
- **ADR-0002 (#8):** the 2.304 MHz oscillator is shared. It is the codec
  MCLK and must stay ±20 ppm with low jitter; any change to the clock source
  needs both ADRs updated. The codec rail is an LP5907-3.0 from 3.3 V.
- **#9:** the isolated jack-side supply runs from 3.3 V and must keep its
  harmonics out of the bands. An SN6505B-Q1 (external clock ≤ 1.6 MHz,
  divided by 2) can't share the 2.304 MHz comb, so it needs filtering and
  shielding, or another topology. The brownout detector's PTT enable gates
  the PhotoMOS LED drive in hardware. There is no external watchdog or PTT
  timer; the ESP32-S3's internal watchdog handles lock-ups.
- **#59 (EU):**
  - Every chosen part is RoHS-compliant per its distributor listing; REACH
    SVHC is still open.
  - EN 301 489-1 V2.2.3 clause 9.6 applies ISO 7637-2:2004 level III pulses;
    the level IV design covers them (the 2004 level mapping and pulse 4 level
    are marked (verify)).
  - Whether UN ECE R10 applies is #59's question.
- **#5:** the radios' low-voltage behavior confirms the cold-crank decision.
- **Firmware:** a power-management task handles the SENSE input, the delayed
  HOLD release, the low-battery shutdown and the brownout interrupt. Each gets
  tests per the PTT fail-safe rule.
- **Risks to verify:**
  - Second-distributor price and stock (Digi-Key/Mouser lookups were
    blocked; stock levels are recorded as data only).
  - The buck's efficiency at 2.304 MHz and 85 °C, and its fold-back
    behavior while synchronized.
  - The divider's additive jitter.
  - Harmonics 22–23 in 6 m, which are unavoidable below 4 MHz and need
    measurement.
  - ISO 10605 ESD levels.
- **Bench tests (later human-task):**
  - ISO 7637-2 level IV pulses, ISO 16750-2 test A/B, jump start, cold
    crank, ESD.
  - A CISPR 25 pre-scan.
  - A receiver noise-floor check from 160 m to 6 m.

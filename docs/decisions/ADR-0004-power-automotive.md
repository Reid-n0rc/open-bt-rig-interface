<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0004: Variant M 12 V power front end: LM74800-Q1 load-dump cut-off, synchronized 2.304 MHz bucks, brownout with PTT forced off

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #10

## Context

Variant M runs from a vehicle's 12 V system and sits next to an HF receiver.
It has to survive the vehicle transients, not raise the receiver's noise
floor, keep PTT off whenever power is marginal, and draw almost nothing when
the vehicle is off. Constraints: [`constraints.md`](../requirements/constraints.md)
§3.2 (automotive input), §3.4 (power budget), §5 (RF environment), §6
(PTT fail-safe, isolation), §11 (−40 to +85 °C), §12 and
[`pcb-fabrication.md`](../requirements/pcb-fabrication.md) (JLCPCB, 2 layers,
MLCC 2× rule). Requirements: REQ-PWR-003, -005, -010 to -017, REQ-EMC-004 to
-007 and REQ-ENV-002 ([`requirements.md`](../requirements/requirements.md)).
Loads follow [ADR-0008](ADR-0008-host-links-esp32-s3.md): ESP32-S3-MINI-1
(0.34 A peak), USB hub, codec and the isolated jack-side supply. **The device
supplies no power to the radio.** The radio port's VBUS is blocked in hardware
(maintainer decision 2026-09-24; ADR-0003,
[#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9)), and the
device never sources power out of any port (USB-C is a sink).

The test levels were confirmed from app notes that reproduce ISO 16750-2:2023
and ISO 7637-2:2011, plus the official ISO previews for editions and clauses
([ISO 16750-2:2023](../references/index.md#iso-16750-2-2023-preview),
[ISO 7637-2:2011](../references/index.md#iso-7637-2-2011-preview);
[research §1](../research/power-automotive.md#1-test-levels)). Design
levels: load dump test A 79–101 V (Ri 0.5–4 Ω, ≤ 400 ms, 10 pulses) and test B
≤ 35 V ([Microchip DS00006186A](../references/index.md#mchp-ds00006186));
ISO 7637-2:2011 level IV pulses 1 / 2a / 3a / 3b = −150 / +112 / −220 / +150 V
([onsemi TND6424](../references/index.md#onsemi-tnd6424)); reverse −14 V for
60 s; **jump start 26 V for 60 s in the 2023 edition** (24 V before); cold
crank to 4.5 V (normal) or 3 V (severe).

Full analysis, calculations, prices and stock:
[`docs/research/power-automotive.md`](../research/power-automotive.md).

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| A. Reverse diode + TVS only (SMBJ/SMCJ) | Cheapest, passive | Test A puts 47–173 J per pulse into the TVS (a 600 W SMBJ takes about 0.9 J); its clamp exceeds the bucks' 42 V absolute maximum | [Vishay SMBJ](../references/index.md#vishay-smbj-ds), research §4.3 |
| B. Load-dump TVS (SLD/SM8S class) + ideal diode | Passive load-dump handling | Needs two series devices for the 79 V / 0.5 Ω corner (58 A vs 38.4 A rating); 50 V clamp still over 42 V; large package | [Littelfuse load-dump note](../references/index.md#littelfuse-tvs-load-dump-an) |
| C. Linear surge stopper (LTC4380, LM5060-Q1 class) | Keeps running through test A | MOSFET SOA design for 10 × 400 ms pulses; LTC4380 datasheet not verified (blocked); separate reverse protection | [LTC4380](../references/index.md#adi-ltc4380-ds), [LM5060](../references/index.md#ti-lm5060-ds) |
| **D. LM74800-Q1 common source (150 V blocking FET, OV cut-off) + TVS stack with breakdown above 101 V** | One IC does reverse polarity, reverse-current blocking and load-dump cut-off; **0 J** in the TVS and FETs during test A and B; 5 µA max shutdown | Resets during a test A pulse (functional status C); needs a 150 V FET and a VS clamp | [LM7480-Q1](../references/index.md#ti-lm7480-q1-ds) §10.3, [TPSMB](../references/index.md#littelfuse-tpsmb-ds) |
| Buck at a default frequency (2.1 / 2.2 MHz, spread spectrum) | Standard, no clock part | 2.1 MHz × 14 and 2.2 MHz × 13 land inside 10 m; spread spectrum smears harmonics into the bands | [47 CFR 97.301](../references/index.md#ecfr-47-97-301), research §6.2 |
| **Buck synchronized at 2.304 MHz** (LMR43620-Q1, sync 0.2–2.5 MHz, FPWM) | Every harmonic from 160 m to 10 m at least 88 kHz outside the US bands (≥ 53 kHz up to ±0.5 % clock error) | Needs an AEC-Q100 oscillator; the LM6x440 family can't sync that high (2.2 MHz max; its only clean point, 2.150 MHz, has 50 kHz margin) | [LMR436x0-Q1](../references/index.md#ti-lmr436x0-q1-ds), [SiT8924B](../references/index.md#sitime-sit8924b-ds) |
| 5 V buck-boost for cold crank | Logic survives the severe 3 V crank | Second converter and frequency, cost; the radio itself isn't specified that low **(verify, #5)** | research §7 |
| **Brownout with PTT forced off** | Simple; logic survives the normal crank; PTT is off below 7 V by hardware | Device resets in a severe crank | [TPS3710-Q1](../references/index.md#ti-tps3710-q1-ds) |

## Decision

1. **Protection (option D):** 2 A 63 V AEC-Q200 fuse (Littelfuse 0437002.WRA)
   → anti-series TVS stack **TPSMB82A + SMBJ33CA-HE3** (positive breakdown
   ≥ 106.9 V at −40 °C, negative clamp ≈ −44 V) and 100 nF 250 V at the
   connector → **LM74800-Q1** in common-source topology with **Q1 150 V**
   ([DMTH15H017SPSWQ](../references/index.md#diodes-dmth15h017spswq-ds)) and
   **Q2 80 V** ([SQSA80ENW](../references/index.md#vishay-sqsa80enw-ds)), VS clamped by 10 kΩ + 56 V
   zener, **overvoltage cut-off at 38.5 V nominal (37–40 V)**.
2. **Filter:** TDK ACM70V-701-2PL common-mode choke, then a 2.2 µF / 2.2 µH /
   2.2 µF pi filter (corner ≈ 78 kHz, ≈ 59 dB ideal at fsw) with a 100 µF
   50 V AEC-Q200 electrolytic for damping and hold-up. Designed against
   CISPR 25 Class 5 ([TI SLYY136](../references/index.md#ti-slyy136)).
3. **Regulation:** **LMR43620-Q1** 12 V → 5 V (Buck A), then a second
   **LMR43620-Q1** 5 V → 3.3 V (Buck B). Both are **synchronized to a
   2.304 MHz AEC-Q100 oscillator** (SiT8924B), in FPWM, with no spread
   spectrum while synchronized. Worst-case load is 0.42 A at 5 V and 2.4 W at
   the input. There is no VBUS switch to the radio (see ADR-0003, #9).
   LP5907-Q1 supplies the codec's analog rail (#8 decides).
4. **Cold crank:** no buck-boost; brown out safely. A **TPS3710-Q1** on the
   protected rail pulls **PTT enable** low below **7.0 V**. That gives
   ≥ 618 µs before logic dropout at worst-case load, and interruptions up to
   2.6 ms (at 2.4 W) are ridden through.
5. **Power-down:** ignition or radio-on SENSE, USB-C host VBUS and a firmware
   HOLD line are diode-ORed into the LM74800-Q1 enable. Off-state drain is
   **≤ 7 µA** at 25 °C (limit 1 mA).
6. **24 V trucks:** not in revision A; the same topology scales (research §14).

Why: option D is the only one that handles unsuppressed load dump without
absorbing its energy anywhere. It is also the lowest-part-count reverse
protection with the fast reverse-current blocking that the hold-up and PTT
fail-safe depend on. The synchronized 2.304 MHz clock is the only way found
to keep every switching harmonic out of the HF amateur bands, which
constraints §5 requires.

## Consequences

- **constraints.md §3.2:** jump start is now 26 V for 60 s (ISO 16750-2:2023),
  with the editions and research doc cited; REQ-PWR-014 should follow (26 V).
  **§3.4:** the radio VBUS row is removed, since the device supplies no power
  to the radio (ADR-0003, #9). Variant M's 2.4 W worst case meets the 3 W peak
  target (REQ-PWR-003). **pcb-fabrication.md §6.3:** capacitors before the
  protection are rated 250 V, after it 100 V.
- **Variant R (#11)** should reuse the LMR43620-Q1 pair on the same 2.304 MHz
  clock, the SMBJ33CA-HE3, the LM74700-Q1 or LM74800-Q1 (common drain, no
  150 V FET), the TPS3710-Q1 and the LP5907-Q1.
- **#9:** the isolated jack-side supply must also keep its harmonics out of the
  bands. An SN6505B-Q1 (external clock ≤ 1.6 MHz, divided by 2) can't share
  the 2.304 MHz comb, so it needs filtering and shielding, or another
  topology. The PTT enable from the brownout detector is ANDed with the
  watchdog gate.
- **#5:** the radios' low-voltage behavior confirms the cold-crank decision.
- **Firmware:** a power-management task handles the SENSE input, the delayed
  HOLD release, the low-battery shutdown and the brownout interrupt. Each gets
  tests per the PTT fail-safe rule.
- **Risks to verify:** thin stock of the LMR43620-Q1, Q1 and TPS3710-Q1
  (Digi-Key/Mouser lookups were blocked, so the second distributor is still
  open); harmonics 22–23 in 6 m, which are unavoidable below 4 MHz and need
  measurement; ISO 10605 ESD levels; buck efficiencies.
- **Bench tests (later human-task):** ISO 7637-2 level IV pulses, ISO 16750-2
  test A/B, jump start, cold crank, ESD, a CISPR 25 pre-scan, and a receiver
  noise-floor check from 160 m to 6 m.

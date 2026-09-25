<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Variant R power: radio accessory DC and USB-C (revision A)

Issue: [#11](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/11).
Decision record: [ADR-0005](../decisions/ADR-0005-power-radio-usbc.md).

Scope: the variant R power input, from the radio's accessory DC pin and the
USB-C port to the 3.3 V rail. It covers protection, source selection,
regulation, the switching frequency, the power budget per mode, radio-on sense,
auto power up/down, parts and cost, and the EU EMC targets. Schematic capture
and layout are out of scope, and so is variant M
([#10](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/10)).

Inputs:

- **Radio DC pins and current limits:** the per-radio table from #5
  ([PR #56](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/56),
  `docs/research/radio-interfaces.md`), which cites the manufacturers' manuals
  page by page. This doc repeats only the figures it needs.
- **Module current:** [ESP32-S3-MINI-1 datasheet](../references/index.md#esp32s3-mini1-ds)
  Tables 6-5 to 6-7. BLE TX is capped at the FCC grant's 10.3 dBm
  ([`module-selection.md`](module-selection.md)), so the +9 dBm figure applies.
- **USB-C host power:** #6 ([PR #54](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/54),
  `docs/research/host-compatibility.md` §2.4) and the
  [USB Type-C specification](../references/index.md#usb-typec-r20).
- **Codec rails:** ADR-0002 from #8 ([PR #55](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/55))
  and the [TLV320AIC3104 datasheet](../references/index.md#ti-tlv320aic3104-ds).
- **Host links:** [ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)
  (accepted). The issue predates it and has no ADR-0008 scope update, so this
  doc applies it: wired USB-C mode with the on-board hub, and isolation of the
  AUDIO and SERIAL jacks whenever the USB-C data link is used.
- **Variant M:** #10 ([PR #57](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/57),
  ADR-0004) owns the **shared regulator core and the 2.304 MHz clock**. This
  doc uses them and specifies only what is particular to variant R.

Conventions: **(verify)** marks a fact or estimate that a bench test or a
later issue must confirm. `unknown` means no primary source was found.

## Maintainer decisions (2026-09-24)

1. **Power flows in only.** The device takes power from the radio's DC
   accessory pin or USB-C (and, on variant M, 12 V). **It never supplies
   power on any port**: it doesn't source VBUS to the radio's USB port in
   either host mode, and it is a sink only on USB-C, in wired mode too.
   Hardware blocks current from the device into the radio's VBUS. That circuit,
   and the matching edits to `constraints.md` and `requirements.md`, belong to
   #9 (ADR-0003). Issue item 5 (a VBUS supply to the radio) and the IC-705
   battery-charging case therefore **no longer apply**. The K4 connects only
   through its USB-B port; its rear USB-A ports are not used for power or data.
2. **Optimize for spurious emissions and cost, and share the regulator core
   with variant M:** one **TI LMR43620MC3RPERQ1** buck (MODE/SYNC, no spread
   spectrum, fixed 3.3 V), synchronized in forced PWM to 2.304 MHz, fed
   directly from the OR of the two inputs. There's no 5 V rail. The input
   protection is sized for a station, not a car. Use the cheapest parts that
   meet −20 to +60 °C and the electrical need; AEC-Q isn't required.
3. **EU requirements (#59):** every part must be RoHS-compliant. EU EMC
   (EN 55032 Class B, EN 301 489-1 immunity) is a design target alongside FCC
   Part 15B (§10).

## Summary

- **Radio DC input (11–15 V nominal, 9–17.5 V accepted):** **0.25 A fast
  fuse** (the current limit: it opens within 5 s at 0.5 A, the K3's pin limit)
  → **B5819W Schottky** (reverse polarity, no back-feed into the radio) →
  **SMBJ15A TVS** → the power mux.
- **USB-C:** 5 V sink, 5.1 kΩ Rd on CC1 and CC2, no USB PD. The firmware reads
  CC to learn the host's current advertisement. Every mode fits the **500 mA**
  Default current.
- **Source selection:** one **TI TPS2121** priority power mux, directly on both
  inputs. Radio DC has priority when it is above 9 V; USB-C is the fallback.
  It controls inrush (soft start), cuts off each input on overvoltage
  (17.5 V and 5.8 V) and blocks back-feed into both sources.
- **Regulation:** the mux feeds one **LMR43620MC3RPERQ1** (3.3 V, 2 A),
  synchronized to the shared **2.304 MHz** clock (ADR-0004: an 18.432 MHz
  oscillator divided by 8 in three flip-flops). No harmonic from 160 m to 10 m lands in an
  amateur band (worst margin 85 kHz). The duty cycle stays inside the part's
  limits from 4.4 V to 17.5 V in. The same 2.304 MHz clock feeds the codec,
  whose PLL makes 48 kHz from it (ADR-0002, PR #55). The
  codec gets an **LP5907-3.0** LDO (analog) and a **TPS7A2018** (core, fed
  from the 3.0 V output), as ADR-0002 specifies.
- **Power budget:** 0.5–1.3 W average, 1.5 W peak. **Wired mode draws about
  276 mA from USB-C at 4.75 V (298 mA at 4.4 V)**, about 200 mA under the
  500 mA Default.
- **Radios that can power variant R:** IC-7300, TS-590SG, K3, K3S and K4
  (documented limits, switched outputs). The Yaesu +13 V pins and the TS-590S
  wait for a bench measurement. The IC-705, KX2, KX3, X6100 and
  FT-817ND/FT-818 use USB-C.
- **Radio-on sense:** the switched DC pin; otherwise the radio's USB attach,
  the SERIAL-jack idle level, or a CAT reply. Radio-powered: the device follows
  the radio. USB-powered: it sleeps after a timeout with no radio and no host.
- **Power-section BOM:** about **$7.9 at LCSC, quantity 100** (§9.2). The buck
  IC alone is $3.99 of that, and LCSC holds only 10 of them.

## 1. Radio DC input

### 1.1 What the radios supply

From #5 (manual page citations there):

| Radio | Pin | Voltage | Documented limit | Switched with radio |
|---|---|---|---|---|
| IC-7300 | ACC pin 8 | 13.8 V | 1 A | Yes |
| TS-590SG | EXT.AT pin 6 "14S" | 13.8 V | 4 A | Yes |
| TS-590S | EXT.AT pin 6 "14S" | 13.8 V | `unknown` | Yes |
| K3 | 12 VDC OUT (RCA) | 13 V no load, **12 V at max load** | **0.5 A** | Yes |
| K3S | 12 VDC OUT (RCA) | 13 V no load, 12 V at max load | 1.0 A | Yes |
| K4 | 12 VDC OUT | 12 V | 1.5 A (self-resetting fuse) | Yes |
| FT-891, FT-991A | TUN/LIN "+13V OUT" | 13 V | `unknown` | `unknown` |
| FT-710 | TUNER/LINEAR pin 1 "+13V" | 13 V | `unknown` | Yes |
| FT-817ND / FT-818 | ACC "+13.8V" | Not stated; radio runs on 8–16 V or battery | `unknown` | `unknown` |

Design limits taken from this table:

- **Current limit below 0.5 A** (the K3, the lowest documented limit).
- **Voltage:** 12 V under load on the Elecraft pins; Icom's supply range of
  13.8 V ± 15 % (up to 15.9 V) passes straight through to the pin.

### 1.2 Protection chain

```text
radio DC ─ F1 0.25 A ─ D1 B5819W ─┬─ D2 SMBJ15A ─┬─ C 1 µF ─ TPS2121 IN1 (priority above 9 V, OV1 17.5 V)
                                  │               │
                                  └─ RADIO_DC_SENSE (divider → RTC GPIO)
USB-C VBUS ─ D5 SMF6.0A ─────────────────────────── TPS2121 IN2 (OV2 5.8 V)
                                                    TPS2121 OUT (soft start) ─ C_in ─ LMR43620MC3 ─ 3.3 V
```

| Function | Part and setting | Basis |
|---|---|---|
| **Current limit** | **Littelfuse 0466.250 (0.25 A, 1206, fast-acting).** It carries 100 % of its rating for at least 4 h and opens within 5 s at 200 % (0.5 A). The device draws at most 0.18 A (at 9 V, §6), 72 % of the rating. Cold resistance 0.691 Ω, melting I²t 0.0022 A²s | [466 Series datasheet](../references/index.md#littelfuse-0466-ds) |
| Reverse polarity, no back-feed | **B5819W** Schottky (40 V, 1 A, SOD-123; JLCPCB Basic). About 0.4 V drop, 70 mW at 0.18 A; the buck doesn't need that headroom from a 12 V input. It also stops USB-C power from reaching the radio's pin, alongside the mux | [LCSC C8598](https://www.lcsc.com/product-detail/C8598.html) |
| Transient | **SMBJ15A**, unidirectional (it sits after the diode). VRWM 15 V; VBR 16.7 V minimum; clamp 24.4 V at 24.6 A. It also clamps the ringing of a hot-plugged ceramic input, which would otherwise reach about twice the supply ([pcb-fabrication §6.1](../requirements/pcb-fabrication.md#61-type)). The clamp at full rated pulse is at the TPS2121's 24 V absolute maximum. The fuse's 0.69 Ω limits the surge current, and real station transients are far smaller, but this is **(verify)** in the surge test (§10) | [Vishay SMBJ](../references/index.md#vishay-smbj-ds) (series data) |
| **Inrush** | The TPS2121's soft start (SS pin) ramps the buck's input capacitors. Before the mux there is only about 1 µF plus the TVS, so a hot-plug puts about C·V²/(2R) = 1 µF × (13.8 V)² / (2 × 0.8 Ω) ≈ **0.00012 A²s** through the fuse, about 5 % of its melting I²t | TPS2121 §8; 466 datasheet |
| Undervoltage / overvoltage | TPS2121 PR1 selects IN1 above **9.0 V** (below that, USB-C if present). OV1 cuts IN1 off above **17.5 V** (1.06 V reference, < ±5 %) | [TPS2121](../references/index.md#ti-tps2121-ds) §7.5 |

**Why a fuse and not an eFuse.** Under the cost rule, the cheapest adequate
limit is a fuse sized so that it opens at the K3's 0.5 A: 0.25 A, 200 % → 5 s.
A 0.25 A fuse also carries the worst normal draw with margin. The fuse opens
only on a device fault, so it needs no reset in normal use. The
electronic alternative is the **TI TPS26600 eFuse** (−60 V reverse polarity,
0.1–2.23 A adjustable limit, dV/dt, UVLO, OVP, reverse blocking, 62 V rating,
[datasheet](../references/index.md#ti-tps2660-ds), LCSC C544399, $1.07). It
would replace the fuse and diode, and its 62 V rating removes the TVS-clamp
margin concern, for about $0.95 more. It is the upgrade path if the EU surge test (§10) or the bench shows the
fuse chain isn't enough.

The connector for the radio DC lead belongs to the board strategy
([#12](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/12)). A
locking 2-pin connector is preferred; a reversed lead does no harm.

## 2. USB-C 5 V sink

| Item | Design | Source |
|---|---|---|
| Rd | **5.1 kΩ ± 1 % from CC1 and CC2 to GND** (0402, C25905). A 5.1 kΩ ± 10 % Rd is the implementation that "can detect power capability" | [Type-C R2.0](../references/index.md#usb-typec-r20) Table 4-25 |
| Current advertisement | The source's Rp sets the CC voltage across Rd. Sink thresholds: **vRd-USB 0.25–0.61 V (Default), vRd-1.5 0.70–1.16 V, vRd-3.0 1.31–2.04 V**, detection thresholds 0.66 V and 1.23 V. Rp values: Default 56 kΩ to 5 V / 36 kΩ to 3.3 V / 80 µA; 1.5 A 22 kΩ / 12 kΩ / 180 µA; 3.0 A 10 kΩ / 4.7 kΩ / 330 µA | Type-C R2.0 Tables 4-24, 4-36 |
| Current at Default | 500 mA for a USB 2.0 port (900 mA or 1,500 mA for USB 3.2 ports). The device is USB 2.0, so plan for 500 mA. USB Type-C Current at 1.5 A or 3.0 A supersedes USB 2.0 and BC 1.2 | Type-C R2.0 Table 4-17; #6 §2.4 (R2.5 §2.3.4) |
| CC sensing | Each CC pin to an ESP32-S3 ADC input through a series resistor. Only one CC pin sees Rp, so the firmware takes the higher of the two and reports Default, 1.5 A or 3.0 A to the host. CC stays at or below 2.04 V (Table 4-25) | ADC range and pins **(verify, #9/#12)** |
| ESD | CC1, CC2: TI TPD1E10B06 (one per line). VBUS: **SMF6.0A** (VRWM 6.0 V, above vSafe5V's 5.5 V maximum). D+/D− ESD belongs to the USB routing block (#9) | [LCSC C48260](https://www.lcsc.com/product-detail/C48260.html), [C123790](https://www.lcsc.com/product-detail/C123790.html) |
| Overvoltage | TPS2121 OV2 cut-off at **5.8 V** (5.53–6.02 V with the reference tolerance). A source may not put more than vSafe5V on VBUS without a PD contract, and this device never negotiates one | TPS2121 §7.5 |
| Inrush | TPS2121 soft start. Keep the VBUS-side capacitance small; USB 2.0 limits it to 10 µF **(verify against the USB 2.0 spec)** | TPS2121 §8 |
| Sink only | Rd only, never Rp; the device never drives VBUS. With radio DC present and no host, the USB-C VBUS pin stays unpowered because the TPS2121 blocks OUT → IN2 | TPS2121 §1 |
| Host detection | A divider from VBUS (before the mux) feeds the hub's VBUS_DET input and an MCU GPIO. Wired mode starts only when a host enumerates the device (ADR-0008) | [USB2422](../references/index.md#microchip-usb2422-ds) pin table |

**Is USB PD needed?** No. The worst case is about 343 mA at 4.4 V (§6), so
the Default 500 mA covers it and 1.5 A isn't needed. PD would add a controller
and firmware for no gain.

Two USB 2.0 rules to check against the USB 2.0 spec, which this repo doesn't
cite yet:

- **Before configuration,** a USB 2.0 device may draw only one unit load
  (100 mA) **(verify)**. The idle draw is about 100 mA average with 216 mA
  peaks (§6). Mitigation: keep the BLE radio and codec off until the host has
  configured the device, or until no host has appeared within a short timeout.
- **USB suspend:** at Default current the USB 2.0 suspend limits apply; at
  1.5 A or 3.0 A the sink may keep drawing current
  ([Type-C R2.0](../references/index.md#usb-typec-r20) §4.6.1.1). Estimated
  suspend draw: hub suspended (425 µA typical), ESP32-S3 in light sleep
  (240 µA), codec and isolated supply off, buck in PFM. That comes to a few mA.
  Whether it meets the USB 2.0 limit is **(verify)**.

## 3. Source selection

| Option | Cost (LCSC, qty 100) | Pros | Cons |
|---|---|---|---|
| Two Schottky diodes (diode-OR) | ≈ $0.05 | Cheapest | Priority goes to the higher voltage, which happens to be the radio, but there's no overvoltage cut-off on USB. **The drop on the USB path (about 0.4 V) pushes the buck past its maximum duty cycle below about 4.8 V of VBUS**, so it folds back in frequency and its harmonics move into the bands (§4) |
| Two ideal-diode controllers (LM74700 class) + N-FETs | ≈ $0.6 + FETs | Low drop | Two controllers and two FETs; no OVP on USB without more parts |
| **TI TPS2121 priority mux** | **$0.70** | One IC: 56 mΩ on both paths, priority (PR1), OVP per input (OV1, OV2), soft start, reverse-current blocking into both inputs, status pin (ST). 2.8–22 V operating | 24 V absolute maximum, near the TVS clamp (§1.2); its lowest current limit (1–2 A) doesn't limit the input, so the fuse does that |

**Chosen: TPS2121** ([datasheet](../references/index.md#ti-tps2121-ds)), IN1 =
radio DC, IN2 = USB-C VBUS:

- **Priority to radio DC** whenever IN1 is above 9 V (PR1 divider). With the
  station supply present, a phone or laptop on USB-C carries data only and its
  battery isn't drained. If the radio switches off, the mux moves to USB-C
  without a reset (TPS2121 switchover 5 µs typical).
- **No back-feed:** the mux blocks current into the unselected input and into
  both inputs when the output is higher. The B5819W is a second barrier for
  the radio's pin.
- **Both present:** radio DC powers the device and USB-C carries data.
- **ST pin** to the MCU, so the firmware knows and reports the source.
- The mux sits directly on the raw inputs (11–15 V and 5 V). That works
  because one buck accepts both (§4). There's no 5 V rail.

## 4. Regulation

| Rail | Part | From | Load |
|---|---|---|---|
| **3.3 V** | **TI LMR43620MC3RPERQ1** (2 A, 3–36 V in, MODE/SYNC, 3.3 V fixed, no spread spectrum), sync 2.304 MHz, FPWM; 4.7 µH shielded inductor | TPS2121 OUT (4.4–17.5 V) | ≤ 0.40 A: module, hub, codec LDOs, USB switches, isolators (side 1), PhotoMOS LED, isolated supply |
| Codec AVDD/DRVDD **3.0 V** | **TI LP5907-3.0** low-noise LDO (< 6.5 µV rms; ADR-0002's choice), TPS7A2030 alternate | 3.3 V | ≈ 11 mA |
| Codec DVDD **1.8 V** | **TI TPS7A2018** | 3.0 V (the AVDD LDO output, per ADR-0002) | ≈ 6 mA (upper estimate) |
| Isolated jack side (only when isolation is fitted) | Push-pull driver plus transformer and LDO (SN6505B class), owned by #9 | **3.3 V** | 0.25 W out, 0.36 W in (allocation) |

Duty-cycle check against the [LMR436x0-Q1 datasheet](../references/index.md#ti-lmr436x0-q1-ds)
§6.5 (the commercial [LMR436x0 datasheet](../references/index.md#ti-lmr436x0-ds)
§7.5 gives the same limits): t_ON-MIN 75 ns max, t_OFF-MIN 85 ns max,
f_SYNC 0.2–2.5 MHz. The MC3 part free-runs at a fixed 2.2 MHz until the
sync clock arrives, with PFM/FPWM selectable and no spread spectrum
(datasheet §4, device comparison table). At
2.304 MHz the period is 434 ns.

| Input | D ≈ 3.3 V / V_IN | On-time | Limit | Result |
|---|---|---|---|---|
| Radio DC 9 V (priority threshold) | 0.37 | 159 ns | ≥ 75 ns | OK |
| Radio DC 13.8 V | 0.24 | 104 ns | ≥ 75 ns | OK |
| Radio DC 17.5 V (OV1 cut-off) | 0.19 | 82 ns | ≥ 75 ns | OK, small margin |
| Surge above about 19 V | < 0.17 | < 75 ns | — | Folds back in frequency for the length of the transient only |
| USB 5.0 V | 0.66 (+ drops ≈ 0.68) | — | ≤ 0.80 (off-time ≥ 85 ns) | OK |
| USB 4.75 V | ≈ 0.71 | — | ≤ 0.80 | OK |
| USB 4.4 V | ≈ 0.77 | — | ≤ 0.80 | OK, small margin |
| Below about 4.2 V | > 0.80 | — | — | Off-time limit: the frequency drops and the harmonics move **(verify)** |

The resistive drops (TPS2121 56 mΩ, inductor about 78 mΩ, and the buck's
switches) add a few hundredths to D at 0.3 A. **A USB source that sags below
about 4.2 V under load** moves the switching frequency off 2.304 MHz. Such a
source is outside the USB 2.0 VBUS range at the device **(verify)**. The
firmware can detect it (ADC on VBUS) and report it.

**Why the codec rail is 3.0 V:** without a 5 V rail, a 3.3 V LDO has no
headroom. The codec's AVDD and DRVDD accept 2.7–3.6 V
([datasheet](../references/index.md#ti-tlv320aic3104-ds) §7.3), and ADR-0002
chose 3.0 V from an LP5907-3.0. Its PSRR at audio frequencies isolates the
codec from the module's BLE current pulses, which recur at audio-band rates.

**Headroom on the USB-C path:** ADR-0002 needs the 3.3 V rail at about 3.1 V
or more for the LP5907. The buck holds 3.3 V at its 2.304 MHz setting down to
about 4.2–4.4 V in (the duty-cycle table above). The mux adds about 20 mV at
0.35 A (56 mΩ). Below that, the buck stretches its off-time and then runs in
dropout (on-time up to 6–13 µs, t_ON-MAX), so the rail tracks the input minus
the switch and inductor drops, roughly 0.1 V at 0.35 A **(verify)**. The rail
stays at 3.1 V or more down to about 3.3 V at the USB-C connector, far below
the USB 2.0 range. The TPS7A2030 alternate would need about 5 mV of dropout at
11 mA ([TPS7A20](../references/index.md#ti-tps7a20-ds), 140 mV maximum at
300 mA). The limit on the USB path is the frequency shift below about 4.2 V,
not the codec's headroom.

**Brownout:** no separate supervisor. The ESP32-S3's brownout reset and the
hardware-default-off PTT output (REQ-PTT-008: a pull-down, or a PhotoMOS that
must be driven) keep PTT off through any brownout (REQ-PWR-005). #10 keeps a
TPS3710-Q1 for automotive cold crank; variant R has no such event.

**Rejected:** the TPS62933F (forced PWM, RT-set frequency; the shortlist in
[`core-devices.md`](core-devices.md#5-power-10-11) had the TPS62933)
([datasheet](../references/index.md#ti-tps62933-ds)). Its oscillator is
specified only at the RT-floating (±10 %) and RT-to-GND (−17/+12 %) points,
and it can't be synchronized (§5.2).

## 5. Switching frequency and the HF bands

### 5.1 Bands and method

US band edges from [47 CFR 97.301](../references/index.md#ecfr-47-97-301)
(Region 2): 160 m 1.800–2.000, 80/75 m 3.500–4.000, 60 m 5.3305–5.4065
(channel envelope), 40 m 7.000–7.300, 30 m 10.100–10.150, 20 m
14.000–14.350, 17 m 18.068–18.168, 15 m 21.000–21.450, 12 m 24.890–24.990,
10 m 28.000–29.700, 6 m 50–54 MHz.

Method: for each switching frequency f, take every harmonic n·f up to 30 MHz,
widen it by the clock tolerance (±tol × n·f), and check it against every band.
The scan covered 0.2–3.0 MHz in 1 kHz steps. This repeats #10's analysis
independently and gets the same result.

### 5.2 Free-running oscillators can't avoid the bands

| f (MHz) | Tolerance | First harmonic that can land in a band |
|---|---|---|
| 0.5 (TPS62933 RT floating) | ±10 % | 4th, 160 m |
| 1.0 (commercial LMR436x0 MB, free-running) | ±10 % | 2nd, 160 m |
| 1.2 (TPS62933 RT to GND) | −17/+12 % | 3rd, 80 m |
| 1.52 (best RT-set choice below 2.2 MHz) | ±10 % | 5th, 40 m |
| 2.1 / 2.2 (common defaults) | ±10 % | Fundamental, 160 m |

- At **±3 %**, no frequency from 0.2 to 3.0 MHz keeps every harmonic up to
  30 MHz out of the bands. At ±10 %, the best case keeps only harmonics 1–4
  clean.
- Nominal 2.1 MHz and 2.2 MHz fail even with an exact clock: 2.1 × 14 =
  29.4 MHz and 2.2 × 13 = 28.6 MHz fall in 10 m.
- **Spread spectrum hurts a receiver.** It lowers peak readings on an EMI
  receiver, but harmonic n is smeared over ±n × the spread (the TPS62933
  spreads ±6 %, so ±1.7 MHz at 28 MHz), straight across bands.
- **PFM is out** during operation. Its burst frequency follows the load.

### 5.3 Chosen: 2.304 MHz, locked to a crystal-grade clock

With a clock accurate to ±0.05 %, the windows where no harmonic up to 30 MHz
lands in a band are 2.148–2.151, 2.287–2.331, 2.478–2.486, 2.502–2.522,
2.706–2.762, 2.780–2.797 and 2.973–2.997 MHz. **2.304 MHz** (= 48 kHz × 48)
sits in the widest window below the LMR436x0's 2.5 MHz sync limit, and it is
variant M's choice:

| n | f (MHz) | Nearest band | Distance to band edge |
|---|---|---|---|
| 1 | 2.304 | 160 m | 304 kHz |
| 2 | 4.608 | 80/75 m | 608 kHz |
| **3** | **6.912** | **40 m** | **88 kHz (worst)** |
| 4 | 9.216 | 30 m | 884 kHz |
| 5 | 11.520 | 30 m | 1,370 kHz |
| 6 | 13.824 | 20 m | 176 kHz |
| 7 | 16.128 | 20 m | 1,778 kHz |
| 8 | 18.432 | 17 m | 264 kHz |
| 9 | 20.736 | 15 m | 264 kHz |
| 10 | 23.040 | 15 m | 1,590 kHz |
| 11 | 25.344 | 12 m | 354 kHz |
| 12 | 27.648 | 10 m | 352 kHz |
| 13 | 29.952 | 10 m | 252 kHz |

- The worst margin is 85 kHz at ±500 ppm and 53 kHz at ±0.5 %. A crystal
  oscillator (±10–50 ppm) is far inside that.
- **6 m can't be kept clean.** Any f below 4 MHz puts a harmonic in the 4 MHz
  band (here n = 22 and 23: 50.688 and 52.992 MHz). Edge-rate control, input
  filtering, layout and a measurement handle it **(verify)**.
- **Before sync starts,** the buck free-runs at 2.2 MHz without spread
  spectrum until the clock is running from the 3.3 V rail. That lasts
  milliseconds at power-up, before any receive or transmit.
- **Clock source: shared, owned by ADR-0004 (#10).** A YXC
  OT322518.432MJBA4SL 18.432 MHz oscillator (±20 ppm over −40 to +85 °C,
  0.7 ps phase jitter max, [datasheet](../references/index.md#yxc-yso110tr-ds))
  is divided by 8 in three SN74LVC1G80 flip-flops
  ([datasheet](../references/index.md#ti-sn74lvc1g80-ds)), giving exactly
  2.304 MHz. One 2.304 MHz net runs in a star, with a 33 Ω series resistor
  per load, to:
  - the buck's MODE/SYNC;
  - the codec's MCLK input. The codec's PLL makes exactly 48 kHz from
    2.304 MHz (P = 3, R = 8, J = 16, D = 0; ADR-0002). The codec does not take
    18.432 MHz directly;
  - the #9 isolated-supply clock.

  The 18.432 MHz net stays **under 5 mm**, because its 8th harmonic
  (147.456 MHz) falls in the 2 m band. The ESP32-S3 can't make 2.304 MHz by
  integer division of its 40 MHz or 80 MHz clocks. The programmable
  [SiT8924B](../references/index.md#sitime-sit8924b-ds) (about $3, not
  stocked at 2.304 MHz) was the alternative ADR-0004 rejected.

### 5.4 The isolated jack-side supply follows the same rule

When isolation is fitted, the jack-side supply (#9) runs **from 3.3 V** and
switches too. An SN6505B driven from the 2.304 MHz clock divides it by two
([SN6505 datasheet](../references/index.md#ti-sn6505-ds) §6.5: external clock
100–1600 kHz, free-running 363–517 kHz; §8.1: the oscillator is divided by
two). The resulting 1.152 MHz comb puts harmonics in bands (for example
1.152 × 25 = 28.8 MHz), and free-running is worse. #9 must either filter and
shield that stage or use an isolated topology that switches at 2.304 MHz
itself (#10 §6.2 has the same open item). The allocation here is 0.25 W out,
0.36 W in (70 %).

## 6. Power budget per mode

### 6.1 Loads (all on the 3.3 V rail)

| Load | Estimate | Basis |
|---|---|---|
| ESP32-S3-MINI-1, CPU | 81 mA average (240 MHz, both cores, peripherals on) | [Datasheet](../references/index.md#esp32s3-mini1-ds) Table 6-6 (Typ2: 81.3 mA) |
| ESP32-S3-MINI-1, BLE | TX 204 mA at +9 dBm (the grant-capped setting), RX 93 mA, both at 100 % duty | Table 6-5; [`module-selection.md`](module-selection.md) |
| Module budget per mode | Average: advertising 90, connected 100, audio streaming 185, audio + USB host 195, wired (BLE off; 240 MHz, 128-bit, all peripherals) 110 mA. **Peak 240–250 mA** (204 mA TX plus about 35 mA of CPU above idle) | Estimates from Tables 6-5/6-6 **(verify at bring-up)** |
| USB2422 hub | **89 mA max** (Hi-Speed, 2 ports, peak traffic); 1 mA max in reset (Bluetooth mode) | [USB2422](../references/index.md#microchip-usb2422-ds) Table 5-1 (I_HCH2, I_CRST, industrial) |
| Codec TLV320AIC3104 | 11 mA analog + 6 mA digital (upper estimate), through its LDOs | ADR-0002 (#8), datasheet §8.5 |
| Misc | 15 mA: LEDs, USB switches, isolator side 1, pull-ups, serial level shifting when not isolated, clock | Allowance |
| PTT PhotoMOS LED | 10 mA while keyed | Allowance (#9 picks the part) |
| Isolated jack side | 109 mA (0.36 W at 3.3 V; 0.25 W out at 70 %) | Allocation, as #10 (#9) |
| Radio USB VBUS | **0: the device doesn't supply it** | Maintainer decision |

Conversion: the buck at **88 % from 5 V** and **80 % from 13.8 V** at
2.304 MHz **(verify against the LMR436x0 efficiency curves)**. Mux, fuse and
diode losses: under 0.1 W.

### 6.2 Results

"Iso" means the isolated jack-side supply is running.

| Mode | 3.3 V avg / peak (mA) | USB-C power avg / peak (W) | USB-C at 4.75 V avg / peak (mA) | USB-C at 4.4 V peak (mA) | Radio DC at 13.8 V avg / peak (mA) | at 11 V peak (mA) | at 9 V peak (mA) |
|---|---|---|---|---|---|---|---|
| Idle / advertising | 123 / 273 | 0.46 / 1.02 | 97 / 216 | 233 | 37 / 82 | 102 | 125 |
| Connected (CAT only) | 133 / 273 | 0.50 / 1.02 | 105 / 216 | 233 | 40 / 82 | 102 | 125 |
| BLE audio streaming | 218 / 273 | 0.82 / 1.02 | 172 / 216 | 233 | 65 / 82 | 102 | 125 |
| USB host active (radio USB) + BLE audio | 228 / 283 | 0.85 / 1.06 | 180 / 223 | 241 | 68 / 85 | 106 | 130 |
| PTT keyed (BLE audio + USB host) | 238 / 293 | 0.89 / 1.10 | 188 / 231 | 250 | 71 / 88 | 110 | 134 |
| Worst Bluetooth case: iso fitted, PTT keyed | 347 / 402 | 1.30 / 1.51 | 274 / 317 | 343 | 104 / 120 | 151 | 184 |
| **Wired mode with hub** (iso mandatory) | 340 | 1.28 | **268** | 290 | 102 | 128 | 156 |
| **Wired mode, PTT keyed** | 350 | 1.31 | **276** | 298 | 105 | 131 | 160 |

Calculation, for example the wired PTT row: 110 (module) + 89 (hub) + 17
(codec) + 15 (misc) + 10 (PTT) + 109 (iso) = 350 mA at 3.3 V = 1.155 W.
From USB-C: 1.155 / 0.88 = 1.31 W, / 4.75 V = 276 mA. From radio DC:
1.155 / 0.80 = 1.44 W, / 13.8 V = 105 mA.

Findings:

- **Wired mode on a 500 mA USB-C port:** 276 mA at 4.75 V and 298 mA at
  4.4 V, about 200 mA of margin. No load shedding is needed, and Rd-only (no
  PD) is enough.
- **Every mode fits 500 mA,** including the worst Bluetooth case (343 mA peak
  at 4.4 V). The firmware still reads CC, reports the advertisement, and
  reports an input droop to the host rather than browning out (REQ-PWR-004).
- **Radio DC:** at most 184 mA (9 V, worst case), 74 % of the 0.25 A fuse and
  well under the K3's 0.5 A. At the usual 12–13.8 V it is 82–151 mA.
- **Against constraints §3.4:** 0.5–1.3 W average and 1.5 W peak, inside
  "about 1.5 W typical, 3 W peak". §3.4's "USB host VBUS to the radio" row no
  longer applies (#9 edits §3.4).
- **Phones as USB-C hosts:** Apple states only "up to 4.5 watts" to a PD
  device; bus power to a non-PD device is `needs bench test` (#6 §2.4).
  Android's CDD recommends at least 1.5 A. The 1.3 W wired draw is under
  4.5 W, but that isn't proof.

## 7. Which radios can power variant R?

Variant R draws at most **0.15 A at 11 V** (0.18 A at 9 V) in its worst case
(§6). The fuse opens within 5 s at 0.5 A.

| Radio | Can it power variant R? | Why |
|---|---|---|
| Icom IC-7300 | **Yes** | ACC pin 8, 13.8 V, 1 A documented; switched |
| Kenwood TS-590SG | **Yes** | EXT.AT pin 6, 4 A; switched. EXT.AT is the tuner port, so an external tuner and the interface can't share it (#5) |
| Elecraft K3 | **Yes, with limits** | 0.5 A, the lowest documented limit: the worst-case draw is 0.15 A at 11 V, and the 0.25 A fuse opens within 5 s at 0.5 A. 12 V at load is above the 9 V priority threshold |
| Elecraft K3S | **Yes** | 1.0 A; switched |
| Elecraft K4 | **Yes** | 12 VDC OUT, 1.5 A; switched. Data goes through the K4's USB-B port; its rear USB-A ports are not used |
| Kenwood TS-590S | **Not yet** (limit `unknown`) | Switched 13.8 V, no documented current. Use USB-C until the pin is measured at ≥ 0.25 A |
| Yaesu FT-891, FT-991A, FT-710 | **Not yet** (limit `unknown`) | "+13V" pins without a documented current. Use USB-C until measured |
| Yaesu FT-817ND / FT-818 | **No** (until measured) | Limit `unknown`, and the voltage follows the radio's 8–16 V supply or battery, which can fall below 9 V |
| Icom IC-705 | **No** | No DC output. USB-C only |
| Elecraft KX2, KX3 | **No** | No DC output. USB-C only |
| Xiegu X6100 | **No** | Only a +8 V MIC pin (current `unknown`), below 9 V. USB-C only |

The "Not yet" pins need the bench measurement already listed in #5 ("Needs
measurement", item 1). A pin that supplies at least 0.25 A at 9 V or more moves
to "Yes".

## 8. Radio-on sense and auto power up/down

### 8.1 Sensing

| Signal | How | Radios |
|---|---|---|
| **RADIO_DC_SENSE** | Divider and clamp from the radio DC input (after D1) to an RTC-capable GPIO, which can wake the ESP32-S3 from deep sleep | Every radio with a switched DC pin: IC-7300, TS-590S/SG, K3/K3S, K4, FT-710 |
| **Radio USB attach** | The radio's USB device pulls up D+ when the radio is on. The device supplies no VBUS, so a radio that doesn't power its own USB side won't attach (#9) **(verify)** | Radios on the USB-host path |
| **SERIAL idle level** | A powered radio holds its TxD at the idle level: RS-232 mark (negative) or logic high. An unpowered radio sits near 0 V. #9 decides whether its receiver or an ADC reads this **(verify)** | RS-232, 3.3 V logic, CI-V |
| **CAT reply** | Firmware sees whether bytes come back after the host polls. It doesn't interpret them | Any |
| **TPS2121 ST** | Which input is active | — |

### 8.2 Behavior

- **Powered from the radio's switched pin:** the device follows the radio. It
  boots when the radio turns on, with PTT off by hardware default
  (REQ-PTT-008), and loses power when the radio turns off.
- **Radio DC and USB-C both present:** when the radio turns off, the mux moves
  to USB-C without a reset. The firmware reports "radio off" to the host,
  turns PTT off, powers down the codec and the isolated supply, and stays
  connected to the host.
- **USB-C only (charger, power bank or host):** after a configurable timeout
  with no radio detected and no host connection (no BLE connection and no USB
  enumeration), the device goes to deep sleep: PTT off, hub in reset, codec
  and isolated supply off, BLE off, the clock stopped and the buck in PFM
  (the MODE/SYNC pin low). It wakes on RADIO_DC_SENSE, on a periodic timer to
  check USB and advertise briefly, or on an optional button (#12). The timeout
  default is **TBD** (maintainer).
- **Power banks** may switch off when the load is small **(verify)**. A
  sleeping device may then lose its power bank until the bank's button is
  pressed.
- **Brownout:** ESP32-S3 brownout reset plus the hardware-default-off PTT
  output (§4); PTT stays off through any brownout on either input
  (REQ-PWR-005).
- **Off-state drain** with radio DC connected and the radio off: zero on
  switched pins, because the pin is off. On an unswitched pin the device stays
  up and sleeps: ESP32-S3 deep sleep 7–8 µA (Table 6-7), TPS2121 about 200 µA
  and the buck's quiescent current in PFM (verify), well under 1 mA.

## 9. Parts, RoHS, price and stock

### 9.1 Parts

LCSC, **2026-09-24**, USD. "$ @100" is the price at the tier that contains
quantity 100. **Digi-Key and Mouser were not checked: the maintainer deferred
those lookups (2026-09-24).** RoHS is LCSC's "RoHS" flag and certificate type.
LCSC doesn't show REACH SVHC status; it is **(verify)** from the manufacturers'
declarations for every part (#59). TI parts are Active on ti.com; lifecycle for
the rest is **(verify)**.

| Ref | Part | Datasheet | LCSC # | $ @100 | LCSC stock | RoHS (LCSC) | Digi-Key / Mouser |
|---|---|---|---|---|---|---|---|
| F1 | Littelfuse 0466.250NRHF (0.25 A, 1206) | [466 Series](../references/index.md#littelfuse-0466-ds) | C83557 | 0.0847 | 20,240 | Yes (RoHS3) | deferred |
| D1 | JSCJ B5819W SL (40 V 1 A Schottky; JLCPCB Basic) | [LCSC C8598](https://www.lcsc.com/product-detail/C8598.html) | C8598 | 0.0283 | 222,220 | Yes | deferred |
| D2 | Brightking SMBJ15A/TR13 | [Vishay SMBJ](../references/index.md#vishay-smbj-ds) (series data) | C78409 | 0.0500 | 41,280 | Yes (RoHS3) | deferred |
| U1 | TI TPS2121RUXR (power mux) | [TPS2121](../references/index.md#ti-tps2121-ds) | C485916 | 0.7004 | 37,215 | Yes (RoHS3) | deferred |
| U2 | TI LMR43620MC3RPERQ1 (buck, 3.3 V) | [LMR436x0-Q1](../references/index.md#ti-lmr436x0-q1-ds) | C41658611 | 3.9856 | **10** | Yes | deferred |
| L1 | 4.7 µH shielded, 4 × 4 mm, 2 A (APV, SWPA4030S4R7MT equivalent) | [LCSC C5363793](https://www.lcsc.com/product-detail/C5363793.html) | C5363793 | 0.0373 | 590 | Yes | deferred |
| Y1 | YXC OT322518.432MJBA4SL (18.432 MHz, ±20 ppm; shared, ADR-0004) | [YSO110TR](../references/index.md#yxc-yso110tr-ds) | C2831385 | 0.2858 | 1,149 | Yes | deferred |
| U3a–c | TI SN74LVC1G80DCKR × 3 (÷8 → 2.304 MHz; shared, ADR-0004) | [SN74LVC1G80](../references/index.md#ti-sn74lvc1g80-ds) | C473331 | 0.3329 (50+) each | **175** | Yes (RoHS3) | deferred |
| U4 | TI LP5907MFX-3.0/NOPB (codec 3.0 V; ADR-0002) | [LCSC C475492](https://www.lcsc.com/product-detail/C475492.html) | C475492 | 0.2313 (50+) | 28,785 | Yes (RoHS3) | deferred |
| U4 alt | TI TPS7A2030PDBVR | [TPS7A20](../references/index.md#ti-tps7a20-ds) | C963429 | 0.1831 (50+) | 6,980 | Yes (RoHS3) | deferred |
| U5 | TI TPS7A2018PDBVR (codec 1.8 V) | [TPS7A20](../references/index.md#ti-tps7a20-ds) | C963430 | 0.2071 (50+) | 19,785 | Yes (RoHS3) | deferred |
| J1 | HRO TYPE-C-31-M-12 (USB-C) | [LCSC C165948](https://www.lcsc.com/product-detail/C165948.html) | C165948 | 0.1484 (50+) | 79,985 | Yes | deferred |
| R1, R2 | UNI-ROYAL 0402WGF5101TCE (5.1 kΩ 1 %, Basic) | [LCSC C25905](https://www.lcsc.com/product-detail/C25905.html) | C25905 | 0.0024 | 4,432,900 | Yes | deferred |
| D3, D4 | TI TPD1E10B06DPYR (CC ESD) | [LCSC C48260](https://www.lcsc.com/product-detail/C48260.html) | C48260 | 0.0422 (20+) | 299,880 | Yes (RoHS3) | deferred |
| D5 | MDD SMF6.0A (VBUS TVS) | [LCSC C123790](https://www.lcsc.com/product-detail/C123790.html) | C123790 | 0.0372 (20+) | 121,640 | Yes | deferred |
| C (in) | Samsung CL31A106KBHNNNE, 10 µF 50 V X5R 1206 (×2) | [LCSC C13585](https://www.lcsc.com/product-detail/C13585.html) | C13585 | 0.2053 | 405,640 | Yes (RoHS3) | deferred |
| C (out) | Samsung CL21A226MAQNNNE, 22 µF 25 V X5R 0805 (×2) | [LCSC C45783](https://www.lcsc.com/product-detail/C45783.html) | C45783 | 0.2222 (20+) | 1,974,840 | Yes | deferred |
| C (small) | Samsung CL05B104KB54PNC, 100 nF 50 V X7R 0402 (×≈10) | [LCSC C307331](https://www.lcsc.com/product-detail/C307331.html) | C307331 | 0.0092 | 751,400 | Yes (RoHS3) | deferred |

LCSC quantity breaks for the main parts (USD, 2026-09-24):

| LCSC # | Breaks |
|---|---|
| C83557 | 5+ 0.1016; 50+ 0.0847; 150+ 0.0763; 500+ 0.0700; 2500+ 0.0589; 5000+ 0.0564 |
| C8598 | 20+ 0.0283; 200+ 0.0221; 600+ 0.0194; 3000+ 0.0173; 9000+ 0.0165 |
| C78409 | 10+ 0.0608; 100+ 0.0500; 300+ 0.0445; 3000+ 0.0332; 6000+ 0.0300 |
| C485916 | 1+ 1.0571; 10+ 0.9223; 30+ 0.7941; 100+ 0.7004; 500+ 0.6609; 1000+ 0.6412 |
| C41658611 | 1+ 4.2036; 10+ 4.1102; 30+ 4.0479; 100+ 3.9856 |
| C5363793 | 10+ 0.0476; 100+ 0.0373; 300+ 0.0322; 2000+ 0.0284; 4000+ 0.0253 |
| C2831385 | 1+ 0.5026; 10+ 0.3925; 30+ 0.3449; 100+ 0.2858; 500+ 0.2595; 1000+ 0.2431 |
| C473331 | 5+ 0.3851; 50+ 0.3329; 150+ 0.3106; 500+ 0.2826; 3000+ 0.2702; 6000+ 0.2627 (checked 2026-09-25) |
| C475492 | 5+ 0.2998; 50+ 0.2313; 150+ 0.2019; 500+ 0.1652; 3000+ 0.1489; 6000+ 0.1391 |
| C963429 | 5+ 0.2332; 50+ 0.1831; 150+ 0.1586; 500+ 0.1368; 3000+ 0.1308 |
| C963430 | 5+ 0.2593; 50+ 0.2071; 150+ 0.1847; 500+ 0.1567; 3000+ 0.1443 |
| C165948 | 5+ 0.1880; 50+ 0.1484; 150+ 0.1291; 1000+ 0.1047; 2000+ 0.1000 |
| C48260 | 20+ 0.0422; 200+ 0.0338; 600+ 0.0301; 2000+ 0.0273; 10000+ 0.0261 |
| C123790 | 20+ 0.0372; 200+ 0.0307; 600+ 0.0272; 3000+ 0.0191; 9000+ 0.0172 |

### 9.2 Rough power-section BOM cost (LCSC, quantity 100)

| Block | Parts | $ |
|---|---|---|
| Radio DC input | F1, D1, D2 | 0.16 |
| USB-C input | J1, R1, R2, D3, D4, D5 | 0.28 |
| Mux | U1 | 0.70 |
| Buck | U2, L1, 2 × 10 µF, 2 × 22 µF, small caps and resistors (≈ 0.10) | 4.98 |
| 2.304 MHz clock (shared; also feeds the codec PLL) | Y1, 3 × U3 | 1.28 |
| Codec LDOs | U4, U5 and their caps (≈ 0.02) | 0.46 |
| **Total** | | **≈ 7.9** |

Excluded: the isolated jack-side supply (#9), the hub and the ESD on D+/D−
(#9), and the radio DC connector (#12). JLCPCB extended-part fees (about $3
per unique extended part per order,
[pcb-fabrication §4](../requirements/pcb-fabrication.md#4-assembly)) come on
top: only D1 and R1/R2 are Basic here.

Cost notes:

- **The buck IC is about half of the total.** It is the maintainer's choice for the
  shared core. The commercial LMR43620MB5RPER (sync, no spread spectrum, but
  5 V fixed/adjustable; $1.80, 34 in stock) could be set to 3.3 V with a
  divider, saving about $2.2; whether that's worth a second part number
  against variant M is for the maintainer.
- The shared 18.432 MHz ÷ 8 clock ($1.28 here at LCSC's 50+ tier) is
  cheaper than a programmable oscillator (about $3).
- The electrolytic damping capacitor in #10 isn't needed here: the TVS clamps
  hot-plug ringing (§1.2).

Sourcing risks:

- **LMR43620MC3RPERQ1: 10 in stock at LCSC.** It is the core of both
  variants. Buy ahead for the prototypes and check TI direct and the deferred
  distributors.
- L1 (590) and Y1 (1,149) have modest stock; L1 has many equivalents. The
  SN74LVC1G80 has only 175 at LCSC; ADR-0004 tracks it for both variants.

## 10. EU EMC targets (#59)

The maintainer requires the product to meet EU requirements
([#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59)). The
design targets **EN 55032 Class B** (emissions) and **EN 301 489-1** (immunity,
with the radio part in EN 301 489-17) alongside FCC Part 15B. This repo doesn't
cite those standards yet, so **every level below is (verify), to be sourced in
#59**.

| Test | Port | What it means for this design |
|---|---|---|
| Radiated emissions, Class B | Enclosure | The 2.304 MHz sync and forced PWM give a fixed, known comb; keep the buck's hot loop small and the input pi filter close to it. The ESP32-S3, hub and isolated supply dominate above 30 MHz |
| Conducted emissions, Class B | DC input, USB-C | Whether EN 55032 applies to a DC input fed from a radio's accessory pin is **(verify)**. Either way, add a **ferrite bead plus MLCC pi filter** between the TPS2121 output and the buck input, so the buck's ripple current doesn't reach the radio's DC lead (it goes straight back into the radio) |
| Surge (EN 61000-4-5 via EN 301 489-1) | DC input | Applicability to short DC leads and the level are **(verify)**. A surge could blow the 0.25 A fuse, which EN 301 489-1's performance criteria may not accept, and the SMBJ15A clamp is near the TPS2121's 24 V absolute maximum. **Upgrade path:** the TPS26600 eFuse (62 V, auto-retry, §1.2) in place of the fuse and in front of the mux |
| EFT/burst (EN 61000-4-4) | DC input, USB-C cable | The TVS and the input MLCCs absorb it; the PR1 and OV dividers need small filter capacitors so bursts don't toggle the mux **(verify)** |
| Conducted RF immunity (EN 61000-4-6) | DC input, USB-C, audio and serial cables | The same pi filter on the DC input; common-mode ferrites on the cables per the RF plan in constraints §5 **(verify)** |
| ESD (EN 61000-4-2) | USB-C, jacks | TPD1E10B06 on CC, SMF6.0A on VBUS, and the D+/D− ESD (#9) |

## 11. Capacitor ratings

Per [pcb-fabrication §6.3](../requirements/pcb-fabrication.md#63-voltage-rating-and-dc-bias):
radio DC input to the buck input **50 V** (13.8 V nominal; 24 V TVS clamp);
USB-C VBUS **16 V** at the connector (hot-plug ringing); the 3.3 V and 3.0 V
rails **10 V**; the 1.8 V rail **6.3 V**. The buck input sees up to 17.5 V
from radio DC, so its MLCCs are 50 V parts. Check DC-bias derating at
schematic capture.

## 12. Battery operation (future)

Out of scope for revision A (maintainer default). A later variant would add a
1S Li-ion cell (3.0–4.2 V) with a charger and power-path management, a third
mux input or a charger ahead of the mux, fuel gauging and a cell temperature
sensor, and must follow the transport rules for lithium cells. The buck
accepts 3.0 V in, so a 1S cell could feed it directly, but its duty cycle would
exceed 0.80 below about 4.2 V and fold back in frequency. A battery variant
therefore needs a different clock plan or a boost stage. Meanwhile, a USB-C
power bank covers portable use.

## 13. Coordination with #10 (variant M) and #9

| Item | Variant M (#10) | Variant R (this doc) | Shared? |
|---|---|---|---|
| Buck | LMR43620MC3RPERQ1, single stage to 3.3 V | Same part | **Yes (ADR-0004 owns it)** |
| Switching clock | 2.304 MHz from 18.432 MHz ÷ 8 (YXC + 3 × SN74LVC1G80), star-distributed | Same | **Yes (ADR-0004 owns it)** |
| Codec supplies | Per ADR-0002 | LP5907-3.0 and TPS7A2018 from 3.3 V (ADR-0002) | Yes; #10 faces the same headroom question |
| Input protection | Fuse, TVS stack, LM74800-Q1 ideal diode, OV cut-off | Fuse, Schottky, TVS | No: different environments |
| Source mux | None (USB-C isn't a power input on M) | TPS2121 | R only |
| Brownout | TPS3710-Q1 (cold crank) | ESP32-S3 BOD + hardware-default-off PTT | No |
| Isolated jack-side supply | #9, from 3.3 V | #9, from 3.3 V | Same design; 2.304 MHz rule |
| Radio USB VBUS | Removed | Removed | Yes (ADR-0003, #9) |

For #9: the radio USB port's VBUS is blocked, not supplied (ADR-0003). There is
no TPS2553-class switch and no 5 V rail. The isolated supply allocation is
0.25 W out from 3.3 V.

## 14. Open items

- **Bench (human-task, later):** input current per mode on radio DC and
  USB-C; hot-plug and inrush; buck efficiency at 2.304 MHz from 5 V and
  13.8 V; the receiver noise floor on every band 160–6 m with the radio next
  to the device, on radio DC and on USB-C; USB-C draw on iPhone, iPad,
  Android and laptops; USB suspend current; EU pre-compliance (#59).
- **(verify):** module current estimates; USB 2.0 pre-configuration and
  suspend limits; buck behavior below 4.2 V from USB; power-bank auto-off;
  whether each radio's USB side attaches without VBUS from the device (#9);
  codec output swing at 3.0 V AVDD (#8); the flip-flops' additive jitter
  (ADR-0004); SN74LVC1G80 stock (175 at LCSC); REACH SVHC declarations for every
  part.
- **Maintainer:** the auto power-down timeout; whether the SERIAL jack's
  optional "3.3 V out" on ring 2 (about 20 mA to a cable's own circuit,
  REQ-RIF-003) counts as supplying power out of a port under the "input only"
  rule; the MC3-Q1 vs commercial MB5 buck trade-off (§9.2).

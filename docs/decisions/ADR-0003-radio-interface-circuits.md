<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0003: Radio interface circuits: SERIAL-jack switching, fail-safe PTT with a hardware timer, no radio VBUS, and USB routing

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #9

## Context

The radio side ([constraints §5–§7](../requirements/constraints.md#7-radio-interfaces),
[`radio-connectors.md`](../requirements/radio-connectors.md)) needs:

- a SERIAL jack whose tip and ring 1 carry 3.3 V logic, RS-232 or Icom CI-V,
  selected in firmware, and which survives ±15 V on any contact in any mode;
- an isolated PTT closure on AUDIO ring 2 that is off by hardware default;
- host RTS/DTR mapped to that closure or to the radio's USB-serial chip;
- jack isolation on variant M and whenever USB-C data is used;
- the USB routing of [ADR-0008](ADR-0008-host-links-esp32-s3.md) (hub, two
  switches, USB-C, radio USB-A port).

Maintainer decisions and directions made while this issue was open
(2026-09-24):

1. **No power to the radio through USB**, in either mode, with hardware that
   blocks current from the device into the radio's VBUS. This supersedes the
   ADR-0008 consequence "the VBUS switch to the radio (TPS2553-class) stays"
   (ADR-0008 is accepted and isn't edited). The IC-705 can't charge from the
   device and is low priority. The K4 connects through its USB-B port only.
2. **A hardware PTT timer**: independent of the MCU, default 10 minutes,
   configurable.
3. **No 5 V rail**: one 3.3 V buck synchronized to a 2.304 MHz master clock
   (#10); every switching stage and clock must keep its harmonics out of the
   HF amateur bands, and the audit covers 6 m, 2 m and 70 cm.
4. **Lowest cost** meeting −40 to +85 °C (AEC-Q not required); RoHS for every
   part and EU EMC (EN 55032 Class B, EN 301 489-1/-17) as targets (#59).

Inputs: the per-radio table (#5, PR #56), host compatibility (#6, PR #54),
[ADR-0002](ADR-0002-audio-codec.md) (#8, PR #55) and the module notes in
[`fcc.md`](../compliance/fcc.md) (#7). Full analysis, circuits and sources:
[`radio-interface-circuits.md`](../research/radio-interface-circuits.md).

## Options considered

| Block | Option | Pros | Cons | Sources |
|---|---|---|---|---|
| SERIAL mode switching | **A. MAX14778** dual 4:1, ±25 V signals from one 3.0–5.5 V supply | No dual rail; one part covers tip and ring 1; −40 to +85 °C; $4.24 @100 | Single source; unpowered behavior and absolute maximum not yet read (ADI blocks scripted download) | [datasheet](../references/index.md#adi-max14778-ds), LCSC C1121866 |
| | B. TMUX6219 per contact | 36 V, −40 to +125 °C | Signals must stay within its rails, even unpowered: needs ≈ ±16 V in the jack domain; 2 × $2.18 | [datasheet](../references/index.md#ti-tmux6219-ds) |
| | C. Signal relays (G6K-2F-Y) | Robust, polarity- and power-agnostic | −40 to +70 °C fails variant M; coil power; mechanical | [datasheet](../references/index.md#omron-g6k-ds) |
| Isolated jack supply (from 3.3 V) | **A. Discrete push-pull at 2.304 MHz** from the master clock (SN74LVC2G02 + 2× AO3400A + 1:1.3 transformer + LDO) | Adds no HF line; cheapest | No built-in current limit or soft start; transformer behavior at 2.3 MHz needs a bench test | [SN6505B datasheet (transformers)](../references/index.md#ti-sn6505b-ds) |
| | B. SN6505B clocked at 1.152 MHz (switches at 576 kHz) | Integrated protection | Lines in 15 m and 10 m (harmonics 37, 49–51) | same |
| | C. SN6507 clocked at 2.304 MHz (switches at 1.152 MHz) | Integrated, synchronizable | 28.8 MHz line (10 m); $2.27+, thin stock | [datasheet](../references/index.md#ti-sn6507-ds) |
| PTT closure | **A. AQY212EH(AX)** PhotoMOS | 60 V AC/DC, 0.55 A, 2.5 Ω max; 5 kV | 5 mA LED drive | [datasheet](../references/index.md#panasonic-aqy21eh-ds) |
| | B. CPC1017N | 1 mA LED drive | 16 Ω, 100 mA | [datasheet](../references/index.md#littelfuse-cpc1017n-ds) |
| PTT gating | **A. TPS3839G33 supervisor + TPS3430 window watchdog + 3-input AND** | Both in stock; RESET valid from 0.6 V; $0.26 + $1.16 | Two parts | [TPS3839](../references/index.md#ti-tps3839-ds), [TPS3430](../references/index.md#ti-tps3430-ds) |
| | B. TPS3850 (supervisor + watchdog in one) | One part | Out of stock at LCSC; $1.42 @250 | [datasheet](../references/index.md#ti-tps3850-ds) |
| Hardware PTT timer | **A. TPL5111 one-shot, powered from the PTT drive; resistor-set** | Independent of the MCU; fails safe (no supply → off); firmware can shorten (DONE), never extend; ±5.4 % worst case over −40 to +85 °C | 100–120 ms keying latency; TI only | [datasheet](../references/index.md#ti-tpl5111-ds) |
| | B. 74HC4060 counter with RC or crystal | No latency | A stopped oscillator means no timeout unless more logic is added | [datasheet](../references/index.md#nexperia-74hc4060-ds) |
| | C. Firmware-set digital potentiometer | Settable in the field | Firmware could lengthen or disable it | — |
| Radio VBUS | **A. Not connected** (ESD diode and test pad only) | No device-to-radio path exists; no back-feed | Radios whose USB chip waits for VBUS won't enumerate | [CP2105](../references/index.md#silabs-cp2105-ds), [CH342](../references/index.md#wch-ch342-ds) |
| | B. Reverse-blocking FET or ideal diode | — | Still a switchable path with a shorted-FET failure; not needed when nothing is switched | — |
| | C. Sense-only feed (3.3 V → Schottky → 4.7 kΩ), DNP | May satisfy VBUS-sense inputs (CH342 yes; CP2105 only without the datasheet's divider) | Connects a device rail to radio VBUS; the maintainer decides | same |
| USB hub | **A. USB2422T-I/MJ** | −40 to +85 °C; HS_DISABLE fallback; $1.48 @100 | Microchip only, 403 in stock; 24 MHz crystal's 6th harmonic is 144.000 MHz | [datasheet](../references/index.md#microchip-usb2422-ds) |
| | B. CH334F | $0.42 @100 | Datasheet not reviewed; RoHS not shown at LCSC | LCSC C5187527 |
| | C. GL850G | $0.32 | 0 to +85 °C: fails both variants | LCSC C136617 |

Price and stock: LCSC, 2026-09-24. Digi-Key and Mouser checks are deferred
by the maintainer (2026-09-24).

## Decision

1. **SERIAL jack:** a **MAX14778** selects, per contact, the logic buffer, the
   **TRS3221E** RS-232 driver/receiver (FORCEOFF off outside RS-232 mode), or
   the CI-V node (SN74LVC1G07 open drain, with a receive tap for echo). The
   receive paths merge in a 3-input AND. Mode control comes from a TCA9534
   whose power-up pulls give the default: **3.3 V logic, RS-232 off, ring 2
   off**. Series pulse-rated resistors and PESD24VL1BA TVS diodes at each
   contact. Ring 2 "3.3 V out" goes through a CPC1017N-class switch with a
   current limiter and a Schottky; firmware never selects it for a cable that
   uses ring 2 (KX2). Fallback if the MAX14778 fails its ±15 V unpowered test:
   relays on variant R, TMUX6219 with a ±16 V supply on M.
2. **PTT:** an **AQY212EHAX** PhotoMOS closes AUDIO ring 2 to sleeve. Its LED
   is driven only when **all** of these are true: PTT_REQ high (pulled down);
   **TPS3839G33** RESET high (brownout; it also resets the ESP32); **TPS3430**
   WDO high (window watchdog); and the **hardware PTT timer** permits it.
   The status LED is in the driven branch.
3. **Hardware PTT timer:** a **TPL5111** in one-shot mode, powered from the
   PTT drive, gates the LED current. Asserting PTT powers and starts it,
   releasing PTT resets it; on expiry DRVn goes low and stays low until PTT is
   released. **Default 600 s, set by REXT = 107 kΩ ∥ 124 kΩ (1 %)**, changed
   only by fitting other resistors. Worst-case tolerance −5.1 % / +5.4 %
   (569–632 s) over −40 to +85 °C. Firmware can end it early (DONE) but can't
   lengthen or disable it; its state is read back for a `PTT_STATUS` reason.
   The firmware limit `MAX_TX_S` (10–600 s, protocol PR #58) must stay at or
   below the hardware setting.
4. **Radio USB VBUS:** the USB-A VBUS pin connects to **nothing but an ESD
   diode and a test pad**. No device rail, the USB-C VBUS or the hub's port
   power reaches it, in any mode or fitting option; the hub's PRTPWR pins are
   left open (which also keeps battery charging off) and its OCS pins pulled
   up. The sense-only feed is documented as an option for the maintainer,
   **not adopted**.
5. **Isolation** follows the ADR-0002 fitting approach: ISO7721 (data),
   ISO1540 + TCA9534 (control) and the jack supply fitted on M, DNP with 0 Ω
   links on R. The jack supply is the **discrete 2.304 MHz push-pull stage**,
   with AC-coupled gates so a stopped clock turns both switches off, and
   stopped by firmware when the SERIAL jack is unused. The SN6505B at 576 kHz
   is the fallback.
6. **USB routing:** USB2422T-I/MJ hub (held in reset in Bluetooth mode),
   two TS3USB221A switches with OE pulled to "disabled" until firmware picks a
   mode, TYPE-C-31-M-12 with 5.1 kΩ Rd (variant R; M needs a −40 °C part),
   USBLC6-2SC6/TPD2E2U06 ESD. The ADuM4160 remains a variant M option; it needs
   5 V on both sides, which this board doesn't have.
7. **Clocks:** I2S BCLK at 64 fs (3.072 MHz), not 32 fs (29.184 MHz line in
   10 m). No new clock is added.
8. **USB host drivers (#43):** esp-usb at commit `bf0f0aa3` supports CP2105
   (PID 0xEA70, per-interface ports), CH340/CH341, CDC-ACM (for the CH342),
   UAC 1.0 and hubs. The FTDI driver's modem-control request must be checked.

## Consequences

- **Supersedes** the ADR-0008 consequence that a TPS2553-class VBUS switch to
  the radio stays. constraints §3.1, §3.4 and §7 and REQ-RIF-007 /
  REQ-PWR-004 change accordingly; a new REQ-PTT-011 and a constraints §6
  bullet add the hardware timer.
- **Radios may not enumerate without VBUS.** The CP2105 and CH342 datasheets
  make VBUS a connect-sense input. The FT-891 and X6100 have no other CAT path,
  so they may lose CAT. A bench test (a `human-task`) must check every radio
  with no VBUS and with the sense-only feed, and the maintainer decides on the
  option.
- **Keying latency:** the closure turns on 100–120 ms after PTT is requested
  (the TPL5111's REXT reading). Scheduled TX must start the drive early.
- **Hardware limit vs firmware limit:** at 600 s nominal the hardware can
  expire at 569 s. Either set the hardware nominal above the firmware maximum
  (for example 640 s) or accept a hardware expiry at the top firmware setting.
  Open for the maintainer.
- **Variant R isolation in wired mode** stays open, as in ADR-0002.
  **Ground path through the radio:** with the radio's USB cable also
  connected and no ADuM4160, jack isolation is bypassed through the radio
  (REQ-EMC-003, #12).
- **2 m and 70 cm:** the USB2422 crystal (24 MHz × 6) and USB full speed
  (12 MHz × 12) sit on 144.000 MHz; the ESP32-S3 crystal's 11th harmonic is
  440.000 MHz; the 2.304 MHz comb has lines in 6 m, 2 m and 70 cm. These need
  measurement and layout care, not a frequency change.
- **EU (#59):** every chosen part is RoHS at LCSC except the TYPE-C-31-M-12
  and CH334F (not stated; verify); no REACH SVHC data at LCSC. Immunity levels
  for the jack and USB ports are to be set in #59; TVS and series parts are
  sized for them then.
- **Single-source parts:** MAX14778, USB2422, TPL5111, TPS3430, each with an
  alternate topology above.
- **Verify before schematic freeze:** MAX14778 unpowered ±15 V behavior;
  2.304 MHz transformer behavior; CI-V pull-up levels (IC-7300); FT-710 CAT-3
  threshold; PTT voltages and currents per radio (#5); the FTDI request;
  TPL5111 latency and slow-ramp brownout.
- Follow-ups: #43 (driver checks, DTR/RTS inactive on open), #15 (CI-V echo
  and collisions, RTS/DTR on change only, ring-2 profile block, DONE use),
  #13 (`PTT_STATUS` reason for a hardware-timer expiry), #10/#11 (3.3 V rail
  above the TPS3839 threshold; the 2.304 MHz clock distribution), #12
  (fitting), #21 (review rule: the radio VBUS net holds only its ESD diode and
  test pad).

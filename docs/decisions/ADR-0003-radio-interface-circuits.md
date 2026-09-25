<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0003: Radio interface circuits: SERIAL-jack switching, fail-safe PTT, no radio VBUS, and USB routing

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
2. **No external PTT timer** (2026-09-25, superseding a 2026-09-24 request
   for a hardware PTT timer): lock-up protection is the ESP32-S3's internal
   watchdogs, and the maximum TX time is a firmware setting (default
   5 minutes, no upper limit, can be disabled; [ADR-0007](ADR-0007-protocol.md)). An external TPL5111 timer and
   TPS3430 window watchdog, proposed earlier in this PR, are rejected: no
   external PTT timer is wanted, and they cost money.
3. **No 5 V rail**: one 3.3 V buck synchronized to a 2.304 MHz master clock
   (#10); every switching stage and clock must keep its harmonics out of the
   HF amateur bands, and the audit covers 6 m, 2 m and 70 cm.
4. **Lowest cost** meeting −40 to +85 °C (AEC-Q not required); RoHS for every
   part and EU EMC (EN 55032 Class B, EN 301 489-1/-17) as targets (#59).
5. **Radio VBUS test options** (2026-09-25): blocked by default, with DNP
   footprints for a sense-only feed and a 0 Ω bypass, so a bench test can give
   the radio VBUS without a respin.
6. **Auto power-down** 30 s after the radio turns off, configurable (accepted;
   #10/#11).

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
| PTT gating and lock-up | **A. TPS3839G33 supervisor + ESP32-S3 internal watchdogs + 2-input AND + PTT_REQ pull-down** | Supervisor specified from 0.6 V and independent of firmware; $0.26; the watchdog reset returns PTT_REQ to its pulled-off default | No backstop for running-but-buggy firmware beyond the firmware max-TX timer | [TPS3839](../references/index.md#ti-tps3839-ds), [ESP-IDF watchdogs](../references/index.md#esp-idf-wdts) |
| | B. ESP32-S3 brown-out detector instead of the TPS3839 | Saves $0.26 | Thresholds approximate and chip-dependent; enabled by firmware; no spec below the operating range | [ESP-IDF brownout Kconfig](../references/index.md#esp-idf-s3-brownout-kconfig) |
| | C. Rejected: external TPL5111 PTT timer and TPS3430 window watchdog | Independent of the MCU | Maintainer: no external PTT timer wanted; cost ($0.46–0.83 + $1.16) | [TPL5111](../references/index.md#ti-tpl5111-ds), [TPS3430](../references/index.md#ti-tps3430-ds) |
| Radio VBUS | **A. Not connected** (ESD diode and test pad only) | No device-to-radio path exists; no back-feed | Radios whose USB chip waits for VBUS won't enumerate | [CP2105](../references/index.md#silabs-cp2105-ds), [CH342](../references/index.md#wch-ch342-ds) |
| | B. Reverse-blocking FET or ideal diode | — | Still a switchable path with a shorted-FET failure; not needed when nothing is switched | — |
| | C. A, plus DNP test footprints: (a) sense-only feed (3.3 V → Schottky → 4.7 kΩ); (b) 0 Ω bypass from 3.3 V or from USB-C VBUS, each through a PTC | Bench test without a respin; the default build is unchanged | Must never be fitted in production; review rule needed | same |
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
   contact. **Ring 2 "3.3 V out" is kept** (maintainer decision, 2026-09-25):
   about 3.2 V, limited to about 22 mA by a BC857BS two-transistor limiter,
   short-circuit protected, through a BAT54 and an AQY212EHAX PhotoMOS that is
   off unless "3.3 V logic + power" mode is selected (about $1.10). It is a
   deliberate exception to "the device only takes power in": it powers
   cable-side circuits for compatibility with the existing cable convention,
   never the radio. Firmware never selects it for a cable that uses ring 2
   (KX2). Fallback if the MAX14778 fails its ±15 V unpowered test:
   relays on variant R, TMUX6219 with a ±16 V supply on M.
2. **PTT:** an **AQY212EHAX** PhotoMOS closes AUDIO ring 2 to sleeve. Its LED
   is driven through an SN74LVC1G08 AND gate only when **both** are true:
   PTT_REQ high (100 kΩ pull-down) and **TPS3839G33** RESET high (brownout;
   it also drives the ESP32 EN pin). The status LED is driven from the gate
   output. The TPS3839 is kept because the ESP32-S3 brown-out detector can't
   guarantee PTT off: its levels are approximate and chip-dependent, it is
   enabled by firmware, and nothing specifies a driving GPIO below the chip's
   operating range.
3. **Lock-up protection:** the ESP32-S3's internal watchdogs (interrupt and
   task watchdogs, RTC watchdog) reset the chip within a few seconds; the
   reset returns PTT_REQ to its pulled-off default. The maximum TX time is a
   firmware setting (default 5 minutes, no upper limit). This **supersedes**
   this ADR's earlier TPL5111 timer and TPS3430 design.
4. **Radio USB VBUS:** the USB-A VBUS pin connects to **nothing but an ESD
   diode and a test pad**. No device rail, the USB-C VBUS or the hub's port
   power reaches it, in any mode, as fitted by default; the hub's PRTPWR pins are
   left open (which also keeps battery charging off) and its OCS pins pulled
   up. **As fitted by default** that is all; three DNP test footprints sit
   on the net: (a) a sense-only feed from 3.3 V, (b1) a 0 Ω bypass from 3.3 V
   and (b2) a 0 Ω bypass from USB-C VBUS, each bypass through a PTC. For the
   bench test, **(b2) with the device powered from a USB-C charger** is
   proposed, since 3.3 V is below the CP2105's detect level when the radio
   uses the datasheet's VBUS divider (2.5 V threshold; ≈ 2.2 V at the pin).
   None is fitted in production.
5. **Isolation** follows the ADR-0002 fitting approach: ISO7721 (data),
   ISO1540 + TCA9534 (control) and the jack supply fitted on M, DNP with 0 Ω
   links on R. The jack supply is the **discrete 2.304 MHz push-pull stage**,
   with AC-coupled gates so a stopped clock turns both switches off, and
   stopped by firmware when the SERIAL jack is unused. The SN6505B at 576 kHz
   is the fallback.
6. **USB routing:** USB2422T-I/MJ hub (held in reset in Bluetooth mode),
   two TS3USB221A switches under ESP32-S3 GPIO control (OE = H is the
   datasheet's "Disconnect" state): the radio-port switch powers up
   **connected to hub port 1**, so plain-port use works out of the box, and
   firmware drives its OE high for `WIRED_PORT_LOCK` level 3 (protocol
   PR #58 §6.1); the ESP32-S3 switch powers up disconnected until firmware
   picks a mode, TYPE-C-31-M-12 with 5.1 kΩ Rd (variant R; M needs a −40 °C part),
   USBLC6-2SC6/TPD2E2U06 ESD. The **ADuM4160** remains a variant M option.
   Its purpose is to break the ground path through the radio's USB cable
   (hum, alternator noise, RF common-mode current, and the bypass of the jack
   isolation). It runs from 3.3 V with VBUSx tied to VDDx: side 1 from the
   3.3 V rail, side 2 from an isolated 3.3 V made by a second transformer on
   the 2.304 MHz push-pull stage. About $8–9.50 per board when fitted.
7. **Clocks:** I2S BCLK at 64 fs (3.072 MHz), not 32 fs (29.184 MHz line in
   10 m). No new clock is added.
8. **USB host drivers (#43):** esp-usb at commit `bf0f0aa3` supports CP2105
   (PID 0xEA70, per-interface ports), CH340/CH341, CDC-ACM (for the CH342),
   UAC 1.0 and hubs. The FTDI driver's modem-control request must be checked.

## Consequences

- **Supersedes** the ADR-0008 consequence that a TPS2553-class VBUS switch to
  the radio stays. constraints §3.1, §3.4 and §7 and REQ-RIF-007 /
  REQ-PWR-004 change accordingly; a new REQ-PTT-011 and a constraints §6
  bullet record the internal-watchdog lock-up protection.
- **Radios may not enumerate without VBUS.** The CP2105 and CH342 datasheets
  make VBUS a connect-sense input. The FT-891 and X6100 have no other CAT path,
  so they may lose CAT. A bench test (a `human-task`) must check every radio
  with no VBUS and with the sense-only feed, and the maintainer decides on the
  option.
- **No hardware max-TX backstop.** Running-but-buggy firmware that keeps
  feeding the watchdog, firmware that disables it, and a failed chip aren't
  covered in hardware; the firmware max-TX timer and the radio's own time-out
  timer are the remaining defenses. The maintainer accepted this for cost and
  simplicity. REQ-PTT-011 and constraints §6 are rewritten to match.
- **The drive circuit must guarantee off during and after any reset:**
  PTT_REQ on a pin without power-up glitches, a pull-down, and a PhotoMOS that
  needs active LED drive.
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
- **Single-source parts:** MAX14778 and USB2422, each with an alternate
  topology above.
- **Verify before schematic freeze:** MAX14778 unpowered ±15 V behavior;
  2.304 MHz transformer behavior; CI-V pull-up levels (IC-7300); FT-710 CAT-3
  threshold; PTT voltages and currents per radio (#5); the FTDI request;
  slow-ramp brownout.
- Follow-ups: #43 (driver checks, DTR/RTS inactive on open), #15 (CI-V echo
  and collisions, RTS/DTR on change only, ring-2 profile block, DONE use),
  #15 (watchdog timeouts; the PTT task subscribed to the task watchdog), #10/#11 (3.3 V rail
  above the TPS3839 threshold; the 2.304 MHz clock distribution), #12
  (fitting), #21 (review rule: the radio VBUS net holds only its ESD diode,
  a test pad and the DNP test footprints, marked TEST ONLY).

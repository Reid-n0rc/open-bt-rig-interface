<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0005: Variant R power from the radio's accessory DC pin or USB-C, input only, one synchronized 3.3 V buck

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #11

## Context

Variant R runs from the radio's accessory DC output (about 13.8 V) where the
radio has one, or from USB-C 5 V: a charger, a power bank, or the host in
wired mode ([constraints §3](../requirements/constraints.md#3-power);
REQ-PWR-001 to REQ-PWR-005). The analysis is in
[`power-radio-usbc.md`](../research/power-radio-usbc.md).

Constraints that shape the decision:

- **Radio pin limits** (#5, [PR #56](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/56)):
  IC-7300 1 A, TS-590SG 4 A, K3 **0.5 A**, K3S 1 A, K4 1.5 A. The Yaesu
  "+13V" pins and the TS-590S have no documented limit. The IC-705, KX2, KX3
  and X6100 have no usable DC output.
- **USB-C Default current is 500 mA** for a USB 2.0 device. 1.5 A and 3.0 A
  are advertised through Rp on CC ([Type-C R2.0](../references/index.md#usb-typec-r20)
  Tables 4-17, 4-24, 4-36).
- **No switching harmonics in the HF bands** (constraints §5, REQ-EMC-004,
  REQ-EMC-005).
- **ADR-0008 applies** (the issue predates it): wired USB-C mode with an
  on-board hub, and isolation of the AUDIO and SERIAL jacks whenever the USB-C
  data link is used.
- **Maintainer decisions, 2026-09-24:**
  - **Power flows in only.** The device never supplies VBUS to the radio's USB
    port, and hardware blocks current into it (#9, ADR-0003). Issue item 5
    (the VBUS supply to the radio) and the IC-705 charging case no longer
    apply. The K4 connects only through its USB-B port.
  - **Optimize for spurious emissions and cost; share the regulator core and
    clock with variant M** (ADR-0004), fed directly from the OR of the two
    inputs, with no 5 V rail. Station-grade input
    protection. The cheapest parts that meet −20 to +60 °C; AEC-Q not
    required.
  - **EU requirements (#59):** RoHS-compliant parts; EN 55032 Class B and
    EN 301 489-1 as design targets.

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **Current limit A: 0.25 A fast fuse + B5819W Schottky + SMBJ15A** | $0.16. The fuse opens within 5 s at 0.5 A (the K3's limit) and carries the 0.19 A worst-case draw. The Schottky blocks reverse polarity and back-feed | A fuse must be replaced after a fault; the TVS clamp at full rated pulse (24.4 V) is at the mux's 24 V absolute maximum **(verify)**; a surge test may blow the fuse | [466 Series](../references/index.md#littelfuse-0466-ds), [SMBJ](../references/index.md#vishay-smbj-ds), LCSC C83557, C8598, C78409 |
| Current limit B: TPS26600 eFuse (−60 V reverse polarity, 0.1–2.23 A adjustable limit, dV/dt, UVLO, OVP, reverse blocking, 62 V) | Precise electronic limit, auto-retry, rugged | $1.07, about $0.95 more | [TPS2660](../references/index.md#ti-tps2660-ds), LCSC C544399 |
| Current limit C: variant M's LM74800-Q1 chain | Shared with variant M | No current limit; automotive cost | #10 |
| **Source selection A: TPS2121 priority mux on both raw inputs** | $0.70. One IC: priority to radio DC, OVP on each input, soft start (inrush), reverse blocking into both sources, status pin; 2.8–22 V | 24 V absolute maximum near the TVS clamp | [TPS2121](../references/index.md#ti-tps2121-ds), LCSC C485916 |
| Source selection B: diode-OR (two Schottkys) | $0.05 | The USB-path drop pushes the buck past its 0.80 maximum duty cycle below about 4.8 V of VBUS, so it folds back in frequency and harmonics move into bands; no USB overvoltage cut-off | — |
| Source selection C: two ideal-diode controllers + FETs | Low drop | About $0.6 plus FETs, more parts, no OVP | — |
| Buck cost alternative (not adopted): commercial LMR43620MB5RPER (sync, no spread spectrum, 5 V fixed/adjustable) set to 3.3 V | $1.80 against $2.494 (TI.com) | Not AEC-Q100. The maintainer keeps automotive grade unless a cheaper automotive drop-in exists; none does (PR #57, 2026-09-25) | [LMR436x0](../references/index.md#ti-lmr436x0-ds) |

The **regulator core and the switching clock are not options here.** They are
owned by **ADR-0004** (#10, [PR #57](https://github.com/Reid-n0rc/open-bt-rig-interface/pull/57)):
one LMR43620MC3RPERQ1 at 3.3 V in FPWM, synchronized to 2.304 MHz from an
18.432 MHz oscillator divided by 8 in three SN74LVC1G80 flip-flops. That
clock is star-distributed with 33 Ω series resistors to the buck, the codec
and the isolated supply. This ADR checks that the core works with variant R's
two inputs: the duty cycle is within limits from 4.4 V to 17.5 V in, and no
harmonic from 160 m to 10 m lands in a band (research doc §4, §5).

Prices: LCSC, quantity 100, 2026-09-24. Digi-Key and Mouser: deferred by the
maintainer. Every chosen part is RoHS-compliant per LCSC; REACH SVHC is
**(verify)**.

## Decision

1. **Power flows in only.** The device takes power from the radio's DC
   accessory pin or USB-C (and, on variant M, 12 V). **It never supplies power
   on any port, with one deliberate exception** (below):
   - **USB-C is a sink only, in both host modes:** 5.1 kΩ Rd on CC1 and CC2,
     never Rp; it never drives VBUS. The TPS2121 keeps the USB-C VBUS pin
     unpowered when radio DC powers the device.
   - **Radio DC input:** no current back into the radio's pin (the B5819W and
     the TPS2121).
   - **Radio USB port:** no VBUS; hardware blocks current into the radio's VBUS
     (#9, ADR-0003).
   - Jack contacts carry signals, and PTT is a closure.
   - **Exception:** the SERIAL jack's ring-2 3.3 V output (about 20 mA,
     current-limited, off by default). It powers circuits inside a cable,
     never the radio (maintainer decision 2026-09-25; ADR-0003, PR #61). Its
     20 mA is in the 3.3 V budget.
2. **Radio DC input:** 0.25 A fast fuse (the current limit) → B5819W → SMBJ15A
   → TPS2121 IN1, which has priority above 9.0 V and cuts off above 17.5 V.
   Inrush is set by the TPS2121 soft start; about 1 µF sits before the mux.
   The TPS26600 eFuse is the upgrade path if the surge test or the bench needs
   it.
3. **USB-C:** 5 V sink, no USB PD (the worst case is about 360 mA at 4.4 V).
   The firmware reads CC against the Type-C thresholds and reports Default,
   1.5 A or 3.0 A. ESD on CC (TPD1E10B06) and VBUS (SMF6.0A); OVP at 5.8 V in
   the mux.
4. **Source selection:** **TPS2121**: IN1 radio DC (priority), IN2 USB-C VBUS.
   No back-feed into either source.
5. **Regulation:** the **ADR-0004 core** (the LMR43620MC3RPERQ1 at 3.3 V and
   its 2.304 MHz clock), fed from the TPS2121 output. There's no 5 V rail. The
   codec takes the 2.304 MHz clock into its PLL (ADR-0002). Codec supplies
   as ADR-0002 specifies: **LP5907-3.0** (AVDD/DRVDD) from 3.3 V, and
   **TPS7A2018** (DVDD) from the 3.0 V output. The 3.3 V rail stays above
   the LP5907's 3.1 V need on every USB-C input in the USB 2.0 range. Brownout: the ESP32-S3
   brownout reset plus hardware-default-off PTT; no external supervisor.
6. **Isolated jack-side supply** (only when isolation is fitted): runs from
   3.3 V, 0.25 W out allocated. #9 designs it, and it follows the same
   2.304 MHz rule.
7. **Radio-on sense:** the switched DC pin (RADIO_DC_SENSE on an RTC GPIO),
   plus the radio's USB attach, the SERIAL idle level and CAT replies.
   **Auto power** (maintainer decisions, 2026-09-25): radio-powered units follow
   the radio. The device **stays awake while a USB host is connected** on
   USB-C. With no USB host attached, it powers down **30 s after radio-on sense
   goes off** (configurable; 0 = never), and wakes on radio DC, a timer or a
   button.
8. **Radios that can power variant R:** IC-7300, TS-590SG, K3 (within 0.5 A),
   K3S, K4. The TS-590S and the Yaesu "+13V" pins qualify after a measurement
   shows at least 0.25 A. The FT-817ND/FT-818, IC-705, KX2, KX3 and X6100 use
   USB-C.

## Consequences

- **Power budget** (research doc §6): 0.5–1.4 W average, 1.6 W peak.
  **Wired mode draws about 292 mA from USB-C at 4.75 V** (315 mA at 4.4 V),
  about 185 mA under the 500 mA Default. Radio DC draws at most 0.16 A at
  11 V (0.19 A at 9 V). REQ-PWR-003 and REQ-PWR-004 are met on paper
  **(verify on the bench)**.
- **Power-section BOM about $6.4** (qty 100, including the shared core and
  clock), with the buck at TI.com's $2.494; about $7.9 at LCSC's $3.99. The
  buck stays the automotive-grade LMR43620MC3RPERQ1 (PR #57).
- **Issue item 5 is withdrawn.** Constraints §3.1, §3.4 and §7 and
  REQ-RIF-007 still describe a VBUS supply to the radio; #9 edits them under
  ADR-0003.
- **Shared with variant M (ADR-0004 owns them):** the buck, the 2.304 MHz
  clock and its distribution. Variant R adds only the input protection, the
  USB-C sink and the TPS2121 mux.
- **Supersedes the power lines of the shortlist**
  ([`core-devices.md`](../research/core-devices.md#5-power-10-11) §5:
  TPS2121 → TPS62933 → TPS7A20). The TPS2121 and TPS7A20 stay.
- **EU EMC (#59):** a ferrite-and-MLCC pi filter between the mux and the buck
  keeps ripple off the radio's DC lead. The surge immunity level and whether
  the fuse chain passes it are **(verify)**; the TPS26600 is the fallback.
- **Sourcing risk:** LMR43620MC3RPERQ1 stock is 10 at LCSC and 3,500 at
  TI.com (2026-09-25); buy from TI and ahead.
- **Risks to verify:** buck efficiency at 2.304 MHz; module current estimates;
  USB 2.0 pre-configuration and suspend limits; frequency foldback from USB
  sources below about 4.2 V; 6 m harmonics (22nd, 23rd); codec output swing
  at 3.0 V AVDD (#8); whether radios attach on USB without VBUS from the
  device (#9); REACH SVHC per part.
- **Follow-ups:** a bench `human-task` for the radio DC pin limits (#5, "Needs
  measurement", item 1), the budget, the noise floor and EU pre-compliance; battery
  operation as a future variant (research doc §12).

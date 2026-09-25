<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Radio interface circuits: SERIAL jack, PTT, RTS/DTR, isolation and USB routing (revision A)

Issue: [#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9).
Researched 2026-09-24. Decision record:
[ADR-0003](../decisions/ADR-0003-radio-interface-circuits.md) (proposed).

Inputs:

- the per-radio table [`radio-interfaces.md`](radio-interfaces.md) (#5);
- host compatibility [`host-compatibility.md`](host-compatibility.md) (#6);
- the audio codec decision [ADR-0002](../decisions/ADR-0002-audio-codec.md) (#8);
- the module integration notes in [`fcc.md`](../compliance/fcc.md) (#7);
- [ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md),
  [`constraints.md`](../requirements/constraints.md),
  [`radio-connectors.md`](../requirements/radio-connectors.md) and
  [`pcb-fabrication.md`](../requirements/pcb-fabrication.md).

This is a design study with reference circuits, not a schematic. Schematic
capture and layout come later (#21 and the hardware issues).

## How to read this

- Datasheets, SDK files and other sources are cited through the
  [reference index](../references/index.md), with local copies in the
  gitignored cache (`python3 tools/refs/refs.py fetch`).
- **(verify)** marks a value that isn't confirmed from a primary source, or
  that needs a bench test. **`unknown`** means no source was found. Radio facts
  come from #5, which cites the manufacturers' manuals.
- Prices and stock are from **LCSC on 2026-09-24**, in USD per unit at the
  quantity break shown. **Digi-Key and Mouser checks are deferred by the
  maintainer** (2026-09-24) and are left blank below, marked "deferred".
- Directions are from the interface's point of view, as in
  [`radio-connectors.md`](../requirements/radio-connectors.md).

## Summary

| Block | Proposed part(s) | Alternates | Key point |
|---|---|---|---|
| SERIAL-jack mode switching | ADI/Maxim **MAX14778** (dual 4:1, ±25 V signals, single 3.0–5.5 V supply) | TI TMUX6219 (needs about ±16 V rails); Omron G6K-2F-Y signal relays | One part switches tip and ring 1 between logic, RS-232 and CI-V without a dual supply (§3.2) |
| RS-232 | TI **TRS3221E** | MaxLinear SP3221E, ADI ADM3101E (from [`core-devices.md`](core-devices.md)) | One driver and one receiver; FORCEOFF shuts both off (§3.4) |
| Logic / CI-V glue | TI SN74LVC1G07 (open drain), SN74LVC1G11 (3-input AND), SN74LXC1T45 (TX buffer) | Nexperia 74LVC equivalents | All behind the ±25 V switch (§3.5, §3.6) |
| Serial isolation | TI **ISO7721** (data), TI **ISO1540** + **TCA9534** (control) | Skyworks Si8621 | Fitted per the ADR-0002 approach (§6) |
| Isolated supply (3.3 V in, no 5 V rail) | **Discrete push-pull at 2.304 MHz**: SN74LVC2G02 + 2× AO3400A + 1:1.3 transformer, driven by the board's 2.304 MHz master clock | TI SN6505B clocked at 1.152 MHz (576 kHz switching) | The only option whose comb adds nothing to the HF bands (§6.2, §7.1) |
| PTT closure | Panasonic **AQY212EH(AX)** PhotoMOS | Littelfuse CPC1017N | 60 V, AC/DC, off unless its LED is driven (§4) |
| PTT fail-safe | TI **TPS3839G33** supervisor (brownout; also the ESP32 EN reset), **ESP32-S3 internal watchdogs**, SN74LVC1G08 2-input AND, PTT_REQ pull-down | — | No external PTT timer or window watchdog (maintainer decision, 2026-09-25; §4.2) |
| USB hub | Microchip **USB2422** (−I, industrial) | WCH CH334F | GL850G rejected (0 to +85 °C) (§8.2) |
| USB 2.0 switches | TI **TS3USB221A** (×2) | onsemi FSUSB42 | Disabled (OE high) until firmware picks a mode (§8.1) |
| USB-C port | HRO TYPE-C-31-M-12, 5.1 kΩ Rd | M-grade connector (verify) | −30 to +80 °C: fine for R, not for M (§8.4) |
| Radio USB VBUS | **Not connected to any device rail, as fitted by default** (maintainer decision) | DNP test footprints: sense-only feed; 0 Ω bypass from 3.3 V or USB-C VBUS (§8.5) | Many radio USB chips may not attach without VBUS (§8.5) |
| Radio USB isolator (variant M option) | ADI ADuM4160 | ADI ADuM3160 | Breaks the ground path through the radio USB cable; runs from 3.3 V on side 1 and an isolated 3.3 V on side 2 (§8.6) |

Main findings:

1. **CP2105 is supported** by the esp-usb CP210x driver (PID 0xEA70, per-port
   interface index); the **CH342 is not** in the CH34x driver but is a CDC-class
   device; the **FTDI driver's RTS/DTR request doesn't match** the FTDI
   modem-control request documented elsewhere (§9).
2. **Without VBUS, many radio USB ports may not enumerate.** The CP2105 and
   CH342 datasheets make VBUS a connect-sense input. The FT-891 and X6100 have no
   other CAT path, so this decision could leave them without CAT (§8.5).
3. **PTT fails safe by circuit:** the PhotoMOS is off unless its LED is
   driven; the drive is an AND of a pulled-down GPIO and a brownout
   supervisor; firmware lock-ups are caught by the ESP32-S3's internal
   watchdogs, whose reset returns the GPIO to its pulled-off default (§4).
   The maximum TX time is a firmware setting (default 5 minutes, no upper
   limit; protocol PR #58).
4. **The TMUX6219 isn't power-off protected:** its signals must stay within
   its supply rails, even unpowered. A single-supply ±25 V multiplexer
   (MAX14778) avoids the dual rail (§3.2).
5. **Jack isolation is bypassed through the radio** whenever the radio's USB
   cable is also connected and the ADuM4160 isn't fitted (§6.3).
6. **Clock audit (§7.1):** with everything on the 2.304 MHz master, no
   deliberate clock lands in an HF band except the I2S word clock and I2C
   (slow edges, low-level combs) and the 64 fs vs 32 fs BCLK choice. 6 m, 2 m
   and 70 cm can't be avoided by frequency choice; the USB2422's 24 MHz crystal
   (6th harmonic) and the USB full-speed 12 MHz (12th) both sit on
   **144.000 MHz**, the 2 m band edge.

## 1. Verify-first results

| Item (issue #9) | Result |
|---|---|
| CI-V electrical details from the manufacturer via #5 | **Partly documented.** #5 confirms a single-wire CI-V bus on the IC-7300 [REMOTE] 3.5 mm jack and that a PC needs a CT-17 converter; the jack's tip/sleeve assignment is only in a figure, and **no pull-up voltage, current or thresholds** were found. The circuit is therefore designed to tolerate any pull-up from 3.3 V to 5.5 V behind the ±25 V switch, with the values marked (verify) (§3.6). Not blocking. |
| Each radio's PTT pin logic via #5 | **Logic documented, levels mostly `unknown`.** All PTT inputs in #5 are "ground to transmit" closures (FT-817/818 "ground to transmit", K4 "pull to ground", IC-7300 SEND TX −0.5 to +0.8 V). Documented limits: IC-7300 SEND RX 2.0–20.0 V, 20 mA max; KX3 ACC2 GPIO 3 V logic, 500 Ω series, 0–5.5 V. The closure is sized to a conservative envelope (§4.1). Not blocking. |
| USB-host driver support for CP2105/FTDI with a cited source | **Checked in esp-usb at commit `bf0f0aa3`** (Apache-2.0). CP2105: supported. FTDI: FT232/FT231X only, with an RTS/DTR request that needs checking. CH342: via the CDC-ACM driver (verify). Details in §9. Linux's GPL FTDI header was read as a fact reference only. |

## 2. Architecture

```text
                   device domain (3.3 V, ESP32-S3 ground)          │ jack domain (isolated on M; see §6)
                                                                   │
 ESP32 UART TX ──► ISO7721 ch1 ─────────────────────────────────── ┼─► TX fan-out ─► logic TX buffer ─┐
 ESP32 UART RX ◄── ISO7721 ch2 ◄────────────────────────────────── ┼── 3-in AND ◄── logic RX, RS-232  │
 ESP32 I2C ──────► ISO1540 ────────────────────────────────────────┼─► TCA9534 ─► mode selects, FORCEOFF, ring-2 enable
 2.304 MHz clock ─► push-pull (2 FETs) ═══ transformer ════════════ ┼═► rectifier ─► ≈ 3.8 V ─► LDO ─► 3.3 V_ISO
                                                                   │
                                                                   │   SERIAL tip    ◄─ MAX14778 mux A ◄─ logic TX / RS-232 DOUT / CI-V node
                                                                   │   SERIAL ring 1 ─► MAX14778 mux B ─► logic RX / RS-232 RIN
                                                                   │   SERIAL ring 2 ◄─ PhotoMOS ◄─ current limiter ◄─ 3.3 V_ISO
 PTT_REQ ─┐                                                        │
 RESET ───┴─► 2-in AND ─► PhotoMOS LED ═════════════════════════════ ┼═► AUDIO ring 2 closure to sleeve
             (TPS3839 RESET)                                       │
 codec ──────────────────────────────── SM-LP-5001E transformers ══┼═► AUDIO tip / ring 1 (ADR-0002)
```

- The **AUDIO and SERIAL sleeves** are the jack-domain ground. On variant M
  (and whenever isolation is fitted) it is isolated from the device ground; on
  a bypassed build it joins it through 0 Ω links (§6).
- The **PTT closure needs no jack-domain power**: its LED is on the device
  side and its output switch is self-contained. PTT therefore works, and stays
  fail-safe, with the isolated supply off.

## 3. SERIAL jack

### 3.1 What it must survive

[`radio-connectors.md`](../requirements/radio-connectors.md) and
REQ-RIF-005: RS-232 levels up to ±15 V on **any** contact, in **any** mode,
must not damage any path. Cases that do happen:

- an RS-232 radio (FT-991A, TS-590S/SG, K3/K3S, K4) on ring 1 while logic mode
  is selected;
- a foreign driver on tip (wrong cable, or a radio output wired to tip);
- the KX2 ACC cable, whose ring 2 is the radio's key-out line (30 V, 100 mA,
  open drain, [`radio-interfaces.md`](radio-interfaces.md#elecraft-kx2-and-kx3));
- the device **unpowered** with the cable connected and the radio on.

### 3.2 Mode switching options

| Option | How ±15 V is survived | Supply | Temp. | $ @100 (LCSC) | Verdict |
|---|---|---|---|---|---|
| **A. MAX14778**, one 4:1 mux per contact (tip = mux A, ring 1 = mux B) | Signals to ±25 V "above and below the rails" from a single 3.0–5.5 V supply; ±6 kV HBM on the signal pins; supports RS-232/RS-485 and USB 1.1 muxing ([LCSC C1121866](https://www.lcsc.com/product-detail/C1121866.html); [datasheet](../references/index.md#adi-max14778-ds)) | 3.3 V_ISO (jack domain) | −40 to +85 °C | 4.24 | **Proposed.** No dual rail; one part for two contacts. Behavior with ±15 V applied while **unpowered** and the absolute maximum ratings: **(verify)** from the datasheet (ADI blocks scripted downloads; not read in full). |
| B. TMUX6219, one SPDT per contact | Signals must stay within VSS–0.5 V to VDD+0.5 V (absolute maximum), so the rails must span ±15.5 V or more; no power-off protection is stated ([datasheet](../references/index.md#ti-tmux6219-ds) §5.1) | ±4.5 to ±18 V dual; generating about ±16 V in the jack domain needs a second transformer winding or a charge pump | −40 to +125 °C | 2.18 each (×2) | Rejected: needs a ±16 V isolated supply that must be up whenever a cable is live, including with the device off, or series resistors and clamps that it wasn't designed for. |
| C. Signal relays (Omron G6K-2F-Y DC3, DPDT) | Contacts don't care about polarity or power | 3 V coil, about 100 mW each | **−40 to +70 °C** | 1.08 each | Robust but fails the variant M 85 °C range, draws coil power, and is mechanical. Fallback if option A fails its unpowered test. |

Series resistors (pulse-rated, per
[`pcb-fabrication.md`](../requirements/pcb-fabrication.md) §5) and TVS diodes
stay in front of the switch in every option (§7).

### 3.3 Channel map

| Mode (firmware) | Tip (mux A) | Ring 1 (mux B) | Ring 2 | TRS3221E |
|---|---|---|---|---|
| **3.3 V logic (power-on default)** | Logic TX buffer | Logic RX buffer | Off | FORCEOFF low (off) |
| RS-232 | TRS3221E DOUT | TRS3221E RIN | Off | On |
| CI-V | CI-V node (open drain + receive tap) | Off | Off | Off |
| 3.3 V logic + power | Logic TX buffer | Logic RX buffer | 3.3 V out (limited) | Off |
| All off (mode change, fault) | Off | Off | Off | Off |

- **Mode changes** go through "all off" first (break before make) and never
  touch the PTT path (REQ-PTT-009).
- **Power-on default:** the TCA9534 powers up with all pins as inputs
  ([datasheet](../references/index.md#ti-tca9534-ds)), so pull resistors on its
  outputs set the default: mux A and B on the logic inputs, TRS3221E FORCEOFF
  low, ring 2 off. The same pulls hold while the ESP32 is in reset.
- **RX combining:** the three receive paths idle high (UART mark), so a 3-input
  AND (SN74LVC1G11) merges them into one UART RX: the logic RX buffer, the
  TRS3221E ROUT (with a pull-up; EN high sets it to high impedance when unused)
  and the CI-V receive tap. Unselected inputs stay high, so the AND passes only
  the active path.
- **TX fan-out:** the one UART TX drives the logic TX buffer, TRS3221E DIN and
  the CI-V open-drain gate. Only the path the mux selects reaches a contact.

### 3.4 RS-232: TRS3221E

From the [TRS3221E datasheet](../references/index.md#ti-trs3221e-ds):

- One driver, one receiver; 3.0–5.5 V supply; up to 250 kbit/s (115200 baud
  needs ≤ 250 kbit/s); `I` grade −40 to +85 °C.
- **FORCEOFF low (with EN high) shuts off both driver and receiver** (1 µA
  supply); driver output leakage when off is ±25 µA at ±12 V (3.0–3.6 V
  supply); powered-off driver output resistance 300 Ω minimum.
- Absolute maximum: RIN ±25 V, **DOUT ±13.2 V**. The driver is therefore
  disconnected by the mux in every non-RS-232 mode. In RS-232 mode a foreign
  ±15 V driver on tip is a driver-to-driver fault: a series resistor (about
  300 Ω–1 kΩ, pulse-rated) limits the current, and the tip TVS clamps above
  24 V. Whether DOUT survives that indefinitely: **(verify)** on the bench.
- Receiver input resistance 3–7 kΩ; hysteresis 0.5 V typical; INVALID reports
  whether a valid RS-232 level is present (readable through the TCA9534, to
  help auto-detect a wrong mode).
- The MAX3243E family named in the constraints has three drivers; they aren't
  needed because the jack has no RTS/DTR contacts.

### 3.5 3.3 V logic and 5 V TTL (FT-710 CAT-3, FT-817/818 ACC)

- **Receive (ring 1):** mux B → series resistor → 5.5 V-tolerant Schmitt input
  (74LVC-family, 3.3 V supply), with a pull-up to 3.3 V_ISO so an open ring 1
  idles as mark. 5 V TTL from the FT-710 is within the input's tolerance.
- **Transmit (tip):** a 3.3 V push-pull buffer (SN74LXC1T45 with VCCB =
  3.3 V_ISO) → series resistor → mux A. The FT-710 CAT-3 RXD is 5 V TTL
  ([`radio-interfaces.md`](radio-interfaces.md#yaesu-ft-710)); whether 3.3 V is a
  valid high there is `unknown`. The board has **no 5 V rail** (maintainer
  decision, 2026-09-24), and the jack domain's rectified supply is only about
  3.8 V, so a 5 V-high option isn't offered. Common 5 V TTL inputs switch at
  1.4–2.0 V, which 3.3 V clears, but that is general knowledge, not a Yaesu
  figure: **measure** it (#5 item 6). If it fails, the FT-710 uses USB for CAT.
- Baud rates 4800–115200 are far inside every part's speed.

### 3.6 Icom CI-V (tip only)

- **Transmit:** SN74LVC1G07 open-drain buffer pulls the CI-V node low for a
  0 bit; the node goes through a series resistor and mux A to the tip.
- **Receive:** the same node feeds a Schmitt input (the CI-V tap into the RX
  AND). The device therefore hears its own transmission (**echo**).
- **Pull-up:** the radio pulls the bus up
  ([`radio-connectors.md`](../requirements/radio-connectors.md#serial-jack-35-mm-trrs)).
  Its voltage and strength aren't documented in the sources #5 found
  **(verify)**. A local pull-up footprint to 3.3 V_ISO is provided **DNP**, for
  a bus without a pull-up (for example two radios on a cable with no
  controller). The LVC1G07 output and the Schmitt input tolerate up to 5.5 V.
- **Collisions and echo** (REQ-CAT-007) are firmware (#15): compare each echoed
  byte with the byte sent; on a mismatch, stop sending, wait for the bus to go
  idle and retry. The exact rule belongs in the CAT design.
- IC-7300 CI-V baud is Auto, 4800–115200
  ([`radio-interfaces.md`](radio-interfaces.md#icom-ic-7300)).

### 3.7 Ring 2: 3.3 V out (logic + power mode)

**Maintainer decision (2026-09-25): keep it.** The "3.3 V logic + power" mode
puts about 3.3 V, limited to about 20 mA and short-circuit protected, on
SERIAL ring 2. It is off unless that mode is selected. It is a **deliberate
exception** to "the device only takes power in": it powers **cable-side
circuits** (cables with their own isolation or level-shift circuit), for
compatibility with the existing cable convention, and **never the radio**.

It must also survive ±15 V, and the KX2 cable puts the radio's key-out line
on ring 2.

```text
jack-domain rectified rail (≈ 3.8–4.1 V, before the 3.3 V_ISO LDO; §6.2)
   │
   R_S 27 Ω ──┬─────────────── Q2 base               (BC857BS dual PNP: Q1 pass, Q2 limit)
   │          │
   Q1 E       Q2 E                Q2 C ──► Q1 base; Q1 base ── 4.7 kΩ ── RING2_EN_n (TCA9534, default high = off)
   Q1 C ──► BAT54 (30 V reverse) ──► PhotoMOS AQY212EHAX output (60 V, AC) ──► 10 Ω ──► ring 2
                                        LED ◄── 680 Ω ◄── TCA9534 "RING2_EN" (default off)
   Q2 C also ──► 100 kΩ divider ──► TCA9534 input "RING2_LIMIT" (high while the limiter is active)
```

- **Current limit:** Q2 turns on when the drop across R_S reaches about
  one V_BE (≈ 0.6 V), so the limit is ≈ 0.6 V / 27 Ω ≈ 22 mA, falling at hot
  temperature (V_BE's tempco; **verify** the spread over −40 to +85 °C).
- **Voltage at ring 2:** ≈ 3.9 V − 0.3 V (R_S at 10 mA) − 0.1 V (Q1
  saturation) − 0.3 V (BAT54) − 0.01 V (PhotoMOS, 0.85 Ω typ) ≈ **3.2 V at
  10 mA**, lower near the limit (**verify** on the bench; the convention only
  asks for "3.3 V, about 20 mA").
- **Off by default:** the PhotoMOS LED is undriven (TCA9534 powers up with
  all pins as inputs; pull-downs hold it off) and Q1's base is pulled off. In
  the off state the AQY212EH blocks up to 60 V in both polarities
  ([datasheet](../references/index.md#panasonic-aqy21eh-ds)), so ±15 V on
  ring 2 does nothing.
- **Short to sleeve:** the limiter holds ≈ 22 mA; Q1 dissipates about
  4 V × 22 mA ≈ 90 mW, inside the BC857BS's 200 mW per transistor and
  300 mW per package at 25 °C
  ([datasheet](../references/index.md#nexperia-bc857bs-ds)); derating to
  85 °C is **(verify)**.
- **On, with a foreign voltage applied:** +15 V is blocked by the BAT54. At
  −15 V the limiter holds 22 mA but Q1 would dissipate about
  (4 + 15) V × 22 mA ≈ 0.42 W, above its rating. The `RING2_LIMIT` flag goes
  high; firmware turns ring 2 off within milliseconds (**verify** Q1 survives
  that pulse; a PTC in series is the fallback).
- **Firmware rule:** never select this mode for a radio profile whose cable
  uses ring 2 for another function (KX2).

**Cost (LCSC, 2026-09-24/25):** AQY212EHAX $0.98 @100 (C29276), BC857BS
$0.056 @100 (Nexperia, [C8654](https://www.lcsc.com/product-detail/C8654.html),
930 in stock, RoHS3), BAT54 $0.03 (BAT54S,
[C47546](https://www.lcsc.com/product-detail/C47546.html), 283,320 in stock,
RoHS3), four resistors: **about $1.10 per board.** Using the AQY212EHAX (the
same part as the PTT closure) instead of a CPC1017N saves $0.41 and lowers
the on-resistance from 16 Ω to 2.5 Ω max.

### 3.8 Why not a solder-jumper design

The convention uses solder jumpers. Firmware selection (REQ-CAT-008) needs the
switch in §3.2, and firmware can't forget to re-jumper; the cost is about
$4.24 (MAX14778) plus the control path.

## 4. PTT (AUDIO jack ring 2)

### 4.1 Closure device and ratings

| | Panasonic AQY212EH(AX) (proposed) | Littelfuse CPC1017N (alternate) |
|---|---|---|
| Load voltage | 60 V peak AC (recommended ≤ 48 V) | 60 V peak |
| Load current | 0.55 A continuous (AC/DC) | 100 mA (80 mA at 80 °C) |
| On-resistance | 0.85 Ω typ, 2.5 Ω max | 16 Ω max |
| LED operate / turn-off | 3.0 mA max to operate; 0.4 mA min turn-off; 5–30 mA recommended | 1 mA max to operate; 0.3 mA min turn-off |
| Isolation | 5,000 V rms | 1,500 V rms |
| Off-state leakage | 1 µA max | 1 µA max |
| Temp. | −40 to +85 °C | −40 to +85 °C |
| Source | [Panasonic GE DIP4 1 Form A (ASCTB126E)](../references/index.md#panasonic-aqy21eh-ds) | [DS-CPC1017N-R08](../references/index.md#littelfuse-cpc1017n-ds) |

Against the radios in #5
([`radio-interfaces.md`](radio-interfaces.md#audio-and-ptt-electrical-levels)):

- **IC-7300 SEND:** TX needs −0.5 to +0.8 V at up to 20 mA. AQY212EH:
  20 mA × 2.5 Ω = 50 mV; CPC1017N: 20 mA × 16 Ω = 0.32 V. Both pass. RX idle
  up to 20 V is inside both ratings.
- **KX3 ACC2 GPIO:** 3 V logic through 500 Ω, tolerant of 0–5.5 V; either
  part pulls it well below any logic low.
- **Every other radio:** PTT voltage and current are `unknown`
  ([Needs measurement](radio-interfaces.md#needs-measurement) item 4). The
  design envelope is **≤ 48 V and ≤ 0.5 A** (AQY212EH recommended conditions),
  either polarity (AC/DC output), which covers the logic-level and 12–20 V
  keying lines normal on current transceivers. A radio outside it needs a
  cable with its own relay.
- The closure is a real short to sleeve, electrically like the convention's
  open-collector NPN, but without the NPN's polarity or saturation-voltage
  limits.

**AQY212EH is proposed** for its low on-resistance and 0.55 A margin. It
needs 5 mA of LED drive, more than the CPC1017N's 1 mA; the drive budget is
trivial. SMD order code: `AQY212EHAX` (tape and reel).

### 4.2 Fail-safe drive (circuit level)

Maintainer decision (2026-09-25): **no hardware PTT timer or window watchdog
outside the ESP32-S3.** Lock-up protection is the ESP32-S3's internal
watchdogs, and the maximum TX time is a firmware setting (default 5 minutes,
no upper limit; protocol PR #58). The drive circuit must therefore make PTT
**off by hardware default** whenever the ESP32-S3 isn't actively driving it.

```text
             3.3 V
               │
            (TPS3839G33: push-pull RESET, active low; also drives ESP32 EN)
               │
 TPS3839 RESET ───────────────────── A ┐
                                       ├─ SN74LVC1G08 ─── R_LED 390 Ω ─┬─ PhotoMOS LED (AQY212EH, pins 1–2) ── GND
 ESP32 PTT_REQ (GPIO) ──┬─────────── B ┘   2-input AND                  │
                        │                                              └─ (optional) BAT54 ── TPS3839 RESET: backup clamp, 0 Ω/NC
                     100 kΩ pull-down
                        │
                       GND
 Status LED: AND output ─► 1 kΩ ─► red LED ─► GND (lit only while the PhotoMOS LED is driven)
 PTT_STATUS readback: AND output ─► ESP32 input (through 10 kΩ)
```

The closure is **on only if every one of these holds**:

1. **PTT_REQ is high.** It has a 100 kΩ pull-down, so a GPIO that is reset,
   unconfigured or unpowered reads low. It uses a pin that is neither a
   strapping pin nor one with power-up glitches
   ([`fcc.md`](../compliance/fcc.md) §1.6).
2. **RESET is high.** The [TPS3839G33](../references/index.md#ti-tps3839-ds)
   (push-pull, active low) holds RESET low whenever VDD is below its
   threshold (3.003–3.126 V, 3.08 V typical) and for 200 ms (typical) after
   it rises above it. Its output is specified to be in the correct state for
   VDD above 0.6 V. The same RESET drives the ESP32 EN pin (in place of the
   usual RC), so brownout resets the MCU and blocks PTT in one step. The
   3.3 V rail must stay above 3.126 V at its low tolerance (#10/#11).
3. The PhotoMOS LED gets at least 3.0 mA (operate current, maximum). With
   R_LED = 390 Ω and VF ≈ 1.14 V at 5 mA, a 3.3 V gate output gives about
   5.5 mA. Below the PhotoMOS turn-off current (0.4 mA minimum) the output is
   guaranteed open.

#### 4.2.1 Firmware lock-up: the ESP32-S3's internal watchdogs

The ESP32-S3 has main-system watchdogs (used by ESP-IDF's interrupt watchdog
and task watchdog) and an RTC watchdog. On timeout they escalate to a CPU,
main-system, or main-system-and-RTC reset
([ESP-IDF watchdogs](../references/index.md#esp-idf-wdts)).

- **Timeout:** a few seconds (#15 sets the values). The task watchdog must
  watch the task that owns PTT, so a stuck PTT state machine is caught, not
  only a stuck idle task.
- **During and after the reset** the GPIO matrix returns PTT_REQ to a
  high-impedance input, so the 100 kΩ pull-down turns PTT off; the AND gate
  output goes low and the PhotoMOS opens. The firmware re-enables PTT only by
  an explicit new command after boot (REQ-PTT-005).
- **Limits (accepted by the maintainer):** firmware that runs but misbehaves
  while still feeding the watchdog, firmware that disables it, and a failed
  chip are not covered by any hardware backstop. The firmware maximum TX
  timer (default 5 minutes) and the radio's own time-out timer, where it has
  one, cover the first case.

**Rejected (maintainer, 2026-09-25):** an external TPL5111 PTT timer and a
TPS3430 window watchdog, evaluated in an earlier revision of this PR. Reason:
no external PTT timer is wanted, and they add cost (about $0.46–0.83 and
$1.16).

#### 4.2.2 Brownout: why the TPS3839 stays

The maintainer asked whether the ESP32-S3's internal brown-out detector could
replace the TPS3839. It can't **guarantee** PTT off in a brownout:

- ESP-IDF describes its thresholds (2.44–3.30 V, selectable) as approximate:
  "there may be some variation of brownout voltage level between each
  ESP32-S3 chip", and the source notes that the levels are "estimates"
  ([ESP-IDF brownout Kconfig](../references/index.md#esp-idf-s3-brownout-kconfig)).
- It is enabled and configured by firmware (`CONFIG_ESP_BROWNOUT_DET`,
  default on), so a firmware build can turn it off, and an interrupt mode
  lets software run before the restart.
- Below the chip's operating range (3.0 V minimum), nothing specifies the
  state of a GPIO that was driving PTT high. At about 2.5 V, a GPIO still
  high would drive about (2.5 − 1.14) V / 390 Ω ≈ 3.5 mA, enough to keep the
  PhotoMOS closed.

The TPS3839G33 is specified from VDD = 0.6 V, independent of firmware, and
costs $0.26 at 100+. It also replaces the RC delay on the ESP32 EN pin that the
module needs anyway ([`fcc.md`](../compliance/fcc.md) §1.4). **Keep it.**

Brownout, rail by rail:

| 3.3 V rail | TPS3839 RESET | Gate (1.65–5.5 V spec) | LED current | Closure |
|---|---|---|---|---|
| < 0.6 V | Undefined | Unpowered | < 0.4 mA (below the LED's forward voltage) | **Open** |
| 0.6 V to 1.65 V | **Low** | Outside its specified range | Backup: RESET clamps the LED node (optional link); the rail is too low to push 3 mA through the LED and 390 Ω **(verify on a slow-ramp bench test)** | **Open** |
| 1.65 V to 3.08 V | **Low** | Output low (input A low) | 0 | **Open** |
| Above threshold, during the 200 ms delay | Low | Low | 0 | **Open** |
| Normal | High | Follows PTT_REQ | 5.5 mA when requested | Requested state |

The **backup clamp** costs one 0 Ω link and a Schottky. It sinks about 8.5 mA
(3.3 V / 390 Ω) into the TPS3839's push-pull output only in a fault where the
gate is high while RESET is low; its sink rating at that current is
**(verify)**. Fit it only if the slow-ramp test shows LED current in the
0.6–1.65 V band.

**RF pickup can't key the radio** (REQ-EMC-001): the closure turns on only
from LED current on the device side. RF on the AUDIO jack reaches the
PhotoMOS output, which has no gain path back to its LED. PTT_REQ is a short,
RC-filtered trace on the device side.

**Status LED:** driven from the same gate output, so it shows the real drive
state, not the firmware request.

**Scope:** this covers the AUDIO-jack closure. PTT sent as a CAT command, or
as RTS/DTR to a radio's USB-serial chip, is covered by the firmware max-TX
timer and the radio's own time-out timer.

### 4.3 PTT state at every event

| Event | What turns PTT off | Layer |
|---|---|---|
| Power-on | PTT_REQ pull-down; TPS3839 RESET low for 200 ms | Hardware |
| Reset (EN low, software reset, watchdog reset) | GPIO goes high impedance → pull-down | Hardware |
| Brownout | TPS3839 RESET (table in §4.2.2); the ESP32-S3 brown-out detector also resets the chip | Hardware |
| Firmware lock-up | ESP32-S3 internal watchdog reset (seconds) → pull-down | On-chip + hardware default |
| Continuous TX > MAX_TX (default 5 min) | Firmware max-TX timer | Firmware (#15) |
| Host link lost, keepalive missing, mode change | Firmware (REQ-PTT-005, -006, -009) | Firmware |
| Isolated supply off | Nothing needed: the closure doesn't use it | — |

## 5. RTS/DTR mapping

| Host link | Source of RTS/DTR | Target options (configurable) |
|---|---|---|
| Wired USB-C | Host → device's own CDC-ACM port: `SET_CONTROL_LINE_STATE` | AUDIO-jack PTT closure (RTS, DTR or neither) |
| Wired USB-C | Host → **radio's own** USB-serial chip, through the hub | Not in the device's path at all; the radio's own settings decide |
| Bluetooth LE | Protocol RTS/DTR messages (#13) | AUDIO-jack PTT closure, **or** RTS/DTR on the radio's USB-serial chip through esp-usb |

Requests the ESP32-S3 sends to a radio chip in Bluetooth mode (esp-usb at
commit `bf0f0aa3`; see §9 for status):

| Chip | Request | wValue |
|---|---|---|
| CP210x / CP2105 | Vendor `SET_MHS` (0x07), wIndex = interface | bit 0 DTR, bit 1 RTS, bits 8–9 masks ([AN571 §5.9](../references/index.md#silabs-an571)); the driver sends mask 0x0300 |
| FTDI | Driver sends vendor request 0x02 with 0x11/0x10 (DTR) and 0x21/0x20 (RTS) | **(verify)**: Linux's FTDI header defines modem control as request 1 with 0x0101/0x0100 and 0x0202/0x0200, and request 2 as flow control ([ftdi_sio.h](../references/index.md#linux-ftdi-sio-h), GPL, facts only) |
| CH340/CH341 | Vendor 0xA4 (MODEM_OUT), DTR 0x20 and RTS 0x40, active low | As the driver implements it |
| CDC-ACM (CH342, others) | Class `SET_CONTROL_LINE_STATE` (0x22) | bit 0 DTR, bit 1 RTS |

**Open-assert hazard.** Linux raises DTR and RTS when a CDC-ACM port opens
and drops them when it closes
([`host-compatibility.md`](host-compatibility.md#62-requirements-for-the-firmware-and-protocol)).
Rules:

1. **Device's own port (wired mode):** RTS/DTR→PTT keys only on a state
   **change after open**, never on the level present at open (#15). The
   default mapping is none; the user enables RTS or DTR explicitly.
2. **Radio's chip in wired mode:** the device can't filter it. Users rely on
   the radio's own settings: IC-7300 "USB SEND" defaults to OFF and its
   "Inhibit Timer at USB Connection" defaults to ON; the FT-991A manual warns
   that the PC may key the radio when it starts
   ([`radio-interfaces.md`](radio-interfaces.md)). User documentation must say
   so.
3. **Radio's chip in Bluetooth mode:** right after opening the chip, firmware
   sets DTR and RTS **inactive** before enabling any mapping. Whether esp-usb
   changes the lines on open is **(verify)** in #43.

## 6. Isolation

### 6.1 What is isolated, and how it's fitted

| Function | Part | Variant M | Variant R (default) |
|---|---|---|---|
| AUDIO audio | SM-LP-5001E transformers (ADR-0002) | Fitted | DNP, 0 Ω bypass (ADR-0002) |
| AUDIO PTT | AQY212EH PhotoMOS | Fitted | **Fitted** (it is the closure itself) |
| SERIAL data | ISO7721 (1 forward, 1 reverse; 100 Mbit/s; 5,000 V rms) | Fitted | DNP, 0 Ω links |
| SERIAL control | ISO1540 (I²C, 2,500 V rms) | Fitted | DNP, 0 Ω links |
| Jack-domain supply | Discrete push-pull at 2.304 MHz + transformer + LDO (§6.2) | Fitted | DNP; 3.3 V from the device rail through a 0 Ω link |
| Jack-domain ground | Sleeves of AUDIO and SERIAL | Isolated | Joined to device ground through 0 Ω |

This follows the **ADR-0002 fitting approach**: one layout, fitted on M, DNP
with 0 Ω bypasses on R. It inherits the same **open question**:
[constraints §6](../requirements/constraints.md#6-safety-and-fail-safe)
requires jack isolation "on every variant whenever the USB-C data link is
used", which a default R build doesn't meet in wired mode. This study doesn't
decide it (options in ADR-0002: amend §6, fit isolation on R builds for wired
use, or leave it to #12).

ISO7721 default outputs are high with the input side unpowered (the non-F
variant), which reads as UART idle. Its 100 Mbit/s rating is far above
115,200 baud ([datasheet](../references/index.md#ti-iso7721-ds)).

### 6.2 Jack-domain supply (from 3.3 V, synchronized)

Maintainer direction (2026-09-24): the board has **no 5 V rail**. One 3.3 V
buck, synchronized to a **2.304 MHz** master clock, feeds everything, and its
harmonics miss every HF amateur band
([`power-automotive.md` §6.2](power-automotive.md#62-switching-frequency-against-the-hf-amateur-bands), #10).
The isolated jack-side supply runs from that 3.3 V and must follow the same
rule.

**What the jack domain needs (at 3.3 V_ISO, single rail):**

| Load | Supply need | Current (estimate) |
|---|---|---|
| TRS3221E | 3.0–5.5 V; makes its own ±5.5 V with an internal charge pump and four 0.1 µF capacitors ([datasheet](../references/index.md#ti-trs3221e-ds)) | ≈ 1 mA + RS-232 load (≈ 1–2 mA into 3–7 kΩ) |
| MAX14778 | Single 3.0–5.5 V; no negative rail (the TMUX6219 would need ≈ ±16 V, §3.2) | (verify) |
| ISO7721 side 2, ISO1540 side 2 | 2.25–5.5 V / 3.0–5.5 V | a few mA each |
| TCA9534, logic, CI-V | 1.65–5.5 V | µA to 1 mA |
| Ring 2 "3.3 V out" | 3.3 V, limited to ≈ 20–25 mA | ≤ 25 mA |
| **Total** | | **≈ 40 mA** (design for 60 mA) |

**Options, all from 3.3 V** (harmonic check: script over every n·fsw to
450 MHz, ±0.01 % clock error, US band edges per 47 CFR 97.301 as in #10):

| Option | Switching frequency | New lines in HF bands (160–10 m) | Cost (LCSC, 2026-09-24) | Verdict |
|---|---|---|---|---|
| **A. Discrete push-pull at 2.304 MHz**: SN74LVC2G02 (NOR, with an enable) makes two antiphase drives from the master clock; each drives an AO3400A through a coupling capacitor with a DC-restore diode and gate pull-down; 1:1.3 center-tapped transformer; Schottky full-wave rectifier; LDO to 3.3 V_ISO | 2.304 MHz (same comb as the buck) | **None** (the 2.304 MHz comb is clean on HF; its 6 m, 2 m and 70 cm lines already exist on the board) | 2G02 $0.17–0.30 + 2× AO3400A $0.06 + transformer (verify) + LDO | **Proposed**: cheapest and the only band-clean option |
| B. TI SN6505B, CLK = 2.304 MHz ÷ 2 = 1.152 MHz (a 74LVC1G74 divider), which the part divides by 2 again | 576 kHz, synchronized; spread spectrum off in external-clock mode | 4: n = 37 (21.312 MHz, 15 m), 49–51 (28.224, 28.800, 29.376 MHz, 10 m); the push-pull primary voltage is a symmetric square wave, so odd harmonics dominate (37, 49, 51) | SN6505B $0.37 + divider | Fallback: known-good soft start and current limit, but lines in 15 m and 10 m (high order, low level, still needs measuring) |
| C. SN6505B on its internal clock | 363–517 kHz with spread spectrum | Every band over its tolerance (for example 424 kHz × 9 = 3.816 MHz, 80 m) | $0.37 | Rejected |
| D. TI SN6507, CLK = 2.304 MHz | 1.152 MHz | 1: n = 25 (28.800 MHz, 10 m) | $2.27+ at LCSC, 149 in stock | Rejected: cost, and still not clean |

Option A details:

- **Transformer:** a center-tapped 1:1.3 push-pull part, as the SN6505B
  datasheet lists for 3.3 V → 3.3 V designs (for example Würth 760390014,
  1:1.3), gives ≈ 3.8–4.1 V after the Schottky rectifier, and an LDO makes
  3.3 V_ISO. Those parts are characterized with the SN6505B at 160–424 kHz.
  At 2.304 MHz the volt-seconds per half cycle fall about 5×, so the core
  doesn't saturate; core loss, leakage-inductance ringing and efficiency at
  2.3 MHz are **(verify)** on the bench. Neither Würth part was found at
  LCSC; distributor checks are deferred.
- **Fail-safe drive:** each gate is AC-coupled with a DC-restore diode and a
  pull-down, so a stopped or stuck clock turns both FETs **off** within a few
  RC time constants (a push-pull stage with one FET stuck on shorts the
  supply through half the primary). The NOR enable (`ISO_PWR_EN`, pulled to
  the "off" state) lets firmware stop the stage.
- **Dead time:** a small RC delay on each gate's rising edge (resistor with a
  bypass diode for the falling edge) keeps both FETs from conducting together.
  The master oscillator's duty cycle sets the balance between the two
  phases; an unbalanced duty cycle adds even harmonics (still multiples of
  2.304 MHz).
- **Soft start and current limit:** there is no built-in limit, so a
  ferrite plus a small series resistor (≈ 1 Ω) at the centre tap limits inrush
  and fault current; a 3.3 V-rail PTC or load switch is an option **(verify)**.
- **Edge control:** gate resistors slow the drain edges to keep the 6 m, 2 m
  and 70 cm lines low; the whole stage sits under a local ground pour with the
  transformer next to the isolation gap.
- **Off when unused:** firmware stops the stage whenever the SERIAL jack
  isn't in use (for example a radio on USB), so it emits nothing then. PTT
  doesn't depend on it. It also stops with the rest of the device during
  **auto power-down** (30 s after the radio turns off, configurable;
  accepted by the maintainer, #10/#11).

### 6.3 Ground paths through the radio

If a radio's **USB cable** and an AUDIO or SERIAL cable are connected at the
same time (for example the FT-891: CAT on USB, audio on the AUDIO jack), the
radio's own ground joins the jack domain to the USB ground. Unless the
ADuM4160 is fitted, the USB ground is the device ground (and, in wired mode,
the computer's ground). **Jack isolation is then bypassed through the radio.**
Isolation still helps for radios that use only the jacks. This belongs in the
grounding plan (REQ-EMC-003) and the variants decision (#12).

## 7. RF and ESD hardening

At each jack contact, from the connector inward:

1. **TVS to the sleeve:** bidirectional, stand-off above the highest normal
   signal. SERIAL tip, ring 1 and ring 2: Nexperia PESD24VL1BA (24 V stand-off,
   bidirectional, 11 pF typical, 70 V clamping at 8/20 µs;
   [datasheet](../references/index.md#nexperia-pesd24vl1ba-ds);
   [LCSC C69324](https://www.lcsc.com/product-detail/C69324.html): $0.16 at
   150+, 32,235 in stock). AUDIO ring 2 (PTT): the same part covers radio PTT
   lines up to 24 V; a radio above that needs a higher-voltage TVS (open
   question, since most PTT voltages are `unknown`).
2. **Ferrite bead** in series (about 600 Ω at 100 MHz class), then a **C0G
   capacitor to the sleeve**: about 1 nF on audio, PTT and 4800–19200-baud
   lines; about 220 pF on lines that must pass 115,200 baud RS-232 edges
   (the TRS3221E slew rate limits them anyway).
3. **Series resistor** (pulse-rated): limits current into the TVS and the
   MAX14778 when the TVS clamps above the switch's rating (**verify** the
   MAX14778 absolute maximum).
4. Short, direct returns from TVS and capacitors to the jack sleeve; the
   sleeve connects to the jack-domain ground plane at one point near the
   jacks.
5. **Radio USB-A:** TPD2E2U06 on D+/D− (no VBUS rail pin, so it adds no path
   to the radio's VBUS; [datasheet](../references/index.md#ti-tpd2e2u06-ds)),
   plus a separate ESD diode on the unconnected VBUS pin (§8.5). **USB-C:**
   USBLC6-2SC6 or TPD2E2U06 on D+/D−, ESD on CC1/CC2 and VBUS.

The AUDIO audio-path filtering is in ADR-0002 /
[`audio-codec.md`](audio-codec.md).

**EU immunity target (#59).** The product must meet EU requirements, so EN
55032 Class B emissions and the immunity side of EN 301 489-1/-17 are design
targets alongside FCC Part 15B. For the jack and USB ports that means ESD,
fast transients (EFT/burst), surge and conducted RF on cables. The test
levels for each port type aren't sourced here **(verify, #59)**. Sizing
rules until they are:

- **ESD:** TVS at every contact, as above; the TRS3221E adds ±15 kV HBM and
  ±8 kV IEC 61000-4-2 contact on its RS-232 pins, and the MAX14778 ±6 kV HBM
  on its signal pins. Choose the jack TVS for the IEC 61000-4-2 level #59
  sets, not HBM.
- **EFT/burst and surge on cables:** the series resistors must be
  pulse-rated (pcb-fabrication §5), and the TVS peak-pulse rating (PESD24VL1BA:
  200 W, 3 A at 8/20 µs) must cover the surge level #59 sets for signal ports,
  if one applies to short radio cables **(verify)**.
- **Conducted RF on cables:** the ferrite + C0G filters at each contact, and
  the PTT design's lack of any RF-to-LED path (§4.2), are the defenses; the
  RF level near a 100 W transmitter is likely more severe than the standard's
  test level, so constraints §5 still drives the design.

### 7.1 Clock audit

Maintainer direction (2026-09-24): list every clock on the board with its
harmonics against the amateur bands, including 6 m, **2 m (144–148 MHz)** and
**70 cm (420–450 MHz)**. Method: every n·f up to 450 MHz with ±0.01 %
tolerance, against the US band edges (47 CFR 97.301, the list in
[`power-automotive.md` §6.2](power-automotive.md#62-switching-frequency-against-the-hf-amateur-bands)).
"n" lists the first harmonic numbers that fall inside a band.

| Clock | f | HF (160–10 m) | 6 m | 2 m | 70 cm | Fix / note |
|---|---|---|---|---|---|---|
| Master oscillator (buck sync, codec MCLK) | 2.304 MHz | **clean** | n = 22, 23 | n = 63, 64 | 13 lines | Chosen by #10 for HF; VHF/UHF by edge control and layout |
| Isolated supply, option A | 2.304 MHz | **clean** | same lines as above | same | same | No new lines |
| Isolated supply, option B | 576 kHz | 15 m (n = 37), 10 m (n = 49–51) | 7 lines | 7 | 52 | Fallback only |
| ESP32-S3 crystal | 40 MHz | clean | clean | clean | **n = 11 (440.000 MHz)** | Inside the module's shield; nothing to change. Measure |
| ESP32-S3 USB full speed | 12 MHz bit rate | clean | clean | **n = 12 (144.000 MHz)** | n = 35–37 | Data, not a steady clock; only while USB is active. Short D± runs, series resistors, common-mode choke footprint (DNP) at the USB-A port |
| USB2422 crystal | 24 MHz | clean | clean | **n = 6 (144.000 MHz)** | n = 18 (432.000 MHz) | Fixed by the part. Hold the hub in **reset** (oscillator stopped) in Bluetooth mode; it runs only in wired mode. 144.000 MHz is the 2 m lower edge; measure |
| USB high speed (hub upstream) | 480 MHz | — | — | — | — | Above 450 MHz; only in wired mode |
| I2S BCLK, 32 fs (codec master) | 1.536 MHz | **10 m (n = 19, 29.184 MHz)** | 3 | 3 | 19 | **Use 64 fs** |
| I2S BCLK, 64 fs | 3.072 MHz | **clean** | n = 17 | n = 47, 48 | 10 | Recommended BCLK (codec setting, #8/#16) |
| I2S WS (LRCLK) | 48 kHz | every band (a 48 kHz comb) | many | many | many | Unavoidable for any audio clock; lines are at n ≥ 38, low level. Series resistor at the source, short trace next to ground |
| I²C (ESP32 ↔ TCA9534 via ISO1540) | 100/400 kHz | every band | many | many | many | Bursty, only at mode changes; idle otherwise. Series resistors; 100 kHz |
| UART CAT (up to 115,200 baud) | data | broadband | | | | Data, not a clock; slow edges (RS-232 slew-limited) |
| TRS3221E charge pump | internal, frequency not stated in the datasheet | (verify) | | | | Runs only in RS-232 mode (FORCEOFF low stops it); measure |
| Buck free-run before sync | 2.1–2.3 MHz | 10 m (n = 13) at nominal | | | | #10: lasts only until the oscillator starts |
| Removed: 16 MHz codec MCLK | 16 MHz | clean | clean | **n = 9 (144.000 MHz)** | n = 27, 28 | Removed by ADR-0002 (PR #55); DNP fallback only |

Every clock below about 4 MHz has lines in 6 m (4 MHz wide) and in 2 m and
70 cm; frequency choice can't avoid them. They are handled by slow edges,
short loops, filtering at each connector (§7) and shielding, and must be
**measured** with a receiver or spectrum analyzer on each band (bring-up).
Mixing products between clocks (for example 24 MHz and 2.304 MHz) aren't
covered by this scan **(verify)**.

## 8. USB routing block

### 8.1 Topology and default states

As [ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md#usb-topology):

```text
USB-C ── ESD ── USB2422 upstream
                USB2422 port 1 ── S1.1 ┐
                USB2422 port 2 ── S2.1 ┐│
radio USB-A ── ESD ── [ADuM4160 option] ── S1 common ┘│  S1.2 ── S2.2 (direct link, Bluetooth mode)
ESP32-S3 GPIO19/20 ── 22–33 Ω ── S2 common ───────────┘
```

- **Power-on:** both TS3USB221A switches have OE pulled **high (disabled)**
  until firmware selects a mode. OE high puts the switch in its 1 µA low-power
  state with both paths open
  ([datasheet](../references/index.md#ti-ts3usb221a-ds)). That hides the
  ESP32's D+ toggling at power-up ([`fcc.md`](../compliance/fcc.md) §1.5) from
  the radio and the hub. Power-off leakage: ±2 µA at 0–3.6 V.
- **Wired mode:** S1 → hub port 1, S2 → hub port 2 (ESP32 as device).
- **Bluetooth mode:** S1.2 ↔ S2.2 direct (ESP32 as host). The hub is idle and
  can be held in reset.
- The topology doesn't preclude the planned **CDC-NCM** network function in
  wired mode: it is another function of the ESP32's USB device (#13/#44),
  behind hub port 2, and needs no hardware change.

### 8.2 Hub

| Part | Ports | Temp. | $ (LCSC, 2026-09-24) | Stock | Notes |
|---|---|---|---|---|---|
| **Microchip USB2422T-I/MJ** | 2 | −40 to +85 °C | 1.48 @100 | 403 | Proposed. Integrated D± resistors; strap or SMBus configuration; **HS_DISABLE** bit forces full-speed-only attach ([datasheet](../references/index.md#microchip-usb2422-ds)). [C622610](https://www.lcsc.com/product-detail/C622610.html) |
| WCH CH334F | 4 | −40 to +85 °C | 0.42 @100 | 4,340 | Alternate; datasheet not reviewed here **(verify)**. [C5187527](https://www.lcsc.com/product-detail/C5187527.html) |
| Genesys GL850G-HHY22 | 4 | **0 to +85 °C** | 0.32 @1,000+ | 20,644 | Rejected: fails −20 °C (R) and −40 °C (M). [C136617](https://www.lcsc.com/product-detail/C136617.html) |

USB2422 port power, with no VBUS on the downstream ports (§8.5):

- **PRTPWR1/PRTPWR2** (active-high power-switch enables) are left
  **unconnected**. The hub turns them on after configuration; with nothing
  attached, nothing is powered.
- **PRTPWR1 doubles as the BC_EN1 strap**, sampled at reset with an internal
  pull-down. Leaving it unconnected keeps battery charging **off**, which is
  required: a battery-charging port turns on port power before enumeration.
- **OCS1_N/OCS2_N** have internal pull-ups; left open, the hub never reports
  an overcurrent.
- **VBUS_DET** senses the upstream (USB-C) VBUS through a divider; the hub
  uses it to decide when to attach.

### 8.3 USB switches

| Part | Temp. | $ (LCSC) | Stock | Notes |
|---|---|---|---|---|
| **TI TS3USB221ARSER** | −40 to +85 °C | 0.19 @150 | 91,820 | 900 MHz bandwidth, 6 Ω; 2.3–3.6 V. [C128396](https://www.lcsc.com/product-detail/C128396.html) |
| onsemi FSUSB42MUX | −40 to +85 °C | 0.21 @150 | 43,120 | 720 MHz, 3.9 Ω typ, 3.7 pF; 3.0–4.4 V. [C11355](https://www.lcsc.com/product-detail/C11355.html). Datasheet download was blocked; LCSC listing only **(verify)** |

### 8.4 USB-C port

- HRO **TYPE-C-31-M-12**, 16-pin, **−30 to +80 °C**
  ([LCSC C165948](https://www.lcsc.com/product-detail/C165948.html): $0.12 at
  150+, 79,985 in stock). It covers variant R (−20 to +60 °C) but **not
  variant M (−40 to +85 °C)**: M needs another receptacle **(verify)**.
- 5.1 kΩ Rd on CC1 and CC2 (no PD), ESD on CC and D±, VBUS to the power path
  (#11), the hub's VBUS_DET and an ESP32 sense input.
- The USB-C VBUS goes **nowhere near** the radio port (§8.5).

### 8.5 Radio USB port: no VBUS (maintainer decision)

**Decision (maintainer, 2026-09-24):** the device never supplies power to a
radio through USB, in either mode, and hardware must block any current from
the device into the radio's VBUS. This replaces the current-limited VBUS
switch of ADR-0008 and constraints §3.1/§3.4/§7. The IC-705 therefore can't
charge from the device, and is **low priority** (its own Wi-Fi, Bluetooth and
USB cover most uses). The K4 connects only through its USB-B port; its USB-A
host ports aren't used with this adapter.

**Blocking element: no connection, as fitted by default.** Two DNP test
options sit on the net (maintainer follow-up, 2026-09-25); neither is fitted
in any production build.

```text
radio USB-A pin 1 (VBUS) ──┬── ESD diode to GND (unidirectional, VRWM ≥ 5.5 V, e.g. TPD1E10B06 class)
                           ├── test pad
                           ├── [DNP] R_SENSE 4.7 kΩ ── [DNP] BAT54 ── 3.3 V          (option a: sense-only feed)
                           ├── [DNP] 0 Ω JP_VB3 ── [DNP] PTC 0.5 A ── 3.3 V          (option b1: bypass from 3.3 V)
                           └── [DNP] 0 Ω JP_VB5 ── [DNP] PTC 0.5 A ── USB-C VBUS (5 V) (option b2: bypass from USB-C)
                           (as fitted by default: no other copper to 3.3 V, USB-C VBUS, hub PRTPWR, ESP32 or any rail)
radio USB-A pin 4 (GND)  ── device ground (or the ADuM4160 side-2 ground, §8.6)
radio USB-A D+/D−        ── TPD2E2U06 ── [ADuM4160] ── S1
```

- **Device → radio:** as fitted by default, no conductor exists from any
  device rail to the radio's VBUS pin, in any mode, so no current can flow. This is
  simpler and stronger than a reverse-blocking FET or ideal diode, which
  would still have leakage and a failure mode (a shorted FET).
- **Radio → device (back-feed):** a radio that outputs voltage on its VBUS pin
  sees only the ESD diode (reverse-biased up to its stand-off) and the test
  pad. Nothing reaches a device rail.
- **Wired mode:** the computer's VBUS reaches only the USB-C input, the hub's
  VBUS_DET divider and the ESP32 sense. The hub's downstream port power pins
  are unconnected (§8.2), so the computer's VBUS can't reach the radio through
  the hub either.
- **Design-review rule:** the `RADIO_VBUS` net may contain only the connector
  pin, its ESD diode, a test pad and the three **DNP** test footprints above,
  each marked DNP in the BOM and "TEST ONLY" on the silkscreen. The KiCad
  review for #21 should check it.

**Test options (DNP, for the bench test only).** They let a bench test give a
radio a VBUS signal, or real VBUS, without a respin. The default stays "no
power to the radio" (REQ-RIF-011).

| Option | Source | What reaches the radio's VBUS | CP2105 (VBUS detect ≥ 2.5 V; regulator input 3.0–5.25 V) | CH342 (VBUS high ≥ 1.7 V; sinks up to 200 µA below 1.3 V) | Use |
|---|---|---|---|---|---|
| a. Sense-only | 3.3 V → BAT54 → 4.7 kΩ | ≈ 3.0 V at µA; 0.7 mA into a short | Passes if VBUS goes straight to the pin; fails behind the datasheet's 24 kΩ/47 kΩ divider (≈ 1.9 V) | Passes (needs ≤ 8.5 kΩ) | Tests whether a sense level is enough |
| b1. Bypass from 3.3 V | 3.3 V rail through a 0.5 A PTC | 3.3 V with current | Passes without the divider; behind the divider 3.3 × 47/71 ≈ 2.2 V, **fails**; a bus-powered CP2105 would run (3.0 V minimum regulator input) | Passes | Always available; below the USB VBUS range (4.40–5.25 V is general USB knowledge, not sourced here), so radios checking for 5 V may reject it |
| b2. Bypass from USB-C VBUS | USB-C VBUS (5 V) through a 0.5 A PTC | 5 V, only while something is plugged into USB-C | Passes, also behind the divider (≈ 3.3 V) | Passes | **Proposed for the bench test**: power the device from a USB-C charger, so the radio sees a real 5 V. It passes the computer's VBUS to the radio in wired mode, so it is test-only |

Thresholds: [CP2105 datasheet](../references/index.md#silabs-cp2105-ds)
Table 5 (VBUS detection 2.5 V min; regulator input 3.0–5.25 V) and §10;
[CH342 datasheet](../references/index.md#wch-ch342-ds) (VIHVBS 1.7 V min,
IPDN 50–200 µA below 1.3 V). **Bench test** (human-task): each USB radio with
none fitted, with (a), with (b1) and with (b2); record enumeration and the
current drawn. Never fit two options at once.

#### Consequence: radios may not enumerate without VBUS

USB device chips use VBUS to decide when to attach (turn on their D+
pull-up). From the datasheets:

- **CP2105:** "VBUS Sense Input. This pin should be connected to the VBUS
  signal of a USB network." VBUS detection threshold 2.5 V; in self-powered
  designs a 24 kΩ/47 kΩ divider on VBUS is shown
  ([CP2105 datasheet Rev. 1.4](../references/index.md#silabs-cp2105-ds), §10,
  Fig. 10). The CP2102N datasheet likewise defines a VBUS input-high level
  (VIO − 0.6 V) "to detect when the device is connected to a bus"
  ([CP2102N datasheet](../references/index.md#silabs-cp2102n-ds)).
- **CH342:** "VBUS should be connected to USB power supply, and when the loss
  of USB power is detected, CH342 will turn off the USB and sleep"; the
  CH342K/J packages have no VBUS pin and assume power is present. VBUS input
  high ≥ 1.7 V; the pin sinks 6–16 µA above 1.6 V and **50–200 µA below
  1.3 V** ([CH342 datasheet V1E](../references/index.md#wch-ch342-ds)).
- **Hubs** use upstream VBUS the same way (the USB2422's VBUS_DET decides when
  to assert its D+ pull-up). A radio with an internal hub is expected to wait
  for VBUS too (inferred).
- A **bus-powered** chip (its supply taken from VBUS) can't run at all.

| Radio | USB chip (#5) | Expected with no VBUS | With a sense-only feed (option below) |
|---|---|---|---|
| FT-891 | Dual CP210x (CP2105 inferred) | **Likely no enumeration** if the radio wires CP2105 VBUS to the connector as the datasheet directs. The FT-891 has **no other CAT path** | Works only if the radio powers the chip itself and has no VBUS divider (`unknown`) |
| FT-710, FT-991A | Dual CP210x (CP2105 inferred) + sound card | Likely no enumeration (serial and audio) | As FT-891 |
| X6100 | CH342 (secondary) | No enumeration if a VBUS-pin package (CH342F) is used; attaches if CH342K/J `unknown`. X6100 CAT is USB-only | Works for the CH342 sense input (4.7 kΩ) if nothing else loads the VBUS line |
| IC-7300, TS-590S/SG, K3S, K4 | `unknown` | `unknown` (expected to depend on VBUS as above) | `unknown` |
| IC-705 (low priority) | `unknown`; charges its battery from VBUS | `unknown` | Its charger would load the feed, so sense likely fails |
| FT-817/818, K3, KX2, KX3 | No USB port | Not affected | Not affected |

**Bench test needed** (human-task, extends #5 "Needs measurement" item 2):
connect each radio to a host with VBUS removed, and with VBUS through a
10 kΩ series resistor, and record whether it enumerates.

**How the device's own USB side behaves with no downstream VBUS:**

- **ESP32-S3 host (esp-usb):** the USB Host Library has no VBUS control. Its
  "root port power" is a controller state
  (`usb_host_lib_set_root_port_power()`: powering on "will allow device
  connections to occur"; [usb_host.h](../references/index.md#esp-usb-host-h)).
  A device is detected by its D+/D− pull-up. A radio chip that waits for VBUS
  never attaches, so the host sees no connection event; nothing fails on the
  ESP32 side.
- **USB2422:** reports its downstream ports as powered (PRTPWR on, OCS
  pulled up) and sees no attach for the same reason.

#### Option (a) in detail: sense-only VBUS feed (DNP, not adopted)

The board has no 5 V rail, and the USB-C VBUS must not feed a sense line in normal use, so option (a) takes 3.3 V:

```text
3.3 V (device) ── BAT54-class Schottky (reverse block) ── R_sense ──► radio VBUS pin   (DNP fitting option)
```

- **Current:** 3.3 V / R_sense into a short (0.7 mA at 4.7 kΩ); tens to
  hundreds of µA into a sense input. No usable power; a bus-powered chip
  still won't run.
- **Level reaching the pin:** at most about 3.0 V (3.3 V less the Schottky).
  - **CH342** (X6100): VBUS input high ≥ 1.7 V, but the pin sinks up to
    200 µA while it is below 1.3 V. To climb past 1.3 V against 200 µA,
    R_sense ≤ (3.0 − 1.3) V / 200 µA ≈ 8.5 kΩ: **4.7 kΩ works** (datasheet
    limits).
  - **CP2105** wired straight to the connector: threshold 2.5 V, so about
    3.0 V passes with a small margin. **With the datasheet's self-powered
    24 kΩ/47 kΩ divider** inside the radio, the pin sees about
    3.0 V × 47/(4.7 + 24 + 47) ≈ 1.9 V: **fails.** How each radio wires it is
    `unknown`.
  - **CP2102N**: VBUS high is VIO − 0.6 V (2.7 V at VIO = 3.3 V): marginal.
- **Doesn't work** where the radio's VBUS line also feeds a charger (IC-705)
  or a bus-powered hub or codec: the feed then sits near 0 V.
- **Back-feed:** the Schottky blocks a radio that drives its VBUS pin; the
  resistor limits the leakage path.
- **Conflict with the decision:** it connects a device rail to the radio's
  VBUS, through a resistor. Whether that counts as "supplying power" is for
  the maintainer. It is placed as a DNP footprint only. The USB-C VBUS (5 V, wired mode
  only) must not be used for it: that would pass the computer's VBUS to the
  radio.

### 8.6 Radio USB isolator option (ADuM4160)

**What it's for.** It breaks the ground path through the radio's USB cable.
Without it, the radio USB port's ground is the device ground, and in wired
mode the computer's ground. With the AUDIO or SERIAL cable also connected,
that path:

- carries **hum and ground-loop currents** between the computer, device and
  radio (heard as hum or buzz in RX and TX audio);
- carries **alternator and ignition noise** from a vehicle supply (variant M);
- carries **RF common-mode current** from the transmitter along the USB cable
  into the device and the computer;
- **bypasses the jack isolation** (§6.3): the jacks' transformers, PhotoMOS
  and digital isolators no longer isolate anything.

**Powering it without a 5 V rail.** Each side has a VBUSx pin (an internal
regulator for 4.5–5.5 V) and a VDDx pin (3.1–3.6 V). Where a side runs from
3.3 V, VBUSx connects to VDDx and to the 3.3 V supply, with a bypass
capacitor ([ADuM4160 datasheet](../references/index.md#adi-adum4160-ds),
Rev. D, pin descriptions and "Power Supply Options"). So:

- **Side 1** (hub/switch side): the device's 3.3 V rail.
- **Side 2** (radio side): an **isolated 3.3 V**, referenced to the radio USB
  ground. It must not connect to the radio's VBUS pin. The cheapest source is
  a second small transformer driven in parallel by the same 2.304 MHz
  push-pull switches as the jack supply (§6.2), with its own rectifier and
  LDO; its output ground is the radio USB ground, not the jack ground.
- **Current:** ≤ 8 mA per side at 12 Mbit/s (≤ 2.3 mA idle), so the side-2
  supply is tiny.

**Cost (LCSC, 2026-09-24):** ADuM4160BRWZ-RL $7.40 at 100+
([C57791](https://www.lcsc.com/product-detail/C57791.html), 5,267 in stock;
the tube part `ADUM4160BRWZ` is $9.08 at 100+ with 124 in stock,
[C579406](https://www.lcsc.com/product-detail/C579406.html)), plus the
side-2 supply: a transformer (**verify**; not found at LCSC), a dual Schottky
and an LDO (tens of cents). About **$8–9.50 per board** with the isolator
fitted.

- With the isolator fitted, the radio port runs at full speed only, even
  through a high-speed hub inside the radio. Speed is set by its pins (SPU,
  SPD).
- **Fitting:** a variant M option (REQ-ISO-003); recommended wherever the
  radio's USB cable and a jack cable are used together.
- VBUS options (a) and (b) in §8.5 connect to the radio side of the port, so
  with the isolator fitted they would have to come from the isolated side-2
  supply, not the device rails **(verify at schematic time)**.

### 8.7 Signal integrity on a 2-layer board

- **Full speed (12 Mbit/s):** the radio port, the ESP32 link and the switch
  paths. No controlled impedance needed
  ([`pcb-fabrication.md`](../requirements/pcb-fabrication.md) §2); route D+/D−
  as a pair, keep them short and matched, over unbroken ground, with few vias
  and 22–33 Ω series resistors at the ESP32 ([`fcc.md`](../compliance/fcc.md)
  §1.5).
- **High speed (480 Mbit/s):** the USB-C ↔ hub upstream link, and hub port 1
  ↔ S1 ↔ radio when the radio has a high-speed hub inside. A 90 Ω pair isn't
  practical on 1.6 mm 2-layer, so: put the hub next to the USB-C connector,
  put S1 next to the USB-A connector, and keep both runs very short.
- **Fallback without a layout change:** set the USB2422's **HS_DISABLE**
  (over SMBus from the ESP32) so the hub attaches at full speed only. Every
  on-board link is then full speed. Total bandwidth is then 12 Mbit/s shared,
  which still covers the radio's serial and audio (48 kHz / 16-bit stereo in
  both directions is about 3 Mbit/s,
  [`core-devices.md`](core-devices.md#4-usb-host-the-radios-usb-sound-card-and-usb-serial-chip-9)).
  If high speed is kept and fails eye or enumeration tests, go to 4 layers
  (pcb-fabrication §2).

## 9. USB-host class and driver support (esp-usb)

Source: [`espressif/esp-usb`](https://github.com/espressif/esp-usb) at commit
`bf0f0aa36227cc60ea9d241d14ef62944af5fb94` (2026-09-24), Apache-2.0. Nothing is
copied; the facts below are read from these files.

| Need | Component (version) | What it supports | RTS/DTR | Status |
|---|---|---|---|---|
| CP210x incl. **CP2105** | `usb_host_cp210x_vcp` 2.2.0 | VID 0x10C4; PIDs 0xEA60 (CP2101–CP2104), **0xEA70 (CP2105, dual)**, 0xEA71 (CP2108); an `interface_idx` selects the port ([vcp_cp210x.h](../references/index.md#esp-usb-cp210x-h)) | `SET_MHS` per interface | **Supported.** Both CP2105 ports can be open at once: the CDC-ACM layer allocates a new CDC device on an already-open USB device ([cdc_acm_host.c](../references/index.md#esp-usb-cdc-acm-host-c)). Radio VID/PID `unknown` (#5); if a radio uses its own PID, pass it explicitly. Bench test in #43 |
| FTDI | `usb_host_ftdi_vcp` 2.1.1 | VID 0x0403; PIDs 0x6001 (FT232), 0x6015 (FT231X) only ([usb_host_ftdi_vcp.c](../references/index.md#esp-usb-ftdi-c)) | Request 0x02 (see §5) | **Partial:** multi-port FT2232/FT4232 not listed; RTS/DTR request **(verify)**. No #5 radio is confirmed to use FTDI |
| CH34x | `usb_host_ch34x_vcp` 2.2.1 | CH340 (0x7522, 0x7523), CH341 (0x5523); "Limited implementation only" ([usb_host_ch34x_vcp.c](../references/index.md#esp-usb-ch34x-c)) | MODEM_OUT 0xA4 | Supported for CH340/CH341; **CH342 not listed** |
| **CH342** (X6100) | `usb_host_cdc_acm` 2.4.1 | The CH342 "supports free installation OS which built-in CDC driver" ([CH342 datasheet](../references/index.md#wch-ch342-ds)), so the generic CDC-ACM driver should bind to each port by interface index | `SET_CONTROL_LINE_STATE` | **(verify)** on hardware; also blocked by §8.5 if it waits for VBUS |
| CDC-ACM | `usb_host_cdc_acm` 2.4.1 | Class devices, including IAD "triple null" devices | `SET_CONTROL_LINE_STATE` (0x22) | Supported |
| USB Audio Class 1.0 | `usb_host_uac` 1.5.0 | "Any UAC 1.0 compatible device"; mic and speaker handled as two logical devices ([README](../references/index.md#esp-usb-uac-readme)) | — | Supported (UAC version per radio `unknown`, #5) |
| Hub inside the radio | USB Host Library `usb` 1.5.0 | `USB_HOST_HUBS_SUPPORTED` (default off), multi-level hubs default on ([Kconfig](../references/index.md#esp-usb-host-kconfig)) | — | Enable in firmware (#43) |

**CP2105 conclusion (verify-first item for #43):** supported by the driver,
subject to the radio's VID/PID and the VBUS consequence in §8.5.

## 10. Connector and cable strategy

One board, per-radio cables. The jack contacts are fixed by
[`radio-connectors.md`](../requirements/radio-connectors.md); the radio-side
pins come from [`radio-interfaces.md`](radio-interfaces.md) (#5), which cites
the manuals. AUDIO: tip = RX audio in, ring 1 = TX audio out, ring 2 = PTT,
sleeve = ground. SERIAL: tip = data to the radio (or CI-V), ring 1 = data from
the radio, ring 2 = off unless "logic + power", sleeve = ground.

| Radio | AUDIO cable (radio connector: tip / ring 1 / ring 2 / sleeve) | SERIAL cable and mode | Radio USB-A | Priority / notes |
|---|---|---|---|---|
| FT-891 | RTTY/DATA 6-pin mini-DIN: DATA OUT (5) / DATA IN (1) / PTT (3) / GND (2); pins other than 3 **(verify)** | None | **CAT over USB (CP2105)** | CAT depends on §8.5 |
| FT-710 | RTTY/DATA: 5 / 1 / 3 / 2 | TUNER/LINEAR 8-pin: RXD (5) ← tip, TXD (4) → ring 1, GND (3); **3.3 V logic** (5 V TTL, §3.5); +13 V (1) to the power input | CAT + audio over USB | Serial is optional |
| FT-991A | RTTY/DATA: 5 / 1 / 3 / 2 | GPS/CAT DE-9: SERIAL IN (3) ← tip, SERIAL OUT (2) → ring 1, GND (5); **RS-232** | CAT + audio over USB | |
| FT-817ND / FT-818 | DATA 6-pin: DATA OUT / DATA IN / PTT / GND (2) | ACC 8-pin: RX D ← tip, TX D → ring 1, GND; **3.3 V logic** (level `unknown`); +13.8 V to power | None | |
| IC-7300 | ACC 13-pin: AF (12) / MOD (11) / SEND (3) / GND (2); 13.8 V (8) to power | [REMOTE] 3.5 mm: **CI-V** on tip, ground on sleeve (figure only, **verify**) | CI-V + audio over USB | |
| IC-705 | [SP] L → tip (attenuator on); [MIC] ← ring 1; [SEND/ALC] SEND → ring 2 | None | CI-V + audio over USB; **no charging** | **Low priority** |
| TS-590S / SG | ACC2 13-pin: ANO (3) / ANI (11) / PKS (9) / GND (4) | COM DE-9: TXD (3) ← tip, RXD (2) → ring 1, GND (5); **RS-232** | USB audio + COM | EXT.AT 14S to power |
| K3 / K3S | LINE OUT L / LINE IN / PTT IN (RCA) / GND | DE-9: pin 3 ← tip, pin 2 → ring 1, pin 5; **RS-232** | K3S only | 12 VDC OUT to power |
| KX2 | PHONES → tip (attenuator on); MIC ← ring 1; MIC PTT → ring 2 | ACC 3.5 mm straight TRRS; mode `unknown` (RS-232 per a secondary source); **ring 2 must stay off** (key-out) | None | Firmware must block "logic + power" |
| KX3 | PHONES → tip (attenuator on); MIC ← ring 1; ACC2 GPIO (LO = PTT) → ring 2 | ACC1 3.5 mm straight TRS; mode `unknown` (RS-232 per a secondary source) | None | |
| K4 | LINE OUT → tip; LINE IN ← ring 1; PTT IN → ring 2 | DE-9 "true RS232"; pinout `unknown` | **USB-B "PC" port only** | USB-A host ports not used |
| X6100 | S/P → tip (attenuator on); [MIC] RJ-45 MIC ← ring 1, PTT → ring 2 | None | **CAT + audio over USB (CH342)** | CAT depends on §8.5 |

Cable rule (from #5): radio output → the jack's tip, radio input → ring 1,
PTT → AUDIO ring 2, ground → sleeve. The "(verify)" pins are those #5 found
only in figures.

## 11. Candidate parts and sourcing

LCSC, checked 2026-09-24, USD per unit. RoHS: as LCSC lists it ("RoHS3" =
RoHS 3, EU 2015/863). **LCSC showed no REACH SVHC data for any of these
parts**; it must come from the manufacturers' declarations (#59). Digi-Key
and Mouser: **deferred** by the maintainer, left blank.

| Part (order code) | Function | Temp. | LCSC # | LCSC price breaks | LCSC stock | RoHS (LCSC) | Digi-Key | Mouser | Datasheet |
|---|---|---|---|---|---|---|---|---|---|
| MAX14778ETP+T | SERIAL mode mux | −40 to +85 °C | C1121866 | 1+ 6.01; 10+ 5.26; 100+ 4.24; 1,000+ 3.94 | 2,817 | RoHS3 | deferred | deferred | [adi-max14778-ds](../references/index.md#adi-max14778-ds) |
| TMUX6219DGKR (alt.) | SPDT, ±18 V | −40 to +125 °C | C2876423 | 1+ 3.24; 10+ 2.75; 100+ 2.18; 1,000+ 1.98 | 3,110 | RoHS3 | deferred | deferred | [ti-tmux6219-ds](../references/index.md#ti-tmux6219-ds) |
| G6K-2F-Y DC3 (alt.) | DPDT signal relay | −40 to +70 °C | C93168 | 1+ 1.60; 10+ 1.36; 100+ 1.08; 900+ 0.88 | 6,889 | RoHS | deferred | deferred | [omron-g6k-ds](../references/index.md#omron-g6k-ds) |
| TRS3221EIPWR | RS-232 1+1 | −40 to +85 °C | C19901 | 1+ 0.86; 10+ 0.69; 100+ 0.52; 1,000+ 0.44 | 1,748 | RoHS | deferred | deferred | [ti-trs3221e-ds](../references/index.md#ti-trs3221e-ds) |
| SN74LVC1G11DBVR | 3-input AND (RX merge; a 1G08 2-input AND is the PTT gate) | −40 to +125 °C | C22046 | 5+ 0.22; 50+ 0.18; 150+ 0.16; 500+ 0.14 | 14,740 | RoHS3 | deferred | deferred | [ti-sn74lvc1g11-ds](../references/index.md#ti-sn74lvc1g11-ds) |
| SN74LVC1G08DBVR | 2-input AND (PTT gate) | −40 to +125 °C | C7666 | 10+ 0.050; 100+ 0.040; 300+ 0.035; 3,000+ 0.030 | 142,110 | RoHS3 | deferred | deferred | [ti-sn74lvc1g08-ds](../references/index.md#ti-sn74lvc1g08-ds) |
| ISO7721DR | Serial data isolator | −55 to +125 °C | C366164 | 1+ 0.75; 10+ 0.65; 100+ 0.52; 1,000+ 0.49 | 18,825 | RoHS3 | deferred | deferred | [ti-iso7721-ds](../references/index.md#ti-iso7721-ds) |
| ISO1540DR | I²C isolator | −40 to +125 °C | C179739 | 1+ 0.95; 10+ 0.80; 100+ 0.59; 1,000+ 0.54 | 19,646 | RoHS3 | deferred | deferred | [ti-iso1540-ds](../references/index.md#ti-iso1540-ds) |
| TCA9534PWR | Jack-domain I/O expander | −40 to +85 °C | C783615 | 1+ 0.72; 10+ 0.59; 100+ 0.45; 1,000+ 0.35 | 16,219 | RoHS3 | deferred | deferred | [ti-tca9534-ds](../references/index.md#ti-tca9534-ds) |
| SN74LVC2G02DCUR | Isolated-supply drive (NOR + enable) | −40 to +125 °C | C133589 | 5+ 0.30 … 6,000+ 0.17 | 1,850 | RoHS3 | deferred | deferred | [ti-sn74lvc2g02-ds](../references/index.md#ti-sn74lvc2g02-ds) |
| AO3400A (×2) | Isolated-supply switches | −55 to +150 °C | C20917 | 5+ 0.085; 50+ 0.068; 150+ 0.060; 500+ 0.052; 3,000+ 0.050 | 693,890 | RoHS3 | deferred | deferred | LCSC listing |
| Würth 760390014 (or equal) | 1:1.3 push-pull transformer | (verify) | not found | — | — | (verify) | deferred | deferred | listed in the SN6505B datasheet |
| SN6505BDBVR (fallback) | Isolated supply driver | −55 to +125 °C | C74518 | 1+ 0.62; 10+ 0.49; 100+ 0.37; 1,000+ 0.32 | 10,616 | RoHS3 | deferred | deferred | [ti-sn6505b-ds](../references/index.md#ti-sn6505b-ds) |
| SN6507DGQR (rejected) | Push-pull driver, 2 MHz | −55 to +125 °C | C5122398 | 1+ 5.30 … 1,000+ 3.78 | 149 | (not checked) | deferred | deferred | [ti-sn6507-ds](../references/index.md#ti-sn6507-ds) |
| AQY212EHAX | PTT closure and ring-2 switch (×2) | −40 to +85 °C | C29276 | 1+ 1.64; 10+ 1.33; 30+ 1.17; 100+ 0.98; 500+ 0.89; 1,000+ 0.85 | 2,706 | RoHS3 | deferred | deferred | [panasonic-aqy21eh-ds](../references/index.md#panasonic-aqy21eh-ds) |
| BC857BS,115 | Ring-2 current limiter | −65 to +150 °C | C8654 | 10+ 0.072; 100+ 0.056; 300+ 0.048; 3,000+ 0.043 | 930 | RoHS3 | deferred | deferred | [nexperia-bc857bs-ds](../references/index.md#nexperia-bc857bs-ds) |
| BAT54S,215 | Ring-2 reverse block | (LCSC listing) | C47546 | 20+ 0.031 … 21,000+ 0.016 | 283,320 | RoHS3 | deferred | deferred | LCSC listing |
| AQY212EH (DIP) | same, through-hole | −40 to +85 °C | C2894775 | 1+ 1.25; 10+ 1.08; 100+ 0.87; 1,000+ 0.79 | 192 | (not checked) | deferred | deferred | [panasonic-aqy21eh-ds](../references/index.md#panasonic-aqy21eh-ds) |
| CPC1017NTR | PTT alternate | −40 to +85 °C | C81521 | 1+ 1.99; 10+ 1.71; 100+ 1.39; 1,000+ 1.12 | 23,818 | RoHS3 | deferred | deferred | [littelfuse-cpc1017n-ds](../references/index.md#littelfuse-cpc1017n-ds) |
| TPS3839G33DBZR | Supervisor (3.08 V, push-pull) | −40 to +85 °C | C485802 | 1+ 0.44; 10+ 0.35; 30+ 0.31; 100+ 0.26; 500+ 0.24; 1,000+ 0.23 | 3,431 | RoHS3 | deferred | deferred | [ti-tps3839-ds](../references/index.md#ti-tps3839-ds) |
| PESD24VL1BA | Jack TVS | −65 to +150 °C | C69324 | 5+ 0.24; 50+ 0.19; 150+ 0.16; 500+ 0.13 | 32,235 | RoHS3 | deferred | deferred | [nexperia-pesd24vl1ba-ds](../references/index.md#nexperia-pesd24vl1ba-ds) |
| USB2422T-I/MJ | USB hub | −40 to +85 °C | C622610 | 1+ 2.31; 10+ 1.95; 100+ 1.48; 1,000+ 1.33 | 403 | RoHS3 | deferred | deferred | [microchip-usb2422-ds](../references/index.md#microchip-usb2422-ds) |
| CH334F (alt.) | USB hub | −40 to +85 °C | C5187527 | 1+ 0.63; 10+ 0.54; 100+ 0.42; 1,000+ 0.39 | 4,340 | not shown (verify) | deferred | deferred | not reviewed |
| TS3USB221ARSER | USB switch (×2) | −40 to +85 °C | C128396 | 5+ 0.26; 50+ 0.21; 150+ 0.19; 500+ 0.16; 3,000+ 0.14 | 91,820 | RoHS3 | deferred | deferred | [ti-ts3usb221a-ds](../references/index.md#ti-ts3usb221a-ds) |
| FSUSB42MUX (alt.) | USB switch | −40 to +85 °C | C11355 | 5+ 0.32; 50+ 0.25; 150+ 0.21; 500+ 0.18 | 43,120 | RoHS3 | deferred | deferred | LCSC listing only |
| TYPE-C-31-M-12 | USB-C receptacle | −30 to +80 °C | C165948 | 5+ 0.17; 50+ 0.14; 150+ 0.12; 1,000+ 0.10 | 79,985 | not shown (verify) | deferred | deferred | LCSC listing |
| USBLC6-2SC6 | USB-C ESD | −40 to +125 °C | C7519 | 5+ 0.18; 50+ 0.14; 150+ 0.12; 500+ 0.10 | 27,505 | RoHS3 | deferred | deferred | LCSC listing |
| ADUM4160BRWZ-RL | Radio USB isolator (option) | up to +105 °C | C57791 | 1+ 9.87; 10+ 8.69; 30+ 8.13; 100+ 7.40 | 5,267 | RoHS3 | deferred | deferred | [adi-adum4160-ds](../references/index.md#adi-adum4160-ds) |
| CH342F | (reference: X6100's chip) | −40 to +85 °C | C2841530 | 1+ 2.31; 100+ 1.24; 1,000+ 1.11 | 4,314 | (not checked) | deferred | deferred | [wch-ch342-ds](../references/index.md#wch-ch342-ds) |

SN74LXC1T45, SN74LVC1G07 and TPD2E2U06 were priced in
[`core-devices.md`](core-devices.md) the same day; their datasheets are in the
index. Single-source risks: MAX14778 (ADI only), USB2422 (Microchip only, 403
in stock). Each has an alternate topology listed above.

## 12. Open questions and measurements

1. **VBUS decision vs USB-only radios** (§8.5): without VBUS, the FT-891 and
   X6100 may have no CAT path. Bench-test enumeration with none fitted and
   with each DNP option (a, b1, b2); the maintainer decides afterwards.
2. **Watchdog timeouts and coverage** (§4.2.1): #15 sets the task and
   interrupt watchdog timeouts (seconds) and subscribes the PTT task.
3. **Variant R isolation in wired mode** (§6.1): same open question as
   ADR-0002.
4. **Ground path through the radio USB cable** (§6.3): the ADuM4160 option
   (§8.6) fixes it on boards where it's fitted; the grounding plan and #12
   decide where.
5. **MAX14778 unpowered behavior and absolute maximum ratings** (§3.2): read the
   ADI datasheet (manual download) and bench-test ±15 V on each contact,
   powered and unpowered. Fallback: relays (variant R only) or TMUX6219 with a
   ±16 V supply.
6. **CI-V pull-up voltage and thresholds** (§3.6): measure on the IC-7300.
7. **FT-710 CAT-3 input high threshold** with a 3.3 V driver (§3.5).
8. **PTT open-circuit voltage and closed current** on every radio (#5 item 4),
   and a TVS rating above 24 V if any radio needs it (§7).
9. **FTDI RTS/DTR request** in esp-usb (§9): check against FTDI documentation
   or on hardware; report upstream if wrong.
10. **Slow-ramp brownout test** of the PTT drive (§4.2.2).
11. **USB-C receptacle for variant M** (−40 to +85 °C) (§8.4).
12. **High speed on 2 layers** (§8.7): keep HS with short runs, or set
    HS_DISABLE.
13. **2.304 MHz push-pull stage** (§6.2): transformer core loss and ringing at
    2.3 MHz, dead time, and fault current; SN6505B at 576 kHz is the fallback
    if it fails, with lines in 15 m and 10 m to measure.
14. **I2S BCLK at 64 fs** (3.072 MHz) instead of 32 fs, which puts a line in
    10 m (§7.1); a codec register setting for #8/#16.
15. **VHF/UHF lines** (§7.1): 144.000 MHz from the USB2422 crystal and USB
    full speed, 440.000 MHz from the ESP32-S3 crystal, and the 2.304 MHz comb
    in 6 m, 2 m and 70 cm. Measure at bring-up; mixing products not covered.
16. **EU immunity levels** (#59): ESD, EFT/burst, surge and conducted RF levels
    per port for EN 301 489-1/-17, to size the jack TVS and filters (§7).
17. **REACH SVHC declarations** for every part (LCSC shows none), and RoHS
    for the CH334F and TYPE-C-31-M-12, which LCSC doesn't state (§11).
18. **3.3 V rail tolerance vs the TPS3839G33** (3.126 V maximum threshold,
    §4.2): the regulator (#10/#11) must stay above it.

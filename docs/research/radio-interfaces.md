<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Radio power and interface table (revision A)

Issue: [#5](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/5).
Researched 2026-09-24 from the manufacturers' manuals. It feeds the interface
circuits (#9), the variant R power front end (#11) and the variants decision
(#12).

Scope: HF and multiband transceivers only, the default for revision A (open
question in #5). The design context is
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md): Bluetooth LE and
wired USB-C host links, one radio side with a radio USB port (the device is USB
host) and the AUDIO and SERIAL TRRS jacks of
[`radio-connectors.md`](../requirements/radio-connectors.md).

## How to read this

- **Every fact cites a manual page**, as `[key] p. N` (the printed page number,
  or section-page for Icom). The keys link to the
  [reference index](../references/index.md) and are listed under
  [Sources](#sources).
- **`unknown`** means no primary source was found. Each one is listed under
  [Needs measurement](#needs-measurement) for a later `human-task`.
- **(secondary)** marks a fact from a non-manufacturer source (a distributor or
  an interface maker). **(inferred)** marks a conclusion drawn from a cited
  fact rather than stated by it.
- "In" and "out" in the radio tables are from the **radio's** point of view.
  The cable mappings use the jack contacts of
  [`radio-connectors.md`](../requirements/radio-connectors.md), whose
  directions are from the interface's point of view.
- **A radio's USB port is a USB device port. It supplies no power**: the
  interface, as USB host, must supply VBUS. None of the manuals states how much
  VBUS current the radio draws, and one radio (IC-705) charges its battery from
  that port.

## Summary

### Can the radio power the interface?

The interface's budget is about 1.5 W typical and 3 W peak
([constraints §3.4](../requirements/constraints.md#34-power-budget-verify)),
which is roughly 0.22 A at 13.8 V. "Switched" means the output is on only while
the radio is on, so it can double as a "radio on" sense for auto power-up.

| Radio | DC output (connector, pin) | Voltage | Documented current limit | Switched with radio? | Verdict |
|---|---|---|---|---|---|
| Yaesu FT-891 | TUN/LIN 8-pin mini-DIN, "+13V OUT" ([891-OM] p. 24) | 13 V nominal | `unknown` | `unknown` | Candidate once measured |
| Yaesu FT-710 | TUNER/LINEAR 8-pin mini-DIN, pin 1 "+13V" ([710-CAT] p. 3) | 13 V | `unknown` | **Yes**: "linked to radio ON" ([710-CAT] p. 3) | Candidate once measured |
| Yaesu FT-991A | TUN/LIN 8-pin mini-DIN, "+13V OUT" ([991A-OM] p. 27) | 13 V nominal | `unknown` | `unknown` | Candidate once measured |
| Yaesu FT-817ND / FT-818 | ACC 8-pin mini-DIN, "+13.8V" ([818-OM] p. 15) | Not stated; the radio runs on 8–16 V or its battery ([818-OM] p. 70) | `unknown` | `unknown` | Candidate once measured; low voltage on battery power |
| Icom IC-7300 | ACC 13-pin DIN, pin 8 "13.8 V" ([7300-FM] p. 18-2) | 13.8 V | **1 A max** ([7300-FM] p. 18-2) | **Yes**: "13.8 V output when power is ON" | **Yes**, within budget |
| Icom IC-705 | None. [MIC] jack: 8 V, 10 mA max, or 3.3 V through 470 Ω ([705-BM] p. 13-3) | — | — | — | **No.** Its micro-USB port *sinks* power (below) |
| Kenwood TS-590SG | EXT.AT 6-pin, pin 6 "14S" ([590SG-IM] p. 72) | 13.8 V | **4 A max** ([590SG-IM] p. 72) | **Yes**: "Switched 13.8 V" | **Yes**, within budget |
| Kenwood TS-590S | EXT.AT 6-pin, pin 6 "14S" ([590S-IM] p. 66) | 13.8 V | `unknown` (not stated) | **Yes**: "Switched 13.8V" | Candidate once the limit is confirmed |
| Elecraft K3 | 12 VDC OUT, RCA ([K3-OM] p. 17) | 13 V no load, 12 V at max load ([K3-OM] p. 8) | **0.5 A max** ([K3-OM] p. 8) | **Yes** | **Yes**, within budget |
| Elecraft K3S | 12 VDC OUT, RCA ([K3S-OM] p. 17) | 13 V no load, 12 V at max load ([K3S-OM] p. 8) | **1.0 A max** ([K3S-OM] p. 8) | **Yes** | **Yes**, within budget |
| Elecraft KX2 | None ([KX2-OM] p. 69) | — | — | — | **No** |
| Elecraft KX3 | None ([KX3-OM] p. 54) | — | — | — | **No** |
| Elecraft K4 | 12 VDC OUT ([K4-OM] p. 11); also rear USB-A host ports ([K4-OM] p. 13) | 12 V; USB-A: 5 V (inferred) | **1.5 A max**, self-resetting fuse; USB-A: 400 mA each | **Yes** (12 VDC OUT) | **Yes**, within budget |
| Xiegu X6100 | None on ACC ([X6100-UM] p. 8). [MIC] RJ-45 has "+8V" | 8 V | `unknown` | `unknown` | **No** (8 V pin current unknown) |
| Generic | Yaesu 8-pin ACC "+13.8V"; Icom 8-pin ACC-1/ACC-2 pin 7 "13.8 V" ([7300-FM] p. 18-3) | 13.8 V | Varies (IC-7300: 1 A) | Varies | Per radio |

### Which radios need the USB-host path, level conversion, or USB audio?

"USB-host path needed" means the radio's USB port is the only wired CAT (and
audio) path, so the ESP32-S3 must be USB host to it in Bluetooth mode. Radios
marked "optional" also have a serial jack.

| Radio | USB port (connector) | USB-host path needed? | Serial CAT on a jack | Level conversion | Audio on USB? |
|---|---|---|---|---|---|
| FT-891 | Device, USB-B ([891-OM] p. 25; A-B cable, [Y-VCP] p. 1) | **Yes** (no serial CAT jack) | None | None | **No**: USB serial only ([Y-VCP] p. 1) |
| FT-710 | Device, USB-B ([710-OM] p. 56). A separate USB-A jack is a host port for a keyboard or mouse ([710-OM] p. 14) | Optional | TUNER/LINEAR pins 4–5, **5 V TTL** ("CAT-3", [710-CAT] p. 3) | 3.3 V logic mode; 5 V TTL input threshold `unknown` | **Yes** ([710-OM] p. 14) |
| FT-991A | Device, USB-B ([991A-OM] p. 28; [Y-VCP] p. 1) | Optional | GPS/CAT DE-9, **RS-232** ([991A-OM] p. 27) | RS-232 | **Yes** ([991A-OM] p. 28) |
| FT-817ND / FT-818 | None | No | ACC 8-pin mini-DIN, TX D / RX D ([818-OM] p. 15) | Logic level (the CT-62 cable holds an RS-232 level converter, [818-OM] p. 64); voltage `unknown` | No (analog DATA jack) |
| IC-7300 | Device, USB-B ([7300-FM] p. 2-5) | Optional | [REMOTE] 3.5 mm **CI-V** ([7300-FM] p. 18-4) | CI-V | **Yes** ([7300-FM] p. 12-7) |
| IC-705 | Device, micro-USB type B, USB 1.1/2.0 ([705-BM] p. 13-3) | **Yes** (no CI-V jack) | None | None | **Yes** ([705-BM] p. 8-13) |
| TS-590SG | Device, USB-B ([590SG-IM] p. 62, 73) | Optional | COM DE-9, **RS-232** ([590SG-IM] p. 71, 73) | RS-232 | **Yes** ([590SG-IM] p. 73; [590SG-USB] p. 4) |
| TS-590S | Device, USB ([590S-IM] p. 18) | Optional | COM DE-9, RS-232 ([590S-IM] p. 18) | RS-232 | **Yes** (USB audio menus, [590S-IM] p. 18–19) |
| K3 (KIO3) | None ("USB adapter option", [K3-OM] p. 8) | No | DE-9 female, **RS-232**, up to 38400 baud ([K3-OM] p. 18) | RS-232 | No (analog LINE IN/OUT) |
| K3S (and K3 with KIO3B) | Device, USB-B ([K3S-OM] p. 8) | Optional | RJ-45 with RJ-45-to-DE-9 adapter, **RS-232** ([K3S-OM] p. 19–20) | RS-232 | **Yes** ([K3S-OM] p. 18) |
| KX2 | None (KXUSB adapter cable on ACC, [KX2-OM] p. 9) | No | ACC 3.5 mm TRRS ([KX2-OM] p. 9) | Level at the jack `unknown`; "RS232 or USB port depending on selected adapter" ([KX2-OM] p. 69). Digirig configures RS-232 for it (secondary, [DR-KX]) | No (analog MIC/PHONES) |
| KX3 | None (KXUSB adapter cable on ACC1, [KX3-OM] p. 5) | No | ACC1 3.5 mm TRS ([KX3-OM] p. 5) | Level at the jack `unknown`; KXSER for RS-232 ports ([KX3-OM] p. 5). Digirig configures RS-232 (secondary, [DR-KX]) | No (analog MIC/PHONES) |
| K4 | Device, USB-B "PC" ([K4-OM] p. 13) | Optional | DE-9, "true RS232" ([K4-OM] p. 16) | RS-232 | **Yes** ([K4-OM] p. 13) |
| X6100 | Device, USB-C "DEV" ([X6100-UM] p. 5); separate USB-C "HOST" port | **Yes** (no serial jack) | None | None | **Yes** (secondary, [X6100-EXT] p. 105) |
| Generic | — | — | 3.5 mm CI-V; DE-9 RS-232; Yaesu ACC TTL | CI-V, RS-232 or logic | — |

### USB details for the radio USB port

| Radio | USB-serial chip | Virtual serial ports | Built-in USB sound card (UAC version) | Internal USB hub | VBUS draw from the interface |
|---|---|---|---|---|---|
| FT-891 | Dual CP210x ("Silicon Labs Dual CP210x USB to UART Bridge", [Y-VCP] p. 6); CP2105 (inferred: the dual-UART CP210x) | 2: Enhanced (CAT, firmware) and Standard (PTT, CW, FSK) ([Y-VCP] p. 7) | **No** ([Y-VCP] p. 1) | `unknown` | `unknown` |
| FT-710 | Dual CP210x ([710-CAT] p. 1); CP2105 (inferred) | 2: Enhanced = CAT-1, Standard = CAT-2 ([710-CAT] p. 2) | Yes; standard OS driver ([Y-VCP] p. 5); UAC version `unknown` | `unknown` | `unknown` |
| FT-991A | Dual CP210x ([991A-CAT] p. 1; [Y-VCP] p. 6); CP2105 (inferred) | 2: Enhanced and Standard ([Y-VCP] p. 7) | Yes; standard OS driver ([Y-VCP] p. 5); UAC `unknown` | `unknown` | `unknown` |
| IC-7300 | `unknown` (Icom driver download, [7300-FM] p. 2-5) | 1 CI-V port with DTR/RTS for SEND/keying ([7300-FM] p. 12-9) | Yes ([7300-FM] p. 12-7); UAC `unknown` | `unknown` | `unknown` |
| IC-705 | `unknown` (Icom driver download, [705-BM] p. 13-3) | 2: "IC-705 Serial Port A (CI-V)" and "Serial Port B" ([705-BM] p. 8-14) | Yes ([705-BM] p. 8-13); UAC `unknown` | `unknown` | **Charges its battery from VBUS** by default; rapid charging with a 2 A USB port ([705-BM] p. 1-2). Draw `unknown` |
| TS-590SG | `unknown` (Kenwood driver download, [590SG-IM] p. 62) | Virtual COM "(Standard)" port for PC control ([590SG-IM] p. 73) | Yes; named "USB Audio Codec", OS drivers install automatically ([590SG-USB] p. 4, 10); UAC `unknown` | `unknown` | `unknown` |
| TS-590S | `unknown` ([590S-IM] p. 57) | `unknown` | Yes ([590S-IM] p. 18–19); UAC `unknown` | `unknown` | `unknown` |
| K3S | `unknown` | 1 COM port ([K3S-OM] p. 18) | Yes ([K3S-OM] p. 18); UAC `unknown` | `unknown` | `unknown` |
| K4 | `unknown` | 2: USB-PC1, USB-PC2 ([K4-OM] p. 13) | Yes, stereo out, mono in ([K4-OM] p. 13); UAC `unknown` | `unknown` | `unknown` |
| X6100 | **CH342** (dual) (secondary, [X6100-EXT] p. 105) | 2; SERIAL-B for CAT and data modes (secondary) | Yes (secondary, [X6100-EXT] p. 105); UAC `unknown` | `unknown` | `unknown` |

A radio that shows both a serial port and a sound card is either a composite
device or has a hub inside; the manuals don't say which. The USB host stack must
handle both
([constraints §7](../requirements/constraints.md#7-radio-interfaces)).

### Audio and PTT electrical levels

| Radio | RX audio out (max level, impedance) | TX audio in (level, impedance) | Hardware PTT input | PTT pull-up voltage / current |
|---|---|---|---|---|
| FT-891 | RTTY/DATA "DATA OUT", fixed level, adjustable 0–100 ([891-ADV] p. 86); level `unknown` | "DATA IN"; level `unknown` | RTTY/DATA pin 3 (DAKY) ([891-ADV] p. 86) | `unknown` |
| FT-710 | RTTY/DATA pin 5, "fixed level receiver audio" ([710-OM] p. 14); REAR OUT LEVEL 0–100 ([710-OM] p. 74); level `unknown` | Pin 1; REAR MOD GAIN 0–100; level `unknown` | Pin 3 DATA PTT ([710-OM] p. 56) | `unknown` |
| FT-991A | RTTY/DATA pin 5 ([991A-OM] p. 27); level `unknown` | Pin 1; level `unknown` | Pin 3 ([991A-OM] p. 27) | `unknown` |
| FT-817ND / FT-818 | DATA OUT 1200 bps: 300 mVpp max; 9600 bps: 500 mVpp max; 10 kΩ ([818-OM] p. 37) | DATA IN: 40 mVpp @1200 bps, 1.0 Vpp @9600 bps, 10 kΩ ([818-OM] p. 37) | DATA PTT, "ground to transmit" ([818-OM] p. 37) | `unknown` |
| IC-7300 | ACC pin 12: 100–300 mV rms, 4.7 kΩ; ~200 mV rms at the 50 % default ([7300-FM] p. 18-2) | ACC pin 11: 100 mV rms at 50 %, 10 kΩ ([7300-FM] p. 18-2) | ACC pin 3 SEND | RX: 2.0–20.0 V; TX: −0.5 to +0.8 V; 20 mA max ([7300-FM] p. 18-2). Open-circuit voltage `unknown` |
| IC-705 | [SP] jack: speaker amp > 0.2 W into 8 Ω, or headphone amp > 5 mW into 16 Ω ([705-BM] p. 13-3) | [MIC] 2.5 mm, microphone level ([705-BM] p. 13-3) | [SEND/ALC] "SEND (I/O)" ([705-BM] p. 13-2; recommended for T/R, [705-AM] p. 18-1) | `unknown` |
| TS-590SG / TS-590S | ACC2 pin 3 ANO: 0–1.2 Vpp, 0.5 Vpp at default, ≈10 kΩ ([590SG-IM] p. 71; [590S-IM] p. 65) | ACC2 pin 11 ANI: ≈10 mV rms at default, ≈10 kΩ ([590SG-IM] p. 71) | ACC2 pin 9 PKS (mutes mic) or pin 13 SS ([590SG-IM] p. 71) | `unknown` |
| K3 / K3S | LINE OUT, stereo, transformer-isolated, 600 Ω nominal ([K3S-OM] p. 22); level `unknown` | LINE IN, mono, transformer-isolated, 600 Ω ([K3S-OM] p. 22) | PTT IN (RCA) or ACC pin 4 ([K3S-OM] p. 17, 20) | `unknown` |
| KX2 / KX3 | PHONES jack, 0.1 W per channel ([KX3-OM] p. 54; [KX2-OM] p. 69): a hot headphone output | MIC jack ([KX3-OM] p. 18) | KX3: ACC2 GPIO set to LO=PTT ([KX3-OM] p. 35), or the MIC jack PTT line ([KX3-OM] p. 18). KX2: MIC jack ring 1 (PTT/UP/DN, [KX2-OM] p. 8) or VOX ([KX2-OM] p. 28) | KX3 GPIO: 3 V logic, 500 Ω series resistor, tolerates 0–5.5 V ([KX3-OM] p. 35) |
| K4 | LINE OUT, stereo, 600 Ω nominal ([K4-OM] p. 14); level `unknown` | LINE IN, 600 Ω nominal ([K4-OM] p. 14) | PTT IN (RCA), "pull to ground" ([K4-OM] p. 16) | `unknown` |
| X6100 | S/P jack, 0.4 W into 8 Ω ([X6100-EXT] p. 124, secondary) | [MIC] RJ-45 ([X6100-UM] p. 8) | [MIC] PTT pin ([X6100-UM] p. 8) | `unknown` |

**RX audio range:** the documented line outputs run from about 0.3 Vpp (FT-818
DATA OUT) to 1.2 Vpp (TS-590SG ANO, maximum). Speaker and headphone outputs
(IC-705 [SP], KX2/KX3 PHONES, X6100 S/P) are far hotter; they need the AUDIO
jack's switchable ~19 dB attenuator or more
([radio-connectors.md](../requirements/radio-connectors.md#audio-jack-35-mm-trrs)).

## Per-radio details and cable mappings

Cable mappings map the radio's pins to the interface's
[AUDIO and SERIAL jack contacts](../requirements/radio-connectors.md).
AUDIO: tip = RX audio from the radio, ring 1 = TX audio to the radio, ring 2 =
PTT closure, sleeve = ground. SERIAL: tip = data to the radio's RxD (or the
CI-V bus), ring 1 = data from the radio's TxD, ring 2 = off (or 3.3 V out),
sleeve = ground.

### Yaesu FT-891

| Item | Value | Source |
|---|---|---|
| DC | TUN/LIN 8-pin mini-DIN: "+13V OUT", plus TX GND, TX INH, band data | [891-OM] p. 24 |
| USB | USB-B device; CAT, PTT and firmware; serial only, no audio | [891-OM] p. 25; [891-CAT] p. 1; [Y-VCP] p. 1 |
| CAT | USB only; 4800 (default) / 9600 / 19200 / 38400 baud, menu 05-06 | [891-ADV] p. 78 |
| PTT | CAT command; RTS or DTR on the Standard COM port (menus 07-12, 08-10, 11-08); RTTY/DATA pin 3 (DAKY, default) | [891-ADV] p. 63, 84, 86; [Y-VCP] p. 7 |
| Audio | RTTY/DATA 6-pin mini-DIN: DATA IN, DATA OUT, PTT, GND, SHIFT, SQL; DATA IN SELECT default REAR | [891-OM] p. 25; [891-ADV] p. 86 |
| Notes | Needs **both** the radio USB port (CAT) and the AUDIO jack (audio). | [Y-VCP] p. 1 |

Cable: RTTY/DATA → AUDIO jack. DATA OUT → tip, DATA IN → ring 1, PTT (pin 3)
→ ring 2, GND → sleeve. The manual labels the pins in a figure; pin numbers
other than pin 3 are taken to match the FT-991A's identically labeled jack
(pin 1 DATA IN, 2 GND, 5 DATA OUT) **(verify)**. No SERIAL-jack cable: CAT is
USB only.

### Yaesu FT-710

| Item | Value | Source |
|---|---|---|
| DC | TUNER/LINEAR pin 1: +13 V, "linked to radio ON"; pin 3 GND | [710-CAT] p. 3 |
| USB | USB-B device: CAT, audio in/out, TX control. Separate USB-A host jack for keyboard/mouse | [710-OM] p. 14 |
| CAT | USB: CAT-1 (Enhanced) 38400 default; CAT-2 (Standard) 4800 default. **CAT-3 on TUNER/LINEAR: 5 V TTL**, pin 4 TXD (out), pin 5 RXD (in), 38400 default, set TUN/LIN PORT SELECT to CAT-3 (default EXT-TUNER); rates 4800–115200 | [710-CAT] p. 2–3; [710-OM] p. 74 |
| PTT | RPTT SELECT: DAKY (RTTY/DATA jack) or RTS/DTR on USB; CAT | [710-OM] p. 58 |
| Audio | RTTY/DATA 6-pin: 1 DATA IN, 2 GND, 3 DATA PTT, 4 SHIFT/FSK IN, 5 DATA OUT, 6 SQL OUT. USB OUT LEVEL / REAR OUT LEVEL 0–100 (default 50); MOD SOURCE MIC/USB/REAR/AUTO | [710-OM] p. 56, 57, 74 |
| Notes | CAT-3 can't be used with an external tuner or amplifier at the same time | [710-CAT] p. 3 |

Cables: RTTY/DATA → AUDIO jack (pin 5 → tip, pin 1 → ring 1, pin 3 → ring 2,
pin 2 → sleeve). TUNER/LINEAR → SERIAL jack plus power: pin 5 RXD ← tip,
pin 4 TXD → ring 1, pin 3 GND → sleeve, pin 1 +13 V → the power input
(separate lead). SERIAL mode: 3.3 V logic; whether 3.3 V is a valid high at the
radio's 5 V TTL RXD is `unknown`.

### Yaesu FT-991A

| Item | Value | Source |
|---|---|---|
| DC | TUN/LIN 8-pin: "+13V OUT" | [991A-OM] p. 27 |
| USB | USB-B device: CAT, audio in/out, TX control; the PC may key the radio when it starts | [991A-OM] p. 28 |
| CAT | GPS/CAT DE-9, RS-232 with a built-in level converter: 2 SERIAL OUT, 3 SERIAL IN, 5 GND, 7 RTS, 8 CTS. Set 028 GPS/232C SELECT to RS232C (default GPS1). 232C RATE and CAT RATE 4800 default (4800–38400) | [991A-OM] p. 27, 125; [991A-CAT] p. 1 |
| PTT | 071 DATA PTT SELECT: DAKY (default) / RTS / DTR; CAT | [991A-OM] p. 126 |
| Audio | RTTY/DATA 6-pin: 1 DATA IN, 2 GND, 3 PTT, 4 SHIFT, 5 DATA OUT, 6 SQL; 072 DATA PORT SELECT DATA (default) / USB | [991A-OM] p. 27, 126 |

Cables: RTTY/DATA → AUDIO jack (pin 5 → tip, pin 1 → ring 1, pin 3 → ring 2,
pin 2 → sleeve). GPS/CAT DE-9 → SERIAL jack in RS-232 mode: pin 3 SERIAL IN ←
tip, pin 2 SERIAL OUT → ring 1, pin 5 → sleeve.

### Yaesu FT-817ND and FT-818

| Item | Value | Source |
|---|---|---|
| DC | ACC 8-pin mini-DIN: "+13.8V", TX D, RX D, TX GND, TX INH, ALC, band data, GND. Supply 8–16 V or internal battery | [818-OM] p. 15, 70 |
| USB | None (optional SCU-17 external interface) | [818-OM] p. 64 |
| CAT | ACC TX D / RX D, logic level (CT-62 cable holds the level converter); 4800 (default) / 9600 / 38400 baud | [818-OM] p. 52, 64; [817ND-OM] p. 58 |
| PTT | DATA jack PTT, ground to transmit; CAT | [818-OM] p. 37 |
| Audio | DATA 6-pin mini-DIN, fixed level: levels in the table above. Pin 5 = DATA OUT 1200 bps, pin 2 = GND | [818-OM] p. 17, 37; [817ND-OM] p. 41, 43 |
| Notes | The FT-817ND manual describes the same ACC and DATA jacks without the ACC pin labels ([817ND-OM] p. 17); the FT-818 pinout is assumed to apply **(verify)** | |

Cables: DATA → AUDIO jack (DATA OUT → tip, DATA IN → ring 1, PTT → ring 2,
GND → sleeve). ACC → SERIAL jack in 3.3 V logic mode (RX D ← tip, TX D → ring
1, GND → sleeve), with +13.8 V to the power input. **Cross-checked** against
Digirig's FT-8xx cable pinouts (secondary, [DR-FT8XX]): its audio cable wires
DATA OUT to tip, DATA IN to ring and GND to sleeve, and its CAT cable wires ACC
RxD to tip, TxD to ring and GND to sleeve, which matches both the manual and
this project's contact assignment.

### Icom IC-7300

| Item | Value | Source |
|---|---|---|
| DC | ACC pin 8: 13.8 V when power is ON, 1 A max. Pin 1: regulated 8 V ± 0.3 V, < 10 mA | [7300-FM] p. 18-2 |
| USB | USB-B device: CI-V, audio (ACC/USB AF output, USB MOD level), SEND/keying on DTR/RTS | [7300-FM] p. 2-5, 12-7, 12-9 |
| CAT | [REMOTE] 3.5 mm CI-V (needs a CT-17 for a PC) and USB CI-V. CI-V baud Auto (4800–115200); address 94h; "CI-V USB Port" default "Link to [REMOTE]" | [7300-FM] p. 12-8, 12-9, 18-4 |
| PTT | CI-V command; USB SEND on DTR or RTS (default OFF); ACC pin 3 SEND (in/out); "Inhibit Timer at USB Connection" (default ON) delays SEND for a few seconds after USB connects | [7300-FM] p. 12-9, 18-2 |
| Audio | ACC pin 11 MOD in, pin 12 AF out, pin 13 SQL; levels in the table above. DATA MOD default ACC | [7300-FM] p. 12-8, 18-2 |
| Other | SEND jack (RCA) output: 16 V DC / 0.5 A max; 8-pin ACC-1/ACC-2 via OPC-599 | [7300-FM] p. 2-6, 18-3 |

Cables: ACC → AUDIO jack plus power: pin 12 → tip, pin 11 → ring 1, pin 3 →
ring 2, pin 2 → sleeve, pin 8 → the power input. [REMOTE] → SERIAL jack in
CI-V mode: the 3.5 mm jack carries the CI-V line; its tip/sleeve assignment is
shown only in a figure **(verify)**.

### Icom IC-705

| Item | Value | Source |
|---|---|---|
| DC | No DC output. [MIC] jack: 8 V (10 mA max) or 3.3 V through 470 Ω, menu "MIC Jack 8V Output" (default OFF = 3.3 V) | [705-BM] p. 8-14, 13-3 |
| USB | Micro-USB type B, USB 1.1/2.0, device: CI-V, AF/IF out, modulation in, RTTY decode, **battery charging** | [705-BM] p. 13-3 |
| USB power | "USB Power Input (Phone, Tablet, PC)" default **ON**: the radio uses a USB host as a power source and charges; faster with a 2 A USB port; may not charge through a hub or low-output port | [705-BM] p. 1-2, 8-6 |
| CAT | USB only (no CI-V jack); 2 virtual COM ports, A = CI-V; address A4h | [705-BM] p. 8-14; [705-AM] p. 9-1 |
| PTT | CI-V; USB SEND on DTR/RTS of port A or B (default OFF); [SEND/ALC] SEND (I/O) | [705-AM] p. 9-1; [705-BM] p. 13-2 |
| Audio | USB AF output (default AF, 50 %); USB MOD level 50 %; DATA MOD default USB. Analog: [SP] 3.5 mm, [MIC] 2.5 mm | [705-BM] p. 8-13, 13-3 |
| Supply | 13.8 V ± 15 % external, or 7.4 V battery | [705-BM] p. 11-1 |

Cable (analog fallback): [SP] left channel → AUDIO tip (attenuator on); [MIC]
2.5 mm "microphone output + PTT" contact ← ring 1; [SEND/ALC] SEND → ring 2;
grounds → sleeve. Icom recommends SEND rather than the MIC PTT for data
([705-AM] p. 18-1). The USB path is the normal one.

**Design consequence:** as USB host the interface supplies VBUS, and the
IC-705 will try to charge from it. #9/#11 must current-limit VBUS (the
current-limited switch in
[constraints §3.4](../requirements/constraints.md#34-power-budget-verify))
and document setting "USB Power Input" to OFF, or budget for charging current.

### Kenwood TS-590SG and TS-590S

| Item | Value | Source |
|---|---|---|
| DC | EXT.AT 6-pin, pin 6 "14S": switched 13.8 V, **4 A max** (TS-590SG); current not stated (TS-590S). Pins 1, 3 GND. Also MIC pin 5 switched 8 V (10 mA max, SG); REMOTE pin 7 ≈ +12 V in TX only (10 mA max) | [590SG-IM] p. 72; [590S-IM] p. 66 |
| USB | USB-B device: PC control through the virtual COM (Standard) port, and USB audio (drivers automatic) | [590SG-IM] p. 62, 73; [590SG-USB] p. 4, 10 |
| CAT | COM DE-9 (female), RS-232: 2 RXD "transmit data" (out), 3 TXD "receive data" (in), 5 GND, 7 RTS (in), 8 CTS (out). COM 9600 default, USB 115200 default (4800–115200); 4800 uses 2 stop bits | [590SG-IM] p. 19, 62, 71; [590S-IM] p. 18 |
| PTT | CAT `TX1;`/`RX;`; ACC2 pin 9 PKS (data PTT, mutes mic) or pin 13 SS; data VOX. The COM port's RTS/CTS can be swapped for PKS/PSQ | [590SG-IM] p. 62, 71, 73 |
| Audio | ACC2 13-pin DIN: 3 ANO out, 11 ANI in, 5 PSQ, 4/8/12 GND. Menu 69 ACC2 (default) / USB; levels menus 71–74 | [590SG-IM] p. 62, 71 |

Cables: ACC2 → AUDIO jack (pin 3 → tip, pin 11 → ring 1, pin 9 → ring 2,
pin 4 → sleeve). COM DE-9 → SERIAL jack in RS-232 mode (pin 3 ← tip, pin 2 →
ring 1, pin 5 → sleeve). EXT.AT pin 6 → power input. The EXT.AT connector is
meant for the AT-300 tuner, so a tuner and the interface can't share it
(inferred).

### Elecraft K3 and K3S

| Item | Value | Source |
|---|---|---|
| DC | 12 VDC OUT (RCA), switched: K3 0.5 A max, K3S 1.0 A max; 13 V no load, 12 V at max load (13.8 V supply). ACC pin 7 "K3 ON" 5 V logic out (unless set to TX INH) | [K3-OM] p. 8, 17; [K3S-OM] p. 8, 17, 21 |
| USB | K3 with KIO3: none (DE-9 RS-232). K3S (KIO3B): USB-B for CAT, audio, PTT/keying on DTR/RTS | [K3-OM] p. 8, 18; [K3S-OM] p. 18 |
| CAT | K3: DE-9 female, RS-232, up to 38400. K3S: RJ-45 + adapter to DE-9 female. DE-9 (PC's view): 2 RXD (data to PC), 3 TXD (data to radio), 5 GND, 4 DTR, 7 RTS; 8N1 | [K3-OM] p. 18; [K3S-OM] p. 19–20 |
| PTT | PTT IN (RCA); ACC pin 4 PTT IN; DTR/RTS via CONFIG:PTT-KEY (default inactive) | [K3S-OM] p. 17, 20 |
| Audio | LINE OUT stereo and LINE IN mono, transformer-isolated, 600 Ω; LIN OUT level default 10 | [K3S-OM] p. 22, 61 |

Cables: LINE OUT (left) → AUDIO tip, LINE IN ← ring 1, PTT IN → ring 2,
grounds → sleeve. DE-9 → SERIAL jack in RS-232 mode (pin 3 ← tip, pin 2 →
ring 1, pin 5 → sleeve). 12 VDC OUT → power input.

### Elecraft KX2 and KX3

| Item | Value | Source |
|---|---|---|
| DC | None; supply 8–15 V | [KX2-OM] p. 69; [KX3-OM] p. 54 |
| USB | None; KXUSB adapter cable (USB) or KXSER (RS-232) on the ACC jack | [KX2-OM] p. 9, 34; [KX3-OM] p. 5 |
| CAT | KX3 ACC1 3.5 mm stereo: tip = RX data (into the radio), ring = TX data (to the PC). KX2 ACC 3.5 mm: tip RX data, ring 1 TX data, ring 2 key out. RS232 menu 4800 default | [KX3-OM] p. 5, 39; [KX2-OM] p. 9, 49 |
| PTT | KX3 ACC2 2.5 mm GPIO (tip) as LO=PTT input: 3 V logic, 500 Ω series, 0–5.5 V tolerant; MIC jack ring 1 (PTT/UP/DN); VOX | [KX3-OM] p. 5, 18, 35; [KX2-OM] p. 8, 28 |
| Audio | Computer audio into the MIC jack; PHONES to the computer, "you may need an attenuator" | [KX3-OM] p. 18; [KX2-OM] p. 28 |
| Other | KX3 ACC2 ring / KX2 ACC ring 2: keyline, 30 V, 100 mA max, open drain | [KX3-OM] p. 54; [KX2-OM] p. 69 |

Cables: the KX3 ACC1 and the KX2 ACC jack already use this project's SERIAL
contact order (tip = data to the radio, ring 1 = data from the radio, sleeve =
ground), so a straight 3.5 mm cable fits. On the KX2 the plug's ring 2 is the
radio's key-out line, so the SERIAL jack's ring 2 **must stay off** (never the
"3.3 V out" mode) with that cable. Serial mode: `unknown` from the manual; the
Digirig KX cable set is used with RS-232 configuration (secondary, [DR-KX]).
Audio: PHONES → AUDIO tip (attenuator on), MIC tip ← ring 1, MIC PTT (ring 1 of
the MIC plug) or KX3 ACC2 GPIO ← ring 2.

### Elecraft K4

| Item | Value | Source |
|---|---|---|
| DC | 12 VDC OUT, switched, 1.5 A max, self-resetting fuse. Rear USB-A host ports: 400 mA max each | [K4-OM] p. 11, 13 |
| USB | USB-B "PC" device: 2 virtual COM ports (USB-PC1/PC2) and sound card (stereo out, mono in) | [K4-OM] p. 13 |
| CAT | USB-PC1/PC2; DE-9 "true RS232" (pinout not given in the manual) | [K4-OM] p. 13, 16 |
| PTT | PTT IN (RCA), pull to ground; RTS/DTR of any serial port | [K4-OM] p. 16 |
| Audio | LINE IN (600 Ω), LINE OUT (stereo, 600 Ω) | [K4-OM] p. 14 |

Cables: LINE OUT → AUDIO tip, LINE IN ← ring 1, PTT IN → ring 2. RS-232
DE-9 pin assignment `unknown` (not in the manual). A rear USB-A port could power
the interface at 5 V / 400 mA, but it is a host port, not a data link to the
interface's radio USB port.

### Xiegu X6100

| Item | Value | Source |
|---|---|---|
| DC | None on ACC (3.5 mm TRRS: BAND_V, TRX, ALC_V, GND). [MIC] RJ-45 has "+8V" (current `unknown`). Supply 9–15 V or internal battery | [X6100-UM] p. 8, 9; [X6100-EXT] p. 123 (secondary) |
| USB | USB-C "DEV" (device) and USB-C "HOST" (host) | [X6100-UM] p. 5 |
| CAT | CI-V subset over USB DEV; CH342 dual serial, SERIAL-B for CAT | [X6100-EXT] p. 105–106 (secondary) |
| PTT | CAT; [MIC] PTT pin | [X6100-UM] p. 8 |
| Audio | USB audio in/out (secondary, [X6100-EXT] p. 105); analog S/P 3.5 mm out, [MIC] RJ-45 in | [X6100-UM] p. 8 |

Cables: the USB DEV path is the normal one. Analog fallback: S/P → AUDIO tip
(attenuator on); [MIC] RJ-45 MIC ← ring 1, PTT ← ring 2, GND → sleeve.
**Cross-checked**: the Radioddity extended manual ([X6100-EXT] p. 105–106) and
Xiegu's manual ([X6100-UM] p. 5) agree that DEV is the USB device port and
HOST the USB host port.

### Generic interfaces

| Interface | Pins | Source |
|---|---|---|
| 6-pin mini-DIN DATA (Yaesu) | 1 DATA IN, 2 GND, 3 PTT, 4 SHIFT, 5 DATA OUT, 6 SQL (FT-991A, FT-710). FT-817ND: pin 5 DATA OUT (1200 bps), pin 2 GND; its other pins are shown only by label, including a 9600 bps DATA OUT | [991A-OM] p. 27; [710-OM] p. 56; [817ND-OM] p. 41, 43 |
| 13-pin DIN ACC (Icom / Kenwood) | Different per maker: IC-7300 pin 11 MOD, 12 AF, 3 SEND, 8 13.8 V; TS-590SG pin 11 ANI, 3 ANO, 9 PKS | [7300-FM] p. 18-2; [590SG-IM] p. 71 |
| 8-pin ACC | Yaesu mini-DIN: +13.8 V, TX D, RX D, GND ([818-OM] p. 15). Icom 8-pin DIN (ACC-1/ACC-2): 1 8 V, 2 GND, 3 SEND, 4 BAND, 5 ALC, 7 13.8 V ([7300-FM] p. 18-3) | as listed |
| 3.5 mm CI-V | Single-wire CI-V bus on a 3.5 mm jack; PC needs a CT-17 level converter | [7300-FM] p. 18-4 |
| DE-9 RS-232 | Radio as DCE: pin 2 data from the radio, pin 3 data to the radio, pin 5 ground | [991A-CAT] p. 1; [590SG-IM] p. 71; [K3-OM] p. 18 |

Cable rule for all of them: radio data/audio **output** → the jack's tip,
radio **input** → ring 1, PTT → AUDIO ring 2, ground → sleeve.

## Cross-checks against a second source

1. **FT-817ND / FT-818:** Digirig FT-8xx cable pinouts ([DR-FT8XX]) match the
   manual's DATA and ACC signals and this project's tip/ring order (see the
   FT-817ND/FT-818 section).
2. **FT-891:** Digirig's DR-891 manual ([DR-891]) confirms the FT-891's USB port
   carries only its USB-to-serial bridge (Enhanced and Standard COM ports) and no
   USB audio, matching [Y-VCP] p. 1 and 7.
3. **KX2 / KX3:** Digirig's KX cable set ([DR-KX]) uses one 3.5 mm TRS cable
   for serial CAT and separate speaker and mic cables for audio, matching the
   manuals (ACC for data, MIC/PHONES for audio), and adds that the serial side
   runs in RS-232 configuration.
4. **X6100:** Radioddity's extended manual ([X6100-EXT]) agrees with Xiegu's
   manual on the DEV/HOST USB ports.

## Needs measurement

For a later `human-task` (bench measurements with each radio):

1. **DC output current limit** (and whether it is switched with the radio):
   FT-891, FT-710 and FT-991A TUN/LIN "+13V"; FT-817ND/FT-818 ACC "+13.8V"
   (also its voltage on battery power); TS-590S EXT.AT 14S (limit only);
   X6100 [MIC] +8 V.
2. **VBUS current drawn from the interface** by every radio with a USB port:
   FT-891, FT-710, FT-991A, IC-7300, IC-705 (with "USB Power Input" ON and
   OFF), TS-590S/SG, K3S, K4, X6100. Include the inrush.
3. **USB descriptors** (`lsusb -v` or equivalent) for the same radios: the
   USB-serial chip (VID/PID), the USB Audio Class version, sample rates, and
   whether there is an internal hub or a composite device.
4. **PTT line:** open-circuit voltage and closed-circuit current on every
   radio's PTT/SEND/DAKY/PKS input: FT-891, FT-710, FT-991A, FT-817ND/FT-818,
   IC-7300 (open-circuit only), IC-705 SEND, TS-590S/SG PKS, K3/K3S PTT IN, K4
   PTT IN, X6100 MIC PTT.
5. **RX audio level at maximum setting**: FT-891, FT-710, FT-991A DATA OUT;
   K3/K3S and K4 LINE OUT; KX2/KX3 PHONES and IC-705 [SP] at full volume;
   X6100 S/P.
6. **Serial logic levels:** FT-817ND/FT-818 ACC TX D/RX D; KX2/KX3 ACC data
   lines; the FT-710 CAT-3 RXD input threshold with a 3.3 V driver.
7. **Pin numbers shown only in figures:** FT-891 RTTY/DATA; IC-7300 [REMOTE]
   contacts; K4 RS-232 DE-9; X6100 [MIC] RJ-45 and ACC TRX function.

## Implications for #9, #11 and #12

- **Radio-powered budget (#11):** documented and sufficient on the IC-7300
  (1 A), TS-590SG (4 A), K3 (0.5 A), K3S (1 A) and K4 (1.5 A); all are
  switched, so they also serve as a "radio on" sense. The Yaesu +13 V pins
  exist but have no documented limit. The IC-705, KX2, KX3 and X6100 can't power
  the interface: USB-C or battery only.
- **USB host (#9):** required for the FT-891, IC-705 and X6100 (no serial
  jack), and the usual path for the FT-710, FT-991A, IC-7300, TS-590S/SG, K3S
  and K4. Chips seen: dual CP210x (Yaesu), CH342 (Xiegu, secondary); the others
  are `unknown`. The FT-891 has USB serial but analog audio: it needs the
  radio USB port and the AUDIO jack together.
- **VBUS (#9, #11):** the VBUS switch must current-limit and report
  overcurrent; the IC-705 charges from VBUS by default.
- **Level conversion (#9):** RS-232 for the FT-991A, TS-590S/SG, K3/K3S and K4;
  CI-V for the IC-7300; 5 V TTL for the FT-710 CAT-3 and logic level for the
  FT-817ND/FT-818 ACC; unknown for the KX2/KX3. All fit the SERIAL jack's
  firmware-selected modes.
- **PTT (#9):** the only documented limits are the IC-7300 SEND input (2–20 V,
  20 mA max) and the KX3 GPIO (3 V logic, 0–5.5 V tolerant). #9 checks them
  against the photo-MOSFET closure's ratings; the rest need measurement.

## Sources

Manufacturer manuals unless marked. Copies are in the gitignored reference
cache (`python3 tools/refs/refs.py fetch`).

| Key | Document |
|---|---|
| [891-OM] | Yaesu FT-891 Operating Manual |
| [891-ADV] | Yaesu FT-891 Advance Manual |
| [891-CAT] | Yaesu FT-891 CAT Operation Reference Book |
| [710-OM] | Yaesu FT-710 Operation Manual |
| [710-CAT] | Yaesu FT-710 CAT Operation Reference Manual |
| [991A-OM] | Yaesu FT-991A Operating Manual |
| [991A-CAT] | Yaesu FT-991A CAT Operation Reference Manual |
| [818-OM] | Yaesu FT-818ND Operating Manual |
| [817ND-OM] | Yaesu FT-817ND Operating Manual |
| [Y-VCP] | Yaesu Virtual COM Port Driver Installation Manual |
| [7300-FM] | Icom IC-7300 Full Manual |
| [705-BM] | Icom IC-705 Basic Manual |
| [705-AM] | Icom IC-705 Advanced Manual |
| [590SG-IM] | Kenwood TS-590SG Instruction Manual |
| [590SG-USB] | Kenwood TS-590SG USB Audio Setting Manual |
| [590S-IM] | Kenwood TS-590S Instruction Manual (third-party mirror) |
| [K3-OM] | Elecraft K3 Owner's Manual |
| [K3S-OM] | Elecraft K3S Owner's Manual |
| [KX2-OM] | Elecraft KX2 Owner's Manual |
| [KX3-OM] | Elecraft KX3 Owner's Manual |
| [K4-OM] | Elecraft K4 Operating Manual |
| [X6100-UM] | Xiegu X6100 User Manual |
| [X6100-EXT] | Radioddity extended manual for the X6100 (secondary) |
| [DR-FT8XX] | Digirig Yaesu FT-8xx cables build (secondary) |
| [DR-891] | Digirig DR-891 setup manual (secondary) |
| [DR-KX] | Digirig Elecraft KX cables set (secondary) |

Digirig's hardware is GPL-3.0; its pinouts are used as facts only, nothing is
copied ([`THIRD_PARTY.md`](../../THIRD_PARTY.md#facts-only-never-copied)).

[891-OM]: ../references/index.md#yaesu-ft891-om
[891-ADV]: ../references/index.md#yaesu-ft891-adv
[891-CAT]: ../references/index.md#yaesu-ft891-cat
[710-OM]: ../references/index.md#yaesu-ft710-om
[710-CAT]: ../references/index.md#yaesu-ft710-cat
[991A-OM]: ../references/index.md#yaesu-ft991a-om
[991A-CAT]: ../references/index.md#yaesu-ft991a-cat
[818-OM]: ../references/index.md#yaesu-ft818-om
[817ND-OM]: ../references/index.md#yaesu-ft817nd-om
[Y-VCP]: ../references/index.md#yaesu-vcp-driver
[7300-FM]: ../references/index.md#icom-ic7300-full
[705-BM]: ../references/index.md#icom-ic705-basic
[705-AM]: ../references/index.md#icom-ic705-adv
[590SG-IM]: ../references/index.md#kenwood-ts590sg-im
[590SG-USB]: ../references/index.md#kenwood-ts590sg-usb-audio
[590S-IM]: ../references/index.md#kenwood-ts590s-im
[K3-OM]: ../references/index.md#elecraft-k3-om
[K3S-OM]: ../references/index.md#elecraft-k3s-om
[KX2-OM]: ../references/index.md#elecraft-kx2-om
[KX3-OM]: ../references/index.md#elecraft-kx3-om
[K4-OM]: ../references/index.md#elecraft-k4-om
[X6100-UM]: ../references/index.md#xiegu-x6100-um
[X6100-EXT]: ../references/index.md#radioddity-x6100-ext
[DR-FT8XX]: ../references/index.md#digirig-ft8xx-cables
[DR-891]: ../references/index.md#digirig-dr891-manual
[DR-KX]: ../references/index.md#digirig-kx-cables

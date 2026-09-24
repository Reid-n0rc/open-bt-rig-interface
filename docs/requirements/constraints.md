<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Design constraints

The hard constraints every hardware, firmware and enclosure decision must meet.
Values marked **(verify)** are not yet confirmed. Each one belongs to a research
issue and must be confirmed (or corrected here) before a design depends on it.
Decisions that settle an open point are recorded in [`../decisions/`](../decisions/).
The numbered, testable form of these constraints is
[`requirements.md`](requirements.md).

## 1. What the device is

An interface between an amateur-radio transceiver and a host (phone, tablet or
computer), over Bluetooth LE or a wired USB-C connection (§2). It carries:

- **CAT serial:** transparent byte passthrough. The device never interprets the
  radio's command set.
- **PTT:** a dedicated PTT output, plus RTS/DTR-style control lines.
- **Audio:** RX audio from the radio to the host, and TX audio from the host to the radio.

## 2. Host compatibility

The device has two host links ([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)):
**Bluetooth LE**, using this project's versioned protocol
([`../../protocol/`](../../protocol/)), and **wired USB-C**, using standard USB
classes. The radio side works the same in both (§7).

| Host | Wired USB-C | Bluetooth LE |
|---|---|---|
| Windows, macOS, Linux | Native serial port (USB CDC-ACM) and sound card (USB Audio Class); no drivers | Apps or host software that implement the protocol |
| Android | Sound card natively; serial through apps (USB CDC-ACM) | Apps that implement the protocol |
| iOS / iPadOS | Sound card natively (USB-C devices); no app access to USB serial | Apps that implement the protocol |

Consequences:

- **No Bluetooth Classic.** SPP, HFP and RFCOMM are not used. Revision A uses the
  ESP32-S3-MINI-1 (§4, §10).
- **Wired mode is selected automatically** when a USB host enumerates the device
  on USB-C, Bluetooth mode otherwise, with a setting to force either. The
  Bluetooth radio is off in wired mode.
- **In wired mode, radios with their own USB port appear to the computer
  directly** (through an on-board USB hub): their USB-serial chip, and their
  sound card if they have one. The device adds its own USB sound card when the
  radio's audio is analog, its own USB serial port bridged to the SERIAL jack
  when the radio's serial isn't USB, and always a USB serial port for
  configuration and AUDIO-jack PTT.
- **No OS shows a Bluetooth LE device as a serial port or audio device
  natively.** Over Bluetooth, hosts need apps or host software that implement
  the protocol. User documentation must state this plainly.
- **Audio over Bluetooth LE:** at least 12 kHz / 16-bit mono, one direction at a
  time (192 kbit/s), over an L2CAP connection-oriented channel or GATT, using the
  2M PHY where the host supports it. Throughput to each OS **(verify)**,
  especially iOS. LC3 compression (permissively licensed implementation) is the
  fallback if raw PCM doesn't fit.
- **Audio over USB:** 48 kHz / 16-bit, USB Audio Class (UAC1 or UAC2, whichever
  every target OS supports without drivers **(verify)**).
- **RTS/DTR:** native in wired mode (USB CDC-ACM line state); protocol messages
  over Bluetooth. PTT through the protocol's control channel, or through the
  radio's CAT command, must work regardless.
- Not required: Hamlib/FLrig-specific features, Wi-Fi, LE Audio (LC3 as a
  Bluetooth profile).

## 3. Power

### 3.1 Sources

- **The radio's USB port provides no power.** Transceivers such as the FT-891,
  FT-710, FT-991A and IC-7300 have USB *device* ports (their internal
  USB-serial and codec chips). When this device is the **USB host** to such a port,
  it must *supply* VBUS to the radio, current-limited. The radio's draw is **(verify)**.
- **Radio accessory DC (preferred where available).** Some radios provide DC on
  an accessory jack (for example the IC-7300 ACC socket, or the FT-891
  tuner/linear jack). The pin, voltage and current limit per radio are **(verify)**,
  tracked in a per-radio table.
- **USB-C 5 V sink:** phone charger, power bank or computer.
- **12 V vehicle or station supply:** the automotive variant (3.2).

### 3.2 Automotive 12 V input (variant M)

Must survive a harsh automotive environment. Target the 12 V-system levels of
**ISO 16750-2:2023** and **ISO 7637-2:2011** (level IV), confirmed in
[`power-automotive.md`](../research/power-automotive.md#1-test-levels) (#10;
clauses and sources there). The design is in
[ADR-0004](../decisions/ADR-0004-power-automotive.md):

| Condition | Target |
|---|---|
| Normal operating range | 9–16 V |
| Cold crank | Operate down to about 6 V (4.5 V desirable), **or** brown out safely with PTT off ("normal" profile 4.5 V then 6.5 V; "severe" 3 V then 5 V) |
| Load dump | Test A (unsuppressed): 79–101 V, Ri 0.5–4 Ω, 40–400 ms, 10 pulses; test B (centrally suppressed): up to 35 V |
| Reverse battery | −14 V for 60 s, no damage |
| Jump start | 26 V for 60 s (ISO 16750-2:2023; 24 V in the 2012 edition) |
| ISO 7637-2 transients | Pulse 1 about −150 V; pulse 2a about +112 V; pulses 3a/3b about −220 V / +150 V |
| ESD | ±15 kV air (ISO 10605) **(verify)** |
| Temperature | −40 °C to +85 °C operating |
| Off-state drain | < 1 mA; auto power-down when the radio or ignition is off |

Design guidance (confirmed in the power-front-end issue):

- ideal-diode or reverse-polarity controller;
- surge stopper and/or TVS for load dump;
- AEC-Q100 wide-input buck converter;
- input common-mode choke plus pi filter;
- **low-EMI conversion** (silent-switcher or spread-spectrum), with the switching
  frequency chosen and filtered so it doesn't land on HF amateur bands;
- target CISPR 25 Class 3 or better conducted and radiated emissions;
- vibration-rated, locking connectors.

### 3.3 Radio-sourced / USB-C input (variant R)

- 13.8 V accessory input: 11–15 V range, reverse-polarity and TVS protection;
  must stay within each radio's accessory-pin current limit **(verify)**.
- USB-C: 5 V sink (USB PD not required).

### 3.4 Power budget **(verify)**

| Load | Estimate |
|---|---|
| Radio module (ESP32-S3-MINI-1), BLE active | about 0.1 A typical, 0.34 A peak (BLE TX at +20 dBm, datasheet) @ 3.3 V |
| USB hub (wired mode) | about 50 mA **(verify with the hub chosen)** |
| Audio codec | about 50 mA |
| Target total | about 1.5 W typical, 3 W peak |

The device supplies no power to the radio: the radio port's VBUS is blocked in
hardware (ADR-0003, [#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9)).

**USB-C budget:** a USB-C host without USB PD may supply only 500 mA at 5 V
(USB 2.0 default). In wired mode that must cover the device, the hub and the
radio's USB VBUS draw. Use the higher USB-C current advertised on CC (1.5 A or
3 A) when present, and report an overcurrent to the host rather than browning out.

## 4. Regulatory

- **FCC-certified radio module required.** Record its FCC ID. Follow the
  module's integration guide exactly: approved antenna type and gain, keep-out
  area, RF trace layout.
- The end product needs **FCC Part 15 Subpart B** (unintentional radiator,
  Class B) SDoC, and a "Contains FCC ID: …" label.
- ISED and CE: optional, later.
- No metal over the module antenna, unless the module is certified with an
  external antenna.

## 5. RF environment

The device operates next to HF transmitters of 100 W or more.

- PTT must **never** assert from RF pickup.
- Filter audio, CAT and PTT lines (ferrites and RC). Plan grounding to avoid
  ground loops.
- The device must not raise the receiver's noise floor: no switching harmonics
  or digital noise in the HF bands.

## 6. Safety and fail-safe

- PTT is **off** at power-on, reset, brownout, Bluetooth disconnect and watchdog timeout.
- **Hardware default off:** a pull-down, or an opto/MOSFET that must be
  actively driven, so a hung MCU cannot key the radio.
- The firmware enforces a maximum continuous TX time (configurable, cannot be disabled).
- **Galvanic isolation** of the AUDIO and SERIAL jacks toward the radio:
  transformer-coupled audio, isolated PTT, and digital isolators on the serial
  lines. **Required for variant M, and on every variant whenever the USB-C data
  link is used** (it brings the computer's ground to the device). Optional only
  for variant R used over Bluetooth and powered from the radio.
- The **radio USB port** is not isolated by default (like a direct USB cable).
  A full-speed USB isolator (ADuM4160-class) is a fitting option, recommended for
  variant M.

## 7. Radio interfaces

- **CAT:** TTL 3.3/5 V, RS-232 levels, Icom CI-V (single-wire open-collector
  bus); 4800–115200 baud. The mode is selected in firmware, and **every mode must
  survive any cable**: RS-232 levels (±15 V) on any SERIAL-jack contact must not
  damage the logic or CI-V paths, and vice versa. Use high-voltage analog
  switches or signal relays and an RS-232 transceiver whose drivers go
  high-impedance when disabled. Power-on default: 3.3 V logic.
- **PTT:** isolated closure to ground on the AUDIO jack. Host RTS/DTR (native in
  wired mode, protocol messages over Bluetooth) map to this closure, or to the
  radio's USB-serial chip when the radio has one. The SERIAL jack has no RS-232
  RTS/DTR contacts ([`radio-connectors.md`](radio-connectors.md)).
- **USB host:** for radios with their own USB port. The device is the USB host to
  the radio's USB-serial chip (CP210x including dual-port CP2105, FTDI, CH34x,
  CDC-ACM) **and** its built-in USB sound card (USB Audio Class 1.0), including
  through a USB hub inside the radio. Full speed (12 Mbit/s) is enough. Supplies
  current-limited VBUS (3.1). In wired mode the same port is routed to the
  on-board hub so the computer reaches the radio's chips directly (§2).
- **Radio-type coverage, in both host modes:** radios with USB serial + USB
  audio; USB serial + analog audio; RS-232, 3.3 V logic or CI-V serial + analog
  audio.
- **Connectors:** two 3.5 mm TRRS jacks (AUDIO and SERIAL) with a fixed pinout
  that is compatible with existing cables made for that convention, plus a USB-A
  host port. Per-radio cables or harnesses (mini-DIN, 3.5 mm, DB9, USB), not
  per-radio boards. See [`radio-connectors.md`](radio-connectors.md).

## 8. Audio

- **Two radio audio paths, both on every board:** analog (the internal codec,
  line level, isolated per §6) and USB (the radio's built-in USB sound card, §7).
  The firmware uses USB audio when a radio sound card enumerates, with a manual
  override. In wired mode with a radio USB sound card, the computer uses that
  sound card directly and the device's audio path is idle.
- The internal codec runs at 48 kHz. The host path is at least 12 kHz / 16-bit (§2).
- **Rate matching:** a radio's USB sound card runs on its own clock. The firmware
  matches rates between it and the Bluetooth LE stream without audible artifacts
  or tone shift.
- ADC SNR ≥ 90 dB (A-weighted) **(verify with codec choice)**.
- Sample clock accuracy ±50 ppm or better (no tone shift).
- No automatic gain control, noise suppression or voice processing in the
  device path. Adjustable TX level.
- Fixed, measured latency.

## 9. Timing

- Clock sync from the host over Bluetooth. Scheduled TX starts within about ±20 ms
  of UTC.
- Optional firmware module: generate scheduled tone sequences on the device
  (FT8/FT4/WSPR/JS8-class modes). This isn't part of the core.

## 10. Firmware

- SDK and all firmware dependencies under permissive licenses (MIT, BSD,
  Apache-2.0, or similar). NDA-only SDKs are excluded. Permissive
  dependencies keep the project's own firmware licensable under both the
  non-commercial and the commercial terms ([`COMMERCIAL.md`](../../COMMERCIAL.md)).
  Dependencies keep their own licenses, with their notices in
  [`THIRD_PARTY.md`](../../THIRD_PARTY.md).
- No radio-specific logic in the core (transparent CAT).
- The protocol is versioned, with capability discovery. Configuration happens
  over Bluetooth.
- Optional signed OTA updates.

## 11. Environmental and mechanical

| | Variant R | Variant M |
|---|---|---|
| Operating temperature | −20 °C to +60 °C | −40 °C to +85 °C |
| Enclosure material | PETG (3D printed) | **ASA** (or ABS), 3D printed; PLA/PETG soften in a parked car |

- **3D-printed enclosure:** parametric CAD source in `hardware/enclosure/`,
  printable without supports where possible, antenna keep-out respected, mounting
  and strain relief, space for the FCC ID label. STL/3MF are generated and
  attached to releases.
- Size target: **TBD by maintainer.**

## 12. Manufacturing and sourcing

- PCB and passive components follow [`pcb-fabrication.md`](pcb-fabrication.md):
  JLCPCB standard (low-cost) process, 2 layers preferred and 4 acceptable,
  0402 resistors by default, MLCC capacitors rated at least 2× the node's
  nominal DC voltage with DC-bias derating checked.
- At least two distributor sources for each key part (e.g. Digi-Key, Mouser, LCSC), with an
  active lifecycle status.
- BOM cost target: **TBD by maintainer.**

## 13. Tooling and documentation

- **KiCad ≥ 10.0.6.** The documented version must match the checked-in files
  ([`KICAD_VERSION`](../../KICAD_VERSION)), enforced by CI.
- All KiCad changes go through the Konnect MCP tools (see [`AGENTS.md`](../../AGENTS.md)).
- **PCB silkscreen:** project name, variant, `${REVISION}`, `${ISSUE_DATE}`,
  "Designed by Reid Crowe, N0RC", and the license mark. The revision is never
  hard-coded.
- Board variants (R = radio/USB-powered, M = mobile/automotive; others possible)
  share one core design. Whether they are separate boards or one board with
  fitting options is a recorded decision.

<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Design constraints

The hard constraints every hardware, firmware and enclosure decision must meet.
Values marked **(verify)** are not yet confirmed. Each one belongs to a research
issue and must be confirmed (or corrected here) before a design depends on it.
Decisions that settle an open point are recorded in [`../decisions/`](../decisions/).

## 1. What the device is

A Bluetooth interface between an amateur-radio transceiver and a host (phone,
tablet or computer). It carries:

- **CAT serial:** transparent byte passthrough. The device never interprets the
  radio's command set.
- **PTT:** a dedicated PTT output, plus RTS/DTR-style control lines.
- **Audio:** RX audio from the radio to the host, and TX audio from the host to the radio.

## 2. Host compatibility

The device must appear as **both an audio device and a serial connection** on
all five host platforms:

| Host | Serial | Audio (in + out) |
|---|---|---|
| Windows | Classic Bluetooth SPP, shown as a COM port | HFP, shown as a headset device |
| macOS | SPP, shown as `/dev/cu.*` | HFP input/output device |
| Linux | SPP, shown as `rfcomm` (BlueZ) | HFP via PipeWire's native backend |
| Android | SPP, opened by apps through `BluetoothSocket` | HFP (apps start the SCO link) |
| iOS | **Bluetooth LE only, app-level only** (see below) | HFP (any headset; no MFi needed) |

Consequences:

- **Dual-mode Bluetooth is mandatory:** Classic BR/EDR (SPP + HFP) and Bluetooth LE,
  running at the same time. A Bluetooth-LE-only radio (for example ESP32-S3/C3/C6/C5
  or Nordic nRF) cannot be the only radio.
- **HFP must support wideband speech (mSBC, 16 kHz).** CVSD (8 kHz) is the
  fallback. FT8-class audio (about 200–3000 Hz) fits in both.
- **iOS has no SPP** for accessories outside Apple's MFi program. On iOS, serial
  is available only to apps that implement this project's Bluetooth LE protocol
  (see [`../../protocol/`](../../protocol/)). This platform limitation must be
  stated in user documentation.
- **RTS/DTR over SPP:** RFCOMM carries modem-status signals, which the device may
  map to PTT. Whether each host OS passes an application's RTS/DTR changes through
  is **(verify)**. PTT through the device's own control channel, or through the
  radio's CAT command, must work regardless.
- **HFP limits:** codec artifacts, packet-loss concealment and host OS voice
  processing may degrade weak-signal decoding **(verify per OS)**. An optional
  higher-quality Bluetooth LE audio channel (L2CAP) may be offered to apps that
  support it. It supplements the standard HFP path and doesn't replace it.
- Not required: Hamlib/FLrig-specific features, Wi-Fi, LE Audio (LC3).

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
ISO 16750-2 and ISO 7637-2 **(verify exact levels and editions)**:

| Condition | Target |
|---|---|
| Normal operating range | 9–16 V |
| Cold crank | Operate down to about 6 V (4.5 V desirable), **or** brown out safely with PTT off |
| Load dump | Unsuppressed up to about 101 V, 40–400 ms; suppressed (centrally clamped) about 35 V |
| Reverse battery | −14 V for 60 s, no damage |
| Jump start | 24 V for 60 s |
| ISO 7637-2 transients | Pulse 1 about −150 V; pulse 2a about +112 V; pulses 3a/3b about −220 V / +150 V |
| ESD | ±15 kV air (ISO 10605) |
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
| Radio module, Classic + BLE active | about 0.1–0.25 A @ 3.3 V |
| Audio codec | about 50 mA |
| USB host VBUS to the radio | up to about 0.5 A (current-limited switch) |
| Target total | about 1.5 W typical, 3 W peak |

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
- **Galvanic isolation** toward the radio: transformer-coupled audio,
  opto-isolated PTT, and digital isolators on CAT when the device is powered
  separately from the radio. Required for variant M, optional for variant R.

## 7. Radio interfaces

- **CAT:** TTL 3.3/5 V, RS-232 levels, Icom CI-V (single-wire open-collector
  bus); 4800–115200 baud.
- **PTT:** isolated closure to ground; RTS/DTR outputs at RS-232 levels for
  cables that expect them.
- **USB host:** for radios whose only CAT/audio path is their own USB port
  (CP210x, FTDI, CDC-ACM serial chips). Supplies current-limited VBUS (3.1).
- **Connectors:** per-radio cables or harnesses (mini-DIN, 3.5 mm, DB9) rather than
  per-radio boards, where practical.

## 8. Audio

- The internal codec runs at 48 kHz. The host path is at least 12 kHz / 16-bit
  where the transport allows (HFP is limited to 16 kHz mSBC or 8 kHz CVSD).
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
  Apache-2.0, or similar). NDA-only SDKs are excluded.
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

- 2–4 layer PCB, assembly-house-friendly parts.
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

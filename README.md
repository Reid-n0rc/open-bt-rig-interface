<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# open-bt-rig-interface

An open-source **hardware + firmware** Bluetooth interface between an amateur-radio
transceiver and a phone, tablet or computer. It carries **CAT serial**, **PTT**
(including RTS/DTR-style control) and **audio** without a cable to the host.

> **Status:** planning and research. No hardware or firmware yet. See the open
> issues.

## Host support (target)

The device appears as a **standard Bluetooth audio device plus a serial
connection**:

| Host | Serial | Audio |
|---|---|---|
| Windows | COM port (Bluetooth SPP) | Headset device (HFP, mSBC) |
| macOS | `/dev/cu.*` (SPP) | Input/output device (HFP) |
| Linux | `rfcomm` (SPP) | PipeWire (HFP) |
| Android | SPP (app-level `BluetoothSocket`) | HFP |
| iOS | **Bluetooth LE, app-level only.** Apps must implement the [protocol](protocol/); iOS offers no SPP to non-MFi accessories | HFP |

## Planned variants

- **R: radio/USB-powered.** Powered from the radio's accessory DC where available,
  or USB-C 5 V. Note that a radio's USB port is a *device* port and supplies no power.
- **M: mobile/automotive.** A 12 V input built for a harsh automotive environment
  (ISO 16750-2 / ISO 7637-2 targets).

Both share one core design: a dual-mode Bluetooth module, audio codec, isolated
PTT, CAT (TTL/RS-232/CI-V), and a USB host for radios that expose only USB. See
[`docs/requirements/constraints.md`](docs/requirements/constraints.md).

## Repository layout

| Path | Contents |
|---|---|
| `hardware/boards/` | KiCad projects, one folder per board revision |
| `hardware/lib/` | Project-local symbols, footprints, 3D models |
| `hardware/enclosure/` | 3D-printed enclosure CAD |
| `firmware/` | Device firmware (`app/` portable, `platform/` SDK glue, `test/`) |
| `protocol/` | Host-device Bluetooth protocol spec and golden vectors |
| `tools/` | Scripts and checks |
| `docs/` | Requirements, decisions (ADRs), bring-up, compliance |

## Contributing

Every change starts from an issue and lands through a PR to `dev`. Read
[`AGENTS.md`](AGENTS.md) (the rules for humans and AI agents alike) and
[`CONTRIBUTING.md`](CONTRIBUTING.md).
For setup, local checks and step-by-step recipes, see the
[developer guide](docs/developer-guide.md).

## Background

This project was prompted by the needs of an iOS FT8 client,
[FT8AF](https://github.com/patrickrb/FT8AF). iOS apps can't reach USB-serial
CAT cables, so a Bluetooth interface is the practical way to get full CAT, PTT
and audio. The design is deliberately app-neutral: FT8AF is one possible
client among many.

## License

Designed by **Reid Crowe, N0RC**.

- Hardware (`hardware/`): **CERN-OHL-P-2.0**
- Firmware, tools and protocol code: **MIT**
- Documentation: **CC-BY-4.0**

See [`LICENSE`](LICENSE) and [`LICENSES/`](LICENSES/). The repo follows the
[REUSE](https://reuse.software/) specification.

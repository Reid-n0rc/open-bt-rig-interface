<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# open-bt-rig-interface

A source-available **hardware + firmware** interface between an amateur-radio
transceiver and a phone, tablet or computer, over **Bluetooth LE** or **wired
USB-C**. It carries **CAT serial**, **PTT** (including RTS/DTR-style control)
and **audio**.

> **Status:** planning and research. No hardware or firmware yet. See the open
> issues.

## Host support (target)

See [ADR-0008](docs/decisions/ADR-0008-host-links-esp32-s3.md):

| Host | Wired USB-C | Bluetooth LE |
|---|---|---|
| Windows, macOS, Linux | Native serial port and sound card, no drivers | Apps or host software that implement the [protocol](protocol/) |
| Android | Sound card natively; serial through apps | Apps that implement the protocol |
| iOS / iPadOS | Sound card natively | Apps that implement the protocol |

On the radio side it supports, in both modes, radios with a built-in USB-serial
chip and sound card, radios with USB serial and analog audio, and radios with
RS-232, 3.3 V logic or Icom CI-V serial and analog audio. In wired mode, radios
with their own USB port appear to the computer directly through an on-board hub.

## Planned variants

- **R: radio/USB-powered.** Powered from the radio's accessory DC where available,
  or USB-C 5 V. Note that a radio's USB port is a *device* port and supplies no power.
- **M: mobile/automotive.** A 12 V input built for a harsh automotive environment
  (ISO 16750-2 / ISO 7637-2 targets).

Both share one core design: an ESP32-S3-MINI-1 Bluetooth LE module, audio codec,
isolated PTT, CAT (TTL/RS-232/CI-V), and a USB host for the radio's USB sound
card and USB-serial chip. See
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
[`CONTRIBUTING.md`](CONTRIBUTING.md). Roles and decision-making, including who
merges and releases, are in [`GOVERNANCE.md`](GOVERNANCE.md). The plan for
revision A (phases, dependencies, exit criteria and merge gates) is in
[`docs/roadmap.md`](docs/roadmap.md), and live status is in issue
[#22](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/22).
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

The hardware and firmware are **free for personal, amateur and non-commercial
use**. **Commercial use** (selling boards, kits or units, or using the design or
firmware in a product) **needs a commercial license**. See
[`COMMERCIAL.md`](COMMERCIAL.md).

| Part | License |
|---|---|
| Hardware (`hardware/`) | **CC-BY-NC-SA-4.0** (non-commercial) |
| Firmware (`firmware/`) | **PolyForm-Noncommercial-1.0.0** (non-commercial) |
| Protocol spec + golden vectors, tools, CI | **MIT**: any app, including commercial ones, can implement the protocol |
| Documentation | **CC-BY-4.0** |

Third-party components keep their own licenses; see
[`THIRD_PARTY.md`](THIRD_PARTY.md). See [`LICENSE`](LICENSE) and
[`LICENSES/`](LICENSES/). The repo follows the
[REUSE](https://reuse.software/) specification.

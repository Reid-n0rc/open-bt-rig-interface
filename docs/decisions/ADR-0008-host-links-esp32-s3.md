<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0008: Bluetooth LE and wired USB-C host links on the ESP32-S3-MINI-1, with a common radio side

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #42

## Context

The original constraints ([`constraints.md`](../requirements/constraints.md) §2,
before this ADR) made **dual-mode Bluetooth** mandatory: Classic SPP + HFP so the
device shows up as a native COM port and headset on Windows, macOS, Linux and
Android, plus BLE for iOS.

Research for the core device shortlist ([`core-devices.md`](../research/core-devices.md))
found that this requirement leaves almost one silicon vendor:

- The only certified, well-stocked, permissively licensed dual-mode module is the
  original **ESP32** (ESP32-WROOM-32E). It has no USB, so radios with a built-in
  USB port need an extra host controller (MAX3421E, whose lifecycle is unclear).
- Other dual-mode options fail a hard constraint: SDK license (Infineon
  CYW20820: Cypress EULA), SPP and audio at once (Microchip BM83), HFP audio
  (Raspberry Pi RM2), no FCC-certified module (SiFli SF32LB52, Beken BK7258), or
  cost at 4–8× (NXP IW416 + host MCU).
- HFP limits audio to 16 kHz mSBC or 8 kHz CVSD, with codec artifacts and host
  voice processing that may hurt weak-signal decoding.

At the same time the maintainer added requirements:

- The radio side must support radios with a **built-in USB sound card and
  USB-serial chip** (often behind a hub inside the radio), radios with a
  **built-in USB-serial chip but analog audio**, and radios with **RS-232,
  3.3 V logic or Icom CI-V** serial and analog audio.
- The host side must work over **Bluetooth LE** and over a **wired USB-C**
  connection, with **the same radio-side support in both**.

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **A. Keep dual mode, ESP32-WROOM-32E + MAX3421E** | Native COM port and headset on four OSes, no host software | Single chip vendor with no USB; MAX3421E needed, and TinyUSB has no USB-audio host driver **(verify)**; HFP audio quality limits | [ESP-IDF `hfp_hf`](../references/index.md#esp-idf-hfp-hf-readme), FCC ID 2AC7Z-ESP32WROOM32E |
| **B. BLE only, ESP32-S3-MINI-1** | Built-in full-speed USB OTG host covers the radio's USB sound card, USB-serial chip and hub in one part; two I2S ports for the analog codec; low cost, deep stock; Apache-2.0 SDK; audio not limited by HFP | No native COM port or audio device on any OS: needs host software or a USB dongle; app-level only on iOS and Android | [Datasheet v1.7](../references/index.md#esp32s3-mini1-ds), FCC ID [2AC7Z-ESPS3MINI1](../references/index.md#fcc-2ac7z-esps3mini1), [`espressif/esp-usb`](https://github.com/espressif/esp-usb) |
| C. BLE only, Raytac MDBT50Q (nRF52840) | Strong BLE radio, Nordic/Zephyr stack, modular FCC grant (SH6MDBT50Q) | USB device only: needs a MAX3421E and a USB-audio host driver written from scratch; about 2.5× the module cost | [Raytac FCC grant](../references/index.md#raytac-mdbt50q-fcc) |
| D. BLE only, budget modules (Ai-Thinker TB-03F, Ai-WB2, WCH BLE-SER) | Under $2 | Weak CPUs, no USB host, no FCC ID confirmed | LCSC listings, 2026-09-24 |

Module prices and stock (LCSC, 2026-09-24, qty 100): ESP32-S3-MINI-1-N8
$3.34, 7,313 in stock; MDBT50Q $7.71–8.53, not stocked at LCSC. A dated check
at a second distributor is still required (#7).

## Decision

**Option B, plus a wired USB-C host link.** Revision A uses the **Espressif
ESP32-S3-MINI-1** (primary ordering code `-N8`; `-N4R2` if PSRAM is needed;
`-1U` variants only with an antenna covered by Espressif's grant), with two host
links and one radio side.

### Host links

- **Bluetooth LE:** phones, tablets and computers, through apps (or host
  software) that implement the protocol ([`protocol/`](../../protocol/)).
- **Wired USB-C:** the device is a USB device to the computer, phone or tablet,
  using only standard classes with built-in OS drivers: USB CDC-ACM serial
  (RTS/DTR carried natively) and USB Audio Class.

The device selects wired mode when a USB host enumerates it on the USB-C port,
and Bluetooth mode otherwise (a USB-C charger alone doesn't count). A setting
can force either mode. The Bluetooth radio is off in wired mode.

### Radio side (identical in both modes)

| Radio has | Bluetooth mode | Wired USB-C mode: the computer sees |
|---|---|---|
| USB-serial chip **and** USB sound card | ESP32-S3 is USB host to both, bridged to the BLE stream | The **radio's own** USB-serial and sound card, through the on-board hub (the device acts as a USB bridge) |
| USB-serial chip, analog audio | ESP32-S3 is USB host to the serial chip; audio via the codec | The **radio's own** USB-serial through the hub, plus the **device's** USB sound card (codec) |
| RS-232, 3.3 V logic or CI-V serial, analog audio | SERIAL jack in the selected mode; audio via the codec | The **device's** USB serial port (bridged to the SERIAL jack) and USB sound card (codec) |

The device always adds its own USB serial port in wired mode for configuration
and for PTT on the AUDIO jack (keyed by RTS/DTR or protocol command), so
PTT-line keying works with every radio type.

### USB topology

The ESP32-S3 has one USB OTG controller, which can be a host **or** a device.
A 2-port USB 2.0 hub and two USB 2.0 switches make both modes work:

```text
                     ┌────────── wired mode ──────────┐
USB-C (to computer) ─┤ hub upstream                   │
                     │ hub port 1 ──┐                  │
                     │ hub port 2 ──┼─┐                │
                     └──────────────┼─┼────────────────┘
                                    │ │
radio USB-A port ── switch S1 ──────┘ │   (S1: hub port 1 | direct link)
ESP32-S3 USB OTG ── switch S2 ────────┘   (S2: hub port 2 | direct link)
                    S1 ◄── direct link ──► S2   (Bluetooth mode: ESP32-S3 is host to the radio)
```

- **Wired mode:** radio port → hub port 1; ESP32-S3 (as USB device) → hub port 2.
- **Bluetooth mode:** the switches join the radio port directly to the ESP32-S3
  (as USB host). The hub is idle.
- Candidate parts (LCSC, 2026-09-24): hub Microchip USB2422 ($1.47), alternates
  Genesys GL850G and WCH CH334; switches TI TS3USB221A ($0.21 each), alternate
  onsemi FSUSB42.

## Consequences

- `constraints.md` §2 changes from dual mode to Bluetooth LE plus wired USB-C;
  §3, §6, §7 and §8 add the USB routing, isolation and power rules. HFP, SPP and
  RFCOMM are no longer requirements.
- **Desktops use wired USB-C** for a native serial port and sound card with no
  drivers. Over Bluetooth, hosts need apps or host software that implement the
  protocol; a desktop bridge is a low-priority follow-up, and a separate USB
  dongle is no longer needed. User documentation must explain both modes.
- The **BLE audio stream** (previously optional, #17) is the primary Bluetooth
  audio path. Budget: 12 kHz / 16-bit mono is 192 kbit/s in one direction at a
  time, within BLE 2M PHY; LC3 (Google `liblc3`, Apache-2.0) is the fallback if
  measured throughput to iOS is too low **(verify)**.
- **Two USB stacks:** Bluetooth mode uses the `esp-usb` host stack; wired mode
  uses a USB device stack (TinyUSB via Espressif's `esp_tinyusb`; licenses to be
  confirmed in `THIRD_PARTY.md` when pulled in). Switching modes restarts the USB
  stack.
- **Rate matching:** a radio USB sound card runs on its own clock, and so does
  a computer's USB audio stream; the firmware matches rates wherever two clock
  domains meet.
- **Isolation:** in wired mode the computer's ground reaches the device.
  Isolation of the AUDIO and SERIAL jacks (transformers, isolated PTT, digital
  isolators) is therefore **mandatory whenever the USB-C data link is used**, on
  both variants. The radio USB port is not isolated by default, like a direct
  USB cable; a full-speed USB isolator (ADuM4160-class) is a fitting option,
  recommended for variant M.
- **Serial mode switching** must tolerate RS-232 levels on every path. Modes are
  selected in firmware through high-voltage analog switches (TMUX6219-class) or
  signal relays, and an RS-232 transceiver whose drivers go high-impedance when
  disabled (MAX3243E-class, which also gives three drivers for TxD, RTS and DTR).
- **USB pins:** the ESP32-S3's USB OTG and USB-Serial-JTAG share one PHY, so
  programming and logs use a UART0 header. Firmware updates in wired mode can
  also use USB DFU.
- The **MAX3421E is removed** from the parts list. The VBUS switch to the radio
  (TPS2553-class) stays.
- **Power:** the module peaks at about 340 mA during BLE TX at +20 dBm
  (datasheet). A USB-C host without USB PD may supply only 500 mA at 5 V, which
  must cover the device, the hub and the radio's USB VBUS draw (#5) in wired mode.
- **Single vendor remains:** the design depends on Espressif. The MDBT50Q is the
  fallback module, at the cost of a USB host controller and a USB-audio host
  driver.
- Issues affected: #4, #5, #6, #7, #9, #15, #16, #17, and the roadmap #22.
  New issues: firmware USB host (Bluetooth mode); firmware USB device and mode
  switching (wired mode); USB routing hardware; host bridge software (later).

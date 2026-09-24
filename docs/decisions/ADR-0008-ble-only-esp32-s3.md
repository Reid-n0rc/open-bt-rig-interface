<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0008: Bluetooth LE-only host link on the ESP32-S3-MINI-1, with analog and USB radio audio

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

At the same time the maintainer added a requirement: the interface must support
**both** radios with a **built-in USB sound card** (USB Audio Class device plus
USB-serial chip, often behind a hub inside the radio) **and** radios that need
**analog** audio in and out.

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

**Option B.** Revision A uses a **Bluetooth LE-only** host link on the
**Espressif ESP32-S3-MINI-1** (primary ordering code `-N8`; `-N4R2` if PSRAM is
needed; `-1U` variants only with an antenna covered by Espressif's grant).

The device supports **both radio audio paths** on every board:

- **Analog:** audio codec over I2S, with isolation transformers (#8).
- **USB:** the ESP32-S3 acts as USB host to the radio's own USB port, using the
  USB Audio Class 1.0 host driver (`usb_host_uac`), the CP210x/FTDI/CH34x/CDC-ACM
  serial drivers, and external hub support (`USB_HOST_HUBS_SUPPORTED`), all
  Apache-2.0 in `espressif/esp-usb`.

Firmware selects USB audio when a radio sound card enumerates, with a manual
override in the settings.

**Host side (open):** how desktops get a serial port and an audio device is not
decided here. The two candidates are host bridge software (serial via a
`ble-serial`-style bridge or a signed virtual COM driver; virtual audio device
per OS) and a USB dongle that presents USB CDC-ACM + USB Audio Class to the
computer. A follow-up issue compares them.

## Consequences

- `constraints.md` §2 changes from dual mode to BLE only; §7 and §8 add the USB
  audio path. HFP, SPP and RFCOMM are no longer requirements.
- **Host software becomes part of the project.** Windows, macOS and Linux need a
  bridge or a dongle; iOS and Android need apps that implement the protocol
  ([`protocol/`](../../protocol/)). User documentation must say this plainly.
- The **BLE audio stream** (previously optional, #17) becomes the primary audio
  path. Budget: 12 kHz / 16-bit mono is 192 kbit/s in one direction at a time,
  within BLE 2M PHY; LC3 (Google `liblc3`, Apache-2.0) is the fallback if
  measured throughput to iOS is too low **(verify)**.
- **Rate matching:** the radio's USB sound card runs on its own clock, so the
  firmware must match rates between it and the BLE stream.
- **USB pins:** the ESP32-S3's USB OTG and USB-Serial-JTAG share one PHY. When the
  port is the radio's USB host, flashing and logs use UART0 instead, so the board
  needs a UART header or bridge for programming.
- The **MAX3421E is removed** from the parts list. The VBUS switch to the radio
  (TPS2553-class) stays.
- Power budget: the module peaks at about 340 mA during BLE TX at +20 dBm
  (datasheet), which replaces the dual-mode estimate in §3.4.
- **Single vendor remains:** the design depends on Espressif. The MDBT50Q is the
  fallback module, at the cost of a USB host controller and a USB-audio host
  driver.
- Issues affected: #4, #5, #6, #7, #9, #15, #16, #17, and the roadmap #22.
  New issues: firmware USB host (audio, serial, hub, rate matching); host
  bridge per OS; USB dongle study.

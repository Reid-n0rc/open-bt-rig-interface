<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Requirements specification

Issue: [#4](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/4).
Written 2026-09-24.

This document turns [`constraints.md`](constraints.md) into numbered, testable
requirements. It follows the host-link decision in
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md): **Bluetooth
LE and wired USB-C** host links, no Bluetooth Classic, and one radio side for
both. The radio-side connectors come from
[`radio-connectors.md`](radio-connectors.md), and PCB and passive-component
rules from [`pcb-fabrication.md`](pcb-fabrication.md).

Where this document and `constraints.md` disagree, `constraints.md` wins until
this document is corrected. Parts, modules and circuits are chosen in the
research and decision issues, not here. The protocol byte format belongs to the
protocol spec ([#13](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/13)).

## How to read this document

Each requirement has:

- **ID:** `REQ-<area>-NNN`. IDs are never reused or renumbered. A dropped
  requirement stays in the table, marked *withdrawn*.
- **Requirement:** one "shall" statement. "Should" marks a preference, not a
  requirement.
- **Rationale:** why it exists, with its source.
- **Verification (Ver.):** **T** test, **A** analysis, **I** inspection,
  **D** demonstration.
- **Status:**
  - `draft`: stated, not yet confirmed by a merged design or test;
  - `verify`: contains a value or fact that a named research issue must confirm
    or correct first;
  - `confirmed`: settled project policy (for example in `AGENTS.md`) or a
    value confirmed by an accepted ADR or a recorded test.
- **Trace:** `design / verification` issues, as `#N` numbers on GitHub. "Bring-up"
  and "Compliance" mean the Phase 6 issues in the [roadmap](../roadmap.md),
  which are not created yet.

Areas: `GEN` overview, `HOST` host platforms, `CAT` serial, `PTT`, `AUD`
audio, `RIF` radio interfaces, `ISO` isolation, `PWR` power, `REG` regulatory,
`EMC` RF environment and EMC, `TIM` timing, `FW` firmware, `MECH`
mechanical, `ENV` environmental, `MFG` manufacturing and sourcing, `TOOL`
tooling.

## 1. Scope and product overview

The device is an interface between one amateur-radio transceiver and one host
(phone, tablet or computer). It carries:

- **CAT serial**, as a transparent byte stream;
- **PTT**, from a CAT command, from RTS/DTR, or from a protocol command, to a
  PTT line or the radio's own USB-serial chip;
- **audio** in both directions: RX audio to the host and TX audio to the radio
  (one direction at a time over Bluetooth LE, §5).

Host links: **Bluetooth LE** with this project's versioned protocol, and
**wired USB-C** with standard USB classes (§2). Radio side: two 3.5 mm TRRS
jacks (AUDIO and SERIAL) and a USB-A host port (§6).

**Variants** (revision A):

- **R:** powered from the radio's accessory DC or USB-C.
- **M:** mobile/automotive 12 V, harsh environment.

*Future note:* a small USB-host-only build (**U**) has been suggested. It is not
part of revision A, and no requirement here depends on it. The variants and
board strategy are decided in [#12](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/12) (ADR-0006).

> **Background.** FT8AF's iOS work
> ([patrickrb/FT8AF#840](https://github.com/patrickrb/FT8AF/issues/840))
> inspired this project. FT8AF is only one of many possible clients; nothing in
> this document is specific to it.

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-GEN-001 | The device shall carry CAT serial, PTT and audio between one radio and one host. | Core purpose ([constraints §1](constraints.md#1-what-the-device-is)). | D | draft | #15, #16 / #18, bring-up |
| REQ-GEN-002 | The device shall provide two host links: Bluetooth LE and wired USB-C. | ADR-0008; [constraints §2](constraints.md#2-host-compatibility). | D | draft | #7, #13, #44 / #6, #18 |
| REQ-GEN-003 | The device shall support, in both host modes, radios with (a) a USB-serial chip and a USB sound card, (b) a USB-serial chip and analog audio, and (c) RS-232, 3.3 V logic or CI-V serial and analog audio. | Radio-type coverage ([constraints §7](constraints.md#7-radio-interfaces), ADR-0008). | T | draft | #9, #43, #44 / #5, bring-up |
| REQ-GEN-004 | The radio-side behavior (serial modes, audio paths, PTT outputs, USB host support) shall be the same in Bluetooth mode and wired mode, except where §2 routes the radio's own USB chips directly to the host. | One radio side for both host links (ADR-0008). | T | draft | #9, #43, #44 / bring-up |
| REQ-GEN-005 | The device's functions and protocol shall not depend on any particular host application; any host software that implements the published protocol shall have access to every function. | App-neutral project ([`AGENTS.md`](../../AGENTS.md), [`GOVERNANCE.md`](../../GOVERNANCE.md)). | I | confirmed | #13 / #13 |
| REQ-GEN-006 | Variants R and M shall share one core design; whether they are separate boards or one board with fitting options shall be a recorded decision. | [constraints §13](constraints.md#13-tooling-and-documentation). | I | draft | #12 / #12 |

## 2. Host platforms

The device has no Bluetooth Classic (no SPP, HFP or RFCOMM), per ADR-0008. What
each host gets:

| Host | Wired USB-C: serial | Wired USB-C: audio | Bluetooth LE: serial and audio | What the user sees |
|---|---|---|---|---|
| Windows, macOS, Linux | Native serial ports (USB CDC-ACM, plus the radio's own USB-serial chip where present); no drivers | Native sound card (USB Audio Class; the radio's own where present) | Apps or host software that implement the protocol | Wired: standard COM/tty ports and a sound card, usable by any radio software. Bluetooth: only inside protocol-aware software. |
| Android | Through apps (USB CDC-ACM via the USB host API); no system serial port | Native sound card | Apps that implement the protocol | Wired: a sound card, and serial inside apps. Bluetooth: only inside protocol-aware apps. |
| iOS / iPadOS | **None: apps have no access to USB serial.** Instead, apps use the protocol over the device's USB network interface ([ADR-0007](../decisions/ADR-0007-protocol.md)) | Native sound card (USB-C devices) | Apps that implement the protocol | Wired: a sound card, plus PTT, configuration and (with SERIAL-jack radios) CAT inside protocol-aware apps. Bluetooth: CAT, PTT and audio inside protocol-aware apps. |

Platform limitations, stated plainly:

- **No OS shows a Bluetooth LE device as a serial port or audio device
  natively.** Over Bluetooth, every host needs an app or host software that
  implements the protocol. A desktop bridge is a later follow-up
  ([#45](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/45)).
- **iOS and iPadOS apps can't use USB serial.** In wired mode they reach the
  device through its USB network interface instead (maintainer decision
  2026-09-24, [ADR-0007](../decisions/ADR-0007-protocol.md)). With a radio
  whose serial is USB, the radio's chip sits behind the hub, so an iOS/iPadOS
  host gets no CAT in wired mode; Bluetooth mode has full CAT. This is a
  **known limitation**, accepted by the maintainer on 2026-09-24.
- Per-OS details (UAC version, CDC-ACM support, BLE throughput) are confirmed in
  [#6](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/6).

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-HOST-001 | In wired mode, the device shall enumerate on USB-C as a USB device using only standard classes (USB CDC-ACM serial and USB Audio Class), with no driver installation on Windows, macOS and Linux. | Native serial port and sound card on desktops ([constraints §2](constraints.md#2-host-compatibility)). | T | verify | #44 / #6, #18 |
| REQ-HOST-002 | In wired mode on Android, the device's sound card shall work natively, and its serial ports shall be usable by apps through USB CDC-ACM. | constraints §2 table. | T | verify | #44 / #6, #18 |
| REQ-HOST-003 | In wired mode on iOS/iPadOS devices with USB-C, the device's sound card shall work natively, and apps shall reach PTT, configuration and SERIAL-jack CAT through the protocol over the USB network interface (REQ-HOST-013). Known limitation: with radios whose serial is USB, iOS/iPadOS get no CAT in wired mode. | constraints §2 table; [ADR-0007](../decisions/ADR-0007-protocol.md) (limitation accepted by the maintainer, 2026-09-24). NCM on iOS/iPadOS is not yet confirmed. | T | verify | #13, #44 / #6, #18 |
| REQ-HOST-004 | Over Bluetooth LE, every function (CAT, PTT, RTS/DTR, audio, configuration, status) shall be available through the versioned protocol on iOS/iPadOS, macOS, Android, Windows and Linux. | Bluetooth link for all five OSes (ADR-0008). | T | verify | #13, #15, #16 / #6, #18 |
| REQ-HOST-005 | User documentation shall state plainly that no OS shows a Bluetooth LE device as a serial port or audio device natively, that iOS/iPadOS apps have no access to USB serial (and so use the USB network interface when wired), and what each host sees in each mode (table above). | Users must know which mode fits their host ([constraints §2](constraints.md#2-host-compatibility)). | I | draft | #13, #45 / #6 |
| REQ-HOST-006 | The device shall select wired mode when a USB host enumerates it on USB-C, and Bluetooth mode otherwise; a USB-C power source with no data connection shall leave it in Bluetooth mode. | Automatic mode selection (constraints §2, ADR-0008). | T | draft | #44 / #44 |
| REQ-HOST-007 | A stored setting shall force either wired or Bluetooth mode. | constraints §2. | T | draft | #44 / #44 |
| REQ-HOST-008 | The Bluetooth radio shall be off in wired mode. | constraints §2. | T | draft | #44 / #44 |
| REQ-HOST-009 | In wired mode, a radio's own USB-serial chip and USB sound card shall appear to the host directly, through an on-board USB hub. | Radio chips usable with their own drivers and software (constraints §2, ADR-0008). | T | draft | #9, #44 / #18, bring-up |
| REQ-HOST-010 | In wired mode, the device shall enumerate the USB function set for the attached radio type given in the protocol spec: its own USB sound card when the radio's audio is analog; a USB network interface (REQ-HOST-013) and a USB serial port for configuration and AUDIO-jack PTT with USB-serial + USB-audio radios; the network interface with USB-serial + analog-audio radios; and, for SERIAL-jack radios, either a USB serial port bridged to the SERIAL jack (the default) or the network interface, chosen by a setting. | Every radio type works wired within the ESP32-S3's endpoint limit (at most 4 IN endpoints besides endpoint 0) ([constraints §2](constraints.md#2-host-compatibility), [ADR-0007](../decisions/ADR-0007-protocol.md), [protocol §14.1](../../protocol/SPEC.md#141-usb-functions-and-the-endpoint-budget)). Changed from "always a USB serial port for configuration", which doesn't fit the endpoint budget. | T | verify | #13, #44 / #18 |
| REQ-HOST-011 | The Bluetooth LE link shall use the 2M PHY where the host supports it. | Audio throughput ([constraints §2](constraints.md#2-host-compatibility)). | T | verify | #13, #16 / #6 |
| REQ-HOST-012 | The device shall not use Bluetooth Classic (SPP, HFP, RFCOMM), and shall not need Wi-Fi, LE Audio, or features specific to one host application or library (for example Hamlib or FLrig). | Scope (constraints §2 "Not required", ADR-0008). | I | draft | #7, #13 / #13 |
| REQ-HOST-013 | In wired mode, the device shall offer a driverless USB network interface (CDC-NCM) carrying the protocol over TCP, with DHCP and DNS-SD discovery as in the protocol spec, whenever its function set includes it (REQ-HOST-010). | Hosts without USB serial (iPhone, iPad) get CAT, PTT and configuration over wired USB-C (maintainer decision 2026-09-24, [ADR-0007](../decisions/ADR-0007-protocol.md), extending ADR-0008). NCM support on iOS/iPadOS, macOS and Android isn't confirmed; Windows 10 has no in-box NCM driver. | T | verify | #13, #44 / #18 |
| REQ-HOST-014 | The device shall require LE Secure Connections bonding (no LE legacy pairing), give unbonded centrals only the Info characteristic, and accept new Bluetooth LE bonds only during a pairing window opened by a local action on the device (power-on or a pairing button), which closes after a configurable time (default 120 s), after the first new bond, or on entering wired mode; outside the window only bonded hosts shall reach the protocol. No protocol message, wired control port or USB network shall be able to open the window. The window's state shall be readable before bonding and in the device status. | "Just Works" pairing has no protection against an active attacker; limiting it to a local action keeps strangers from keying the transmitter (maintainer decision 2026-09-24, [ADR-0007](../decisions/ADR-0007-protocol.md), [protocol §13.5](../../protocol/SPEC.md#135-security-and-pairing)). The trigger hardware is not yet decided. | T | verify | #9, #13, #14 / #14 |
| REQ-HOST-015 | A wired host (USB-network TCP or CDC-ACM control port) shall be approved by the same local action on the device that opens the pairing window before it can use the protocol; the host shall identify itself with a random 128-bit token it generates, the device shall remember approved hosts, and there shall be no default password. Until approval the device shall answer only identification messages and refuse the rest as not authorized. | Security to the Cyber Resilience Act level (maintainer decision 2026-09-25, [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64), [ADR-0007](../decisions/ADR-0007-protocol.md), [protocol §15](../../protocol/SPEC.md#15-security)). Encrypting the TCP transport (EN 18031-1 level) is planned in #64. | T | draft | #13, #44, #64 / #64 |
| REQ-HOST-016 | An authorized host shall be able to list and remove Bluetooth bonds and approved wired hosts, and to factory-reset the device (erasing configuration, bonds and approved hosts, PTT off); a factory reset shall also be possible by a local action on the device. | CRA-level security (#64); a user must be able to revoke access. The local action is set in #14 and the hardware. | T | draft | #9, #13, #14, #64 / #64 |
| REQ-HOST-017 | The device's native USB serial ports (data and RTS/DTR) and the radio's own USB devices behind the hub shall work without approval, and an approved host shall be able to restrict them with a setting: native RTS/DTR never key PTT; native serial ports not enumerated; and, where the hardware can, the radio USB port isolated from the host. Changing the setting shall never assert PTT. | A physical cable connection counts as the owner's consent (maintainer decision 2026-09-25, [ADR-0007](../decisions/ADR-0007-protocol.md), [protocol §15.2](../../protocol/SPEC.md#152-wired-hosts-usb-network-and-cdc-acm-control-port)). Whether the radio-port switch can isolate is confirmed in #9. | T | draft | #9, #13, #44, #64 / #64 |

## 3. Serial / CAT

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-CAT-001 | The device shall pass CAT bytes unchanged in both directions between the host and the radio's serial interface. It shall not parse, filter or generate radio commands. | Transparent CAT; no radio-specific logic ([constraints §1](constraints.md#1-what-the-device-is), [§10](constraints.md#10-firmware)). | T | draft | #15 / #14 |
| REQ-CAT-002 | On the SERIAL jack, the device shall support baud rates from 4800 to 115200, including 4800, 9600, 19200, 38400, 57600 and 115200. | [constraints §7](constraints.md#7-radio-interfaces). | T | draft | #9, #15 / bring-up |
| REQ-CAT-003 | On the SERIAL jack, the device shall use 8 data bits with configurable parity (none, even, odd) and stop bits (1 or 2); default 8N1. | Some radios' CAT settings differ from 8N1; per-radio values come from #5. | T | verify | #15 / #5 |
| REQ-CAT-004 | In Bluetooth mode with a radio USB-serial chip, the device shall apply the host's requested line settings (baud rate, format, RTS/DTR) to that chip. In wired mode, the host sets the radio's chip directly. | The radio's USB-serial chip needs line coding from its host ([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)). | T | draft | #43 / #43, bring-up |
| REQ-CAT-005 | The device shall not drop or reorder CAT bytes under sustained traffic at 115200 baud in either direction. It shall apply flow control toward the host (protocol flow control over Bluetooth LE; USB flow control in wired mode) and buffer toward the radio. | CAT reliability. The SERIAL jack has no RTS/CTS contacts ([radio-connectors](radio-connectors.md#serial-jack-35-mm-trrs)), so hardware flow control to the radio isn't available. | T | draft | #13, #15 / #15 |
| REQ-CAT-006 | CAT latency (byte in to byte out, each direction, each host link) shall be measured and documented; the target value is **TBD**. | Host software timeouts depend on it. No sourced target exists yet. | T | verify | #15 / #6, #18 |
| REQ-CAT-007 | In CI-V mode, the device shall handle its own transmit echo and bus collisions on the single-wire bus; the exact behavior is defined in the CAT design. | CI-V is a shared open-drain bus ([radio-connectors](radio-connectors.md#serial-jack-35-mm-trrs)). | T | verify | #9, #15 / #5, bring-up |
| REQ-CAT-008 | The SERIAL-jack mode shall be selected in firmware, stored in the device configuration, and set over the protocol or the wired control port. | Replaces solder jumpers ([radio-connectors](radio-connectors.md#serial-jack-35-mm-trrs)). | T | draft | #9, #15 / #15 |

## 4. PTT and fail-safe

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-PTT-001 | The device shall accept PTT from three sources: (a) a CAT command from the host, passed through unchanged (REQ-CAT-001); (b) the host's RTS/DTR; (c) a protocol PTT command. | [constraints §2, §7](constraints.md#7-radio-interfaces). | T | draft | #13, #15 / #15, #18 |
| REQ-PTT-002 | Host RTS/DTR shall be native CDC-ACM line state on the device's wired serial ports, and protocol messages over Bluetooth LE and the USB network interface. | constraints §2. | T | draft | #13, #15, #44 / #15 |
| REQ-PTT-003 | The device shall map host RTS/DTR, by configuration, to the AUDIO-jack PTT closure, or to RTS/DTR on the radio's USB-serial chip when the radio has one. | [constraints §7](constraints.md#7-radio-interfaces). The SERIAL jack has no RS-232 RTS/DTR contacts, so RS-232-level RTS/DTR outputs are not provided. | T | draft | #15, #43 / #15 |
| REQ-PTT-004 | The PTT output shall be an isolated closure to ground (sleeve) on AUDIO-jack ring 2, closed = keyed, rated for every supported radio's PTT pull-up voltage and current. | [radio-connectors](radio-connectors.md#audio-jack-35-mm-trrs). Radio PTT voltages and currents come from #5. | T | verify | #9 / #5, bring-up |
| REQ-PTT-005 | PTT shall be off at power-on, reset, brownout, watchdog timeout, loss of the host link (Bluetooth disconnect; USB-C disconnect or suspend in wired mode) and host-mode switch. | Fail-safe ([constraints §6](constraints.md#6-safety-and-fail-safe), [`AGENTS.md`](../../AGENTS.md#firmware)). The wired-mode cases extend "disconnect" to the USB-C link. | T | draft | #14, #15, #44 / #15, bring-up |
| REQ-PTT-006 | PTT keyed through the protocol shall drop when the host's keepalive is missing for longer than a timeout set in the protocol spec. | A host that hangs while connected must not hold PTT ([roadmap](../roadmap.md) Phase 3 exit criteria). | T | draft | #13, #15 / #15 |
| REQ-PTT-007 | The firmware shall enforce a maximum continuous TX time from any PTT source, configurable by the user: default 300 s, at least 10 s when enabled, no upper limit, and 0 disables it. On expiry, PTT goes off and the host is notified. | constraints §6; maintainer decision 2026-09-25 ([ADR-0007](../decisions/ADR-0007-protocol.md)). | T | draft | #15 / #15 |
| REQ-PTT-008 | The PTT output shall be off unless actively driven (a pull-down, or an opto/MOSFET that must be driven), so a hung or unpowered MCU cannot key the radio. | Hardware default off (constraints §6). | T, I | draft | #9 / bring-up |
| REQ-PTT-009 | Changing the SERIAL-jack mode, the audio path or the host mode shall never assert PTT. | [radio-connectors](radio-connectors.md#serial-jack-35-mm-trrs). | T | draft | #15, #44 / #15 |
| REQ-PTT-010 | Every PTT fail-safe path shall have a host-run firmware test. | [`AGENTS.md`](../../AGENTS.md#firmware). | I | confirmed | #14, #15 / #14 |
| REQ-PTT-011 | After a firmware lock-up, the internal watchdog shall reset the device within a few seconds, with PTT off during and after the reset, and the device shall report the watchdog reset to the host afterwards. | No external hardware PTT timer (maintainer decision 2026-09-25, [ADR-0003](../decisions/ADR-0003-radio-interface-circuits.md)); the lock-up path must still end in PTT off ([ADR-0007](../decisions/ADR-0007-protocol.md), [protocol §8.5](../../protocol/SPEC.md#85-maximum-tx-time-and-other-fail-safes)). The exact timeout is set in #14. | T | draft | #13, #14, #15 / bring-up |

RF pickup and PTT: see REQ-EMC-001.

## 5. Audio

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-AUD-001 | Every board shall have two radio audio paths: analog (an internal codec at line level on the AUDIO jack) and USB (the radio's built-in USB sound card, USB Audio Class 1.0, through the radio USB port). | [constraints §8](constraints.md#8-audio). | I, T | draft | #8, #16, #43 / bring-up |
| REQ-AUD-002 | The firmware shall use the USB audio path when a radio USB sound card enumerates, and the analog path otherwise, with a manual override. | constraints §8. | T | draft | #16, #43 / #16 |
| REQ-AUD-003 | In wired mode with a radio USB sound card, the host shall use that sound card directly, and the device's own audio path shall be idle. | constraints §8, REQ-HOST-009. | T | draft | #44 / #18 |
| REQ-AUD-004 | Over Bluetooth LE, audio shall be at least 12 kHz / 16-bit mono, one direction at a time (192 kbit/s), over an L2CAP connection-oriented channel or GATT. | [constraints §2](constraints.md#2-host-compatibility). Throughput to each OS, especially iOS, is not yet measured. | T | verify | #13, #16 / #6, #18 |
| REQ-AUD-005 | If raw PCM doesn't fit the measured Bluetooth LE throughput, audio shall use LC3 compression from a permissively licensed implementation. | constraints §2, ADR-0008. | T | verify | #16 / #6 |
| REQ-AUD-006 | In wired mode, the device's sound card shall run at 48 kHz / 16-bit, using USB Audio Class 1 or 2, whichever every target OS supports without drivers. | constraints §2. | T | verify | #44 / #6, #18 |
| REQ-AUD-007 | The internal codec shall run at 48 kHz. | constraints §8. | T | draft | #8, #16 / bring-up |
| REQ-AUD-008 | Where two clock domains meet (radio USB sound card, codec, Bluetooth LE stream, host USB audio), the firmware shall match rates without audible artifacts and without tone shift beyond REQ-AUD-011. | Independent clocks ([constraints §8](constraints.md#8-audio), ADR-0008). | T | draft | #16, #43, #44 / #18 |
| REQ-AUD-009 | The AUDIO-jack RX input shall be line level, AC-coupled and high impedance, with about 19 dB of switchable attenuation (or an equivalent gain range) for hot speaker outputs; its maximum input level is set from the radio table. | [radio-connectors](radio-connectors.md#audio-jack-35-mm-trrs). | T | verify | #8 / #5 |
| REQ-AUD-010 | The AUDIO-jack TX output shall be line level, AC-coupled, up to about 2.5 V peak-to-peak, adjustable down to microphone level. | radio-connectors. | T | draft | #8 / bring-up |
| REQ-AUD-011 | The sample clock shall be accurate to ±50 ppm or better. | No tone shift (constraints §8). | T | draft | #8 / bring-up |
| REQ-AUD-012 | ADC SNR shall be at least 90 dB (A-weighted). | constraints §8. Depends on the codec choice. | T | verify | #8 / #8, bring-up |
| REQ-AUD-013 | The device audio path shall have no automatic gain control, noise suppression or voice processing. | Weak-signal decoding needs unprocessed audio (constraints §8). | I, T | draft | #8, #16 / #18 |
| REQ-AUD-014 | TX audio level shall be adjustable through the protocol and the wired control port. | constraints §8. | T | draft | #16 / #16 |
| REQ-AUD-015 | End-to-end audio latency shall be fixed (not drifting within a session) and measured, per path and host link, with results documented. | constraints §8. | T | draft | #16 / #18 |

## 6. Radio interfaces

Connectors and pinouts: [`radio-connectors.md`](radio-connectors.md).

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-RIF-001 | The radio side shall have two 3.5 mm TRRS jacks (AUDIO and SERIAL) and a USB-A host port. | [constraints §7](constraints.md#7-radio-interfaces). | I | draft | #9 / #21 |
| REQ-RIF-002 | The AUDIO and SERIAL jack pinouts shall be exactly as in [`radio-connectors.md`](radio-connectors.md), so existing cables made for that convention work unchanged, and plain TRS plugs work in the SERIAL jack. | Cable compatibility. | I, T | draft | #9 / bring-up |
| REQ-RIF-003 | The SERIAL jack shall support four firmware-selected modes: 3.3 V logic (5 V-tolerant input), RS-232 levels, Icom CI-V (single-wire open-drain bus) and 3.3 V logic with a 3.3 V output on ring 2 (current-limited to about 20 mA, short-circuit protected). | constraints §7, radio-connectors. | T | draft | #9, #15 / bring-up |
| REQ-RIF-004 | The SERIAL jack shall default to 3.3 V logic at power-on. | Safe with every cable. | T | draft | #9, #15 / #15 |
| REQ-RIF-005 | Every SERIAL-jack mode shall survive any cable: RS-232 levels up to ±15 V on any contact, in any selected mode, shall not damage the logic, CI-V or RS-232 paths, and RS-232 drivers shall be high-impedance when disabled. | constraints §7, radio-connectors. | T, A | draft | #9 / bring-up |
| REQ-RIF-006 | Through the radio USB port, the device shall be USB host (full speed, 12 Mbit/s) to the radio's USB-serial chip (CP210x including dual-port CP2105, FTDI, CH34x, CDC-ACM) and its USB Audio Class 1.0 sound card, including through a hub inside the radio. | constraints §7. Which chips each radio uses, and USB host driver support, are confirmed per radio. | T | verify | #9, #43 / #5, bring-up |
| REQ-RIF-007 | *Withdrawn (2026-09-24):* the device no longer supplies VBUS to the radio; see REQ-RIF-011. | Maintainer decision, [ADR-0003](../decisions/ADR-0003-radio-interface-circuits.md). | — | withdrawn | #9 |
| REQ-RIF-008 | In wired mode, the radio USB port shall be routed to the on-board USB hub; in Bluetooth mode, it shall be joined directly to the device's USB host. | ADR-0008 USB topology. | T | draft | #9, #44 / #44 |
| REQ-RIF-009 | The host-side USB-C port shall be a receptacle with 5.1 kΩ Rd on CC1 and CC2 (no USB PD), connected to the hub's upstream port. | [radio-connectors](radio-connectors.md#usb-c-port-host-link-and-power). | I | draft | #9, #11 / #21 |
| REQ-RIF-010 | Radios shall be supported through per-radio cables or harnesses (mini-DIN, 3.5 mm, DB9, USB), not per-radio boards. | constraints §7. | I | draft | #5, #9 / #5 |
| REQ-RIF-011 | As fitted by default, the device shall never supply power to a radio through the radio USB port, in either host mode: the port's VBUS pin shall connect to no device rail, to the USB-C VBUS or to hub port power, and no current shall flow from the device into the radio's VBUS or from a radio's VBUS into the device. Test-only DNP footprints (a sense-only feed and a 0 Ω bypass) are allowed on that net, but no production build shall fit them. | Maintainer decision ([constraints §3.1](constraints.md#31-sources), [ADR-0003](../decisions/ADR-0003-radio-interface-circuits.md)). | I, T | draft | #9 / #21, bring-up |

## 6a. Isolation

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-ISO-001 | The AUDIO and SERIAL jacks shall be galvanically isolated from the rest of the device (transformer-coupled audio, isolated PTT, digital isolators on the serial lines) on variant M, and on every variant whenever the USB-C data link is used. | The USB-C link brings the computer's ground to the device ([constraints §6](constraints.md#6-safety-and-fail-safe)). | I, T | draft | #8, #9, #12 / bring-up |
| REQ-ISO-002 | Isolation of the AUDIO and SERIAL jacks shall be optional only for variant R used over Bluetooth and powered from the radio; the board strategy shall record how this option is built and how a user can tell which build they have. | constraints §6. | I | draft | #12 / #12 |
| REQ-ISO-003 | The radio USB port shall be non-isolated by default, with a full-speed USB isolator (ADuM4160-class) as a fitting option, recommended for variant M. | constraints §6. | I | draft | #9, #10 / #21 |

## 7. Power

### 7.1 Sources and budget (all variants)

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-PWR-001 | Variant R shall run from a radio accessory DC supply (13.8 V nominal, 11–15 V range) with reverse-polarity and TVS protection, drawing no more than each supported radio's accessory-pin current limit. | [constraints §3.1, §3.3](constraints.md#33-radio-sourced--usb-c-input-variant-r). Pin, voltage and limit per radio are not yet known. | T | verify | #11 / #5 |
| REQ-PWR-002 | Every variant shall run from a USB-C 5 V sink (phone charger, power bank or computer); USB PD is not required. | constraints §3.1, §3.3. | T | draft | #11 / bring-up |
| REQ-PWR-003 | The device's total draw shall be about 1.5 W typical and 3 W peak or less, confirmed by a power budget per variant. | [constraints §3.4](constraints.md#34-power-budget-verify). Load estimates are not yet confirmed. | A, T | verify | #10, #11 / bring-up |
| REQ-PWR-004 | In wired mode, the device and hub draw shall fit within the current the USB-C host offers: 500 mA at 5 V by default, or 1.5 A / 3 A when advertised on CC. On overcurrent the device shall report it to the host rather than brown out. | [constraints §3.4](constraints.md#34-power-budget-verify). | T, A | verify | #11, #44 / #5, bring-up |
| REQ-PWR-005 | A brownout on any input shall leave PTT off (REQ-PTT-005). | constraints §3.2, §6. | T | draft | #10, #11 / bring-up |
| REQ-PWR-018 | The device shall power itself down a configurable delay after the radio (or, on variant M, the ignition) turns off: default 30 s, range 5–3600 s or 0 = never, set with the protocol's `POWER_DOWN_DELAY_S` key. The delay shall run only while no USB host is connected on USB-C; the device stays awake in wired mode. It shall end any session and turn PTT off before powering down. | Maintainer decisions 2026-09-25; [protocol §6.1](../../protocol/SPEC.md#61-keys). With 0, variant M can exceed REQ-PWR-016's off-state drain target, as the user's choice. How the device detects "radio off" is set in #10/#11. | T | verify | #10, #11, #13 / bring-up |

### 7.2 Variant M automotive 12 V input

Levels come from ISO 16750-2 and ISO 7637-2 for 12 V systems. The exact levels
and editions are confirmed in [#10](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/10).

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-PWR-010 | Variant M shall operate normally from 9 to 16 V. | [constraints §3.2](constraints.md#32-automotive-12-v-input-variant-m). | T | verify | #10 / #10, bring-up |
| REQ-PWR-011 | During cold crank, variant M shall operate down to about 6 V (4.5 V desirable), or brown out safely with PTT off. | constraints §3.2. | T | verify | #10 / bring-up |
| REQ-PWR-012 | Variant M shall survive an unsuppressed load dump up to about 101 V for 40–400 ms, and a suppressed (centrally clamped) load dump of about 35 V. | constraints §3.2. | T | verify | #10 / bring-up |
| REQ-PWR-013 | Variant M shall survive −14 V reverse battery for 60 s without damage. | constraints §3.2. | T | verify | #10 / bring-up |
| REQ-PWR-014 | Variant M shall survive a 26 V jump start for 60 s (ISO 16750-2:2023). | constraints §3.2; [ADR-0004](../decisions/ADR-0004-power-automotive.md). | T | verify | #10 / bring-up |
| REQ-PWR-015 | Variant M shall survive ISO 7637-2 pulse 1 (about −150 V), pulse 2a (about +112 V) and pulses 3a/3b (about −220 V / +150 V). | constraints §3.2. | T | verify | #10 / bring-up |
| REQ-PWR-016 | Variant M shall draw less than 1 mA when off, and power down automatically when the radio or ignition is off. | constraints §3.2. | T | verify | #10 / bring-up |
| REQ-PWR-017 | Variant M's power front end shall use a reverse-polarity protection (ideal-diode or controller), load-dump protection (surge stopper and/or TVS), an AEC-Q100 wide-input buck converter, and an input common-mode choke plus pi filter. | constraints §3.2 design guidance. | I | draft | #10 / #10 |

## 8. Regulatory

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-REG-001 | The device shall use an FCC-certified radio module, with its FCC ID recorded (candidate: [2AC7Z-ESPS3MINI1](../references/index.md#fcc-2ac7z-esps3mini1), ESP32-S3-MINI-1). | [constraints §4](constraints.md#4-regulatory), [`AGENTS.md`](../../AGENTS.md#parts-and-sourcing). | I | verify | #7 / #7 |
| REQ-REG-002 | The board shall follow the module's integration guide exactly: approved antenna type and gain, keep-out area and RF trace layout. | Keeps the modular grant valid (constraints §4). | I | verify | #7, #21 / design review |
| REQ-REG-003 | The end product shall meet FCC Part 15 Subpart B, Class B (unintentional radiator), documented by a Supplier's Declaration of Conformity. | constraints §4. | T | draft | #21 / Compliance |
| REQ-REG-004 | The product shall carry a "Contains FCC ID: …" label. | constraints §4. | I | draft | #19 / #20 |
| REQ-REG-005 | No metal shall cover the module antenna, unless the module is certified with an external antenna. | constraints §4. | I | draft | #19, #21 / #20 |
| REQ-REG-006 | ISED and CE compliance are not required for revision A; any decision that would block them later shall be noted in the relevant ADR. | ISED and CE optional, later (constraints §4). | I | draft | #7, #12 / — |

## 9. RF environment and EMC

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-EMC-001 | PTT shall never assert from RF pickup while the device operates next to an HF transmitter of 100 W or more. | Safety ([constraints §5](constraints.md#5-rf-environment)). | T | draft | #9 / bring-up |
| REQ-EMC-002 | Every jack contact (audio, CAT, PTT) shall have RF filtering (ferrite and RC, or ferrite plus a small capacitor to its sleeve) and ESD protection at the jack. | constraints §5, [radio-connectors](radio-connectors.md#isolation-and-protection). | I | draft | #9 / design review |
| REQ-EMC-003 | The grounding plan shall avoid ground loops between radio, device and host, and shall be documented. | constraints §5. | A, I | draft | #8, #9, #12 / bring-up |
| REQ-EMC-004 | The device shall not raise the connected receiver's noise floor: no switching harmonics or digital noise in the HF bands. | constraints §5. | T | draft | #10, #11 / bring-up |
| REQ-EMC-005 | Switching converters shall use low-EMI conversion (silent-switcher or spread-spectrum), with the switching frequency chosen and filtered so it doesn't land on HF amateur bands. | constraints §3.2, §5. | A | draft | #10, #11 / #10, #11 |
| REQ-EMC-006 | Variant M shall meet CISPR 25 Class 3 or better for conducted and radiated emissions. | constraints §3.2. | T | verify | #10 / Compliance |
| REQ-EMC-007 | Variant M shall withstand ±15 kV air discharge (ISO 10605). | constraints §3.2. | T | verify | #10 / Compliance |

## 10. Timing (optional features)

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-TIM-001 | The device shall synchronize its clock from the host over Bluetooth LE. | Scheduled TX needs UTC ([constraints §9](constraints.md#9-timing)). | T | draft | #13, #17 / #17 |
| REQ-TIM-002 | Scheduled TX shall start within about ±20 ms of the scheduled UTC time. | constraints §9. | T | draft | #17 / #17 |
| REQ-TIM-003 | On-device generation of scheduled tone sequences (FT8/FT4/WSPR/JS8-class modes) shall be an optional firmware module; the core firmware shall build and meet every other requirement without it. | Not part of the core (constraints §9). | I, T | draft | #17 / #14 |

## 11. Firmware

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-FW-001 | The SDK and every firmware dependency shall be under a permissive license (MIT, BSD, Apache-2.0 or similar), with no NDA-only SDK, and each shall be recorded in [`THIRD_PARTY.md`](../../THIRD_PARTY.md). | Keeps the firmware licensable under both the non-commercial and commercial terms ([constraints §10](constraints.md#10-firmware)). | I | confirmed | #14 / #14 |
| REQ-FW-002 | The firmware core shall contain no radio-specific logic. | Transparent CAT (constraints §10). | I | draft | #14, #15 / #14 |
| REQ-FW-003 | Portable logic shall live in `firmware/app/` and be host-tested in `firmware/test/`; SDK glue shall live in `firmware/platform/<sdk>/` behind a small hardware-abstraction interface. | [`AGENTS.md`](../../AGENTS.md#firmware). | I | confirmed | #14 / #14 |
| REQ-FW-004 | The host protocol shall be versioned, with capability discovery; a change shall bump the protocol version and update the golden byte vectors. | constraints §10, [`AGENTS.md`](../../AGENTS.md#protocol). | I, T | draft | #13 / #13 |
| REQ-FW-005 | Configuration shall be possible over Bluetooth LE (protocol) and, in wired mode, over the protocol on the USB network interface or the device's control serial port, whichever the function set includes. | constraints §10, radio-connectors. | T | draft | #13, #44 / #15 |
| REQ-FW-006 | Over-the-air firmware updates are optional; if implemented, the firmware shall accept only signed images. Wired mode may also offer USB DFU. | constraints §10, ADR-0008. | T | draft | #14, #44 / — |
| REQ-FW-007 | Switching between Bluetooth mode (USB host stack) and wired mode (USB device stack) shall restart the USB stack cleanly, with PTT off throughout. | Two USB stacks share one OTG controller (ADR-0008). | T | draft | #43, #44 / #44 |
| REQ-FW-008 | Programming and logs shall be available through a UART0 header, since the USB pins are used for the host links. | ADR-0008 (USB OTG and USB-Serial-JTAG share one PHY). | I | draft | #21 / #21 |

## 12. Mechanical

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-MECH-001 | Each variant shall have a 3D-printed enclosure with parametric CAD source in `hardware/enclosure/`, printable without supports where possible. | [constraints §11](constraints.md#11-environmental-and-mechanical). | I | draft | #19 / #20 |
| REQ-MECH-002 | Enclosure material shall be PETG for variant R, and ASA (or ABS) for variant M. | PLA/PETG soften in a parked car (constraints §11). | I | draft | #19 / #20 |
| REQ-MECH-003 | The enclosure shall respect the module's antenna keep-out. | constraints §4, §11. | I | draft | #19 / #20 |
| REQ-MECH-004 | The enclosure shall provide mounting, cable strain relief, and space for the FCC ID label. | constraints §11. | I, D | draft | #19 / #20 |
| REQ-MECH-005 | Enclosure STL/3MF files shall be generated by CI and attached to releases, not committed. | constraints §11. | I | draft | #19 / #19 |
| REQ-MECH-006 | Variant M shall use vibration-rated, locking connectors for its power input. | constraints §3.2. | I | draft | #10 / #20 |
| REQ-MECH-007 | The PCB silkscreen shall show the project name, variant, `${REVISION}`, `${ISSUE_DATE}`, "Designed by Reid Crowe, N0RC" and the license mark (CC BY-NC-SA 4.0), using text variables; the title-block revision shall match the `rev<X>` folder and the release tag. | [`AGENTS.md`](../../AGENTS.md#hardware-kicad), [constraints §13](constraints.md#13-tooling-and-documentation). | I | confirmed | #21 / #3 |
| REQ-MECH-008 | Each enclosure shall fit within a size target **TBD by maintainer**. | constraints §11. | I | draft | maintainer / #20 |

## 13. Environmental

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-ENV-001 | Variant R shall operate from −20 °C to +60 °C; every part shall be rated for that range. | [constraints §11](constraints.md#11-environmental-and-mechanical). | A, T | draft | #11, #12 / bring-up |
| REQ-ENV-002 | Variant M shall operate from −40 °C to +85 °C; every part shall be rated for that range (AEC-Q100/Q200 where available). | constraints §3.2, §11. | A, T | draft | #10, #12 / bring-up |
| REQ-ENV-003 | The variant M enclosure shall pass a heat soak representative of a parked car without deforming. | constraints §11; roadmap Phase 5 exit criteria. | T | draft | #19 / #20 |

## 14. Manufacturing and sourcing

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-MFG-001 | Boards shall be designed for JLCPCB's standard (low-cost) PCB and assembly process: FR-4 1.6 mm, 1 oz outer copper, lead-free HASL, top-side assembly only. | [pcb-fabrication §1, §4](pcb-fabrication.md#1-board), [constraints §12](constraints.md#12-manufacturing-and-sourcing). | I | draft | #21 / design review |
| REQ-MFG-002 | Boards shall be 2 layers where possible; 4 layers (JLC04161H-7628) only when a reason in pcb-fabrication §2 applies, recorded in the board's design notes. | [pcb-fabrication §2](pcb-fabrication.md#2-two-layers-or-four). | I | draft | #21 / design review |
| REQ-MFG-003 | Board design rules shall be at least the project values in pcb-fabrication §3, set in each board's KiCad setup and enforced by DRC. | [pcb-fabrication §3](pcb-fabrication.md#3-design-rules). | T | draft | #21 / #3 |
| REQ-MFG-004 | High-current and high-voltage nets shall be sized for ≤ 10 °C rise (IPC-2221), with ≥ 0.5 mm clearance on nets that can exceed 50 V. | pcb-fabrication §3. | A | draft | #10, #11 / design review |
| REQ-MFG-005 | Resistors shall be 0402 thick film ±1 % by default, larger only when power, voltage or pulse derating (≤ 50 % of rating) requires it. | [pcb-fabrication §5](pcb-fabrication.md#5-resistors). | I | draft | schematic issues / design review |
| REQ-MFG-006 | Capacitors shall be MLCC by default, with dielectric per pcb-fabrication §6.2 (C0G/NP0 in the audio signal path; never Y5V/Z5U), rated at least 2× the node's nominal DC voltage and never below its worst-case transient, with effective capacitance checked at DC bias. | [pcb-fabrication §6](pcb-fabrication.md#6-capacitors), constraints §12. | I, A | draft | schematic issues / design review |
| REQ-MFG-007 | Parts shall be JLCPCB Basic parts unless no Basic part meets the electrical requirement, with values consolidated to limit unique extended parts. | Assembly cost ([pcb-fabrication §4](pcb-fabrication.md#4-assembly)). | I | draft | parts list / design review |
| REQ-MFG-008 | Each key part shall have at least two distributor sources (for example Digi-Key, Mouser, LCSC) and an active lifecycle, recorded with its alternates in the relevant ADR or BOM notes. | constraints §12, [`AGENTS.md`](../../AGENTS.md#parts-and-sourcing). | I | confirmed | #7, #8, #9, #10, #11 / parts list |
| REQ-MFG-009 | Fabrication outputs shall be generated by CI and attached to releases, not committed. | [`AGENTS.md`](../../AGENTS.md#hardware-kicad). | I | confirmed | #21 / release workflow |
| REQ-MFG-010 | Each variant's BOM cost shall meet a target **TBD by maintainer**. | constraints §12. | A | draft | maintainer / parts list |

## 15. Tooling

| ID | Requirement | Rationale | Ver. | Status | Trace |
|---|---|---|---|---|---|
| REQ-TOOL-001 | Hardware files shall use KiCad ≥ 10.0.6, matching [`KICAD_VERSION`](../../KICAD_VERSION), enforced by CI. | [constraints §13](constraints.md#13-tooling-and-documentation). | T | confirmed | #3, #21 / #3 |
| REQ-TOOL-002 | All KiCad changes shall go through the Konnect MCP tools. | [`AGENTS.md`](../../AGENTS.md#hardware-kicad). | I | confirmed | #21 / PR review |
| REQ-TOOL-003 | ERC shall pass when a schematic changes, and DRC when a PCB changes, before a merge into `dev` or `main`. | [`AGENTS.md`](../../AGENTS.md#ercdrc-merge-gate-required). | T | confirmed | #26 / #26 |
| REQ-TOOL-004 | Every file shall carry SPDX headers or be covered by `REUSE.toml`, and `reuse lint` shall pass. | [`AGENTS.md`](../../AGENTS.md#licensing-reuse). | T | confirmed | #3 / #3 |

## 16. Traceability

Issues that design or verify each requirement. Issues that are not created yet
are named by their roadmap item.

| Issue | Role | Requirements |
|---|---|---|
| [#3](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/3) CI gates | Verify | REQ-MECH-007, REQ-MFG-003, REQ-TOOL-001, REQ-TOOL-004 |
| [#5](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/5) Radio power and interface table | Verify (research) | REQ-GEN-003, REQ-CAT-003, REQ-CAT-007, REQ-PTT-004, REQ-AUD-009, REQ-RIF-006, REQ-RIF-010, REQ-PWR-001, REQ-PWR-004 |
| [#6](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/6) Five-OS host matrix | Verify (research) | REQ-GEN-002, REQ-HOST-001 to -005, REQ-HOST-011, REQ-CAT-006, REQ-AUD-004 to -006 |
| [#7](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/7) Module (ESP32-S3-MINI-1) | Design, verify | REQ-GEN-002, REQ-HOST-012, REQ-REG-001, REQ-REG-002, REQ-REG-006, REQ-MFG-008 |
| [#8](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/8) Audio codec and isolation | Design, verify | REQ-AUD-001, REQ-AUD-007, REQ-AUD-009 to -013, REQ-ISO-001, REQ-EMC-003, REQ-MFG-008 |
| [#9](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/9) CAT/PTT circuits, USB routing | Design | REQ-GEN-003, REQ-GEN-004, REQ-HOST-009, REQ-HOST-014, REQ-CAT-002, REQ-CAT-007, REQ-CAT-008, REQ-PTT-004, REQ-PTT-008, REQ-PTT-011, REQ-RIF-001 to -006, REQ-RIF-008 to -011, REQ-ISO-001, REQ-ISO-003, REQ-EMC-001 to -003, REQ-MFG-008 |
| [#10](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/10) Variant M power | Design, verify | REQ-PWR-003, REQ-PWR-005, REQ-PWR-010 to -018, REQ-ISO-003, REQ-EMC-004 to -007, REQ-MECH-006, REQ-ENV-002, REQ-MFG-004, REQ-MFG-008 |
| [#11](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/11) Variant R power | Design, verify | REQ-RIF-009, REQ-RIF-011, REQ-PWR-001 to -005, REQ-PWR-018, REQ-EMC-004, REQ-EMC-005, REQ-ENV-001, REQ-MFG-004, REQ-MFG-008 |
| [#12](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/12) Variants and board strategy | Design, verify | REQ-GEN-006, REQ-ISO-001, REQ-ISO-002, REQ-REG-006, REQ-EMC-003, REQ-ENV-001, REQ-ENV-002 |
| [#13](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/13) Protocol spec | Design, verify | REQ-GEN-002, REQ-GEN-005, REQ-HOST-003 to -005, -010 to -017, REQ-CAT-005, REQ-PTT-001, -002, -006, REQ-AUD-004, REQ-TIM-001, REQ-FW-004, REQ-FW-005, REQ-PWR-018 |
| [#14](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/14) Firmware core, HAL, host tests | Design, verify | REQ-CAT-001, REQ-HOST-014, REQ-PTT-005, REQ-PTT-010, REQ-PTT-011, REQ-TIM-003, REQ-FW-001 to -003, REQ-FW-006 |
| [#15](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/15) Firmware CAT bridge and PTT | Design, verify | REQ-GEN-001, REQ-HOST-004, REQ-CAT-001 to -003, REQ-CAT-005 to -008, REQ-PTT-001 to -003, REQ-PTT-005 to -007, REQ-PTT-009 to -011, REQ-RIF-003, REQ-RIF-004, REQ-FW-002, REQ-FW-005 |
| [#16](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/16) Firmware audio pipeline | Design, verify | REQ-GEN-001, REQ-HOST-004, REQ-HOST-011, REQ-AUD-001, -002, -004, -005, -007, -008, -013 to -015 |
| [#17](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/17) Firmware tone TX (optional) | Design, verify | REQ-TIM-001 to -003 |
| [#18](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/18) 👤 Audio bench test | Verify | REQ-GEN-001, REQ-GEN-002, REQ-HOST-001 to -004, REQ-HOST-009, REQ-HOST-010, REQ-CAT-006, REQ-PTT-001, REQ-AUD-003, -004, -006, -008, -013, -015 |
| [#19](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/19) Enclosure | Design | REQ-REG-004, REQ-REG-005, REQ-MECH-001 to -005, REQ-ENV-003 |
| [#20](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/20) 👤 Print and fit check | Verify | REQ-REG-004, REQ-REG-005, REQ-MECH-001 to -004, REQ-MECH-006, REQ-MECH-008, REQ-ENV-003 |
| [#21](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/21) KiCad setup | Design | REQ-RIF-001, REQ-RIF-009, REQ-ISO-003, REQ-REG-002, REQ-REG-003, REQ-REG-005, REQ-FW-008, REQ-MECH-007, REQ-MFG-001 to -003, REQ-MFG-009, REQ-TOOL-001, REQ-TOOL-002 |
| [#26](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/26) ERC/DRC merge gate | Design, verify | REQ-TOOL-003 |
| [#43](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/43) Firmware USB host (Bluetooth mode) | Design, verify | REQ-GEN-003, REQ-GEN-004, REQ-CAT-004, REQ-PTT-003, REQ-AUD-001, REQ-AUD-002, REQ-AUD-008, REQ-RIF-006, REQ-FW-007 |
| [#44](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/44) Firmware wired USB-C mode | Design, verify | REQ-GEN-002 to -004, REQ-HOST-001 to -003, REQ-HOST-006 to -010, REQ-HOST-013, REQ-PTT-002, REQ-PTT-005, REQ-PTT-009, REQ-AUD-003, REQ-AUD-006, REQ-AUD-008, REQ-RIF-008, REQ-PWR-004, REQ-FW-005 to -007 |
| [#45](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/45) Host Bluetooth bridge (later) | Design | REQ-HOST-005 |
| [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64) Product security (CRA level) | Design, verify | REQ-HOST-015 to -017, REQ-FW-006 |
| Parts list (to be created) | Verify | REQ-MFG-007, REQ-MFG-008, REQ-MFG-010 |
| Design review (to be created) | Verify | REQ-REG-002, REQ-EMC-002, REQ-MFG-001, -002, -004 to -007 |
| Release workflow (to be created) | Verify | REQ-MFG-009 |
| Bring-up (Phase 6, to be created) | Verify | REQ-GEN-001, -003, -004, REQ-HOST-009, REQ-CAT-002, -004, -007, REQ-PTT-004, -005, -008, REQ-AUD-001, -007, -010 to -012, REQ-RIF-002, -003, -005, -006, REQ-ISO-001, REQ-PWR-002 to -005, REQ-PWR-010 to -016, REQ-EMC-001, -003, -004, REQ-ENV-001, REQ-ENV-002 |
| Compliance (Phase 6, to be created) | Verify | REQ-REG-003, REQ-EMC-006, REQ-EMC-007 |
| Maintainer | Decide | REQ-MECH-008, REQ-MFG-010 |

## 17. Coverage checklist

Every section and bullet of [`constraints.md`](constraints.md) maps to at least
one requirement. Update this list when either document changes.

**§1 What the device is**

- [x] Interface over Bluetooth LE or wired USB-C: REQ-GEN-001, REQ-GEN-002
- [x] CAT serial, transparent passthrough: REQ-CAT-001, REQ-FW-002
- [x] PTT output plus RTS/DTR control lines: REQ-PTT-001 to -004
- [x] Audio RX and TX: REQ-AUD-001, REQ-AUD-004, REQ-AUD-006

**§2 Host compatibility**

- [x] Two host links, radio side the same in both: REQ-GEN-002, REQ-GEN-004
- [x] Host table (Windows/macOS/Linux, Android, iOS/iPadOS): REQ-HOST-001 to -004
- [x] No Bluetooth Classic: REQ-HOST-012
- [x] Wired mode selected automatically, setting to force, radio off in wired mode: REQ-HOST-006 to -008
- [x] Radio USB chips appear directly in wired mode; device adds its own sound card, serial ports and USB network interface per radio type: REQ-HOST-009, REQ-HOST-010, REQ-HOST-013
- [x] No OS shows a BLE device as serial/audio natively; documentation: REQ-HOST-005
- [x] BLE audio ≥ 12 kHz / 16-bit, L2CAP CoC or GATT, 2M PHY, LC3 fallback: REQ-AUD-004, REQ-AUD-005, REQ-HOST-011
- [x] USB audio 48 kHz / 16-bit, UAC1 or UAC2: REQ-AUD-006
- [x] RTS/DTR native in wired mode, protocol over Bluetooth; PTT via protocol or CAT: REQ-PTT-001, REQ-PTT-002
- [x] Not required (Hamlib/FLrig, Wi-Fi, LE Audio): REQ-HOST-012

**§3 Power**

- [x] §3.1 Radio USB port provides no power; device supplies no VBUS to it: REQ-RIF-011
- [x] §3.1 Radio accessory DC: REQ-PWR-001
- [x] §3.1 USB-C 5 V sink: REQ-PWR-002
- [x] §3.1 12 V vehicle or station supply: REQ-PWR-010 to -016
- [x] §3.2 Normal range, cold crank, load dump, reverse battery, jump start, ISO 7637-2 pulses: REQ-PWR-010 to -015
- [x] §3.2 ESD ±15 kV: REQ-EMC-007
- [x] §3.2 Temperature −40 to +85 °C: REQ-ENV-002
- [x] §3.2 Off-state drain and auto power-down: REQ-PWR-016, REQ-PWR-018
- [x] §3.2 Design guidance (ideal diode, surge stopper/TVS, AEC-Q100 buck, CM choke + pi filter): REQ-PWR-017
- [x] §3.2 Low-EMI conversion, switching frequency off the HF bands: REQ-EMC-005
- [x] §3.2 CISPR 25 Class 3: REQ-EMC-006
- [x] §3.2 Vibration-rated locking connectors: REQ-MECH-006
- [x] §3.3 13.8 V accessory input, protection, current limit: REQ-PWR-001
- [x] §3.3 USB-C 5 V sink, no PD: REQ-PWR-002, REQ-RIF-009
- [x] §3.4 Power budget: REQ-PWR-003
- [x] §3.4 USB-C budget and overcurrent reporting: REQ-PWR-004

**§4 Regulatory**

- [x] FCC-certified module, FCC ID recorded, integration guide: REQ-REG-001, REQ-REG-002
- [x] Part 15 Subpart B Class B SDoC and label: REQ-REG-003, REQ-REG-004
- [x] ISED and CE optional: REQ-REG-006
- [x] No metal over the antenna: REQ-REG-005, REQ-MECH-003

**§5 RF environment**

- [x] PTT never asserts from RF pickup: REQ-EMC-001
- [x] Filter audio, CAT and PTT lines; grounding plan: REQ-EMC-002, REQ-EMC-003
- [x] No noise in the HF bands: REQ-EMC-004, REQ-EMC-005

**§6 Safety and fail-safe**

- [x] PTT off at power-on, reset, brownout, disconnect, watchdog: REQ-PTT-005, REQ-PWR-005
- [x] Hardware default off: REQ-PTT-008
- [x] Maximum continuous TX time: REQ-PTT-007
- [x] Firmware lock-up → internal watchdog reset, PTT off: REQ-PTT-011
- [x] Isolation of AUDIO and SERIAL jacks (M, and whenever USB-C data is used): REQ-ISO-001, REQ-ISO-002
- [x] Radio USB port not isolated by default; USB isolator option: REQ-ISO-003

**§7 Radio interfaces**

- [x] CAT modes (TTL, RS-232, CI-V), 4800–115200 baud, firmware-selected: REQ-RIF-003, REQ-CAT-002, REQ-CAT-008
- [x] Every mode survives any cable; power-on default 3.3 V logic: REQ-RIF-005, REQ-RIF-004
- [x] PTT isolated closure; RTS/DTR mapping; no RS-232 RTS/DTR contacts: REQ-PTT-003, REQ-PTT-004
- [x] USB host to CP210x/CP2105/FTDI/CH34x/CDC-ACM and UAC1, through a radio hub, full speed, no VBUS, routed to the hub in wired mode: REQ-RIF-006, REQ-RIF-008, REQ-RIF-011
- [x] Radio-type coverage in both host modes: REQ-GEN-003
- [x] Connectors (two TRRS jacks, USB-A), fixed pinout, per-radio cables: REQ-RIF-001, REQ-RIF-002, REQ-RIF-010

**§8 Audio**

- [x] Two radio audio paths, selection and override, wired-mode behavior: REQ-AUD-001 to -003
- [x] Codec at 48 kHz; host path ≥ 12 kHz / 16-bit: REQ-AUD-007, REQ-AUD-004
- [x] Rate matching: REQ-AUD-008
- [x] ADC SNR ≥ 90 dB(A): REQ-AUD-012
- [x] Clock ±50 ppm: REQ-AUD-011
- [x] No AGC/NR/voice processing; adjustable TX level: REQ-AUD-013, REQ-AUD-014
- [x] Fixed, measured latency: REQ-AUD-015

**§9 Timing**

- [x] Clock sync, scheduled TX within about ±20 ms of UTC: REQ-TIM-001, REQ-TIM-002
- [x] Optional tone-sequence module, not in the core: REQ-TIM-003

**§10 Firmware**

- [x] Permissive SDK and dependencies, no NDA SDKs, THIRD_PARTY.md: REQ-FW-001
- [x] No radio-specific logic in the core: REQ-FW-002, REQ-CAT-001
- [x] Versioned protocol with capability discovery; configuration over Bluetooth: REQ-FW-004, REQ-FW-005
- [x] Optional signed OTA: REQ-FW-006

**§11 Environmental and mechanical**

- [x] Operating temperature per variant: REQ-ENV-001, REQ-ENV-002
- [x] Enclosure material per variant: REQ-MECH-002, REQ-ENV-003
- [x] Parametric CAD, no supports, keep-out, mounting, strain relief, label space, CI-generated STL/3MF: REQ-MECH-001, REQ-MECH-003 to -005
- [x] Size target TBD: REQ-MECH-008

**§12 Manufacturing and sourcing**

- [x] PCB and passives per pcb-fabrication.md (JLCPCB standard, 2/4 layers, 0402, MLCC 2× rule): REQ-MFG-001 to -007
- [x] Two distributor sources, active lifecycle: REQ-MFG-008
- [x] BOM cost target TBD: REQ-MFG-010

**§13 Tooling and documentation**

- [x] KiCad ≥ 10.0.6, KICAD_VERSION enforced by CI: REQ-TOOL-001
- [x] KiCad changes through Konnect: REQ-TOOL-002
- [x] Silkscreen content, revision not hard-coded: REQ-MECH-007
- [x] Variants share one core design, recorded decision: REQ-GEN-006

**Requirements from other sources** (not in `constraints.md`):

- [`radio-connectors.md`](radio-connectors.md): REQ-AUD-009, REQ-AUD-010, REQ-CAT-005, REQ-CAT-007, REQ-PTT-009
- [ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md): REQ-CAT-004, REQ-FW-007, REQ-FW-008
- [ADR-0007](../decisions/ADR-0007-protocol.md): REQ-HOST-013 to -017, REQ-PTT-011
- [`AGENTS.md`](../../AGENTS.md) and [`GOVERNANCE.md`](../../GOVERNANCE.md): REQ-GEN-005, REQ-PTT-010, REQ-FW-003, REQ-MFG-009, REQ-TOOL-003, REQ-TOOL-004
- Per-radio needs (#5): REQ-CAT-003

## Open questions

- **CAT latency target** (REQ-CAT-006) and **keepalive and maximum-TX
  defaults** (REQ-PTT-006, REQ-PTT-007): settled at 3000 ms and 300 s,
  both configurable ([protocol §6.1](../../protocol/SPEC.md#61-keys)).
- **BOM cost and enclosure size targets** (REQ-MFG-010, REQ-MECH-008): maintainer.

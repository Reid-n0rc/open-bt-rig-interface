<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0007: One framed message stream over BLE GATT, L2CAP CoC and wired USB (network and serial)

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #13

## Context

[ADR-0008](ADR-0008-host-links-esp32-s3.md) (accepted) gives the device two
host links: **Bluetooth LE** and **wired USB-C**, with no Bluetooth Classic.
The issue text for #13 still describes SPP, HFP and RFCOMM modem-status
messages. Those are obsolete and are replaced here.

What the protocol has to work with ([host compatibility](../research/host-compatibility.md),
[`constraints.md`](../requirements/constraints.md) §2, §6, §9):

- No OS shows a BLE device as a serial port or sound card, so every Bluetooth
  host needs an app that implements this protocol.
- L2CAP CoC is available to apps on iOS/iPadOS, macOS, Android API 29+ and
  Linux, but **Windows has no public L2CAP CoC API**, so the protocol must also
  work over GATT alone. iOS and Android expose L2CAP channels as byte streams.
- 12 kHz / 16-bit audio (192 kbit/s) needs about 98 full 2M-PHY packets per
  second. The ESP32-S3 needs NimBLE for L2CAP CoC.
- Linux asserts RTS and DTR when a serial port opens, so line-based PTT must not
  key on a port open.
- PTT must fail safe: keepalive, maximum TX time, off on any disconnect.
- The module's FCC grant covers BLE at 10.3 dBm conducted at most
  ([`fcc.md`](../compliance/fcc.md)).
- iOS/iPadOS apps can't open USB serial ports, so ADR-0008's wired CDC-ACM
  ports give iPhone and iPad no CAT, PTT or configuration.

## Options considered

### Transport and framing

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **A. One COBS-framed, CRC-checked message stream, carried unchanged by GATT (write without response + notify), L2CAP CoC, a TCP connection over a USB network interface, and a CDC-ACM control port** | One message set and one set of golden vectors for every transport; works on Windows (GATT); faster where L2CAP exists; resynchronizes after loss; byte-stream semantics match iOS/Android L2CAP and TCP | Custom framing; audio shares the stream with control (solved by send priority and small audio frames) | [COBS](../references/index.md#cobs-paper), [CRC catalogue](../references/index.md#crc-catalogue-16), [host compatibility §3](../research/host-compatibility.md#3-bluetooth-le) |
| B. GATT only, one characteristic per function (PTT, CAT, audio, config) | Common BLE style; OS stacks handle attribute boundaries | Per-write overhead and the 512-byte attribute limit; no path to L2CAP's throughput and flow control; a different design for the wired link | [host compatibility §3.2](../research/host-compatibility.md#32-gatt-vs-l2cap-coc) |
| C. L2CAP CoC only | Best throughput and flow control | **No Windows support** | [host compatibility §3.1](../research/host-compatibility.md#31-capabilities-per-os) |
| D. LE Audio (LC3 over isochronous channels) for audio | Standard profile | ESP32-S3 has no isochronous channels; not on Apple; hardware-dependent elsewhere; brings OS voice processing back; no CAT or PTT | [host compatibility §4](../research/host-compatibility.md#4-le-audio) |
| E. Bluetooth Classic SPP + HFP | Native COM port and headset on four OSes | Rejected by ADR-0008 (module choice, HFP audio quality) | [ADR-0008](ADR-0008-host-links-esp32-s3.md) |
| F. Reuse a third-party UUID set (for example a vendor "UART" service) | Existing app support | Not ours; clashes with other devices that use it; the issue requires new UUIDs | — |
| G. Length-prefixed frames without a delimiter | Slightly simpler | No resynchronization after a lost or joined chunk, or junk on the wired serial port | — |
| H. CBOR or Protocol Buffers payloads | Self-describing, extensible | Extra firmware dependencies; variable encodings make exact golden bytes harder; fixed little-endian fields plus TLV capabilities are enough | — |

### iPhone and iPad CAT/PTT in wired mode (maintainer decision, 2026-09-24)

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **W1. A USB network interface (CDC-NCM) in wired mode, carrying the protocol over TCP** | Driverless on Windows 11, Linux and Android 14 GKI kernels; Apple recommends Ethernet over USB for iPhone accessories; same protocol and framing | Endpoint budget (below); Windows 10 has no in-box NCM driver; NCM on iOS/iPadOS/macOS and Android bring-up unconfirmed; needs a TCP/IP stack in the firmware | [ms-usb-classes](../references/index.md#ms-usb-classes), [apple-forum-747847](../references/index.md#apple-forum-747847), [apple-forum-802640](../references/index.md#apple-forum-802640), [android-kconfig-cdc-ncm](../references/index.md#android-kconfig-cdc-ncm), [linux-cdc-ncm](../references/index.md#linux-cdc-ncm) |
| W2. Keep Bluetooth LE on for control while wired | No new USB function | Contradicts ADR-0008 (radio off in wired mode); two links to keep safe at once | [host compatibility §6.4](../research/host-compatibility.md#64-open-design-question-catptt-for-iphone-and-ipad-in-wired-mode) |
| W3. Accept audio-only wired mode on iOS | Simplest | No CAT or PTT for iPhone/iPad users when wired | — |
| W4. RNDIS or CDC-ECM instead of NCM | ECM is simpler | Microsoft recommends NCM over RNDIS, and Windows ships no in-box ECM driver | [ms-usb-classes](../references/index.md#ms-usb-classes) |

## Decision

**Option A with W1.** The protocol ([`protocol/SPEC.md`](../../protocol/SPEC.md),
version 0.1.0) is one framed byte stream:

- **Framing:** `type | token | payload | CRC-16/IBM-3740`, COBS-encoded, 0x00
  delimited. Little-endian fixed fields; capabilities as TLVs.
- **Transports:** GATT (a project-owned service with new random 128-bit UUIDs:
  RX write-without-response, TX notify, and an Info characteristic readable
  before pairing that carries the version and the L2CAP PSM); optional L2CAP
  CoC; TCP over a CDC-NCM USB network interface in wired mode (DHCP on
  10.169.160.0/30, DNS-SD `_rig-interface._tcp`, TCP port 51621); and the
  CDC-ACM control port where the endpoint budget allows.
- **PTT safety in the protocol:** a keepalive for every protocol-originated
  PTT source; a maximum TX time that can't be disabled, with lockout until
  every source is released; an arming rule so that a port open that raises
  RTS and DTR together never keys; PTT off at every session end.
- **SPEC §8 (PTT rules) approved by the maintainer, 2026-09-24**, together
  with three user-facing notes in §8.5: wired CDC-ACM RTS/DTR needs no
  keepalive, so a hung desktop program can hold PTT until `MAX_TX_S`, the
  hardware watchdog or a USB disconnect; CAT-command keying is invisible to
  the device and guarded only by the radio's timers; RTS and DTR rising
  together count as a port open and don't key (map only one line to PTT).
- **Hardware PTT watchdog** (maintainer decision, 2026-09-24, designed in
  #9): a hardware timer (configurable, default 10 minutes) forces PTT off
  even if the firmware is hung. It backs up `MAX_TX_S`, which must not exceed
  it. The `PTT` capability reports the limit (`hw_max_tx_s`, 0 = unknown),
  and `PTT_STATUS` reason `HW_WATCHDOG` reports a trip after the fact.
- **Audio:** PCM16 at 12 or 16 kHz (LC3 optional), 10 ms frames with a
  sequence number and a sample timestamp, one direction at a time, the
  device's sample clock as the stream clock, and optional scheduled start.
- **Clock sync:** NTP-style exchanges with min-RTT filtering, then a UTC
  mapping for scheduled audio and the optional tone-sequence module.
- **BLE TX power** can only be lowered through the protocol; the firmware cap
  (at or below the grant's 10.3 dBm) can't be exceeded.
- **OTA transport:** deferred to #14. Message types 0xE0–0xEF and a feature
  bit are reserved; wired mode may use USB DFU (ADR-0008).
- **Pairing window** (maintainer decision, 2026-09-24): new BLE bonds are
  accepted only during a window opened by a local action on the device
  (power-on or a pairing button; the exact trigger is for #14 and the
  hardware **(verify)**). It closes after `PAIRING_WINDOW_S` (default 120 s),
  after the first new bond, or on entering wired mode. Outside the window
  only bonded hosts can use the device. Info `flags` bit 1 and `STATUS.flags`
  bit 6 show the window; the `PAIRING` capability reports the triggers, the
  allowed window lengths and the bond capacity. Only a local action opens
  the window: no protocol message, and neither the wired control port nor
  the USB network, can open it (maintainer decision, 2026-09-24).
- **Configurable defaults** (maintainer decision, 2026-09-24): keepalive
  3000 ms (500–10 000), max TX 180 s (10–600, can't be disabled), serial
  9600 8N1 (`SERIAL_DEFAULT`), `LINE_MAP` (SERIAL-jack and control-port RTS →
  PTT, DTR ignored; radio ports pass-through), `WIRED_PROFILE` serial, USB
  network subnet 10.169.160.0/30 (`USB_NET_SUBNET`), pairing window 120 s.
  Each has a config key with a documented range (SPEC §6.1).

### Relationship to ADR-0008

This ADR **extends ADR-0008's wired link** with the USB network interface, and
**supersedes ADR-0008's wired function list**: the "Wired USB-C mode: the
computer sees" column of its radio-side table, and its statement that the
device "always adds its own USB serial port in wired mode for configuration
and for PTT on the AUDIO jack". That list doesn't fit the ESP32-S3 (next
section) and is replaced by the four function sets below. The rest of
ADR-0008 stands. ADR-0008 itself is not edited (accepted ADRs are never
changed, [`README.md`](README.md)); the maintainer accepted this change on
2026-09-24.

### Endpoint budget (checked for W1)

The ESP32-S3 USB OTG has endpoint 0 plus 6 endpoints configurable as IN or
OUT, **at most 5 IN endpoints including endpoint 0**
([esp-usb device guide](../references/index.md#esp-usb-device-s3)). TinyUSB's
descriptor templates use 2 IN + 1 OUT for CDC-ACM and for CDC-NCM (notification,
bulk IN, bulk OUT), and UAC1 speaker + microphone needs 1 IN + 1 OUT with an
adaptive OUT endpoint ([`usbd.h`](../references/index.md#tinyusb-usbd-h)).
`esp_tinyusb` documents composite devices, CDC-ACM and a network driver
(ECM/NCM/RNDIS) ([esp-usb device guide](../references/index.md#esp-usb-device-s3)).
Its guide doesn't list audio; UAC1 comes from TinyUSB's own audio class
([TinyUSB README](../references/index.md#tinyusb-readme)). That the TinyUSB
version `esp_tinyusb` pulls in builds NCM + UAC1 (or NCM + CDC-ACM) in one
full-speed composite is **(verify, #44)**.

- ADR-0008's full wired set (SERIAL-jack bridge + control port + UAC1) needs
  5 IN + 3 OUT: **it already doesn't fit.** Adding NCM would need 7 IN.
- So the device enumerates one of four function sets, chosen from the radio
  it finds on the radio port before switching to wired mode (SPEC §14.1):
  NCM + control port (USB-serial + USB-audio radios), NCM + UAC1 (USB-serial +
  analog audio), and, for SERIAL-jack radios, either NCM + UAC1 (profile
  *network*, CAT over TCP) or CDC-ACM bridge + UAC1 (profile *serial*, native
  COM port and RTS/DTR PTT). Each uses at most 4 IN and 2 OUT endpoints.
- A new `WIRED_PROFILE` setting picks the SERIAL-jack profile. The default is
  *serial* (maintainer decision, 2026-09-24), so desktop radio software gets a
  native COM port. iPhone and iPad users set *network*.
- The maintainer accepted these four function sets on 2026-09-24.

## Consequences

- One reference codec and one set of golden vectors (`protocol/vectors/`)
  cover every transport. CI checks them (`Protocol vectors` job).
- **iPhone and iPad in wired mode** get CAT, PTT and configuration over the
  network interface with SERIAL-jack radios.
- **Known limitation (accepted by the maintainer, 2026-09-24):** with radios
  whose serial is USB, the radio's chip sits behind the hub where iOS apps
  can't reach it. iOS gets audio, AUDIO-jack PTT and configuration, but **no
  CAT**, in wired mode. Bluetooth mode has full CAT (REQ-HOST-003).
- **ADR-0008's "always a USB serial port for configuration and AUDIO-jack PTT"
  no longer holds** for every radio type: with USB-serial + analog-audio
  radios, and with SERIAL-jack radios in the network profile, configuration
  and PTT use the network interface. `constraints.md` §2 and REQ-HOST-003,
  REQ-HOST-010, REQ-HOST-013, REQ-HOST-014 and REQ-FW-005 are updated to
  match.
- **Windows 10** has no in-box NCM driver; it uses the serial profile, the
  control port where present, or Bluetooth.
- Firmware (#44) needs a TCP/IP stack (lwIP in ESP-IDF), a DHCP server and
  mDNS on the NCM interface, and must probe the radio port before
  enumerating. Their licenses go in `THIRD_PARTY.md` when pulled in, and RAM
  use without PSRAM (-N8) is **(verify)**.
- To verify on hardware (#18): NCM on iOS, iPadOS, macOS and Android;
  Windows and macOS RTS/DTR behavior at port open; BLE throughput per OS; the
  clock-sync accuracy against ±20 ms.
- Firmware (#14) needs a pairing-window state machine and a trigger; the
  hardware (#9) decides whether there is a pairing button.
- Open: EU conformity (RED Delegated Regulation 2022/30, EN 18031-1) may
  add access-control or authentication requirements to the wired control
  port and the USB-network TCP transport; #59 will report back.

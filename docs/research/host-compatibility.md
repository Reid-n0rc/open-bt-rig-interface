<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Host compatibility: wired USB-C and Bluetooth LE on five operating systems

Issue: [#6](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/6).
Researched 2026-09-24. Design basis:
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md) (proposed):
ESP32-S3-MINI-1, **Bluetooth LE + wired USB-C** host links, no Bluetooth
Classic. Per the issue's scope update, SPP, HFP/mSBC and RFCOMM are dropped.

This document records how each host OS handles the device's two host links,
and what that means for the protocol (#13), the CAT bridge (#15), the audio
pipeline (#16) and wired mode (#44).

## How to read this

- **OS rows:** iOS/iPadOS 18 and 26, macOS 15 and 26, Android 13–16,
  Windows 10 and 11, Linux (BlueZ 5.x + PipeWire). ChromeOS is noted only (§5).
- **Sources** are the vendors' current documents, checked on 2026-09-24. Where a
  document doesn't separate the two versions in a row (for example iOS 18 and
  26), the finding applies to both until a bench test shows otherwise.
  Each source is listed in the [reference index](../references/index.md), with
  its date. Apple developer-forum replies by Apple engineers are labeled as such.
- **`needs bench test`** marks anything that only hardware can confirm. These
  items are collected in §7 for the bench-test issue #18.
- **(verify)** marks facts that come from a secondary source or from reading
  source code, and still need a primary-document or bench check.
- Linux kernel and BlueZ sources are GPL/LGPL. They were read for facts only
  and nothing was copied ([`THIRD_PARTY.md`](../../THIRD_PARTY.md#facts-only-never-copied)).

## Summary

| Host | Wired USB-C: serial (CDC-ACM) | Wired USB-C: audio (UAC1) | Bluetooth LE: L2CAP CoC for apps | Bluetooth LE: GATT |
|---|---|---|---|---|
| iOS / iPadOS 18, 26 | **No** app access to USB serial | Yes (built-in class driver) | Yes (`openL2CAPChannel`, iOS 11+) | Yes |
| macOS 15, 26 | Yes (built-in, `/dev/cu.*` (verify)) | Yes | Yes (`openL2CAPChannel`, macOS 10.14+) | Yes |
| Android 13–16 | Apps only (USB host API) | Yes (UAC1 subset) | Yes (`createL2capChannel`, API 29+) | Yes |
| Windows 10, 11 | Yes (`usbser.sys`, COM port) | Yes (`usbaudio.sys`) | **No public API found** | Yes |
| Linux (BlueZ + PipeWire) | Yes (`cdc_acm`, `/dev/ttyACM*`) | Yes (`snd-usb-audio`) | Yes (L2CAP sockets, Linux 5.10+) | Yes |

Main conclusions (details in §6):

1. **USB audio: use UAC1** (USB Audio Class 1.0) for the device's own sound card.
   It is the only version every target OS documents, Android included.
2. **iPhone and iPad can't reach CAT or PTT over the wired link.** Apple has no
   app access to USB serial on iOS/iPadOS. Wired mode on Apple mobile devices is
   audio-only unless the design adds another channel (§6.4).
3. **The Bluetooth protocol needs two transports:** L2CAP CoC where available,
   and GATT for Windows (no L2CAP CoC API) and as a fallback everywhere.
4. **Raw 12 kHz PCM fits** the Bluetooth LE link on paper with 2M PHY and data
   length extension, at about 98 full-size packets per second. Whether each OS
   delivers that is `needs bench test`. LC3 stays a fallback, not a baseline.
5. **Opening a serial port can raise RTS and DTR** (Linux does this by design).
   PTT keyed from RTS/DTR must not key on port open (§6.2).

---

## 1. Wired USB-C: USB serial (CDC-ACM) and RTS/DTR

The device's own serial port is a USB CDC-ACM function (ADR-0008). RTS and DTR
travel from host to device in the CDC `SET_CONTROL_LINE_STATE` request.

| Host | Built-in driver | How the port appears | Can apps set RTS/DTR? | Sources |
|---|---|---|---|---|
| iOS / iPadOS | None for apps | Not visible to apps | No | Apple DTS (Quinn, Jan 2025): iOS and iPadOS support many USB classes through higher-level APIs, "However, the list of supported classes doesn't include serial", while macOS does. DriverKit is possible on iPadOS for a custom USB device, "not an option on iOS" [apple-forum-772812] |
| macOS 15, 26 | Yes (Apple CDC-ACM class driver) | `/dev/cu.*` / `/dev/tty.*` serial device (verify) | Expected via `ioctl(TIOCMSET)`; mapping to `SET_CONTROL_LINE_STATE` `needs bench test` | Apple DTS: serial is in the supported class list on macOS [apple-forum-772812] |
| Android 13–16 | No system serial port; apps drive the device through the USB host API (`claimInterface`, `controlTransfer`, `bulkTransfer`) | Per-app, after the user grants USB permission | Yes, by sending `SET_CONTROL_LINE_STATE` itself with `controlTransfer` | [android-usb-host] |
| Windows 10, 11 | Yes: `usbser.sys` loads automatically for class 02h / subclass 02h (ACM) | COM port (Ports class) | Yes: Win32 serial APIs, and WinRT `SerialDevice` exposes `IsDataTerminalReadyEnabled` / `IsRequestToSendEnabled`; wire-level behavior `needs bench test` | [ms-usbser], [ms-usb-classes], [ms-winrt-serialdevice] |
| Linux | Yes: `cdc_acm` | `/dev/ttyACM*` | Yes: `TIOCMSET` sends `SET_CONTROL_LINE_STATE` | [linux-cdc-acm] (v6.16 source) |

Findings that matter for PTT (#15):

- **Linux raises DTR and RTS when a port is opened** and drops both when it
  closes (the driver's `dtr_rts` handler sends DTR|RTS when active, 0 otherwise).
  Setting the baud rate to B0 also drops DTR [linux-cdc-acm]. Windows and macOS
  behavior at open and close is `needs bench test`.
- On Windows the device descriptor, or the function's interface class and
  subclass, must be 02h/02h for `usbser.sys` to load without an INF [ms-usbser].
- **Radio USB-serial chips** (CP210x, FTDI, CH34x) that the computer reaches
  through the on-board hub (ADR-0008) are vendor classes, not CDC-ACM. Their
  driver status on each OS is outside this report (verify in #15 / #44). On
  iOS/iPadOS they are unreachable by apps for the same reason as above.

## 2. Wired USB-C: USB Audio Class

### 2.1 UAC1 and UAC2 support

| Host | UAC1 (ADC 1.0) | UAC2 (ADC 2.0) | Notes | Sources |
|---|---|---|---|---|
| iOS / iPadOS | Yes | Yes | Apple's USB audio driver serves Mac, iPad and iPhone. Supports ADC1–3, not ADC4. Adaptive IN endpoints are treated as asynchronous; implicit feedback is supported. Without the ADC2 Terminal **Connector** control, iOS/iPadOS may route audio to the device "last-in-wins" | [apple-tn3190] (2025-10-07) |
| macOS 15, 26 | Yes | Yes | Same driver as above. Processing, Extension and Effect units are not supported | [apple-tn3190] |
| Android 13–16 | Yes, a **subset**: device must be the host side's peripheral, PCM Type I, 16/24/32-bit, 8–48 kHz (48, 44.1, 32, 24, 22.05, 16, 12, 11.025, 8 kHz), 1 or 2 channels | **Not claimed**: "more advanced features are not yet claimed" | Handheld devices with USB host mode must implement the USB audio class (CDD 7.7.2); the same clause is in the Android 13 and 16 CDDs | [android-usb-audio] (2025-02-27), [android-cdd-16], [android-cdd-13] |
| Windows 10, 11 | Yes: `usbaudio.sys` (compatible ID `USB\Class_01`) | Yes: `usbaudio2.sys`, **Windows 10 1703 and later** | `usbaudio2.sys` supports asynchronous, synchronous and adaptive endpoints; asynchronous OUT needs an **explicit** feedback endpoint (no implicit feedback); one clock source only | [ms-usb-classes], [ms-usbaudio2] (2025-10-27) |
| Linux | Yes | Yes (and UAC3) | `snd-usb-audio` handles UAC versions 1, 2 and 3 | [linux-snd-usb-card] (v6.16 source) |

UAC1 limits (from Apple's technote): full-speed only, 1 ms polling, at most
1023 bytes per millisecond, and no clock-source or latency descriptors
[apple-tn3190]. The device needs 48 kHz / 16-bit mono (`constraints.md` §2),
which is 96–98 bytes per millisecond, about a tenth of that limit. The ESP32-S3
USB OTG is full-speed anyway [esp32s3-ds].

### 2.2 Composite device (CDC + UAC) behind the on-board hub

- **Windows:** the generic parent driver `usbccgp.sys` splits a composite device
  into functions and treats interface collections as one function each
  [ms-usbccgp]. CDC-ACM and UAC each use two or more interfaces, so both need
  an Interface Association Descriptor (IAD).
- **Apple, Android, Linux:** the class drivers above bind per interface. iPad
  supports hubs and docks on its USB-C port [apple-ipad-usbc]. Enumeration of
  the hub + device + radio tree on iPhone, iPad and Android is
  `needs bench test`.
- **Android:** the audio HAL uses the USB audio interfaces while an app claims
  the CDC-ACM interfaces through the USB host API [android-usb-host].
  Both at once `needs bench test`.

### 2.3 OS voice processing on a USB sound card, and how to avoid it

| Host | When the OS processes audio | How to avoid it | Sources |
|---|---|---|---|
| iOS / iPadOS | When the app uses voice processing (`voiceChat`/`videoChat` modes, the voice-processing I/O unit, or `setVoiceProcessingEnabled`) | Apps use `AVAudioSession.Mode.measurement`, which minimizes "system-supplied signal processing", and don't enable voice processing. Whether Control Center mic modes affect a USB input `needs bench test` | [apple-avaudiosession-measurement], [apple-voice-processing] |
| macOS | Same: only when the app enables voice processing | Apps read the device directly through Core Audio without voice processing | [apple-voice-processing] |
| Android | AGC and noise suppression are typical on voice-communication capture; CDD requires them **off by default** for `VOICE_RECOGNITION` | Apps record with `AudioSource.UNPROCESSED` where the device reports support, else `VOICE_RECOGNITION`. Behavior on the USB route `needs bench test` | [android-cdd-16] §5.4.2, §5.11 |
| Windows 10, 11 | Streams tagged **Communications** get the driver's communications-mode effects. On Windows 11, **Voice Clarity** (AI noise suppression and echo cancellation, on by default) runs for apps using communications mode when the device has no OEM communications processing | Apps use a non-communications category, or request **Raw** mode where the device reports `RawProcessingSupported`; Raw capture must not include AEC, AGC or noise suppression. Which effects apply to the in-box USB class drivers `needs bench test` | [ms-audio-modes], [ms-voice-clarity] |
| Linux | PipeWire's echo-cancel module only when configured; it creates separate virtual nodes and leaves the device node untouched | Apps use the device node, not an `echo-cancel` node | [pipewire-echo-cancel] |

### 2.4 Power from the host's USB-C port

| Host | What the port offers | Sources |
|---|---|---|
| Any USB-C host | "USB Type-C Current at Default": 500 mA for USB 2.0 ports; 900 mA (single-lane) or 1,500 mA (dual-lane) for USB 3.2. Or 1.5 A or 3.0 A, advertised on CC. A device must stay within the advertised level and scale back if it drops | [usb-typec-r25] §2.3.4 |
| iPhone 15 and later | Can charge a small USB PD device "at up to 4.5 watts". Bus power to a non-PD device `needs bench test` | [apple-iphone-usbc] (2026-09-17) |
| iPad | Apple publishes no figure; some accessories "can request higher power". `needs bench test` | [apple-ipad-usbc] |
| Android | CDD: SHOULD advertise at least 1.5 A (Type-C) when charging a peripheral in host mode; a recommendation, not a requirement | [android-cdd-16] §7.7.2 |
| Windows / macOS / Linux PCs | Per port, per machine; read the CC advertisement. `needs bench test` on typical laptops | [usb-typec-r25] |

The device is USB 2.0, so a host advertising Default current offers **500 mA**.
`constraints.md` §3.4 already plans for this. The device, the hub and the
radio's VBUS draw must fit in 500 mA unless CC shows more.

## 3. Bluetooth LE

The device is the LE peripheral; the host is the central. No OS exposes an LE
device as a serial port or sound card, so every host needs an app or host
software (`constraints.md` §2).

### 3.1 Capabilities per OS

| Host | 2M PHY | Data length extension (DLE) | ATT MTU | Connection interval | L2CAP CoC API | Sources |
|---|---|---|---|---|---|---|
| iOS / iPadOS | Supported; chosen by the OS, no app API. DTS lists "PHY2" among ways to raise throughput | Negotiated by the OS; accessories "should support" DLE and must run the length update **before** the MTU exchange | OS requests the "optimal" MTU; the accessory should accept at least that | Accessory requests: min ≥ **15 ms**, multiple of 15 ms, max ≥ min + 15 ms (or both 15 ms), latency ≤ 30, supervision timeout 6–18 s. The OS may answer with other parameters | `CBPeripheral.openL2CAPChannel(_:)`, iOS 11+ | [apple-adg] R31 §58.6, §58.7, §58.11; [apple-forum-770717] (DTS, Dec 2024); [apple-cb-openl2cap] |
| macOS 15, 26 | As iOS (Core Bluetooth) | As iOS ("iOS devices and Mac computers" negotiate data length) | As iOS | Same guidelines | `openL2CAPChannel(_:)`, macOS 10.14+ | [apple-adg], [apple-cb-openl2cap] |
| Android 13–16 | `setPreferredPhy` (API 26); a recommendation the controller may override | Stack and controller dependent; `needs bench test` | Android 14+ requests **517** for the first GATT client and ignores later requests; `requestMtu` on 13 | `requestConnectionPriority(HIGH)` for bulk transfer; exact values are device-dependent | `BluetoothDevice.createL2capChannel(psm)` / `createInsecureL2capChannel(psm)`, **API 29+**, LE only | [android-bluetoothgatt], [android-bluetoothdevice] |
| Windows 10, 11 | Read-only: `GetConnectionPhy` reports 1M/2M/Coded (**Windows 11** only) | OS-managed | OS-managed; `GattSession.MaxPduSize` reports it and the OS fragments larger writes | Windows 11: `RequestPreferredConnectionParameters` with `ThroughputOptimized`, `Balanced` or `PowerOptimized`. Windows 10: no app API | **None found**: the WinRT `Windows.Devices.Bluetooth` namespace lists no L2CAP classes (checked 2026-09-24). GATT only | [ms-winrt-ble-phy], [ms-winrt-ble-connparams], [ms-winrt-gatt-maxpdu], [ms-winrt-bluetooth] |
| Linux (BlueZ 5.x) | `BT_PHY` socket option (Linux 5.10+) | Kernel / controller | `ExchangeMTU` default **517** in `main.conf`; GATT through D-Bus, with `AcquireWrite` / `AcquireNotify` file descriptors for bulk data | `MinConnectionInterval` / `MaxConnectionInterval` in `main.conf` | L2CAP sockets, `SOCK_SEQPACKET`, `BT_MODE_LE_FLOWCTL` (Linux 5.10+) | [bluez-l2cap], [bluez-main-conf], [bluez-gatt-characteristic] |

Pairing (Apple guidelines, useful for all hosts): services must be
discoverable without pairing; an accessory that needs a bond rejects an ATT
request with *Insufficient Authentication* and lets the central start pairing
[apple-adg] §58.9–58.10.

### 3.2 GATT vs L2CAP CoC

- **Apple DTS (Argun Tekant, December 2024):** to raise throughput use 2M PHY,
  write without response, a larger MTU, DLE, L2CAP and a 15 ms interval.
  "GATT will have a lot of overhead which you can avoid by using L2CAP", and
  L2CAP also removes GATT's 512-byte attribute limit. iOS does not limit the
  number of writes per interval; the limit is how fast both sides can go. The
  developer in that thread measured about 600 kbit/s to iOS without DLE or
  L2CAP, which DTS called "not a bad number" [apple-forum-770717].
- L2CAP CoC gives a credit-based byte stream with its own flow control. Over
  GATT the app must pace writes (iOS `canSendWriteWithoutResponse`) and frame
  the stream itself.
- **Windows has no L2CAP CoC path** (§3.1), so the protocol must also run over
  GATT (notifications device→host, write-without-response host→device).

### 3.3 Throughput budget

The audio stream is 12 kHz × 16 bit mono = **192 kbit/s = 24,000 bytes/s**, one
direction at a time (`constraints.md` §2). CAT and control share the link. CAT
at 115,200 baud 8N1 is at most 11,520 bytes/s (92 kbit/s) if the radio
streams continuously; normal polling is far less.

Link-layer arithmetic from the Bluetooth Core Specification [bt-core-spec]
(Vol 6 Part B: LE 2M preamble 2 bytes, access address 4, header 2, payload up
to 251 with DLE, MIC 4 on an encrypted link, CRC 3; inter-frame space
T_IFS = 150 µs):

| Quantity | Value |
|---|---|
| Full data PDU on 2M PHY, encrypted (2+4+2+251+4+3 = 266 bytes) | 1,064 µs |
| Empty acknowledging PDU on 2M PHY (2+4+2+3 = 11 bytes) | 44 µs |
| One exchange (PDU + T_IFS + empty PDU + T_IFS) | 1,408 µs |
| L2CAP CoC payload per PDU (251 − 4 basic header − 2 SDU length, one PDU per SDU) | 245 bytes |
| Ceiling for back-to-back PDUs in one direction | about 710 PDUs/s, about **1.39 Mbit/s** |
| PDUs needed for the 192 kbit/s audio alone | 24,000 / 245 ≈ **98 PDUs/s** |
| … at a 15 ms interval (66.7 events/s) | ≥ **2 PDUs per connection event** (about 2.8 ms of air time) |
| … at a 30 ms interval (33.3 events/s) | ≥ **3 PDUs per connection event** (about 4.2 ms) |
| Audio + worst-case CAT (24,000 + 11,520 bytes/s) | ≈ 145 PDUs/s |

GATT notifications carry 3 bytes of ATT header instead of the 2-byte SDU
length, so the per-PDU payload is similar (244 bytes at an MTU of 247 or more).
The difference DTS describes is in the host APIs and flow control, not the
link layer.

So the budget uses about 14 % of the 2M PHY ceiling, but only if the host
schedules **at least 2–3 full PDUs per connection event**. That depends on the
host's controller, its connection event length and other Bluetooth traffic
(DTS: "your app may not even be the only app using Bluetooth")
[apple-forum-770717]. Without DLE (27-byte PDUs) the packet count grows about
tenfold, so **DLE is required** in practice. Measured throughput to each OS is
`needs bench test` (§7).

### 3.4 ESP32-S3 side

- **Radio:** Bluetooth 5 LE with 1M, 2M and Coded PHY, extended advertising and
  multiple connections [esp32s3-ds]. The datasheet lists no isochronous
  channels, which LE Audio needs (§4).
- **L2CAP CoC is in NimBLE, not Bluedroid.** ESP-IDF v6.1 ships NimBLE CoC
  examples (`coc_bleprph`, `coc_blecent`) for the ESP32-S3; CoC is compiled in
  when `BT_NIMBLE_L2CAP_COC_MAX_NUM` > 0 (0–9 channels), with an option for
  enhanced credit-based mode [esp-idf-nimble-coc-readme], [esp-idf-nimble-kconfig].
  The example sends 512-byte SDUs with MTU 512 / MPS 504.
  Bluedroid's `BT_L2CAP_ENABLED` depends on Classic and is "Only supported
  classic bluetooth" [esp-idf-bluedroid-kconfig].
- **Espressif's GATT throughput figures** (ESP32 to ESP32, not to a phone):
  NimBLE, MTU 512, 7.5 ms interval, DLE 251, **1M PHY**: notify about 340 kbit/s,
  write about 500 kbit/s [esp-idf-nimble-throughput]. Bluedroid: "up to
  720–767 Kbps between two ESP32 boards" [esp-idf-bluedroid-throughput].
  L2CAP CoC throughput from an ESP32-S3 is not published: `needs bench test`.
- Because of the L2CAP CoC point, **NimBLE is the recommended host stack** for
  #13 / #16.

### 3.5 Background operation

| Host | Behavior | Sources |
|---|---|---|
| iOS / iPadOS | With the `bluetooth-central` background mode the system wakes the app for Core Bluetooth events. On wake the app has "around 10 seconds" per task, and the system may terminate a background app, dropping its connections. State restoration can reconnect later. Continuous background audio streaming `needs bench test`; plan for foreground use plus automatic reconnection | [apple-cb-background] (archive, 2013-09-18) |
| Android 13–16 | Long-running transfer needs a foreground service of type `connectedDevice` (with a Bluetooth runtime permission); Google suggests the companion device manager for continuous transfer | [android-fgs-types] |
| macOS, Windows, Linux | Desktop apps and services run in the background normally | — |

## 4. LE Audio

| Host | Status | Sources |
|---|---|---|
| iOS / iPadOS, macOS | Apple's accessory guidelines (R31) define no LE Audio profile, and Core Bluetooth has no isochronous-channel API for apps | [apple-adg] |
| Android 13–16 | Built in since Android 13 (API 33) with LC3, **if the phone hardware supports it** (`isLeAudioSupported()`) | [android-le-audio] |
| Windows 11 | Since Windows 11 22H2 (KB5026446), with supporting hardware and drivers | [ms-le-audio] |
| Linux | PipeWire's BlueZ plugin has LE Audio BAP sink/source roles and the LC3 codec | [wireplumber-bluetooth] |

Why it isn't the baseline:

- **The ESP32-S3 can't do it:** no isochronous channels in its Bluetooth 5 LE
  controller [esp32s3-ds].
- Not available on Apple platforms, and hardware-dependent on Android and Windows.
- LE Audio is a system audio profile, so it would bring back the OS voice
  processing and routing that the app-level stream avoids (§2.3), and it
  carries no CAT or PTT.

`constraints.md` §2 already lists LE Audio as not required.

## 5. ChromeOS

Noted only, per the issue. Not assessed in this revision.

## 6. Conclusions

### 6.1 UAC1 vs UAC2 for the device's own sound card (#44)

**Recommendation: UAC1 (USB Audio Class 1.0), full speed, 48 kHz, 16-bit, mono.**

- UAC1 is the only version all five targets document: Android claims only a
  UAC1 subset [android-usb-audio]; Apple [apple-tn3190], Windows
  [ms-usb-classes] and Linux [linux-snd-usb-card] support both.
- UAC2's gains (high-speed bandwidth, clock-source entities, latency and
  connector controls) don't apply to a full-speed ESP32-S3 streaming one
  48 kHz mono channel. That uses about a tenth of UAC1's 1023-byte/ms limit.
- Stay inside Android's subset: PCM Type I, 16-bit, 48 kHz, 1 (or 2) channels.
- **Endpoints:** IN (radio → host) asynchronous, clocked by the codec. For OUT
  (host → radio), choose in #44 between **adaptive** (no feedback endpoint; the
  firmware's rate matcher follows the host rate) and **asynchronous with
  explicit feedback** (the only feedback Windows' UAC2 driver accepts
  [ms-usbaudio2]). Windows `usbaudio.sys` (UAC1) behavior for both
  `needs bench test`.
- **Stack:** TinyUSB 0.21.0 (MIT) lists UAC1 and UAC2 device support
  [tinyusb-readme]. Confirm the TinyUSB version that `esp_tinyusb` pulls in has
  the UAC1 path (verify in #44). TinyUSB also notes a Windows UAC2 driver quirk
  (16.16 feedback format even at full speed) [tinyusb-audio-device-h], one more
  reason to avoid UAC2 here.
- Trade-off accepted: UAC1 has no Terminal Connector control, so iOS/iPadOS
  may route **all** system audio to the device [apple-tn3190]. See §6.3.

### 6.2 Requirements for the firmware and protocol

For **#13 (protocol)**:

- Define the byte stream over two transports: **L2CAP CoC** (iOS/iPadOS,
  macOS, Android 10+, Linux) and **GATT** (notify + write-without-response) for
  Windows and as a fallback. Advertise the CoC PSM in a GATT characteristic so
  apps can find it (Android and Apple both need the PSM from the device
  [android-bluetoothdevice], [apple-cb-openl2cap]).
- Frames must fit a 245-byte PDU payload well, and the protocol must tolerate a
  host that grants fewer packets per connection event (backpressure, audio
  buffer sized for at least one 30 ms interval plus jitter).
- Control, PTT and CAT take priority over audio in the send queue.
- Capability discovery reports transport, negotiated MTU/MPS, PHY and interval
  so apps can show link quality.
- Services readable without pairing; pairing on *Insufficient Authentication*
  [apple-adg] §58.9–58.10.

For **#15 (CAT bridge and PTT fail-safes)**:

- RTS/DTR→PTT mapping must **not key on the line state present at port open**
  (Linux raises DTR and RTS on open [linux-cdc-acm]). Keying needs a state
  change after open, or an explicit protocol command, and mapping is
  configurable per line. Drop PTT when the port closes (Linux clears both),
  on USB suspend or reset, and on disconnect.
- Expose the device's port as class 02h/subclass 02h CDC-ACM with IAD so
  Windows loads `usbser.sys` without an INF [ms-usbser], [ms-usbccgp].

For **#16 (audio pipeline)** and the Bluetooth firmware:

- Use **NimBLE** (L2CAP CoC; Bluedroid's L2CAP is Classic-only)
  [esp-idf-bluedroid-kconfig].
- Request 2M PHY, run the DLE update to 251 before the MTU exchange
  [apple-adg] §58.11, and request 15 ms intervals within Apple's rules
  (min ≥ 15 ms, multiple of 15 ms) [apple-adg] §58.6.
- Budget for ≥ 98 full PDUs/s of audio plus CAT (§3.3).

For **#44 (wired mode)**:

- UAC1 as in §6.1, composite CDC-ACM + UAC1 with IADs, behind the hub.
- Keep the device + hub + radio VBUS draw within 500 mA unless CC advertises
  1.5 A or 3 A; scale back if the advertisement drops [usb-typec-r25] §2.3.4.
- Name the audio function clearly: Apple uses the interface and product strings
  and warns against generic names [apple-tn3190].

### 6.3 User-facing limitations per OS (for the user docs)

- **iPhone / iPad:** wired USB-C gives **audio only**; apps can't reach the
  device's serial port or the radio's USB-serial chip [apple-forum-772812].
  CAT and PTT need Bluetooth, or a VOX/audio-keyed setup. iOS may send
  **all** system sounds to the device while it's connected [apple-tn3190];
  silence notifications while operating (`needs bench test` to confirm).
  Bluetooth works while the app is in the foreground; background streaming may
  be stopped by the system [apple-cb-background]. The iPhone may not power the
  device and a USB-powered radio port (4.5 W figure is for PD charging)
  [apple-iphone-usbc].
- **Android:** wired serial needs an app that supports USB CDC-ACM devices;
  there is no system COM port [android-usb-host]. Bluetooth streaming in the
  background needs the app's foreground service notification [android-fgs-types].
- **Windows:** wired mode is driverless (COM port + sound card). Turn off
  communications effects / Voice Clarity, or use apps that don't tag streams as
  Communications [ms-voice-clarity]. Bluetooth needs host software and runs
  over GATT only; Windows 10 can't request faster connection parameters
  [ms-winrt-ble-connparams].
- **macOS:** wired mode is driverless. Bluetooth needs an app or host software.
- **Linux:** wired mode is driverless (`/dev/ttyACM*`, ALSA/PipeWire card).
  Opening the port raises RTS and DTR (§6.2). Bluetooth needs host software.

### 6.4 Open design question: CAT/PTT for iPhone and iPad in wired mode

ADR-0008 turns Bluetooth off in wired mode, so iOS/iPadOS users would lose CAT
and PTT when wired. Options for the maintainer:

- Allow Bluetooth LE control (CAT/PTT only, no audio) while wired to an Apple
  mobile device.
- Add a USB **CDC-NCM (Ethernet over USB)** function and carry the protocol over
  IP. An Apple DTS engineer calls Ethernet over USB "an incredibly powerful
  option … it supports the iPhone" (June 2026) [apple-forum-747847]; TinyUSB
  supports NCM [tinyusb-readme]. This adds a network function on every OS.
- Accept audio-only wired mode on iOS/iPadOS and document it.

### 6.5 Is an LC3 fallback needed?

**Not as a baseline.** By the arithmetic in §3.3, 12 kHz PCM needs about 14 %
of the 2M PHY ceiling. A developer measured about 600 kbit/s to iOS even
without DLE or L2CAP, a figure Apple DTS called "not a bad number"
[apple-forum-770717]. Keep LC3 (`liblc3`,
Apache-2.0, per ADR-0008) as a contingency for three cases:
a host stuck on 1M PHY with a long interval, Windows GATT throughput, or heavy
radio coexistence. Decide after the bench tests. The effect of LC3 on
weak-signal decoding is untested.

## 7. Items for the bench-test issue (#18)

These can only be settled on hardware. They're listed here for #18, which is a
`human-task` issue and wasn't edited:

1. Sustained L2CAP CoC and GATT throughput, ESP32-S3 (NimBLE, 2M PHY, DLE 251)
   to: iPhone and iPad (iOS/iPadOS 18 and 26), Mac (macOS 15 and 26), Android
   13–16 phones, Windows 10 and 11 (GATT), Linux/BlueZ. Record the negotiated
   PHY, data length, MTU, connection interval and PDUs per event.
2. Same, with the 12 kHz audio stream plus continuous CAT at 115,200 baud.
3. iOS/Android background behavior during a continuous stream.
4. RTS/DTR on port open and close, and `TIOCMSET` / `EscapeCommFunction`
   mapping to `SET_CONTROL_LINE_STATE` on macOS and Windows (Linux from source).
5. UAC1 device enumeration and 48 kHz mono streaming on each OS, including
   Windows `usbaudio.sys` with adaptive and with asynchronous OUT endpoints.
6. The composite CDC + UAC1 device plus the radio's USB device behind the
   on-board hub, on iPhone, iPad, Android, Windows, macOS and Linux.
7. Voice processing on the USB route: iOS mic modes, Android capture sources,
   Windows communications effects and Voice Clarity.
8. iOS/iPadOS audio routing of system sounds to the device.
9. Power available to the device from iPhone, iPad, Android phones and typical
   laptops (Default vs 1.5 A / 3 A advertisement).

<!-- Reference links. Every source is listed in docs/references/manifest.json. -->

[android-bluetoothdevice]: ../references/index.md#android-bluetoothdevice
[android-bluetoothgatt]: ../references/index.md#android-bluetoothgatt
[android-cdd-13]: ../references/index.md#android-cdd-13
[android-cdd-16]: ../references/index.md#android-cdd-16
[android-fgs-types]: ../references/index.md#android-fgs-types
[android-le-audio]: ../references/index.md#android-le-audio
[android-usb-audio]: ../references/index.md#android-usb-audio
[android-usb-host]: ../references/index.md#android-usb-host
[apple-adg]: ../references/index.md#apple-adg
[apple-avaudiosession-measurement]: ../references/index.md#apple-avaudiosession-measurement
[apple-cb-background]: ../references/index.md#apple-cb-background
[apple-cb-openl2cap]: ../references/index.md#apple-cb-openl2cap
[apple-forum-747847]: ../references/index.md#apple-forum-747847
[apple-forum-770717]: ../references/index.md#apple-forum-770717
[apple-forum-772812]: ../references/index.md#apple-forum-772812
[apple-ipad-usbc]: ../references/index.md#apple-ipad-usbc
[apple-iphone-usbc]: ../references/index.md#apple-iphone-usbc
[apple-tn3190]: ../references/index.md#apple-tn3190
[apple-voice-processing]: ../references/index.md#apple-voice-processing
[bluez-gatt-characteristic]: ../references/index.md#bluez-gatt-characteristic
[bluez-l2cap]: ../references/index.md#bluez-l2cap
[bluez-main-conf]: ../references/index.md#bluez-main-conf
[bt-core-spec]: ../references/index.md#bt-core-spec
[esp-idf-bluedroid-kconfig]: ../references/index.md#esp-idf-bluedroid-kconfig
[esp-idf-bluedroid-throughput]: ../references/index.md#esp-idf-bluedroid-throughput
[esp-idf-nimble-coc-readme]: ../references/index.md#esp-idf-nimble-coc-readme
[esp-idf-nimble-kconfig]: ../references/index.md#esp-idf-nimble-kconfig
[esp-idf-nimble-throughput]: ../references/index.md#esp-idf-nimble-throughput
[esp32s3-ds]: ../references/index.md#esp32s3-ds
[linux-cdc-acm]: ../references/index.md#linux-cdc-acm
[linux-snd-usb-card]: ../references/index.md#linux-snd-usb-card
[ms-audio-modes]: ../references/index.md#ms-audio-modes
[ms-le-audio]: ../references/index.md#ms-le-audio
[ms-usb-classes]: ../references/index.md#ms-usb-classes
[ms-usbaudio2]: ../references/index.md#ms-usbaudio2
[ms-usbccgp]: ../references/index.md#ms-usbccgp
[ms-usbser]: ../references/index.md#ms-usbser
[ms-voice-clarity]: ../references/index.md#ms-voice-clarity
[ms-winrt-ble-connparams]: ../references/index.md#ms-winrt-ble-connparams
[ms-winrt-ble-phy]: ../references/index.md#ms-winrt-ble-phy
[ms-winrt-bluetooth]: ../references/index.md#ms-winrt-bluetooth
[ms-winrt-gatt-maxpdu]: ../references/index.md#ms-winrt-gatt-maxpdu
[ms-winrt-serialdevice]: ../references/index.md#ms-winrt-serialdevice
[pipewire-echo-cancel]: ../references/index.md#pipewire-echo-cancel
[tinyusb-audio-device-h]: ../references/index.md#tinyusb-audio-device-h
[tinyusb-readme]: ../references/index.md#tinyusb-readme
[usb-typec-r25]: ../references/index.md#usb-typec-r25
[wireplumber-bluetooth]: ../references/index.md#wireplumber-bluetooth

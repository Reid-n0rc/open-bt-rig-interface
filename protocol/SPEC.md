<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Host-device protocol specification

- **Protocol version:** 0.1.0 (draft; nothing has been released yet)
- **Issue:** [#13](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/13)
- **Decision record:** [ADR-0007](../docs/decisions/ADR-0007-protocol.md)
- **Golden vectors:** [`vectors/`](vectors/)
- **Reference codec:** [`tools/protocol/`](../tools/protocol/) (MIT)

This is the app-neutral protocol between the device and any host
application. It carries CAT serial, PTT and modem lines, audio, configuration,
status, clock sync and optional tone-sequence TX. Any host software may
implement it. Nothing in it depends on a particular host application or
digital mode.

The design basis is [ADR-0008](../docs/decisions/ADR-0008-host-links-esp32-s3.md):
ESP32-S3-MINI-1-N8, **Bluetooth LE and wired USB-C**, no Bluetooth Classic.
The system context (block diagram, data paths, state machines) is in
[`docs/architecture.md`](../docs/architecture.md).

Key words: **must** and **must not** are requirements; **should** is a
recommendation; **optional** marks a feature that capability discovery (§5)
reports. **(verify)** marks a value that bench tests
([#18](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/18)) or the
firmware issues must confirm. Every default in §6.1 was accepted by the
maintainer on 2026-09-24, and each one can be changed with a config key.

## 1. Transports

One **framed byte stream** (§3) carries every message. Three transports carry
the stream:

| Transport | Host link | Used by | Notes |
|---|---|---|---|
| **GATT** | Bluetooth LE | Every host OS; the only choice on Windows, which has no public L2CAP CoC API ([host compatibility §3.1](../docs/research/host-compatibility.md#31-capabilities-per-os)) | Host → device: write without response to **RX**. Device → host: notifications on **TX** (§13) |
| **L2CAP CoC** (optional) | Bluetooth LE | iOS/iPadOS and macOS (`openL2CAPChannel`), Android API 29+, Linux 5.10+ | One LE credit-based channel; PSM read from the Info characteristic (§13.2) |
| **TCP over the USB network** | Wired USB-C | Every host with a driverless USB network (CDC-NCM) class driver, including iPhone and iPad | One TCP connection to the device on its USB network interface ([ADR-0007](../docs/decisions/ADR-0007-protocol.md)); §14.2 |
| **CDC-ACM control port** | Wired USB-C | Desktop hosts and Android apps, when the wired profile includes it | The device's own configuration/PTT serial port ([ADR-0008](../docs/decisions/ADR-0008-host-links-esp32-s3.md)); §14.3 |

Every transport treats the stream as an ordered byte sequence. GATT write and
notification boundaries, L2CAP SDU boundaries, TCP segments and USB packet
boundaries carry **no meaning**: iOS and Android present an L2CAP channel as a byte stream, so a
receiver must not rely on them.

## 2. Conventions

- All multi-byte integers are **little-endian**. `u8`…`u64` are unsigned,
  `i8`/`i16` are two's-complement signed.
- `char` is one ASCII byte. `bytes` and `utf8` take the rest of the payload.
- Times are microseconds (`_us`), milliseconds (`_ms`) or seconds (`_s`).
  **Device time** is a `u64` count of microseconds since the device booted. It
  is monotonic and never set by the host.
- Reserved bits and flags must be sent as 0 and ignored on receipt.
- Enumerated values not listed here are reserved. A device that receives one
  answers `BAD_VALUE` (§12).

## 3. Framing

### 3.1 Frame layout

A **frame** (before encoding) is:

| Offset | Size | Field | Meaning |
|---|---|---|---|
| 0 | 1 | `type` | Message type (§16) |
| 1 | 1 | `token` | Request/response correlation (§4.3) |
| 2 | N | `payload` | Message fields, 0 ≤ N ≤ `max_payload` (§12) |
| 2+N | 2 | `crc` | CRC-16 over `type`, `token` and `payload`, little-endian |

**CRC-16** is CRC-16/IBM-3740, also called CRC-16/CCITT-FALSE
([CRC catalogue](../docs/references/index.md#crc-catalogue-16)): polynomial
0x1021, initial value 0xFFFF, no reflection, no final XOR. Its check value for
the ASCII string `123456789` is 0x29B1.

### 3.2 Encoding on the wire

Each frame is encoded with **Consistent Overhead Byte Stuffing (COBS)**
([Cheshire and Baker](../docs/references/index.md#cobs-paper)) and followed by a
single **0x00** delimiter. COBS output contains no 0x00 bytes, so the delimiter
always marks a frame boundary. The overhead is 1 byte per 254 bytes, plus the
delimiter.

```text
wire = COBS(type | token | payload | crc) | 0x00
```

Example (vector `ptt_on`): `PTT_SET` with token 24 and `state` = 1.
Frame `30 18 01` + CRC; see [`vectors/messages.json`](vectors/messages.json)
for the exact bytes of every message.

### 3.3 Why a delimited stream

- The same bytes work over GATT, L2CAP CoC, TCP and a USB serial port.
- A receiver resynchronizes at the next 0x00 after any loss or corruption, for
  example when a terminal program writes to the wired control port.
- The CRC catches frames that were joined or cut by a lost chunk. The link
  layers below (BLE, USB) have their own CRCs and retransmission; this CRC
  protects the stream as a whole, including host and device buffers.

### 3.4 Receiver rules

A receiver must:

1. Collect bytes until 0x00. An empty frame (two delimiters in a row) is
   ignored. Senders may send a lone 0x00 at any time, for example when a
   session starts, to flush the peer's receive state.
2. COBS-decode, then check the length (at least 4 bytes, and no more than
   `max_payload` + 4) and the CRC. On failure it drops the frame and counts it
   (`STATUS.frame_errors`). The device also sends `RESULT` with `code` =
   `FRAME_ERROR`, `token` 0 and `request_type` 0, at most once per second.
3. Discard a partial frame longer than `max_payload` + 4 bytes (after COBS
   decoding) and wait for the next delimiter.
4. Handle an unknown `type`: the device answers `RESULT` `UNKNOWN_TYPE`; a host
   ignores unknown device → host types (forward compatibility, §4.1).
5. Reject a payload that is shorter than the message's fixed fields
   (`BAD_LENGTH`), or longer, except where a trailing field takes the rest.

### 3.5 Send priority

Frames are never interleaved: each is sent whole. When several are waiting, a
sender should send them in this order: `RESULT`, `PTT_*`, `KEEPALIVE` and other
control messages first, then `CAT_DATA`, then `AUDIO_FRAME`. Audio frames are
small (§9.4) so that control is never delayed by more than about one audio
frame.

## 4. Session, versioning and compatibility

### 4.1 Versioning

- The protocol version is **semver** `major.minor.patch`, released with the tag
  `proto-v<major>.<minor>.<patch>` ([`AGENTS.md`](../AGENTS.md#releases-and-tags)).
  Only the maintainer tags releases ([`GOVERNANCE.md`](../GOVERNANCE.md#release-policy)).
- **Major:** any change that an older peer can misread: a changed field, a
  changed meaning, or a removed message. It needs an ADR.
- **Minor:** backward-compatible additions: new message types, new TLV tags,
  new config keys, new enum values, new flag bits, or fields appended to the
  end of a TLV. Each addition is reported by capability discovery (§5).
- **Patch:** clarifications with no change on the wire.
- **Pre-1.0 (0.x):** the format may still change. Hosts and devices must match
  **major and minor** exactly while the major is 0.
- Every format change updates [`vectors/`](vectors/) in the same PR
  ([`AGENTS.md`](../AGENTS.md#protocol)).

These parts are **frozen across all major versions**, so that any host can
always identify any device: the framing (§3), `HELLO`, `DEVICE_INFO`,
`RESULT`, and the GATT Info characteristic (§13.2).

### 4.2 Session start

1. The host connects (Bluetooth: bonds, then subscribes to TX or opens the
   L2CAP channel; wired: opens the control port or the TCP connection) and
   sends `HELLO`.
2. The device answers `DEVICE_INFO`. It always answers, whatever the host's
   version.
3. If the major versions differ (or, while major is 0, the minors differ), the
   device answers every later request except `HELLO`, `CAPS_GET` and `PING`
   with `RESULT` `VERSION_MISMATCH`. The host should tell the user to update
   the app or the firmware.
4. **Wired hosts** send `AUTH` with their host token and must be approved
   before anything else works (§15.2). Bluetooth hosts are already
   authenticated by their bond.
5. The host sends `CAPS_GET` and uses only what `CAPS` reports.
6. The host reads or sets configuration, opens serial ports, and sets the time
   as needed.

A new `HELLO` restarts the session: the device first does everything it does
when a session ends (§8.7), then answers. The **active transport** is the one
that carried the most recent `HELLO`. The device sends every device → host
message on the active transport only. When the active transport closes, the
session ends.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x01` | `HELLO` | H→D | `proto_major u8`, `proto_minor u8`, `proto_patch u8`, `max_payload u16` (what the host can receive, 256–1024), `flags u8` (reserved) |
| `0x02` | `DEVICE_INFO` | D→H | `proto_major u8`, `proto_minor u8`, `proto_patch u8`, `fw_major u8`, `fw_minor u8`, `fw_patch u8`, `variant char` (`R`, `M`, …), `hw_revision char` (`A`, …), `max_payload u16`, `build utf8` (free text, at most 32 bytes) |

### 4.3 Tokens and replies

- `token` 1–255: the host expects a reply. The device sends exactly one reply
  with the same token: the message named in the request's description, or
  `RESULT`.
- `token` 0: no reply on success. Errors are still reported, with `token` 0.
  Streams (`CAT_DATA`, `AUDIO_FRAME`, `MODEM_LINES`, `KEEPALIVE`) normally use 0.
- Device-initiated messages (notifications) use `token` 0.
- A host should not reuse a token until its reply has arrived or 1 s has
  passed. The device should reply within 100 ms **(verify)**.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x05` | `RESULT` | D→H | `request_type u8` (the type being answered, or 0), `code u8` (§12) |
| `0x06` | `PING` | H→D | `data bytes` (0–64 bytes, opaque). Reply: `PONG` |
| `0x07` | `PONG` | D→H | `data bytes` (the same bytes) |

## 5. Capability discovery

`CAPS` lists everything the device supports, as TLVs (`tag u8`, `len u8`,
`value`). A host must skip unknown tags by their length, and must accept a
known tag that is longer than it expects (a newer minor version may append
fields) by reading the fields it knows. A tag may appear more than once (for
example one `RADIO_USB_SERIAL` per port).

The device resends `CAPS` with token 0 whenever it changes, for example when a
radio USB-serial chip or sound card enumerates or disappears.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x03` | `CAPS_GET` | H→D | none. Reply: `CAPS` |
| `0x04` | `CAPS` | D→H | `tlvs`: a list of TLVs |

### 5.1 TLVs

| Tag | Name | Value |
|---|---|---|
| 0x01 | `FEATURES` | `features u32`, bits below. Always present. |
| 0x02 | `SERIAL_JACK` | `port u8` (0), `modes u8` (bit *n* = SERIAL-jack mode *n* of §6.2 is supported), `min_baud u32`, `max_baud u32`, `tx_buffer u16` (initial CAT credit, §7.4) |
| 0x03 | `RADIO_USB_SERIAL` | `port u8` (1–4), `chip u8` (0 other, 1 CP210x, 2 FTDI, 3 CH34x, 4 CDC-ACM), `vid u16`, `pid u16`, `interface u8` (USB interface number), `tx_buffer u16` |
| 0x04 | `AUDIO` | `directions u8` (bit 0 RX, bit 1 TX), `codecs u8` (bit 0 PCM16, bit 1 LC3), `rates u8` (bit 0 12 000 Hz, bit 1 16 000 Hz), `paths u8` (bit 0 analog codec, bit 1 radio USB sound card present now), `max_frame_samples u16`, `tx_buffer_samples u16` |
| 0x05 | `PTT` | `outputs u8` (bit 0 AUDIO-jack closure, bit 1 RTS on a radio USB-serial port, bit 2 DTR on a radio USB-serial port), `keepalive_min_ms u16`, `keepalive_max_ms u16`, `max_tx_min_s u32` (the smallest `MAX_TX_S`; there is no upper bound) |
| 0x06 | `TONE` | `max_symbols u16`, `max_tone_index u8`, `shaping u8` (bit 0 phase-continuous FSK, bit 1 GFSK), `min_symbol_us u32`, `max_symbol_us u32` |
| 0x07 | `BLE_TX_POWER` | `min_dbm i8`, `max_dbm i8`. `max_dbm` is the firmware cap (§6.3) |
| 0x08 | `PAIRING` | `triggers u8` (the local actions that open the pairing window: bit 0 power-on, bit 1 pairing button), `window_min_s u16`, `window_max_s u16`, `max_bonds u8` (bonded hosts the device can store), `max_wired_hosts u8` (approved wired hosts it can store) (§13.5, §15) |

### 5.2 Feature bits

| Bit | Feature | Detail |
|---|---|---|
| 0 | `CAT` | CAT over the protocol (§7) |
| 1 | `PTT_CLOSURE` | AUDIO-jack PTT closure |
| 2 | `AUDIO_RX` | Radio → host audio over the protocol (§9) |
| 3 | `AUDIO_TX` | Host → radio audio over the protocol |
| 4 | `AUDIO_LC3` | LC3 codec (optional, §9.2) |
| 5 | `CLOCK_SYNC` | `TIME_*` messages (§10) |
| 6 | `TONE_SEQUENCE` | Optional tone-sequence TX module (§11) |
| 7 | `L2CAP_COC` | L2CAP CoC transport (§13.3) |
| 8 | `MODEM_STATUS` | `MODEM_STATUS` inputs from radio USB-serial chips |
| 9 | `BLE_TX_POWER` | `BLE_TX_POWER` config key |
| 10 | `RADIO_USB_HOST` | Radio USB port (USB host) |
| 11 | `WIRED_MODE` | Wired USB-C host link (§14) |
| 12 | `RX_ATTENUATOR` | Switchable AUDIO-jack RX attenuator |
| 13 | `CONFIG_PERSIST` | Configuration survives power cycles |
| 14 | `SCHEDULED_AUDIO` | `AUDIO_START.start_time_us` (§9.3) |
| 15 | `FIRMWARE_UPDATE` | Reserved; always 0 in 0.1 (§15.4) |
| 16 | `USB_NETWORK` | Wired USB network interface and TCP transport (§14.2) |
| 17 | `PAIRING_WINDOW` | Pairing window (§13.5), `PAIRING` TLV |

A message that belongs to an absent feature is answered with `RESULT`
`UNSUPPORTED`.

## 6. Configuration

Configuration is a set of typed keys. Each key has a `selector` byte, which
picks an instance (a port number for `LINE_MAP`) and is 0 otherwise.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x10` | `CONFIG_GET` | H→D | `key u8`, `selector u8`. Reply: `CONFIG` |
| `0x11` | `CONFIG_SET` | H→D | `flags u8` (bit 0 persist), `key u8`, `selector u8`, `value`. Reply: `CONFIG` with the value now in effect, or `RESULT` |
| `0x12` | `CONFIG` | D→H | `key u8`, `selector u8`, `value`. Also sent with token 0 when a value changes for another reason |
| `0x13` | `CONFIG_RESET` | H→D | `flags u8` (bit 0 persist). Restores every key to its default. Reply: `RESULT` |

Without the persist flag a value lasts until the device resets. Persisting
writes flash, so hosts should persist rarely; the device may answer
`RATE_LIMITED` to more than one persisted write per second.

### 6.1 Keys

| Key | Name | Selector | Value | Default |
|---|---|---|---|---|
| 0x01 | `SERIAL_JACK_MODE` | 0 | `mode u8` (§6.2) | 0 (3.3 V logic) |
| 0x02 | `PTT_TARGETS` | 0 | `targets u8` (bit 0 AUDIO-jack closure, bit 1 RTS, bit 2 DTR on radio USB-serial port `usb_port`), `usb_port u8` (1–4, or 0 when bits 1–2 are clear) | closure only (0x01, 0) |
| 0x03 | `LINE_MAP` | port (0–4, or 0x0F for the wired control port) | `rts_action u8`, `dtr_action u8`: 0–2 (§8.3); action 2 only on ports 1–4 | port 0 and 0x0F: RTS → PTT, DTR ignored; ports 1–4: both pass-through |
| 0x04 | `PTT_KEEPALIVE_MS` | 0 | `ms u16`: 500–10 000 (the device reports its limits in the `PTT` TLV) | 3000 |
| 0x05 | `MAX_TX_S` | 0 | `s u32`: **0 disables the timer**; otherwise at least `PTT.max_tx_min_s` (10), with **no upper bound** (maintainer decisions, 2026-09-25). Values from 1 to `max_tx_min_s` − 1 are refused with `OUT_OF_RANGE` | 300 |
| 0x06 | `AUDIO_PATH` | 0 | `path u8`: 0 automatic (radio USB sound card when present), 1 analog, 2 radio USB | 0 |
| 0x07 | `TX_LEVEL` | 0 | `centibel i16`: TX audio level in 0.1 dB, ≤ 0 (0 = full-scale line level) | set in #16 |
| 0x08 | `RX_ATTENUATOR` | 0 | `on u8` (the about 19 dB AUDIO-jack pad, [radio-connectors](../docs/requirements/radio-connectors.md#audio-jack-35-mm-trrs)) | 0 |
| 0x09 | `RX_GAIN` | 0 | `centibel i16`: RX gain in 0.1 dB | set in #16 |
| 0x0A | `HOST_MODE` | 0 | `mode u8`: 0 automatic, 1 force wired, 2 force Bluetooth. Takes effect at the next mode evaluation ([architecture §6](../docs/architecture.md#6-host-link-and-connection-state-machine)) | 0 |
| 0x0B | `BLE_TX_POWER` | 0 | `dbm i8` (§6.3) | the cap |
| 0x0C | `WIRED_PROFILE` | 0 | `profile u8`: 0 network, 1 serial. Picks the USB functions for radios whose serial is on the SERIAL jack (§14.1). Takes effect at the next enumeration | 1 (serial) |
| 0x0D | `SERIAL_DEFAULT` | port (0–4) | `baud u32` (the port's `min_baud`–`max_baud`), `data_bits u8`, `parity u8`, `stop_bits u8` (values as in `SERIAL_SET`, §7.1). Applied when the port is opened (§7.2), and to the wired SERIAL-jack bridge port until the host sets its own line coding | 9600 8N1 |
| 0x0E | `USB_NET_SUBNET` | 0 | `a u8`, `b u8`, `c u8`, `d u8`: the network address of the USB network /30, in address order (`a.b.c.d`). Must be a private IPv4 address (10/8, 172.16/12 or 192.168/16) with `d` a multiple of 4; the device takes `d`+1, the host gets `d`+2 (§14.2). Takes effect at the next enumeration | 10.169.160.0 |
| 0x0F | `PAIRING_WINDOW_S` | 0 | `s u16`: how long the pairing window stays open, 30–600 (the device reports its limits in the `PAIRING` TLV) (§13.5) | 120 |
| 0x10 | `POWER_DOWN_DELAY_S` | 0 | `s u16`: how long after the radio (or, on variant M, the ignition) turns off the device powers itself down: 5–3600, or 0 = never. With 0, variant M can exceed its off-state drain target (REQ-PWR-016); that is the user's choice. The delay runs only while no USB host is connected on USB-C: the device stays awake in wired mode. Powering down ends the session first, which turns PTT off (§8.7) (REQ-PWR-018) | 30 |

### 6.2 SERIAL-jack modes

From [`radio-connectors.md`](../docs/requirements/radio-connectors.md#serial-jack-35-mm-trrs):
0 = 3.3 V logic (power-on default), 1 = RS-232 levels, 2 = Icom CI-V
(single-wire bus), 3 = 3.3 V logic with 3.3 V out on ring 2. Changing the mode
never asserts PTT (REQ-PTT-009).

### 6.3 BLE TX power cap

The module's FCC grant covers Bluetooth LE at **10.3 dBm conducted** at most,
and the firmware caps BLE TX power at or below it
([`fcc.md` §1.3](../docs/compliance/fcc.md#13-rf-configuration-firmware)).
The cap is a firmware constant. The protocol can only **lower** the power:

- `CAPS` `BLE_TX_POWER.max_dbm` reports the cap, rounded down to a level the
  radio supports (for example +9 dBm **(verify the ESP-IDF levels)**).
- `CONFIG_SET` `BLE_TX_POWER` above `max_dbm` or below `min_dbm` is refused
  with `OUT_OF_RANGE`; the power doesn't change. No message, flag or key can
  raise the power above the cap.
- A firmware test checks this (REQ-REG-002, [`fcc.md`](../docs/compliance/fcc.md)).

## 7. Serial ports and CAT

The device never interprets CAT bytes (REQ-CAT-001). It moves them unchanged
between the host and a **port**:

| Port | What it is | Present when |
|---|---|---|
| 0 | The SERIAL jack, in the mode of §6.2 | Always (`SERIAL_JACK` TLV) |
| 1–4 | USB-serial interfaces of the radio, on the radio USB port. A dual-port chip (for example a CP2105) appears as two ports, one per USB interface, in interface-number order | Bluetooth mode, while the radio's chip is enumerated (`RADIO_USB_SERIAL` TLVs) |
| 0x0F | The wired control port's own RTS/DTR (§14); no data | Wired mode only |

`CAPS` reports each radio port's chip, VID, PID and interface number, so a
host can tell the user which port is which, for example which interface of a
dual-port chip carries CAT on their radio. The protocol doesn't encode that
per-radio knowledge.

In wired mode the host drives the radio's own USB-serial chip directly
through the hub, so ports 1–4 don't exist. Port 0 is either the device's
CDC-ACM bridge port (serial profile), or reached with these messages over the
USB network (network profile, §14.1)
([architecture §4](../docs/architecture.md#4-data-paths)).

### 7.1 Messages

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x20` | `SERIAL_OPEN` | H→D | `port u8`, `open u8` (1 open, 0 close). Reply: `RESULT` |
| `0x21` | `SERIAL_SET` | H→D | `port u8`, `baud u32`, `data_bits u8` (7 or 8), `parity u8` (0 none, 1 odd, 2 even, 3 mark, 4 space), `stop_bits u8` (0 one, 1 one and a half, 2 two). Reply: `RESULT` |
| `0x22` | `CAT_DATA` | both | `port u8`, `data bytes` (1 to `max_payload` − 1 bytes) |
| `0x23` | `CAT_CREDIT` | D→H | `port u8`, `credit u16`: bytes the host may send in addition to its current credit |
| `0x24` | `MODEM_LINES` | H→D | `port u8`, `lines u8` (bit 0 DTR, bit 1 RTS; 1 = asserted) |
| `0x25` | `MODEM_STATUS` | D→H | `port u8`, `lines u8` (bit 0 CTS, bit 1 DSR, bit 2 DCD, bit 3 RI), sent on change. Optional (`MODEM_STATUS` feature) |

### 7.2 Opening and line settings

- A port must be opened before `SERIAL_SET`, `CAT_DATA` or `MODEM_LINES`;
  otherwise `STATE_ERROR`.
- Opening a port resets its CAT credit to its `tx_buffer`, and restarts RTS/DTR
  arming (§8.4).
- Closing a port (or ending the session) deasserts its RTS and DTR, releases
  any PTT source that came from it, and discards unsent bytes.
- `SERIAL_SET` applies the settings to the SERIAL jack's UART, or to the
  radio's USB-serial chip (REQ-CAT-004). Supported range: `min_baud` to
  `max_baud` of the port; 4800 to 115 200 at least on port 0 (REQ-CAT-002).
  After `SERIAL_OPEN` the port uses its `SERIAL_DEFAULT` settings
  (default 9600 8N1).

### 7.3 Chunking

A `CAT_DATA` frame carries up to `max_payload` − 1 bytes. The device sends
bytes from the radio as they arrive, and should batch them for up to 2 ms
**(verify)** to fill frames without adding noticeable latency (REQ-CAT-006).
The frame is then split into GATT writes or notifications of at most
ATT_MTU − 3 bytes, or L2CAP SDUs (§13).

### 7.4 Flow control

- **Host → device:** credit-based, per port. After `SERIAL_OPEN` the host has
  `tx_buffer` bytes of credit (from `CAPS`). Each `CAT_DATA` byte uses one
  byte. The device sends `CAT_CREDIT` as bytes leave toward the radio. A host
  must not exceed its credit. Excess bytes are dropped and reported with
  `RESULT` `OVERFLOW`. This keeps a slow radio (4800 baud) from overrunning the
  device (REQ-CAT-005).
- **Device → host:** no credit. The radio's rate (at most 11 520 bytes/s at
  115 200 baud) is well within the link budget
  ([host compatibility §3.3](../docs/research/host-compatibility.md#33-throughput-budget)).
  If the device's buffer toward the host fills anyway, it drops the newest
  bytes and counts them in `STATUS.cat_overflows`.
- L2CAP CoC credits and GATT pacing (iOS `canSendWriteWithoutResponse`) sit
  underneath and are independent of this.

## 8. PTT and modem lines

PTT safety is the most important part of this protocol
([`GOVERNANCE.md`](../GOVERNANCE.md#scope-and-principles)). The rules here are
the protocol's side of REQ-PTT-001 to REQ-PTT-011. The device-side state
machine is in [architecture §5](../docs/architecture.md#5-ptt-safety-state-machine).
The maintainer approved this section on 2026-09-24, and amended the max-TX
timer (no upper bound, can be disabled) and the lock-up handling on
2026-09-25
([ADR-0007](../docs/decisions/ADR-0007-protocol.md)). Later changes need the
maintainer's explicit approval again
([`GOVERNANCE.md`](../GOVERNANCE.md#safety-and-compliance)).

### 8.1 PTT sources and outputs

The device keeps one **logical PTT**, which is on while any source is active
and no fail-safe blocks it:

| Source | `sources` bit | Keepalive needed |
|---|---|---|
| `PTT_SET` state 1 | 0 | Yes |
| A host RTS/DTR line mapped to PTT (`MODEM_LINES` over Bluetooth) | 1 | Yes |
| A tone sequence with device-keyed PTT (§11) | 2 | Yes |
| A native RTS/DTR line on a wired CDC-ACM port mapped to PTT (§14) | 3 | No (USB state covers it) |
| A pass-through RTS/DTR line on a radio USB-serial port (§8.3) | 4 | Yes |

Logical PTT drives the outputs chosen by `PTT_TARGETS`: the AUDIO-jack closure,
and/or RTS or DTR on one radio USB-serial port. A CAT command sent in
`CAT_DATA` can also key the radio; the device can't see that (it doesn't parse
CAT), so the radio's own timers are the only guard for CAT keying.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x30` | `PTT_SET` | H→D | `state u8` (0 release, 1 key). Reply: `PTT_STATUS` |
| `0x31` | `PTT_STATUS` | D→H | `state u8` (logical PTT), `sources u8` (active sources, bits above), `reason u8` (§8.6), `remaining_s u32` (max-TX time left; 0 when off or when the timer is disabled). Sent on every change and as the reply to `PTT_SET` |
| `0x32` | `KEEPALIVE` | H→D | none. Refreshes the keepalive of every source that needs one |

### 8.2 Keepalive

- While any source that needs a keepalive is active, the host must send
  `KEEPALIVE` (or repeat `PTT_SET` state 1, or `MODEM_LINES`) at least every
  `PTT_KEEPALIVE_MS` / 3. Defaults: timeout 3000 ms, host interval
  1000 ms; range 500–10 000 ms (`PTT_KEEPALIVE_MS`, §6.1).
- If `PTT_KEEPALIVE_MS` passes with none of these, the device releases every
  keepalive source (bits 0, 1, 2 and 4), deasserts pass-through lines, and
  sends `PTT_STATUS` with reason `KEEPALIVE_TIMEOUT`.
- Rationale: a host app that hangs while the Bluetooth link stays up must not
  hold PTT (REQ-PTT-006). The keepalive is shorter than a typical BLE
  supervision timeout, so it also catches a failing link sooner.
- Only these three messages count. Audio or CAT traffic doesn't: a stalled
  control thread must drop PTT even if an audio thread still runs.

### 8.3 RTS/DTR mapping

`LINE_MAP` sets, per port, what each host line does:

| Action | Meaning | Allowed on |
|---|---|---|
| 0 | Ignore | Any port |
| 1 | PTT: the line is a PTT source (arming rules §8.4) | Port 0, port 0x0F, radio ports 1–4 |
| 2 | Pass-through: the line goes to the radio's USB-serial chip, as with a direct cable. The device treats an asserted pass-through line as a possible PTT (the radio may key on it): it needs the keepalive, counts toward max TX, and drops at session end | Radio ports 1–4 only |

The SERIAL jack has no RTS/DTR contacts, so action 2 isn't available on port 0
([radio-connectors](../docs/requirements/radio-connectors.md#serial-jack-35-mm-trrs)).
Over Bluetooth, the host sends line state in `MODEM_LINES`; in wired mode it is
the native CDC-ACM `SET_CONTROL_LINE_STATE` state (REQ-PTT-002).

### 8.4 Arming: opening a port must never key

**Hazard:** Linux raises DTR and RTS when a CDC-ACM port is opened, and drops
both when it closes
([host compatibility §1](../docs/research/host-compatibility.md#1-wired-usb-c-usb-serial-cdc-acm-and-rtsdtr)).
A host bridge that exposes a virtual serial port may do the same over
Bluetooth. Other OSes' behavior at open is unconfirmed **(verify, #18)**. A
program that merely opens the port to send CAT must not key the transmitter.

So each line with action 1 has its own state:

```text
BLOCKED  --line seen deasserted-->                        ARMED
ARMED    --line rises, the other line doesn't rise too--> KEYING (PTT source active)
ARMED    --line and the other line rise together-->       BLOCKED (a port open)
KEYING   --line deasserted-->                             ARMED
any      --session start, SERIAL_OPEN, USB reset/configure,
           keepalive timeout, max TX, port close-->       BLOCKED
```

- A line starts **BLOCKED**, and keys only after it has been seen deasserted
  and then rises **on its own**. When both lines rise in the same update, the
  device treats it as a port open, not a key.
- Software that keys with RTS normally drops RTS after opening and raises it
  to transmit, so it keeps working. Software that never touches RTS leaves it
  high after open, and never keys.
- Pass-through lines (action 2) are not armed this way, so that the radio's
  chip sees what a direct cable would show. The keepalive and max-TX guards
  still apply.

### 8.5 Maximum TX time and other fail-safes

- **Maximum TX:** a timer starts when logical PTT turns on, from any source.
  At `MAX_TX_S` the device releases every source, deasserts pass-through lines
  and sends `PTT_STATUS` reason `MAX_TX`. PTT then stays **locked out** until
  every source has been released (for example `PTT_SET` 0 and the mapped
  lines deasserted); a key attempt meanwhile gets `PTT_STATUS` reason
  `LOCKED_OUT`. Default 300 s, minimum 10 s, no upper bound; the user can
  disable the timer with `MAX_TX_S` = 0 (REQ-PTT-007, §6.1). The other
  fail-safes (keepalive, session end, watchdog reset) still apply when it is
  disabled.
- **Firmware lock-up:** there is no separate hardware PTT timer. The
  ESP32-S3's internal watchdog resets the device within a few seconds of a
  firmware lock-up. PTT is off during the reset (the output is off unless
  actively driven) and after it (power-on state). After such a reset the
  device reports `PTT_STATUS` reason `WATCHDOG` to the next session
  (REQ-PTT-011).
- **PTT is off** at power-on, reset, brownout, watchdog timeout, session end,
  loss of the active transport or the Bluetooth link, USB-C unplug, suspend
  or reset in wired mode, and host-mode change (REQ-PTT-005). After any of
  these, every line is BLOCKED.
- Changing `SERIAL_JACK_MODE`, `AUDIO_PATH`, `PTT_TARGETS`, `LINE_MAP` or
  `HOST_MODE` never asserts PTT (REQ-PTT-009). A change to `PTT_TARGETS` or
  `LINE_MAP` while keyed releases PTT first.
- The hardware keeps PTT off unless actively driven (REQ-PTT-008); the
  protocol doesn't rely on the firmware alone.

**Notes for users** (known behaviour, not extra rules):

- **Wired RTS/DTR has no keepalive.** A desktop program that hangs while it
  holds RTS or DTR on a wired CDC-ACM port keeps PTT keyed until `MAX_TX_S`
  expires or the USB link goes away; with the max-TX timer disabled, only
  the USB link going away ends it.
- **CAT keying is invisible to the device.** PTT keyed by a CAT command in the
  byte stream isn't seen by the device (§8.1), so only the radio's own timers
  guard it.
- **Raising RTS and DTR together doesn't key.** The device treats both lines
  rising in the same update as a port open (§8.4). Software that keys by
  raising both at once won't key; map only one line to PTT (`LINE_MAP`) and
  use that line.

### 8.6 PTT reason codes

| Code | Name | Meaning |
|---|---|---|
| 0 | `NONE` | No change of note (for example keyed normally) |
| 1 | `RELEASED` | Every source was released by the host |
| 2 | `KEEPALIVE_TIMEOUT` | §8.2 |
| 3 | `MAX_TX` | §8.5 |
| 4 | `LINK_LOST` | The transport or the Bluetooth link closed, or the session restarted |
| 5 | `PORT_CLOSED` | The port that carried the source closed (including a CDC-ACM line drop at close) |
| 6 | `MODE_CHANGE` | Host-mode switch, or a PTT-related configuration change |
| 7 | `NOT_ARMED` | A key attempt from a BLOCKED line, ignored (§8.4); informational |
| 8 | `LOCKED_OUT` | A key attempt during max-TX lockout, refused |
| 9 | `FAULT` | Brownout, watchdog or another internal fault |
| 10 | `SEQUENCE_DONE` | A tone sequence ended and released PTT (§11) |
| 11 | `BOOT` | First status after power-on or reset |
| 12 | `WATCHDOG` | The internal watchdog reset the device after a firmware lock-up, so PTT went off (§8.5); reported after the reset |

### 8.7 Session end

When a session ends (§4.2), the device: turns PTT off and blocks every line;
stops audio; closes every port (deasserting their lines); cancels any tone
sequence; and forgets CAT credit. It keeps the configuration and the time
mapping (§10.3).

## 9. Audio over Bluetooth LE

Bluetooth LE audio uses the protocol stream (GATT or L2CAP CoC). Wired mode
uses USB Audio Class instead, and these messages are then refused with
`UNSUPPORTED` (§14).

### 9.1 Stream rules

- **One direction at a time** (REQ-AUD-004): `AUDIO_START` for one direction
  while the other runs is refused with `BUSY`.
- **The device's sample clock is the stream clock** (±50 ppm, REQ-AUD-011).
  The host adapts to it, using the TX fill level (§9.5) or the RX arrival rate.
  The device does its own rate matching toward a radio USB sound card
  (REQ-AUD-008).
- The audio source is the path in `AUDIO_PATH`; `STATUS.audio_path` reports the
  one in use.

### 9.2 Formats

| `codec` | Format | `codec_param` | Status |
|---|---|---|---|
| 0 | PCM, signed 16-bit little-endian, mono | 0 | Required when `AUDIO_*` is supported |
| 1 | LC3, one frame per `AUDIO_FRAME` | Bytes per LC3 frame | Optional (`AUDIO_LC3`), a fallback if PCM doesn't fit a host's measured throughput (REQ-AUD-005) **(verify, #16)** |

Rates: 12 000 Hz (at least) and 16 000 Hz, as reported in `AUDIO.rates`.
12 kHz PCM is 192 kbit/s, which needs about 98 full 2M-PHY packets per second
([host compatibility §3.3](../docs/research/host-compatibility.md#33-throughput-budget));
measured throughput per OS is **(verify, #18)**.

### 9.3 Messages

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x40` | `AUDIO_START` | H→D | `direction u8` (1 RX = radio → host, 2 TX = host → radio), `codec u8`, `sample_rate u16` (Hz), `frame_samples u16`, `codec_param u16`, `start_time_us u64` (device time; 0 = now). Reply: `AUDIO_STATUS` |
| `0x41` | `AUDIO_STOP` | H→D | `direction u8`. Reply: `AUDIO_STATUS` |
| `0x42` | `AUDIO_FRAME` | both | `seq u16`, `timestamp u32`, `flags u8` (bit 0 discontinuity), `data bytes` |
| `0x43` | `AUDIO_STATUS` | D→H | `direction u8`, `state u8` (0 stopped, 1 waiting to start, 2 running, 3 underrun), `fill_samples u16`, `target_samples u16`, `underruns u16`, `overruns u16`, `first_sample_time_us u64` (device time of sample 0; 0 = not yet) |

- `seq` counts frames from 0 at `AUDIO_START`, wrapping at 65 536.
- `timestamp` is the index of the frame's first sample, from 0 at
  `AUDIO_START`, wrapping at 2³². Compare both modulo their range.
- Sample *n* is captured or played at `first_sample_time_us` + *n* / rate (in
  device time). This fixes the latency (REQ-AUD-015) and lets a host align TX
  audio to UTC through clock sync (§10).
- **Scheduled start (optional, `SCHEDULED_AUDIO`):** for TX, `start_time_us`
  sets when sample 0 plays; the host sends frames ahead of time. For RX it
  must be 0 in 0.1.

### 9.4 Frame size

`frame_samples` should be 10 ms of audio (120 samples at 12 kHz, 240 bytes of
PCM), and must not exceed `AUDIO.max_frame_samples` or fit a payload larger
than `max_payload`. A 10 ms frame fits in two full 2M-PHY packets and keeps
control latency low (§3.5).

### 9.5 Flow control and gaps

- **TX (host → device):** the device plays sample *n* at its fixed time. It
  sends `AUDIO_STATUS` about every 100 ms while TX runs. The host keeps
  `fill_samples` near `target_samples` (the device's jitter buffer, for example
  60 ms **(verify, #16)**). Frames that arrive after their play time are
  dropped. An empty buffer plays silence, counts an underrun and sets `state` 3
  until audio resumes. Frames beyond `tx_buffer_samples` are dropped and
  counted in `overruns`.
- **RX (device → host):** if the link can't keep up, the device drops whole
  frames, oldest first, and sets the discontinuity flag on the next frame it
  sends.
- **Receiver gap handling (both sides):** a `timestamp` beyond the expected one
  is a gap. The receiver fills it with silence (PCM zeros), up to 200 ms; a
  larger gap, or a timestamp behind the expected one, is a resync: the
  receiver restarts its timeline at the new frame. The discontinuity flag
  tells the receiver that the sender itself lost samples.
- L2CAP CoC and GATT flow control sit underneath (§7.4).

## 10. Clock sync

Needed for scheduled audio (§9.3) and tone sequences (§11), which must start
within about ±20 ms of UTC (REQ-TIM-002). Optional (`CLOCK_SYNC`).

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x50` | `TIME_REQ` | H→D | `host_t1 u64` (host time, opaque to the device) |
| `0x51` | `TIME_RESP` | D→H | `host_t1 u64` (echoed), `device_t2 u64` (device time the request was received), `device_t3 u64` (device time the reply was sent) |
| `0x52` | `TIME_SET` | H→D | `device_time_us u64`, `utc_us u64` (microseconds since 1970-01-01 UTC at that device time), `uncertainty_us u32`. Reply: `RESULT` |

### 10.1 Offset and round-trip time

For each exchange, with `t4` the host time the reply arrived:

```text
rtt    = (t4 - t1) - (t3 - t2)
offset = ((t2 - t1) + (t3 - t4)) / 2      # device time - host time
error  ≤ rtt / 2
```

### 10.2 RTT filtering

Over Bluetooth, replies wait for connection events, so RTT varies by one or
more connection intervals (15 ms or more on Apple hosts,
[host compatibility §3.1](../docs/research/host-compatibility.md#31-capabilities-per-os)).
The host should:

1. Send a burst of 8–16 `TIME_REQ`, at least 50 ms apart (at most 10 per
   second).
2. Keep the sample with the **smallest RTT**, and discard any sample whose RTT
   is more than twice the smallest.
3. Use that sample's offset; its `rtt / 2` is the uncertainty.
4. Repeat every 60 s or so. It may fit a line through the kept offsets to
   estimate the drift between the clocks.

Whether this reaches ±20 ms on every OS is **(verify, #18)**.

### 10.3 Setting UTC

The host converts its UTC clock to device time with the offset and sends
`TIME_SET`. The device keeps the mapping across sessions, reports it with
`STATUS` flag bit 3, and treats it as stale after 10 minutes without a new
`TIME_SET` **(verify)**. A UTC-scheduled request without a fresh mapping gets
`STATE_ERROR`.

## 11. Scheduled tone-sequence TX (optional)

An optional firmware module (REQ-TIM-003, [#17](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/17))
that plays a list of audio tones on the TX audio path, starting at a UTC
time. It is generic multi-tone FSK: any mode that sends one tone per symbol
from a fixed set can use it. Hosts that would rather send audio can use
scheduled audio (§9.3) instead: that is the "raw sample schedule" option.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x60` | `TONE_SETUP` | H→D | `count u16` (symbols), `base_mhz u32` (audio frequency of tone 0, in millihertz), `spacing_mhz u32` (tone spacing, millihertz), `symbol_us u32`, `shaping u8` (0 phase-continuous FSK, 1 GFSK), `bt_x100 u8` (GFSK bandwidth-time product × 100), `amplitude u16` (0–65 535, relative to `TX_LEVEL`), `ramp_us u16` (amplitude ramp at start and end), `ptt_lead_ms u16`, `ptt_tail_ms u16`, `flags u8` (bit 0 the device keys PTT). Reply: `RESULT` |
| `0x61` | `TONE_DATA` | H→D | `offset u16`, `tones bytes` (one tone index per symbol). Reply: `RESULT` |
| `0x62` | `TONE_START` | H→D | `start_utc_us u64` (0 = now). Reply: `TONE_STATUS` |
| `0x63` | `TONE_CANCEL` | H→D | none. Reply: `TONE_STATUS` |
| `0x64` | `TONE_STATUS` | D→H | `state u8` (0 idle, 1 loaded, 2 scheduled, 3 PTT lead, 4 playing, 5 PTT tail, 6 done, 7 cancelled), `symbol u16` (current symbol), `reason u8` (a §8.6 code when a fail-safe ended it, else 0). Sent on each state change |

- Symbol *k* plays at `base_mhz` + `tones[k]` × `spacing_mhz`, for `symbol_us`,
  with continuous phase. Tone indices must not exceed `TONE.max_tone_index`,
  and `count` must not exceed `TONE.max_symbols`.
- `TONE_DATA` loads tones in pieces; `TONE_START` is refused with
  `STATE_ERROR` until all `count` tones are loaded, or without a fresh UTC
  mapping.
- With `flags` bit 0 the device keys PTT `ptt_lead_ms` before the start and
  releases it `ptt_tail_ms` after the last symbol (reason `SEQUENCE_DONE`).
  Device-keyed PTT is PTT source bit 2: **the keepalive (§8.2) and max TX
  (§8.5) apply**, and a session end cancels the sequence. Without bit 0 the
  host keys PTT itself.
- The sequence can't run while `AUDIO_START` TX is active (`BUSY`).

*Example (not a requirement):* an FT8-like transmission would use 79 symbols,
8 tones, 6.25 Hz spacing, 160 ms symbols and GFSK with BT 2.0 **(verify
against the mode's own specification)**. Vector `tone_setup_ft8_example` shows
it with a 1500 Hz base tone.

## 12. Errors and limits

### 12.1 Result codes

| Code | Name | Meaning |
|---|---|---|
| 0 | `OK` | Success |
| 1 | `UNKNOWN_TYPE` | Message type not known |
| 2 | `BAD_LENGTH` | Payload too short or too long for the type |
| 3 | `BAD_VALUE` | A field has a reserved or invalid value |
| 4 | `UNSUPPORTED` | The feature is absent, or unavailable in this host mode |
| 5 | `BUSY` | Conflicts with something running (for example audio in the other direction) |
| 6 | `STATE_ERROR` | Not allowed now (port not open, tones not loaded, no UTC mapping) |
| 7 | `OVERFLOW` | CAT credit exceeded; bytes were dropped |
| 8 | `VERSION_MISMATCH` | Incompatible protocol version (§4.2) |
| 9 | `OUT_OF_RANGE` | A value outside the limits in `CAPS` (including the BLE TX power cap) |
| 10 | `FRAME_ERROR` | A frame failed COBS, length or CRC checks |
| 11 | `RATE_LIMITED` | Too many requests of this kind (§12.2) |
| 12 | `INTERNAL` | Device fault |
| 13 | `NOT_AUTHORIZED` | The host isn't approved (wired) or bonded (Bluetooth) yet (§15) |

### 12.2 Limits

| Limit | Value |
|---|---|
| `max_payload` | Each side states its own in `HELLO` / `DEVICE_INFO`: 256 to 1024 bytes. A sender must not exceed the peer's value. Every implementation must be able to parse 1024 |
| Frame on the wire | At most `max_payload` + 4 bytes, plus COBS overhead and the delimiter |
| `PING` data | 64 bytes |
| `DEVICE_INFO.build` | 32 bytes |
| Host requests | `TIME_REQ` and `PING`: 10 per second each. Persisted `CONFIG_SET`: 1 per second. `KEEPALIVE`: 10 per second |
| Device notifications | `STATUS`: 2 per second. `AUDIO_STATUS`: about 10 per second while audio runs. `PTT_STATUS`, `CONFIG`, `CAPS`: on change |
| Serial ports | Radio ports 1–4 |
| Keepalive and max TX | From the `PTT` TLV |

## 13. Bluetooth LE transport

### 13.1 GATT service

The device is the GATT server and LE peripheral. These UUIDs belong to this
project. They were generated at random for it (version 4) and must not be
reused for anything else:

| Item | UUID | Properties | Security |
|---|---|---|---|
| **Rig Interface service** | `274b5780-7929-4cf2-8a88-af1180d8fc04` | Primary service | — |
| **RX** (host → device) | `5c287ba9-de76-46de-8ca1-3519ad5fa899` | Write without response, Write | Encrypted, bonded |
| **TX** (device → host) | `3b7d006a-e34e-448c-a911-7f683a261abf` | Notify | Encrypted, bonded (CCCD write) |
| **Info** | `9e6aea0a-b69f-4c23-9d32-ff0aa98bc000` | Read | None: readable before pairing |

- The device advertises the service UUID (in the advertising data or the scan
  response), so hosts can filter scans by it.
- The host writes stream bytes to RX, preferably without response, in chunks
  of at most ATT_MTU − 3 bytes. The device notifies TX in chunks of at most
  ATT_MTU − 3 bytes, and should fill each notification. Chunk boundaries are
  not frame boundaries (§1). Vector `gatt.json` → `chunking` shows a `CAPS`
  reply split at the minimum ATT_MTU of 23.
- Windows reports the negotiated size as `GattSession.MaxPduSize`; iOS paces
  writes with `canSendWriteWithoutResponse`
  ([host compatibility §3](../docs/research/host-compatibility.md#3-bluetooth-le)).

### 13.2 Info characteristic

Readable without pairing, so a host can check compatibility and find the
L2CAP PSM first. The format is frozen across major versions (§4.1).

| Offset | Field | Meaning |
|---|---|---|
| 0 | `proto_major u8` | Protocol version |
| 1 | `proto_minor u8` | |
| 2 | `proto_patch u8` | |
| 3 | `flags u8` | Bit 0: L2CAP CoC available. Bit 1: the pairing window is open (§13.5) |
| 4 | `psm u16` | L2CAP PSM for the CoC channel (0 when not available) |

A host must read the PSM from here, not hard-code it: the device may choose
any dynamic LE PSM (both Apple's and Android's APIs take the PSM from the
peripheral).

### 13.3 L2CAP CoC (optional)

- One LE credit-based connection-oriented channel on the Info PSM, open to
  bonded hosts over an encrypted link.
- It carries the same byte stream as GATT, with the channel's own credit flow
  control. SDU boundaries carry no meaning. The device's SDU MTU should be at
  least 512 bytes **(verify with NimBLE)**.
- A host that can use L2CAP CoC should: it avoids per-write GATT overhead
  ([host compatibility §3.2](../docs/research/host-compatibility.md#32-gatt-vs-l2cap-coc)).
  It sends `HELLO` on the channel, which makes the channel the active
  transport (§4.2). It keeps the GATT subscription as well, but nothing is
  sent there while the channel is active.
- The firmware uses the NimBLE host stack, because Bluedroid's L2CAP is
  Classic-only ([host compatibility §3.4](../docs/research/host-compatibility.md#34-esp32-s3-side)).

### 13.4 Link parameters

The device should request, following Apple's accessory guidelines and #6:

- **Data length extension** to 251 bytes, **before** the ATT MTU exchange;
- an ATT MTU of at least 247, accepting the host's value;
- the **2M PHY** where the host supports it (REQ-HOST-011);
- a connection interval of 15 ms (minimum ≥ 15 ms, in multiples of 15 ms, on
  Apple hosts), and peripheral latency 0 while audio runs or PTT is keyed.

The host may grant other values. `STATUS` reports what was negotiated, so apps
can show link quality.

### 13.5 Security and pairing

- RX, TX and the L2CAP channel need an **encrypted link with a bonded host**
  using **LE Secure Connections**; the device refuses LE legacy pairing, and
  an unbonded central can read only the Info characteristic (§15.1). The device rejects an unauthenticated ATT request
  with *Insufficient Authentication*, which makes Apple and other hosts start
  pairing ([host compatibility §3.1](../docs/research/host-compatibility.md#31-capabilities-per-os)).
  The Info characteristic stays readable without a bond.
- The device has no display or keypad, so pairing is "Just Works", which
  gives no protection against an active attacker during pairing. The
  **pairing window** limits when that can happen (maintainer decision,
  2026-09-24):
  - **New bonds are accepted only while the pairing window is open.** A
    local action on the device opens it: power-on, or a press of a pairing
    button. `PAIRING.triggers` reports which of these the hardware has; the
    exact trigger and whether a button exists belong to the firmware and
    hardware issues (#14, #9) **(verify)**.
  - The window closes after `PAIRING_WINDOW_S` (default 120 s), after the
    first new bond or wired-host approval, or when the host mode changes,
    whichever comes first. The same window approves new **wired** hosts
    (§15.2).
  - **Outside the window, only bonded hosts can use the device.** Any host
    can still connect and read Info, but the device refuses pairing
    requests, so an unbonded host never reaches RX, TX or the L2CAP channel.
  - The window's state is shown by Info `flags` bit 1 (readable before
    bonding, so an app can tell the user to press the button) and by
    `STATUS.flags` bit 6.
  - When all `PAIRING.max_bonds` slots are in use, a new bond replaces the
    oldest **(verify with NimBLE's bond storage, #14)**.
- **Only a local action on the device opens the pairing window.** No
  protocol message opens it, over any transport: neither the wired control
  port nor the USB network can (maintainer decision, 2026-09-24).

## 14. Wired USB-C transports

In wired mode the device is a USB device on the USB-C port
([ADR-0008](../docs/decisions/ADR-0008-host-links-esp32-s3.md)). Besides the
standard audio and serial functions, it carries this protocol over a **USB
network interface** (TCP), and, where the endpoint budget allows, over a
**CDC-ACM control port**
([ADR-0007](../docs/decisions/ADR-0007-protocol.md),
[#44](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/44)).

Rules for both:

- They carry **the same framed stream** as Bluetooth (§3), with the same
  session rules (§4.2). A host must be **approved** before it can use them
  (§15.2). Only one session is active at a time: a `HELLO` on
  another transport takes over, and the device closes the previous one's
  session (PTT off, §8.7).
- Available messages: session, security (§15), `CAPS`, `STATUS`,
  configuration, `PTT_*`, `KEEPALIVE`, clock sync and tone sequences. `CAT_DATA`, `SERIAL_*` and
  `MODEM_LINES` are available only for port 0 in the **network** profile
  (§14.1), where no native serial port exists. Audio messages get
  `UNSUPPORTED`: wired audio uses USB Audio Class.
- `PTT_SET` needs the keepalive, as over Bluetooth.
- A session also ends on USB reset, suspend or unplug.

### 14.1 USB functions and the endpoint budget

The ESP32-S3's USB OTG has endpoint 0 plus **6 endpoints configurable as IN
or OUT, with at most 5 IN endpoints (including endpoint 0) active at once**
([esp-usb device guide](../docs/references/index.md#esp-usb-device-s3)).
So at most **4 IN endpoints** besides endpoint 0. The functions need
(TinyUSB descriptors, [`usbd.h`](../docs/references/index.md#tinyusb-usbd-h)):

| Function | IN | OUT |
|---|---|---|
| CDC-ACM serial port (notification + bulk IN, bulk OUT) | 2 | 1 |
| CDC-NCM network (notification + bulk IN, bulk OUT) | 2 | 1 |
| UAC1 speaker + microphone, adaptive OUT (no feedback endpoint) | 1 | 1 |

ADR-0008's full set (SERIAL-jack bridge port + control port + UAC1) already
needs 5 IN and 3 OUT endpoints (8 of the 6 available), which **doesn't fit**; adding the network
interface makes it 7 IN. The device therefore enumerates one of these
function sets, chosen from what is on the radio port when it enters wired
mode (it probes the radio port as USB host first **(verify, #44)**):

| Radio has | Functions | IN / OUT | The host gets |
|---|---|---|---|
| USB serial + USB sound card | NCM + CDC-ACM control port | 4 / 2 | Radio's own serial and sound card (through the hub); protocol over TCP or the control port; control-port RTS/DTR → AUDIO-jack PTT |
| USB serial + analog audio | NCM + UAC1 | 3 / 2 | Radio's own serial (hub); the device's sound card; protocol over TCP |
| SERIAL-jack serial + analog audio, profile **network** (0) | NCM + UAC1 | 3 / 2 | CAT (`CAT_DATA` port 0), PTT and configuration over TCP; the device's sound card |
| SERIAL-jack serial + analog audio, profile **serial** (1) | CDC-ACM bridge + UAC1 | 3 / 2 | A native serial port bridged to the SERIAL jack, whose RTS/DTR key PTT (`LINE_MAP` selector 0); the device's sound card. No protocol transport in wired mode |

The `WIRED_PROFILE` key picks between the last two rows. The default is
**serial** (maintainer decision, 2026-09-24): a native COM/tty port that any
radio software can use with no protocol support. iPhone and iPad users, who
can't open USB serial ports, set **network**. The function sets and the
missing wired CAT below were accepted by the maintainer on 2026-09-24. That `esp_tinyusb` builds each of these composites is
**(verify, #44)**: its guide documents composite devices, CDC-ACM and an
NCM network driver, and UAC1 comes from TinyUSB's own audio class. An asynchronous UAC1 OUT endpoint with explicit
feedback would add one IN endpoint, which still fits every row above.

Consequences, stated plainly:

- **iPhone and iPad in wired mode** get CAT, PTT and configuration through
  the network interface when the radio's serial is on the SERIAL jack
  (profile network). With a radio whose serial is USB, the radio's own chip
  sits behind the hub, where iOS apps can't reach it, so an iOS host gets
  AUDIO-jack PTT and configuration over TCP but **no CAT** in wired mode;
  Bluetooth mode has full CAT.
- **Windows 10** has no in-box NCM driver (Windows 11 has `UsbNcm.sys`
  [ms-usb-classes](../docs/references/index.md#ms-usb-classes)); there it uses
  the serial profile, the control port or Bluetooth.

### 14.2 USB network transport (CDC-NCM)

- **Class:** USB CDC-NCM (class 02h, subclass 0Dh), the one network class with
  in-box drivers on Windows 11 ([ms-usb-classes](../docs/references/index.md#ms-usb-classes)),
  Linux (`cdc_ncm`, [source](../docs/references/index.md#linux-cdc-ncm)) and
  Android 14 GKI kernels ([kernel config](../docs/references/index.md#android-kconfig-cdc-ncm)).
  Apple confirms a built-in USB Ethernet driver on iOS
  ([apple-forum-802640](../docs/references/index.md#apple-forum-802640)) and
  recommends Ethernet over USB for iPhone accessories
  ([apple-forum-747847](../docs/references/index.md#apple-forum-747847)), but
  doesn't name the classes it supports: NCM on iOS, iPadOS and macOS is
  **(verify, #18)**.
- **MAC address:** the address the host uses (the NCM `iMACAddress` string)
  must be **universally administered**, not locally administered. Linux names
  a point-to-point NCM interface with a locally administered address `usbN`
  and one with a universal address `ethN`
  ([usbnet source](../docs/references/index.md#linux-usbnet), facts only), and
  Android only brings up Ethernet interfaces named `ethN`
  ([Webb 2023](../docs/references/index.md#jordemort-android-cdc)). Use
  addresses from the module's factory MAC allocation **(verify, #44)**.
  Android support as a whole is **(verify, #18)**.
- **IPv4 addressing:** the device runs a DHCP server on the link, on a /30
  subnet set by `USB_NET_SUBNET` (default **10.169.160.0/30**: the device is
  **10.169.160.1**, the host gets 10.169.160.2). The lease has no router and
  no DNS server, so hosts never send internet traffic to the device. A small,
  randomly chosen /30 makes a clash with the host's other networks unlikely;
  a user whose network does clash can move it.
- **IPv6:** the device also answers on its IPv6 link-local address.
- **Discovery:** DNS-SD over mDNS, service type **`_rig-interface._tcp`**,
  instance name = the device name. Hosts may also connect straight to the
  device address (10.169.160.1 by default). Registering the service name with IANA is a follow-up **(verify)**.
- **Port:** TCP **51621** (in the dynamic range, so no registration is
  needed), and the port in the DNS-SD record wins if they differ. One
  connection at a time; a new connection's `HELLO` takes over (§14).
- Hosts should disable Nagle's algorithm (`TCP_NODELAY`) so PTT and CAT
  aren't delayed. The device does the same.
- The device doesn't route or forward packets, and listens on nothing but
  DHCP, mDNS and this port.
- iOS apps need the user's local-network permission to connect **(verify)**.
- The framing is identical to the other transports, so there are no extra
  golden vectors; `status_usb_network` shows `STATUS` with transport 4.

### 14.3 CDC-ACM control port

Present in the first row of the table in §14.1.

- The port's baud rate and line format are ignored.
- Its **native RTS/DTR** are PTT sources through `LINE_MAP` selector 0x0F
  (default RTS → PTT). The RTS/DTR of the SERIAL-jack bridge port (serial
  profile) use selector 0. Native lines follow the arming rules of §8.4 and
  count toward max TX, but need no keepalive: USB unplug, suspend, reset and
  the lines' drop at port close release them. So a radio program that keys
  PTT with RTS or DTR works in wired mode with no protocol support.
- iOS and iPadOS apps can't open USB serial ports
  ([host compatibility §6.3](../docs/research/host-compatibility.md#63-user-facing-limitations-per-os));
  they use the network transport.

## 15. Security

The device keys a transmitter, so every link that can reach it needs
authorization. This project designs to the **Cyber Resilience Act** level
(maintainer decision, 2026-09-25,
[#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64),
[ADR-0007](../docs/decisions/ADR-0007-protocol.md)). There is **no default
password** anywhere.

### 15.1 Bluetooth LE

- **Bonding with LE Secure Connections is required.** LE legacy pairing is
  refused.
- An **unbonded central gets only the Info characteristic** (§13.2). RX, TX
  and the L2CAP channel need an encrypted link with a bonded host.
- New bonds are made only during the pairing window (§13.5).

### 15.2 Wired hosts (USB network and CDC-ACM control port)

A wired link has no pairing of its own, so the device approves **hosts**:

- **Host identity:** each host generates a random 128-bit **host token** once
  (from a cryptographically secure random source) and keeps it. It sends the
  token in `AUTH` after `HELLO`. The token identifies the host; it isn't a
  password the user types, and the device has no default one.
- **Approval:** an unknown token is approved only while the pairing window
  is open, and the window is opened only by the same **local action on the
  device** that allows new Bluetooth bonds (§13.5). The device then stores
  the host (it should store a hash of the token, not the token itself
  **(verify, #64)**) and closes the window. Approved hosts are remembered
  across power cycles, up to `PAIRING.max_wired_hosts`.
- **Until a host is approved** the device answers only `HELLO`, `CAPS_GET`,
  `PING` and `AUTH`, and refuses every other message with `RESULT`
  `NOT_AUTHORIZED`. The flow: the host sends `AUTH`; the device answers
  `AUTH_STATUS` state 1 (not approved); the user presses the button (or power
  cycles the device) to open the window; the host sends `AUTH` again (for
  example every 2 s) and gets state 0.
- Approval lasts for the session. A new `HELLO` needs a new `AUTH`.
- The token travels unencrypted over the USB cable. Encrypting the TCP
  transport (the EN 18031-1 level) is planned and costed in #64, not
  specified here.
- **Native USB serial functions aren't protocol transports.** The RTS/DTR
  line state and data of the device's CDC-ACM ports (§14.3) work like any USB
  serial adapter, with no approval. Whether that is acceptable at the CRA
  level is reviewed in #64.

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x70` | `AUTH` | H→D | `host_token b16` (16 bytes). Reply: `AUTH_STATUS`. Over Bluetooth the bond already authorizes the host; `AUTH` then just returns state 0 |
| `0x71` | `AUTH_STATUS` | D→H | `state u8` (0 approved, 1 not approved: open the pairing window and retry, 2 refused: no free slot), `slot u8` (the host's slot, 0xFF when not approved) |

### 15.3 Managing trusted hosts

An authorized host can list and remove bonds and approved wired hosts:

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x72` | `TRUST_LIST_GET` | H→D | none. Reply: `TRUST_LIST` |
| `0x73` | `TRUST_LIST` | D→H | `records`: 8 bytes each: `kind u8` (1 Bluetooth bond, 2 approved wired host), `slot u8`, `ident b6` (the bond's identity address, or the first 6 bytes of SHA-256 of a wired host's token, so a host can recognize itself) |
| `0x74` | `TRUST_REMOVE` | H→D | `kind u8`, `slot u8` (0xFF = every entry of that kind). Reply: `RESULT`. Removing the host that sent it ends its session |
| `0x75` | `FACTORY_RESET` | H→D | `confirm u32`, which must be 0x54455352 (the bytes `RSET`), otherwise `BAD_VALUE`. Reply: `RESULT`, then the device erases configuration, bonds, approved hosts and the UTC mapping, and restarts with PTT off |

A factory reset should also be possible by a local action on the device, for
a user who has lost every trusted host; the exact action belongs to #14 and
the hardware **(verify)**.

### 15.4 Firmware updates

- The firmware accepts **only signed images** (REQ-FW-006). Secure boot and
  flash encryption on the ESP32-S3 are planned in #64 **(verify)**.
- The OTA transport is **not decided here**. It is deferred to
  [#14](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/14) and #64.
  Message types 0xE0–0xEF are reserved for a firmware-update extension,
  added in a minor version and reported by `FEATURES` bit 15; only an
  authorized host will be able to use it. Wired mode may use USB DFU instead,
  outside this protocol ([ADR-0008](../docs/decisions/ADR-0008-host-links-esp32-s3.md)).

## 16. Message index

| Type | Message | Dir. | Section |
|---|---|---|---|
| `0x00` | reserved | — | never used |
| `0x01` | `HELLO` | H→D | §4.2 |
| `0x02` | `DEVICE_INFO` | D→H | §4.2 |
| `0x03` | `CAPS_GET` | H→D | §5 |
| `0x04` | `CAPS` | D→H | §5 |
| `0x05` | `RESULT` | D→H | §4.3, §12 |
| `0x06` | `PING` | H→D | §4.3 |
| `0x07` | `PONG` | D→H | §4.3 |
| `0x08` | `STATUS_GET` | H→D | §16.1 |
| `0x09` | `STATUS` | D→H | §16.1 |
| `0x10` | `CONFIG_GET` | H→D | §6 |
| `0x11` | `CONFIG_SET` | H→D | §6 |
| `0x12` | `CONFIG` | D→H | §6 |
| `0x13` | `CONFIG_RESET` | H→D | §6 |
| `0x20` | `SERIAL_OPEN` | H→D | §7 |
| `0x21` | `SERIAL_SET` | H→D | §7 |
| `0x22` | `CAT_DATA` | both | §7 |
| `0x23` | `CAT_CREDIT` | D→H | §7.4 |
| `0x24` | `MODEM_LINES` | H→D | §7, §8.3 |
| `0x25` | `MODEM_STATUS` | D→H | §7 |
| `0x30` | `PTT_SET` | H→D | §8 |
| `0x31` | `PTT_STATUS` | D→H | §8 |
| `0x32` | `KEEPALIVE` | H→D | §8.2 |
| `0x40` | `AUDIO_START` | H→D | §9 |
| `0x41` | `AUDIO_STOP` | H→D | §9 |
| `0x42` | `AUDIO_FRAME` | both | §9 |
| `0x43` | `AUDIO_STATUS` | D→H | §9 |
| `0x50` | `TIME_REQ` | H→D | §10 |
| `0x51` | `TIME_RESP` | D→H | §10 |
| `0x52` | `TIME_SET` | H→D | §10 |
| `0x60` | `TONE_SETUP` | H→D | §11 |
| `0x61` | `TONE_DATA` | H→D | §11 |
| `0x62` | `TONE_START` | H→D | §11 |
| `0x63` | `TONE_CANCEL` | H→D | §11 |
| `0x64` | `TONE_STATUS` | D→H | §11 |
| `0x70` | `AUTH` | H→D | §15.2 |
| `0x71` | `AUTH_STATUS` | D→H | §15.2 |
| `0x72` | `TRUST_LIST_GET` | H→D | §15.3 |
| `0x73` | `TRUST_LIST` | D→H | §15.3 |
| `0x74` | `TRUST_REMOVE` | H→D | §15.3 |
| `0x75` | `FACTORY_RESET` | H→D | §15.3 |
| `0xE0`–`0xEF` | reserved | — | firmware update (§15.4) |
| `0xF0`–`0xFE` | reserved | — | experiments; never in a release |
| `0xFF` | reserved | — | never used |

### 16.1 Status

| Type | Message | Dir. | Payload |
|---|---|---|---|
| `0x08` | `STATUS_GET` | H→D | none. Reply: `STATUS` |
| `0x09` | `STATUS` | D→H | `host_link u8` (1 Bluetooth, 2 wired), `transport u8` (1 GATT, 2 L2CAP CoC, 3 CDC-ACM control port, 4 TCP over the USB network), `phy u8` (0 n/a, 1 1M, 2 2M, 3 Coded), `conn_interval_us u32`, `att_mtu u16`, `coc_mtu u16` (0 if none), `flags u16` (below), `audio_path u8` (0 none, 1 analog, 2 radio USB), `supply_mv u16` (0 unknown), `frame_errors u16`, `cat_overflows u16` (both saturate at 65 535). Sent on request and, with token 0, on change |

`STATUS.flags`: bit 0 radio USB-serial chip present; bit 1 radio USB sound
card present; bit 2 a device is attached to the radio USB port but isn't usable (enumeration failed or unsupported class); bit 3
UTC mapping fresh (§10.3); bit 4 USB-C source advertises 1.5 A or more; bit 5
audio running; bit 6 pairing window open (§13.5); bit 7 this session's host
is authorized (§15). Other bits reserved.

## 17. Golden vectors and the reference codec

- [`vectors/messages.json`](vectors/messages.json): at least one vector for
  every message, config key and TLV, each with its fields, payload, frame and
  wire bytes in hex.
- [`vectors/framing.json`](vectors/framing.json): CRC and COBS known answers,
  invalid frames a receiver must reject, and a stream with a resync.
- [`vectors/gatt.json`](vectors/gatt.json): the Info characteristic, and a
  message split into GATT chunks.
- The reference encoder/decoder and its tests are in
  [`tools/protocol/`](../tools/protocol/) (Python standard library, MIT).
  Firmware and host implementations should test against the same JSON files.
  CI runs the tests and checks that the vectors are current.

## Open questions

None open in this version. Settled on 2026-09-24 (maintainer, ADR-0007):
the defaults in §6.1 (all configurable, `WIRED_PROFILE` serial), the wired
function sets and the missing wired CAT for iOS with USB-serial radios
(§14.1), the pairing window, which only a local action opens (§13.5), and the
USB network interface instead of Bluetooth control while wired to iOS.

EU conformity ([#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59))
may add access-control requirements to the wired transports (§14); that
issue will report back.

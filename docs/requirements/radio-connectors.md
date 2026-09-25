<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Radio-side connectors

The radio side of the interface uses **two 3.5 mm TRRS jacks** (AUDIO and
SERIAL) plus a **radio USB port**; the host side has a **USB-C port** (and
Bluetooth LE). The radio side works the same with either host link. The two jacks follow a pinout convention that
is already widely used by 3.5 mm TRRS digital-mode interfaces, so **existing
per-radio cables made for that convention work unchanged**. Keep this pinout
exactly: changing a contact's function breaks cable compatibility.

The pinout is an interface fact. No third-party schematic, layout or text is
copied (see [`THIRD_PARTY.md`](../../THIRD_PARTY.md)).

Directions below are from the **interface's** point of view: "out" means the
interface drives the contact toward the radio.

## AUDIO jack (3.5 mm TRRS)

| Contact | Signal | Direction | Electrical |
|---|---|---|---|
| Tip | RX audio (radio's audio/speaker/data output) | In | Line level, AC-coupled, high impedance. Provide about 19 dB of switchable attenuation, or an equivalent input gain range, for radios with a hot speaker output (the convention uses a 100 kΩ / 12 kΩ divider that can be bypassed). Maximum input level **(verify)** from the radio table (#5). |
| Ring 1 | TX audio (radio's mic/data input) | Out | Line level, AC-coupled, up to about 2.5 V peak-to-peak, adjustable down to microphone level. |
| Ring 2 | PTT | Out | **Closure to sleeve (ground), active = keyed.** The radio pulls the line up. The convention uses an open-collector NPN rated for high voltage; this design uses an isolated photo-MOSFET closure, which is electrically compatible. Hardware default off ([constraints §6](constraints.md#6-safety-and-fail-safe)). |
| Sleeve | Ground | — | Radio signal ground; reference for tip, ring 1 and ring 2. |

## SERIAL jack (3.5 mm TRRS)

Plain TRS (3-contact) plugs from older cables also work: their sleeve spans
ring 2 and the sleeve, which is harmless because ring 2 is off by default.

| Contact | 3.3 V logic (default) | RS-232 | Icom CI-V | 3.3 V logic + power |
|---|---|---|---|---|
| Tip | Data out (radio's RxD), 3.3 V CMOS | Data out (radio's RxD), RS-232 levels | **CI-V bus** (single wire, open drain; the radio pulls it up) | Data out (radio's RxD), 3.3 V CMOS |
| Ring 1 | Data in (radio's TxD), 3.3 V CMOS, 5 V tolerant | Data in (radio's TxD), RS-232 levels | Not used | Data in (radio's TxD), 3.3 V CMOS |
| Ring 2 | Off | Off | Off | **3.3 V out**, current-limited to about 20 mA (for cables with their own isolation circuit) |
| Sleeve | Ground | Ground | Ground | Ground |

Mode behavior:

- **3.3 V logic** is the default at power-on. It is also the mode that is safe
  with every cable.
- **CI-V:** the interface's transmit path pulls the tip low through an
  open-drain driver (or diode), and its receive path reads the same line, so
  the interface hears its own echo. Firmware handles the echo and collisions.
- **RS-232** swings about ±5 V or more, only on tip. Ring 1 accepts RS-232
  receive levels.
- **3.3 V out** on ring 2 is off unless selected, and short-circuit protected.
- The convention selects modes with solder jumpers. **This design selects them
  in firmware**, stored in the device configuration and set over the protocol
  or the wired control port. The contact functions stay as in the table. A mode
  change never asserts PTT.
- **Every mode must survive any cable.** RS-232 levels (up to ±15 V) can appear
  on tip or ring 1 with the wrong mode selected. The tip and ring 1 paths are
  therefore routed through high-voltage analog switches (TMUX6219-class, 36 V)
  or signal relays, and the RS-232 transceiver's drivers go high-impedance when
  disabled (MAX3243E-class). The logic and CI-V paths add series resistance and
  clamps as a second line of defense.
- Baud rates: 4800–115200 ([constraints §7](constraints.md#7-radio-interfaces)).

## Radio USB port

A **USB-A receptacle** (the radio end of the cable is usually USB-B or
micro/mini-B). It connects to the radio's own USB port, for radios with a
built-in USB-serial chip, with or without a USB sound card
([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)). The interface
supplies **no VBUS**: the port's VBUS pin connects to no device rail, in either
mode ([ADR-0003](../decisions/ADR-0003-radio-interface-circuits.md)).

- **Bluetooth mode:** the ESP32-S3 is USB host to the radio's chips.
- **Wired mode:** the port is switched to the on-board USB hub, so the computer
  sees the radio's USB-serial chip (and sound card) directly, as if connected
  by a cable.

When a radio USB sound card is present, audio uses the USB path and the AUDIO
jack's audio is idle; PTT can still use ring 2 of the AUDIO jack, the radio's
CAT command, or RTS/DTR on the radio's USB-serial chip.

## USB-C port (host link and power)

A **USB-C receptacle** that is both the wired host link and a 5 V power input
(5.1 kΩ Rd on CC1 and CC2; no USB PD). It connects to the upstream port of the
on-board 2-port USB hub. What the computer sees in wired mode:

| Radio | Computer sees |
|---|---|
| USB-serial + USB sound card | The radio's USB-serial and sound card, plus the device's control/PTT serial port |
| USB-serial + analog audio | The radio's USB-serial, the device's USB sound card, and the device's control/PTT serial port |
| RS-232 / 3.3 V logic / CI-V + analog audio | The device's USB serial port (bridged to the SERIAL jack, RTS/DTR native), USB sound card, and control/PTT serial port |

A USB-C charger with no data connection powers the device and leaves it in
Bluetooth mode.

## Isolation and protection

- The AUDIO and SERIAL jacks are isolated on variant M and **whenever the USB-C
  data link is used** ([constraints §6](constraints.md#6-safety-and-fail-safe)):
  audio transformers, isolated PTT closure, digital isolators on the SERIAL jack.
  The jacks' sleeves are then the isolated radio-side ground.
- The radio USB port isn't isolated by default; a full-speed USB isolator
  (ADuM4160-class) is a fitting option, recommended for variant M.
- Every contact gets ESD protection and RF filtering (ferrite plus a small
  capacitor to its sleeve) at the jack
  ([constraints §5](constraints.md#5-rf-environment)).

## Open points

- Measure the actual RX audio level range across the radios in #5 and confirm
  the attenuation step.
- Confirm that the photo-MOSFET's on-resistance and voltage rating suit every
  radio's PTT pull-up (#5, #9).
- Per-radio cable pinouts (radio side) belong in #5 and #9.

<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# Development board: ESP32-S3-DevKitM-1

Until the custom board exists (#9, #21), the firmware runs on Espressif's
**ESP32-S3-DevKitM-1**. It carries the **ESP32-S3-MINI-1** (or the -1U, with
an antenna connector), the module chosen in
[ADR-0008](../../../docs/decisions/ADR-0008-host-links-esp32-s3.md)
([user guide](../../../docs/references/index.md#esp32s3-devkitm1-guide)).
Use the MINI-1 version with the PCB antenna.

> **Lifecycle:** Espressif's user guide says the DevKitM-1 is discontinued
> and suggests the ESP32-S3-DevKitC-1-N8R8 for software testing. The MINI-1
> modules themselves are still supplied. The DevKitC-1 carries a different
> module (ESP32-S3-WROOM-1), and its RGB LED pin differs by board revision
> **(verify before using it)**, so this pin map is for the DevKitM-1 only.

The pin map is in [`board/board_pins.h`](board/board_pins.h), which both the
app and the bootloader hook include.

## Pin map

| Function | GPIO | DevKitM-1 header | Notes |
|---|---|---|---|
| **PTT output** (active high) | **4** | J1 pin 6 | See "PTT pin" below. **Fit an external 10 kΩ pull-down to GND.** Drive an LED or an optocoupler input through a resistor; never a radio's PTT line directly from the dev kit |
| Status LED | 48 | on board | The board's addressable RGB LED (WS2812-type), driven by the RMT peripheral. Blinks 1 Hz when idle, 4 Hz while the pairing window is open, and stays on during a session |
| Pairing button | 0 | on board (Boot) | Press after boot to open the pairing window. Holding it while resetting enters the ROM download mode (GPIO0 is the boot-mode strapping pin, datasheet §3; that Boot connects to GPIO0 is from the guide's description of the button **(verify on the board schematic)**) |
| SERIAL-jack UART TX | 17 | J1 pin 19 | UART1, 3.3 V logic only on the dev kit (no RS-232 or CI-V circuits) |
| SERIAL-jack UART RX | 18 | J1 pin 20 | UART1 |
| I2S BCLK | 5 | J1 pin 7 | For an audio codec (#16); nothing is fitted |
| I2S WS | 6 | J1 pin 8 | |
| I2S DOUT | 7 | J1 pin 9 | |
| I2S DIN | 8 | J1 pin 10 | |
| Console and flashing | 43 (TX), 44 (RX) | USB-to-UART port | UART0, through the board's USB-to-UART bridge (REQ-FW-008) |
| USB OTG D−/D+ | 19/20 | ESP32-S3 USB port | Reserved for the host links (#43, #44); unused by this firmware |

Header positions are from the user guide's J1 table (pin 1 is 3V3).

## PTT pin

The PTT output must stay off whenever the firmware isn't driving it
(REQ-PTT-008, SPEC §8.5). GPIO4 was chosen because, in the
[ESP32-S3 datasheet](../../../docs/references/index.md#esp32s3-ds) v2.2:

- Table 2-1 lists it with **no pull-up and input disabled at reset** (some
  pins, such as GPIO0, GPIO45 and GPIO46, have pulls at reset);
- it isn't a strapping pin (§3);
- Table 2-2 lists only a **low-level** power-up glitch (about 60 µs) for
  GPIO1–GPIO14, which can't key an active-high output.

The layers that keep PTT off, from power-on:

1. **Hardware:** the external pull-down holds the pin low while the chip is
   in reset, in the ROM bootloader, or hung. On the custom board the PTT
   closure is off unless driven (REQ-PTT-008).
2. **Second-stage bootloader:** the hook in
   [`bootloader_components/ptt_boot_hook`](bootloader_components/ptt_boot_hook/hooks.c)
   drives the pin low, enables the internal pull-down and makes it an output,
   before the bootloader initializes anything else
   ([bootloader guide](../../../docs/references/index.md#esp-idf-bootloader-guide),
   [hooks example](../../../docs/references/index.md#esp-idf-bootloader-hooks-example)).
3. **App:** the first call in `app_main()` drives it low again, and the PTT
   controller initializes with every output off.
4. **Watchdogs:** a lock-up resets the chip, which goes back to step 1
   ([`README.md`](README.md#watchdogs)).

## Not on the dev kit

- The AUDIO-jack isolation, the RS-232 transceiver and CI-V buffer, the
  SERIAL-jack mode switches, the USB hub and switches, and the radio USB port.
  The firmware reports only what the dev kit has (`CAPS`: CAT on port 0 at
  3.3 V logic, the PTT closure, clock sync, BLE TX power, the pairing window).
- A supply-voltage monitor (`STATUS.supply_mv` is 0).

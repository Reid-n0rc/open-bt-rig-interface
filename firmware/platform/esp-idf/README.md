<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# firmware/platform/esp-idf (ESP32-S3)

The ESP-IDF project for the ESP32-S3-MINI-1
([ADR-0008](../../../docs/decisions/ADR-0008-host-links-esp32-s3.md)). It
implements [`hal.h`](../../app/include/hal.h) and runs the portable core in
[`firmware/app`](../../app/) (as the `rig_app` component). The board is the
ESP32-S3-DevKitM-1 for now: pin map in [`BOARD.md`](BOARD.md).

## ESP-IDF version (pinned)

**ESP-IDF v6.0.3** (tag `v6.0.3`, commit `76f5dedd9950a3012fee8fb7d5586df21fc67802`,
[release notes](../../../docs/references/index.md#esp-idf-v6-0-3)), Apache-2.0
([LICENSE](../../../docs/references/index.md#esp-idf-license)).

- v6.0 is the newest release series with patch releases (v6.0 was released
  on 2026-03-20, [release notes](../../../docs/references/index.md#esp-idf-v6-0)).
  Each ESP-IDF minor release is supported for 30 months from its first
  release: 12 months of service, then 18 of maintenance
  ([versions](../../../docs/references/index.md#esp-idf-versions)).
- CI builds in Espressif's official image `espressif/idf:v6.0.3`, pinned by
  digest in [`checks.yml`](../../../.github/workflows/checks.yml)
  ([Docker image guide](../../../docs/references/index.md#esp-idf-docker-image)).
- To move to a newer ESP-IDF: change the image tag and digest in CI, this
  section and [`THIRD_PARTY.md`](../../../THIRD_PARTY.md) in one PR, and
  re-check the references cited here.

## Build and flash

With ESP-IDF v6.0.3 installed and its environment exported
(`. $IDF_PATH/export.sh`):

```sh
cd firmware/platform/esp-idf
idf.py set-target esp32s3     # once
idf.py build
idf.py -p <UART port> flash monitor
```

Or with Docker, as CI does:

```sh
docker run --rm -v "$PWD":/project -w /project/firmware/platform/esp-idf \
  espressif/idf:v6.0.3 idf.py build
```

Flash through the DevKitM-1's **USB-to-UART** port (UART0). The ESP32-S3's
own USB port is reserved for the host links (REQ-FW-008). CI uploads the
images (`rig_interface.bin`, the bootloader, the partition table and
`flash_args`) as the `firmware-esp32s3-devkitm1` artifact; flash them with
`python -m esptool --chip esp32s3 write-flash @flash_args` from that folder.

On boot the log (UART0, 115200 baud) shows the firmware version, the
protocol version, the ESP-IDF version and the reset cause, and the device
advertises as "Rig Interface" with the Rig Interface service UUID
(SPEC §13.1).

## What runs

| Part | File | Notes |
|---|---|---|
| Boot | [`bootloader_components/ptt_boot_hook`](bootloader_components/ptt_boot_hook/hooks.c) | Drives the PTT pin low in the second-stage bootloader's first hook ([`BOARD.md`](BOARD.md#ptt-pin)) |
| App task | [`main/main.c`](main/main.c) | PTT pin low first, reset cause, NVS, the core, drivers, NimBLE. Then one loop: events from a queue, `app_tick()` at least every 10 ms, the status LED, the task watchdog |
| HAL | [`main/hal_esp.c`](main/hal_esp.c) | GPIO (PTT, button), UART1 (SERIAL jack), I2S (codec hooks for #16), `esp_timer` (device time), NVS (configuration) |
| Bluetooth | [`main/ble.c`](main/ble.c) | NimBLE host; the GATT service with the SPEC §13.1 UUIDs; LE Secure Connections, "Just Works", bonds only in the pairing window; the TX power cap |
| Status LED | [`main/status_led.c`](main/status_led.c) | The DevKitM-1's RGB LED via the RMT bytes encoder |

**NimBLE, not Bluedroid:** the L2CAP CoC transport (SPEC §13.3) needs it.
The flow follows ESP-IDF's
[`bleprph`](../../../docs/references/index.md#esp-idf-nimble-bleprph) example.

**Pairing (SPEC §13.5):** power-on and the Boot button open the pairing
window (`PAIRING.triggers` = both). New bonds are stored only while it is
open; outside it an encrypted but unbonded link is dropped and RX refuses
unbonded writers. Up to 8 bonds (`CONFIG_BT_NIMBLE_MAX_BONDS`, kept in NVS);
when the store is full, NimBLE's `ble_store_util_status_rr` removes the
**oldest** bond for the new one
([source](../../../docs/references/index.md#esp-idf-nimble-store-util)).

Not yet here: L2CAP CoC, the radio USB host (#43), wired mode (#44), audio
(#16), tone sequences (#17), and the security stores and secure storage
(#64; the hooks in [`security.h`](../../app/include/security.h) and
`hal_secure_*` fail closed).

## BLE TX power cap

One cap for all markets, in [`ble_power.h`](../../app/include/ble_power.h):
**+6 dBm** (`ESP_PWR_LVL_P6`). It is the highest controller step at or below
both limits:

- **FCC:** the module's grant covers BLE at 10.3 dBm conducted
  ([`fcc.md` §1.3](../../../docs/compliance/fcc.md#13-rf-configuration-firmware));
- **EU:** Espressif's EU-type examination certificate
  ([0370-RED-4972](../../../docs/references/index.md#espressif-s3-mini1-ce-cert))
  lists BLE at 9.96 dBm e.i.r.p., and staying at or below it keeps the device
  under EN 300 328's 10 dBm e.i.r.p. threshold
  ([`eu.md` §3.1](../../../docs/compliance/eu.md#31-spectrum-art-32-en-300-328),
  ADR-0009, REQ-REG-008). The conducted limit is 9.96 dBm minus the
  antenna gain. The PCB antenna's gain isn't published; the working figure is
  2.33 dBi, the gain of the antenna Espressif used to certify the -1U
  variant ([module datasheet](../../../docs/references/index.md#esp32s3-mini1-ds)
  v1.7 §10). **(verify, #59: the exact level against Espressif's EN 300 328
  test report.)** With it, +9 dBm would be 11.3 dBm e.i.r.p.; +6 dBm is
  8.3 dBm.

The ESP32-S3 controller sets power in 3 dB steps from −24 dBm, and the
applied power may be up to 2 dB below the request
([`esp_bt.h`](../../../docs/references/index.md#esp-idf-esp-bt-h-s3)). How
it is enforced:

- `_Static_assert`s in `ble_power.h` check the cap against both limits, and
  in `ble.c` that it equals `ESP_PWR_LVL_P6` and that the controller's
  default level (`CONFIG_BT_CTRL_DFT_TX_POWER_LEVEL_EFF`, set to P6 in
  [`sdkconfig.defaults`](sdkconfig.defaults),
  [Kconfig](../../../docs/references/index.md#esp-idf-bt-ctrl-kconfig-s3)) is
  not above it;
- the protocol can only lower the power (`CONFIG_SET BLE_TX_POWER` above the
  cap is `OUT_OF_RANGE`, host-tested in `firmware/test`);
- after each setting the firmware reads the level back and restarts if it
  is above the cap.

## Watchdogs

There is no separate hardware PTT timer (maintainer decision, 2026-09-25;
SPEC §8.5). A firmware lock-up is caught by the ESP32-S3's own watchdogs
([watchdogs](../../../docs/references/index.md#esp-idf-wdts)), set in
[`sdkconfig.defaults`](sdkconfig.defaults):

| Watchdog | Setting | Source |
|---|---|---|
| RTC watchdog, from the second-stage bootloader until `app_main()` | on, 9 s | `CONFIG_BOOTLOADER_WDT_ENABLE`, `CONFIG_BOOTLOADER_WDT_TIME_MS` ([Kconfig](../../../docs/references/index.md#esp-idf-bootloader-kconfig)) |
| Interrupt watchdog | on, 300 ms | `CONFIG_ESP_INT_WDT`, `CONFIG_ESP_INT_WDT_TIMEOUT_MS` ([Kconfig](../../../docs/references/index.md#esp-idf-system-kconfig)) |
| Task watchdog: the app task and both idle tasks | on, 3 s, **panics (resets)** on timeout | `CONFIG_ESP_TASK_WDT_EN`, `_INIT`, `_PANIC`, `_TIMEOUT_S`, `_CHECK_IDLE_TASK_CPU0/1` |

- `main.c` refuses to build (`#error`) if the task watchdog isn't enabled at
  startup with a panic on timeout, if the interrupt or boot watchdog is off,
  or if the task watchdog timeout is above 5 s. No code path deinitializes
  them.
- A reset leaves PTT off (the pin's pull-down, then the bootloader hook).
- After a watchdog reset, `esp_reset_reason()`
  ([`esp_system.h`](../../../docs/references/index.md#esp-idf-esp-system-h))
  returns `ESP_RST_TASK_WDT`, `ESP_RST_INT_WDT` or `ESP_RST_WDT`; the core maps
  that to `PTT_STATUS` reason `WATCHDOG` and reports it to the next session
  (`app_boot_reason()`, host-tested). Brownout and panic resets report
  `FAULT`.

## Firmware version

`CMakeLists.txt` sets `PROJECT_VER` from the `fw-v<semver>` tags
([`fw_version.cmake`](../../cmake/fw_version.cmake);
[build system guide](../../../docs/references/index.md#esp-idf-build-system)),
so the version is in the image's app description and in `DEVICE_INFO`.

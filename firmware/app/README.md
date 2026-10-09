<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# firmware/app

Portable firmware logic in C11 with no SDK includes. It reaches the hardware
only through [`include/hal.h`](include/hal.h). Every code path is covered by
the host tests in [`../test/`](../test/).

| Module | Header | What |
|---|---|---|
| Protocol codec | [`proto.h`](include/proto.h) | COBS framing, CRC-16/IBM-3740, the stream receiver, and every message, config key, CAPS TLV and trust record of SPEC 0.1.0, table-driven like the reference codec in `tools/protocol/`. Checked against every golden vector |
| PTT controller | [`ptt.h`](include/ptt.h) | SPEC §8: sources, keepalive, RTS/DTR arming, max TX with lockout, every PTT-off event and reason. Pure logic; outputs through callbacks |
| Configuration | [`cfg.h`](include/cfg.h) | Every SPEC §6.1 key with its default and range, and the storage format (every stored value is checked again on load) |
| BLE TX power | [`ble_power.h`](include/ble_power.h) | The cap for all markets (+6 dBm) with static assertions against the FCC and EU limits |
| Core | [`app.h`](include/app.h) | Sessions (SPEC §4), dispatch, CAPS, STATUS, the watchdog/boot reason, the status LED pattern |
| CAT bridge | [`cat.h`](include/cat.h) | SPEC §7: serial ports, a per-port queue toward the radio with credit returned as bytes leave, radio-side batching (full frame or 2 ms), RTS/DTR. Radio USB-serial ports arrive with #43 |
| Clock sync | [`clock_sync.h`](include/clock_sync.h) | `TIME_REQ`/`TIME_SET` and the UTC mapping (SPEC §10) |
| Pairing window | [`pairing.h`](include/pairing.h) | SPEC §13.5: opened only by a local action |
| Audio | [`audio.h`](include/audio.h) | Hooks; `UNSUPPORTED` until #16 |
| Security | [`security.h`](include/security.h) | Hooks for #64 (wired-host approval, bond store, factory reset); they fail closed |
| Board | [`board.h`](include/board.h) | What the hardware can do, filled in by the platform |
| Version | [`fw_version.h`](include/fw_version.h) | Set at build time from the `fw-v*` tags |

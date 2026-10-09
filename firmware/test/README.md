<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
-->

# firmware/test

Host build of [`../app/`](../app/) against a fake HAL, with
[Unity](https://github.com/ThrowTheSwitch/Unity) tests. CI runs it as
`Firmware host tests` on every PR and push.

```sh
cmake -S firmware/test -B build/fw-test
cmake --build build/fw-test
ctest --test-dir build/fw-test --output-on-failure
```

Needs CMake 3.16 or newer, a C11 compiler (GCC or Clang), Python 3 and git.
The build fetches Unity v2.7.0 (MIT,
[LICENSE](../../docs/references/index.md#unity-license)) at a pinned commit;
it is used only here, never in a firmware image. The tests build with
AddressSanitizer and UndefinedBehaviorSanitizer; pass
`-DFW_TEST_SANITIZE=OFF` where they aren't available.

| Test | Covers |
|---|---|
| `test_ptt` | Every PTT path of SPEC §8: boot and watchdog reset, `PTT_SET`, keepalive (and what counts), arming (BLOCKED/ARMED/KEYING, both lines rising), native wired lines, pass-through, targets, max TX with lockout, max TX disabled, session end, mode change, faults, configuration changes that never assert PTT, tone source |
| `test_proto` | The codec against **every** vector in `protocol/vectors/` (messages both ways, CRC, COBS, invalid frames, the resync stream, GATT Info and chunking), plus limits and error paths |
| `test_cfg` | Every config key's default, range and errors; the storage format and its checks |
| `test_app` | Sessions, replies and tokens, version mismatch, frame errors, CAPS, PTT over the protocol, link loss, serial ports and CAT (credit returned as bytes leave, overflow, batching, session end), configuration with persistence, wired-mode rules, security (authorization, AUTH), the pairing window (no message opens it), the LED pattern |
| `test_modules` | BLE TX power cap, pairing window, clock sync, CAT bridge (queues, credit, batching by size and time, overflow, deadlines), audio and security stubs, the reset-reason mapping |
| `test_fw_version` | `../cmake/fw_version.cmake` (a CMake script) |

`gen_vectors.py` turns the golden vectors into C at build time, using the
field kinds of the reference codec (`tools/protocol/proto_codec.py`), so the
tests always use the current JSON. `fake_hal.c` records every output;
`fake_security.c` stands in for the #64 hooks in `test_app` only.

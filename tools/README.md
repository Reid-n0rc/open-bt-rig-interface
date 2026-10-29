<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# tools

Repository scripts and checks (KiCad version check, silkscreen revision check, release helpers). License: MIT.

## kicad_ci

KiCad CI checks (see `AGENTS.md`, Build and test). The ERC/DRC merge gate used by `.github/workflows/kicad-checks.yml`:

- `select_projects.py`: works out which projects need ERC and/or DRC from the changed files, and reads the pinned image tag from `KICAD_VERSION`.
- `run_checks.py`: runs `kicad-cli sch erc` / `pcb drc` for one project, writes JSON reports and logs, and summarises the results.

Used by `.github/workflows/checks.yml`:

- `check_kicad_version.py`: every `*.kicad_sch/pcb/sym/mod/pro` and every doc quoting the minimum KiCad version must match `KICAD_VERSION`.
- `check_silkscreen.py`: title-block revision against the `rev<X>` folder, `${REVISION}`/`${ISSUE_DATE}` on the front silkscreen, no hard-coded revision, the required markings, and the `hw-*` release tag.
- `kicad_files.py`: shared helpers (`KICAD_VERSION` parsing, file discovery, a small s-expression parser).

Tests:

- `test_*.py`: unit tests (`python3 -m unittest discover -s tools/kicad_ci -p 'test_*.py'`). KiCad fixtures are strings inside the tests, written to temporary directories; don't commit `.kicad_*` fixtures.

## protocol

Reference encoder/decoder for [`protocol/SPEC.md`](../protocol/SPEC.md), Python standard library only. CI runs it in `.github/workflows/checks.yml` (`Protocol vectors`).

- `proto_codec.py`: framing (COBS + CRC-16), every message, the capability TLVs and the config keys. `python3 tools/protocol/proto_codec.py decode <hex>` decodes a byte stream.
- `make_vectors.py`: writes `protocol/vectors/*.json` from its examples; `--check` fails if they're out of date.
- `test_proto_codec.py`: round-trips every vector (`python3 -m unittest discover -s tools/protocol -p 'test_*.py'`).

## cat_loopback

Dev-kit loopback test for the CAT bridge (#15, [`protocol/SPEC.md`](../protocol/SPEC.md) §7). With the SERIAL-jack UART's TX wired to its RX, `cat_loopback.py` connects over Bluetooth LE, opens port 0, sends random bytes within the device's credit and checks that the same bytes come back. Needs `bleak` (MIT) on the host: `pip install bleak==0.22.3`; it is never part of a firmware image.

- `cat_loopback.py`: `python3 tools/cat_loopback/cat_loopback.py --bytes 4096 --baud 115200` (`--help` for options). Exit code 0 means a byte-exact loopback.
- `test_cat_loopback.py`: the protocol logic against a simulated device, no hardware (`python3 -m unittest discover -s tools/cat_loopback -p 'test_*.py'`). CI runs it in `Protocol vectors`.

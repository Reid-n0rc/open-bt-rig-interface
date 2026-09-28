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

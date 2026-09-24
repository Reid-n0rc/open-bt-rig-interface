<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# tools

Repository scripts and checks (KiCad version check, silkscreen revision check, release helpers). License: MIT.

## kicad_ci

The ERC/DRC merge gate used by `.github/workflows/kicad-checks.yml` (see `AGENTS.md`):

- `select_projects.py`: works out which projects need ERC and/or DRC from the changed files, and reads the pinned image tag from `KICAD_VERSION`.
- `run_checks.py`: runs `kicad-cli sch erc` / `pcb drc` for one project, writes JSON reports and logs, and summarises the results.
- `test_*.py`: unit tests (`python3 -m unittest discover -s tools/kicad_ci -p 'test_*.py'`).

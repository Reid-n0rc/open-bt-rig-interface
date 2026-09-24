<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Contributing

The complete rules are in [`AGENTS.md`](AGENTS.md). They apply to humans and AI agents
alike. For environment setup, local checks and recipes, see the
[developer guide](docs/developer-guide.md). In short:

1. **Start from an issue.** No change without one. Propose one first if it doesn't exist.
2. **Plan** in the issue before working.
3. **Branch** from `dev` as `<type>/<issue#>-<slug>`.
4. **Signed commits only.**
5. **Open a PR to `dev`** with `Closes #N` and fill in the PR template. `main` is
   updated only by release PRs.
6. KiCad work uses **KiCad ≥ 10.0.6** through the Konnect tools. Keep
   [`KICAD_VERSION`](KICAD_VERSION) in step with the files.
7. Add SPDX/REUSE licensing for every new file (`uvx reuse lint`).

By contributing you agree your contribution is licensed under the license of the
path it touches (see [`LICENSE`](LICENSE)).

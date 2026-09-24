<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Contributing

The complete rules are in [`AGENTS.md`](AGENTS.md). They apply to humans and AI agents
alike. In short:

1. **Start from an issue.** No change without one. Propose one first if it doesn't exist.
2. **Plan** in the issue before working.
3. **Branch** from `dev` as `<type>/<issue#>-<slug>`.
4. **Signed commits (SSH or GPG) are preferred but not required.** The branch
   ruleset doesn't enforce signatures; PRs are still required for `main` and `dev`.
5. **Open a PR to `dev`** with `Closes #N` and fill in the PR template. `main` is
   updated only by release PRs.
6. KiCad work uses **KiCad ≥ 10.0.6** through the Konnect tools. Keep
   [`KICAD_VERSION`](KICAD_VERSION) in step with the files.
7. Add SPDX/REUSE licensing for every new file (`uvx reuse lint`).
8. Record any material brought in from another project (code, KiCad library
   items, copied text or figures, a firmware dependency) in
   [`THIRD_PARTY.md`](THIRD_PARTY.md) in the same PR.

## Contributor terms

The hardware and firmware are source-available under non-commercial licenses,
with commercial licenses available from the copyright holder (see
[`COMMERCIAL.md`](COMMERCIAL.md)). Offering those commercial licenses requires
rights to every contribution, so by submitting a contribution (a pull request,
patch, design file or other material) you agree that:

1. **You have the right to contribute it.** It is your own original work, or
   you have permission to submit it under these terms, and it contains no
   third-party material except with its license and notices recorded in
   [`THIRD_PARTY.md`](THIRD_PARTY.md).
2. **Your contribution is licensed under the license of the path it touches**
   (see [`LICENSE`](LICENSE)).
3. **You also grant Reid Crowe, N0RC** (and any successor maintainer of this
   project) a perpetual, worldwide, non-exclusive, royalty-free, irrevocable
   license to use, copy, modify, distribute, sublicense and relicense your
   contribution, **including under commercial license terms**.
4. You keep the copyright in your contribution. This grant does not transfer
   ownership.

If you can't agree to these terms, open an issue describing the change instead
of submitting it.

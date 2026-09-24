# Developer guide

This is the practical *how-to* for working on open-bt-rig-interface, for people and
AI agents alike. The **rules** live in [`AGENTS.md`](../AGENTS.md) and the short
version in [`CONTRIBUTING.md`](../CONTRIBUTING.md). This guide links to both
rather than repeating them. If this guide and `AGENTS.md` disagree, `AGENTS.md`
wins; please open an issue so the guide gets fixed.

Before any design work, read
[`docs/requirements/constraints.md`](requirements/constraints.md).

## Contents

1. [Prerequisites and setup](#1-prerequisites-and-setup)
2. [Repo tour](#2-repo-tour)
3. [From issue to merged PR](#3-from-issue-to-merged-pr)
4. [Local checks](#4-local-checks)
5. [Recipes by type of work](#5-recipes-by-type-of-work)
6. [Releases and tags](#6-releases-and-tags)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Prerequisites and setup

| Tool | Why | Notes |
|---|---|---|
| git ≥ 2.34 | Version control, SSH commit signing | Signed commits are required (see below) |
| [GitHub CLI `gh`](https://cli.github.com/) | Issues, PRs, checks | `gh auth login` |
| [KiCad](https://www.kicad.org/download/) **≥ 10.0.6** | Schematic/PCB | Must match [`KICAD_VERSION`](../KICAD_VERSION) |
| `kicad-cli` | ERC/DRC and exports from the shell | Ships with KiCad (paths below) |
| [`uv`](https://docs.astral.sh/uv/) | Runs `reuse` without a global install | `uvx --from 'reuse[charset-normalizer]' reuse lint` |
| [Konnect](https://github.com/Reid-n0rc/Konnect) | KiCad MCP server that agents **must** use for KiCad changes | Load the `konnect` skill first (see [`AGENTS.md`](../AGENTS.md#hardware-kicad)) |
| Firmware SDK | Device firmware | **TBD**: chosen by the module decision (#7) and set up in #14 |

### `kicad-cli` location

| OS | Path |
|---|---|
| macOS | `/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli` |
| Windows | `C:\Program Files\KiCad\10.0\bin\kicad-cli.exe` |
| Linux | `kicad-cli` on `PATH` (distribution or official PPA/Flatpak package) |

Check the version with `kicad-cli version`. It must be at least the
`KICAD_MIN_VERSION` in [`KICAD_VERSION`](../KICAD_VERSION). CI runs the pinned
`kicad/kicad:<KICAD_MIN_VERSION>` Docker image.

### Signed commits (required)

The `main` and `dev` branches only accept signed commits, and we sign every
commit on feature branches too. SSH signing is the simplest option:

```sh
git config --global gpg.format ssh
git config --global user.signingkey ~/.ssh/<your-key>.pub
git config --global commit.gpgsign true
# Let git verify your own signatures locally:
echo "$(git config user.email) $(cat ~/.ssh/<your-key>.pub)" >> ~/.ssh/allowed_signers
git config --global gpg.ssh.allowedSignersFile ~/.ssh/allowed_signers
```

Then add the same public key to GitHub as a **Signing key** (Settings → SSH and
GPG keys → New SSH key → Key type: *Signing Key*). GPG signing works too.

Never bypass signing (`--no-gpg-sign`, `-c commit.gpgsign=false`). If signing
fails, fix the setup or stop and ask.

### Private or local tooling

Anything private, credentialed or specific to your machine goes in the untracked
`AGENTS.local.md` or `CLAUDE.local.md` at the repo root (both are gitignored).
Never commit credentials, and never name private tools in tracked files.

---

## 2. Repo tour

| Path | What's there |
|---|---|
| [`hardware/boards/`](../hardware/boards/) | KiCad projects, `hardware/boards/<board>/rev<X>/` |
| [`hardware/lib/`](../hardware/lib/) | Project-local symbols, footprints, 3D models |
| [`hardware/enclosure/`](../hardware/enclosure/) | 3D-printed enclosure CAD source |
| [`firmware/`](../firmware/) | `app/` portable core, `platform/<sdk>/` SDK glue, `test/` host tests |
| [`protocol/`](../protocol/) | Versioned host-device Bluetooth protocol spec and golden vectors |
| [`tools/`](../tools/) | Scripts and checks (KiCad version check, etc.) |
| [`docs/requirements/`](requirements/) | Constraints and requirements |
| [`docs/decisions/`](decisions/) | ADRs, one decision per file |
| [`docs/bringup/`](bringup/), [`docs/compliance/`](compliance/) | Bench bring-up notes, FCC/regulatory material |
| [`LICENSES/`](../LICENSES/), [`REUSE.toml`](../REUSE.toml) | License texts and the path-to-license mapping |

Every directory has a `README.md` explaining its purpose. Read it before adding files there.

**Licensing and governance:** this guide deliberately doesn't restate license
terms. Which license applies to which path, and any commercial-use or
governance terms, are defined in [`LICENSE`](../LICENSE),
[`REUSE.toml`](../REUSE.toml) and [`AGENTS.md`](../AGENTS.md#licensing-reuse),
plus `COMMERCIAL.md` and `GOVERNANCE.md` at the repo root once they are added.

---

## 3. From issue to merged PR

The full rules are in [AGENTS.md → Workflow](../AGENTS.md#workflow). Step by step:

1. **Pick or open an issue.** No change without one. Open issues use the
   templates in `.github/ISSUE_TEMPLATE/`. Issues labeled `human-task` need a
   person (hardware, accounts, a bench) and aren't for agents. The overall
   order of work is tracked in the roadmap issue (#22).
2. **Plan.** Write the plan in the issue (or in the PR description for small
   changes) before editing anything. Check the issue's *Verify first* items.
   If one turns out false, stop and comment on the issue.
3. **Branch from `dev`:**
   ```sh
   git fetch origin
   git switch -c <type>/<issue#>-<slug> origin/dev   # e.g. hw/9-cat-ptt-circuits
   ```
   Types: `hw`, `fw`, `proto`, `docs`, `ci`, `enc`, `research`. Git worktrees
   (`git worktree add ../<repo>-<slug> -b <branch> origin/dev`) keep parallel
   work separate.
4. **Commit (signed) and verify** before pushing:
   ```sh
   git log --format='%h %G? %s' origin/dev..HEAD   # every line must show G
   ```
5. **Run the local checks** in [section 4](#4-local-checks).
6. **Push and open the PR against `dev`:**
   ```sh
   git push -u origin <branch>
   gh pr create --base dev --title "…" --body "Closes #<issue>

   …"
   ```
   Fill in the PR template checklist.
7. **Merge.** Required checks must pass, and the ruleset requires a PR plus signed
   commits. `dev` is the default branch, so `Closes #N` closes the issue when
   the PR merges. Merged branches are deleted automatically.
8. **Releases** go from `dev` to `main` through a release PR ([section 6](#6-releases-and-tags)).

---

## 4. Local checks

Run what applies to your change before pushing. CI runs the same checks once #3
lands; until then, these commands are the reference.

**Licensing (every PR):**
```sh
uvx --from 'reuse[charset-normalizer]' reuse lint
```
Every new file needs SPDX headers or must be covered by
[`REUSE.toml`](../REUSE.toml). Which license a path gets is defined there and in
[AGENTS.md → Licensing](../AGENTS.md#licensing-reuse).

**KiCad version consistency (hardware PRs):** the files must match
[`KICAD_VERSION`](../KICAD_VERSION). KiCad files record only `generator_version
"10.0"` plus a format date (`.kicad_sch` `(version 20260306)`, `.kicad_pcb`
`(version 20260206)`). A quick manual check:
```sh
grep -rhoE --include='*.kicad_sch' --include='*.kicad_pcb' \
  '\((generator_version "[^"]+"|version [0-9]{8})\)' hardware/ | sort | uniq -c
```
The scripted check in `tools/` arrives with #3. If you saved files with a newer
KiCad, bump `KICAD_VERSION` in the same PR.

**ERC for schematic changes, DRC for PCB changes (both must pass to merge):**
```sh
kicad-cli sch erc --severity-all --exit-code-violations --format json \
  -o erc.json hardware/boards/<board>/rev<X>/<board>.kicad_sch

kicad-cli pcb drc --schematic-parity --severity-all --exit-code-violations \
  --format json -o drc.json hardware/boards/<board>/rev<X>/<board>.kicad_pcb
```
A nonzero exit code means violations. Fix them, or document an intentional
exclusion in the KiCad project (not in CI), and say why in the PR. Agents can run
the same checks through Konnect's review tools.

**Silkscreen revision (hardware PRs):** the board text must use `${REVISION}`,
and the title-block revision must match the `rev<X>` folder (see
[AGENTS.md → Hardware](../AGENTS.md#hardware-kicad)). The automated check comes
with #3.

**Firmware and protocol:** host tests and golden-vector tests. Commands are
added with #14 (firmware) and #13 (protocol).

---

## 5. Recipes by type of work

### Research or decision issue
1. Answer every question in the issue, with a source for each claim. Check the
   license of any design or code you look at and follow the third-party rules
   in [AGENTS.md → Licensing](../AGENTS.md#licensing-reuse): reuse only
   license-compatible material, keeping its notices. GPL, LGPL, AGPL and
   unlicensed sources are **facts only, never copied**.
2. Write findings to the path the issue names, and for decisions copy
   [`ADR-0000-template.md`](decisions/ADR-0000-template.md) to
   `docs/decisions/ADR-NNNN-<slug>.md`. Accepted ADRs are never edited; a later
   ADR supersedes them.
3. Update [`constraints.md`](requirements/constraints.md) when a *(verify)*
   value gets confirmed or corrected.
4. For parts: prefer **FCC-certified modules** (record the FCC ID and follow the
   integration guide), with at least two sources and an active lifecycle.
   Record alternates.

### Hardware change (KiCad)
1. **Agents: Konnect only.** Load the `konnect` skill, then the task skill
   (`kicad-schematic`, `kicad-pcb`, `kicad-library`, `kicad-review`,
   `kicad-manufacture`). Never hand-edit `.kicad_*` files.
2. Put new symbols, footprints and 3D models in `hardware/lib/…`, referenced
   through the project library tables with `${KIPRJMOD}`-relative paths. Never
   point at personal global libraries.
3. Keep the silkscreen template intact: project name, variant, `${REVISION}`,
   `${ISSUE_DATE}`, "Designed by Reid Crowe, N0RC", and the license mark
   required by [AGENTS.md](../AGENTS.md#hardware-kicad).
4. **New board revision:** copy `rev<X>` to `rev<X+1>`, update the title-block
   revision and date, and note it in [`CHANGELOG.md`](../CHANGELOG.md).
5. Run ERC and DRC ([section 4](#4-local-checks)). Don't commit fab outputs; CI
   builds them for releases.

### Firmware change
Portable logic goes in `firmware/app/` with host tests in `firmware/test/`, and
SDK glue in `firmware/platform/<sdk>/`. **PTT fail-safes are mandatory**, and
every new code path needs a test ([AGENTS.md → Firmware](../AGENTS.md#firmware)).
The SDK, build and flash commands are defined by #7 and #14 and will be added
here.

### Protocol change
Edit the spec in [`protocol/`](../protocol/), bump the protocol version, and
regenerate or extend the golden byte vectors in the same PR. Host apps pin a
`proto-v…` release, so never change a released version in place. Details
arrive with #13.

### Enclosure change
Edit the parametric CAD source in `hardware/enclosure/<variant>/`. STL/3MF files
are exported for releases, not committed (#19). Keep the antenna keep-out
area free of metal, and use ASA for the automotive variant.

### Human tasks
`human-task` issues (bench tests, printing, measurements, account actions) are
done by a person. Record the results in the issue and, where relevant, under
`docs/bringup/` or `docs/compliance/` through a normal PR.

---

## 6. Releases and tags

`main` receives releases only, through a `dev` → `main` PR. Tags:

| Artifact | Tag | Example |
|---|---|---|
| Hardware | `hw-<variant>-rev<X>-v<semver>` | `hw-R-revA-v1.0` |
| Firmware | `fw-v<semver>` | `fw-v0.1.0` |
| Protocol | `proto-v<semver>` | `proto-v1.0.0` |

A hardware tag must match the board's title-block revision and folder. Release
assets (Gerbers, drill, BOM, pick-and-place, PDFs, STL/3MF, firmware images)
are produced by CI and attached to the GitHub release.

---

## 7. Troubleshooting

| Symptom | Fix |
|---|---|
| Commit shows `N` or `E` in `git log --format='%G?'` | Signing isn't set up, or your key isn't in `allowed_signers` ([section 1](#signed-commits-required)). Don't push unsigned commits; the ruleset rejects them on `dev`/`main`. |
| Push to `dev` or `main` rejected | Expected. Push a feature branch and open a PR. |
| `uvx reuse lint` fails with an encoding / `charset_normalizer` error | Use `uvx --from 'reuse[charset-normalizer]' reuse lint`. |
| `reuse lint` reports an unused license | A license text in `LICENSES/` must be used by at least one file. Keep `REUSE.toml` and `LICENSES/` in step. |
| `kicad-cli: command not found` | Use the full path from [section 1](#kicad-cli-location), or add KiCad's `bin` to `PATH`. |
| KiCad version check fails after saving | You saved with a different KiCad. Use the version in `KICAD_VERSION`, or bump it in the same PR if the project is moving forward. |
| Issue closed but PR not merged, or the reverse | `Closes #N` only acts on merges into `dev` (the default branch). Check the PR base. |

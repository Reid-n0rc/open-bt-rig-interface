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
4. **Signed commits only.**
5. **Open a PR to `dev`** with `Closes #N` and fill in the PR template. `main` is
   updated only by release PRs.
6. KiCad work uses **KiCad ≥ 10.0.6** through the Konnect tools. Keep
   [`KICAD_VERSION`](KICAD_VERSION) in step with the files.
7. Add SPDX/REUSE licensing for every new file (`uvx reuse lint`).

By contributing you agree your contribution is licensed under the license of the
path it touches (see [`LICENSE`](LICENSE)).

---

## Ways to contribute

| You want to… | Do this |
|---|---|
| Report a bug | Open a **Task** issue and add the `bug` label. Include what you expected, what happened, the board variant and revision (from the silkscreen), the firmware version, the host OS and the radio model. |
| Propose a feature or change | Open a **Task** issue describing the problem first, then the proposed change. Larger ideas start as a **Research / decision** issue. |
| Investigate or decide something | Open a **Research / decision** issue. It ends in a findings doc or an ADR in [`docs/decisions/`](docs/decisions/). |
| Help with physical work | Pick up a **Human task** issue (`human-task`): bench tests, printing and fitting enclosures, measurements, on-air checks. These need a person and real hardware. |
| Ask a question | Open a **Task** issue and add the `question` label. |

Blank issues are disabled, so pick the template that fits best. The roadmap
tracking issue lists what is planned and in what order.

## Picking up work

- Search open issues first. Comment on an issue to claim it before you start.
- One issue per PR. If the work grows, split it into new issues.
- **AI agents take only agent-sized issues that aren't labeled `human-task`.**
  If an issue's "Verify first" item turns out false, stop and comment on the
  issue instead of guessing.
- Plan in the issue before making changes (approach, files, checks).

## Hardware contributions

- Use the KiCad version in [`KICAD_VERSION`](KICAD_VERSION) (10.0.6 or newer). If
  you save files with a newer KiCad, bump `KICAD_VERSION` in the same PR.
- AI agents make every KiCad change through the Konnect tools and never
  hand-edit `.kicad_*` files (see [`AGENTS.md`](AGENTS.md)).
- **ERC must pass for any schematic change and DRC must pass for any PCB
  change.** Until the CI gate is required, paste your local
  `kicad-cli sch erc` / `kicad-cli pcb drc` output in the PR.
- The PCB silkscreen shows the revision through `${REVISION}` (and
  `${ISSUE_DATE}`); never type the revision as fixed text. The title-block
  revision must match the `rev<X>` folder.
- Put new symbols, footprints and 3D models in `hardware/lib/`, not in your
  personal KiCad libraries.
- Don't commit Gerbers, drill files or other fabrication outputs. CI generates
  them for releases.
- For radio modules, follow the manufacturer's integration guide (antenna,
  keep-out area, trace), and record the part choice and alternates in an ADR.
- Part choices need at least two sources and an active lifecycle.

## Firmware and protocol contributions

- Portable logic goes in `firmware/app/` with host tests in `firmware/test/`;
  SDK-specific code goes in `firmware/platform/<sdk>/`. **Every new code path
  gets a test.**
- The core firmware stays radio-agnostic: CAT bytes pass through unchanged.
- **The PTT fail-safe is mandatory and must never be weakened:** PTT off at
  boot, reset, brownout, disconnect and watchdog timeout, plus a maximum-TX
  timer.
- Changes to the host-device protocol in [`protocol/`](protocol/) bump the
  protocol version and update the golden byte vectors in the same PR.

## Documentation contributions

- Keep docs app-neutral: this project serves any host application.
- Mark unverified values as **(verify)** and link the issue that will confirm them.
- Record significant decisions as ADRs using
  [`docs/decisions/ADR-0000-template.md`](docs/decisions/ADR-0000-template.md).

## Safety and compliance

- **Never claim that a design, board or product is FCC, ISED or CE certified or
  compliant.** Using a pre-certified radio module is not a product
  certification. Only the maintainer states compliance status (see
  [`GOVERNANCE.md`](GOVERNANCE.md)).
- Don't submit changes that disable or bypass the PTT fail-safe, the maximum-TX
  timer or input protection, even for testing. Use a local branch for experiments.
- RF, transmitter and automotive high-voltage testing is `human-task` work, done
  with a dummy load and appropriate precautions.

## Third-party material

- Before bringing in code, schematics, footprints, 3D models, datasheet
  excerpts or other material from another project, check that its license is
  compatible with the path it lands in.
- Keep the original copyright and license notices, and **record every
  third-party item and its required notices in
  [`THIRD_PARTY.md`](THIRD_PARTY.md)** in the same PR.
- Material under copyleft licenses (GPL, LGPL, AGPL), or with no license, may be
  used for facts only. Never copy it into this repo.

## AI-assisted contributions

- AI agents are welcome and follow [`AGENTS.md`](AGENTS.md) exactly like
  everyone else.
- The person submitting the PR is responsible for its content.
- Credit the agent with a `Co-Authored-By:` trailer in the commit message.
- Don't name private, paid or credentialed tools or services in tracked files,
  issues or PRs. Keep that setup in your untracked `AGENTS.local.md` /
  `CLAUDE.local.md`.

## Conduct

Be respectful and constructive. Critique designs and code, not people. No
harassment, personal attacks or discriminatory language. The maintainer may
edit, hide or lock discussions and decline contributions that don't meet these
expectations; see [`GOVERNANCE.md`](GOVERNANCE.md).

## More information

- Environment setup, local checks and recipes: [developer guide](docs/developer-guide.md).
- Roles, decisions, merging and releases: [`GOVERNANCE.md`](GOVERNANCE.md).
- Commercial use: [`COMMERCIAL.md`](COMMERCIAL.md).
- Third-party notices: [`THIRD_PARTY.md`](THIRD_PARTY.md).

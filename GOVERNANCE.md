<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Governance

This document says **who decides what** in open-bt-rig-interface. *How* to do the
work (issues, branches, commits, PRs) is in [`AGENTS.md`](AGENTS.md); this
document doesn't repeat those rules. Where the two appear to disagree, raise an
issue. Until it's resolved, `AGENTS.md` governs day-to-day work and this document
governs authority.

## Scope and principles

The project is an open **hardware and firmware** Bluetooth interface between an
amateur-radio transceiver and a host, carrying CAT serial, PTT (RTS/DTR) and audio.
Decisions are weighed against these principles, in order:

1. **Safety first.** The device keys a transmitter. PTT fail-safes, behavior in a
   strong RF field, and power-input robustness outrank features and schedule.
2. **Open.** Sources and design decisions are public. Licensing is defined only in
   [`LICENSE`](LICENSE), [`LICENSES/`](LICENSES/) and
   [`COMMERCIAL.md`](COMMERCIAL.md); this document doesn't restate it.
3. **App-neutral.** No design, name or protocol is tied to a particular host
   application. Host apps are consumers of published protocol releases.
4. **Standards over custom.** Standard Bluetooth profiles come first, so the device
   works on iOS, macOS, Android, Windows and Linux. Custom extensions are optional.

## Roles

| Role | Who | Can |
|---|---|---|
| **Maintainer** | Reid Crowe, N0RC ([@Reid-n0rc](https://github.com/Reid-n0rc)) | Everything below, plus: merge PRs, cut releases, appoint or remove reviewers, set priorities, make final decisions, grant commercial licenses, change governance. The maintainer is the project's final decision-maker. |
| **Reviewer** | Appointed by the maintainer, listed in this file | Review and approve PRs; merge PRs the maintainer delegates to them. Can't cut releases or change governance. |
| **Contributor** | Anyone | Open issues, propose plans, submit PRs, review and comment. |
| **AI agent** | Automated assistants working on a contributor's behalf | Prepare plans, issues, branches, commits and PRs under `AGENTS.md`. **Can't merge, approve, release, or do `human-task` work** (anything needing a person, physical hardware, test equipment or accounts). A person is always accountable for an agent's output. |

Current reviewers: *none yet.*

## Decision-making

- **Everyday decisions** happen in the issue or PR by lazy consensus. If no one
  objects after reasonable time for discussion, the plan in the issue stands.
- **Significant decisions** are recorded as ADRs in
  [`docs/decisions/`](docs/decisions/). That covers module or major-part selection,
  board or variant strategy, power architecture, protocol design, licensing, and
  anything that is hard to reverse. An ADR is accepted when the maintainer approves
  the PR that adds it.
- **Changing a decision** takes a new ADR that supersedes the old one. Accepted ADRs
  are never edited to change their meaning.
- **Disagreements** are settled by the maintainer, who records the reasoning in the
  issue or ADR.

## Merge policy

- All changes reach `dev` through a PR linked to an issue. `main` and `dev` are
  protected: PRs only, no force-push or deletion. Signed commits (SSH or GPG)
  are preferred but not required; the ruleset doesn't enforce signatures.
- A PR can be merged when its required checks pass, its
  author has accepted the contributor terms, and it meets its issue's acceptance
  criteria. Decisions are made through the linked issue and PR review. Hardware PRs must pass ERC for schematic
  changes and DRC for PCB changes.
- Only the maintainer, or a reviewer the maintainer delegates to, merges PRs.
  Authors, human or agent, don't merge their own PRs unless they're the maintainer.

## Release policy

- **Release authority belongs to the maintainer alone.** Only the maintainer opens
  and merges the `dev` → `main` release PR and creates release tags (`hw-…`, `fw-…`,
  `proto-…`, using the scheme in [`AGENTS.md`](AGENTS.md#releases-and-tags)).
  Reviewers and agents may prepare release notes but can't release.
- A **hardware release** (`hw-<variant>-rev<X>-v…`) needs all of the following:
  - clean ERC and DRC;
  - a completed design review;
  - bring-up results recorded in [`docs/bringup/`](docs/bringup/);
  - revision text on the silkscreen that matches the tag.
- A **firmware release** (`fw-v…`) requires passing tests, and a note of which
  hardware revisions it supports.
- A **protocol release** (`proto-v…`) follows semantic versioning. Breaking changes
  need a major version bump and an ADR, and host applications pin a released version.

## Safety and compliance

- No contributor may claim that the product is FCC-certified, or certified under any
  other regulatory scheme. Only the maintainer states compliance status, in
  [`docs/compliance/`](docs/compliance/). Using a certified radio module covers the
  module's own grant only. The finished product still needs its own work (for
  example FCC Part 15B), and it isn't certified until that's documented.
- Changes that affect PTT fail-safes, power-input protection or isolation need the
  maintainer's explicit approval in the PR.

## Contributions and licensing

- The project is **dual-licensed**: public non-commercial terms, plus separate
  commercial licenses granted by the maintainer (see [`COMMERCIAL.md`](COMMERCIAL.md)).
  To keep that possible, **every contribution must be made under the contributor
  terms in [`CONTRIBUTING.md`](CONTRIBUTING.md)** (a non-exclusive license grant,
  including relicensing rights; contributors keep their copyright). Those terms
  let the maintainer license contributions
  under both the public and the commercial terms. The maintainer won't merge a PR
  whose author hasn't accepted them. This document doesn't restate the license or
  contributor terms themselves.
- Third-party material must be license-compatible with **both** the public and the
  commercial terms, and must keep its notices, recorded in
  [`THIRD_PARTY.md`](THIRD_PARTY.md). Sources under GPL, LGPL or AGPL, or
  with no license, may be used for facts only, never copied.

## Conduct and reporting

- Be respectful and assume good faith. The maintainer moderates issues, PRs and
  discussions, and may lock threads or block participants who are abusive.
- Report **security or safety** problems privately to the maintainer, not in a
  public issue. Examples: firmware that can key PTT unexpectedly, or a power fault
  that can damage a radio.

## Relationship to other projects

This project is independent. It was inspired by the needs of a mobile FT8 client
([patrickrb/FT8AF](https://github.com/patrickrb/FT8AF)), but that application, like
any other, is only a consumer of the published protocol and hardware. No other
project's roadmap binds this one.

## Changing this document

Changes to governance follow the normal process: an issue and a PR to `dev`. They
take effect when the maintainer merges the PR.

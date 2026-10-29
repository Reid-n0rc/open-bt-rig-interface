#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Decide which areas of .github/workflows/checks.yml a run checks.

A PR to `dev` checks only the areas its changed files touch. Everything else
is a full regression: PRs to `main` (a release), pushes to `dev`/`main`,
release tags and manual runs. A change to the CI itself (this script, the
workflow) is a full regression too. The quick repository-wide checks (REUSE,
KiCad version, silkscreen) always run; the ERC/DRC gate selects its own
projects (tools/kicad_ci/select_projects.py).
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

# Path prefixes per area. The firmware host tests generate the C golden
# vectors from protocol/vectors/ with tools/protocol/proto_codec.py.
AREAS: dict[str, tuple[str, ...]] = {
    "firmware": ("firmware/", "protocol/", "tools/protocol/"),
    "protocol": ("protocol/", "tools/protocol/", "tools/cat_loopback/"),
}
GLOBAL_PREFIXES = ("tools/ci/",)
GLOBAL_FILES = {".github/workflows/checks.yml"}
RELEASE_BRANCH = "main"


def is_full(event: str, base_ref: str) -> bool:
    """Whether the event alone calls for the full regression."""
    return event != "pull_request" or base_ref == RELEASE_BRANCH


def select(changed: list[str], full: bool = False) -> dict[str, bool]:
    """{"full": …, <area>: …}: what to check for these changed files."""
    full = full or any(
        p.strip() in GLOBAL_FILES or p.strip().startswith(GLOBAL_PREFIXES) for p in changed)
    plan = {"full": full}
    for name, prefixes in AREAS.items():
        plan[name] = full or any(p.strip().startswith(prefixes) for p in changed)
    return plan


def write_outputs(pairs: dict[str, str], github_output: str | None) -> None:
    lines = [f"{k}={v}" for k, v in pairs.items()]
    if github_output:
        with open(github_output, "a", encoding="utf-8") as fh:
            fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--event", required=True, help="github.event_name")
    ap.add_argument("--base-ref", default="", help="the PR's base branch (github.base_ref)")
    ap.add_argument("--changed-files", help="file with one changed path per line (PRs)")
    ap.add_argument("--github-output", help="append key=value outputs to this file")
    args = ap.parse_args(argv)

    full = is_full(args.event, args.base_ref)
    changed: list[str] = []
    if not full:
        if not args.changed_files:
            ap.error("a PR to a non-release branch needs --changed-files")
        changed = Path(args.changed_files).read_text().splitlines()
    plan = select(changed, full)
    write_outputs({k: str(v).lower() for k, v in plan.items()}, args.github_output)
    return 0


if __name__ == "__main__":
    sys.exit(main())

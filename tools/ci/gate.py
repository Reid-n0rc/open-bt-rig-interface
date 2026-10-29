#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""`CI gate` in .github/workflows/checks.yml: did every needed job pass?

Reads the workflow's `needs` context (JSON) on stdin. A job passes if it
succeeded, or if it was skipped because its area didn't change. In the full
regression nothing may be skipped, except the jobs that only run on release
tags when this isn't one. Selecting the checks (`changes`) must succeed.
"""

from __future__ import annotations

import argparse
import json
import os
import sys

SELECT_JOB = "changes"


def failures(needs: dict, full: bool, tag_only: set[str], is_tag: bool) -> dict[str, str]:
    bad = {}
    for job, n in needs.items():
        result = n.get("result", "missing")
        if result == "success":
            continue
        if result == "skipped" and job != SELECT_JOB:
            if not full or (job in tag_only and not is_tag):
                continue
        bad[job] = result
    if SELECT_JOB not in needs:
        bad[SELECT_JOB] = "missing"
    return bad


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--full", choices=("true", "false", ""), default="",
                    help="the `full` output of `changes` (empty if it didn't run)")
    ap.add_argument("--tag-only", action="append", default=[],
                    help="a job that runs only on release tags (repeatable)")
    ap.add_argument("--is-tag", action="store_true", help="this run is for a tag")
    args = ap.parse_args(argv)

    needs = json.load(sys.stdin)
    full = args.full == "true"
    bad = failures(needs, full, set(args.tag_only), args.is_tag)
    kind = "Full regression" if full else "Checks for the changed areas"
    lines = ["| Job | Result |", "|---|---|"]
    lines += [f"| {job} | {n.get('result', 'missing')} |" for job, n in needs.items()]
    lines.append("")
    lines.append(f"{kind}: {'not passed: ' + ', '.join(sorted(bad)) if bad else 'passed'}.")
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as fh:
            fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))
    if bad:
        print(f"::error::{kind}: not passed: {bad}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

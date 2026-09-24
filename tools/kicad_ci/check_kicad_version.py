#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Check that every KiCad file and every doc agrees with KICAD_VERSION.

- Each *.kicad_sch/pcb/sym/mod must carry the format `(version ...)` and
  `(generator_version ...)` that KICAD_VERSION records; each *.kicad_pro its
  `meta.version`. A file saved by an older or newer KiCad fails, so saving
  with a newer KiCad means bumping KICAD_VERSION in the same PR.
- Docs (*.md, except CHANGELOG.md) that quote a minimum KiCad version
  ("KiCad >= X.Y.Z", "X.Y.Z or newer", "kicad/kicad:X.Y.Z") must quote
  KICAD_MIN_VERSION, and AGENTS.md must quote it at least once.

Files record only "10.0", never the patch level; the patch minimum is enforced
by pinning the CI image to KICAD_MIN_VERSION (see select_projects.py image).
With no KiCad files in the repo only KICAD_VERSION and the docs are checked.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path

from kicad_files import (KICAD_SUFFIXES, child_value, iter_repo_files, parse_sexpr,
                         read_kicad_version_file)

REQUIRED_KEYS = ("KICAD_MIN_VERSION", "KICAD_GENERATOR_VERSION", "KICAD_SCH_FORMAT",
                 "KICAD_PCB_FORMAT", "KICAD_SYM_FORMAT", "KICAD_PRO_META_VERSION")

# suffix -> (accepted top-level heads, KICAD_VERSION key of the format token).
# Footprints are written with the board file format, so they share KICAD_PCB_FORMAT.
SEXPR_FORMATS = {
    ".kicad_sch": (("kicad_sch",), "KICAD_SCH_FORMAT"),
    ".kicad_pcb": (("kicad_pcb",), "KICAD_PCB_FORMAT"),
    ".kicad_sym": (("kicad_symbol_lib",), "KICAD_SYM_FORMAT"),
    ".kicad_mod": (("footprint", "module"), "KICAD_PCB_FORMAT"),
}

DOC_PATTERNS = (
    re.compile(r"KiCad[^\n\d]{0,60}?(?:≥|>=)\s*\**\s*v?(\d+\.\d+(?:\.\d+)?)"),
    re.compile(r"\b(\d+\.\d+\.\d+) or newer"),
    re.compile(r"kicad/kicad:(\d+\.\d+\.\d+)\b"),
)
DOC_EXCLUDE = {"CHANGELOG.md"}


def check_version_file(v: dict[str, str]) -> list[str]:
    errors = [f"KICAD_VERSION: {key} is missing" for key in REQUIRED_KEYS if not v.get(key)]
    if errors:
        return errors
    if not re.fullmatch(r"\d+\.\d+\.\d+", v["KICAD_MIN_VERSION"]):
        errors.append(f"KICAD_VERSION: KICAD_MIN_VERSION {v['KICAD_MIN_VERSION']!r} is not X.Y.Z")
    elif not v["KICAD_MIN_VERSION"].startswith(v["KICAD_GENERATOR_VERSION"] + "."):
        errors.append(f"KICAD_VERSION: KICAD_GENERATOR_VERSION {v['KICAD_GENERATOR_VERSION']} "
                      f"does not match KICAD_MIN_VERSION {v['KICAD_MIN_VERSION']}")
    for key in ("KICAD_SCH_FORMAT", "KICAD_PCB_FORMAT", "KICAD_SYM_FORMAT", "KICAD_PRO_META_VERSION"):
        if not v[key].isdigit():
            errors.append(f"KICAD_VERSION: {key} {v[key]!r} is not a number")
    return errors


def check_sexpr_file(rel: str, text: str, v: dict[str, str]) -> list[str]:
    suffix = Path(rel).suffix
    heads, key = SEXPR_FORMATS[suffix]
    try:
        tree = parse_sexpr(text)
    except ValueError as exc:
        return [f"{rel}: not a readable KiCad file ({exc})"]
    if tree[0] not in heads:
        return [f"{rel}: expected a ({' or '.join(heads)} ...) file, found ({tree[0]} ...)"]
    errors = []
    version = child_value(tree, "version")
    if version != v[key]:
        errors.append(f"{rel}: format (version {version}) but KICAD_VERSION {key}={v[key]}; "
                      f"save it with KiCad {v['KICAD_MIN_VERSION']} (or bump KICAD_VERSION in this PR)")
    gen = child_value(tree, "generator_version")
    if gen != v["KICAD_GENERATOR_VERSION"]:
        errors.append(f"{rel}: generator_version {gen!r} but KICAD_VERSION "
                      f"KICAD_GENERATOR_VERSION={v['KICAD_GENERATOR_VERSION']}")
    return errors


def check_pro_file(rel: str, text: str, v: dict[str, str]) -> list[str]:
    try:
        meta = json.loads(text).get("meta", {})
    except (json.JSONDecodeError, AttributeError) as exc:
        return [f"{rel}: not a readable KiCad project file ({exc})"]
    version = meta.get("version") if isinstance(meta, dict) else None
    if str(version) != v["KICAD_PRO_META_VERSION"]:
        return [f"{rel}: meta.version {version!r} but KICAD_VERSION "
                f"KICAD_PRO_META_VERSION={v['KICAD_PRO_META_VERSION']}"]
    return []


def check_kicad_files(root: Path, v: dict[str, str]) -> tuple[list[str], int]:
    errors, count = [], 0
    for rel in iter_repo_files(root, KICAD_SUFFIXES):
        count += 1
        text = (root / rel).read_text(encoding="utf-8")
        if rel.endswith(".kicad_pro"):
            errors += check_pro_file(rel, text, v)
        else:
            errors += check_sexpr_file(rel, text, v)
    return errors, count


def check_docs(root: Path, v: dict[str, str]) -> list[str]:
    errors = []
    quoted_in_agents = False
    for rel in iter_repo_files(root, (".md",)):
        if rel in DOC_EXCLUDE or rel.startswith("docs/references/cache/"):
            continue
        for lineno, line in enumerate((root / rel).read_text(encoding="utf-8").splitlines(), 1):
            for pattern in DOC_PATTERNS:
                for m in pattern.finditer(line):
                    if m.group(1) != v["KICAD_MIN_VERSION"]:
                        errors.append(f"{rel}:{lineno}: quotes KiCad {m.group(1)} but KICAD_VERSION "
                                      f"KICAD_MIN_VERSION={v['KICAD_MIN_VERSION']}")
                    elif rel == "AGENTS.md":
                        quoted_in_agents = True
    if (root / "AGENTS.md").is_file() and not quoted_in_agents:
        errors.append(f"AGENTS.md: does not quote the minimum KiCad version "
                      f"(expected 'KiCad ≥ {v['KICAD_MIN_VERSION']}')")
    return errors


def run(root: Path) -> tuple[list[str], str]:
    try:
        v = read_kicad_version_file(root)
    except OSError as exc:
        return [f"KICAD_VERSION: cannot read ({exc})"], ""
    errors = check_version_file(v)
    if errors:
        return errors, ""
    file_errors, count = check_kicad_files(root, v)
    errors += file_errors + check_docs(root, v)
    summary = (f"KiCad {v['KICAD_MIN_VERSION']}: {count} KiCad file(s) checked"
               + ("" if count else " (no KiCad files yet)") + ", docs checked")
    return errors, summary


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=".", help="repository root")
    args = ap.parse_args(argv)
    errors, summary = run(Path(args.root))
    annotate = os.environ.get("GITHUB_ACTIONS") == "true"
    for e in errors:
        print(f"::error::{e}" if annotate else f"error: {e}")
    if errors:
        print(f"KiCad version check failed: {len(errors)} problem(s)")
        return 1
    print(f"KiCad version check passed. {summary}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

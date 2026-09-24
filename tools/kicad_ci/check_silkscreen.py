#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Check the revision and required markings on every board's silkscreen.

For each PCB under hardware/boards/<board>/rev<X>/ (see AGENTS.md, Hardware):

- the title block sets a revision, equal to <X>, and an issue date;
- front silkscreen text uses the ${REVISION} and ${ISSUE_DATE} text variables;
- no silkscreen text hard-codes a revision ("Rev A", "revB", "REV: 2");
- silkscreen text includes the project name, "Designed by Reid Crowe, N0RC"
  and the CC BY-NC-SA 4.0 license mark (project text variables are resolved);
- on a hardware tag build (hw-<variant>-rev<X>-v<semver>), a board matches the
  tag: folder rev<X> and a board folder named <variant> or ending in -<variant>.

The variant marking is not checked automatically: its form (text variable,
folder name or logo) is not decided yet. Hidden text is ignored. With no
boards the check passes (except on a hardware tag build, which needs a board).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from pathlib import Path, PurePosixPath

from kicad_files import Str, child_value, iter_repo_files, parse_sexpr, walk

BOARDS_DIR = "hardware/boards"
REV_DIR = re.compile(r"rev([A-Z0-9]+)")
TAG = re.compile(r"hw-(?P<variant>[A-Za-z0-9]+(?:-[A-Za-z0-9]+)*?)-rev(?P<rev>[A-Z0-9]+)"
                 r"-v(?P<version>\d+\.\d+(?:\.\d+)?(?:-[0-9A-Za-z.-]+)?)")
# node -> index of its text. Footprint fields (`property`) count when shown on silkscreen.
TEXT_NODES = {"gr_text": 1, "gr_text_box": 1, "fp_text": 2, "fp_text_box": 1, "property": 2}
VAR = re.compile(r"\$\{([^}]*)\}")
HARD_CODED_REV = (
    re.compile(r"(?i)\brev(?:ision)?(?:[.:#]\s*|\s+)[A-Z0-9]"),  # "Rev A", "REV: 2", "Revision 1"
    re.compile(r"(?i:\brev)[A-Z0-9]{1,2}\b"),                       # "revA", "REV2"
)
REQUIRED_FRONT_VARS = ("${REVISION}", "${ISSUE_DATE}")
REQUIRED_MARKS = {
    "project name": "open-bt-rig-interface",
    "designer credit": "Designed by Reid Crowe, N0RC",
    "license mark": "CC BY-NC-SA 4.0",
}


def is_hidden(node: list) -> bool:
    for n in walk(node):
        if any(a == "hide" and not isinstance(a, Str) for a in n[1:]):  # bare `hide` atom
            return True
        if n[0] == "hide" and len(n) > 1 and n[1] == "yes":
            return True
    return False


def silkscreen_texts(tree: list) -> list[tuple[str, str]]:
    """(layer, text) for every visible text item on a silkscreen layer."""
    found = []
    for node in walk(tree):
        idx = TEXT_NODES.get(node[0])
        if idx is None or len(node) <= idx or not isinstance(node[idx], str):
            continue
        layer = child_value(node, "layer")
        if isinstance(layer, str) and layer.endswith(".SilkS") and not is_hidden(node):
            found.append((layer, node[idx]))
    return found


def project_text_variables(pcb: Path) -> dict[str, str]:
    pro = pcb.with_suffix(".kicad_pro")
    try:
        data = json.loads(pro.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    tv = data.get("text_variables") if isinstance(data, dict) else None
    return {str(k): str(v) for k, v in tv.items()} if isinstance(tv, dict) else {}


def resolve(text: str, variables: dict[str, str]) -> str:
    for _ in range(3):  # nested variables, bounded
        new = VAR.sub(lambda m: variables.get(m.group(1), m.group(0)), text)
        if new == text:
            break
        text = new
    return text


def normalise(text: str) -> str:
    return " ".join(text.split()).casefold()


def hard_coded_revision(text: str) -> str | None:
    masked = VAR.sub("\0", text)
    for pattern in HARD_CODED_REV:
        m = pattern.search(masked)
        if m:
            return m.group(0)
    return None


def check_board(root: Path, rel: str) -> tuple[list[str], dict | None]:
    parts = PurePosixPath(rel).parts
    rev_match = REV_DIR.fullmatch(parts[3]) if len(parts) == 5 else None
    if not rev_match:
        return [f"{rel}: board files must be in {BOARDS_DIR}/<board>/rev<X>/ (X = A, B, ... or 1, 2, ...)"], None
    board, folder_rev = parts[2], rev_match.group(1)
    info = {"path": rel, "board": board, "rev": folder_rev}
    try:
        tree = parse_sexpr((root / rel).read_text(encoding="utf-8"))
    except ValueError as exc:
        return [f"{rel}: not a readable KiCad PCB ({exc})"], info
    if tree[0] != "kicad_pcb":
        return [f"{rel}: expected a (kicad_pcb ...) file"], info

    errors = []
    title = next((n for n in tree[1:] if isinstance(n, list) and n and n[0] == "title_block"), None)
    title_rev = child_value(title, "rev") if title else None
    title_date = child_value(title, "date") if title else None
    if not title_rev:
        errors.append(f"{rel}: title block revision is not set (expected {folder_rev!r} for folder {parts[3]})")
    elif title_rev != folder_rev:
        errors.append(f"{rel}: title block revision {title_rev!r} does not match folder {parts[3]} "
                      f"(expected {folder_rev!r})")
    if not title_date:
        errors.append(f"{rel}: title block date is not set, so ${{ISSUE_DATE}} would print nothing")

    texts = silkscreen_texts(tree)
    front = " ".join(t for layer, t in texts if layer == "F.SilkS")
    for var in REQUIRED_FRONT_VARS:
        if var not in front:
            errors.append(f"{rel}: no front silkscreen (F.SilkS) text contains {var}")
    for layer, text in texts:
        hit = hard_coded_revision(text)
        if hit:
            errors.append(f"{rel}: {layer} text {text!r} hard-codes a revision ({hit!r}); use ${{REVISION}}")

    variables = project_text_variables(root / rel)
    all_text = normalise(" ".join(resolve(t, variables) for _, t in texts))
    for what, mark in REQUIRED_MARKS.items():
        if normalise(mark) not in all_text:
            errors.append(f"{rel}: silkscreen is missing the {what} {mark!r}")
    return errors, info


def check_tag(tag: str, boards: list[dict]) -> list[str]:
    if not tag.startswith("hw-"):
        return []
    m = TAG.fullmatch(tag)
    if not m:
        return [f"tag {tag!r} does not follow hw-<variant>-rev<X>-v<semver> (e.g. hw-R-revA-v1.0)"]
    variant, rev = m.group("variant"), m.group("rev")
    matches = [b for b in boards if b["rev"] == rev
               and (b["board"] == variant or b["board"].endswith("-" + variant))]
    if not matches:
        have = ", ".join(sorted({f"{b['board']}/rev{b['rev']}" for b in boards})) or "none"
        return [f"tag {tag!r}: no board {BOARDS_DIR}/<{variant} or *-{variant}>/rev{rev}/ (boards: {have})"]
    return []


def run(root: Path, tag: str = "") -> tuple[list[str], str]:
    errors, boards = [], []
    pcbs = [r for r in iter_repo_files(root, (".kicad_pcb",)) if r.startswith(BOARDS_DIR + "/")]
    for rel in pcbs:
        errs, info = check_board(root, rel)
        errors += errs
        if info:
            boards.append(info)
    errors += check_tag(tag, boards)
    summary = f"{len(pcbs)} board(s) checked" + ("" if pcbs else " (no boards yet)")
    if tag.startswith("hw-"):
        summary += f", tag {tag}"
    return errors, summary


def tag_from_env() -> str:
    ref = os.environ.get("GITHUB_REF", "")
    return ref[len("refs/tags/"):] if ref.startswith("refs/tags/") else ""


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--root", default=".", help="repository root")
    ap.add_argument("--tag", default=None, help="release tag to check (default: from $GITHUB_REF on tag builds)")
    args = ap.parse_args(argv)
    tag = args.tag if args.tag is not None else tag_from_env()
    errors, summary = run(Path(args.root), tag)
    annotate = os.environ.get("GITHUB_ACTIONS") == "true"
    for e in errors:
        print(f"::error::{e}" if annotate else f"error: {e}")
    if errors:
        print(f"Silkscreen revision check failed: {len(errors)} problem(s)")
        return 1
    print(f"Silkscreen revision check passed: {summary}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

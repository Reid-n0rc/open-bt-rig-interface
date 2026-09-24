#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Decide which KiCad projects need ERC and/or DRC for a set of changed files.

Rules: a changed schematic triggers ERC, a changed PCB triggers DRC, and
project-wide files trigger both. Library, CI-script, workflow and
KICAD_VERSION changes trigger every check on every project.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path, PurePosixPath

HARDWARE_DIR = "hardware"
ERC_SUFFIXES = {".kicad_sch"}
DRC_SUFFIXES = {".kicad_pcb"}
BOTH_SUFFIXES = {".kicad_pro", ".kicad_dru"}
BOTH_NAMES = {"sym-lib-table", "fp-lib-table"}
GLOBAL_PREFIXES = ("hardware/lib/", "tools/kicad_ci/")
GLOBAL_FILES = {".github/workflows/kicad-checks.yml", "KICAD_VERSION"}
IMAGE_REPO = "kicad/kicad"


def is_ignored(path: str) -> bool:
    p = PurePosixPath(path)
    if any(part.endswith("-backups") or part == ".history" for part in p.parts[:-1]):
        return True
    name = p.name
    return (
        name.startswith("_autosave-")
        or name.endswith((".lck", ".kicad_prl"))
        or name == "fp-info-cache"
    )


def find_projects(root: Path) -> list[str]:
    hw = root / HARDWARE_DIR
    if not hw.is_dir():
        return []
    found = []
    for pro in hw.rglob("*.kicad_pro"):
        rel = pro.relative_to(root).as_posix()
        if not is_ignored(rel):
            found.append(rel)
    return sorted(found)


def owning_projects(path: str, projects: list[str]) -> list[str]:
    """Projects in the nearest ancestor directory of `path` that holds a .kicad_pro."""
    by_dir: dict[str, list[str]] = {}
    for pro in projects:
        by_dir.setdefault(PurePosixPath(pro).parent.as_posix(), []).append(pro)
    for parent in PurePosixPath(path).parents:
        owners = by_dir.get(parent.as_posix())
        if owners:
            return owners
    return []


def wanted_checks(path: str) -> tuple[bool, bool] | None:
    p = PurePosixPath(path)
    if p.suffix in ERC_SUFFIXES:
        return True, False
    if p.suffix in DRC_SUFFIXES:
        return False, True
    if p.suffix in BOTH_SUFFIXES or p.name in BOTH_NAMES:
        return True, True
    return None


def is_global(path: str) -> bool:
    return path in GLOBAL_FILES or path.startswith(GLOBAL_PREFIXES)


def select(changed: list[str], projects: list[str], root: Path, run_all: bool = False) -> list[dict]:
    flags = {pro: [run_all, run_all] for pro in projects}
    for path in (c.strip() for c in changed):
        if not path or is_ignored(path):
            continue
        if is_global(path):
            for f in flags.values():
                f[0] = f[1] = True
            continue
        wanted = wanted_checks(path)
        if wanted is None:
            continue
        for owner in owning_projects(path, projects):
            flags[owner][0] |= wanted[0]
            flags[owner][1] |= wanted[1]

    dir_counts: dict[str, int] = {}
    for pro in projects:
        d = PurePosixPath(pro).parent.as_posix()
        dir_counts[d] = dir_counts.get(d, 0) + 1

    plan = []
    for pro, (erc, drc) in flags.items():
        pro_path = PurePosixPath(pro)
        stem = pro_path.stem
        project_dir = pro_path.parent
        sch = (project_dir / f"{stem}.kicad_sch").as_posix()
        pcb = (project_dir / f"{stem}.kicad_pcb").as_posix()
        erc = erc and (root / sch).is_file()
        drc = drc and (root / pcb).is_file()
        if not (erc or drc):
            continue
        dir_name = project_dir.as_posix()
        plan.append({
            "name": dir_name if dir_counts[dir_name] == 1 else f"{dir_name}/{stem}",
            "slug": f"{dir_name}/{stem}".replace("/", "__"),
            "project": pro,
            "schematic": sch,
            "pcb": pcb,
            "erc": erc,
            "drc": drc,
        })
    return plan


def read_kicad_version(root: Path) -> str:
    for line in (root / "KICAD_VERSION").read_text().splitlines():
        line = line.strip()
        if line.startswith("KICAD_MIN_VERSION="):
            value = line.split("=", 1)[1].strip()
            if value:
                return value
    raise ValueError("KICAD_MIN_VERSION not found in KICAD_VERSION")


def write_outputs(pairs: dict[str, str], github_output: str | None) -> None:
    lines = [f"{k}={v}" for k, v in pairs.items()]
    if github_output:
        with open(github_output, "a", encoding="utf-8") as fh:
            fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--root", default=".", help="repository root")
    ap.add_argument("--github-output", help="append key=value outputs to this file")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("image", help="print the pinned KiCad container image")
    sel = sub.add_parser("select", help="print the ERC/DRC plan as a JSON matrix")
    sel.add_argument("--changed-files", help="file with one changed path per line")
    sel.add_argument("--all", action="store_true", help="check every project")
    args = ap.parse_args(argv)
    root = Path(args.root)

    if args.cmd == "image":
        write_outputs({"image": f"{IMAGE_REPO}:{read_kicad_version(root)}"}, args.github_output)
        return 0

    changed: list[str] = []
    if args.changed_files:
        changed = Path(args.changed_files).read_text().splitlines()
    elif not args.all:
        ap.error("select needs --changed-files or --all")
    plan = select(changed, find_projects(root), root, run_all=args.all)
    write_outputs(
        {"any": "true" if plan else "false", "matrix": json.dumps(plan, separators=(",", ":"))},
        args.github_output,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

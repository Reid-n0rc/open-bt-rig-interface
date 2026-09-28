#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Export the enclosure STL/3MF files per variant, headless (ADR-0010).

Finds every variant under hardware/enclosure/<V>/enclosure_<V>.scad, runs the
fit check (the enclosure intersected with the board model must be empty) and
exports each part listed in the file's ``export_parts`` as STL and 3MF.

    python3 tools/enclosure/export_enclosures.py --list
    python3 tools/enclosure/export_enclosures.py --out build/enclosure
    python3 tools/enclosure/export_enclosures.py --variant M --formats stl

Outputs are release artifacts: never commit them (.gitignore covers build/
and hardware/enclosure/**/*.stl|3mf).
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
ENCLOSURE_DIR = REPO_ROOT / "hardware" / "enclosure"
DEFAULT_OUT = REPO_ROOT / "build" / "enclosure"
FORMATS = ("stl", "3mf")
EMPTY_MARKER = "Current top level object is empty"

_PARTS_RE = re.compile(r"^\s*export_parts\s*=\s*\[([^\]]*)\]\s*;", re.MULTILINE)
_NAME_RE = re.compile(r'"([A-Za-z0-9_-]+)"')


@dataclass(frozen=True)
class Variant:
    name: str
    scad: Path
    parts: tuple[str, ...]


def parse_parts(text: str) -> tuple[str, ...]:
    """Return the names in ``export_parts = ["a", "b"];``."""
    m = _PARTS_RE.search(text)
    if not m:
        return ()
    return tuple(_NAME_RE.findall(m.group(1)))


def find_variants(enclosure_dir: Path = ENCLOSURE_DIR) -> list[Variant]:
    """Every <V>/enclosure_<V>.scad under enclosure_dir, sorted by name."""
    variants = []
    for d in sorted(p for p in enclosure_dir.iterdir() if p.is_dir()):
        scad = d / f"enclosure_{d.name}.scad"
        if not scad.is_file():
            continue
        parts = parse_parts(scad.read_text(encoding="utf-8"))
        if not parts:
            raise ValueError(f"{scad}: no export_parts = [...]; list")
        variants.append(Variant(d.name, scad, parts))
    return variants


def openscad_cmd(openscad: str, scad: Path, part: str, out: Path) -> list[str]:
    """Command line for one headless export. --hardwarnings makes any
    OpenSCAD warning (undefined variable, reassignment, ...) fail the run."""
    cmd = [openscad, "--hardwarnings", "-D", f'part="{part}"']
    if out.suffix == ".stl":
        cmd += ["--export-format", "binstl"]
    return cmd + ["-o", str(out), str(scad)]


def openscad_version(openscad: str) -> str:
    res = subprocess.run([openscad, "--version"], capture_output=True, text=True, check=False)
    text = (res.stdout + res.stderr).strip()
    m = re.search(r"OpenSCAD version (\S+)", text)
    if not m:
        raise RuntimeError(f"can't read the OpenSCAD version from: {text!r}")
    return m.group(1)


def _run(cmd: list[str]) -> subprocess.CompletedProcess:
    print("+ " + " ".join(cmd), flush=True)
    res = subprocess.run(cmd, capture_output=True, text=True, check=False)
    for line in (res.stdout + res.stderr).splitlines():
        if line.startswith(("ECHO", "WARNING", "ERROR", "TRACE")) or EMPTY_MARKER in line:
            print("  " + line)
    return res


def fit_check(openscad: str, v: Variant) -> bool:
    """True when the enclosure and the board model don't overlap."""
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "fit.stl"
        res = _run(openscad_cmd(openscad, v.scad, "fit", out))
        text = res.stdout + res.stderr
        if "ERROR" in text or "WARNING" in text:
            print(f"FAIL {v.name}: fit check did not run cleanly")
            return False
        if EMPTY_MARKER in text and not out.exists():
            print(f"ok   {v.name}: no collision between enclosure and board model")
            return True
        print(f"FAIL {v.name}: the enclosure collides with the board model "
              "(render part=\"fit\" to see where)")
        return False


def export_variant(openscad: str, v: Variant, out_dir: Path, formats: tuple[str, ...]) -> bool:
    ok = True
    vdir = out_dir / v.name
    vdir.mkdir(parents=True, exist_ok=True)
    for part in v.parts:
        for fmt in formats:
            out = vdir / f"enclosure-{v.name}-{part}.{fmt}"
            if out.exists():
                out.unlink()
            res = _run(openscad_cmd(openscad, v.scad, part, out))
            if res.returncode != 0 or not out.is_file() or out.stat().st_size == 0:
                print(f"FAIL {v.name}/{part}.{fmt} (exit {res.returncode})")
                ok = False
            else:
                print(f"ok   {out.relative_to(out_dir)} ({out.stat().st_size} bytes)")
    return ok


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--list", action="store_true", help="list variants and parts, then exit")
    ap.add_argument("--variant", action="append", help="only this variant (repeatable)")
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT, help="output directory")
    ap.add_argument("--formats", default=",".join(FORMATS), help="comma-separated: stl,3mf")
    ap.add_argument("--openscad", default=os.environ.get("OPENSCAD", "openscad"),
                    help="OpenSCAD binary (env OPENSCAD)")
    ap.add_argument("--require-version", default=os.environ.get("OPENSCAD_VERSION"),
                    help="fail unless OpenSCAD reports this version (env OPENSCAD_VERSION)")
    ap.add_argument("--skip-fit", action="store_true", help="skip the fit check")
    args = ap.parse_args(argv)

    variants = find_variants()
    if args.variant:
        wanted = set(args.variant)
        missing = wanted - {v.name for v in variants}
        if missing:
            ap.error(f"unknown variant(s): {', '.join(sorted(missing))}")
        variants = [v for v in variants if v.name in wanted]
    if not variants:
        print(f"no enclosure variants found under {ENCLOSURE_DIR}")
        return 1
    if args.list:
        for v in variants:
            print(f"{v.name}: {v.scad.relative_to(REPO_ROOT)} parts={','.join(v.parts)}")
        return 0

    formats = tuple(f.strip() for f in args.formats.split(",") if f.strip())
    bad = [f for f in formats if f not in FORMATS]
    if bad:
        ap.error(f"unsupported format(s): {', '.join(bad)}")
    if shutil.which(args.openscad) is None and not Path(args.openscad).is_file():
        print(f"OpenSCAD not found: {args.openscad}")
        return 1
    version = openscad_version(args.openscad)
    print(f"OpenSCAD {version}")
    if args.require_version and version != args.require_version:
        print(f"FAIL: need OpenSCAD {args.require_version}, found {version}")
        return 1

    ok = True
    for v in variants:
        if not args.skip_fit:
            ok &= fit_check(args.openscad, v)
        ok &= export_variant(args.openscad, v, args.out, formats)
    print("all exports ok" if ok else "enclosure export FAILED")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())

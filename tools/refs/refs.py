#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Maintain the local reference library in docs/references/.

The manifest (docs/references/manifest.json) is the tracked source of truth.
Downloaded copies go to docs/references/cache/, which is gitignored: most
datasheets and filings may not be redistributed, so they are never committed.

    python3 tools/refs/refs.py fetch            # download missing copies
    python3 tools/refs/refs.py fetch --id X     # (re)download one entry
    python3 tools/refs/refs.py fetch --force    # re-download everything
    python3 tools/refs/refs.py verify           # compare copies with recorded hashes
    python3 tools/refs/refs.py index            # regenerate docs/references/index.md
    python3 tools/refs/refs.py check            # validate manifest; index up to date
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import sys
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REF_DIR = ROOT / "docs" / "references"
MANIFEST = REF_DIR / "manifest.json"
CACHE = REF_DIR / "cache"
INDEX = REF_DIR / "index.md"

REQUIRED = ("id", "title", "publisher", "kind", "url", "file", "used_in")
KINDS = {"datasheet", "app-note", "fcc", "sdk", "license", "issue", "web", "standard"}
USER_AGENT = "Mozilla/5.0 (open-bt-rig-interface reference fetcher)"


def load(path: Path = MANIFEST) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def save(data: dict, path: Path = MANIFEST) -> None:
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def validate(data: dict) -> list[str]:
    """Return a list of problems with the manifest (empty when valid)."""
    errors = []
    seen_ids, seen_files = set(), set()
    for i, ref in enumerate(data.get("references", [])):
        where = ref.get("id", f"entry {i}")
        for key in REQUIRED:
            if not ref.get(key):
                errors.append(f"{where}: missing '{key}'")
        rid, fname = ref.get("id", ""), ref.get("file", "")
        if rid in seen_ids:
            errors.append(f"{where}: duplicate id")
        if fname in seen_files:
            errors.append(f"{where}: duplicate file name")
        seen_ids.add(rid)
        seen_files.add(fname)
        if rid and (rid != rid.lower() or " " in rid):
            errors.append(f"{where}: id must be lowercase with no spaces")
        if ref.get("kind") and ref["kind"] not in KINDS:
            errors.append(f"{where}: unknown kind '{ref['kind']}'")
        if "/" in fname or fname.startswith("."):
            errors.append(f"{where}: file must be a plain file name")
        if ref.get("fetch", "auto") not in ("auto", "manual"):
            errors.append(f"{where}: fetch must be 'auto' or 'manual'")
    return errors


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def download(url: str, dest: Path) -> None:
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    with urllib.request.urlopen(req, timeout=60) as resp:
        body = resp.read()
    if not body:
        raise OSError("empty response")
    dest.write_bytes(body)


def cmd_fetch(args) -> int:
    data = load()
    CACHE.mkdir(parents=True, exist_ok=True)
    changed, failed = False, []
    for ref in data["references"]:
        if args.id and ref["id"] not in args.id:
            continue
        dest = CACHE / ref["file"]
        if dest.exists() and not (args.force or args.id):
            continue
        if ref.get("fetch", "auto") == "manual":
            if not dest.exists():
                failed.append(ref)
                print(f"MANUAL  {ref['id']}: save {ref['url']} as {dest.relative_to(ROOT)}")
            continue
        tmp = dest.with_suffix(dest.suffix + ".part")
        try:
            download(ref["url"], tmp)
        except (urllib.error.URLError, OSError) as exc:
            failed.append(ref)
            print(f"FAILED  {ref['id']}: {exc}")
            tmp.unlink(missing_ok=True)
            continue
        digest = sha256(tmp)
        old = ref.get("sha256")
        if old and old != digest and not args.accept:
            print(f"CHANGED {ref['id']}: upstream file differs from the recorded hash "
                  f"(new revision?). Kept the old copy; rerun with --accept to take it.")
            tmp.unlink()
            continue
        tmp.replace(dest)
        if old != digest:
            ref["sha256"] = digest
            ref["retrieved"] = dt.date.today().isoformat()
            changed = True
        print(f"OK      {ref['id']}")
    if changed:
        save(data)
    return 1 if failed and args.strict else 0


def cmd_verify(_args) -> int:
    bad = 0
    for ref in load()["references"]:
        path = CACHE / ref["file"]
        if not path.exists():
            print(f"MISSING {ref['id']}")
            bad += 1
        elif ref.get("sha256") and sha256(path) != ref["sha256"]:
            print(f"DIFFERS {ref['id']}")
            bad += 1
    print("all copies match" if not bad else f"{bad} problem(s)")
    return 1 if bad else 0


def render_index(data: dict) -> str:
    # REUSE-IgnoreStart (the header below is emitted into index.md, not this file's license)
    lines = [
        "<!--",
        "SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC",
        "SPDX-License-Identifier: CC-BY-4.0",
        "-->",
        # REUSE-IgnoreEnd
        "",
        "# Reference index",
        "",
        "Generated from [`manifest.json`](manifest.json) by `tools/refs/refs.py index`. "
        "Don't edit by hand.",
        "",
        "Local copies are in `cache/` (gitignored). Run "
        "`python3 tools/refs/refs.py fetch` to download them; the **Local copy** links "
        "work once the cache is populated. See [README.md](README.md).",
        "",
    ]
    for ref in sorted(data["references"], key=lambda r: r["id"]):
        lines += [
            f"### {ref['id']}",
            "",
            f"**{ref['title']}** ({ref['publisher']}, {ref['kind']})",
            "",
            f"- Local copy: [cache/{ref['file']}](cache/{ref['file']})",
            f"- Original: <{ref['url']}>",
        ]
        if ref.get("retrieved"):
            lines.append(f"- Retrieved: {ref['retrieved']}; SHA-256 `{ref.get('sha256', '')[:16]}…`")
        if ref.get("fetch") == "manual":
            lines.append("- Download: manual (the site blocks scripted downloads)")
        if ref.get("notes"):
            lines.append(f"- Notes: {ref['notes']}")
        used = ", ".join(f"[`{p}`](../../{p})" for p in ref["used_in"])
        lines += [f"- Cited in: {used}", ""]
    return "\n".join(lines)


def cmd_index(_args) -> int:
    INDEX.write_text(render_index(load()), encoding="utf-8")
    print(f"wrote {INDEX.relative_to(ROOT)}")
    return 0


def cmd_check(_args) -> int:
    data = load()
    errors = validate(data)
    if not INDEX.exists() or INDEX.read_text(encoding="utf-8") != render_index(data):
        errors.append("index.md is out of date: run `python3 tools/refs/refs.py index`")
    for e in errors:
        print(e)
    print("ok" if not errors else f"{len(errors)} problem(s)")
    return 1 if errors else 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="cmd", required=True)
    f = sub.add_parser("fetch", help="download copies into the cache")
    f.add_argument("--id", action="append", help="only this entry (repeatable)")
    f.add_argument("--force", action="store_true", help="re-download existing copies")
    f.add_argument("--accept", action="store_true", help="accept upstream changes to recorded hashes")
    f.add_argument("--strict", action="store_true", help="exit non-zero if any download fails")
    sub.add_parser("verify", help="compare cached copies with recorded hashes")
    sub.add_parser("index", help="regenerate index.md")
    sub.add_parser("check", help="validate the manifest and index")
    args = parser.parse_args(argv)
    return {"fetch": cmd_fetch, "verify": cmd_verify, "index": cmd_index, "check": cmd_check}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())

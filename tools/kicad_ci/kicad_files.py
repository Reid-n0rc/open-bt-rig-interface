# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Shared helpers for the KiCad CI checks: KICAD_VERSION, file discovery and a
minimal read-only s-expression parser for KiCad files."""

from __future__ import annotations

import re
import subprocess
from pathlib import Path, PurePosixPath

from select_projects import is_ignored

KICAD_SUFFIXES = (".kicad_sch", ".kicad_pcb", ".kicad_sym", ".kicad_mod", ".kicad_pro")


def read_kicad_version_file(root: Path) -> dict[str, str]:
    """Parse KICAD_VERSION (KEY=value lines, '#' comments) into a dict."""
    values: dict[str, str] = {}
    for line in (root / "KICAD_VERSION").read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()
    return values


def _git_files(root: Path) -> list[str] | None:
    """Tracked plus untracked-but-not-ignored files, or None outside a git repo."""
    try:
        proc = subprocess.run(
            ["git", "-C", str(root), "ls-files", "-z", "--cached", "--others", "--exclude-standard"],
            capture_output=True, check=True)
    except (OSError, subprocess.CalledProcessError):
        return None
    return [f for f in proc.stdout.decode("utf-8").split("\0") if f]


def iter_repo_files(root: Path, suffixes: tuple[str, ...]):
    """Yield repo-relative POSIX paths ending in one of `suffixes`: the files git
    would commit (tracked, or untracked and not gitignored), or every file
    under `root` outside a git repo. Hidden directories (.git, .claude, ...)
    and KiCad backups/autosaves are skipped."""
    files = _git_files(root)
    if files is None:
        files = [p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file()]
    for rel in sorted(set(files)):
        if not rel.endswith(suffixes) or not (root / rel).is_file():
            continue
        if any(part.startswith(".") for part in PurePosixPath(rel).parts[:-1]) or is_ignored(rel):
            continue
        yield rel


_TOKEN = re.compile(r'(?P<open>\()|(?P<close>\))|"(?P<str>(?:[^"\\]|\\.)*)"|(?P<atom>[^\s()"]+)', re.S)
_ESCAPE = re.compile(r"\\(.)", re.S)


def _unescape(m: re.Match) -> str:
    return {"n": "\n", "t": "\t", "r": "\r"}.get(m.group(1), m.group(1))


class Str(str):
    """A quoted s-expression string, so "version" (atom) and "\\"version\\"" differ."""


def parse_sexpr(text: str):
    """Parse one KiCad s-expression into nested lists of str/Str atoms."""
    stack: list[list] = [[]]
    pos = 0
    for m in _TOKEN.finditer(text):
        if m.start() != pos and text[pos:m.start()].strip():
            raise ValueError(f"unexpected text at offset {pos}")
        pos = m.end()
        if m.group("open"):
            stack.append([])
        elif m.group("close"):
            if len(stack) == 1:
                raise ValueError(f"unbalanced ')' at offset {m.start()}")
            done = stack.pop()
            stack[-1].append(done)
        elif m.group("str") is not None:
            stack[-1].append(Str(_ESCAPE.sub(_unescape, m.group("str"))))
        else:
            stack[-1].append(m.group("atom"))
    if text[pos:].strip():
        raise ValueError(f"unexpected text at offset {pos} (unterminated string?)")
    if len(stack) != 1:
        raise ValueError("unbalanced '(': missing ')'")
    top = stack[0]
    if len(top) != 1 or not isinstance(top[0], list):
        raise ValueError("expected exactly one top-level (...) expression")
    return top[0]


def children(node: list, name: str) -> list[list]:
    """Direct child lists of `node` whose head is `name`."""
    return [c for c in node[1:] if isinstance(c, list) and c and c[0] == name]


def child(node: list, name: str):
    found = children(node, name)
    return found[0] if found else None


def child_value(node: list, name: str):
    """First value of the child `(name value ...)`, or None."""
    c = child(node, name)
    return c[1] if c is not None and len(c) > 1 else None


def walk(node: list):
    """Yield `node` and every nested list, depth first."""
    yield node
    for c in node[1:]:
        if isinstance(c, list):
            yield from walk(c)

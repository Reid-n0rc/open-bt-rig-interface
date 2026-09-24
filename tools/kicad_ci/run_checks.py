#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Run KiCad ERC/DRC for one planned project and fail on any violation.

Reads one entry from select_projects.py (via --project-json or $PROJECT_JSON),
runs `kicad-cli sch erc` / `pcb drc` with --exit-code-violations, writes the
JSON reports and logs to --out, and appends a Markdown summary to
$GITHUB_STEP_SUMMARY when set.

Severity: by default KiCad's defaults apply, so errors and warnings fail and
excluded items never do. KICAD_GATE_SEVERITY=error makes only errors fail.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

EXIT_VIOLATIONS = 5
MAX_LISTED = 20


def severity_args(mode: str) -> list[str]:
    if mode == "error":
        return ["--severity-error"]
    if mode in ("", "error+warning"):
        return []  # kicad-cli default: errors and warnings, exclusions not reported
    raise ValueError(f"unknown KICAD_GATE_SEVERITY {mode!r} (use 'error+warning' or 'error')")


def build_command(kicad_cli: str, check: str, target: str, report: str, mode: str) -> list[str]:
    if check == "erc":
        base = [kicad_cli, "sch", "erc"]
        extra: list[str] = []
    elif check == "drc":
        base = [kicad_cli, "pcb", "drc"]
        extra = ["--schematic-parity", "--refill-zones"]
    else:
        raise ValueError(check)
    return base + ["--exit-code-violations", "--format", "json", "-o", report] + extra + severity_args(mode) + [target]


def collect_violations(check: str, report: dict) -> list[dict]:
    if check == "erc":
        items = []
        for sheet in report.get("sheets", []):
            for v in sheet.get("violations", []):
                items.append({**v, "sheet": sheet.get("path", "")})
        return items
    return (
        list(report.get("violations", []))
        + list(report.get("unconnected_items", []))
        + list(report.get("schematic_parity", []))
    )


def run_one(kicad_cli: str, check: str, target: str, out_dir: Path, slug: str, mode: str) -> dict:
    report = out_dir / f"{slug}-{check}.json"
    log = out_dir / f"{slug}-{check}.log"
    cmd = build_command(kicad_cli, check, target, str(report), mode)
    proc = subprocess.run(cmd, capture_output=True, text=True)
    log.write_text(f"$ {' '.join(cmd)}\n{proc.stdout}{proc.stderr}")

    result = {"check": check, "target": target, "returncode": proc.returncode,
              "report": report.name, "violations": [], "errors": None, "warnings": None}
    if report.is_file():
        try:
            data = json.loads(report.read_text())
            items = collect_violations(check, data)
            result["violations"] = items
            result["errors"] = sum(1 for v in items if v.get("severity") == "error")
            result["warnings"] = sum(1 for v in items if v.get("severity") == "warning")
        except (json.JSONDecodeError, OSError) as exc:
            result["parse_error"] = str(exc)

    if proc.returncode == 0:
        result["status"] = "pass"
    elif proc.returncode == EXIT_VIOLATIONS:
        result["status"] = "violations"
    else:
        result["status"] = "error"
    return result


def summary_markdown(project: dict, results: list[dict]) -> str:
    lines = [f"### KiCad checks: `{project['name']}`", "",
             "| Check | File | Result | Errors | Warnings | Report |",
             "|---|---|---|---|---|---|"]
    icons = {"pass": "✅ pass", "violations": "❌ violations", "error": "❌ could not run"}
    for r in results:
        errs = "–" if r["errors"] is None else r["errors"]
        warns = "–" if r["warnings"] is None else r["warnings"]
        lines.append(f"| {r['check'].upper()} | `{r['target']}` | {icons[r['status']]} | {errs} | {warns} | `{r['report']}` |")
    for r in results:
        if r["status"] == "error":
            lines += ["", f"**{r['check'].upper()} could not run** (kicad-cli exit {r['returncode']}); see the `.log` artifact."]
        if r["violations"] and r["status"] != "pass":
            lines += ["", f"<details><summary>{r['check'].upper()} violations ({len(r['violations'])})</summary>", ""]
            for v in r["violations"][:MAX_LISTED]:
                lines.append(f"- **{v.get('severity', '?')}** `{v.get('type', '?')}`: {v.get('description', '')}")
            if len(r["violations"]) > MAX_LISTED:
                lines.append(f"- …and {len(r['violations']) - MAX_LISTED} more (see the JSON report)")
            lines += ["", "</details>"]
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--project-json", default=os.environ.get("PROJECT_JSON"),
                    help="one plan entry from select_projects.py (default: $PROJECT_JSON)")
    ap.add_argument("--kicad-cli", default=os.environ.get("KICAD_CLI", "kicad-cli"))
    ap.add_argument("--out", default="kicad-reports", help="directory for reports and logs")
    ap.add_argument("--summary", default=os.environ.get("GITHUB_STEP_SUMMARY"),
                    help="append a Markdown summary to this file")
    args = ap.parse_args(argv)
    if not args.project_json:
        ap.error("--project-json or $PROJECT_JSON is required")

    project = json.loads(args.project_json)
    mode = os.environ.get("KICAD_GATE_SEVERITY", "error+warning").strip()
    severity_args(mode)  # fail fast on a bad setting
    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    results = []
    if project.get("erc"):
        results.append(run_one(args.kicad_cli, "erc", project["schematic"], out_dir, project["slug"], mode))
    if project.get("drc"):
        results.append(run_one(args.kicad_cli, "drc", project["pcb"], out_dir, project["slug"], mode))

    md = summary_markdown(project, results)
    print(md)
    if args.summary:
        with open(args.summary, "a", encoding="utf-8") as fh:
            fh.write(md)
    return 0 if results and all(r["status"] == "pass" for r in results) else 1


if __name__ == "__main__":
    sys.exit(main())

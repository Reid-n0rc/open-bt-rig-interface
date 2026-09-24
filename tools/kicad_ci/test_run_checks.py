# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for run_checks.py using a stub kicad-cli (no KiCad install needed)."""

import contextlib
import io
import json
import os
import stat
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_checks as rc  # noqa: E402

# The stub mimics kicad-cli 10.0.6: it writes the JSON report named by -o and
# exits 0 (clean), 5 (violations) or 3 (load failure) per $STUB_<CHECK>.
STUB = textwrap.dedent("""\
    #!/usr/bin/env python3
    import json, os, sys
    args = sys.argv[1:]
    check = args[1]
    mode = os.environ.get("STUB_" + check.upper(), "pass")
    with open(os.environ["STUB_LOG"], "a") as log:
        log.write(" ".join(args) + "\\n")
    if mode == "load_error":
        print("Failed to load file", file=sys.stderr)
        sys.exit(3)
    out = args[args.index("-o") + 1]
    bad = [{"severity": "error", "type": "pin_not_connected", "description": "Pin not connected"},
           {"severity": "warning", "type": "lib_symbol_mismatch", "description": "Symbol mismatch"}]
    items = bad if mode == "violations" else []
    if check == "erc":
        report = {"sheets": [{"path": "/", "violations": items}]}
    else:
        report = {"violations": items[:1], "unconnected_items": [], "schematic_parity": items[1:]}
    with open(out, "w") as fh:
        json.dump(report, fh)
    sys.exit(5 if items else 0)
    """)

PROJECT = {"name": "hardware/boards/R/revA", "slug": "hardware__boards__R__revA__board",
           "project": "hardware/boards/R/revA/board.kicad_pro",
           "schematic": "hardware/boards/R/revA/board.kicad_sch",
           "pcb": "hardware/boards/R/revA/board.kicad_pcb", "erc": True, "drc": True}


class RunChecksTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = Path(self._tmp.name)
        self.stub = self.tmp / "kicad-cli"
        self.stub.write_text(STUB)
        self.stub.chmod(self.stub.stat().st_mode | stat.S_IEXEC)
        self.log = self.tmp / "calls.log"
        self.summary = self.tmp / "summary.md"
        self.out = self.tmp / "reports"

    def tearDown(self):
        self._tmp.cleanup()

    def run_main(self, project=PROJECT, **env):
        with mock.patch.dict(os.environ, {"STUB_LOG": str(self.log), **env}), \
                contextlib.redirect_stdout(io.StringIO()):
            if "KICAD_GATE_SEVERITY" not in env:
                os.environ.pop("KICAD_GATE_SEVERITY", None)
            return rc.main(["--project-json", json.dumps(project), "--kicad-cli", str(self.stub),
                            "--out", str(self.out), "--summary", str(self.summary)])

    def calls(self):
        return self.log.read_text().splitlines()

    def test_clean_project_passes_and_runs_both_checks(self):
        self.assertEqual(self.run_main(), 0)
        calls = self.calls()
        self.assertEqual(len(calls), 2)
        self.assertTrue(calls[0].startswith("sch erc --exit-code-violations --format json -o"))
        self.assertIn("pcb drc", calls[1])
        self.assertIn("--schematic-parity", calls[1])
        self.assertIn("--refill-zones", calls[1])
        self.assertTrue(calls[0].endswith(PROJECT["schematic"]))
        self.assertTrue(calls[1].endswith(PROJECT["pcb"]))
        self.assertIn("✅ pass", self.summary.read_text())
        self.assertTrue((self.out / f"{PROJECT['slug']}-erc.json").is_file())
        self.assertTrue((self.out / f"{PROJECT['slug']}-drc.log").is_file())

    def test_erc_violations_fail_with_counts(self):
        self.assertEqual(self.run_main(STUB_ERC="violations"), 1)
        text = self.summary.read_text()
        self.assertIn("| ERC | `hardware/boards/R/revA/board.kicad_sch` | ❌ violations | 1 | 1 |", text)
        self.assertIn("`pin_not_connected`", text)

    def test_drc_violations_include_parity_items(self):
        self.assertEqual(self.run_main(STUB_DRC="violations"), 1)
        self.assertIn("| DRC | `hardware/boards/R/revA/board.kicad_pcb` | ❌ violations | 1 | 1 |",
                      self.summary.read_text())

    def test_load_error_fails(self):
        self.assertEqual(self.run_main(STUB_DRC="load_error"), 1)
        text = self.summary.read_text()
        self.assertIn("❌ could not run", text)
        self.assertIn("kicad-cli exit 3", text)

    def test_only_requested_checks_run(self):
        self.assertEqual(self.run_main(project={**PROJECT, "drc": False}), 0)
        self.assertEqual(len(self.calls()), 1)
        self.assertIn("sch erc", self.calls()[0])

    def test_error_only_severity_mode(self):
        self.assertEqual(self.run_main(KICAD_GATE_SEVERITY="error"), 0)
        self.assertTrue(all("--severity-error" in c for c in self.calls()))

    def test_default_mode_passes_no_severity_flags(self):
        self.run_main()
        self.assertTrue(all("--severity" not in c for c in self.calls()))

    def test_bad_severity_mode_rejected(self):
        with self.assertRaises(ValueError):
            self.run_main(KICAD_GATE_SEVERITY="warnings-only")

    def test_nothing_to_run_fails(self):
        self.assertEqual(self.run_main(project={**PROJECT, "erc": False, "drc": False}), 1)

    def test_long_violation_lists_are_truncated(self):
        many = {"sheets": [{"path": "/", "violations": [
            {"severity": "error", "type": f"t{i}", "description": "d"} for i in range(rc.MAX_LISTED + 5)]}]}
        results = [{"check": "erc", "target": "x.kicad_sch", "returncode": 5, "report": "r.json",
                    "violations": rc.collect_violations("erc", many), "errors": 25, "warnings": 0,
                    "status": "violations"}]
        md = rc.summary_markdown(PROJECT, results)
        self.assertIn("…and 5 more", md)

    def test_unknown_check_rejected(self):
        with self.assertRaises(ValueError):
            rc.build_command("kicad-cli", "lvs", "x", "r.json", "")


if __name__ == "__main__":
    unittest.main()

# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for gate.py (run: python -m unittest discover -s tools/ci)."""

import contextlib
import io
import json
import sys
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gate  # noqa: E402


def needs(**results):
    return {job.replace("_", "-"): {"result": r, "outputs": {}} for job, r in results.items()}


class FailuresTests(unittest.TestCase):
    def test_filtered_run_accepts_skipped_areas(self):
        n = needs(changes="success", reuse="success", firmware_host_tests="skipped",
                  kicad_release="skipped")
        self.assertEqual(gate.failures(n, False, {"kicad-release"}, False), {})

    def test_failure_and_cancellation_fail(self):
        n = needs(changes="success", reuse="failure", protocol="cancelled")
        self.assertEqual(gate.failures(n, False, set(), False),
                         {"reuse": "failure", "protocol": "cancelled"})

    def test_selection_must_succeed(self):
        # If `changes` fails, everything after it is skipped: still a failure.
        n = needs(changes="failure", firmware_host_tests="skipped")
        self.assertEqual(gate.failures(n, False, set(), False), {"changes": "failure"})
        n = needs(changes="skipped")
        self.assertEqual(gate.failures(n, False, set(), False), {"changes": "skipped"})

    def test_selection_missing_from_needs(self):
        self.assertEqual(gate.failures(needs(reuse="success"), False, set(), False),
                         {"changes": "missing"})

    def test_full_regression_skips_nothing(self):
        n = needs(changes="success", firmware_host_tests="skipped")
        self.assertEqual(gate.failures(n, True, set(), False),
                         {"firmware-host-tests": "skipped"})

    def test_full_regression_off_a_tag_skips_the_tag_only_jobs(self):
        n = needs(changes="success", kicad_release="skipped")
        self.assertEqual(gate.failures(n, True, {"kicad-release"}, False), {})

    def test_full_regression_on_a_tag_runs_the_tag_only_jobs(self):
        n = needs(changes="success", kicad_release="skipped")
        self.assertEqual(gate.failures(n, True, {"kicad-release"}, True),
                         {"kicad-release": "skipped"})


class MainTests(unittest.TestCase):
    def run_main(self, n, *args):
        out = io.StringIO()
        with mock.patch.dict("os.environ", {"GITHUB_STEP_SUMMARY": ""}), \
                mock.patch("sys.stdin", io.StringIO(json.dumps(n))), \
                contextlib.redirect_stdout(out):
            rc = gate.main(list(args))
        return rc, out.getvalue()

    def test_pass(self):
        rc, out = self.run_main(needs(changes="success", reuse="success"), "--full", "false")
        self.assertEqual(rc, 0)
        self.assertIn("Checks for the changed areas: passed.", out)

    def test_fail_reports_the_jobs(self):
        rc, out = self.run_main(needs(changes="success", protocol="skipped"),
                                "--full", "true")
        self.assertEqual(rc, 1)
        self.assertIn("Full regression: not passed: protocol.", out)
        self.assertIn("::error::", out)

    def test_changes_did_not_run(self):
        rc, _ = self.run_main(needs(changes="failure"), "--full", "")
        self.assertEqual(rc, 1)


if __name__ == "__main__":
    unittest.main()

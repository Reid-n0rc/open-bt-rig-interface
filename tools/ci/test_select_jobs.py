# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for select_jobs.py (run: python -m unittest discover -s tools/ci)."""

import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import select_jobs as sj  # noqa: E402


class IsFullTests(unittest.TestCase):
    def test_pr_to_dev_is_filtered(self):
        self.assertFalse(sj.is_full("pull_request", "dev"))

    def test_pr_to_main_is_a_release(self):
        self.assertTrue(sj.is_full("pull_request", "main"))

    def test_push_tag_and_manual_runs_are_full(self):
        for event in ("push", "workflow_dispatch", "workflow_call"):
            self.assertTrue(sj.is_full(event, ""), event)


class SelectTests(unittest.TestCase):
    def test_docs_only_checks_no_area(self):
        self.assertEqual(sj.select(["docs/x.md", "README.md", "hardware/README.md"]),
                         {"full": False, "firmware": False, "protocol": False})

    def test_firmware_change(self):
        self.assertEqual(sj.select(["firmware/app/src/ptt.c"]),
                         {"full": False, "firmware": True, "protocol": False})

    def test_protocol_change_feeds_the_firmware_tests(self):
        for path in ("protocol/vectors/caps.json", "tools/protocol/proto_codec.py"):
            self.assertEqual(sj.select([path]),
                             {"full": False, "firmware": True, "protocol": True}, path)

    def test_cat_loopback_is_protocol_only(self):
        self.assertEqual(sj.select(["tools/cat_loopback/cat_loopback.py"]),
                         {"full": False, "firmware": False, "protocol": True})

    def test_ci_changes_are_a_full_regression(self):
        for path in (".github/workflows/checks.yml", "tools/ci/select_jobs.py"):
            self.assertEqual(sj.select(["docs/x.md", path]),
                             {"full": True, "firmware": True, "protocol": True}, path)

    def test_other_workflows_and_kicad_ci_are_not_global_here(self):
        # kicad-checks.yml and tools/kicad_ci/ select their own ERC/DRC work.
        self.assertEqual(sj.select([".github/workflows/kicad-checks.yml",
                                    "tools/kicad_ci/run_checks.py"]),
                         {"full": False, "firmware": False, "protocol": False})

    def test_prefix_needs_the_directory(self):
        self.assertFalse(sj.select(["firmware.md"])["firmware"])

    def test_blank_lines_and_whitespace(self):
        self.assertTrue(sj.select(["", "  firmware/test/test_ptt.c  "])["firmware"])

    def test_full_turns_every_area_on(self):
        self.assertEqual(sj.select([], full=True),
                         {"full": True, "firmware": True, "protocol": True})


class MainTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def run_main(self, *args):
        out = self.dir / "out"
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(sj.main([*args, "--github-output", str(out)]), 0)
        return dict(line.split("=", 1) for line in out.read_text().splitlines())

    def test_pr_to_dev_reads_the_changed_files(self):
        changed = self.dir / "changed.txt"
        changed.write_text("firmware/app/src/cat.c\n")
        self.assertEqual(
            self.run_main("--event", "pull_request", "--base-ref", "dev",
                          "--changed-files", str(changed)),
            {"full": "false", "firmware": "true", "protocol": "false"})

    def test_push_needs_no_changed_files(self):
        self.assertEqual(self.run_main("--event", "push"),
                         {"full": "true", "firmware": "true", "protocol": "true"})

    def test_pr_to_main_ignores_the_changed_files(self):
        self.assertEqual(self.run_main("--event", "pull_request", "--base-ref", "main"),
                         {"full": "true", "firmware": "true", "protocol": "true"})

    def test_filtered_pr_without_changed_files_is_an_error(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            sj.main(["--event", "pull_request", "--base-ref", "dev"])


if __name__ == "__main__":
    unittest.main()

# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for select_projects.py (run: python -m unittest discover -s tools/kicad_ci)."""

import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import select_projects as sp  # noqa: E402


def make_tree(root: Path, files: list[str]) -> None:
    for f in files:
        p = root / f
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text("")


BOARD = "hardware/boards/R/revA"
FULL_PROJECT = [f"{BOARD}/board.kicad_pro", f"{BOARD}/board.kicad_sch", f"{BOARD}/board.kicad_pcb",
                f"{BOARD}/power.kicad_sch"]


class SelectTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def plan(self, changed, run_all=False):
        return sp.select(changed, sp.find_projects(self.root), self.root, run_all=run_all)

    def test_no_projects_means_empty_plan(self):
        self.assertEqual(self.plan(["README.md", "hardware/boards/README.md"]), [])
        self.assertEqual(self.plan([], run_all=True), [])

    def test_schematic_change_runs_erc_on_root_schematic_only(self):
        make_tree(self.root, FULL_PROJECT)
        plan = self.plan([f"{BOARD}/power.kicad_sch"])
        self.assertEqual(len(plan), 1)
        entry = plan[0]
        self.assertTrue(entry["erc"])
        self.assertFalse(entry["drc"])
        self.assertEqual(entry["schematic"], f"{BOARD}/board.kicad_sch")
        self.assertEqual(entry["name"], BOARD)

    def test_pcb_change_runs_drc_only(self):
        make_tree(self.root, FULL_PROJECT)
        entry = self.plan([f"{BOARD}/board.kicad_pcb"])[0]
        self.assertEqual((entry["erc"], entry["drc"]), (False, True))
        self.assertEqual(entry["pcb"], f"{BOARD}/board.kicad_pcb")

    def test_project_wide_files_run_both(self):
        make_tree(self.root, FULL_PROJECT + [f"{BOARD}/board.kicad_dru", f"{BOARD}/fp-lib-table"])
        for changed in (f"{BOARD}/board.kicad_pro", f"{BOARD}/board.kicad_dru",
                        f"{BOARD}/fp-lib-table", f"{BOARD}/sym-lib-table"):
            entry = self.plan([changed])[0]
            self.assertEqual((entry["erc"], entry["drc"]), (True, True), changed)

    def test_library_ci_and_version_changes_check_everything(self):
        other = "hardware/boards/M/revA"
        make_tree(self.root, FULL_PROJECT + [f"{other}/m.kicad_pro", f"{other}/m.kicad_sch", f"{other}/m.kicad_pcb"])
        for changed in ("hardware/lib/symbols/x.kicad_sym", "tools/kicad_ci/run_checks.py",
                        ".github/workflows/kicad-checks.yml", "KICAD_VERSION"):
            plan = self.plan([changed])
            self.assertEqual(len(plan), 2, changed)
            self.assertTrue(all(e["erc"] and e["drc"] for e in plan), changed)

    def test_run_all(self):
        make_tree(self.root, FULL_PROJECT)
        entry = self.plan([], run_all=True)[0]
        self.assertEqual((entry["erc"], entry["drc"]), (True, True))

    def test_unrelated_and_ignored_files_are_skipped(self):
        make_tree(self.root, FULL_PROJECT)
        for changed in ("README.md", f"{BOARD}/README.md", f"{BOARD}/board.kicad_prl",
                        f"{BOARD}/fp-info-cache", f"{BOARD}/_autosave-board.kicad_sch",
                        f"{BOARD}/board-backups/board.kicad_sch", f"{BOARD}/.history/board.kicad_pcb",
                        f"{BOARD}/~board.kicad_sch.lck"):
            self.assertEqual(self.plan([changed]), [], changed)

    def test_backup_projects_are_not_discovered(self):
        make_tree(self.root, FULL_PROJECT + [f"{BOARD}/board-backups/old/board.kicad_pro"])
        self.assertEqual(sp.find_projects(self.root), [f"{BOARD}/board.kicad_pro"])

    def test_missing_pcb_skips_drc_and_missing_files_skip_project(self):
        make_tree(self.root, [f"{BOARD}/board.kicad_pro", f"{BOARD}/board.kicad_sch"])
        entry = self.plan([f"{BOARD}/board.kicad_pro"])[0]
        self.assertEqual((entry["erc"], entry["drc"]), (True, False))
        self.assertEqual(self.plan([f"{BOARD}/board.kicad_pcb"]), [])

    def test_deleted_project_file_is_ignored(self):
        make_tree(self.root, FULL_PROJECT)
        self.assertEqual(self.plan(["hardware/boards/gone/revA/gone.kicad_sch"]), [])

    def test_nested_file_belongs_to_nearest_project(self):
        make_tree(self.root, FULL_PROJECT + [f"{BOARD}/sheets/usb.kicad_sch"])
        entry = self.plan([f"{BOARD}/sheets/usb.kicad_sch"])[0]
        self.assertEqual(entry["project"], f"{BOARD}/board.kicad_pro")
        self.assertTrue(entry["erc"])

    def test_two_projects_in_one_dir_get_distinct_names(self):
        make_tree(self.root, [f"{BOARD}/a.kicad_pro", f"{BOARD}/a.kicad_sch",
                              f"{BOARD}/b.kicad_pro", f"{BOARD}/b.kicad_sch"])
        plan = self.plan([f"{BOARD}/shared.kicad_sch"])
        self.assertEqual(sorted(e["name"] for e in plan), [f"{BOARD}/a", f"{BOARD}/b"])
        self.assertEqual(len({e["slug"] for e in plan}), 2)


class CliTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.out = self.root / "out.txt"

    def tearDown(self):
        self._tmp.cleanup()

    def outputs(self):
        return dict(line.split("=", 1) for line in self.out.read_text().splitlines())

    def test_image_reads_kicad_version(self):
        (self.root / "KICAD_VERSION").write_text("# comment\nKICAD_MIN_VERSION=10.0.6\nKICAD_GENERATOR_VERSION=10.0\n")
        with contextlib.redirect_stdout(io.StringIO()):
            sp.main(["--root", str(self.root), "--github-output", str(self.out), "image"])
        self.assertEqual(self.outputs()["image"], "kicad/kicad:10.0.6")

    def test_image_without_version_fails(self):
        (self.root / "KICAD_VERSION").write_text("KICAD_GENERATOR_VERSION=10.0\n")
        with self.assertRaises(ValueError):
            sp.main(["--root", str(self.root), "image"])

    def test_select_outputs_any_and_matrix(self):
        make_tree(self.root, FULL_PROJECT)
        changed = self.root / "changed.txt"
        changed.write_text(f"{BOARD}/board.kicad_pcb\nREADME.md\n")
        with contextlib.redirect_stdout(io.StringIO()):
            sp.main(["--root", str(self.root), "--github-output", str(self.out), "select", "--changed-files", str(changed)])
        out = self.outputs()
        self.assertEqual(out["any"], "true")
        self.assertEqual(json.loads(out["matrix"])[0]["drc"], True)

    def test_select_nothing(self):
        changed = self.root / "changed.txt"
        changed.write_text("README.md\n")
        with contextlib.redirect_stdout(io.StringIO()):
            sp.main(["--root", str(self.root), "--github-output", str(self.out), "select", "--changed-files", str(changed)])
        self.assertEqual(self.outputs(), {"any": "false", "matrix": "[]"})

    def test_select_requires_input(self):
        with self.assertRaises(SystemExit):
            sp.main(["--root", str(self.root), "select"])


if __name__ == "__main__":
    unittest.main()

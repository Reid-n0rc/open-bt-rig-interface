# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for kicad_files.py (run: python -m unittest discover -s tools/kicad_ci)."""

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import kicad_files as kf  # noqa: E402


class ParseTests(unittest.TestCase):
    def test_nested_lists_atoms_and_strings(self):
        tree = kf.parse_sexpr('(kicad_pcb (version 20260206) (generator "pcbnew")\n\t(gr_text "a b" (at 1 2)))')
        self.assertEqual(tree[0], "kicad_pcb")
        self.assertEqual(kf.child_value(tree, "version"), "20260206")
        self.assertEqual(kf.child_value(tree, "generator"), "pcbnew")
        self.assertEqual(kf.child(tree, "gr_text")[1], "a b")
        self.assertIsNone(kf.child_value(tree, "missing"))

    def test_quoted_strings_are_marked_and_unescaped(self):
        tree = kf.parse_sexpr(r'(x hide "hide" "say \"hi\"\nnext" "back\\slash")')
        self.assertNotIsInstance(tree[1], kf.Str)
        self.assertIsInstance(tree[2], kf.Str)
        self.assertEqual(tree[3], 'say "hi"\nnext')
        self.assertEqual(tree[4], "back\\slash")

    def test_parentheses_inside_strings(self):
        self.assertEqual(kf.parse_sexpr('(a "(b)")'), ["a", "(b)"])

    def test_malformed_input_raises(self):
        for bad in ("(a (b)", "(a))", '(a "open)', "", "(a) (b)", "atom"):
            with self.assertRaises(ValueError, msg=bad):
                kf.parse_sexpr(bad)

    def test_walk_visits_every_list(self):
        heads = [n[0] for n in kf.walk(kf.parse_sexpr("(a (b (c)) (d))"))]
        self.assertEqual(heads, ["a", "b", "c", "d"])


class RepoTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def test_read_kicad_version_file(self):
        (self.root / "KICAD_VERSION").write_text("# comment\n\nKICAD_MIN_VERSION = 10.0.6\nKICAD_SCH_FORMAT=20260306\n")
        self.assertEqual(kf.read_kicad_version_file(self.root),
                         {"KICAD_MIN_VERSION": "10.0.6", "KICAD_SCH_FORMAT": "20260306"})

    def test_iter_repo_files_skips_hidden_backups_and_autosaves(self):
        for f in ("hardware/boards/R/revA/b.kicad_pcb", "hardware/lib/footprints/x.pretty/y.kicad_mod",
                  ".claude/worktrees/w/hardware/b.kicad_pcb", "hardware/boards/R/revA/b-backups/b.kicad_pcb",
                  "hardware/boards/R/revA/_autosave-b.kicad_pcb", "hardware/boards/R/revA/b.kicad_prl",
                  "README.md"):
            p = self.root / f
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text("")
        self.assertEqual(list(kf.iter_repo_files(self.root, (".kicad_pcb", ".kicad_mod"))),
                         ["hardware/boards/R/revA/b.kicad_pcb", "hardware/lib/footprints/x.pretty/y.kicad_mod"])


if __name__ == "__main__":
    unittest.main()

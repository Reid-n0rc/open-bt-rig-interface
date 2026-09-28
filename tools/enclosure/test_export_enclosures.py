# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Self-tests for export_enclosures.py (no OpenSCAD needed)."""

import tempfile
import unittest
from pathlib import Path

import export_enclosures as ee


class FindVariantsTest(unittest.TestCase):
    def test_repo_has_both_variants(self):
        variants = {v.name: v for v in ee.find_variants()}
        self.assertEqual(sorted(variants), ["M", "R"])
        for v in variants.values():
            self.assertEqual(v.parts, ("base", "lid"))
            self.assertTrue(v.scad.is_file())

    def test_ignores_dirs_without_a_variant_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "common").mkdir()
            (root / "common" / "enclosure.scad").write_text("module x() {}\n")
            (root / "X").mkdir()
            (root / "X" / "enclosure_X.scad").write_text('export_parts = ["base"];\n')
            self.assertEqual([v.name for v in ee.find_variants(root)], ["X"])

    def test_variant_without_parts_is_an_error(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "Y").mkdir()
            (root / "Y" / "enclosure_Y.scad").write_text("part = \"base\";\n")
            with self.assertRaises(ValueError):
                ee.find_variants(root)


class ParsePartsTest(unittest.TestCase):
    def test_parse(self):
        text = 'variant = "R";\nexport_parts = ["base", "lid"];\npart = "assembly";\n'
        self.assertEqual(ee.parse_parts(text), ("base", "lid"))

    def test_missing(self):
        self.assertEqual(ee.parse_parts('part = "base";\n'), ())


class CommandTest(unittest.TestCase):
    def test_stl_is_binary_and_warnings_fail(self):
        cmd = ee.openscad_cmd("openscad", Path("a.scad"), "lid", Path("o/x.stl"))
        self.assertIn("--hardwarnings", cmd)
        self.assertEqual(cmd[cmd.index("-D") + 1], 'part="lid"')
        self.assertEqual(cmd[cmd.index("--export-format") + 1], "binstl")
        self.assertEqual(cmd[-3:], ["-o", str(Path("o/x.stl")), "a.scad"])

    def test_3mf_uses_extension(self):
        cmd = ee.openscad_cmd("openscad", Path("a.scad"), "base", Path("x.3mf"))
        self.assertNotIn("--export-format", cmd)


class ListTest(unittest.TestCase):
    def test_list_needs_no_openscad(self):
        self.assertEqual(ee.main(["--list", "--openscad", "/nonexistent/openscad"]), 0)

    def test_unknown_variant(self):
        with self.assertRaises(SystemExit):
            ee.main(["--list", "--variant", "Z"])


if __name__ == "__main__":
    unittest.main()

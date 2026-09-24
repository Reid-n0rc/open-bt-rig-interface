# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for check_kicad_version.py (run: python -m unittest discover -s tools/kicad_ci).

TEST FIXTURES: the KiCad snippets below are minimal hand-written s-expression
and JSON headers, modelled on files saved by KiCad 10.0.6. They are not real
design files and are only written to temporary directories at test time, so no
.kicad_* file is committed.
"""

import contextlib
import io
import json
import sys
import tempfile
import unittest
import unittest.mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_kicad_version as ckv  # noqa: E402

KICAD_VERSION = """# test fixture
KICAD_MIN_VERSION=10.0.6
KICAD_GENERATOR_VERSION=10.0
KICAD_SCH_FORMAT=20260306
KICAD_PCB_FORMAT=20260206
KICAD_SYM_FORMAT=20251024
KICAD_PRO_META_VERSION=3
"""

# --- test fixtures (good: KiCad 10.0.6 tokens) ---
SCH = '(kicad_sch\n\t(version 20260306)\n\t(generator "eeschema")\n\t(generator_version "10.0")\n\t(paper "A4")\n)\n'
PCB = '(kicad_pcb\n\t(version 20260206)\n\t(generator "pcbnew")\n\t(generator_version "10.0")\n)\n'
SYM = '(kicad_symbol_lib\n\t(version 20251024)\n\t(generator "kicad_symbol_editor")\n\t(generator_version "10.0")\n)\n'
MOD = '(footprint "X"\n\t(version 20260206)\n\t(generator "pcbnew")\n\t(generator_version "10.0")\n\t(layer "F.Cu")\n)\n'
PRO = json.dumps({"meta": {"filename": "board.kicad_pro", "version": 3}})
# --- test fixtures (bad) ---
SCH_KICAD9 = '(kicad_sch (version 20250114) (generator "eeschema") (generator_version "9.0"))'
PCB_NEWER = '(kicad_pcb (version 20270101) (generator "pcbnew") (generator_version "11.0"))'
SYM_NO_GENERATOR_VERSION = '(kicad_symbol_lib (version 20251024) (generator "kicad_symbol_editor"))'
MOD_SCH_FORMAT = '(footprint "X" (version 20260306) (generator "pcbnew") (generator_version "10.0"))'
PRO_OLD = json.dumps({"meta": {"filename": "board.kicad_pro", "version": 1}})

AGENTS_OK = "- **KiCad ≥ 10.0.6.** `KICAD_VERSION` is authoritative.\n"
BOARD = "hardware/boards/R/revA"


class VersionCheckTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        self.write("KICAD_VERSION", KICAD_VERSION)
        self.write("AGENTS.md", AGENTS_OK)

    def tearDown(self):
        self._tmp.cleanup()

    def write(self, rel, text):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8")

    def errors(self):
        return ckv.run(self.root)[0]

    def assertOneError(self, *fragments):
        errors = self.errors()
        self.assertEqual(len(errors), 1, errors)
        for f in fragments:
            self.assertIn(f, errors[0])

    def test_no_kicad_files_passes(self):
        errors, summary = ckv.run(self.root)
        self.assertEqual(errors, [])
        self.assertIn("no KiCad files yet", summary)

    def test_good_files_pass(self):
        self.write(f"{BOARD}/board.kicad_sch", SCH)
        self.write(f"{BOARD}/board.kicad_pcb", PCB)
        self.write(f"{BOARD}/board.kicad_pro", PRO)
        self.write("hardware/lib/symbols/project.kicad_sym", SYM)
        self.write("hardware/lib/footprints/project.pretty/X.kicad_mod", MOD)
        errors, summary = ckv.run(self.root)
        self.assertEqual(errors, [])
        self.assertIn("5 KiCad file(s)", summary)

    def test_file_saved_by_older_kicad_fails(self):
        self.write(f"{BOARD}/power.kicad_sch", SCH_KICAD9)
        errors = self.errors()
        self.assertEqual(len(errors), 2, errors)
        self.assertIn("power.kicad_sch: format (version 20250114)", errors[0])
        self.assertIn("generator_version '9.0'", errors[1])

    def test_file_saved_by_newer_kicad_fails(self):
        self.write(f"{BOARD}/board.kicad_pcb", PCB_NEWER)
        self.assertIn("bump KICAD_VERSION", self.errors()[0])

    def test_missing_generator_version_fails(self):
        self.write("hardware/lib/symbols/project.kicad_sym", SYM_NO_GENERATOR_VERSION)
        self.assertOneError("project.kicad_sym", "generator_version None")

    def test_footprint_uses_pcb_format(self):
        self.write("hardware/lib/footprints/project.pretty/X.kicad_mod", MOD_SCH_FORMAT)
        self.assertOneError("X.kicad_mod", "KICAD_PCB_FORMAT=20260206")

    def test_old_project_file_fails(self):
        self.write(f"{BOARD}/board.kicad_pro", PRO_OLD)
        self.assertOneError("board.kicad_pro", "meta.version 1")

    def test_unreadable_files_fail(self):
        self.write(f"{BOARD}/a.kicad_sch", "(kicad_sch (version 20260306)")
        self.write(f"{BOARD}/b.kicad_pro", "{not json")
        self.write(f"{BOARD}/c.kicad_sch", PCB)
        errors = self.errors()
        self.assertEqual(len(errors), 3, errors)
        self.assertIn("a.kicad_sch: not a readable KiCad file", errors[0])
        self.assertIn("b.kicad_pro: not a readable KiCad project file", errors[1])
        self.assertIn("c.kicad_sch: expected a (kicad_sch ...) file", errors[2])

    def test_hidden_dirs_and_backups_are_ignored(self):
        self.write(".claude/worktrees/x/board.kicad_sch", SCH_KICAD9)
        self.write(f"{BOARD}/board-backups/board.kicad_sch", SCH_KICAD9)
        self.assertEqual(self.errors(), [])

    def test_kicad_version_file_problems(self):
        self.write("KICAD_VERSION", KICAD_VERSION.replace("KICAD_SYM_FORMAT=20251024\n", ""))
        self.assertOneError("KICAD_SYM_FORMAT is missing")
        self.write("KICAD_VERSION", KICAD_VERSION.replace("KICAD_GENERATOR_VERSION=10.0", "KICAD_GENERATOR_VERSION=9.0"))
        self.assertOneError("does not match KICAD_MIN_VERSION")
        self.write("KICAD_VERSION", KICAD_VERSION.replace("KICAD_MIN_VERSION=10.0.6", "KICAD_MIN_VERSION=10.0"))
        self.assertOneError("is not X.Y.Z")
        (self.root / "KICAD_VERSION").unlink()
        self.assertOneError("KICAD_VERSION: cannot read")

    def test_docs_quoting_another_version_fail(self):
        self.write("README.md", "Needs KiCad >= 10.0.5.\n")
        self.write("CONTRIBUTING.md", "Use KiCad (10.0.6 or newer).\n")
        self.write("docs/developer-guide.md",
                   "| [KiCad](https://www.kicad.org/download/) **≥ 9.0.1** | x |\nRuns `kicad/kicad:10.0.4`.\n")
        errors = self.errors()
        self.assertEqual(len(errors), 3, errors)
        self.assertIn("README.md:1: quotes KiCad 10.0.5", errors[0])
        self.assertIn("docs/developer-guide.md:1: quotes KiCad 9.0.1", errors[1])
        self.assertIn("docs/developer-guide.md:2: quotes KiCad 10.0.4", errors[2])

    def test_docs_without_a_version_number_and_changelog_are_ignored(self):
        self.write("hardware/boards/README.md", "with KiCad >= the version in `KICAD_VERSION`.\n")
        self.write("CHANGELOG.md", "- Moved to KiCad ≥ 9.0.1 (#1).\n")
        self.assertEqual(self.errors(), [])

    def test_agents_md_must_quote_the_version(self):
        self.write("AGENTS.md", "KiCad version: see KICAD_VERSION.\n")
        self.assertOneError("AGENTS.md: does not quote the minimum KiCad version")

    @unittest.mock.patch.dict("os.environ", {"GITHUB_ACTIONS": ""})
    def test_cli_exit_codes(self):
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(ckv.main(["--root", str(self.root)]), 0)
        self.assertIn("passed", out.getvalue())
        self.write(f"{BOARD}/board.kicad_pcb", PCB_NEWER)
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(ckv.main(["--root", str(self.root)]), 1)
        self.assertIn("error: hardware/boards/R/revA/board.kicad_pcb", out.getvalue())


if __name__ == "__main__":
    unittest.main()

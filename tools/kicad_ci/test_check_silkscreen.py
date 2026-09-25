# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for check_silkscreen.py (run: python -m unittest discover -s tools/kicad_ci).

TEST FIXTURES: the boards below are minimal hand-written s-expression snippets
in the KiCad 10 board format, not real design files. They are only written to
temporary directories at test time, so no .kicad_* file is committed.
"""

import contextlib
import io
import json
import shutil
import sys
import tempfile
import unittest
import unittest.mock
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_silkscreen as cs  # noqa: E402

BOARD = "hardware/boards/interface/revA"
PCB_PATH = f"{BOARD}/interface.kicad_pcb"

GOOD_TEXTS = [
    ('"open-bt-rig-interface ${VARIANT}"', "F.SilkS"),
    ('"Rev ${REVISION}  ${ISSUE_DATE}"', "F.SilkS"),
    ('"Designed by Reid Crowe, N0RC"', "B.SilkS"),
    ('"CC BY-NC-SA 4.0"', "B.SilkS"),
]


def gr_text(text, layer, extra=""):
    return (f'\t(gr_text {text}\n\t\t(at 10 10 0)\n\t\t(layer "{layer}"){extra}\n'
            f'\t\t(effects (font (size 1 1) (thickness 0.15)))\n\t)\n')


def variants_block(*names):
    """TEST FIXTURE: the board-level list of KiCad design variants."""
    items = "".join(f'\t\t(variant\n\t\t\t(name "{n}")\n\t\t)\n' for n in names)
    return f"\t(variants\n{items}\t)\n" if names else ""


def board(texts=GOOD_TEXTS, title='(title_block (date "2026-09-24") (rev "A"))', extra="",
          variants=("R", "M")):
    """TEST FIXTURE: a minimal KiCad 10 board with the given silkscreen texts and design variants."""
    body = "".join(gr_text(t, layer) for t, layer in texts)
    return (f'(kicad_pcb\n\t(version 20260206)\n\t(generator "pcbnew")\n\t(generator_version "10.0")\n'
            f'\t(paper "A4")\n\t{title}\n{variants_block(*variants)}{body}{extra})\n')


class SilkscreenTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def write(self, rel, text):
        p = self.root / rel
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(text, encoding="utf-8")

    def errors(self, tag=""):
        return cs.run(self.root, tag)[0]

    def assertOneError(self, *fragments, tag=""):
        errors = self.errors(tag)
        self.assertEqual(len(errors), 1, errors)
        for f in fragments:
            self.assertIn(f, errors[0])

    def replace_text(self, index, text, layer=None):
        texts = list(GOOD_TEXTS)
        texts[index] = (text, layer or texts[index][1])
        return texts

    def test_no_boards_passes(self):
        errors, summary = cs.run(self.root)
        self.assertEqual(errors, [])
        self.assertIn("no boards yet", summary)

    def test_good_board_passes(self):
        self.write(PCB_PATH, board())
        self.assertEqual(self.errors(), [])

    def test_title_block_revision_must_match_folder(self):
        self.write(PCB_PATH, board(title='(title_block (date "2026-09-24") (rev "B"))'))
        self.assertOneError("title block revision 'B' does not match folder revA")

    def test_title_block_revision_and_date_must_be_set(self):
        self.write(PCB_PATH, board(title='(title_block (rev ""))'))
        errors = self.errors()
        self.assertEqual(len(errors), 2, errors)
        self.assertIn("title block revision is not set", errors[0])
        self.assertIn("title block date is not set", errors[1])
        self.write(PCB_PATH, board(title=""))
        self.assertEqual(len(self.errors()), 2)

    def test_front_silkscreen_needs_revision_and_date_variables(self):
        self.write(PCB_PATH, board(self.replace_text(1, '"Rev ${REVISION} ${ISSUE_DATE}"', "B.SilkS")))
        errors = self.errors()
        self.assertEqual(len(errors), 2, errors)
        self.assertIn("contains ${REVISION}", errors[0])
        self.assertIn("contains ${ISSUE_DATE}", errors[1])

    def test_hard_coded_revision_fails(self):
        for text in ('"Rev A ${ISSUE_DATE} ${REVISION}"', '"revA ${REVISION} ${ISSUE_DATE}"',
                     '"REV: 2 ${REVISION} ${ISSUE_DATE}"', '"Revision B ${REVISION} ${ISSUE_DATE}"'):
            self.write(PCB_PATH, board(self.replace_text(1, text)))
            self.assertOneError("hard-codes a revision", "use ${REVISION}")

    def test_revision_variable_and_ordinary_words_are_not_hard_coded(self):
        for text in ('"Rev ${REVISION} ${ISSUE_DATE}"', '"Rev. ${REVISION} / ${ISSUE_DATE}"',
                     '"REV:${REVISION} ${ISSUE_DATE} Review"', '"Revision ${REVISION} ${ISSUE_DATE} Rx audio"'):
            self.write(PCB_PATH, board(self.replace_text(1, text)))
            self.assertEqual(self.errors(), [], text)

    def test_required_marks(self):
        self.write(PCB_PATH, board(GOOD_TEXTS[1:2] + [('"${VARIANT}"', "F.SilkS")]))
        errors = self.errors()
        self.assertEqual(len(errors), 3, errors)
        self.assertIn("missing the project name 'open-bt-rig-interface'", errors[0])
        self.assertIn("missing the designer credit 'Designed by Reid Crowe, N0RC'", errors[1])
        self.assertIn("missing the license mark 'CC BY-NC-SA 4.0'", errors[2])

    def test_marks_may_come_from_project_text_variables(self):
        texts = self.replace_text(3, '"${LICENSE}"')
        self.write(PCB_PATH, board(texts))
        self.assertOneError("license mark")
        self.write(f"{BOARD}/interface.kicad_pro", json.dumps({"text_variables": {"LICENSE": "CC  BY-NC-SA 4.0"}}))
        self.assertEqual(self.errors(), [])

    def test_hidden_and_non_silkscreen_text_is_ignored(self):
        texts = GOOD_TEXTS + [('"Rev A"', "F.Fab"), ('"Rev B"', "Cmts.User")]
        hidden = gr_text('"Rev C"', "F.SilkS", extra="\n\t\t(hide yes)")
        self.write(PCB_PATH, board(texts, extra=hidden))
        self.assertEqual(self.errors(), [])

    def test_footprint_text_on_silkscreen_counts(self):
        footprint = ('\t(footprint "Logo" (layer "F.Cu")\n'
                     '\t\t(property "Reference" "G1" (at 0 0 0) (layer "F.SilkS") (hide yes))\n'
                     '\t\t(property "Credit" "Designed by Reid Crowe, N0RC" (at 0 2 0) (layer "F.SilkS"))\n'
                     '\t\t(fp_text user "rev2" (at 0 1 0) (layer "F.SilkS"))\n\t)\n')
        self.write(PCB_PATH, board(self.replace_text(2, '"x"'), extra=footprint))
        self.assertOneError("F.SilkS text 'rev2' hard-codes a revision")

    def test_board_must_sit_in_a_rev_folder(self):
        for rel in ("hardware/boards/interface/interface.kicad_pcb", "hardware/boards/interface/A/interface.kicad_pcb",
                    "hardware/boards/interface/revA/sub/interface.kicad_pcb"):
            with self.subTest(rel=rel):
                shutil.rmtree(self.root / "hardware", ignore_errors=True)
                self.write(rel, board())
                self.assertOneError("must be in hardware/boards/<board>/rev<X>/")

    def test_unreadable_board_fails(self):
        self.write(PCB_PATH, "(kicad_pcb (version 20260206)")
        self.assertOneError("not a readable KiCad PCB")

    def test_boards_outside_hardware_boards_are_not_checked(self):
        self.write("hardware/lib/footprints/demo.kicad_pcb", board(texts=[]))
        self.assertEqual(self.errors(), [])

    def test_board_needs_design_variants(self):
        self.write(PCB_PATH, board(variants=()))
        self.assertOneError("no KiCad design variant is defined")

    def test_variant_names_must_fit_a_tag(self):
        for name in ("R M", "R_1", "-R", "R-", "Mobile/12V"):
            with self.subTest(name=name):
                self.write(PCB_PATH, board(variants=("R", name)))
                self.assertOneError(f"design variant {name!r} can't be used in a hw-<variant>")
        self.write(PCB_PATH, board(variants=("R", "M", "R-ISO", "U2")))
        self.assertEqual(self.errors(), [])
        self.assertEqual(self.errors("hw-R-ISO-revA-v1.0"), [])

    def test_front_silkscreen_needs_variant_variable(self):
        self.write(PCB_PATH, board(self.replace_text(0, '"open-bt-rig-interface R"')))
        self.assertOneError("no front silkscreen (F.SilkS) text contains ${VARIANT}")
        self.write(PCB_PATH, board(self.replace_text(0, '"open-bt-rig-interface ${VARIANT}"', "B.SilkS")))
        self.assertOneError("contains ${VARIANT}")

    def test_design_variants_are_read_from_the_board_only(self):
        footprint = ('\t(footprint "R_0402" (layer "F.Cu")\n'
                     '\t\t(variant (name "X") (dnp yes))\n\t)\n')
        tree = cs.parse_sexpr(board(extra=footprint))
        self.assertEqual(cs.design_variants(tree), ["R", "M"])

    def test_tag_matching(self):
        self.write(PCB_PATH, board())
        for tag in ("hw-R-revA-v1.0", "hw-M-revA-v1.0", "hw-R-revA-v1.0.1", "hw-R-revA-v1.2.3-rc.1",
                    "fw-v0.1.0", ""):
            self.assertEqual(self.errors(tag), [], tag)
        self.assertOneError("no board", "revB", tag="hw-R-revB-v1.0")
        self.assertOneError("declares the design variant 'U'", "interface/revA [R M]", tag="hw-U-revA-v1.0")
        self.assertOneError("declares the design variant 'r'", tag="hw-r-revA-v1.0")
        self.assertOneError("does not follow hw-<variant>-rev<X>-v<semver>", tag="hw-R-A-1.0")

    def test_tag_matches_the_variant_not_the_folder_name(self):
        self.write("hardware/boards/interface-U/revA/interface-U.kicad_pcb", board(variants=("R",)))
        self.assertOneError("declares the design variant 'U'", tag="hw-U-revA-v1.0")
        self.write("hardware/boards/small/revA/small.kicad_pcb", board(variants=("U",)))
        self.assertEqual(self.errors("hw-U-revA-v1.0"), [])

    def test_hardware_tag_without_boards_fails(self):
        self.assertOneError("boards: none", tag="hw-R-revA-v1.0")

    @unittest.mock.patch.dict("os.environ", {"GITHUB_ACTIONS": ""})
    def test_cli(self):
        self.write(PCB_PATH, board())
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(cs.main(["--root", str(self.root), "--tag", "hw-R-revA-v1.0"]), 0)
        self.assertIn("1 board(s) checked, tag hw-R-revA-v1.0", out.getvalue())
        with contextlib.redirect_stdout(io.StringIO()) as out:
            self.assertEqual(cs.main(["--root", str(self.root), "--tag", "hw-R-revB-v1.0"]), 1)
        self.assertIn("error: tag 'hw-R-revB-v1.0'", out.getvalue())

    def test_tag_from_github_ref(self):
        env = {"GITHUB_REF": "refs/tags/hw-R-revA-v1.0"}
        with unittest.mock.patch.dict("os.environ", env):
            self.assertEqual(cs.tag_from_env(), "hw-R-revA-v1.0")
        with unittest.mock.patch.dict("os.environ", {"GITHUB_REF": "refs/heads/dev"}):
            self.assertEqual(cs.tag_from_env(), "")


if __name__ == "__main__":
    unittest.main()

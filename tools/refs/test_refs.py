# SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
# SPDX-License-Identifier: MIT
"""Tests for tools/refs/refs.py. Run: python3 -m unittest discover -s tools/refs -p 'test_*.py'"""

import unittest

import refs


def entry(**over):
    base = {
        "id": "x-ds", "title": "X datasheet", "publisher": "Acme", "kind": "datasheet",
        "url": "https://example.com/x.pdf", "file": "x-ds.pdf", "used_in": ["README.md"],
    }
    base.update(over)
    return base


class ValidateTest(unittest.TestCase):
    def test_repo_manifest_is_valid(self):
        self.assertEqual(refs.validate(refs.load()), [])

    def test_valid_entry(self):
        self.assertEqual(refs.validate({"references": [entry()]}), [])

    def test_missing_field(self):
        errors = refs.validate({"references": [entry(url="")]})
        self.assertTrue(any("missing 'url'" in e for e in errors))

    def test_duplicates(self):
        errors = refs.validate({"references": [entry(), entry()]})
        self.assertTrue(any("duplicate id" in e for e in errors))
        self.assertTrue(any("duplicate file" in e for e in errors))

    def test_bad_values(self):
        errors = refs.validate({"references": [entry(id="Bad Id", kind="blog", file="../x", fetch="later")]})
        self.assertEqual(len(errors), 4)


class IndexTest(unittest.TestCase):
    def test_index_has_anchor_and_links(self):
        text = refs.render_index({"references": [entry(fetch="manual")]})
        self.assertIn("### x-ds", text)
        self.assertIn("[cache/x-ds.pdf](cache/x-ds.pdf)", text)
        self.assertIn("<https://example.com/x.pdf>", text)
        self.assertIn("manual", text)

    def test_repo_index_is_current(self):
        self.assertEqual(refs.INDEX.read_text(encoding="utf-8"), refs.render_index(refs.load()))


if __name__ == "__main__":
    unittest.main()

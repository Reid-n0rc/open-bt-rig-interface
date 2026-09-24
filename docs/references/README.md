<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Reference library

Every third-party document cited in this repo (datasheets, application notes,
FCC filings, SDK files, license texts, vendor pages) is listed in
[`manifest.json`](manifest.json), and browsable in [`index.md`](index.md).
Docs link to an entry's heading in the index, for example
`[ESP32-S3-MINI-1 datasheet](../references/index.md#esp32s3-mini1-ds)`.

## Why copies aren't committed

Most of these documents are copyrighted and don't allow redistribution
([`THIRD_PARTY.md`](../../THIRD_PARTY.md#standards-and-datasheets)). The repo
therefore tracks only the manifest (title, publisher, original URL, retrieval
date, SHA-256). Copies are downloaded into `cache/`, which is gitignored.

## Use

```sh
python3 tools/refs/refs.py fetch     # download everything missing into cache/
python3 tools/refs/refs.py verify    # check copies against the recorded SHA-256
```

Entries marked `"fetch": "manual"` come from sites that block scripted
downloads. `fetch` prints where to save them; open the URL in a browser and
save the file under that name.

## Adding a reference

1. Add an entry to `manifest.json`: a lowercase `id`, `title`, `publisher`,
   `kind` (`datasheet`, `app-note`, `fcc`, `sdk`, `license`, `issue`, `web`,
   `standard`), the original `url` (pin GitHub links to a commit), a cache
   `file` name, and the repo files that cite it in `used_in`.
2. Run `python3 tools/refs/refs.py fetch --id <id>`. It records the SHA-256
   and retrieval date in the manifest.
3. Run `python3 tools/refs/refs.py index` and link to `index.md#<id>` from your doc.
4. `python3 tools/refs/refs.py check` must pass.

When a publisher revises a document, `fetch --force` reports a hash change and
keeps the old copy. Review the new revision, then rerun with `--accept`.

<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Security policy

## Reporting a vulnerability

**Don't report security or safety problems in a public issue, pull request or
discussion.** Use GitHub's private vulnerability reporting instead:

1. Open the repository's
   [**Security** tab](https://github.com/Reid-n0rc/open-bt-rig-interface/security).
2. Choose **Report a vulnerability**
   ([direct link](https://github.com/Reid-n0rc/open-bt-rig-interface/security/advisories/new)).
3. Fill in the form. Only you and the maintainer can see the report.

From the command line, the GitHub CLI can file the same private report through
the [private vulnerability reporting API](https://docs.github.com/en/rest/security-advisories/repository-advisories#privately-report-a-security-vulnerability):

```sh
gh api --method POST repos/Reid-n0rc/open-bt-rig-interface/security-advisories/reports \
  -f summary='Short title of the problem' \
  -f description='What is affected, how to reproduce it, and the impact'
```

If you can't use GitHub, contact the maintainer, Reid Crowe, N0RC
([@Reid-n0rc](https://github.com/Reid-n0rc)), and ask for a private channel
without including the details.

## What to report

Anything that could let someone compromise the device or its host, or that could
cause harm to people or equipment, for example:

- **PTT safety:** firmware, protocol or hardware behavior that can key the
  transmitter unexpectedly, keep it keyed past the maximum TX timer, or defeat
  the fail-safes (off at boot, reset, brownout, disconnect, watchdog timeout).
- **Bluetooth LE and protocol:** unauthenticated control of CAT, PTT or audio,
  pairing or bonding weaknesses, parsing bugs in the protocol
  ([`protocol/`](protocol/)), or ways to bypass signed OTA updates.
- **Wired USB-C:** USB descriptors or class handling that can crash or
  compromise a host, or reach the radio's USB port unexpectedly.
- **Firmware update and supply chain:** unsigned or downgradeable firmware,
  compromised build or CI steps, or malicious third-party dependencies.
- **Hardware:** power or protection faults that can damage a radio, a vehicle
  electrical system or a host, or isolation failures that create a shock or
  fire risk.
- **Host tools** in [`tools/`](tools/) that execute untrusted input unsafely.

Regular bugs without a security or safety impact belong in a normal issue.

## What to include

- The affected area (firmware, protocol, hardware, tools, CI) and the commit,
  release tag or hardware revision.
- Steps to reproduce, or a proof of concept. Don't test against radios,
  vehicles or networks you don't own or aren't authorized to use, and never
  transmit on frequencies you aren't licensed for.
- The impact you expect, and any suggested fix.

## What happens next

This is a volunteer project, so these are goals, not guarantees:

- **Acknowledgement** within 7 days.
- **Assessment** (confirmed or not, severity) within 30 days.
- A fix, or a mitigation and a timeline, agreed with you before anything is
  published. Fixes are developed in a private security advisory fork when
  needed.
- **Credit** in the published GitHub security advisory and the release notes,
  unless you prefer to stay anonymous.

Please keep the report private until an advisory is published or 90 days have
passed, whichever comes first, unless we agree otherwise.

## Supported versions

The project is pre-release. Only the latest `dev` branch receives security
fixes. Once releases exist, the latest release of each line (`hw-*`, `fw-*`,
`proto-*`) will be supported; older releases get fixes only for safety issues
where practical.

## Scope notes

- Third-party components (SDKs, libraries, module firmware) keep their own
  security processes. Report their vulnerabilities upstream too; tell us if
  this project is affected.
- The FCC-certified radio module's RF behavior is the module vendor's
  responsibility; integration mistakes in this project are in scope.

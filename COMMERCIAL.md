<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Commercial Licensing

open-bt-rig-interface is **source-available**. Anyone can read, build, study
and modify it, but commercial use of the **hardware** and **firmware** needs a
commercial license from the copyright holder, Reid Crowe, N0RC.

| Part | License | Commercial use |
|---|---|---|
| Hardware (`hardware/`: KiCad designs, enclosure CAD) | [CC BY-NC-SA 4.0](LICENSES/CC-BY-NC-SA-4.0.txt) | Commercial license required |
| Firmware (`firmware/`) | [PolyForm Noncommercial 1.0.0](LICENSES/PolyForm-Noncommercial-1.0.0.txt) | Commercial license required |
| Protocol spec + golden vectors (`protocol/`), tools, CI | [MIT](LICENSES/MIT.txt) | Allowed |
| Documentation | [CC BY 4.0](LICENSES/CC-BY-4.0.txt) | Allowed |

See [`LICENSE`](LICENSE) for the per-path overview.

## What that means in practice

**Personal and amateur use is free. Always.** Build one for your own station,
modify it, experiment with it and share your changes non-commercially. The
firmware license explicitly covers "hobby projects, amateur pursuits" and
personal study. The hardware license covers any use that isn't "primarily
intended for or directed towards commercial advantage or monetary
compensation."

**Non-commercial organizations are welcome too.** The firmware license permits
use by charitable organizations, educational institutions, public research,
public safety and government institutions. Schools, EmComm groups and
non-profit clubs can build and use the design.

**The protocol is open to everyone, including commercial apps.** The
host-device protocol in `protocol/` (spec and golden vectors) is MIT, so any
app, open-source or commercial, can implement it and talk to the device
without a license from us. Only the hardware design and the device firmware
are restricted.

## What needs a commercial license

- Selling boards, kits, assembled units or refurbished units built from this
  design, including selling "at a small profit".
- Shipping this firmware, modified or not, in any product you sell or rent out.
- Incorporating the hardware design, or substantial parts of it, into a
  commercial product.
- Offering paid services built on the hardware or firmware, such as paid
  assembly, installation-as-a-service or programming services.

**Clubs or groups organizing at-cost group builds:** ask first. These requests
are considered case by case.

If you're unsure whether your use is commercial, ask before you start.

## The commercial option

A commercial license removes the non-commercial restriction for the hardware
and/or firmware under agreed terms, for example per-unit or per-product
royalties, a flat fee, or a no-cost license for approved community projects.
Attribution and any support terms can be part of the same agreement.

**Contact:** Reid Crowe, N0RC, via the contact details on
[github.com/Reid-n0rc](https://github.com/Reid-n0rc) or
[qrz.com/db/N0RC](https://www.qrz.com/db/N0RC). Include a sentence or two about
what you plan to build or sell, the expected volume, and your organization. You
can also open a GitHub issue titled "Commercial license inquiry" to start the
conversation. Don't put confidential details in a public issue.

## Third-party components

Third-party components (libraries, SDKs, reference designs) keep their own
licenses. They are **not** covered by the non-commercial terms or by a
commercial license from this project. Their required notices are in
[`THIRD_PARTY.md`](THIRD_PARTY.md). A commercial licensee must still comply
with those licenses.

## Contributions

To keep dual licensing possible, contributions require agreeing to the
contributor terms in [`CONTRIBUTING.md`](CONTRIBUTING.md#contributor-terms).

## Licensing history

Before the change to non-commercial terms (up to and including commit
`b4041a4` on `dev`), `hardware/` was licensed CERN-OHL-P-2.0 and `firmware/`
was MIT. At that point both directories contained only placeholder READMEs.
Copies obtained under those earlier terms remain under them. All later
versions are licensed as described above.

## Limits of these licenses

These licenses are copyright licenses. They cover the design files, firmware
and documentation in this repository. They don't cover general ideas,
techniques or functional behavior, and they don't stop someone from
independently designing a similar device. This page is not legal advice.

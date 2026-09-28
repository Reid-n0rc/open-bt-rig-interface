<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0009: EU conformity is a design requirement for every variant

- **Status:** proposed
- **Date:** 2026-09-25
- **Issue:** #59

## Context

[`constraints.md`](../requirements/constraints.md) §4 said "ISED and CE:
optional, later", and REQ-REG-006 said CE wasn't required for revision A. On
2026-09-24 the maintainer set a new requirement: **the product must meet EU
requirements for electronics.**

The research is in [`docs/compliance/eu.md`](../compliance/eu.md). In short, it
found that for units made available on the EU market (facts, not legal advice):

- The **RED** (2014/53/EU) applies to both variants: safety and RF exposure
  (3.1(a)), EMC (3.1(b)) and spectrum (3.2). The amateur-radio exclusion
  doesn't cover units that are sold ([eu.md](../compliance/eu.md) §3.5).
- **RoHS**, **REACH** Art. 33, **WEEE** marking and registration, part of the
  **GPSR**, and the **EU economic operator** rule apply ([eu.md](../compliance/eu.md) §6–§7).
- The **Cyber Resilience Act** applies to units placed on the market from
  2027-12-11, for any data connection (BLE or USB), with reporting duties from
  2026-09-11 ([eu.md](../compliance/eu.md) §5). The RED cybersecurity rules
  (Delegated Regulation 2022/30) apply before that date only if the device is
  "internet-connected", which can be read two ways ([eu.md](../compliance/eu.md) §4.2),
  and are repealed from 2027-12-11 by Delegated Regulation (EU) 2026/339.
- Espressif's EU-type examination of the module lists BLE at **9.96 dBm
  e.i.r.p.** EN 300 328's adaptivity rules start at 10 dBm e.i.r.p.; the FCC
  cap (10.3 dBm conducted) plus antenna gain can exceed that
  ([eu.md](../compliance/eu.md) §3.1).
- Module A (self-assessment) is available for everything if the cited
  harmonised standards are applied in full ([eu.md](../compliance/eu.md) §10).

## Options considered

| Option | Pros | Cons | Sources |
|---|---|---|---|
| A. Keep EU conformity optional, later | No change now | Retrofitting TX power, labels, EMC margins and security after layout and firmware exist costs more; contradicts the maintainer's requirement | [`constraints.md`](../requirements/constraints.md) §4 before this ADR |
| **B. EU conformity required for every variant; design to it now; testing and declarations stay `human-task`** | Meets the requirement; the design choices (TX power, EMC, parts, labels, security) are cheap now; one design for the US and EU | More design work in #10, #11, #13, #14, #19; a support period and vulnerability process to run | [`eu.md`](../compliance/eu.md), [RED](../references/index.md#eu-red-2014-53), [CRA](../references/index.md#eu-cra-2024-2847) |
| C. Required only for commercial licensees; the project design ignores it | Less work for the project | Each licensee would redesign; the project design couldn't be sold in the EU as is | [`COMMERCIAL.md`](../../COMMERCIAL.md) |

## Decision

**Option B.** Every variant is designed so that a manufacturer can place it on
the EU market with an EU declaration of conformity and CE marking, under the
RED, RoHS and, from 2027-12-11, the CRA, using Module A where the harmonised
standards allow. Test, declaration and registration work stays `human-task`.

**Security level (maintainer decision, 2026-09-25):** design product security
to the **Cyber Resilience Act** level now (Annex I; [eu.md](../compliance/eu.md) §4.4, §5).
At implementation time, also plan for the RED internet-connected level
(Delegated Regulation 2022/30, EN 18031-1), with an explanation of its
complexity and production cost before it is built. The implementation, the
EN 18031-1 gap analysis and the cost go to [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64).

## Consequences

- [`constraints.md`](../requirements/constraints.md) §4: EU conformity is
  required, not optional. ISED stays optional.
- [`requirements.md`](../requirements/requirements.md): REQ-REG-006 is withdrawn;
  REQ-REG-007 to -014 and REQ-EMC-008, -009 are added.
- **BLE TX power (#14):** one firmware cap for all markets, no higher than the
  CE-tested 9.96 dBm e.i.r.p. and the FCC's 10.3 dBm conducted (REQ-REG-008).
- **EMC (#10, #11):** EN 301 489-1/-17 and EN 55032 Class B / EN 55035 targets
  for both variants, alongside FCC Part 15B and CISPR 25; variant M is tested as
  vehicular (ISO 7637-2 level III within EN 301 489-1).
- **Parts (#8, #9, #10, #11):** every part RoHS-compliant, with RoHS and REACH
  SVHC status in the BOM notes.
- **Enclosure and labels (#19):** CE mark, WEEE bin with date bar, type or
  serial number, manufacturer and EU operator address, on the product and packaging.
- **Protocol and firmware (#13, #14):** the recommendations in
  [`eu.md`](../compliance/eu.md) §4.4 (pairing, wired-transport trust, no default
  passwords, signed user-installable updates, secure storage, factory reset,
  logging, robustness); an SBOM, a support period of at least five years, and a
  CRA Art. 14 reporting process in [`SECURITY.md`](../../SECURITY.md) before any
  unit is placed on the market.
- **Decided:** the "internet-connected" question under 2022/30
  ([eu.md](../compliance/eu.md) §4.2) is closed as "design to the CRA level; the
  EN 18031-1 gap analysis and cost go to [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64)".
- **Open decisions for the maintainer** (not made by this ADR):
  1. Is variant R declared for use in vehicles ([eu.md](../compliance/eu.md) §3.3)?
  2. RED only, or RED plus a UN R10 §3.2.9 declaration for variant M
     ([eu.md](../compliance/eu.md) §8)?
  3. Who is the EU economic operator and WEEE representative if units are sold
     from the US ([eu.md](../compliance/eu.md) §7)?
  4. The firmware's CRA status when supplied under commercial licences
     ([eu.md](../compliance/eu.md) §11); get advice.
  5. The support period length.
- **Risks to verify:** the module's antenna gain and the ESP-IDF power index that
  gives 9.96 dBm e.i.r.p.; whether a lab accepts Espressif's radio reports for
  the host; the EN 62479 threshold; the R10 06-series text; CRA harmonised
  standards when published.
- UKCA, Switzerland and ISED are out of scope.

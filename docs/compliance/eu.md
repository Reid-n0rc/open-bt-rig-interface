<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# EU: applicable legislation, harmonised standards and conformity route

Issue: [#59](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/59).
Written 2026-09-25 for revision A (ESP32-S3-MINI-1-N8,
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)), both variants:
**R** (radio DC or USB-C powered) and **M** (vehicle 12 V). The decision that EU
conformity is required is [ADR-0009](../decisions/ADR-0009-eu-compliance.md).
The FCC side is in [`fcc.md`](fcc.md).

**These are facts from the legal texts, not legal advice.** Where a text can be
read two ways, both readings and their consequences are given, and the choice
is marked **(maintainer)**. Testing, the declaration of conformity and
registrations need a person, a lab and accounts; they are `human-task` work.

| Item | Value |
|---|---|
| Radio | Bluetooth LE only, 2402–2480 MHz; Wi-Fi off ([`fcc.md`](fcc.md) §1.3) |
| Module EU status | EU-type examination certificate **0370-RED-4972** (notified body 0370, LGAI/Applus, 2022-02-25), RED Art. 3.1(a), 3.1(b), 3.2; **BLE 9.96 dBm e.i.r.p.** maximum; Art. 3.3 not assessed ([certificate](../references/index.md#espressif-s3-mini1-ce-cert)) |
| Main act | Radio Equipment Directive 2014/53/EU (RED); the LVD and the EMC Directive don't apply to radio equipment (RED Art. 1(4); [RED guide](../references/index.md#ec-red-guide-2018) §9.5) |
| Conformity route | Internal production control (Module A) for everything, **if** the cited harmonised standards are applied in full (§9, §10) |
| Key dates | RED cybersecurity (Delegated Regulation 2022/30) since 2025-08-01, **repealed from 2027-12-11**; Cyber Resilience Act reporting from 2026-09-11, **full application 2027-12-11** |

## 1. Texts and versions used

Consolidated texts are documentation only; the Official Journal (OJ) text is
authentic. Each reference entry links the EUR-Lex page by CELEX number.

| Act | Version read | Reference |
|---|---|---|
| RED 2014/53/EU | Consolidated 2026-05-30 (amendments up to (EU) 2024/2749) | [eu-red-2014-53](../references/index.md#eu-red-2014-53) |
| Delegated Regulation (EU) 2022/30 | Consolidated 2023-10-27 (as amended by (EU) 2023/2444); recitals from the OJ text | [eu-red-da-2022-30](../references/index.md#eu-red-da-2022-30), [OJ](../references/index.md#eu-red-da-2022-30-oj) |
| Delegated Regulation (EU) 2026/339 | OJ L, 2026/339, 29.4.2026 | [eu-da-2026-339](../references/index.md#eu-da-2026-339) |
| RED harmonised standards, Implementing Decision (EU) 2022/2191 | Consolidated 2025-12-11, plus (EU) 2026/2003 (doesn't touch the standards below) | [eu-red-hs-2022-2191](../references/index.md#eu-red-hs-2022-2191), [2026/2003](../references/index.md#eu-red-hs-2026-2003), [EC summary list](../references/index.md#ec-red-hs-summary) |
| Cyber Resilience Act (EU) 2024/2847 (CRA) | OJ L, 2024/2847 (no consolidated version yet); Implementing Regulation (EU) 2025/2392 | [eu-cra-2024-2847](../references/index.md#eu-cra-2024-2847), [eu-cra-2025-2392](../references/index.md#eu-cra-2025-2392) |
| RoHS 2011/65/EU | Consolidated 2026-07-01; standard: Implementing Decision (EU) 2020/659 | [eu-rohs-2011-65](../references/index.md#eu-rohs-2011-65), [eu-rohs-hs-2020-659](../references/index.md#eu-rohs-hs-2020-659) |
| REACH (EC) 1907/2006 | Consolidated 2026-06-22; CJEU C-106/14; Waste Framework Directive consolidated 2025-10-16 | [eu-reach-1907-2006](../references/index.md#eu-reach-1907-2006), [cjeu-c-106-14](../references/index.md#cjeu-c-106-14), [eu-wfd-2008-98](../references/index.md#eu-wfd-2008-98) |
| WEEE 2012/19/EU | Consolidated 2024-04-08 | [eu-weee-2012-19](../references/index.md#eu-weee-2012-19) |
| GPSR (EU) 2023/988 | Consolidated 2026-05-29 | [eu-gpsr-2023-988](../references/index.md#eu-gpsr-2023-988) |
| Market surveillance (EU) 2019/1020 | Consolidated 2026-08-12 | [eu-msr-2019-1020](../references/index.md#eu-msr-2019-1020) |
| CE marking, (EC) 765/2008 | Consolidated 2021-07-16 | [eu-nlf-765-2008](../references/index.md#eu-nlf-765-2008) |
| Short-range devices, Decision 2006/771/EC | Consolidated 2025-01-23 | [eu-srd-2006-771](../references/index.md#eu-srd-2006-771) |
| Blue Guide 2022 (2022/C 247/01) and RED Guide (2018) | Guidance, not binding | [eu-blue-guide-2022](../references/index.md#eu-blue-guide-2022), [ec-red-guide-2018](../references/index.md#ec-red-guide-2018) |

## 2. Applicability per variant

| Legislation / requirement | Variant R | Variant M | Basis |
|---|---|---|---|
| RED 2014/53/EU | **Applies** | **Applies** | Radio equipment: intentionally emits radio waves for radio communication (Art. 2(1)(1)). The amateur-radio exclusion doesn't cover units made available on the market (§3.5) |
| RED 3.1(a) safety and RF exposure | Applies, **no voltage limit** | Applies, no voltage limit | Art. 3(1)(a); §3.4 |
| RED 3.1(b) EMC | Applies | Applies, as **vehicular** equipment | Art. 3(1)(b); EN 301 489-17 Table 3 and 5 (§3.3) |
| RED 3.2 spectrum | Applies | Applies | Art. 3(2); EN 300 328 (§3.1) |
| RED 3.3(d)(e) cybersecurity (2022/30) | Depends on "internet-connected", only for units placed on the market before 2027-12-11. **Decided:** design to the CRA level; EN 18031-1 gap analysis and cost in [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64) | Same | §4 |
| RED 3.3(f) fraud | No | No | No money transfer (2022/30 Art. 1(3)) |
| RED Art. 3(4), common charger (2022/2380) | No | No | No battery; not an Annex Ia category (§8) |
| LVD 2014/35/EU, EMC Directive 2014/30/EU | No | No | RED Art. 1(4); EMCD Art. 2(2)(a) with RED Art. 50 ([RED guide](../references/index.md#ec-red-guide-2018) §9.5) |
| Cyber Resilience Act | **Applies** to units placed from 2027-12-11; Art. 14 reporting from 2026-09-11 for units in scope already on the market | Same (not type-approved under (EU) 2019/2144) | CRA Art. 2(1), 2(2)(c), 69, 71 (§5) |
| RoHS 2011/65/EU | Applies | Applies (not a "means of transport", Art. 2(4)(f)) | §6.1 |
| REACH Art. 33 (SVHC) | Applies if any article holds an SVHC > 0.1 % w/w | Same | §6.2 |
| WEEE 2012/19/EU | Applies (marking, registration per Member State) | Same | §6.3 |
| GPSR (EU) 2023/988 | Partly: distance sales, accidents, recalls | Same | Art. 2(1) (§7) |
| EU economic operator (2019/1020 Art. 4) | Required | Required | RED and RoHS are listed in Art. 4(5) (§7) |
| UN ECE Regulation No 10 (E-mark) | No | **Two readings (maintainer)** | §8 |
| Battery Regulation (EU) 2023/1542 | No (no battery) | No | Revisit if a coin cell or battery is added |

## 3. Radio Equipment Directive

### 3.1 Spectrum (Art. 3.2): EN 300 328

EN 300 328 V2.2.2 is cited under the RED (2022/2191 Annex I row 16;
[EN 300 328](../references/index.md#etsi-en-300-328)). BLE is non-FHSS
wideband equipment there. Its limits are **e.i.r.p.**, so they include antenna gain:

| Parameter (non-FHSS) | EN 300 328 V2.2.2 | Clause |
|---|---|---|
| RF output power | ≤ 20 dBm e.i.r.p. | 4.3.2.2.3 |
| Power spectral density | ≤ 10 dBm/MHz e.i.r.p. | 4.3.2.3.3 |
| Duty cycle, medium utilisation, adaptivity | **Don't apply below 10 dBm e.i.r.p.** | 4.3.2.4.1, 4.3.2.5.1, 4.3.2.6.1 |
| Receiver category | 2 for > 0 dBm and ≤ 10 dBm e.i.r.p. | 4.2.3.2.2 |

The band plan agrees: 2400–2483.5 MHz wideband data transmission, 100 mW
e.i.r.p. and 10 mW/MHz for non-hopping modulation, harmonised across the Union
(Decision 2006/771/EC Annex, band 57c; [eu-srd-2006-771](../references/index.md#eu-srd-2006-771)).
So the device can be used in every Member State (RED Art. 10(2)), and there are
no Art. 10(10) restrictions to print on the packaging.

**Against the FCC cap.** The FCC grant certifies BLE at 10.3 dBm
**conducted** ([`fcc.md`](fcc.md) §1.3). Espressif's EU certificate lists BLE
at **9.96 dBm e.i.r.p.** (annex A.6, whose footnote gives e.i.r.p. for
non-cellular technologies). The antenna gain isn't published: the
certificate leaves it blank and the datasheet doesn't give it **(verify, from
Espressif's EN 300 328 report R2112A1104-R1V1)**. So:

- The FCC cap (10.3 dBm conducted plus a PCB antenna's gain) can exceed
  **10 dBm e.i.r.p.** The PSD limit (10 dBm/MHz) always applies, and a
  1 Mbit/s BLE channel puts most of its power within 1 MHz, so about 10 dBm
  e.i.r.p. is also the practical PSD ceiling **(verify with the lab)**. At or
  above 10 dBm e.i.r.p. the duty-cycle, medium-utilisation and adaptivity
  rules apply as well.
- *Project rule (proposed):* one firmware TX-power setting for all markets, no
  higher than the setting Espressif used for the 9.96 dBm e.i.r.p. result.
  This keeps the device below 10 dBm e.i.r.p., inside both the FCC grant and the
  module's EU test, and out of the adaptivity requirements. Which ESP-IDF power
  index that is **(verify)**. PR #58 §6.3 already makes the cap a firmware
  constant that the protocol can only lower; only its value changes. REQ-REG-008.

### 3.2 Reusing Espressif's RED documents

- The certificate 0370-RED-4972 is a Module B EU-type examination of the
  **module** as Espressif places it on the market (HW V1.1, SW V1.1.3.0),
  against EN IEC 62368-1:2020+A11:2020, EN IEC 62311:2020, EN 50665:2017,
  EN 55032:2015+A11:2020, EN 55035:2017+A11:2020, EN 301 489-1 V2.2.3,
  EN 301 489-17 V3.2.4 and EN 300 328 V2.2.2. Art. 3.3 is not ticked.
- A module is radio equipment in its own right. The rules for placing it on
  the market are "without prejudice to any new obligations that might arise"
  when it is integrated into another product ([RED guide](../references/index.md#ec-red-guide-2018)
  §1.6.3.9). The **end product needs its own conformity assessment, technical
  documentation and EU declaration of conformity**, by its own manufacturer (§10).
- What can be reused: the module's radio test results as supporting evidence
  for Art. 3.2, where the integration follows the module's guidelines and the
  TX-power setting is the tested one. Whether a lab accepts that instead of
  conducted and radiated re-tests in the product is **(verify with the lab)**.
  Espressif's test reports aren't public; ask Espressif for them **(verify)**.
- EMC, safety and RF exposure must be assessed on the **whole product**: the
  module's EMC and safety reports cover the module alone.

### 3.3 EMC (Art. 3.1(b)): EN 301 489 series

| Standard | Status under the RED | Use |
|---|---|---|
| EN 301 489-17 V3.3.1 | Cited, 2022/2191 row 169 (added by (EU) 2025/893), with two notices: no presumption for emissions below 9 kHz, or if its clause 6 performance criteria are applied | Product EMC standard for BLE ([EN 301 489-17](../references/index.md#etsi-en-301-489-17)) |
| EN 301 489-1 V2.2.3 | Not cited on its own; -17 V3.3.1 makes it a normative reference and applies its test methods | Test methods, including vehicle transients ([EN 301 489-1](../references/index.md#etsi-en-301-489-1)) |
| EN 55035:2017 | Cited, row 5 | Immunity of the multimedia (audio) functions |
| EN 55032:2015/A11:2020 | Cited under the EMC Directive only ([2019/1326](../references/index.md#eu-emc-hs-2019-1326) row 13), not under the RED | Emissions of the multimedia functions; an "other technical specification" under the RED |

Only standards cited under the RED give a presumption of conformity with the
RED ([RED guide](../references/index.md#ec-red-guide-2018) §11.2). Art. 3.1(b)
may still use Module A with other specifications (RED Art. 17), as Espressif's
list above does.

**Vehicular equipment.** EN 301 489-17 V3.3.1 (Tables 3 and 5) adds, for
vehicular equipment: conducted emissions on the DC power port, RF common-mode
immunity, and **transients and surges in the vehicular environment**
(EN 301 489-1 §9.6: ISO 7637-2 (2004) pulses 1, 2a, 2b, 3a, 3b and 4 at
**test level III**). It also treats portable equipment that can be powered from a
vehicle's main battery in its intended use as vehicular.

- Variant M is vehicular. Its [#10](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/10)
  targets (ISO 7637-2:2011 level IV, PR #57) are stricter than level III.
- Variant R powered from a **mobile** radio's accessory socket is indirectly
  on a vehicle battery. Whether R is declared vehicular depends on its stated
  intended use **(maintainer)**. If the manual allows use in a vehicle, test R
  as vehicular too.

### 3.4 Safety and RF exposure (Art. 3.1(a))

- **No safety or RF-exposure standard is cited under the RED.** 2022/2191
  lists none for these products; EN 62368-1:2014, EN 62311:2008 and EN 62479:2010
  are cited under the LVD ([2023/2723](../references/index.md#eu-lvd-hs-2023-2723)
  rows 554, 555, 561), which doesn't give a presumption under the RED. Module A
  is still allowed for Art. 3.1 (RED Art. 17).
- Use the current editions as the technical specifications, as Espressif did:
  EN IEC 62368-1:2020+A11:2020 for safety; EN 62479 (low-power exclusion) or
  EN IEC 62311:2020 / EN 50665:2017 for RF exposure. The module's maximum,
  9.96 dBm (about 10 mW) e.i.r.p., is below EN 62479's low-power threshold for
  the general public (20 mW) **(verify against the standard)**. That would mean
  no minimum separation distance in the EU, unlike the FCC's 20 cm condition.
- Art. 3.1(a) has **no voltage limit**, and the assessment must cover
  "reasonably foreseeable conditions" (Art. 17). For variant M that includes
  load dump, reverse battery and wiring faults (PR #57); for both, fire and
  temperature risks of the enclosure (PETG/ASA, [`constraints.md`](../requirements/constraints.md) §11).

### 3.5 The amateur-radio exclusion

RED Annex I(1) excludes "radio equipment used by radio amateurs within the
meaning of Article 1, definition 56, of the [ITU] Radio Regulations, unless the
equipment is made available on the market", and treats as not made available:
kits for assembly and use by radio amateurs, equipment modified by and for
radio amateurs, and "equipment constructed by individual radio amateurs for
experimental and scientific purposes related to amateur radio".

- **A unit that is sold, or supplied in the course of a commercial activity
  even for free, is not excluded** (Annex I(1) "unless ... made available";
  Art. 2(1)(9)). Confirmed.
- **A unit a licensed amateur builds for their own station:** two readings.
  (a) It is excluded: amateur-built equipment used in the amateur station.
  (b) It isn't covered by the exclusion, because the device's radio is a
  Bluetooth LE link, not an amateur-service transmitter; but it is also not
  "placed on the market" (§11). The practical result is the same for a
  one-off build; it differs for regular transfers between amateurs, which the
  [RED guide](../references/index.md#ec-red-guide-2018) §1.6.2.2 assesses case
  by case.

## 4. RED cybersecurity: Art. 3.3(d)(e)(f) and EN 18031

### 4.1 The rule and its end date

- Art. 3.3(d) (network harm) applies to "any radio equipment that can
  communicate itself over the internet, whether it communicates directly or via
  any other equipment" (2022/30 Art. 1(1)). Art. 3.3(e) (personal data and
  privacy) applies to such equipment if it can process personal data, traffic
  data or location data (Art. 1(2)(a)). The childcare, toy and wearable
  categories (Art. 1(2)(b)–(d)) don't fit this device. Art. 3.3(f) needs money
  transfer, which the device doesn't do.
- Recital 5 explains the term: such equipment "operates protocols necessary to
  exchange data with the internet either directly or by means of an
  intermediate equipment" ([OJ text](../references/index.md#eu-red-da-2022-30-oj)).
- Applies from 2025-08-01 (Art. 3 as amended by (EU) 2023/2444).
- **Repealed with effect from 2027-12-11** by Delegated Regulation (EU)
  2026/339, because the CRA's Annex I covers the same ground. Market
  surveillance of equipment placed between 2025-08-01 and 2027-12-10 continues
  (2026/339 recital 5; [eu-da-2026-339](../references/index.md#eu-da-2026-339)).
- **So the question below matters only if units are placed on the market
  before 2027-12-11.** After that date the CRA applies regardless (§5).

### 4.2 Is the device "internet-connected"?

**Decision (maintainer, 2026-09-25):** design product security to the Cyber
Resilience Act level now (§4.4, §5). At implementation time, also plan for the
internet-connected level (EN 18031-1), with its complexity and production cost
explained before it is built. The EN 18031-1 gap analysis and cost go to
[#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64) ([ADR-0009](../decisions/ADR-0009-eu-compliance.md)). The analysis
below is kept as the basis for that work.

No Commission guidance that defines the term further was found **(verify)**.
The only structured reading found is an industry paper ([Orgalim 2022](../references/index.md#orgalim-internet-connected-2022),
not binding): equipment is in scope if it is designed to communicate over the
internet without modification, has the technical capability ("internet-ready"),
and supports protocols that allow it, typically the Internet Protocol suite.
Its examples: headphones and mice linked to an internet-ready phone are out of
scope (scenario 5, product B); products that are internet-ready through a
**wired** interface are in scope even if their radio isn't (scenario 3).

| Interface | Reading A: not internet-connected | Reading B: internet-connected |
|---|---|---|
| **BLE to a phone, tablet or computer** | The device speaks the project protocol over GATT or L2CAP. It runs no internet protocol over BLE, so it doesn't "operate protocols necessary to exchange data with the internet" (recital 5). Orgalim scenario 5, product B | A host app can relay the protocol over the internet (remote station operation), so the device communicates "via any other equipment" (Art. 1(1)) |
| **USB network (CDC-NCM, PR #58 §14.2)** | Link-local only: a /30 DHCP lease with no router and no DNS, no forwarding, listening only on DHCP, mDNS and TCP 51621. It can't reach the internet unless the host is deliberately set up to forward or relay | The device runs IPv4, IPv6, TCP, DHCP and mDNS: the Internet Protocol suite. A host that routes, NATs or port-forwards makes it reachable (Orgalim scenario 3, wired) |

Consequences:

- **Reading A for both interfaces:** Art. 3.3 doesn't apply. Only Art. 3.1 and
  3.2 are declared until 2027-12-10. The security work is still needed for the
  CRA from 2027-12-11.
- **Reading B for either interface:** Art. 3.3(d) applies, and 3.3(e) too,
  because the device can process personal data (a callsign in the device name,
  host identities in the bond table, voice audio). Apply **EN 18031-1:2024**
  and **EN 18031-2:2024** (2022/2191 rows 164–165, added by (EU) 2025/138).
  Module A is allowed only if they are applied in full without triggering
  their OJ restrictions (§4.3); otherwise a notified body is needed (RED Art. 17).
- **Mixed (A for BLE, B for the USB network)** is the reading the Orgalim
  scenarios point to. The USB network is the only IP stack in the product.
  `WIRED_PROFILE` now defaults to **serial** (PR #58 §6.1), which leaves the
  network out only for radios whose serial is on the SERIAL jack; for radios
  with their own USB serial chip, the wired function sets always include the
  network (PR #58 §14.1). Either way the capability is in the product.
- A design option, if the maintainer wants reading A to be clear-cut for units
  placed before 2027-12-11: ship those units without the network function. That
  loses the protocol over TCP, so wired iOS hosts lose PTT and configuration,
  and radios with USB serial and analog audio lose the wired protocol transport
  (PR #58 §14.1). To be weighed in [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64).

### 4.3 EN 18031-1 in brief

EN 18031-1:2024 is a paid standard ([catalogue](../references/index.md#cen-en-18031-1));
don't commit a copy. Its mechanisms are access control (ACM), authentication
(AUM), secure update (SUM), secure storage (SSM), secure communication (SCM),
resilience (RLM), network monitoring (NMM), traffic control (TCM), confidential
cryptographic keys (CCK), general equipment capabilities (GEC) and cryptography
(CRY); EN 18031-2 adds logging, deletion and user notification
([BSI overview](../references/index.md#bsi-red-cyber-guide), secondary source).
The OJ citation restricts the presumption of conformity
([2022/2191](../references/index.md#eu-red-hs-2022-2191) rows 164–166):

- the "rationale" and "guidance" sections give no presumption;
- **no presumption if, under clauses 6.2.5.1 and 6.2.5.2, the user may choose
  not to set and use any password.**

### 4.4 Recommendations for the protocol (#13) and firmware (#14)

Recommendations only; the protocol isn't changed here. They serve both RED
reading B and the CRA's Annex I Part I, which applies from 2027-12-11 whatever
the reading.

| Topic | Current design (PR #58) | Recommendation | Basis |
|---|---|---|---|
| BLE pairing | LE Secure Connections, bonded and encrypted link for RX, TX and L2CAP; "Just Works"; new bonds only in a pairing window opened by a local action (§13.5) | Keep. Reject legacy pairing and unencrypted access to everything but Info. Record the "Just Works" MITM risk and the pairing-window mitigation in the risk assessment | ACM, AUM, SCM; CRA Annex I(2)(d)(e) |
| Wired control port (CDC-ACM) and TCP 51621 over the USB network | No authentication; trust rests on physical access to the USB-C cable | Decide and document the trust boundary. If a host that forwards traffic is in scope (reading B), add authentication to the TCP transport: for example a per-device secret shown or set by a local action, never a shared default. Accept connections only from the link's own subnet or link-local addresses. Keep one session at a time and rate-limit `HELLO` | ACM, AUM, RLM; CRA Annex I(2)(d)(h)(j) |
| Passwords | None | If any credential is added, no default or shared password: unique per device, or set by the user, and the user can't skip it (the OJ restriction above) | AUM; 2022/2191 row 164 notice 2 |
| Firmware update | OTA transport deferred to #14; signed images required if OTA exists (REQ-FW-006) | Updates must be possible and secure: signed images (Secure Boot v2), anti-rollback, updates accepted only on an authenticated channel or by a local action. Decide production state of the ROM download mode on UART0 and USB (it bypasses signed updates unless secure download mode is set **(verify, #14)**) | SUM, GEC; CRA Annex I(2)(c), Part II(7)(8) |
| Secure storage | Bond storage in NimBLE (#14) | Keep LTKs, IRKs and any secrets in encrypted NVS with flash encryption; no keys in logs | SSM, CCK; CRA Annex I(2)(e)(f) |
| Factory reset | Not specified | A local action that erases bonds, secrets and configuration | CRA Annex I(2)(b)(m) |
| Logging | Not specified | Keep a small log of security events (pairing, rejected connections, update results), readable by the user; opt-out | CRA Annex I(2)(l); EN 18031-2 logging |
| Robustness | Framing with payload limits and a CRC (§3, §12) | Fuzz the frame parser and TCP listener; bounded buffers; no crash on malformed input | RLM; CRA Annex I(2)(h)(k) |
| Attack surface | USB network listens on DHCP, mDNS and TCP 51621 only | Keep that list minimal and documented; no debug services in production | GEC; CRA Annex I(2)(j) |
| Data | Device name (may hold a callsign), bond table | Store only what's needed; document it | CRA Annex I(2)(g) |

## 5. Cyber Resilience Act (EU) 2024/2847

- **Dates** (Art. 71): in force since 2024-12-10; Chapter IV (notified bodies)
  from 2026-06-11; **Art. 14 reporting from 2026-09-11**; everything else from
  **2027-12-11**. Units placed on the market before 2027-12-11 fall under the CRA
  only after a substantial modification (Art. 69(2)), but Art. 14 reporting
  applies to them too (Art. 69(3)).
- **Scope** (Art. 2(1)): products with digital elements whose intended or
  reasonably foreseeable use includes "a direct or indirect logical or physical
  data connection to a device or network". BLE and USB both qualify, so there
  is **no internet threshold**, unlike 2022/30. The exclusion for products
  under Regulation (EU) 2019/2144 (Art. 2(2)(c)) covers vehicle type approval,
  which variant M doesn't have.
- **Class:** the device's core functionality (a rig interface) isn't an
  Annex III or IV category. Implementing Regulation (EU) 2025/2392 judges by
  core functionality (recital 5), and integrating an important product doesn't
  make the host important (CRA Art. 7(1)). The closest entries are class I
  "physical and virtual network interfaces" (network cards and adapters) and
  "microcontrollers with security-related functionalities" (the ESP32-S3
  itself, Espressif's matter). So the device is in the **default category:
  Module A** (Art. 32(1)(a)) **(verify when the CRA harmonised standards appear)**.
- **Free and open-source provisions don't apply to the firmware.** The CRA's
  definition requires a licence that grants "all rights to make it freely
  accessible, usable, modifiable and redistributable" (Art. 3(48)); PolyForm
  Noncommercial doesn't. Whether publishing the source counts as making it
  available on the market is covered in §11.

Manufacturer obligations when a unit is placed on the market from 2027-12-11:

| Obligation | Where | Project status |
|---|---|---|
| Meet Annex I Part I; documented cybersecurity risk assessment in the technical documentation | Art. 13(1)–(4), Annex VII | To do ([#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64), #13, #14; §4.4) |
| Due diligence on third-party components (ESP-IDF, NimBLE, TinyUSB, lwIP) | Art. 13(5) | [`THIRD_PARTY.md`](../../THIRD_PARTY.md) records licences; add security tracking |
| Support period of **at least five years**, end date (month and year) shown at purchase | Art. 13(8), 13(19) | To decide **(maintainer)** |
| Each security update kept available for 10 years or the support period, whichever is longer | Art. 13(9) | Release workflow |
| SBOM (at least top-level dependencies, machine-readable) | Annex I Part II(1) | To do (CI) |
| Coordinated vulnerability disclosure policy and contact | Annex I Part II(5)(6), Art. 13(17) | [`SECURITY.md`](../../SECURITY.md) has private reporting; add the support period and advisory publication |
| Free security updates, with advisories | Annex I Part II(2)(4)(8) | Release workflow |
| Report actively exploited vulnerabilities and severe incidents to the coordinating CSIRT and ENISA through the single reporting platform: early warning within 24 h, notification within 72 h, final report within 14 days of a fix (vulnerabilities) or one month (incidents) | Art. 14 | Add to `SECURITY.md` before the first unit is placed on the market |
| User information and instructions (Annex II) | Art. 13(18) | User documentation (issue to be created) |
| EU declaration of conformity and CE marking; technical documentation kept 10 years or the support period | Art. 13(12)–(13), 28, 30 | §10 |

## 6. Substances and waste

### 6.1 RoHS 2011/65/EU

- Applies to the finished product and to cables and spare parts (Art. 4(1)).
  Annex I category 3 (IT and telecommunications) or 11 (other EEE) **(verify)**;
  the obligations are the same. None of the Art. 2(4) exclusions fit: variant M
  isn't a means of transport (2(4)(f)), and it isn't equipment that "can fulfil
  its function only if it is part of" excluded equipment (2(4)(c)).
- Limits per homogeneous material (Annex II): lead, mercury, hexavalent
  chromium, PBB, PBDE, DEHP, BBP, DBP, DIBP 0.1 %; cadmium 0.01 %.
- Manufacturer (Art. 7): Module A, technical documentation, EU declaration
  of conformity, CE marking, type or serial number, name and address.
  Harmonised standard for the documentation: **EN IEC 63000:2018**
  (Implementing Decision (EU) 2020/659). In practice: a supplier RoHS
  declaration or test report per part, kept in the BOM notes.
- JLCPCB lead-free HASL and lead-free assembly
  ([`pcb-fabrication.md`](../requirements/pcb-fabrication.md)) are consistent
  with this; confirm the solder alloy per order **(verify)**.

### 6.2 REACH Art. 33 (SVHC)

- A supplier of an article containing a Candidate List substance above
  0.1 % w/w must give the recipient enough information for safe use, at least
  the substance name; consumers get it on request within 45 days (Art. 33).
- The 0.1 % applies to **each constituent article** (a connector, a cable, the
  enclosure), not to the whole product (CJEU C-106/14).
- Suppliers of such articles must also notify ECHA's SCIP database (Waste
  Framework Directive Art. 9(1)(i), as transposed nationally).
- Practice: collect REACH SVHC declarations for each part, record them in the
  BOM notes, and recheck when ECHA updates the Candidate List.

### 6.3 WEEE 2012/19/EU

- The product is EEE; likely Annex III category 6, small IT and
  telecommunication equipment (no external dimension over 50 cm) **(verify with
  the national registers)**.
- **Marking:** the crossed-out wheeled bin (Annex IX), preferably per
  **EN 50419:2022**, printed visibly, legibly and indelibly on the product (on the
  packaging, instructions and warranty only if size or function require it)
  (Art. 14(4)); plus a mark showing it was placed on the market after
  2005-08-13 (Art. 15(2)), which EN 50419 provides as a bar under the symbol.
- **Producer registration** in **every Member State** where units are sold
  (Art. 16). A seller established in a third country selling by distance
  communication registers through an authorised representative in each of
  those Member States (Art. 3(1)(f)(iv), 16(1), 17).
- Information for private users (Art. 14(2)) and for treatment facilities
  within a year of first placing on the market (Art. 15(1)).

## 7. General product safety and the EU economic operator

- The GPSR applies to CE-marked products only for risks and aspects the sector
  legislation doesn't cover (Art. 2(1)), and its Chapter III Section 1 (Art. 9,
  16 and the other general obligations) doesn't apply to them (Art. 2(1)(b)).
  What still applies: **distance-sales information** in every online offer
  (Art. 19: manufacturer name, postal and electronic address; the EU
  responsible person if the manufacturer is outside the Union; product
  identification with a picture; warnings), **accident notification** through
  the Safety Business Gateway (Art. 20), and the recall and remedy rules
  (Chapters VI and VIII: Safety Gate, recalls and remedies).
- **EU economic operator:** a product under the RED or RoHS may be placed on the
  market only if an economic operator established in the Union (the
  manufacturer, the importer, an authorised representative with a written
  mandate, or a fulfilment service provider) is responsible for the Art. 4(3)
  tasks, and its name and postal address are on the product, packaging, parcel
  or an accompanying document (Regulation (EU) 2019/1020 Art. 4(1)–(5)).
- The maintainer is in the United States. Selling to EU customers directly
  from the US therefore needs an EU authorised representative or an importer,
  plus WEEE authorised representatives (§6.3). **(maintainer)**

## 8. Variant M in vehicles, and the common charger

**UN ECE Regulation No 10.** Paragraph 3.2.9 (05 series as published in
[OJ L 41, 2017](../references/index.md#unece-r10-oj-2017)): aftermarket
components for installation in motor vehicles "need no type-approval if they
are not related to immunity related functions"; the manufacturer then issues a
declaration that the ESA meets the Regulation, in particular the emission limits
of paragraphs 6.5 to 6.9. Whether the current 06 series keeps this paragraph
unchanged is **(verify against Revision 6 on unece.org, which blocks scripted
access)**.

| | Reading A: RED only | Reading B: RED plus an R10 declaration |
|---|---|---|
| Basis | The device isn't a vehicle component under type approval; the RED applies to radio equipment installed in vehicles ([RED guide](../references/index.md#ec-red-guide-2018) §1.6.3.10), and EN 301 489-1/-17 already contain the vehicular clauses (§3.3). ETSI removed its aftermarket-vehicle annex when the EU motor-vehicle EMC Directive ended (EN 301 489-1 V2.2.3 history) | R10 §3.2.9 describes exactly this case: aftermarket equipment with no immunity-related function, self-declared |
| E-mark | None | None: §3.2.9 needs no type approval, so no E-mark |
| Extra work | None beyond §3.3 | Test to the R10 ESA emission limits (CISPR 25 methods **(verify)**) and write the declaration. PR #57 already targets CISPR 25 Class 3, so the extra cost is mainly the report |
| Risk | A vehicle manufacturer, dealer or fleet may ask for R10 evidence | Small |

The device doesn't control the vehicle, so it has no immunity-related function
(R10 §2.12). ISO 7637-2 (#10) is a test method used by both paths; EN 301 489-1
§9.6 uses the 2004 edition at level III, #10 targets the 2011 edition at level IV.

**Common charger ([Directive (EU) 2022/2380](../references/index.md#eu-common-charger-2022-2380), RED Art. 3(4) and Annex Ia).** Applies
to 13 listed categories (phones, tablets, cameras, headphones, headsets,
handheld consoles, portable speakers, e-readers, keyboards, mice, portable
navigation, earbuds, laptops) "in so far as they are capable of being recharged
by means of wired charging". The device has no battery and isn't in the list:
**doesn't apply.** The USB-C receptacle is a design choice, not this rule. If a
power supply is ever sold with the product, check the external power supply
ecodesign rules then **(verify)**.

## 9. Harmonised standards

References as published in the OJ. "Presumption" means compliance with the
standard is presumed to meet the requirement it covers (RED Art. 16).

| Requirement | Standard | OJ reference | Presumption under the RED |
|---|---|---|---|
| RED 3.2 | EN 300 328 V2.2.2 | Implementing Decision (EU) 2022/2191, Annex I row 16 | Yes |
| RED 3.1(b) | EN 301 489-17 V3.3.1 (with EN 301 489-1 V2.2.3) | (EU) 2022/2191 row 169, inserted by (EU) 2025/893; notices on < 9 kHz emissions and clause 6 | Yes, within the notices |
| RED 3.1(b) | EN 55035:2017 | (EU) 2022/2191 row 5 | Yes |
| RED 3.1(b) | EN 55032:2015/A11:2020 (Class B) | (EU) 2019/1326 row 13 (EMC Directive only) | No: other specification |
| RED 3.1(a) | EN IEC 62368-1:2020+A11:2020 | Not cited under the RED (LVD cites EN 62368-1:2014, (EU) 2023/2723 row 555) | No: other specification |
| RED 3.1(a) | EN 62479:2010; EN IEC 62311:2020; EN 50665:2017 | Not cited under the RED (LVD cites EN 62479:2010 and EN 62311:2008, rows 561 and 554) | No: other specification |
| RED 3.3(d) | EN 18031-1:2024 | (EU) 2022/2191 row 164, inserted by (EU) 2025/138; two notices (§4.3) | Yes, within the notices (reading B only; until 2027-12-10) |
| RED 3.3(e) | EN 18031-2:2024 | (EU) 2022/2191 row 165; three notices | Same |
| RoHS | EN IEC 63000:2018 | Implementing Decision (EU) 2020/659 | Yes (RoHS Art. 16(2)) |
| WEEE marking | EN 50419:2022 | Named in WEEE Art. 14(4) and 15(2) | "Preferably" applied |
| CRA | None published yet | — | Check before 2027-12-11 **(verify)** |

Not EU requirements, but project design targets: CISPR 25 Class 3 (variant M,
REQ-EMC-006) and FCC Part 15B Class B (REQ-REG-003).

## 10. Conformity route, declaration and technical documentation

| Act | Procedure | Condition |
|---|---|---|
| RED 3.1(a), 3.1(b) | Module A (Annex II), B+C or H | Any; harmonised standards optional (Art. 17) |
| RED 3.2 (and 3.3 if it applies) | **Module A** only if cited harmonised standards are applied in full; otherwise B+C (notified body) or H | Art. 17 |
| RoHS | Module A | Art. 7(b) |
| CRA (from 2027-12-11) | Module A for the default category | Art. 32(1) |

- **One EU declaration of conformity** for all acts that require one, naming
  each act and its OJ reference (RED Art. 18). Contents per RED Annex VI: product
  and type, manufacturer name and address, the sole-responsibility statement,
  object of the declaration (a picture may help), the acts (Directive
  2014/53/EU, 2011/65/EU; Regulation (EU) 2024/2847 from 2027-12-11), the
  standards with version and date, any notified body, accessories and
  **software version** covered, signature. Translated into the language(s)
  each Member State requires; kept up to date.
- Each unit comes with the full declaration or the **simplified** one (Annex VII:
  "Hereby, [manufacturer] declares that the radio equipment type [type] is in
  compliance with Directive 2014/53/EU" plus the web address of the full text)
  (Art. 10(9)).
- **Technical documentation** (RED Annex V; RoHS Art. 7 with EN IEC 63000; CRA
  Annex VII from 2027-12-11): general description with photos and firmware
  versions affecting compliance; user information; schematics, layout and BOM;
  explanations; the list of standards applied, and the solutions used where they
  aren't; the declaration; calculations and test reports; the Art. 10(2)
  statement (usable in at least one Member State) and the Art. 10(10) packaging
  decision; the risk assessment (safety and cybersecurity); the RoHS evidence.
  Kept **10 years** after the last unit is placed on the market (RED Art. 10(4)),
  or the CRA support period if longer.
- Most design data is public in this repository; the manufacturer's file adds
  the test reports, production records and the signed declaration.

## 11. Who carries the manufacturer's obligations

Facts from the definitions; how they apply to a given arrangement is for the
people involved to decide, with advice where needed.

- **Only the one who places units on the market carries the obligations.**
  Placing on the market is the first making available, and making available
  is supply "in the course of a commercial activity, whether in return for
  payment or free of charge" (RED Art. 2(1)(9)–(10); CRA Art. 3(21)–(22)). A
  product is not placed on the market when it is "manufactured for one's own
  use", unless the act covers own use, and even then not "occasional
  manufacturing for own use by a private person in a non-commercial context"
  ([Blue Guide](../references/index.md#eu-blue-guide-2022) §2.3 and note 49).
  "Commercial activity is understood as providing goods in a business related
  context"; non-profit organisations can be in one (Blue Guide §2.2).
- **The manufacturer** is whoever manufactures the equipment, or has it
  designed or manufactured, "and markets that equipment under his name or
  trade mark" (RED Art. 2(1)(12); similar in CRA Art. 3(13) and GPSR Art. 3(8)).
  An importer or distributor that sells under its own name, or modifies a unit
  so compliance may be affected, becomes the manufacturer (RED Art. 14;
  CRA Art. 21).

| Situation | Who holds the obligations |
|---|---|
| A hobbyist builds one unit for their own station | Not placed on the market: no declaration, CE marking, WEEE registration or CRA duties (Blue Guide §2.3, note 49). RED Annex I(1)(c) may also exclude it (§3.5). The user still must not cause harmful interference, and national rules on radio use apply |
| Amateurs pass a unit on occasionally | "May be considered as not making available on the market"; regular supply or a business context may be ([RED guide](../references/index.md#ec-red-guide-2018) §1.6.2.2). Case by case |
| A club runs an at-cost group build | Case by case: a non-profit can be in a commercial activity (Blue Guide §2.2). Under the CRA, "accepting donations without the intention of making a profit" isn't commercial (CRA recital 15). [`COMMERCIAL.md`](../../COMMERCIAL.md) already asks such groups to ask first |
| A licensee under [`COMMERCIAL.md`](../../COMMERCIAL.md) builds and sells units under its own name | The licensee is the manufacturer: conformity assessment, declaration, CE and WEEE marking, WEEE registration, CRA duties, EU economic operator if outside the Union. The licence is a copyright licence; it doesn't move regulatory obligations to the project |
| A licensee sells kits | The kit maker is responsible for compliance when the kit is assembled per the instructions; an assembler who sells assembled kits becomes the manufacturer if the function changes or the instructions weren't followed ([RED guide](../references/index.md#ec-red-guide-2018) §1.6.3.8) |
| The maintainer sells units | The maintainer is the manufacturer, and needs an EU economic operator (§7) |
| The maintainer publishes the design and firmware source | Publishing files isn't supplying a product; "the sole act of hosting products with digital elements on open repositories" isn't making available (CRA recital 20). **Open question:** under the CRA, software is itself a product with digital elements (Art. 3(1)). If the firmware is supplied to licensees for payment, the firmware may be "made available on the market" by the maintainer as a component, with CRA manufacturer duties for it **(maintainer; get advice)** |

## 12. Marking and user information

On the product (or, where the RED allows, on the packaging or documents):

- [ ] **CE marking**, visible, legible and indelible, on the product or its
      data plate **and on the packaging**; before placing on the market (RED
      Art. 20). At least 5 mm high, but a radio product may use less if it stays
      legible (RED Art. 19; Regulation (EC) 765/2008 Annex II). No notified-body
      number with Module A.
- [ ] **WEEE crossed-out bin** with the date bar (EN 50419:2022) (§6.3).
- [ ] **Type, batch or serial number** (RED Art. 10(6); RoHS Art. 7(g); CRA Art. 13(15)).
- [ ] **Manufacturer** name or trade mark and a single postal address; CRA adds
      an email or other digital contact and a website (RED Art. 10(7); CRA Art. 13(16)).
- [ ] **Importer** name and address where there is one (RED Art. 12(3)), and the
      **EU economic operator** (2019/1020 Art. 4(4)).
- [ ] Keep label space for these next to the FCC label ([`fcc.md`](fcc.md) §2).

With the product, in a language easily understood by users as each Member
State determines (RED Art. 10(8); CRA Art. 13(18)):

- [ ] Instructions and safety information, including accessories and
      **software** needed to operate as intended.
- [ ] **Frequency band and maximum RF power**: "2402–2480 MHz, maximum
      [x] dBm e.i.r.p." (RED Art. 10(8)(a)(b)).
- [ ] Full or simplified EU declaration of conformity (§10).
- [ ] WEEE information for private users (WEEE Art. 14(2)).
- [ ] REACH Art. 33 information if any article holds an SVHC (§6.2).
- [ ] From 2027-12-11: CRA Annex II information, including the vulnerability
      contact, secure-use instructions, how to install updates, and the
      **support-period end date** (also shown at purchase, Art. 13(19)).
- [ ] Online listings: the GPSR Art. 19 information (§7).

## 13. Design-affecting rules (checklist)

- [ ] **BLE TX power:** one setting for all markets, no higher than the
      module's CE-tested 9.96 dBm e.i.r.p. and the FCC's 10.3 dBm conducted;
      firmware test (REQ-REG-008, #14).
- [ ] **Antenna:** only the module's PCB antenna, per the integration guide
      (keeps the module test results relevant) (REQ-REG-002).
- [ ] **EMC:** design both variants to EN 301 489-1/-17 and EN 55032 Class B
      emissions and EN 55035 immunity; variant M, and R if declared for vehicle
      use, to the vehicular clauses (REQ-EMC-008, -009; #10, #11).
- [ ] **Safety:** assess to EN IEC 62368-1 including foreseeable faults (load
      dump, reverse polarity, shorted jacks); enclosure material and temperature (#10, #11, #19).
- [ ] **RoHS and REACH:** every part RoHS-compliant, RoHS and REACH SVHC status
      recorded in the BOM notes (REQ-REG-009; #8, #9, #10, #11).
- [ ] **Labels:** space and artwork for CE, WEEE, type/serial, manufacturer and
      EU operator address on the enclosure and packaging (REQ-REG-010; #19).
- [ ] **Security by design:** §4.4 for #13 and #14; signed, user-installable
      updates; factory reset; SBOM in CI (REQ-REG-012, -013).
- [ ] **Support period** of at least five years and a vulnerability-reporting
      process that meets CRA Art. 14 (REQ-REG-013; [`SECURITY.md`](../../SECURITY.md)).
- [ ] **Security level (decided):** CRA level now; EN 18031-1 gap analysis and
      cost in [#64](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/64) (§4.2).
- [ ] **Decisions for the maintainer:** whether R is declared for vehicle use (§3.3); the R10 reading (§8); the EU
      economic operator (§7); the firmware's CRA status under commercial licences (§11).

## 14. Not covered

UKCA (Great Britain), Switzerland, ISED (Canada) and other markets are outside
this issue. Testing
and certification are later `human-task` issues.

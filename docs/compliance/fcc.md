<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# FCC: module integration, host labeling and Part 15B SDoC

Issue: [#7](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/7).
Written 2026-09-24 for revision A, which uses the **ESP32-S3-MINI-1-N8**
([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)). The grant review
and sourcing are in [`module-selection.md`](../research/module-selection.md).

| Item | Value |
|---|---|
| Module | Espressif ESP32-S3-MINI-1-N8 (on-board PCB antenna) |
| FCC ID | **2AC7Z-ESPS3MINI1**: single modular approval, DTS, Part 15C, granted 2022-02-28 ([grant](../references/index.md#esp32s3-mini1-fcc-grant)) |
| Module manual | FCC user-manual exhibit v0.6 ([mirror](../references/index.md#esp32s3-mini1-fcc-manual); FCC [filing list](../references/index.md#fcc-2ac7z-esps3mini1-filings)); "module manual" below |
| ISED | 21098-ESPS3MINI1 ([certificate](../references/index.md#esp32s3-mini1-ised-cert)); optional, later |
| End product | Part 15 Subpart B unintentional radiator, **Class B**, authorized by Supplier's Declaration of Conformity (SDoC) ([`constraints.md`](../requirements/constraints.md) §4) |

The module's grant covers the transmitter only when it is integrated as the
grantee describes. The grant tells the integrator to check the final product
against the FCC rules, including the transmitter's operation, using
[KDB 996369](../references/index.md#fcc-kdb-996369). Every rule below marked
**must** comes from the grant, the module documents or the eCFR. Rules marked
*project rule* are this project's reading of them.

## 1. Integration constraints

### 1.1 Placement and antenna keep-out

From the [ESP32-S3 hardware design guidelines](../references/index.md#esp32s3-hw-design)
§"General Principles of PCB Layout for Modules" (Figs. 24–26) and the
[module datasheet](../references/index.md#esp32s3-mini1-ds) §3.1 and §11:

- **Preferred:** the module sits at a board edge with its PCB antenna
  **outside** the base board, and the antenna feed point near the edge
  (positions marked ✓ in Figs. 24 and 25).
- **If the antenna can't overhang:** keep the feed point as close to the edge
  as possible and **cut the base board away** below the antenna and on both
  sides of it. Don't place the module in the middle of the board with a
  clearance cut out on all four sides.
- **Keep-out dimensions (Fig. 26, mm):** the antenna region is 6 deep; the
  clearance area covers it and extends **at least 15** past the antenna,
  along the board edge, on the side away from the module's corner; the
  module's antenna end sits at most 1 from the board edge and its side at most
  2 from the side edge. The figure shows the clearance area cut out of the
  board. Draw the footprint's keep-out from the figure itself.
- *Project rule:* the keep-out is free of copper, tracks, vias and parts on
  **all layers** (as [`pcb-fabrication.md`](../requirements/pcb-fabrication.md)
  §2 already says), and free of metal hardware.
- Place **plenty of ground copper and dense ground vias** on the base board
  next to the antenna area.
- The recommended land pattern is datasheet Fig. 11-1 (15.4 × 20.5 mm, 60
  pads 0.4 × 0.8 mm, 4 corner pads 0.8 × 0.8 mm, and the "Antenna Area").
  Soldering the EPAD to ground is optional but helps thermally; too much paste
  on it can lift the module.

### 1.2 Enclosure

- Keep **at least 15 mm** of clearance around the PCB antenna **in all
  directions** inside the housing (hardware design guidelines). This sets a
  floor on the enclosure size near the module.
- *Project rule:* no metal over or near the antenna: no metal-filled or
  carbon-filled filament, conductive paint or metal labels there. PETG and ASA
  are fine ([`constraints.md`](../requirements/constraints.md) §11).
- **Test throughput and range in the final enclosure** (hardware design
  guidelines). For this device that means the BLE audio stream (#17).

### 1.3 RF configuration (firmware)

- **Must:** BLE TX power no higher than the certified **0.0107 W (10.3 dBm)
  conducted**. The chip can reach +20 dBm, which the grant doesn't cover.
  Add a firmware test that the configured level stays at or below the cap.
- **Must:** mobile use only, with **at least 20 cm** between the antenna and
  people (grant note, module manual). Portable (body-worn or handheld) use
  needs a separate approval (grant note, 47 CFR 2.1093).
- **Must:** no other transmitter operating at the same time in the product
  (module manual). Wi-Fi is not used and stays off.
- **Must:** only the module's own PCB antenna. Different antenna
  configurations need a separate approval (grant note).
- Keep the UART0 download interface: Espressif's RF test firmware supports only
  UART (hardware design guidelines, "Download Guidelines"). An EMC lab may need it.

### 1.4 Power and reset

From the hardware design guidelines §1.3.2–1.3.3 and the datasheet §4.5 and §9:

- 3.3 V supply (3.0–3.6 V) able to deliver **at least 500 mA**.
- **ESD protection diode and at least 10 µF** at the main power entrance.
  The datasheet reference circuit puts 22 µF and 0.1 µF on the module's 3V3 pin.
- **EN** must not float. Use an RC delay, usually **10 kΩ / 1 µF**, tuned to the
  supply's ramp. The rails must be stable for at least 50 µs before EN goes high,
  and EN must be held low at least 50 µs to reset.
- *Project rule:* the window watchdog (TPS3430, [`core-devices.md`](../research/core-devices.md) §3)
  can drive EN or hold PTT off, but PTT safety must not depend on the module
  booting ([`constraints.md`](../requirements/constraints.md) §6).

### 1.5 USB and programming

- USB D− is **GPIO19** and D+ is **GPIO20**. Reserve series resistors (22/33 Ω
  to start) and footprints for capacitors to ground (unpopulated at first),
  close to the module. Route a 90 Ω ±10 % differential pair over a continuous
  ground reference, with few vias (hardware design guidelines §1.3.13, §1.4.8).
- **USB_D+ toggles during power-up.** Add an external pull-up if a stable
  initial level is needed. *Project rule:* the USB switches (TS3USB221A,
  ADR-0008) stay in a safe position until the firmware selects a mode.
- **Shared PHY:** the USB OTG controller and the USB Serial/JTAG controller use
  the same pins. USB download stops working when the application uses the USB
  port as a host or standard device, which this design always does. So
  **keep a UART0 header (TXD0/RXD0) and access to GPIO0 and GPIO46**
  (hardware design guidelines, "Download Guidelines"). A 499 Ω series resistor
  on U0TXD is recommended.

### 1.6 Strapping pins

Sampled at reset and held for at least 3 ms after EN goes high (datasheet §4):

| Pin | Default | Function | Design rule |
|---|---|---|---|
| GPIO0 | Weak pull-up (1) | Boot mode: 1 = SPI boot; 0 with GPIO46 = 0 = download | External pull-up recommended; no large capacitor on it, or the chip may boot into download mode |
| GPIO46 | Weak pull-down (0) | Boot mode (with GPIO0); ROM message printing | Leave at the default |
| GPIO45 | Weak pull-down (0) | VDD_SPI voltage: 0 = 3.3 V (default), 1 = 1.8 V | Keep low at reset |
| GPIO3 | Floating | JTAG source, only if the related eFuses are burnt | Give it a defined level if used |

*Project rule:* don't connect PTT, relay or mode-switch drive signals to
strapping pins, or to pins with power-up glitches (hardware design guidelines,
"Power-Up Glitches on Pins"). Their state during reset isn't under firmware
control. On -N4R2, IO26 is taken by the PSRAM.

## 2. Host label

On the outside of the enclosure, because the module's own label isn't visible
once it is installed ([47 CFR 15.212(a)(1)(vi)(A)](../references/index.md#ecfr-47-15-212), module manual):

```text
Contains FCC ID: 2AC7Z-ESPS3MINI1
```

The product must also carry the Part 15 statement in a conspicuous place
([47 CFR 15.19(a)(3)](../references/index.md#ecfr-47-15-19)):

```text
This device complies with part 15 of the FCC Rules. Operation is subject to the
following two conditions: (1) This device may not cause harmful interference,
and (2) this device must accept any interference received, including
interference that may cause undesired operation.
```

- If the device is too small for the statement in 4-point type and has no
  display, the statement goes in the user manual **and** on the packaging or a
  removable label (15.19(a)(5)).
- **Product identification** (SDoC): a unique model name that can't be mistaken
  for an FCC ID ([47 CFR 2.1074](../references/index.md#ecfr-47-2-1074)). The
  FCC logo is optional.
- If the product is later sold in Canada: `Contains IC: 21098-ESPS3MINI1`
  (module manual).
- The enclosure keeps space for this label
  ([`constraints.md`](../requirements/constraints.md) §11).
- [KDB 784748](../references/index.md#fcc-kdb-784748) is the FCC's labeling
  guidance. It wasn't readable by the tools used here, so check the final label
  against it **(verify)**.

## 3. User manual statements

- The Part 15 statement above (15.19(a)(3)).
- The warning that changes or modifications not expressly approved by the party
  responsible for compliance could void the user's authority to operate the
  equipment ([47 CFR 15.21](../references/index.md#ecfr-47-15-21)).
- The Class B statement of [47 CFR 15.105(b)](../references/index.md#ecfr-47-15-105),
  in a prominent place:

  ```text
  Note: This equipment has been tested and found to comply with the limits for a
  Class B digital device, pursuant to part 15 of the FCC Rules. These limits are
  designed to provide reasonable protection against harmful interference in a
  residential installation. This equipment generates, uses and can radiate radio
  frequency energy and, if not installed and used in accordance with the
  instructions, may cause harmful interference to radio communications. However,
  there is no guarantee that interference will not occur in a particular
  installation. If this equipment does cause harmful interference to radio or
  television reception, which can be determined by turning the equipment off and
  on, the user is encouraged to try to correct the interference by one or more of
  the following measures:
  —Reorient or relocate the receiving antenna.
  —Increase the separation between the equipment and receiver.
  —Connect the equipment into an outlet on a circuit different from that to which
  the receiver is connected.
  —Consult the dealer or an experienced radio/TV technician for help.
  ```

- The RF exposure condition: keep the device at least 20 cm from people
  (grant, module manual).
- The **compliance information statement** ([47 CFR 2.1077(a)](../references/index.md#ecfr-47-2-1077)):
  product name and model; the 15.19(a)(3) compliance statement; and the name,
  address and phone number or web contact of the **responsible party, who must
  be in the United States**. It can be in the manual or on a separate sheet,
  and an online manual is allowed (2.1077(c)).

## 4. SDoC checklist

Before the product is marketed ([47 CFR 15.101](../references/index.md#ecfr-47-15-101)):

- [ ] Name the **responsible party** (in the US) for the SDoC.
- [ ] **Radiated emissions** to the Class B limits at 3 m
      ([47 CFR 15.109(a)](../references/index.md#ecfr-47-15-109)), with the
      module installed and working (KDB 996369 grant note).
- [ ] **AC conducted emissions** to the Class B limits
      ([47 CFR 15.107(a)](../references/index.md#ecfr-47-15-107)), if the
      product is marketed with, or is designed to be powered through, an AC
      supply (for example a USB-C charger) **(verify the test setup with the lab)**.
- [ ] Test in every operating mode: Bluetooth mode (BLE audio streaming),
      wired USB-C mode, radio USB host active, both variants (R and M).
- [ ] Confirm the host doesn't break the module's grant: BLE power cap,
      only the PCB antenna, keep-out, no co-located transmitter (§1).
- [ ] Label (§2) and user manual statements (§3) in place.
- [ ] Keep the records required by [47 CFR 2.938](../references/index.md#ecfr-47-2-938):
      design drawings and changes, production test procedure, the test report
      (dates, lab, method, equipment under test and support equipment, cables,
      set-up photos of the highest radiated and conducted emissions, any
      modifications, signatures), a copy of the compliance information, and the
      signed Covered List statement of 2.938(b)(2).
- [ ] Store pre-scan and final results in `docs/compliance/`.
- [ ] Variant M also targets CISPR 25 for the vehicle environment
      ([`constraints.md`](../requirements/constraints.md) §3.2). That is separate
      from the FCC SDoC.

Emissions testing, labels and the SDoC itself need a person, a lab and a
responsible party. They are `human-task` work.

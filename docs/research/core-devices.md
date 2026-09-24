<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Core device shortlist (revision A)

Issue: [#42](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/42).
Researched 2026-09-24.

This is the **shortlist of key parts** for revision A: one primary candidate
and alternates per functional block. It is a starting point for the block
issues, **not a decision**. Each block's ADR makes the final choice, with dated
price and stock from at least two distributors, as required by
[`AGENTS.md`](../../AGENTS.md#parts-and-sourcing).

Facts marked **(verify)** come from memory or secondary sources and must be
confirmed from the manufacturer's datasheet by the issue named in the
"Settled by" column before a design depends on them.

## Summary

| Block | Primary candidate | Alternates | Settled by |
|---|---|---|---|
| Bluetooth radio + MCU | Espressif **ESP32-WROOM-32E** (`-32UE` for an external antenna) | ESP32-WROVER-E (adds PSRAM); ESP32-S31 modules (watch list) | #7 |
| Audio codec | TI **TLV320AIC3104** (`-Q1` for variant M) | TI TLV320AIC3204, NXP SGTL5000, Nuvoton NAU88C22, Cirrus WM8960 | #8 |
| Audio isolation | Bourns **LM-NP-1001-B1L** (600:600 Ω) | Bourns LM-LP-1001 (SMD), Triad / Xicon 600:600 Ω line transformers | #8 |
| RS-232 CAT + RTS/DTR | TI **TRS3232E** | ADI MAX3232E, MaxLinear SP3232E | #9 |
| TTL CAT level shift | TI **SN74LXC1T45** / TXU0102 | Nexperia 74LVC1T45 | #9 |
| CI-V bus | Open-drain buffer **74LVC1G07** + pull-up | Discrete NPN/N-MOSFET | #9 |
| CAT isolation | TI **ISO7721** | Skyworks Si8621, ADI ADuM1201 | #9 |
| PTT closure | Panasonic **AQY212EH** PhotoMOS | Littelfuse/IXYS CPC1017N, Toshiba TLP-series photorelay | #9 |
| USB host (radio's USB port) | ADI **MAX3421E** (SPI) + TinyUSB | Small USB-host coprocessor (e.g. RP2040/RP2350 + TinyUSB) | #9 |
| VBUS supply to the radio | TI **TPS2553** (`-Q1` for M) | Diodes AP22653, TI TPS2051C | #9, #11 |
| Supervisor / watchdog | TI **TPS3430** window watchdog | TI TPS3840 (reset only) | #9, #14 |
| Power R: source mux | TI **TPS2121** | Two ideal-diode controllers (LM66100-class) | #11 |
| Power R: buck | TI **TPS62933** | TI LMR51430, ADI LT8609S | #11 |
| Power M: reverse polarity | TI **LM74700-Q1** ideal diode | TI LM74800-Q1 (adds load-dump cut-off) | #10 |
| Power M: load dump / surge | ADI **LTC4380** surge stopper | TI LM5060-Q1, TVS only (SM8S-class) | #10 |
| Power M: buck | TI **LMQ62440-Q1** | ADI LT8609S, ADI LT8636 | #10 |
| Low-noise LDO (audio rail) | TI **TPS7A20** | TI LP5907 | #8, #10, #11 |
| USB-C sink (R) | Receptacle + 5.1 kΩ Rd on CC1/CC2 | — | #11 |
| ESD (USB and radio lines) | TI **TPD4E05U06** / TPD1E10B06 | ST USBLC6-2 | #9 |

## 1. Bluetooth radio and MCU (#7)

The constraints require **Classic BR/EDR (SPP + HFP with mSBC) and BLE at the
same time**, an FCC modular grant, and a firmware SDK under a permissive license
with no NDA ([`constraints.md`](../requirements/constraints.md) §2, §4, §10).
That rules out nearly every dual-mode part on the market.

| Candidate | Result | Reason (source) |
|---|---|---|
| **Espressif ESP32-WROOM-32E / -32UE** (original ESP32) | **Passes every hard check found so far** | Dual mode, SPP + HFP hands-free + BLE in ESP-IDF (Apache-2.0). mSBC works only on the **HCI data path** (codec in the Bluedroid host, audio out over I2S), not the PCM path ([ESP-IDF `hfp_hf` example](https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/classic_bt/hfp_hf/README.md)). FCC ID **2AC7Z-ESP32WROOM32E** ([fccid.io](https://fccid.io/2AC7Z-ESP32WROOM32E)). −40 to +85 °C (`-H` codes to +105 °C), mass production, datasheet v2.1 of 2026-08-05 ([datasheet](https://documentation.espressif.com/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.html)). **No USB**, so the USB-host block needs its own chip (§5). |
| Espressif ESP32-S31-WROOM-1 / -3 | **Watch list** | Dual mode (Classic + BLE 5.4), **USB 2.0 HS OTG** (host capable), −40 to +85 °C. HFP is HCI-only on this chip, which mSBC needs anyway. The datasheet is **pre-release v0.5** and lists no FCC ID ([datasheet](https://documentation.espressif.com/esp32-s31-wroom-1_wroom-1u_datasheet_en.html)). If it reaches mass production with a modular grant before layout, it would remove the MAX3421E. |
| Raspberry Pi RM2 (CYW43439) + RP2350 | **Fails: HFP** and variant M temperature | SCO links don't come up on CYW43439 over its gSPI transport; the issue is still open ([pico-sdk#1461](https://github.com/raspberrypi/pico-sdk/issues/1461), last update 2026-02). −30 to +70 °C only ([RM2 docs](https://www.raspberrypi.com/documentation/microcontrollers/radio-modules.html)). BTstack is free here only under Raspberry Pi's product-restricted licence (`pico-sdk/src/rp2_common/pico_btstack/LICENSE.RP`), not a permissive one. |
| Infineon CYBT-243053-02 (CYW20820) | **Fails: SDK license** (as written) | The BTSDK is under the **Cypress End User License Agreement** (`Infineon/wiced_btsdk/LICENSE.txt`), not a permissive license. The 20820 SDK repositories have seen little activity since 2024 (lifecycle risk). Would need a maintainer exception to §10. |
| Microchip BM83 (IS2083) | **Fails: SPP + audio at once** | Microchip support states audio and SPP cannot run at the same time because both are Classic ([Microchip KB](https://support.microchip.com/s/article/BM83---BLE-and-SPP-Simultaneous-Connection)). The firmware is configurable, not programmable. |
| BTstack on any other controller | **Fails: license** | BTstack is free for non-commercial use only; commercial use needs a paid licence from BlueKitchen. |

**Consequences for the design if ESP32 is chosen:**

- **Single silicon vendor.** "Two sources" means the same Espressif module from
  at least two distributors, plus the module family (WROOM-32E, WROOM-32UE,
  WROVER-E) as drop-in alternates. The ADR must accept this explicitly.
- mSBC is encoded and decoded in software, so the audio chip connects to the
  ESP32 **I2S** peripheral, not the controller's PCM pins.
- The 3D-printed enclosure suits the PCB antenna (`-32E`). Use `-32UE` only
  with an antenna covered by Espressif's grant **(verify)**.
- Wi-Fi comes for free; it is not a requirement (possible OTA/config side benefit).

## 2. Audio codec and isolation (#8)

Requirements: line-level in and out, 48 kHz, ADC SNR ≥ 90 dB(A), AGC and all
voice processing disabled, I2S, ±50 ppm clock, −40 to +85 °C for variant M.

| Part | Why it's on the list | Notes |
|---|---|---|
| **TI TLV320AIC3104** | Mature, widely stocked; AGC can be bypassed (PGA mode); has an AEC-Q100 **-Q1** variant for M ([TI](https://www.ti.com/product/TLV320AIC3104-Q1)). | ADC SNR about 92 dB(A) **(verify)**. Built-in PLL lets it run from the ESP32 MCLK or its own crystal. |
| TI TLV320AIC3204 | Better converters, 93 dB ADC SNR with AGC off ([datasheet](https://www.ti.com/lit/ds/symlink/tlv320aic3204.pdf)). | No automotive grade **(verify)**; −40 to +85 °C. |
| NXP SGTL5000 | Cheap, well known, many open drivers. | ADC SNR about 85 dB **(verify)**, likely below the 90 dB target. |
| Nuvoton NAU88C22 | Good specs, low cost. | Distributor depth **(verify)**. |
| Cirrus WM8960 | Common in hobby designs. | Lifecycle status **(verify)**. |

Everest ES8388 was dropped from the list: datasheet quality and authorized
distributor availability outside China are weak.

**Isolation transformers:** Bourns LM-NP-1001-B1L, 600:600 Ω, 200–3500 Hz,
insertion loss ≤ 1.5 dB ([Bourns LM-NP/LP datasheet](https://www.bourns.com/pdfs/LMNPLP.pdf)).
Its 200–3500 Hz band matches the requirement, but whether it is flat to
**±1 dB** across that band must be checked against the datasheet curve. The
SMD sibling (LM-LP-1001) suits assembly houses better.

## 3. CAT, CI-V, RTS/DTR and PTT (#9)

- **RS-232:** TRS3232E / MAX3232E / SP3232E are pin-compatible second sources
  (2 drivers, 2 receivers). Two drivers cover TXD + one of RTS/DTR; RTS **and**
  DTR plus TXD needs a 3-driver part (e.g. MAX3243E-class) **(verify)**.
- **TTL 3.3/5 V:** a direction-controlled level translator (SN74LXC1T45 or
  74LVC1T45) per line, 5 V tolerant.
- **CI-V:** single wire, open collector. 74LVC1G07 drives it low; the radio side
  sets the pull-up voltage. Echo and collision handling is a firmware matter.
- **Isolation (variant M, separately powered R):** ISO7721 (one channel each
  way, ≥ 115.2 kBd). The radio-side supply of the isolator is an open question
  for #9: a small isolated converter, or power taken from the radio's port.
- **PTT:** a PhotoMOS (AQY212EH, 60 V) gives an isolated closure to ground and
  is **off unless its LED is driven**, which meets the hardware default-off
  rule. Drive it through a gate that a window watchdog (TPS3430) can hold off,
  so a hung MCU cannot key the radio.

## 4. USB host for radios with a built-in USB port (#9)

The ESP32 has no USB, so revision A needs a host controller:

- **MAX3421E** (SPI) with **TinyUSB** (MIT). TinyUSB has a MAX3421E host driver
  and host class drivers for CDC-ACM, FTDI and CP210x
  ([TinyUSB docs](https://docs.tinyusb.org/en/latest/)). Open points: dual-port
  **CP2105** support (FT-891/FT-991A), RTS/DTR control requests per chip, and
  whether TinyUSB's MAX3421E driver runs under ESP-IDF on the original ESP32
  **(verify)**. MAX3421E lifecycle: at least one ordering code
  (`MAX3421EEHJ+T`) is listed as no longer manufactured; check the ADI product
  page for the active codes **(verify)**.
- **Alternative:** a small USB-host coprocessor (RP2040/RP2350, native host via
  TinyUSB) linked to the ESP32 over UART. More parts, but better lifecycle and
  more USB headroom.
- If ESP32-S31 becomes viable (§1), its OTG port replaces both.
- **VBUS to the radio:** TPS2553 (adjustable current limit, fault flag), sized
  from the per-radio draw in #5.

## 5. Power (#10, #11)

Shared rule: the switching frequency and its harmonics must stay out of the HF
amateur bands. For example, a 2.2 MHz converter's 13th harmonic is 28.6 MHz, in
the 10 m band. Each power ADR must include this harmonic check.

**Variant R** (11–15 V radio accessory DC or USB-C 5 V):

- USB-C: receptacle with 5.1 kΩ Rd on CC1 and CC2 (5 V default current, no PD).
- Source selection: TPS2121 power mux (priority to radio DC, no back-feed).
- Regulation: TPS62933 buck to 3.3 V, then TPS7A20 LDO for the codec's analog rail.
- Reverse polarity and TVS on the DC input; parts chosen in #11.

**Variant M** (automotive 12 V, ISO 16750-2 / ISO 7637-2):

- Fuse → LM74700-Q1 ideal diode (reverse battery) → LTC4380 surge stopper
  (load dump, jump start) → CM choke + pi filter → LMQ62440-Q1 buck → LDOs.
- All AEC-Q100/Q101 where practical; TVS sizing per the pulse energies in #10.
- Cold crank to 6 V: the 3.3 V rail survives with a low-dropout buck, but the
  5 V VBUS to the radio may need a buck-boost or must drop out safely with PTT off.

## 6. What this shortlist changes in the open issues

- **#7:** the realistic field is ESP32 only (plus ESP32-S31 as a watch item). The
  ADR should record the single-vendor risk and the rejected options above.
- **#8:** the codec connects over **I2S** to the ESP32, because mSBC needs the
  HCI data path.
- **#9:** a separate USB host controller (MAX3421E or a coprocessor) is needed
  unless ESP32-S31 becomes available.
- **#10 / #11:** use the same buck family in both variants where the input range
  allows, to share layout and the HF harmonic analysis.

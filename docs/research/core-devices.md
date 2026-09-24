<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Core device shortlist (revision A)

Issue: [#42](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/42).
Researched 2026-09-24. Host link and module decision:
[ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md) (Bluetooth LE only,
ESP32-S3-MINI-1, analog **and** USB radio audio).

This is the **shortlist of key parts** for revision A: one primary candidate
and alternates per functional block. It is a starting point for the block
issues, **not a decision**. Each block's ADR makes the final choice, with dated
price and stock from at least two distributors, as required by
[`AGENTS.md`](../../AGENTS.md#parts-and-sourcing).

Datasheets and other sources for every part named here are listed in the
[reference index](../references/index.md), with local copies in the
gitignored reference cache (`python3 tools/refs/refs.py fetch`).

Facts marked **(verify)** come from memory or secondary sources and must be
confirmed from the manufacturer's datasheet by the issue named in the
"Settled by" column before a design depends on them.

## Summary

| Block | Primary candidate | Alternates | Settled by |
|---|---|---|---|
| Bluetooth LE radio + MCU | Espressif **ESP32-S3-MINI-1-N8** (`-1U` for an external antenna) | ESP32-S3-MINI-1-N4R2 (adds PSRAM); Raytac MDBT50Q (nRF52840) as the non-Espressif fallback | #7, ADR-0008 |
| Audio codec | TI **TLV320AIC3104** (`-Q1` optional for variant M) | TI TAC5112, TI TLV320AIC3204 (WM8960 is end of life) | #8, [ADR-0002](../decisions/ADR-0002-audio-codec.md) |
| Audio isolation | Bourns **SM-LP-5001** (600:600 Ω, SMD) | Triad TY-250P (LM-NP-1001-B1L rejected: −10 to +60 °C) | #8, [ADR-0002](../decisions/ADR-0002-audio-codec.md) |
| RS-232 CAT (1 driver, 1 receiver; drivers high-Z when off) | TI **TRS3221E** | MaxLinear SP3221E, ADI ADM3101E | #9 |
| SERIAL-jack mode switching (RS-232 tolerant) | TI **TMUX6219** (36 V SPDT), one per contact | Small signal relays | #9 |
| TTL CAT level shift (behind the mode switch) | TI **SN74LXC1T45** / TXU0102 | Nexperia 74LVC1T45 | #9 |
| CI-V bus | Open-drain buffer **74LVC1G07** + pull-up | Discrete NPN/N-MOSFET | #9 |
| CAT isolation | TI **ISO7721** | Skyworks Si8621, ADI ADuM1201 | #9 |
| PTT closure | Panasonic **AQY212EH** PhotoMOS | Littelfuse/IXYS CPC1017N, Toshiba TLP-series photorelay | #9 |
| USB host (radio's USB sound card + USB-serial chip) | **ESP32-S3 built-in USB OTG** + `espressif/esp-usb` | MAX3421E (SPI), only if the fallback module is used | #9 |
| USB hub (wired mode) | Microchip **USB2422** (2-port) | Genesys GL850G, WCH CH334 | #9 |
| USB routing switches (×2) | TI **TS3USB221A** | onsemi FSUSB42 | #9 |
| USB-C port | HRO **TYPE-C-31-M-12** + 5.1 kΩ Rd, ESD TPD2E2U06 | ST USBLC6-2 | #9, #11 |
| Radio USB isolator (fitting option, variant M) | ADI **ADuM4160** (full speed) | ADI ADuM3160 | #9, #10 |
| VBUS supply to the radio | TI **TPS2553** (`-Q1` for M) | Diodes AP22653, TI TPS2051C | #9, #11 |
| Supervisor / watchdog | TI **TPS3430** window watchdog | TI TPS3840 (reset only) | #9, #14 |
| Power R: source mux | TI **TPS2121** | Two ideal-diode controllers (LM66100-class) | #11 |
| Power R: buck | TI **TPS62933** | TI LMR51430, ADI LT8609S | #11 |
| Power M: reverse polarity | TI **LM74700-Q1** ideal diode | TI LM74800-Q1 (adds load-dump cut-off) | #10 |
| Power M: load dump / surge | ADI **LTC4380** surge stopper | TI LM5060-Q1, TVS only (SM8S-class) | #10 |
| Power M: buck | TI **LMQ62440-Q1** | ADI LT8609S, ADI LT8636 | #10 |
| Low-noise LDO (audio rail) | TI **TPS7A20** | TI LP5907 | #8, #10, #11 |

| Programming/log port | UART0 header (USB is used as host to the radio) | — | #21 |
| ESD (USB and radio lines) | TI **TPD4E05U06** / TPD1E10B06 | ST USBLC6-2 | #9 |

## 1. Bluetooth LE radio and MCU (#7, ADR-0008)

### 1.1 Bluetooth LE modules (modules only)

Prices and stock: LCSC, 2026-09-24, USD at quantity 100. Nordic, Silicon Labs
and u-blox modules sell mainly through Digi-Key and Mouser, which were not
checked here, so their LCSC stock understates availability. #7 must add a dated
second-distributor check.

| Module | Chip | $ @100 | LCSC stock | FCC ID | I2S | USB host |
|---|---|---|---|---|---|---|
| Ai-Thinker TB-03F | Telink TLSR8253 | 1.35 | 301 | not found | ? | ✗ |
| Ai-Thinker Ai-WB2 | Bouffalo BL602 | 1.47–1.54 | 45–469 | claimed, ID not found | ? | ✗ |
| WCH BLE-SER-A-ANT | WCH CH58x | 1.88 | 232 | not found | ? | ✗ |
| ESP32-C3-MINI-1 | ESP32-C3 | 2.37–2.92 | 1,341–14,778 | [2AC7Z-ESPC3MINI1](../references/index.md#fcc-2ac7z-espc3mini1) | ✓ | ✗ |
| ESP32-C6-MINI-1-N4 | ESP32-C6 | 3.00 | 1,143 | not checked | ✓ | ✗ |
| **ESP32-S3-MINI-1-N8** | ESP32-S3 | **3.34** | **7,313** | [**2AC7Z-ESPS3MINI1**](../references/index.md#fcc-2ac7z-esps3mini1) | ✓ (2) | **✓ full speed** |
| Ai-Thinker BW16 | Realtek RTL8720DN | 3.42 | 521 | [2AHMR-BW16](../references/index.md#fcc-2ahmr-bw16) | ? | ✗ |
| Silicon Labs BGM220P | EFR32BG22 | 5.36–8.22 | 38–58 | [QOQ-GM220P](../references/index.md#fcc-qoq-gm220p) | ? | ✗ |
| Raytac MDBT50Q | nRF52840 | 7.71–8.53 | 0 | [SH6MDBT50Q](../references/index.md#raytac-mdbt50q-fcc) (modular) | ✓ | ✗ (device only) |
| Raytac MDBT53 | nRF5340 | 9.59 | 0 | not checked | ✓ | ✗ (device only) |
| u-blox ANNA-B402 | nRF52833 | 11.89 | 18 | not checked | ✓ | ✗ |
| u-blox NINA-B306 | nRF52840 | 19.58 | 68 | not checked | ✓ | ✗ |

**ESP32-S3-MINI-1** ([datasheet v1.7](../references/index.md#esp32s3-mini1-ds)):
mass production; −40 to +85 °C for all ordering codes; 15.4 × 20.5 mm (`-1U`:
15.4 × 15.4 mm); Bluetooth LE at 125 k/500 k/1 M/2 Mbit/s; two I2S interfaces;
full-speed USB 2.0 OTG; 3.0–3.6 V; peak 340 mA at +20 dBm BLE TX. ESP-IDF is
Apache-2.0. It is the only module in the table with a USB host, which the radio
USB audio path needs (§4).

### 1.2 Why Bluetooth Classic was dropped

Dual mode (Classic SPP + HFP with BLE) was the original requirement. These
dual-mode candidates were checked before [ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)
replaced it; the findings are kept for the record.

| Candidate | Result | Reason (source) |
|---|---|---|
| Espressif ESP32-WROOM-32E | Passed | SPP + HFP + BLE in ESP-IDF; mSBC only on the HCI data path ([`hfp_hf` example](../references/index.md#esp-idf-hfp-hf-readme)); FCC ID 2AC7Z-ESP32WROOM32E. No USB. |
| Espressif ESP32-S31 | Watch list | Classic + BLE + high-speed USB OTG; pre-release datasheet, no FCC ID found; listed at LCSC at $5.90 with no stock. |
| Raspberry Pi RM2 (CYW43439) | Failed | SCO links don't come up ([pico-sdk#1461](../references/index.md#pico-sdk-1461)); −30 to +70 °C. |
| Infineon CYBT-243053-02 | Failed | BTSDK under the Cypress End User License Agreement. |
| Microchip BM83 | Failed | Audio and SPP can't run at the same time ([Microchip KB](../references/index.md#microchip-bm83-spp-kb)). |
| Bouffalo BL616/BL618 (Ai-Thinker Ai-M62/Ai-M61) | Unconfirmed | Classic HFP code in `bouffalo_sdk` (Apache-2.0); Ai-M61 grant 2ATPO-AIM61, Classic coverage unconfirmed; thin stock. |
| SiFli SF32LB52, Beken BK7258 | Failed | No FCC-certified module found. |
| NXP IW416 modules + host MCU (Zephyr) | Passed, costly | Zephyr HFP/SCO/mSBC support; about $14–26 plus a host MCU. |

## 2. Audio codec and isolation (#8)

Requirements: line-level in and out, 48 kHz, ADC SNR ≥ 90 dB(A), AGC and all
voice processing disabled, I2S to the ESP32-S3, ±50 ppm clock, −40 to +85 °C
for variant M. This is the **analog** radio audio path; radios with a built-in
USB sound card use the USB path (§4) instead.

| Part | Why it's on the list | Notes |
|---|---|---|
| **TI TLV320AIC3104** | Mature, widely stocked; AGC can be bypassed (PGA mode); has an AEC-Q100 **-Q1** variant for M ([TI](../references/index.md#ti-tlv320aic3104-q1-ds)). | ADC SNR about 92 dB(A) **(verify)**. Built-in PLL lets it run from the ESP32-S3 MCLK or its own crystal. |
| TI TLV320AIC3204 | Better converters, 93 dB ADC SNR with AGC off ([datasheet](../references/index.md#ti-tlv320aic3204-ds)). | No automotive grade **(verify)**; −40 to +85 °C. |
| NXP SGTL5000 | Cheap, well known, many open drivers. | ADC SNR about 85 dB **(verify)**, likely below the 90 dB target. |
| Nuvoton NAU88C22 | Good specs, low cost. | Distributor depth **(verify)**. |
| Cirrus WM8960 | Common in hobby designs. | Lifecycle status **(verify)**. |

Everest ES8388 was dropped from the list: datasheet quality and authorized
distributor availability outside China are weak.

Superseded by [ADR-0002](../decisions/ADR-0002-audio-codec.md) and
[`audio-codec.md`](audio-codec.md), which choose the TLV320AIC3104 and the
Bourns SM-LP-5001; the note below is kept for the record.

**Isolation transformers:** Bourns LM-NP-1001-B1L, 600:600 Ω, 200–3500 Hz,
insertion loss ≤ 1.5 dB ([Bourns LM-NP/LP datasheet](../references/index.md#bourns-lm-np-ds)).
Its 200–3500 Hz band matches the requirement, but whether it is flat to
**±1 dB** across that band must be checked against the datasheet curve. The
SMD sibling (LM-LP-1001) suits assembly houses better.

## 3. CAT, CI-V, RTS/DTR and PTT (#9)

The SERIAL and AUDIO jack pinouts are fixed by
[`radio-connectors.md`](../requirements/radio-connectors.md). Prices and stock:
LCSC, 2026-09-24, USD at quantity 100.

- **RS-232:** the SERIAL jack carries only data (tip out, ring 1 in), so one
  driver and one receiver are enough: TRS3221E ($0.51, 1,780 in stock),
  SP3221E ($0.67), ADM3101E ($1.21). Pulling FORCEOFF low shuts the TRS3221E
  driver off (datasheet); confirm the off-state output leakage so the tip can
  serve the other modes **(verify)**. The
  jacks have no RS-232 RTS/DTR contacts; host RTS/DTR maps to the PTT closure on
  the AUDIO jack or to the radio's USB-serial chip.
- **Mode switching:** tip and ring 1 each go through a 36 V SPDT analog switch
  (TMUX6219, $2.16, 3,110 in stock) selecting the RS-232 path or the logic/CI-V
  path, so RS-232 levels never reach the 3.3 V parts. The switch needs a dual
  supply that spans the RS-232 swing (±4.5 V to ±18 V per its datasheet), e.g.
  from a small charge pump on the isolated side; its behavior with signals
  present while unpowered **(verify)**. Signal relays are the alternative (more
  robust, larger, no negative rail). Default at power-on: logic path.
- **TTL 3.3/5 V:** a direction-controlled level translator (SN74LXC1T45 or
  74LVC1T45) per line, 5 V tolerant, behind the mode switch, with series
  resistance and clamps.
- **CI-V:** single wire on the tip, open drain. 74LVC1G07 pulls it low; the
  radio sets the pull-up. Echo and collision handling is a firmware matter.
- **Isolation (variant M, and whenever USB-C data is used):** ISO7721 (one
  channel each way, ≥ 115.2 kBd). The radio-side supply of the isolated section
  (switches, RS-232, logic) is an open question for #9: a small isolated
  converter is the likely answer.
- **PTT:** a PhotoMOS (AQY212EH, 60 V) gives an isolated closure to ground on
  AUDIO-jack ring 2 and is **off unless its LED is driven**, which meets the
  hardware default-off rule. Drive it through a gate that a window watchdog
  (TPS3430) can hold off, so a hung MCU cannot key the radio.

## 4. USB host: the radio's USB sound card and USB-serial chip (#9)

Radios with their own USB port usually expose a USB-serial chip and a USB Audio
Class sound card, often behind a hub inside the radio. The ESP32-S3's
full-speed USB OTG port is the host, with Espressif's
[`esp-usb`](https://github.com/espressif/esp-usb) components (Apache-2.0):

| Need | Component |
|---|---|
| Radio's USB sound card (USB Audio Class 1.0) | `usb_host_uac` |
| Radio's USB-serial chip | `usb_host_cp210x_vcp`, `usb_host_ftdi_vcp`, `usb_host_ch34x_vcp`, `usb_host_cdc_acm` |
| Hub inside the radio | USB Host Library with `USB_HOST_HUBS_SUPPORTED` (off by default) |

- Bandwidth: 48 kHz / 16-bit stereo both ways is about 3 Mbit/s of the 12 Mbit/s bus.
- Open points **(verify)**: dual-port **CP2105** (FT-991A class) with the CP210x
  driver; RTS/DTR through each chip's vendor control requests; which radios carry
  audio on USB and how much VBUS current they draw (#5).
- The ESP32-S3's USB OTG and USB-Serial-JTAG share one PHY, so programming and
  logs go through a UART0 header.
- **VBUS to the radio:** TPS2553 (adjustable current limit, fault flag), sized
  from the per-radio draw in #5.
- The MAX3421E is needed only if the design falls back to a module without USB
  host (e.g. MDBT50Q). TinyUSB, the usual stack for it, has no USB Audio Class
  host driver **(verify)**.

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

## 5a. USB routing for wired USB-C mode (#9)

Wired mode ([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md)) needs the
computer to reach the radio's own USB chips and the ESP32-S3 at the same time,
but the ESP32-S3 has one USB controller. An on-board hub and two USB switches
solve it:

| Part | Role | $ @100 | LCSC stock |
|---|---|---|---|
| Microchip USB2422 | 2-port USB 2.0 hub: upstream to USB-C; ports to the radio USB-A and the ESP32-S3 | 1.47 | 885 |
| Genesys GL850G / WCH CH334 | Cheaper 4-port hub alternates | 0.37 / 0.26 | 20,644 / 38,000 |
| TI TS3USB221A (×2) | USB 2.0 1:2 switches: radio port and ESP32-S3 each go to the hub (wired) or to each other (Bluetooth, ESP32-S3 as host) | 0.21 | 92,257 |
| HRO TYPE-C-31-M-12 | USB-C receptacle (data + 5 V sink) | 0.15 | 92,244 |
| TI TPD2E2U06 / ST USBLC6-2 | USB ESD protection | 0.26 / 0.14 | 8,738 / 29,521 |
| ADI ADuM4160 | Optional full-speed USB isolator on the radio port (variant M) | 7.31 | 5,581 |

## 6. What this shortlist changes in the open issues

- **#7:** confirm the ESP32-S3-MINI-1 per ADR-0008 (dated second-distributor
  check, FCC integration notes); MDBT50Q is the fallback.
- **#8:** the codec connects to the ESP32-S3 over I2S and serves the analog
  radio audio path in both host modes.
- **#9:** no separate USB host controller. Design the radio USB port, the USB-C
  port, the hub and switch routing, the RS-232-tolerant SERIAL-jack mode
  switching, and jack isolation.
- **#10 / #11:** use the same buck family in both variants where the input range
  allows, to share layout and the HF harmonic analysis. Budget the module at
  340 mA peak and respect the 500 mA USB-C default in wired mode.

<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# Audio codec and isolation transformers

Issue: [#8](https://github.com/Reid-n0rc/open-bt-rig-interface/issues/8).
Researched 2026-09-24. Decision: [ADR-0002](../decisions/ADR-0002-audio-codec.md).

This study covers the **analog radio audio path**: the codec, the AUDIO-jack
isolation transformers, RF hardening at the jack, and the level plan. Radios
with a built-in USB sound card use the USB path instead
([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md),
[constraints §8](../requirements/constraints.md#8-audio)).

**Scope adapted to ADR-0008.** The issue text predates ADR-0008 and asks for
compatibility with an HFP audio path. There is no Bluetooth Classic and no HFP.
The codec connects to the **ESP32-S3 over I2S** and serves both host links:
the Bluetooth LE audio stream and the USB Audio Class sound card in wired mode.

Datasheet figures are typical values at 25 °C unless marked *min*. Each source
is listed in the [reference index](../references/index.md) with its revision.

## 1. Requirements

From [constraints §6 and §8](../requirements/constraints.md#8-audio) and
[`radio-connectors.md`](../requirements/radio-connectors.md#audio-jack-35-mm-trrs):

- Line-level RX input (AUDIO tip) and TX output (AUDIO ring 1), AC-coupled.
  Typical radio data ports: about 100 mV–1 Vrms, 600 Ω–10 kΩ (issue #8; the
  per-radio levels come from #5). About 19 dB of switchable attenuation for hot
  speaker outputs. TX up to about 2.5 V peak-to-peak, adjustable down to
  microphone level.
- 48 kHz internal rate, ADC SNR ≥ 90 dB(A), sample clock within ±50 ppm.
- No AGC, ALC, noise suppression or voice processing in the path. Fixed, measured latency.
- I2S to the ESP32-S3. −40 to +85 °C for variant M (−20 to +60 °C for variant R).
- JLCPCB standard assembly, top side, 2 sources per key part
  ([`pcb-fabrication.md`](../requirements/pcb-fabrication.md#4-assembly)).
- EU market (maintainer, 2026-09-24, #59): every chosen part RoHS-compliant;
  REACH status recorded where the manufacturer shows it. ESD and immunity
  levels at the jacks come from EN 301 489-1 via #59.
- Design priority (maintainer, 2026-09-24): minimize spurious emissions and cost.
- Galvanic isolation of the AUDIO jack: **mandatory on variant M and whenever
  the USB-C data link is used**; optional only for variant R used over
  Bluetooth and powered from the radio.

## 2. Sample clock and the ±50 ppm requirement

### 2.1 ESP32-S3 I2S clocking

From the [ESP32-S3 TRM v1.8](../references/index.md#esp32s3-trm), §28.6, and the
[hardware design guidelines](../references/index.md#esp32s3-hw-design), §1.3.5:

- Two I2S controllers. Each can be clock master or slave, and can output
  `I2Sn_MCLK_out` as a master clock for an external device, routed through the
  GPIO matrix. In slave mode the I2S clock must be at least 8 × BCLK.
- **No audio PLL.** The I2S clock is divided from 40 MHz `XTAL_CLK`, 160 MHz
  `PLL_F160M_CLK`, 240 MHz `PLL_D2_CLK`, or an external `I2Sn_MCLK_in`. The
  divider is N + b/a (N = 2–256) with a fractional part. The TRM warns that
  the fractional divider "may introduce some clock jitter".
- All internal clocks derive from the 40 MHz crystal. The design guidelines
  require it to be within ±10 ppm; the module schematic
  ([datasheet v1.7](../references/index.md#esp32s3-mini1-ds)) shows a
  40 MHz ±10 ppm part. Its tolerance over temperature and ageing isn't stated
  **(verify)**.

### 2.2 Can the codec PLL run from the 2.304 MHz buck-sync clock?

The shared board clock is defined in [ADR-0004](../decisions/ADR-0004-power-automotive.md) (proposed): an
**18.432 MHz ±20 ppm oscillator** (YXC OT322518.432MJBA4SL, 0.7 ps maximum
phase jitter) divided by 8 with SN74LVC1G80 flip-flops to **2.304 MHz**
(= 48 × 48 kHz), star-distributed through 33 Ω series resistors to the buck
SYNC, the codec MCLK and the isolated supply. No spread spectrum. ADR-0004
rejected feeding the codec the 18.432 MHz clock directly (the non-PLL path)
for emissions. Checked against the TLV320AIC3104 datasheet
([SLAS510G](../references/index.md#ti-tlv320aic3104-ds) §10.3.3.1, pages 26–28):

- With the PLL off, fs = CLKDIV_IN / (128 × Q) with Q ≥ 2, so MCLK must be at
  least 256 × 48 kHz = 12.288 MHz. **2.304 MHz needs the PLL.**
- With the PLL on, fs = MCLK × K × R / (2048 × P), with P = 1–8, R = 1–16,
  K = J.D (J = 1–63, D = 0–9999). For 48 kHz: K × R / P = 48,000 × 2048 /
  2,304,000 = 128/3.
- **D ≠ 0 is not possible:** it requires 10 MHz ≤ MCLK / P ≤ 20 MHz
  (page 27), and 2.304 MHz is below 10 MHz for every P.
- **D = 0 works:** the limits are 512 kHz ≤ MCLK / P ≤ 20 MHz,
  80 MHz ≤ MCLK × K × R / P ≤ 110 MHz and 4 ≤ J ≤ 55 (page 27). Only P = 3
  keeps MCLK / P ≥ 512 kHz (P = 3 gives 768 kHz; P = 6 gives 384 kHz). Then
  J × R = 128: **P = 3, R = 8, J = 16, D = 0** (J = 8 with R = 16, or J = 32
  with R = 4, also fit).
- Result: PLL output 2.304 MHz × 16 × 8 / 3 = **98.304 MHz** (inside
  80–110 MHz), ÷ 8 = 12.288 MHz = 256 × 48 kHz, so fs = **48,000 Hz exactly**,
  with integer settings only.

The datasheet's example table (Table 10-1, page 28) starts at 2.048 MHz and
doesn't list 2.304 MHz, so this setting is derived from the equations and
limits above and must be confirmed on the bench **(needs bench test)**.

### 2.3 Options

| Option | How | Accuracy | Spurious and cost |
|---|---|---|---|
| A. ESP32-S3 outputs 12.288 MHz | 160 MHz ÷ 13.0208 (fractional) | Crystal, ±10 ppm | Fractional-divider jitter at the converters; a 12.288 MHz clock net |
| B. ESP32-S3 outputs 16 MHz; codec PLL | 160 MHz ÷ 10; P = 1, R = 1, J = 6, D = 1440 (Table 10-1) | Crystal, ±10 ppm | Adds a **16 MHz** clock net and its harmonics; no parts; uses one GPIO |
| **D. Codec MCLK from the 2.304 MHz shared clock (ADR-0004); codec PLL** | P = 3, R = 8, J = 16, D = 0 (§2.2); codec is I2S master, ESP32-S3 is I2S slave | Oscillator, ±20 ppm | **No new clock frequency on the board**; its 33 Ω series resistor and clock branch are part of ADR-0004; frees one GPIO |
| C. Separate 12.288 MHz crystal at the codec | Codec is I2S master | That crystal | Adds a part and a third clock domain |

Spurious comparison between B and D (US amateur bands per 47 CFR 97.301, as
used in ADR-0004):

- **16 MHz (option B):** no harmonic lands in an HF band (16 and 32 MHz are
  outside 1.8–29.7 MHz; the 6 m band 50–54 MHz falls between 48 and 64 MHz),
  but the 9th harmonic, **144.000 MHz**, is at the bottom edge of the 2 m band.
  It is also a second, unrelated comb: its mixing products with the 2.304 MHz
  buck harmonics fall at n × 2.304 ± m × 16 MHz, which ADR-0004's band scan
  doesn't cover.
- **2.304 MHz (option D):** the codec MCLK adds no new frequency; its
  harmonics are the buck's, already scanned in ADR-0004 (HF clear by ≥ 88 kHz,
  6 m flagged for measurement). Every audio clock (MCLK 2.304 MHz, BCLK
  3.072 MHz at 64 fs, the codec PLL at 98.304 MHz) is then a multiple of
  768 kHz. The BCLK comb exists in both options.
- **Supply ripple into the converters.** The ADC and DAC delta-sigma modulators run
  at 128 × fs = 6.144 MHz (datasheet §10.3.3.2, §10.3.4), and the buck's 8th harmonic
  is 8 × 2.304 = 18.432 MHz = 3 × 6.144 MHz. Any ripple coupled into the
  modulator at that harmonic aliases to **0 Hz** when both come from the same
  oscillator (option D), where AC coupling and the HPF remove it. With the
  codec on the ESP32-S3 crystal (option B), the two clocks differ by up to
  about 30 ppm (±10 and ±20 ppm), and the alias becomes a tone at up to
  18.432 MHz × 30 ppm ≈ **550 Hz**, inside the audio passband. How much ripple
  reaches the modulator is unknown **(needs bench test)**, but option D removes
  the mechanism.

**Recommendation: option D.** It costs nothing extra (the shared clock is
already on the board, ADR-0004), removes the 16 MHz net and its 2 m harmonic, keeps every clock
on one grid, and ties the audio sample clock to the buck so supply ripple can't
beat into the audio band. The sample clock accuracy is the oscillator's
(±20 ppm, ADR-0004), inside ±50 ppm. Conditions:

- The codec is the I2S master (it drives BCLK and WCLK); the ESP32-S3 runs as
  I2S slave, which the TRM supports (I2S clock ≥ 8 × BCLK).
- The codec MCLK is one branch of ADR-0004's star distribution, with its own
  33 Ω series resistor. Route it away from the audio inputs.
- **Fallback:** if ADR-0004's clock isn't adopted, or the PLL setting
  fails on the bench, use option B. A DNP 0 Ω link from an ESP32-S3 GPIO to
  the codec MCLK keeps that fallback without a board respin.

Option C buys nothing: the host side is another clock domain in both modes
(USB SOF in wired mode, the phone or computer over Bluetooth), and firmware
already rate-matches there ([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md#consequences)).

## 3. Codec comparison

Figures from each manufacturer's datasheet (revision in the index entry).
SNR is A-weighted, referenced to full scale, unless noted.

| | TI **TLV320AIC3104** | TI TLV320AIC3204 | TI TAC5112 | NXP SGTL5000 | Nuvoton NAU88C22 | TI PCM1808 + PCM5102A |
|---|---|---|---|---|---|---|
| Datasheet | [SLAS510G, 2021](../references/index.md#ti-tlv320aic3104-ds); -Q1: [SLAS715D, 2024](../references/index.md#ti-tlv320aic3104-q1-ds) | [SLOS602E, 2019](../references/index.md#ti-tlv320aic3204-ds) | [SLASF24A, 2025](../references/index.md#ti-tac5112-ds); -Q1: [SLASFC2A, 2025](../references/index.md#ti-tac5112-q1-ds) | [Rev. 7, 1/2022](../references/index.md#nxp-sgtl5000-ds) | [Rev 0.8, Dec 2019](../references/index.md#nuvoton-nau88c22-ds) | [SLES177B, 2015](../references/index.md#ti-pcm1808-ds); [SLAS859C, 2015](../references/index.md#ti-pcm5102a-ds) |
| ADC SNR | 92 dB (**80 min**), single-ended or differential | 93 dB (**80 min**) | 102 dB single-ended, 104 dB differential | 90 dB at 3.3 V (−60 dB input method); 85 dB at 1.8 V | 89 dB | 99 dB |
| ADC THD / THD+N | THD −89 dB single-ended, −94 dB differential (−2 dBFS) | THD+N −85 dB (−3 dBFS) | THD+N −94 dB single-ended, −97 dB differential (−1 dBFS) | THD+N −72 dB | THD+N −78 dB | THD+N −93 dB |
| DAC line-out SNR / THD | 102 dB / −95 dB differential; 96 dB / −71 dB single-ended | 100 dB / THD+N −83 dB | DR 114 dB / THD+N −96 dB differential | 100 dB / THD+N −85 dB | 89 dB / −84 dB | 112 dB / THD+N −93 dB |
| Line-in full scale | 0.707 Vrms single-ended (0 dB PGA) | 0.5 Vrms single-ended | 1 Vrms single-ended, 2 Vrms differential | 1.0 Vrms (3.3 V) | 1.0 Vrms | 3 Vpp (5 V analog supply) |
| Input gain | PGA 0–59.5 dB, 0.5 dB steps; input level control 0 to −12 dB, 1.5 dB steps | PGA 0–47.5 dB, 0.5 dB steps | Digital volume −80 to +47 dB, 0.5 dB steps; input impedance 5/10/40 kΩ | ADC analog volume 0–22.5 dB, 1.5 dB steps (−6 dB range shift) | PGA −12 to +35.25 dB, 0.75 dB steps | None |
| Line-out full scale | 1.414 Vrms differential, 0.707 Vrms single-ended | 0.5 Vrms | 2 Vrms differential, 1 Vrms single-ended | 1.0 Vrms (3.3 V) | VDDA/3.3 Vrms | 2.1 Vrms, fixed |
| AGC/ALC and filters off? | Yes: AGC, ADC HPF and DAC effects filter all off at reset (registers 26, 12) | AGC off in the specified setup; processing blocks selectable (PRB modes) | Yes: AGC off at reset; HPF defaults to 1 Hz, settable to an all-pass IIR | Yes: AVC sits in the DAP, which is off until `DAP_EN` is set | ALC off at reset; HPF on at reset, can be disabled | No AGC. PCM1808 HPF fixed at 0.91 Hz |
| I2S master/slave, PLL | Both; PLL, MCLK 0.512–50 MHz or BCLK | Both; PLL (PLL input 0.512–20 MHz) | Controller and target; PLL; auto clock detection from BCLK/FSYNC | Both; PLL from 8–27 MHz SYS_MCLK | Both; fractional PLL from 8–33 MHz | Both (PCM1808, needs 256/384/512 fs); PCM5102A slave, PLL from BCK |
| Supplies | AVDD 2.7–3.6 V, DVDD 1.525–1.95 V, IOVDD 1.1–3.6 V | LDOIN 1.9–3.6 V (internal LDOs), IOVDD 1.1–3.6 V | Single AVDD 1.8 or 3.3 V; IOVDD 1.2/1.8/3.3 V | 1.62–3.6 V | Analog 2.5–3.6 V, core 1.7 V+ | 5 V analog + 3.3 V digital (PCM1808); 3.3 V (PCM5102A) |
| Current (48 kHz) | ADC stereo 4.31 + 2.45 mA; DAC to line-out 4.9 + 2.3 mA; PLL +1.4 + 0.9 mA | 6.1 mW record, 4.1 mW playback | ADC 2-ch 5.7 mA (PLL off); ADC + DAC to headphone 27.6 mA | Record + playback 4.67 + 0.34 mA (3.3 V) | See datasheet §11.3 | PCM1808 8.6 + 5.9 mA |
| Temperature | −40 to +85 °C; -Q1: −40 to +105 °C, AEC-Q100 grade 2 | −40 to +85 °C | **−40 to +125 °C** (grade 1); -Q1 AEC-Q100 | −40 to +85 °C | −40 to +85 °C | PCM1808 −40 to +85 °C; PCM5102A junction −40 to +130 °C, ambient **(verify)** |
| Package | VQFN-32, 5 × 5 mm | VQFN-32, 5 × 5 mm | VQFN-24, 4 × 4 mm, 0.5 mm pitch | QFN-20 3 × 3 mm / QFN-32 | QFN-32, 5 × 5 mm | TSSOP-14 + TSSOP-20 |
| Lifecycle (2026-09-24) | ACTIVE; TI shows "a newer version is available" (TAC5112) ([TI](../references/index.md#ti-tlv320aic3104-product)) | ACTIVE; same TI notice | ACTIVE (TI product page) | QFN-32 punch (XNAA3) EOL by Q1 2022 per datasheet; others **(verify)** | **(verify)** | Both ACTIVE; TI shows newer versions |

Everything in the table can be assembled by JLCPCB's standard process
(QFN with 0.5 mm pitch and an exposed pad, or TSSOP). Whether a part is a
JLCPCB Basic or Extended part is checked at order time **(verify)**.

### 3.1 Price and stock

LCSC, 2026-09-24, USD, live product pages. Digi-Key and Mouser blocked
automated lookups on 2026-09-24 (bot protection), so those cells are blank.
The maintainer deferred the second-distributor lookups (2026-09-24); they
don't block ADR-0002. RoHS and REACH are from TI's part pages (2026-09-24),
checked for the chosen part and its alternates; "not checked" marks rejected
candidates.

| Part (LCSC #) | Stock | $ @ 1 | $ @ 100 | RoHS / REACH | Digi-Key | Mouser |
|---|---|---|---|---|---|---|
| TLV320AIC3104IRHBR ([C181753](https://www.lcsc.com/product-detail/C181753.html)) | 2,601 | 1.5076 | 0.9305 | Yes / Yes ([TI](https://www.ti.com/product/TLV320AIC3104/part-details/TLV320AIC3104IRHBR)) | | |
| TLV320AIC3104IRHBT ([C2867364](https://www.lcsc.com/product-detail/C2867364.html)) | 7 | 0.926 | 0.8785 | not checked | | |
| TLV320AIC3104IRHBRQ1 | not found at LCSC | | | TI part page not reachable **(verify)** | | |
| TLV320AIC3204IRHBR ([C24109](https://www.lcsc.com/product-detail/C24109.html)) | 5,022 | 1.7233 | 1.0498 | Yes / Yes ([TI](https://www.ti.com/product/TLV320AIC3204/part-details/TLV320AIC3204IRHBR)) | | |
| TAC5112IRGER | not found at LCSC; TI store showed out of stock with a sample-quantity limit | | | Yes / Yes ([TI](https://www.ti.com/product/TAC5112/part-details/TAC5112IRGER)) | | |
| TAC5212IRGER (pin-compatible sibling, [C44853694](https://www.lcsc.com/product-detail/C44853694.html)) | 207 | 8.2002 | 6.1962 | not checked | | |
| SGTL5000XNLA3R2 ([C2651833](https://www.lcsc.com/product-detail/C2651833.html)) | 1,110 | 9.3462 (@5) | 8.0286 (@50) | not checked | | |
| NAU88C22YG ([C914209](https://www.lcsc.com/product-detail/C914209.html)) | 5,262 | 1.213 | 0.8054 | not checked | | |
| PCM1808PWR ([C55513](https://www.lcsc.com/product-detail/C55513.html)) | 28,212 | 0.7084 | 0.4274 | not checked | | |
| PCM5102APWR ([C107671](https://www.lcsc.com/product-detail/C107671.html)) | 1,616 | 1.4395 | 0.9444 | not checked | | |

### 3.2 Notes per candidate

- **TLV320AIC3104.** Mature and stocked. ADC SNR meets 90 dB(A) only as a
  typical value (80 dB minimum), so the ADC noise must be measured on the
  first boards **(needs bench test)**. The differential line input has better
  THD (−94 dB) than the single-ended input (−89 dB), which suits a floating
  transformer secondary. The catalog part covers −40 to +85 °C, which meets
  variant M; the maintainer chose the catalog part for variant M
  (2026-09-24). The **-Q1** part is a pin-for-pin identical option (same pin
  table, same 32-pin VQFN) with AEC-Q100 grade 2, not required. Needs a 1.8 V DVDD rail. Filter group
  delay: ADC 17/fs (354 µs), DAC 21/fs (438 µs) at 48 kHz.
- **TLV320AIC3204.** Similar performance, internal LDOs, stocked. Not
  pin-compatible with the AIC3104. No automotive grade found.
- **TAC5112.** The best converters here (102 dB(A) single-ended ADC SNR),
  single 3.3 V supply, −40 to +125 °C even in the catalog grade, and TI's
  named successor to the AIC3104. **No stock found** at LCSC or TI on
  2026-09-24; it's a 2023 part. Pin-compatible with the TAC5212 (119 dB DR,
  about 7× the price). Not pin-compatible with the AIC3104.
- **SGTL5000.** ADC SNR 90 dB (3.3 V) measured with a −60 dB input, not the
  same method as the others; THD+N −72 dB. Most expensive at LCSC. One package
  already EOL.
- **NAU88C22.** Cheap and stocked, but ADC SNR 89 dB misses the 90 dB target.
- **PCM1808 + PCM5102A.** Excellent converters and cheap, but: no analog gain
  (the RX level must be set outside), a 5 V analog supply for the ADC, a fixed
  2.1 Vrms DAC output (TX level only digital, plus a pad), two packages.
- **Excluded:** Cirrus **WM8960** (last-time buy 2022-07-26, last ship
  2024-01-17) and **WM8731** (last-time buy 2022-09-21, last ship 2023-12-25),
  both end of life per [Cirrus Logic](../references/index.md#cirrus-eol).
  **Everest ES8388** stays excluded for the reasons in
  [`core-devices.md`](core-devices.md#2-audio-codec-and-isolation-8).

## 4. Isolation transformers

| | Bourns **SM-LP-5001** | Triad TY-250P | Bourns LM-NP-1001-B1L |
|---|---|---|---|
| Datasheet | [SM-LP-5001](../references/index.md#bourns-sm-lp-5001-ds) | [TY-250P](../references/index.md#triad-ty-250p-ds) (May 2019) | [LM-NP/LP 1000](../references/index.md#bourns-lm-np-ds) |
| Impedance | 600:600 Ω, 1:1 | 1 kΩ CT : 1 kΩ CT / 250 Ω; 600:600 and 10k:10k usable at ≤ 4.2 Vrms, ≤ 7 mA | 600:600 Ω, 1:1 |
| Frequency response | **±0.25 dB max, 200–4000 Hz** | ±1 dB, 20–20,000 Hz | −0.3 dB typ, 200–3500 Hz |
| Insertion loss | 2.0 dB max at 2 kHz | < 2.8 dB at 1 kHz | ≤ 1.5 dB at 2 kHz |
| Distortion | −76 dB max at 600 Hz, −10 dBm | — | ≤ 0.1 % at 0 dB, 1 kHz |
| Maximum level | +10 dBm (2.45 Vrms at 600 Ω) | 20 mW; ≤ 4.2 Vrms for the alternative impedances | −45 to +3 dBm (1.10 Vrms at 600 Ω) |
| Isolation | 2000 Vrms, 1 min | 1500 V | 6.5 kVDC |
| Temperature | **−40 to +85 °C** | −40 to +105 °C | **−10 to +60 °C** |
| Mounting, size | SMD 6-pin, 12.8 × 9.0 mm, 7.5 mm high; tape and reel (`-5001E`, 400/reel) | Through-hole, 8 pins; outline up to 0.9 in (23 mm) per side, 0.4 oz | Through-hole, 17.7 × 12.7 mm |
| LCSC, 2026-09-24 | `SM-LP-5001E` [C840532](https://www.lcsc.com/product-detail/C840532.html): 1,086 in stock, $2.3619 @ 1, $1.4848 @ 100. `SM-LP-5001` (tubes) [C7503474](https://www.lcsc.com/product-detail/C7503474.html): 315, $3.065 @ 1, $2.30 @ 53 | not found | [C5361839](https://www.lcsc.com/product-detail/C5361839.html): 252, $4.3237 @ 1, $2.875 @ 100 |
| Digi-Key / Mouser | deferred (lookup blocked 2026-09-24) | deferred | deferred |
| RoHS | Compliant (RoHS 2015/863, datasheet); RoHS3 compliant ([LCSC](https://www.lcsc.com/product-detail/C840532.html)); REACH not shown | RoHS 3 (2015/863/EU) from February 2016 manufacturing date (datasheet) | Compliant (datasheet) |

Findings:

- **SM-LP-5001** meets every requirement: flat well inside ±1 dB over
  200–4000 Hz, −40 to +85 °C, SMD (top-side JLCPCB assembly), and +10 dBm
  handling, which covers the TX maximum (2.5 Vpp ≈ 0.88 Vrms) with about 9 dB
  of margin. Its reflow profile peaks at 240 °C **(verify against the JLCPCB
  reflow profile)**.
- **LM-NP-1001-B1L**, the shortlist's candidate in
  [`core-devices.md`](core-devices.md#2-audio-codec-and-isolation-8), is rated
  only −10 to +60 °C, which fails both variant R (−20 °C) and variant M. Its
  low-profile sibling LM-LP-1001 is also a pin-through-hole part, not SMD, and
  has the same temperature range.
- **TY-250P** is the through-hole fallback with the widest temperature range
  and the option of a 10k:10k connection, but it is large and heavy (0.4 oz)
  for a small, vibration-exposed board.
- **Saturation.** Saturation is worst at the lowest frequency. Keep the level
  at the transformer at or below its rating: put the switchable RX attenuator
  (§6) on the radio side, **before** the transformer, so a hot speaker output
  never drives the core past +10 dBm. Check 200 Hz distortion at maximum
  level on the bench **(needs bench test)**.

### 4.1 Fitting per variant

[Constraints §6](../requirements/constraints.md#6-safety-and-fail-safe)
makes isolation mandatory on variant M and whenever the USB-C data link is
used, and optional only for variant R over Bluetooth powered from the radio.
In that last case the device's only ground is the radio's ground (the
accessory DC cable), so there is no second path for a ground loop, and
capacitive coupling into the codec is enough.

The maintainer decided the fitting on 2026-09-24:

| Variant | Transformers (RX, TX) | 0 Ω bypass resistors |
|---|---|---|
| M | Fitted | Not fitted |
| R | **Not fitted (DNP) by default** | **Fitted** |

One layout carries both the transformer footprints and the 0 Ω bypass
footprints.

**Conflict, not resolved here:** constraints §6 requires isolation "on every
variant whenever the USB-C data link is used". A variant R board with the DNP
default has no audio isolation, so it doesn't meet §6 in wired mode (the
computer's ground then reaches the radio's ground through the device). The
same applies to variant R powered from a USB-C charger whose output is
earth-referenced **(verify)**. Options for the maintainer: amend §6; fit the
transformers on variant R boards sold or configured for wired use; or leave it
to the variants decision (#12). See [ADR-0002](../decisions/ADR-0002-audio-codec.md#consequences).

## 5. RF hardening at the AUDIO jack

The device works next to 100 W+ HF transmitters
([constraints §5](../requirements/constraints.md#5-rf-environment)). Per
contact (tip, ring 1), from the jack inward:

1. **ESD diode** to the jack sleeve, as close to the jack as possible. Its
   working voltage must exceed the largest expected RX signal peak (hot speaker
   outputs can be several volts; the per-radio levels come from #5), and its
   capacitance must be low enough not to load a 600 Ω source in the audio band.
   Part choice in the schematic issue (TPD1E10B06-class, see
   [`core-devices.md`](core-devices.md#summary)) **(verify working voltage)**.
   The ESD and RF immunity levels the jack must pass come from EN 301 489-1
   (EU market) **(verify, #59)**.
2. **Ferrite bead** in series, then a **C0G capacitor** to the sleeve (for
   example 1–4.7 nF). Chip ferrites are specified at 100 MHz and have less
   impedance at HF, so the RC stage below does the HF work; the bead handles
   VHF.
3. **RC low-pass** (series resistor plus C0G capacitor) after the transformer,
   at the codec input, for example 100 Ω and 1 nF (corner about 1.6 MHz, far
   above the audio band, and below the 1.8 MHz band).
4. **Grounding:** with the transformer fitted, the jack sleeve is the isolated
   radio-side ground, kept as a separate copper area from the device ground
   with no connection except through the transformer. The filter capacitors in
   steps 1–2 go to that isolated ground.

Values are starting points for the schematic issue, checked by simulation and
by the RF immunity test in the bring-up plan **(needs bench test)**. All
capacitors in the audio path are C0G
([`pcb-fabrication.md` §6.2](../requirements/pcb-fabrication.md#62-dielectric)).

## 6. Level plan

For the recommended TLV320AIC3104 with a 1:1 transformer (insertion loss up to
2 dB, ignored below). Codec full scale (0 dBFS): **0.707 Vrms** single-ended
input at 0 dB PGA, **1.414 Vrms** differential line output
([datasheet](../references/index.md#ti-tlv320aic3104-ds) §8.5).

**RX (radio → codec).** Target the radio's normal level at about −12 dBFS,
leaving headroom for peaks and level changes. Generic data-port levels from
issue #8; per-radio values come from #5.

| Radio output | Attenuator (radio side) | At codec | Codec setting | Result |
|---|---|---|---|---|
| 100 mVrms (low data port) | bypassed | 100 mVrms (−17 dBFS at 0 dB) | PGA +5 dB | −12 dBFS |
| 300 mVrms | bypassed | 300 mVrms (−7.5 dBFS) | input level control −4.5 dB | −12 dBFS |
| 1 Vrms (hot data port) | bypassed | 1 Vrms (+3 dBFS) | input level control −12 dB **(verify input swing at the pin)**, or switch in the attenuator | −9 dBFS |
| Speaker output, a few Vrms | about −19.4 dB (100 kΩ / 12 kΩ, the jack convention) | e.g. 3 Vrms → 0.32 Vrms | PGA 0 to +5 dB | about −12 dBFS |

- Range: PGA 0–59.5 dB in 0.5 dB steps, and 0 to −12 dB in 1.5 dB steps
  before the mixer. With the −19.4 dB attenuator switched in, the input
  reaches full scale at about 6.6 Vrms; without it, the PGA gain covers inputs
  down to tens of millivolts.
- The attenuator is switchable (firmware-controlled analog switch or relay, or
  a solder jumper as a fallback) and sits before the transformer (§4).
- Firmware applies no automatic gain: levels are set once per radio and stored
  in the configuration.

**Transformer bypassed (variant R default).** The 0 Ω bypass removes the
transformer's insertion loss (up to 2 dB), so the RX figures above move up by
at most 2 dB. The RX path becomes single-ended: tip into one leg of the
codec's differential input, the other leg referenced to the device ground,
which the bypass joins to the jack sleeve.

**TX (codec → radio).** With the transformer fitted, drive its primary
differentially from the line outputs: up to 1.414 Vrms (4 Vpp) at 0 dB, which
covers the 2.5 Vpp maximum of the jack convention. With the transformer
bypassed (variant R default), ring 1 is driven single-ended from one line
output through a DC-blocking capacitor: full scale 0.707 Vrms (2 Vpp), below
the 2.5 Vpp maximum. Radios whose data input needs more than 2 Vpp would need
the output level control (up to +9 dB, limited by the output swing
**(verify)**) or the transformer fitted.

| Control | Range | Use |
|---|---|---|
| DAC digital volume | 0 to −63.5 dB, 0.5 dB steps | Main TX level control, set per radio |
| Output level control | 0 to +9 dB, 1 dB steps | Coarse gain (0 dB normally) |
| Optional resistive pad on the radio side (fitting option) | e.g. −20 dB or −40 dB | For microphone-level inputs, so the DAC runs near full scale and keeps its SNR |

- The line outputs are specified into 10 kΩ; whether they can drive a 600 Ω
  winding directly, or need a series resistor or the headphone drivers
  (specified into 16 Ω), is decided in the schematic **(verify)**.
- A series capacitor on the primary blocks any DC offset between the two
  outputs.

## 7. Supplies

The board has one 3.3 V rail from a buck synchronized at 2.304 MHz
([ADR-0004](../decisions/ADR-0004-power-automotive.md)); there
is no 5 V rail. TLV320AIC3104 supply limits
([SLAS510G](../references/index.md#ti-tlv320aic3104-ds) §8.1, §8.3, Table 10-5
and §12, pages 7, 35 and 91):

| Rail | Range | Notes |
|---|---|---|
| AVDD, DRVDD | 2.7–3.6 V | Must stay within 0.1 V of each other (absolute maximum), so one source feeds both |
| DVDD | 1.525–1.95 V | Recommended range depends on the output common-mode setting (Table 10-5) |
| IOVDD | 1.1–3.6 V | 3.3 V, the ESP32-S3 I/O level |
| Sequencing | IOVDD first, then AVDD/DRVDD, then DVDD within 5 ms; analog supplies ≥ DVDD at all times; RESET low until all are stable | |

### 7.1 Analog supply: filtered 3.3 V or a low-noise LDO

| | Ferrite bead + capacitors from 3.3 V | **LDO 3.3 V → 3.0 V** |
|---|---|---|
| Parts | Murata BLM18PG221SN1D (0603, 220 Ω at 100 MHz) plus MLCCs | TI **LP5907MFX-3.0/NOPB** (SOT-23-5; < 6.5 µVrms noise, PSRR 82 dB at 1 kHz, 20 mA; [SNVS798Q](../references/index.md#ti-lp5907-ds)) plus 2 × 1 µF |
| LCSC, 2026-09-24 | [C80165](https://www.lcsc.com/product-detail/C80165.html): 335,150 in stock, $0.0123 @ 500 | [C475492](https://www.lcsc.com/product-detail/C475492.html): 28,780 in stock, $0.2312 @ 50, $0.2018 @ 150 |
| Buck ripple at 2.304 MHz | Attenuated | Attenuated |
| Audio-band disturbances on the 3.3 V rail (ESP32-S3 BLE TX bursts up to 340 mA per the [module datasheet](../references/index.md#esp32s3-mini1-ds), the buck's load-step response) | **Not attenuated**: a bead and MLCC have their corner far above 20 kHz | Attenuated by the LDO's PSRR |
| Extra cost | about $0.02 | about $0.20 |

The codec's own supply rejection is limited: ADC PSRR is 55 dB at 217 Hz and
44 dB at 1 kHz (single-ended input, signal on DRVDD; datasheet §8.5). A
disturbance of 1 mVrms on DRVDD at 1 kHz then appears at about 6.3 µVrms at
the ADC, −101 dBFS against the 0.707 Vrms full scale; 10 mVrms appears at
about −81 dBFS, above the ADC's 92 dB noise floor. The size of the BLE-burst
disturbance on the 3.3 V rail depends on the buck's transient response
(#10, #11) and isn't known yet. The ferrite option can't guarantee the audio
targets; the LDO can, with its 82 dB at 1 kHz.

**Decision: LP5907MFX-3.0/NOPB for AVDD/DRVDD.** It is the cheapest option
that keeps the audio targets independent of the buck's load behavior. What
3.0 V costs:

- **Full scale and SNR: nothing in the datasheet figures.** The output
  common-mode and range come from an internal bandgap, not the supply
  (§10.3.4.6), and the electrical characteristics are specified at the 1.35 V
  output common-mode, which Table 10-5 allows from 2.7 V. At 3.0 V the 1.5 V
  setting is also allowed; the 1.65 V and 1.8 V settings (for 3.3 V and above)
  are not needed. The datasheet has no SNR-versus-supply data, so confirm the
  ADC SNR at 3.0 V on the bench **(needs bench test)**.
- **Headroom to the rail:** a full-scale differential output (1.414 Vrms)
  swings each leg about 1.35 ± 1.0 V, so 0.65 V below a 3.0 V rail instead of
  0.95 V below 3.3 V. The ADC input full scale (0.707 Vrms) is specified at
  DRVDD = 3.3 V; check it at 3.0 V, and the input swing at 1 Vrms with the
  −12 dB input level control, which gets tighter at 3.0 V **(verify)**.
- **LDO headroom:** 300 mV from a 3.3 V input. The LP5907's dropout is 250 mV
  maximum at 250 mA (SOT-23); the codec draws about 11 mA. The PSRR figures
  are specified at VIN = VOUT + 1 V, so the PSRR at 0.3 V headroom is lower
  **(verify against the datasheet curves)**. The buck's output tolerance must
  keep its minimum above about 3.1 V.

**Alternate:** TI TPS7A2030PDBVR (7 µVrms, PSRR 95 dB at 1 kHz;
[SBVS338H](../references/index.md#ti-tps7a20-ds)); the genuine TI part wasn't
found at LCSC on 2026-09-24.

### 7.2 Digital core supply

**TI TPS7A2018PDBVR** (1.8 V, SOT-23-5,
[SBVS338H](../references/index.md#ti-tps7a20-ds)), fed from the **3.0 V
AVDD output**, not from 3.3 V. Cascading guarantees the sequencing rule:
DVDD can't rise before AVDD, and can't exceed it. 1.8 V is inside the
recommended DVDD range for the 1.35 V and 1.5 V common-mode settings.
LCSC 2026-09-24: [C963430](https://www.lcsc.com/product-detail/C963430.html),
19,785 in stock, $0.2071 @ 50, $0.1847 @ 150. Check that it reaches 1.8 V
within 5 ms of AVDD **(verify)**. Alternate: LP5907MFX-1.8/NOPB.

IOVDD connects to the 3.3 V rail directly. The ESP32-S3 holds the codec's
RESET low until the supplies are stable.

### 7.3 RoHS and REACH (supply parts)

| Part | RoHS | REACH | Source |
|---|---|---|---|
| LP5907MFX-3.0/NOPB | Yes | Yes | [TI part page](https://www.ti.com/product/LP5907/part-details/LP5907MFX-3.0/NOPB), 2026-09-24 |
| TPS7A2018PDBVR | Yes | Yes | [TI part page](https://www.ti.com/product/TPS7A20/part-details/TPS7A2018PDBVR), 2026-09-24 |
| BLM18PG221SN1D (not chosen) | RoHS3 compliant | not shown | [LCSC C80165](https://www.lcsc.com/product-detail/C80165.html), 2026-09-24 |

## 8. Latency

Codec filter delays at 48 kHz: ADC 354 µs, DAC 438 µs (datasheet group
delays). The end-to-end latency (I2S DMA buffers, Bluetooth or USB buffering)
is fixed by firmware configuration and measured at bring-up
**(needs bench test)**.

## 9. Open points

- Digi-Key and Mouser price and stock for the chosen parts and alternates:
  deferred by the maintainer (2026-09-24), still needed for the two-source rule.
- Check TAC5112 availability again; if it stocks at two distributors, it is
  the better codec (see ADR-0002).
- ADC SNR of the AIC3104 on the first boards (datasheet minimum 80 dB).
- Bench-confirm the codec PLL at P = 3, R = 8, J = 16, D = 0 from 2.304 MHz
  (§2.2). Fallback: 16 MHz from
  the ESP32-S3 (option B).
- ADC SNR and input full scale at AVDD = 3.0 V; LP5907 PSRR at 0.3 V headroom (§7).
- ESD and immunity levels at the AUDIO jack from EN 301 489-1 (#59).
- Variant R with the DNP default doesn't meet constraints §6 in wired mode
  (§4.1): amend §6, fit transformers for wired use, or leave it to #12.

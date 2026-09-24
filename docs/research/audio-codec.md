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
- Galvanic isolation of the AUDIO jack: **mandatory on variant M and whenever
  the USB-C data link is used**; optional only for variant R used over
  Bluetooth and powered from the radio.

## 2. ESP32-S3 I2S clocking and the ±50 ppm requirement

From the [ESP32-S3 TRM v1.8](../references/index.md#esp32s3-trm), §28.6, and the
[hardware design guidelines](../references/index.md#esp32s3-hw-design), §1.3.5:

- Two I2S controllers. Each can be clock master or slave, and can output
  `I2Sn_MCLK_out` as a master clock for an external device, routed through the
  GPIO matrix.
- **No audio PLL.** The I2S clock is divided from 40 MHz `XTAL_CLK`, 160 MHz
  `PLL_F160M_CLK`, 240 MHz `PLL_D2_CLK`, or an external `I2Sn_MCLK_in`. The
  divider is N + b/a (N = 2–256) with a fractional part. The TRM warns that
  the fractional divider "may introduce some clock jitter".
- All internal clocks derive from the 40 MHz crystal. The design guidelines
  require it to be within ±10 ppm; the module schematic
  ([datasheet v1.7](../references/index.md#esp32s3-mini1-ds)) shows a
  40 MHz ±10 ppm part. Its tolerance over temperature and ageing isn't stated
  **(verify)**.

Options for a 48 kHz sample clock:

| Option | How | Accuracy | Jitter |
|---|---|---|---|
| A. ESP32-S3 outputs 12.288 MHz (256 × 48 kHz) | 160 MHz ÷ 13.0208 (N = 13, b/a = 1/48) | Crystal, ±10 ppm | Fractional divider: cycle-to-cycle jitter at the converter clock |
| **B. ESP32-S3 outputs an integer-divided MCLK; codec PLL makes 48 kHz** | 160 MHz ÷ 10 = 16 MHz. TLV320AIC3104 PLL: P = 1, R = 1, J = 6, D = 1440 gives exactly 48,000 Hz ([datasheet](../references/index.md#ti-tlv320aic3104-ds) Table 10-1) | Crystal, ±10 ppm | Integer divider; codec PLL generates the converter clocks |
| C. Separate 12.288 MHz crystal or oscillator at the codec | Codec is I2S master | That crystal's tolerance | Low; but a second clock domain the firmware must rate-match against the ESP32-S3 |

**Option B is recommended.** It stays in the ESP32-S3's crystal domain
(±10 ppm initial, well inside ±50 ppm), avoids fractional-divider jitter at
the converters, and needs no extra crystal. The 16 MHz MCLK meets the
AIC3104's PLL limits for a fractional K (10–20 MHz input, 80–110 MHz VCO,
J = 4–11). Option C buys nothing: the host side is another clock domain in
both modes (USB SOF in wired mode, the phone or computer over Bluetooth), and
firmware already rate-matches there ([ADR-0008](../decisions/ADR-0008-host-links-esp32-s3.md#consequences)).

Confirm on the bench that `I2Sn_MCLK_out` carries the divided `I2Sn_TX_CLK`
and measure the 48 kHz frame rate against a reference **(needs bench test)**.

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
don't block ADR-0002.

| Part (LCSC #) | Stock | $ @ 1 | $ @ 100 | Digi-Key | Mouser |
|---|---|---|---|---|---|
| TLV320AIC3104IRHBR ([C181753](https://www.lcsc.com/product-detail/C181753.html)) | 2,601 | 1.5076 | 0.9305 | | |
| TLV320AIC3104IRHBT ([C2867364](https://www.lcsc.com/product-detail/C2867364.html)) | 7 | 0.926 | 0.8785 | | |
| TLV320AIC3104IRHBRQ1 | not found at LCSC | | | | |
| TLV320AIC3204IRHBR ([C24109](https://www.lcsc.com/product-detail/C24109.html)) | 5,022 | 1.7233 | 1.0498 | | |
| TAC5112IRGER | not found at LCSC; TI store showed out of stock with a sample-quantity limit | | | | |
| TAC5212IRGER (pin-compatible sibling, [C44853694](https://www.lcsc.com/product-detail/C44853694.html)) | 207 | 8.2002 | 6.1962 | | |
| SGTL5000XNLA3R2 ([C2651833](https://www.lcsc.com/product-detail/C2651833.html)) | 1,110 | 9.3462 (@5) | 8.0286 (@50) | | |
| NAU88C22YG ([C914209](https://www.lcsc.com/product-detail/C914209.html)) | 5,262 | 1.213 | 0.8054 | | |
| PCM1808PWR ([C55513](https://www.lcsc.com/product-detail/C55513.html)) | 28,212 | 0.7084 | 0.4274 | | |
| PCM5102APWR ([C107671](https://www.lcsc.com/product-detail/C107671.html)) | 1,616 | 1.4395 | 0.9444 | | |

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

## 7. Latency

Codec filter delays at 48 kHz: ADC 354 µs, DAC 438 µs (datasheet group
delays). The end-to-end latency (I2S DMA buffers, Bluetooth or USB buffering)
is fixed by firmware configuration and measured at bring-up
**(needs bench test)**.

## 8. Open points

- Digi-Key and Mouser price and stock for the chosen parts and alternates:
  deferred by the maintainer (2026-09-24), still needed for the two-source rule.
- Check TAC5112 availability again; if it stocks at two distributors, it is
  the better codec (see ADR-0002).
- ADC SNR of the AIC3104 on the first boards (datasheet minimum 80 dB).
- Crystal tolerance over temperature and ageing for the module's 40 MHz crystal.
- Variant R with the DNP default doesn't meet constraints §6 in wired mode
  (§4.1): amend §6, fit transformers for wired use, or leave it to #12.

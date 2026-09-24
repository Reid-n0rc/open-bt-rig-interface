<!--
SPDX-FileCopyrightText: 2026 Reid Crowe, N0RC
SPDX-License-Identifier: CC-BY-4.0
-->

# ADR-0002: Audio codec TLV320AIC3104 and Bourns SM-LP-5001 isolation transformers

- **Status:** proposed
- **Date:** 2026-09-24
- **Issue:** #8

## Context

The analog radio audio path (AUDIO jack RX and TX) needs a codec and, on most
configurations, isolation transformers. Requirements
([constraints §5, §6, §8](../requirements/constraints.md#8-audio),
[`radio-connectors.md`](../requirements/radio-connectors.md#audio-jack-35-mm-trrs)):
line-level in and out, 48 kHz, ADC SNR ≥ 90 dB(A), ±50 ppm sample clock, no
AGC or voice processing, adjustable TX level, fixed latency, −40 to +85 °C for
variant M, JLCPCB standard assembly, two sources per key part. Isolation is
mandatory on variant M and whenever the USB-C data link is used.

The issue predates [ADR-0008](ADR-0008-host-links-esp32-s3.md) and asked for
compatibility with an HFP audio path. Under ADR-0008 there is no HFP: the
codec connects to the **ESP32-S3-MINI-1 over I2S** and feeds the Bluetooth LE
audio stream and the USB Audio Class sound card. The ESP32-S3 has no audio
PLL; its I2S clocks are divided (integer or fractional) from its 40 MHz
crystal domain ([TRM v1.8 §28.6](../references/index.md#esp32s3-trm)).

The full comparison, level plan and RF hardening are in
[`audio-codec.md`](../research/audio-codec.md).

## Options considered

Codecs (datasheet typicals; LCSC price at qty 100 and stock on 2026-09-24):

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **A. TI TLV320AIC3104** | ADC 92 dB(A), DAC 102 dB(A) differential; AGC, HPF and effects off at reset; PLL makes exact 48 kHz from a 16 MHz MCLK; −40 to +85 °C; pin-identical **-Q1** (AEC-Q100 grade 2); 2,601 in stock, $0.93 | ADC SNR minimum 80 dB (typical only meets 90 dB); extra 1.8 V rail; TI lists a newer part (TAC5112) | [Datasheet SLAS510G](../references/index.md#ti-tlv320aic3104-ds), [-Q1 SLAS715D](../references/index.md#ti-tlv320aic3104-q1-ds), [TI status](../references/index.md#ti-tlv320aic3104-product), [LCSC C181753](https://www.lcsc.com/product-detail/C181753.html) |
| B. TI TAC5112 | ADC 102 dB(A) single-ended; single 3.3 V supply; −40 to +125 °C; smaller 4 × 4 mm VQFN-24; AGC off at reset | No stock found at LCSC or TI (2026-09-24); new part; not pin-compatible with A | [Datasheet SLASF24A](../references/index.md#ti-tac5112-ds), [-Q1 SLASFC2A](../references/index.md#ti-tac5112-q1-ds) |
| C. TI TLV320AIC3204 | ADC 93 dB(A); internal LDOs; 5,022 in stock, $1.05 | ADC minimum 80 dB; no automotive grade found; not pin-compatible with A | [Datasheet SLOS602E](../references/index.md#ti-tlv320aic3204-ds), [LCSC C24109](https://www.lcsc.com/product-detail/C24109.html) |
| D. NXP SGTL5000 | Well known; AVC only inside the DAP, off by default | ADC 90 dB (−60 dB method), THD+N −72 dB; $8.03 @ 50; one package EOL | [Data sheet Rev. 7](../references/index.md#nxp-sgtl5000-ds), [LCSC C2651833](https://www.lcsc.com/product-detail/C2651833.html) |
| E. Nuvoton NAU88C22 | Cheap ($0.81), 5,262 in stock | ADC 89 dB, below target; lifecycle unverified | [Datasheet Rev 0.8](../references/index.md#nuvoton-nau88c22-ds), [LCSC C914209](https://www.lcsc.com/product-detail/C914209.html) |
| F. TI PCM1808 + PCM5102A | ADC 99 dB, DAC 112 dB; cheap ($0.43 + $0.94) | No analog input gain; 5 V analog supply; fixed 2.1 Vrms DAC output; two parts | [PCM1808](../references/index.md#ti-pcm1808-ds), [PCM5102A](../references/index.md#ti-pcm5102a-ds) |
| Excluded | Cirrus WM8960 and WM8731 (end of life, last ship 2024-01-17 and 2023-12-25); Everest ES8388 (see core-devices) | | [Cirrus EOL list](../references/index.md#cirrus-eol) |

Isolation transformers:

| Option | Pros | Cons | Sources |
|---|---|---|---|
| **Bourns SM-LP-5001** (600:600) | ±0.25 dB 200–4000 Hz; +10 dBm; −40 to +85 °C; SMD, tape and reel; 1,086 in stock, $1.48 (`-5001E`) | 7.5 mm tall; reflow peak 240 °C **(verify with JLCPCB)** | [Datasheet](../references/index.md#bourns-sm-lp-5001-ds), [LCSC C840532](https://www.lcsc.com/product-detail/C840532.html) |
| Triad TY-250P (600:600 or 10k:10k) | ±1 dB 20–20,000 Hz; −40 to +105 °C | Through-hole, large; not stocked at LCSC | [Datasheet](../references/index.md#triad-ty-250p-ds) |
| Bourns LM-NP-1001-B1L (600:600) | −0.3 dB 200–3500 Hz; 6.5 kVDC | **−10 to +60 °C** fails both variants; +3 dBm max; through-hole | [Datasheet](../references/index.md#bourns-lm-np-ds), [LCSC C5361839](https://www.lcsc.com/product-detail/C5361839.html) |

Digi-Key and Mouser blocked automated lookups on 2026-09-24; their price and
stock are still to be recorded (see Consequences).

## Decision

**Codec: TI TLV320AIC3104** (`TLV320AIC3104IRHBR`) on both variants. It is
the only candidate that meets every requirement *and* is stocked today; the
catalog part covers −40 to +85 °C. `TLV320AIC3104IRHBRQ1` is a drop-in
(same pins and package) where AEC-Q100 is wanted on variant M.

- **Alternate 1: TI TAC5112** (`TAC5112IRGER`, `-Q1`: `TAC5112WQRTVRQ1`).
  Technically better and TI's named successor. Switch to it, before the
  revision A schematic is frozen, if it shows stock at two distributors.
- **Alternate 2: TI TLV320AIC3204** (`TLV320AIC3204IRHBR`), stocked.
- Neither alternate is pin-compatible with the AIC3104: changing needs a
  schematic change, not a BOM swap.

**Clocking:** the ESP32-S3 outputs a 16 MHz MCLK (160 MHz ÷ 10, integer
divide) and the AIC3104 PLL generates exactly 48 kHz (P = 1, R = 1, J = 6,
D = 1440). The sample clock then inherits the module crystal's ±10 ppm, inside
the ±50 ppm requirement, without fractional-divider jitter at the converters
and without a second crystal. I2S master/slave direction is a firmware choice.

**Transformers: Bourns SM-LP-5001E** (600:600, SMD, tape and reel), one each
for RX and TX. **Alternate:** `SM-LP-5001` (same part in tubes) for sourcing,
and Triad TY-250P as the through-hole fallback. The LM-NP-1001-B1L listed in
[`core-devices.md`](../research/core-devices.md) is rejected on temperature range.

**Isolation fitting:** transformers are always fitted on variant M and on any
board used with the USB-C data link. For variant R over Bluetooth, powered from
the radio, they are optional; the board provides bypass footprints (0 Ω or
series capacitors). Fitted by default or as an option is decided in #12.

**Level plan and RF hardening** as in
[`audio-codec.md` §5–6](../research/audio-codec.md#5-rf-hardening-at-the-audio-jack):
a switchable −19.4 dB attenuator on the radio side before the RX transformer;
the codec's PGA and input level control for fine RX level; DAC digital volume
(0 to −63.5 dB, 0.5 dB steps) for TX, with an optional radio-side pad for
microphone-level inputs; ESD, ferrite and C0G filtering at the jack and an RC
low-pass at the codec input. AGC, HPF and effects filters stay at their reset
state (off).

## Consequences

- **Before this ADR is accepted:** record dated Digi-Key and Mouser price and
  stock for `TLV320AIC3104IRHBR`, `TLV320AIC3104IRHBRQ1`, `TLV320AIC3204IRHBR`,
  `TAC5112IRGER`, `SM-LP-5001E` and `SM-LP-5001` (two sources per part).
  Only LCSC was confirmed on 2026-09-24.
- **Power (#10, #11):** the codec needs AVDD/DRVDD 3.3 V (low-noise LDO,
  TPS7A20-class per [`core-devices.md`](../research/core-devices.md#5-power-10-11)),
  DVDD 1.525–1.95 V and IOVDD 3.3 V. The datasheet (§8.5) gives the blocks
  separately (analog + digital): stereo ADC 4.31 + 2.45 mA, DAC to line-out
  4.9 + 2.3 mA, PLL 1.4 + 0.9 mA. Their sum, about 11 mA analog and 6 mA
  digital, is an upper estimate for the budget.
- **Firmware:** I2S driver with a 16 MHz MCLK output; codec register setup that
  leaves AGC, HPF and effects off; per-radio RX gain and TX volume stored in
  the configuration; rate matching toward the host clock domains
  (ADR-0008). Measure end-to-end latency and fix it by configuration.
- **Schematic (#12 and the hardware issues):** transformer secondary into the
  codec's differential line input; differential TX drive with a DC-blocking
  capacitor; decide line-out or headphone-driver drive into the 600 Ω winding;
  isolated radio-side ground for the AUDIO jack; filter and ESD values.
- **Risks to verify:** AIC3104 ADC SNR on real boards (80 dB minimum);
  transformer distortion at 200 Hz and maximum level; input pin swing at 1 Vrms
  with the −12 dB input level control; module crystal tolerance over
  temperature; JLCPCB reflow profile versus the SM-LP-5001's 240 °C peak;
  TI's "newer version available" notice on the AIC3104 (still ACTIVE).
- [`core-devices.md`](../research/core-devices.md) is updated to point at this
  ADR for the codec and isolation rows.

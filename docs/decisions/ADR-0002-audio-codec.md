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
mandatory on variant M and whenever the USB-C data link is used. The maintainer
added (2026-09-24): optimize for **spurious emissions and cost**, and meet
**EU requirements** (#59): RoHS-compliant parts, REACH recorded where shown,
jack ESD and immunity levels from EN 301 489-1.

The board has a single 3.3 V rail from a buck synchronized to the shared
2.304 MHz clock defined in [ADR-0004](ADR-0004-power-automotive.md) (proposed): an 18.432 MHz
±20 ppm oscillator (YXC OT322518.432MJBA4SL, 0.7 ps maximum phase jitter)
÷ 8 with SN74LVC1G80 flip-flops, star-distributed through 33 Ω series
resistors to the buck SYNC, the codec MCLK and the isolated supply. There is
no 5 V rail.

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
| **A. TI TLV320AIC3104** | ADC 92 dB(A), DAC 102 dB(A) differential; AGC, HPF and effects off at reset; PLL makes exact 48 kHz with integer settings from the 2.304 MHz sync clock; −40 to +85 °C; pin-identical **-Q1** (AEC-Q100 grade 2); 2,601 in stock, $0.93 | ADC SNR minimum 80 dB (typical only meets 90 dB); extra 1.8 V rail; TI lists a newer part (TAC5112) | [Datasheet SLAS510G](../references/index.md#ti-tlv320aic3104-ds), [-Q1 SLAS715D](../references/index.md#ti-tlv320aic3104-q1-ds), [TI status](../references/index.md#ti-tlv320aic3104-product), [LCSC C181753](https://www.lcsc.com/product-detail/C181753.html) |
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

Only LCSC price and stock were recorded. Digi-Key and Mouser blocked automated
lookups on 2026-09-24, and the maintainer deferred those lookups (2026-09-24);
they don't block this decision.

## Decision

**Codec: TI TLV320AIC3104** (`TLV320AIC3104IRHBR`, catalog grade) on **both
variants**, including variant M (maintainer decision, 2026-09-24). It is the
only candidate that meets every requirement *and* is stocked today; the
catalog part covers −40 to +85 °C, the variant M range. AEC-Q100 is not
required. `TLV320AIC3104IRHBRQ1` is noted only as a pin-identical option
(same pins and package).

- **Alternate 1: TI TAC5112** (`TAC5112IRGER`, `-Q1`: `TAC5112WQRTVRQ1`).
  Technically better and TI's named successor. Switch to it, before the
  revision A schematic is frozen, if it shows stock at two distributors.
- **Alternate 2: TI TLV320AIC3204** (`TLV320AIC3204IRHBR`), stocked.
- Neither alternate is pin-compatible with the AIC3104: changing needs a
  schematic change, not a BOM swap.

**Clocking:** the codec MCLK comes from the **2.304 MHz shared clock
(ADR-0004)**, and the AIC3104 PLL makes exactly 48 kHz with integer settings:
**P = 3, R = 8, J = 16, D = 0** (PLL input 768 kHz, PLL 98.304 MHz; checked
against the limits in [SLAS510G](../references/index.md#ti-tlv320aic3104-ds)
§10.3.3.1, page 27: D = 0 needs 512 kHz–20 MHz after P and 80–110 MHz, J 4–55;
D ≠ 0 would need ≥ 10 MHz and is impossible from 2.304 MHz). The codec is the
I2S master; the ESP32-S3 is the I2S slave. Compared with a 16 MHz MCLK from
the ESP32-S3:

- **Spurious:** no new clock frequency on the board (the 16 MHz net's 9th
  harmonic is 144.000 MHz, in the 2 m band, and its mixing products with the
  buck comb aren't covered by ADR-0004's scan); and supply ripple at the buck's
  8th harmonic (18.432 MHz = 3 × the 6.144 MHz modulator rate) aliases to 0 Hz
  instead of a tone of up to about 550 Hz.
- **Cost:** no extra parts (the clock and its codec branch are in ADR-0004),
  one GPIO freed.
- **Accuracy:** ±20 ppm (ADR-0004's 18.432 MHz oscillator), inside ±50 ppm.

Feeding the codec 18.432 MHz directly (no PLL) was rejected in ADR-0004 for
emissions; the codec gets the ÷ 8 output.

**Fallback:** 16 MHz from the ESP32-S3 (160 MHz ÷ 10; P = 1, R = 1, J = 6,
D = 1440), through a DNP 0 Ω link, if ADR-0004's clock isn't adopted or the
2.304 MHz setting fails on the bench.

**Supplies** ([`audio-codec.md` §7](../research/audio-codec.md#7-supplies)):

| Rail | Source | Part (LCSC, 2026-09-24) |
|---|---|---|
| AVDD, DRVDD | LDO 3.3 → **3.0 V** | TI **LP5907MFX-3.0/NOPB** ([SNVS798Q](../references/index.md#ti-lp5907-ds)), C475492, $0.20 @ 150, 28,780 in stock. Alternate TPS7A2030PDBVR |
| DVDD | LDO 3.0 → **1.8 V**, fed from the AVDD output (keeps DVDD ≤ AVDD and the sequence) | TI **TPS7A2018PDBVR** ([SBVS338H](../references/index.md#ti-tps7a20-ds)), C963430, $0.18 @ 150, 19,785 in stock. Alternate LP5907MFX-1.8/NOPB |
| IOVDD | 3.3 V rail | — |

A ferrite-and-capacitor filter from 3.3 V would save about $0.18 but can't
reject audio-band disturbances (BLE TX bursts), and the codec's own ADC PSRR
is only 44 dB at 1 kHz, so it can't guarantee the SNR target. Running at
3.0 V costs no datasheet performance (full scale and common mode come from an
internal bandgap; specs are at the 1.35 V common mode, allowed from 2.7 V),
but reduces the headroom to the rail from 0.95 V to 0.65 V and must be
confirmed on the bench.

**EU (#59):** the chosen codec, transformer and LDOs are RoHS-compliant, and
TI's part pages show REACH "Yes" for the codec and LDOs (2026-09-24; see
[`audio-codec.md`](../research/audio-codec.md#31-price-and-stock) §3.1, §4, §7.3).

**Transformers: Bourns SM-LP-5001E** (600:600, SMD, tape and reel), one each
for RX and TX. **Alternate:** `SM-LP-5001` (same part in tubes) for sourcing,
and Triad TY-250P as the through-hole fallback. The LM-NP-1001-B1L listed in
[`core-devices.md`](../research/core-devices.md) is rejected on temperature range.

**Isolation fitting** (maintainer decision, 2026-09-24):

| Variant | Transformers (RX, TX) | 0 Ω bypass resistors |
|---|---|---|
| M | Fitted | Not fitted |
| R | **Not fitted (DNP) by default** | **Fitted** |

The board carries both footprints, so either build uses one layout.

**Unresolved conflict:** [constraints §6](../requirements/constraints.md#6-safety-and-fail-safe)
requires AUDIO-jack isolation "on every variant whenever the USB-C data link
is used". A variant R board built with the DNP default has no audio isolation,
so it doesn't meet §6 in wired mode. This ADR doesn't resolve that; it is an
open question for the maintainer (see Consequences).

**Level plan and RF hardening** as in
[`audio-codec.md` §5–6](../research/audio-codec.md#5-rf-hardening-at-the-audio-jack):
a switchable −19.4 dB attenuator on the radio side before the RX transformer;
the codec's PGA and input level control for fine RX level; DAC digital volume
(0 to −63.5 dB, 0.5 dB steps) for TX, with an optional radio-side pad for
microphone-level inputs; ESD, ferrite and C0G filtering at the jack and an RC
low-pass at the codec input. AGC, HPF and effects filters stay at their reset
state (off).

## Consequences

- **Open question for the maintainer: variant R isolation in wired mode.**
  With the DNP default, variant R conflicts with constraints §6 whenever the
  USB-C data link is used. Options:
  1. amend constraints §6 (for example, isolation required only on variant M,
     with the wired-mode ground-loop risk on variant R documented for users);
  2. fit the transformers (and not the bypass resistors) on variant R boards
     sold or configured for wired use;
  3. leave it to the variants decision (#12).
- **Second-distributor sourcing (deferred by the maintainer):** dated Digi-Key
  and Mouser price and stock for `TLV320AIC3104IRHBR`, `TLV320AIC3204IRHBR`,
  `TAC5112IRGER`, `SM-LP-5001E` and `SM-LP-5001` are still to be recorded, to
  meet the two-source rule. Deferred on 2026-09-24; not a condition for
  accepting this ADR.
- **Power (#10, #11):** the codec takes AVDD/DRVDD 3.0 V and DVDD 1.8 V from
  its own two LDOs, and IOVDD 3.3 V. The 3.3 V buck's minimum output must stay
  above about 3.1 V for the LP5907's headroom. The datasheet (§8.5) gives the blocks
  separately (analog + digital): stereo ADC 4.31 + 2.45 mA, DAC to line-out
  4.9 + 2.3 mA, PLL 1.4 + 0.9 mA. Their sum, about 11 mA analog and 6 mA
  digital, is an upper estimate for the budget.
- **Firmware:** ESP32-S3 I2S in slave mode (codec is master); codec PLL at
  P = 3, R = 8, J = 16, D = 0; register setup that leaves AGC, HPF and
  effects off; per-radio RX gain and TX volume stored in
  the configuration; rate matching toward the host clock domains
  (ADR-0008). Measure end-to-end latency and fix it by configuration.
- **Schematic (#12 and the hardware issues):** transformer secondary into the
  codec's differential line input; differential TX drive with a DC-blocking
  capacitor; decide line-out or headphone-driver drive into the 600 Ω winding;
  isolated radio-side ground for the AUDIO jack; filter and ESD values. The
  0 Ω bypass (variant R) must also give a valid single-ended path: the jack
  sleeve joins the device ground, RX tip feeds one leg of the differential
  input with the other leg referenced to ground, and TX drives ring 1 from one
  line output only (0.707 Vrms, 2 Vpp full scale; see
  [`audio-codec.md` §6](../research/audio-codec.md#6-level-plan)).
- **Risks to verify:** AIC3104 ADC SNR on real boards (80 dB minimum), at
  AVDD = 3.0 V; the 2.304 MHz PLL setting (not in the datasheet's example
  table); LP5907 PSRR at 0.3 V
  headroom; transformer distortion at 200 Hz and maximum level; input pin
  swing at 1 Vrms with the −12 dB input level control at 3.0 V; the
  acceptance of ADR-0004 (fallback: 16 MHz from the ESP32-S3);
  AUDIO-jack ESD and immunity levels from EN 301 489-1 **(verify, #59)**;
  RoHS/REACH of `TLV320AIC3104IRHBRQ1` (TI page not reachable); JLCPCB reflow profile versus the SM-LP-5001's 240 °C peak;
  TI's "newer version available" notice on the AIC3104 (still ACTIVE).
- [`core-devices.md`](../research/core-devices.md) is updated to point at this
  ADR for the codec and isolation rows.

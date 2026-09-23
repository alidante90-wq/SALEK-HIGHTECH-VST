# GITI BY SALEK HIGHTECH — DSP / Dependency Report

## Scope
This pass adds no new third-party runtime DSP dependency. SHAE improvements are implemented in original project code and use the existing JUCE audio primitives already present in the repository.

## Research decisions

| Component | Source / reference | License | Decision |
|---|---|---|---|
| JUCE 8.0.4 | JUCE Framework | Dual AGPLv3 / commercial JUCE licensing | Existing dependency; not expanded in this pass |
| Signalsmith DSP | Signalsmith Audio | MIT | Researched; not integrated because the required SHAE primitives could be implemented locally without adding dependency/packaging complexity |
| Signalsmith basics | Signalsmith Audio | MIT | Researched; not integrated |
| Surge XT DSP | Surge Synthesizer | GPL-3.0-or-later | Rejected for direct code integration because the project's free-only/permissive-commercial requirement does not permit introducing a GPL DSP dependency without changing the product licensing strategy |
| MusicDSP band-limited synthesis references | MusicDSP documentation / published examples | Reference material, not copied code | Used only for engineering direction; no source code copied |

## Implemented SHAE components
- Realtime-safe safety stage
- DC blocking
- Finite-value / NaN / infinity protection
- Conditional final ceiling instead of always-on full-signal saturation
- Parameter smoother
- Adaptive anti-alias cutoff helper
- High-frequency oscillator post-filtering after nonlinear wavetable shaping
- Eight macro modulation sources
- Realtime-safe deterministic random modulation source

## Licensing note
JUCE itself remains the repository's existing framework dependency. Its current licensing terms are separate from the SHAE code added in this pass. A fully proprietary/commercial release must use a JUCE licensing path compatible with the intended distribution model; this pass does not change that existing framework dependency.

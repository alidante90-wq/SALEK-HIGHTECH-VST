# GITI BY SALEK HIGHTECH — Dependency & License Report

| Component | Source | License | Why | Commercial OK |
|-----------|--------|---------|-----|---------------|
| **JUCE 8** | juce.com / GitHub | AGPLv3 / Commercial JUCE license | Plugin framework (VST3, UI, audio) | Yes if JUCE license terms met by project owner |
| **SHAE (custom)** | Source/DSP/SHAE.h | Project license (SALEK HIGHTECH) | Osc AA helpers, distortion matrix, formant, OS, master utils | Yes — original code |
| **Factory presets** | Source/PluginProcessorPresets.inl | Project license | 92 production presets | Yes — original design |
| **Embedded UI assets** | Source/Assets/* | Project / user-supplied | Logos, characters, icon atlas | Verify asset ownership before commercial ship |

## Explicitly NOT used
- Serum / Vital / any commercial synth source
- Dolby / THX SDKs
- GPL DSP libraries (would force copyleft on distribution)
- Paid plugin SDKs

## Research notes (not vendored this pass)
- **Signalsmith DSP** — MIT, high quality (delay/FFT/envelopes). Safe to integrate later as header-only; not required for this build.
- **Surge XT** — GPLv3 — studied for ideas only; **not copied**.

## SHAE algorithm references (public domain knowledge / literature)
- PolyBLEP / BLAMP (Välimäki et al. style bandlimited steps)
- ZDF state-variable filter coefficient forms
- Soft clipping / waveshaping gain compensation

## SHAE Spectral / MSEG (this revision)
- Spectral: custom inharmonic comb-resonator bank + freeze/shift/gate (no FFT, no third-party)
- MSEG: custom multi-point envelope with curve modes and shape presets

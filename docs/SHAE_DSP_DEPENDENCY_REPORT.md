# GITI BY SALEK HIGHTECH — SHAE Dependency & DSP Report

## SHAE implementation policy

SHAE currently adds no third-party DSP library. The new DSP work is original project code built on the JUCE audio primitives already present in the repository.

This avoids introducing GPL/copyleft DSP code into the product. Surge XT was evaluated as an architectural/DSP reference only and is not integrated.

## Existing framework

| Component | Version | Role | License / distribution note |
|---|---|---|---|
| JUCE | 8.0.4 | Existing plugin framework, VST3/Standalone, audio primitives | JUCE modules are dual-licensed under AGPLv3 and the JUCE commercial licence. The repository must use a licence path compatible with the intended distribution. No new JUCE dependency was introduced by SHAE. |

## Researched candidates

| Component | Result | Decision |
|---|---|---|
| Signalsmith DSP | MIT | Not integrated in this pass; the required SHAE features were small enough to implement locally without adding a dependency. |
| Signalsmith basics | MIT | Not integrated; existing effect modules are retained. |
| Surge XT DSP | GPL-3.0-or-later | Rejected for integration because the requested product direction requires avoiding copyleft dependency entanglement. Reference/learning only. |
| Proprietary Dolby / THX / commercial synth DSP | Proprietary | Rejected. No proprietary DSP or code is included. |

## SHAE engineering changes

- Adaptive 1x/2x/4x internal oversampling around oscillator nonlinear processing.
- Safer wavetable phase wrapping and finite-value handling.
- DC protection after nonlinear oscillator processing.
- Realtime-safe SafetyStage.
- Realtime-safe parameter smoothing utility.
- Stereo filter state isolation: left and right channels no longer share one stateful filter memory.
- Lightweight SHAE self-test target covering finite-value protection, saturation, smoothing convergence, and output bounds.

## Deliberately not added

No large spectral/granular/resonator dependency was introduced merely to increase the feature list. The existing granular/effects/modulation architecture remains intact while SHAE is established as the reusable DSP foundation.

## Compatibility

The existing manufacturer/plugin codes are intentionally preserved:
- Manufacturer: Salk
- Plugin code: Sht8

The product-facing name remains:
- GITI BY SALEK HIGHTECH

The working Windows/Ninja/MSVC toolchain is preserved.
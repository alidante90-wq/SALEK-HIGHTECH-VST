# GITI BY SALEK HIGHTECH

**Persian Cyber Anime • ISATIS Engine**

Professional JUCE 8 **VST3 + Standalone** wavetable synthesizer for Windows.

## Final UI Design (vFinal)

- Hyper-realistic Anime Persian Cyber style
- 3D-inspired knobs + holographic panels
- Central psychedelic mandala / sonic core visualizer
- **GITI BY SALEK HIGHTECH** large top-left branding
- **ISATIS** large bottom-right branding
- Persian logo «سالک» with geometric frame
- Neon cyan / magenta / gold palette
- OpenGL visualizers + live meters

## Features (real DSP + SHAE)\n\nSHAE (SALEK HIGHTECH AUDIO ENGINE) is integrated into the existing synthesis path. It provides realtime-safe finite-value protection, DC blocking, parameter smoothing, adaptive high-frequency anti-alias filtering around nonlinear oscillator stages, and eight macro modulation sources without replacing the existing UI or preset architecture.

- 3 wavetable oscillators (morph, warp, fold, drive)
- Cross-mod: FM / PM / AM / RM
- Unison 1–7 with detune + stereo spread
- Virus-inspired SVF filter + drive
- ADSR + filter env
- LFO + macros
- FX: Chorus → Delay → Reverb
- Arpeggiator + 16-step sequencer
- Factory presets
- MIDI keyboard

## Tabs

`OSC` · `FILTER` · `ENV` · `MOD` · `FX` · `ARP` · `PRESET`

## Build (Windows)

GitHub Actions builds VST3 + Standalone EXE on every push.

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Note

Original engine and UI. Not a clone of Serum, Vital or Virus TI.

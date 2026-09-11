# SALEK HIGHTECH

Professional JUCE 8 **VST3 + Standalone** wavetable synthesizer for Windows.

## Features (real DSP)

- 3 wavetable oscillators (16-frame morph, warp, fold, drive)
- Cross-mod: FM / PM / AM / RM
- Unison 1–7 with detune + stereo spread
- Virus-inspired SVF filter (LP/HP/BP/Notch) + drive
- ADSR amp envelope + filter env amount
- LFO (5 shapes) + 4 macros
- FX chain: Chorus → Delay → Reverb (+ master drive)
- Arpeggiator + 16-step sequencer
- Factory presets
- MIDI keyboard in UI

## Tabs (Serum-inspired)

`OSC` · `FILTER` · `ENV` · `MOD` · `FX` · `ARP` · `PRESET`

## Build (Windows)

GitHub Actions builds VST3 + Standalone EXE on every push.

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Note

Not a clone of Serum, Vital, or Virus TI. Original engine and UI.

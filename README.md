# SALEK HIGHTECH

Professional JUCE VST3 + Standalone synthesizer for Hi-Tech / Darkpsy / Psytrance / FM / Acid / Alien / Cyberpunk.

## Build

GitHub Actions (Windows) produces `SALEK-HIGHTECH-Windows.zip` with `.vst3` + `.exe`.

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Features

3 wavetable oscs, warp/fold/drive, FM/PM/AM/RM, multimode SVF, LFO, mod matrix, macros, arp, 16-step seq, delay, wavetable lab.

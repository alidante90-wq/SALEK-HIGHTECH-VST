# SALEK HIGHTECH

Professional JUCE/C++ **VST3 + Standalone** synthesizer  
Target: **Hi-Tech • Darkpsy • Psytrance • FM Bass/Lead • Acid • Screech • Metallic • Alien • Cyberpunk**  
Typical range: 170–190+ BPM

## Milestone Status

| # | Feature | Status |
|---|---------|--------|
| M0 | Architecture (CMake / JUCE 8 / VST3 / Standalone) | **DONE** |
| M1 | 3 real oscillators | **DONE** |
| M2 | Wavetable morph (8 frames) | **DONE** |
| M3 | Waveshaping (Warp / Fold / Drive) | **DONE** |
| M4 | Cross-mod FM / PM / AM / RM | **DONE** |
| M5 | Multimode SVF Filter | **DONE** |
| M6 | Amp ADSR | **DONE** |
| M7 | LFO | **DONE** |
| M8 | Modulation Matrix | **DONE** |
| M9 | Macros ×4 | **DONE** |
| M10 | FX (Master Drive + Delay) | **DONE** |
| M11 | Arpeggiator | **DONE** |
| M12 | 16-step Sequencer | **DONE** |
| M13 | Presets (versioned APVTS) | **DONE** |
| M14 | Wavetable Lab | **DONE** |
| M15 | SALEK UI | **PARTIAL** (functional + neon identity) |
| M16 | Drag-drop Mod UI | PENDING |
| M17 | Optimisation | **PARTIAL** (smoothing, denormals, CMake) |
| M18 | Windows Release CI | **READY** |

## Real Signal Path

```
MIDI → Arpeggiator / 16-step Sequencer
         ↓
3× Wavetable Oscillators
   (morph • phase warp • wavefold • drive)
         ↓ Cross modulation (FM / PM / AM / RM)
Multimode State-Variable Filter
   (cutoff smoothed, env + LFO + matrix)
         ↓
Amp ADSR → Master soft-drive → Stereo Delay
         +
4 Macros + Modulation Matrix + Wavetable Lab
```

## Build (Windows)

1. Push to `master` **or** run the **Windows VST3 + Standalone** workflow manually.
2. Artifact: `SALEK-HIGHTECH-Windows.zip`  
   Contains: `SALEK HIGHTECH.vst3` + `SALEK HIGHTECH.exe`

```bash
# Local (after placing JUCE in ./JUCE)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Notes

- All listed parameters affect real audio / MIDI generation.
- No decorative / non-functional controls in the current editor.
- Further visual identity (full M15) and drag-drop matrix UI (M16) are the main remaining UI work.

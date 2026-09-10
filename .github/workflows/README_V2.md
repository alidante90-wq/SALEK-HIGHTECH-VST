# SALEK HIGHTECH V2 — READY BUILD PACK

Independent V2 handoff pack. The original repository is not modified.

The GitHub Actions workflow obtains the public original source as a clean starting point, builds the JUCE/CMake project on Windows, verifies a VST3 was produced, and packages it as an Actions artifact.

## Engineering priorities
- Real-time-safe DSP / no audio-thread allocations
- Stable APVTS parameters and state recall
- Smoothed modulation
- Efficient polyphony and voice stealing
- Wavetable quality / anti-aliasing
- Robust FM / PM / AM / RM
- Stable filter and envelope modulation
- Controlled oversampling
- Versioned presets/state
- Lightweight futuristic GUI
- Genuine Hi-Tech / Darkpsy / Psychedelic / FM Bass / Screech / Metallic / Alien / Laser sound design

## Run
Create the new repository, upload this pack, then:
Actions → SALEK HIGHTECH V2 Build → Run workflow

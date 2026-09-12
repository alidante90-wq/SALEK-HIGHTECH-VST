# OpenGL Sonic Core + VBlank Integration

## Added
- `Source/UI/VisualFifo.h` — lock-free mono ring for viz samples
- `Source/UI/SonicCoreGL.h` — OpenGL waveform + FFT spectrum + cyber rings
- Processor pushes stereo mix into VisualFifo every block
- Editor: VBlankAttachment drives animPhase + pulse; SonicCoreGL center panel

## Manual patch if auto-merge missed processBlock

In `prepareToPlay` add:
```cpp
visualFifo.prepare ((int) sr * 2);
```

At end of `processBlock` after `outputPeak.store (peak);`:
```cpp
visualFifo.pushStereo (buffer);
```

Editor constructor (after glBackdrop):
```cpp
sonicCore = std::make_unique<SonicCoreGL> (processor.getVisualFifo());
addAndMakeVisible (*sonicCore);
vblank = juce::VBlankAttachment (this, [this] (double) {
    animPhase += 0.025f;
    if (sonicCore) {
        sonicCore->setPulse (processor.getOutputPeak());
        sonicCore->setAccent (juce::Colour (0xff00e8ff));
        sonicCore->setAccent2 (juce::Colour (0xffff2d9b));
    }
    if (glBackdrop) glBackdrop->setPulse (processor.getOutputPeak());
    repaint();
});
```

In `resized()` center sonicCore over main area (~420x360).

Rebuild with existing Cyber / Windows workflow.

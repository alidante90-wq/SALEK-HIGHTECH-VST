# OpenGL Sonic Core + VBlank — Integration complete checklist

## Already on main
- `Source/UI/VisualFifo.h`
- `Source/UI/SonicCoreGL.h`
- `Source/PluginProcessor.h` — `getVisualFifo()` + `visualFifo` member
- `Source/PluginProcessor.cpp` — `visualFifo.prepare` + `pushStereo`

## Apply to PluginEditor.h (if not present)

1. Add include:
```cpp
#include "UI/SonicCoreGL.h"
```

2. In private members after `glBackdrop`:
```cpp
std::unique_ptr<SonicCoreGL> sonicCore;
juce::VBlankAttachment vblank;
```

## Apply to PluginEditor.cpp constructor (after glBackdrop create)

```cpp
glBackdrop->setVisible (true);
addAndMakeVisible (*glBackdrop);
sonicCore = std::make_unique<SonicCoreGL> (processor.getVisualFifo());
addAndMakeVisible (*sonicCore);
vblank = juce::VBlankAttachment (this, [this] (double) {
    animPhase += 0.025f;
    if (sonicCore != nullptr) {
        sonicCore->setPulse (processor.getOutputPeak());
        const int th = themeBox.getSelectedId();
        juce::Colour a1 (0xff00e8ff), a2 (0xffff2d9b);
        if (th == 2) { a1 = juce::Colour (0xff39ff14); a2 = juce::Colour (0xffc0ff00); }
        if (th == 3) { a1 = juce::Colour (0xff4fc3f7); a2 = juce::Colour (0xff7c4dff); }
        sonicCore->setAccent (a1);
        sonicCore->setAccent2 (a2);
    }
    if (glBackdrop != nullptr)
        glBackdrop->setPulse (processor.getOutputPeak());
    repaint();
});
```

## resized() — center sonic core

```cpp
if (sonicCore != nullptr) {
    auto core = getLocalBounds().reduced (6);
    core.removeFromLeft (190);
    core.removeFromBottom (90);
    core.removeFromTop (56);
    core = core.withSizeKeepingCentre (juce::jmin (420, core.getWidth()), juce::jmin (360, core.getHeight()));
    sonicCore->setBounds (core);
}
if (glBackdrop != nullptr)
    glBackdrop->setBounds (getLocalBounds());
```

## Destructor
```cpp
vblank = {};
sonicCore.reset();
glBackdrop.reset();
```

Rebuild via **SALEK HIGHTECH Cyber Build** workflow.

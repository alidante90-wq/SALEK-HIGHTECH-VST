# Finish PluginEditor.cpp (4 quick edits on GitHub)

Open: https://github.com/alidante90-wq/SALEK-HIGHTECH-VST/edit/main/Source/PluginEditor.cpp

## 1) Constructor — find this block near the top:

```
glBackdrop = std::make_unique<OpenGLGridBackdrop>();
glBackdrop->setVisible (false);
addAndMakeVisible (keyboard);
startTimerHz (30);
```

**Replace with:**

```cpp
glBackdrop = std::make_unique<OpenGLGridBackdrop>();
glBackdrop->setVisible (true);
glBackdrop->setInterceptsMouseClicks (false, false);
addAndMakeVisible (*glBackdrop);
sonicCore = std::make_unique<SonicCoreGL> (processor.getVisualFifo());
addAndMakeVisible (*sonicCore);
addAndMakeVisible (keyboard);
vblank = juce::VBlankAttachment (this, [this] (double) {
    animPhase += 0.025f;
    if (sonicCore != nullptr)
    {
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
startTimerHz (15);
```

## 2) Destructor — replace body with:

```cpp
stopTimer();
vblank = {};
sonicCore.reset();
glBackdrop.reset();
setLookAndFeel (nullptr);
```

## 3) timerCallback — replace with:

```cpp
void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 40) resized();
}
```

## 4) resized() — right after `glBackdrop->setBounds(...)` and `auto full = getLocalBounds().reduced (6);` add:

```cpp
if (sonicCore != nullptr)
{
    auto core = full;
    core.removeFromLeft (190);
    core.removeFromBottom (90);
    core.removeFromTop (56);
    core = core.withSizeKeepingCentre (juce::jmin (420, core.getWidth()),
                                       juce::jmin (360, core.getHeight()));
    sonicCore->setBounds (core);
}
```

Commit on main. Then run **Actions → SALEK HIGHTECH Cyber Build**.

PluginEditor.h is already updated on main.

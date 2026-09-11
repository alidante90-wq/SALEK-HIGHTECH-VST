#pragma once
#include <JuceHeader.h>
namespace SalekAssets {
inline juce::Image loadLogo() {
    juce::Image img (juce::Image::ARGB, 64, 64, true);
    juce::Graphics g (img);
    g.setColour (juce::Colour (0xff00f0ff));
    g.drawEllipse (4.0f, 4.0f, 56.0f, 56.0f, 3.0f);
    g.setColour (juce::Colour (0xffff2d6a));
    g.drawEllipse (14.0f, 14.0f, 36.0f, 36.0f, 2.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("S", 0, 0, 64, 64, juce::Justification::centred);
    return img;
}
/** Placeholder hero art (replace with BinaryData later). */
inline juce::Image loadHero() {
    juce::Image img (juce::Image::ARGB, 128, 160, true);
    juce::Graphics g (img);
    juce::ColourGradient grad (juce::Colour (0xff2a0a40), 64.0f, 0.0f,
                               juce::Colour (0xff0a0614), 64.0f, 160.0f, false);
    g.setGradientFill (grad);
    g.fillAll();
    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.5f));
    g.drawEllipse (24.0f, 20.0f, 80.0f, 100.0f, 2.0f);
    g.setColour (juce::Colour (0xffff2d6a).withAlpha (0.6f));
    g.fillEllipse (48.0f, 48.0f, 12.0f, 10.0f);
    g.fillEllipse (68.0f, 48.0f, 12.0f, 10.0f);
    g.setColour (juce::Colours::white.withAlpha (0.4f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("SALEK", 0, 130, 128, 20, juce::Justification::centred);
    return img;
}
}

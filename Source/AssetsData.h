#pragma once
#include <JuceHeader.h>
namespace SalekAssets {
inline juce::Image loadLogo() {
    // Minimal brand mark placeholder (full art can replace later)
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
}

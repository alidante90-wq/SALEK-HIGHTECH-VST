#pragma once
#include <JuceHeader.h>

namespace SalekAssets
{
/** Brand mark — neon SALEK hexagon badge (no external file needed). */
inline juce::Image loadLogo()
{
    juce::Image img (juce::Image::ARGB, 128, 128, true);
    juce::Graphics g (img);
    const float cx = 64.0f, cy = 64.0f;

    for (int i = 5; i >= 1; --i)
    {
        g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.04f * (float) i));
        g.fillEllipse (cx - 50 - i * 2, cy - 50 - i * 2, 100 + i * 4, 100 + i * 4);
    }

    juce::Path hex;
    for (int i = 0; i < 6; ++i)
    {
        float a = juce::MathConstants<float>::twoPi * (float) i / 6.0f - juce::MathConstants<float>::halfPi;
        float x = cx + std::cos (a) * 46.0f;
        float y = cy + std::sin (a) * 46.0f;
        if (i == 0) hex.startNewSubPath (x, y);
        else hex.lineTo (x, y);
    }
    hex.closeSubPath();
    juce::ColourGradient body (juce::Colour (0xff2a1040), cx, cy - 40, juce::Colour (0xff0a0614), cx, cy + 40, false);
    g.setGradientFill (body);
    g.fillPath (hex);
    g.setColour (juce::Colour (0xff00f0ff));
    g.strokePath (hex, juce::PathStrokeType (2.5f));
    g.setColour (juce::Colour (0xffff2d6a).withAlpha (0.7f));
    g.strokePath (hex, juce::PathStrokeType (1.0f));

    juce::Path dia;
    dia.startNewSubPath (cx, cy - 22);
    dia.lineTo (cx + 22, cy);
    dia.lineTo (cx, cy + 22);
    dia.lineTo (cx - 22, cy);
    dia.closeSubPath();
    g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.25f));
    g.fillPath (dia);
    g.setColour (juce::Colour (0xffff00aa));
    g.strokePath (dia, juce::PathStrokeType (1.5f));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("SALEK", 0, 52, 128, 18, juce::Justification::centred);
    g.setColour (juce::Colour (0xff00f0ff));
    g.setFont (juce::FontOptions (9.0f));
    g.drawText ("HIGHTECH", 0, 70, 128, 14, juce::Justification::centred);

    return img;
}
inline juce::Image loadHero() { return loadLogo(); }
}

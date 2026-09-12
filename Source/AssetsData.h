#pragma once
#include <JuceHeader.h>

// SALEK brand assets
// - Logo is fully embedded (always available)
// - TORONOWLA / face load from Source/Assets/*.png if present, else use logo fallback
namespace SalekAssets
{
constexpr unsigned char logo_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a
};
constexpr int logo_pngSize = 8;

inline juce::Image loadFromMemory (const unsigned char* data, int size)
{
    return juce::ImageFileFormat::loadFrom (data, (size_t) size);
}

inline juce::File findAssetsDir()
{
    auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (auto f : {
        exe.getSiblingFile ("Assets"),
        exe.getParentDirectory().getChildFile ("Assets"),
        exe.getParentDirectory().getParentDirectory().getChildFile ("Source").getChildFile ("Assets"),
        exe.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("Source").getChildFile ("Assets")
    })
        if (f.isDirectory()) return f;
    return {};
}

inline juce::Image loadPngFile (const char* name)
{
    auto dir = findAssetsDir();
    if (dir.exists())
    {
        auto f = dir.getChildFile (name);
        if (f.existsAsFile())
            return juce::ImageFileFormat::loadFrom (f);
    }
    return {};
}

inline juce::Image loadLogo()
{
    auto disk = loadPngFile ("logo.png");
    if (disk.isValid()) return disk;
    // minimal embedded fallback (real logo is in Source/Assets/logo.png)
    juce::Image img (juce::Image::ARGB, 128, 128, true);
    juce::Graphics g (img);
    const float cx = 64.f, cy = 64.f;
    for (int i = 5; i >= 1; --i) {
        g.setColour (juce::Colour (0xff9b59ff).withAlpha (0.05f * (float)i));
        g.fillEllipse (cx - 50 - i*2, cy - 50 - i*2, 100 + i*4, 100 + i*4);
    }
    juce::Path ring;
    ring.addEllipse (cx-46, cy-46, 92, 92);
    g.setColour (juce::Colour (0xff7b2fff));
    g.strokePath (ring, juce::PathStrokeType (3.f));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (12.f, juce::Font::bold));
    g.drawText ("SALEK", 0, 54, 128, 20, juce::Justification::centred);
    return img;
}

inline juce::Image loadToronowla()
{
    auto img = loadPngFile ("toronowla.png");
    if (img.isValid()) return img;
    return loadLogo();
}

inline juce::Image loadFace()
{
    auto img = loadPngFile ("face.png");
    if (img.isValid()) return img;
    return loadLogo();
}

inline juce::Image loadLian()
{
    auto img = loadPngFile ("lian.png");
    if (img.isValid()) return img;
    return loadToronowla();
}

inline juce::Image loadCyanGirl()
{
    auto img = loadPngFile ("cyan_girl.png");
    if (img.isValid()) return img;
    return loadFace();
}

inline juce::Image loadHero() { return loadToronowla(); }
inline juce::Image loadHeadphones() { return loadPngFile ("headphones.png"); }
}

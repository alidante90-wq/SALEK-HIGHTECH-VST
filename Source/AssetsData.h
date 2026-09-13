#pragma once
#include <JuceHeader.h>

#if __has_include(<BinaryData.h>)
 #include <BinaryData.h>
 #define SALEK_HAS_BINARY_DATA 1
#else
 #define SALEK_HAS_BINARY_DATA 0
#endif

namespace SalekAssets
{
inline juce::Image fromMemory (const void* data, int size)
{
    if (data == nullptr || size <= 64) return {};
    return juce::ImageFileFormat::loadFrom (data, (size_t) size);
}

inline juce::File findAssetsDir()
{
    auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (auto f : {
        exe.getSiblingFile ("Assets"),
        exe.getParentDirectory().getChildFile ("Assets"),
        exe.getParentDirectory().getParentDirectory().getChildFile ("Source/Assets"),
        juce::File ("Source/Assets"),
        juce::File ("../Source/Assets")
    })
        if (f.isDirectory()) return f;
    return {};
}

inline juce::Image fromDisk (std::initializer_list<const char*> names)
{
    auto dir = findAssetsDir();
    if (! dir.exists()) return {};
    for (auto* name : names)
    {
        auto f = dir.getChildFile (name);
        if (f.existsAsFile() && f.getSize() > 64)
        {
            auto img = juce::ImageFileFormat::loadFrom (f);
            if (img.isValid()) return img;
        }
    }
    return {};
}

inline juce::Image makeFallbackLogo()
{
    juce::Image img (juce::Image::ARGB, 256, 256, true);
    juce::Graphics g (img);
    juce::ColourGradient gr (juce::Colour (0xff00e8ff), 128, 40, juce::Colour (0xffff2d9b), 128, 220, false);
    g.setGradientFill (gr);
    g.fillEllipse (24.f, 24.f, 208.f, 208.f);
    g.setColour (juce::Colour (0xff0a0614));
    g.fillEllipse (48.f, 48.f, 160.f, 160.f);
    g.setColour (juce::Colour (0xffffd700));
    g.setFont (juce::FontOptions (28.f, juce::Font::bold));
    g.drawText ("SALEK", 0, 110, 256, 36, juce::Justification::centred);
    return img;
}

#if SALEK_HAS_BINARY_DATA
inline juce::Image fromBinaryName (const char* name)
{
    int sz = 0;
    const char* data = BinaryData::getNamedResource (name, sz);
    return fromMemory (data, sz);
}

inline juce::Image anyEmbedded()
{
    for (auto* n : { "face_png", "lian_png", "wp5627062_jpg",
                     "purple_haired_girl_with_ribbon_by_tuwalg_dg4rfhqfullview_jpg",
                     "b4ef89321df2b8f2c7bd314d352b8f42_jpg", "OIP_webp" })
    {
        auto img = fromBinaryName (n);
        if (img.isValid()) return img;
    }
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        int sz = 0;
        const char* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], sz);
        auto img = fromMemory (data, sz);
        if (img.isValid()) return img;
    }
    return {};
}
#else
inline juce::Image anyEmbedded() { return {}; }
#endif

inline juce::Image loadLogo()
{
    auto img = anyEmbedded();
    if (img.isValid()) return img;
    img = fromDisk ({ "face.png", "lian.png", "OIP.webp", "b4ef89321df2b8f2c7bd314d352b8f42.jpg" });
    return img.isValid() ? img : makeFallbackLogo();
}

inline juce::Image loadToronowla()
{
    auto img = anyEmbedded();
    if (img.isValid()) return img;
    img = fromDisk ({ "face.png", "lian.png", "akhd 1.png", "wp5627062.jpg",
                      "purple_haired_girl_with_ribbon_by_tuwalg_dg4rfhq-fullview.jpg" });
    return img.isValid() ? img : loadLogo();
}

inline juce::Image loadFace()
{
    auto img = fromDisk ({ "face.png" });
    if (img.isValid()) return img;
    return loadToronowla();
}

inline juce::Image loadLian()
{
    auto img = fromDisk ({ "lian.png" });
    return img.isValid() ? img : loadToronowla();
}

inline juce::Image loadCyanGirl()
{
    auto img = fromDisk ({ "tumblr_rorujgb5gF1tpwuis_smart1.jpg", "face.png" });
    return img.isValid() ? img : loadFace();
}

inline juce::Image loadHero() { return loadToronowla(); }
}

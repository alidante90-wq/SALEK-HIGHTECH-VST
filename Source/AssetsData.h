#pragma once
#include <JuceHeader.h>

// SALEK brand assets — loads from Source/Assets (or next to the binary)
namespace SalekAssets
{
inline juce::File findAssetsDir()
{
    auto exe = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (auto f : {
        exe.getSiblingFile ("Assets"),
        exe.getParentDirectory().getChildFile ("Assets"),
        exe.getParentDirectory().getParentDirectory().getChildFile ("Source").getChildFile ("Assets"),
        exe.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("Source").getChildFile ("Assets"),
        juce::File ("Source/Assets"),
        juce::File ("../Source/Assets")
    })
        if (f.isDirectory()) return f;
    return {};
}

inline juce::Image loadNamed (std::initializer_list<const char*> names)
{
    auto dir = findAssetsDir();
    if (dir.exists())
    {
        for (auto* name : names)
        {
            auto f = dir.getChildFile (name);
            if (f.existsAsFile())
            {
                auto img = juce::ImageFileFormat::loadFrom (f);
                if (img.isValid()) return img;
            }
        }
        for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.png;*.jpg;*.jpeg;*.webp"))
        {
            auto img = juce::ImageFileFormat::loadFrom (f);
            if (img.isValid()) return img;
        }
    }
    return {};
}

inline juce::Image makeFallbackLogo()
{
    juce::Image img (juce::Image::ARGB, 128, 128, true);
    juce::Graphics g (img);
    g.setColour (juce::Colour (0xff9b59ff));
    g.drawEllipse (16.f, 16.f, 96.f, 96.f, 3.f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.f, juce::Font::bold));
    g.drawText ("SALEK", 0, 54, 128, 20, juce::Justification::centred);
    return img;
}

inline juce::Image loadLogo()
{
    auto img = loadNamed ({ "logo.png", "LOGO BACK.png", "OIP.webp", "akhd 1.png" });
    return img.isValid() ? img : makeFallbackLogo();
}

inline juce::Image loadToronowla()
{
    auto img = loadNamed ({ "toronowla.png", "TOROMOWLA-final.jpg", "Redesign the uploade.png", "TORONOWLA.png" });
    return img.isValid() ? img : loadLogo();
}

inline juce::Image loadFace()
{
    auto img = loadNamed ({ "face.png", "dfz4fjc-7dc3ec11-4c32-4f22-965a-a6bd4b2f9ff8.png" });
    return img.isValid() ? img : loadLogo();
}

inline juce::Image loadLian()
{
    auto img = loadNamed ({ "lian.png", "lian-final.jpg" });
    return img.isValid() ? img : loadToronowla();
}

inline juce::Image loadCyanGirl()
{
    auto img = loadNamed ({ "cyan_girl.png", "tumblr_rorujgb5gF1tpwuis_smart1.jpg", "purple_haired_girl_with_ribbon_by_tuwalg_dg4rfhq-fullview.jpg" });
    return img.isValid() ? img : loadFace();
}

inline juce::Image loadHero() { return loadToronowla(); }
inline juce::Image loadHeadphones() { return loadNamed ({ "headphones.png", "wp5627062.jpg" }); }
}

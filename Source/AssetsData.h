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
    juce::Array<juce::File> candidates;
    candidates.add (exe.getSiblingFile ("Assets"));
    candidates.add (exe.getParentDirectory().getChildFile ("Assets"));
    candidates.add (exe.getParentDirectory().getParentDirectory().getChildFile ("Assets"));
    candidates.add (exe.getParentDirectory().getParentDirectory().getChildFile ("Resources/Assets"));
    candidates.add (exe.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("Assets"));
    candidates.add (exe.getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("Source/Assets"));
    candidates.add (exe.getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getChildFile ("Source/Assets"));
    candidates.add (juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("SALEK-HIGHTECH/Assets"));
    candidates.add (juce::File ("Source/Assets"));
    candidates.add (juce::File ("../Source/Assets"));
    candidates.add (juce::File ("../../Source/Assets"));
    for (auto& f : candidates)
        if (f.isDirectory())
            if (f.getChildFile ("logo_giti.png").existsAsFile()
                || f.getChildFile ("char_catgirl.png").existsAsFile()
                || f.getChildFile ("bg_isatis.jpg").existsAsFile())
                return f;
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
    for (auto* n : { "logo_giti_png", "bg_isatis_jpg", "bg_salek_jpg",
                     "face_png", "lian_png", "wp5627062_jpg" })
    {
        auto img = fromBinaryName (n);
        if (img.isValid()) return img;
    }
    return {};
}
#else
inline juce::Image anyEmbedded() { return {}; }
#endif

// ---- All loaders: single juce::Image img (MSVC C2374 safe) ----

inline juce::Image loadLogoGiti()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("logo_giti_png");
    if (img.isValid()) return img;
#endif
    img = fromDisk ({ "logo_giti.png", "LOGO.png" });
    return img.isValid() ? img : makeFallbackLogo();
}

inline juce::Image loadLogo()
{
    return loadLogoGiti();
}

inline juce::Image loadBgIsatis()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("bg_isatis_jpg");
    if (img.isValid()) return img;
#endif
    img = fromDisk ({ "bg_isatis.jpg", "bg_isatis.png" });
    return img;
}

inline juce::Image loadBgSalek()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("bg_salek_jpg");
    if (img.isValid()) return img;
#endif
    img = fromDisk ({ "bg_salek.jpg", "bg_salek.png" });
    return img;
}

inline juce::Image loadFace()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("face_png");
    if (img.isValid()) return img;
#endif
    return fromDisk ({ "face.png" });
}

inline juce::Image loadLian()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("lian_png");
    if (img.isValid()) return img;
#endif
    img = fromDisk ({ "lian.png" });
    return img.isValid() ? img : loadFace();
}

inline juce::Image loadToronowla()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("wp5627062_jpg");
    if (img.isValid()) return img;
#endif
    return fromDisk ({ "wp5627062.jpg", "toronowla.png" });
}

inline juce::Image loadCyanGirl()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("purple_haired_girl_with_ribbon_by_tuwalg_dg4rfhqfullview_jpg");
    if (img.isValid()) return img;
#endif
    img = fromDisk ({ "purple_haired_girl_with_ribbon_by_tuwalg_dg4rfhq-fullview.jpg", "OIP.webp" });
    return img.isValid() ? img : loadFace();
}

inline juce::Image loadCharPurple()  { return fromDisk ({ "char_purple_latex.png", "char_purple_latex.png.png", "char_purple.png" }); }
inline juce::Image loadCharCatgirl() { return fromDisk ({ "char_catgirl.png" }); }
inline juce::Image loadCharFox()     { return fromDisk ({ "char_foxgirl.png", "char_fox.png" }); }
inline juce::Image loadCharCyber()   { return fromDisk ({ "char_cyber_white.png", "char_cyber.png" }); }
inline juce::Image loadCharGun()     { return fromDisk ({ "char_gun.png" }); }
inline juce::Image loadCharApron()   { return fromDisk ({ "char_apron.png" }); }
inline juce::Image loadCharWhite()   { return fromDisk ({ "char_white_suit.png", "char_white.png" }); }
inline juce::Image loadCharToronowla()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("char_toronowla_png");
    if (img.isValid()) return img;
#endif
    return fromDisk ({ "char_toronowla.png" });
}
inline juce::Image loadCharNeonStreet()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("char_neon_street_png");
    if (img.isValid()) return img;
#endif
    return fromDisk ({ "char_neon_street.png" });
}
inline juce::Image loadCharPinkDead()
{
    juce::Image img;
#if SALEK_HAS_BINARY_DATA
    img = fromBinaryName ("char_pink_dead_png");
    if (img.isValid()) return img;
#endif
    return fromDisk ({ "char_pink_dead.png" });
}

/** Only 3 official models — keeps plugin size small */
inline juce::Image loadCharPortrait (int index = 0)
{
    juce::Image imgs[] = {
        loadCharToronowla(), loadCharNeonStreet(), loadCharPinkDead()
    };
    const int n = 3;
    for (int k = 0; k < n; ++k)
    {
        auto& im = imgs[(index + k) % n];
        if (im.isValid()) return im;
    }
    return {};
}

inline juce::Image loadHero() { return loadBgIsatis(); }
} // namespace SalekAssets

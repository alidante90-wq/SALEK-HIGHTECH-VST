#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"

namespace giti
{
inline void showAbout (juce::Component* parent, const juce::String& trialStatus)
{
    const auto version = juce::String (ProjectInfo::versionString);
    const auto& edition = giti::currentEdition();

    const juce::String message =
        "GITI BY SALEK HIGHTECH\n"
        "DIGITAL INSTRUMENT UNIVERSE\n\n"
        "GITI is an original high-tech synthesizer and sound-design instrument "
        "created by ALI AGHAKOOCHAK AKA SALEK.\n\n"
        "Designed for high-tech, psytrance, darkpsy, experimental electronic music "
        "and modern sound design, GITI combines synthesis, modulation, sequencing, "
        "creative FX and performance controls in one cyber-futuristic instrument.\n\n"
        "CREATOR\n"
        "ALI AGHAKOOCHAK AKA SALEK\n"
        "High-Tech Psytrance Producer • Sound Designer • Songwriter\n\n"
        "PRODUCT\n"
        "GITI BY SALEK HIGHTECH\n"
        "Software Version: " + version + "\n"
        "Genesis Edition: " + giti::editionId() + "\n"
        "Edition Code: " + juce::String (edition.code) + "\n"
        "Edition Name: " + juce::String (edition.name) + "\n"
        "Signature: " + giti::editionSignature() + "\n"
        "Platform: Windows VST3 / Standalone\n\n"
        "TRIAL / LICENSE\n" + trialStatus + "\n\n"
        "© 2026 Ali Aghakoochak / SALEK\n"
        "All rights reserved.\n"
        "GITI and GITI BY SALEK HIGHTECH are product names of the creator.";

    juce::AlertWindow::showAsync (
        juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::InfoIcon)
            .withTitle ("GITI • ABOUT • " + giti::editionId())
            .withMessage (message)
            .withButton ("CLOSE")
            .withAssociatedComponent (parent),
        nullptr);
}
} // namespace giti

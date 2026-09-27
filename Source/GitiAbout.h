#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"

namespace giti
{
inline void showAbout (juce::Component* parent, const juce::String& trialStatus)
{
    const juce::String message =
        "GITI BY SALEK HIGHTECH\n\n"
        "GITI is an original high-tech synthesizer and sound-design instrument "
        "created by ALI AGHAKOOCHAK AKA SALEK.\n\n"
        "Designed for high-tech, psytrance, darkpsy, experimental electronic music "
        "and modern sound design, GITI combines synthesis, modulation, sequencing, "
        "creative FX and performance-oriented controls in one cyber-futuristic instrument.\n\n"
        "CREATOR\n"
        "ALI AGHAKOOCHAK AKA SALEK\n"
        "High-Tech Psytrance Producer • Sound Designer • Songwriter\n\n"
        "PRODUCT\n"
        "GITI BY SALEK HIGHTECH\n"
        "Edition " + giti::editionId() + " • " + giti::editionName() + "\n"
        "Signature: " + giti::editionSignature() + "\n"
        "Version 1.0.0 • Windows VST3 / Standalone\n\n"
        "© 2026 Ali Aghakoochak / SALEK. All rights reserved.\n"
        "GITI and GITI BY SALEK HIGHTECH are product names of the creator.\n\n"
        + trialStatus;

    juce::AlertWindow::showAsync (
        juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::InfoIcon)
            .withTitle ("GITI • ABOUT")
            .withMessage (message)
            .withButton ("CLOSE")
            .withAssociatedComponent (parent),
        nullptr);
}
} // namespace giti

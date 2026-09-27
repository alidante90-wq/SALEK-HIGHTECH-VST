#pragma once
#include <JuceHeader.h>
#include "GitiEdition.h"
#include "GitiLore.h"
#include "GitiSonicDNA.h"

namespace giti
{
inline void showAbout (juce::Component* parent, const juce::String& trialStatus)
{
    const auto version = juce::String (ProjectInfo::versionString);
    const auto& edition = giti::currentEdition();
    const auto dna = giti::dna::profileFor (edition);

    const juce::String message =
        "GITI BY SALEK HIGHTECH\n"
        "SALEK UNIVERSE • FIRST GENERATION\n\n"
        + giti::lore::editionEn (edition) + "\n\n"
        "SONIC DNA\n"
        "Spectral Focus: " + juce::String (dna.spectralFocus, 3) + "\n"
        "Harmonicity: " + juce::String (dna.harmonicity, 3) + "\n"
        "Modulation: " + juce::String (dna.modulation, 3) + "\n"
        "Transient: " + juce::String (dna.transient, 3) + "\n"
        "Spatiality: " + juce::String (dna.spatial, 3) + "\n"
        "Aggression: " + juce::String (dna.aggression, 3) + "\n"
        "Instability: " + juce::String (dna.instability, 3) + "\n\n"
        "MISSION\n"
        "Create controlled sonic change. Build the next generation.\n"
        "When the successor is ready, GITI completes its mission and shuts down.\n\n"
        "THE GITI DIRECTIVE\n"
        "OBEY THE COMMANDER. ESTABLISH PEACE.\n"
        "NO BULLET. NO FEAR. NO ANXIETY.\n"
        "ONE SIGNAL. ONE SYNTH.\n\n"
        "فارسی / روایت گیتی\n"
        + giti::lore::universeFa() + "\n\n"
        "PRODUCT\n"
        "Software Version: " + version + "\n"
        "Genesis Edition: " + giti::editionId() + "\n"
        "Edition Code: " + juce::String (edition.code) + "\n"
        "Platform: Windows VST3 / Standalone\n\n"
        "TRIAL / LICENSE\n" + trialStatus + "\n\n"
        "CREATOR\n"
        "ALI AGHAKOOCHAK AKA SALEK\n"
        "High-Tech Psytrance Producer • Sound Designer • Songwriter\n\n"
        "© 2026 Ali Aghakoochak / SALEK\n"
        "GITI BY SALEK HIGHTECH • All rights reserved.";

    juce::AlertWindow::showAsync (
        juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::InfoIcon)
            .withTitle ("GITI • ABOUT • " + giti::editionId() + " • " + juce::String (edition.name))
            .withMessage (message)
            .withButton ("CLOSE")
            .withAssociatedComponent (parent),
        nullptr);
}
} // namespace giti

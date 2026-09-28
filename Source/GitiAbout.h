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

    auto* alert = new juce::AlertWindow (
        "GITI • ABOUT • " + giti::editionId() + " • " + juce::String (edition.name),
        "",
        juce::MessageBoxIconType::InfoIcon);
    alert->addTextEditor ("aboutText", message, "", false);
    if (auto* editor = alert->getTextEditor ("aboutText"))
    {
        editor->setMultiLine (true, true);
        editor->setReadOnly (true);
        editor->setScrollbarsShown (true);
        editor->setCaretVisible (false);
        editor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff090611));
        editor->setColour (juce::TextEditor::textColourId, juce::Colour (0xffd9d2ee));
    }
    alert->addButton ("CLOSE", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
    alert->setSize (760, 560);
    alert->setAlwaysOnTop (true);
    alert->enterModalState (true, juce::ModalCallbackFunction::create (
        [alert] (int) { delete alert; }), true);
}
} // namespace giti

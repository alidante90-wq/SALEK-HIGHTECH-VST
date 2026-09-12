#include <cmath>
#include "PluginProcessor.h"
#include "PluginEditor.h"

SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    initFactoryPresets();
}

// --- Parameter layout, presets, loadFactoryPreset, getPresetNames, setCurrentProgram, getProgramName
// are unchanged from previous main; this restore focuses on prepareToPlay + processBlock + createPluginFilter.
// FULL BODY restored from commit f0b65fd + VisualFifo lines.

#include "PluginProcessor_Body.inl"

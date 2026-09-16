
SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    initFactoryPresets();
}

// NOTE: Full body continues in repo - if this is truncated the previous complete file must be restored.
// Placeholder marker check:
// FULL_BODY_MARKER

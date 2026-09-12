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

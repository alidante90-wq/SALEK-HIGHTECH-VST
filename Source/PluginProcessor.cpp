#include <cmath>
#include "PluginProcessor.h"
#include "License/SalekLicense.h"
#include "PluginEditor.h"
#include "PluginProcessor_full_p1.inl"
#include "PluginProcessor_full_p2.inl"

bool SalekHightechAudioProcessor::isEngineUnlocked() const
{
    return SalekLicense::isLicensed();
}
juce::String SalekHightechAudioProcessor::getMachineIdForLicense() const
{
    return SalekLicense::getMachineId();
}
juce::String SalekHightechAudioProcessor::tryLicenseActivate (const juce::String& code)
{
    return SalekLicense::tryActivate (code);
}

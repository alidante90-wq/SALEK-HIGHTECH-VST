void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };
    #include "PluginProcessorPresetsData.inl"
    #include "PluginProcessorPresetsExtra.inl"
}

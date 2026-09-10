#include "PluginProcessor.h"
#include "PluginEditor.h"

SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) {}

juce::AudioProcessorValueTreeState::ParameterLayout SalekHightechAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    auto addF = [&](const char* id, const char* name, float min, float max, float def) {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, name, juce::NormalisableRange<float>(min,max), def));
    };
    auto addI = [&](const char* id, const char* name, int min, int max, int def) {
        params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id,1}, name, min, max, def));
    };
    addF("osc1_level","OSC1 Level",0,1,0.7f); addF("osc2_level","OSC2 Level",0,1,0.5f); addF("osc3_level","OSC3 Level",0,1,0.4f);
    addF("osc1_table","OSC1 Table",0,1,0); addF("osc2_table","OSC2 Table",0,1,0.33f); addF("osc3_table","OSC3 Table",0,1,0.66f);
    addF("osc1_warp","OSC1 Warp",0,1,0); addF("osc2_warp","OSC2 Warp",0,1,0); addF("osc3_warp","OSC3 Warp",0,1,0);
    addF("osc1_fold","OSC1 Fold",0,1,0); addF("osc2_fold","OSC2 Fold",0,1,0); addF("osc3_fold","OSC3 Fold",0,1,0);
    addF("osc1_drive","OSC1 Drive",0,1,0); addF("osc2_drive","OSC2 Drive",0,1,0); addF("osc3_drive","OSC3 Drive",0,1,0);
    addF("fm_2to1","FM 2to1",0,1,0); addF("fm_3to1","FM 3to1",0,1,0); addF("rm_2to1","RM 2to1",0,1,0);
    addF("filter_cutoff","Filter Cutoff",20,20000,8000); addF("filter_reso","Filter Reso",0,1,0.3f);
    addF("filter_drive","Filter Drive",0,1,0); addI("filter_mode","Filter Mode",0,3,0); addF("filter_env","Filter Env",0,1,0.4f);
    addF("lfo_rate","LFO Rate",0.01f,40,2); addF("lfo_amount","LFO Amount",0,1,0);
    addF("macro1","Macro 1",0,1,0); addF("delay_mix","Delay Mix",0,1,0); addF("master_drive","Master Drive",0,1,0);
    return {params.begin(), params.end()};
}

void SalekHightechAudioProcessor::prepareToPlay(double sr, int spb) {
    synthEngine.prepareToPlay(sr, spb);
    delay.prepare(sr, spb);
}

bool SalekHightechAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    auto set = layouts.getMainOutputChannelSet();
    return set == juce::AudioChannelSet::mono() || set == juce::AudioChannelSet::stereo();
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    auto get = [&](const char* id) -> float {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return 0.f;
    };
    synthEngine.setOsc1Level(get("osc1_level")); synthEngine.setOsc2Level(get("osc2_level")); synthEngine.setOsc3Level(get("osc3_level"));
    synthEngine.setOsc1TablePos(get("osc1_table")); synthEngine.setOsc2TablePos(get("osc2_table")); synthEngine.setOsc3TablePos(get("osc3_table"));
    synthEngine.setOsc1Warp(get("osc1_warp")); synthEngine.setOsc2Warp(get("osc2_warp")); synthEngine.setOsc3Warp(get("osc3_warp"));
    synthEngine.setOsc1Fold(get("osc1_fold")); synthEngine.setOsc2Fold(get("osc2_fold")); synthEngine.setOsc3Fold(get("osc3_fold"));
    synthEngine.setOsc1Drive(get("osc1_drive")); synthEngine.setOsc2Drive(get("osc2_drive")); synthEngine.setOsc3Drive(get("osc3_drive"));
    synthEngine.setFm2to1(get("fm_2to1")); synthEngine.setFm3to1(get("fm_3to1")); synthEngine.setRm2to1(get("rm_2to1"));
    synthEngine.setFilterCutoff(get("filter_cutoff")); synthEngine.setFilterResonance(get("filter_reso"));
    synthEngine.setFilterDrive(get("filter_drive")); synthEngine.setFilterMode(int(get("filter_mode")));
    synthEngine.setFilterEnvAmt(get("filter_env"));
    synthEngine.setLfoRate(get("lfo_rate")); synthEngine.setLfoAmount(get("lfo_amount"));
    float m1 = get("macro1");
    synthEngine.setFilterCutoff(get("filter_cutoff") * (0.3f + 0.7f*(1-m1) + m1*2.5f));
    synthEngine.processBlock(buffer, midi);
    float md = get("master_drive");
    if (md > 1e-4f) {
        float g = 1 + md * 4;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) d[i] = std::tanh(d[i] * g) * 0.9f;
        }
    }
    delay.setMix(get("delay_mix"));
    delay.process(buffer);
}

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor() {
    return new SalekHightechAudioProcessorEditor(*this);
}
void SalekHightechAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}
void SalekHightechAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new SalekHightechAudioProcessor();
}

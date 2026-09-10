#include "PluginProcessor.h"
#include "PluginEditor.h"

SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    initFactoryPresets();
}

juce::AudioProcessorValueTreeState::ParameterLayout SalekHightechAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto F = [&](const char* id, const char* n, float a, float b, float d) {
        p.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1}, n, juce::NormalisableRange<float>(a,b), d)); };
    auto I = [&](const char* id, const char* n, int a, int b, int d) {
        p.push_back (std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id,1}, n, a, b, d)); };
    auto C = [&](const char* id, const char* n, juce::StringArray ch, int d) {
        p.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1}, n, ch, d)); };
    auto B = [&](const char* id, const char* n, bool d) {
        p.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1}, n, d)); };

    F("osc1_level","OSC1 Level",0,1,0.7f); F("osc2_level","OSC2 Level",0,1,0.5f); F("osc3_level","OSC3 Level",0,1,0.4f);
    F("osc1_table","OSC1 Table",0,1,0); F("osc2_table","OSC2 Table",0,1,0.33f); F("osc3_table","OSC3 Table",0,1,0.66f);
    F("osc1_warp","OSC1 Warp",0,1,0); F("osc2_warp","OSC2 Warp",0,1,0); F("osc3_warp","OSC3 Warp",0,1,0);
    F("osc1_fold","OSC1 Fold",0,1,0); F("osc2_fold","OSC2 Fold",0,1,0); F("osc3_fold","OSC3 Fold",0,1,0);
    F("osc1_drive","OSC1 Drive",0,1,0); F("osc2_drive","OSC2 Drive",0,1,0); F("osc3_drive","OSC3 Drive",0,1,0);
    I("osc1_octave","OSC1 Oct",-3,3,0); I("osc2_octave","OSC2 Oct",-3,3,0); I("osc3_octave","OSC3 Oct",-3,3,-1);
    I("osc1_semi","OSC1 Semi",-12,12,0); I("osc2_semi","OSC2 Semi",-12,12,0); I("osc3_semi","OSC3 Semi",-12,12,0);
    F("osc1_fine","OSC1 Fine",-100,100,0); F("osc2_fine","OSC2 Fine",-100,100,0); F("osc3_fine","OSC3 Fine",-100,100,0);
    F("osc1_detune","OSC1 Detune",-50,50,0); F("osc2_detune","OSC2 Detune",-50,50,0); F("osc3_detune","OSC3 Detune",-50,50,0);
    I("unison_voices","Unison",1,7,1); F("unison_detune","Uni Detune",0,50,12); F("unison_spread","Uni Spread",0,1,0.7f);
    F("fm_2to1","FM 2to1",0,1,0); F("fm_3to1","FM 3to1",0,1,0); F("fm_3to2","FM 3to2",0,1,0);
    F("pm_2to1","PM 2to1",0,1,0); F("rm_2to1","RM 2to1",0,1,0); F("am_2to1","AM 2to1",0,1,0);
    F("filter_cutoff","Cutoff",20,20000,8000); F("filter_reso","Reso",0,1,0.25f);
    F("filter_drive","F Drive",0,1,0); F("filter_env","F Env",0,1,0.4f);
    C("filter_mode","F Mode",{"LowPass","HighPass","BandPass","Notch"},0);
    F("amp_attack","Attack",0.001f,5,0.01f); F("amp_decay","Decay",0.001f,5,0.15f);
    F("amp_sustain","Sustain",0,1,0.75f); F("amp_release","Release",0.001f,8,0.25f);
    F("lfo_rate","LFO Rate",0.01f,40,2); F("lfo_amount","LFO Amt",0,1,0);
    C("lfo_wave","LFO Wave",{"Sine","Triangle","Saw","Square","S&H"},0);
    F("macro1","Macro1",0,1,0); F("macro2","Macro2",0,1,0); F("macro3","Macro3",0,1,0); F("macro4","Macro4",0,1,0);
    F("delay_mix","Delay Mix",0,1,0); F("delay_time","Delay Time",50,800,280); F("delay_fb","Delay FB",0,0.95f,0.35f);
    F("master_drive","Master Drive",0,1,0); F("master_gain","Master Gain",0,1,0.8f);
    B("arp_on","Arp On",false); I("arp_rate","Arp Rate",1,8,4); I("arp_octaves","Arp Oct",1,4,1);
    B("seq_on","Seq On",false); I("seq_rate","Seq Rate",1,8,4);
    return { p.begin(), p.end() };
}

void SalekHightechAudioProcessor::initFactoryPresets()
{
    auto add = [&](const juce::String& n, std::map<juce::String,float> v){ factoryPresets.push_back({n, std::move(v)}); };
    add("Init", {});
    add("FM Bass", {{"osc1_level",0.9f},{"osc2_level",0.7f},{"osc2_octave",1.f},{"fm_2to1",0.65f},{"filter_cutoff",1200.f},{"filter_reso",0.55f},{"filter_env",0.7f},{"amp_decay",0.35f},{"amp_sustain",0.4f},{"master_drive",0.25f}});
    add("Acid Screech", {{"osc1_fold",0.45f},{"osc1_drive",0.5f},{"filter_cutoff",900.f},{"filter_reso",0.85f},{"filter_env",0.9f},{"filter_drive",0.4f},{"lfo_rate",6.f},{"lfo_amount",0.35f}});
    add("Alien Pad", {{"osc1_table",0.7f},{"osc2_table",0.9f},{"osc1_warp",0.3f},{"filter_cutoff",3500.f},{"amp_attack",0.8f},{"amp_release",2.5f},{"delay_mix",0.35f},{"lfo_amount",0.2f},{"lfo_rate",0.3f}});
    add("Hi-Tech Lead", {{"osc1_level",0.85f},{"osc1_fold",0.25f},{"fm_2to1",0.4f},{"filter_cutoff",6000.f},{"filter_reso",0.4f},{"amp_sustain",0.7f},{"master_drive",0.15f},{"delay_mix",0.2f}});
    add("Dark Psy", {{"osc1_table",0.15f},{"osc2_table",0.55f},{"osc3_octave",-1.f},{"fm_3to1",0.5f},{"rm_2to1",0.2f},{"filter_cutoff",800.f},{"filter_reso",0.7f},{"filter_env",0.8f},{"master_drive",0.3f}});
    add("Metallic Hit", {{"osc1_fold",0.7f},{"osc1_drive",0.6f},{"fm_2to1",0.8f},{"filter_cutoff",4000.f},{"filter_mode",2.f},{"amp_attack",0.001f},{"amp_decay",0.15f},{"amp_sustain",0.1f}});
    add("Massive Super", {{"osc1_level",0.95f},{"osc2_level",0.4f},{"unison_voices",5.f},{"unison_detune",18.f},{"unison_spread",0.85f},{"filter_cutoff",4500.f},{"filter_reso",0.35f},{"master_drive",0.2f},{"delay_mix",0.15f}});
    add("WT Morph Lead", {{"osc1_table",0.55f},{"osc1_warp",0.35f},{"osc1_fold",0.2f},{"unison_voices",3.f},{"unison_detune",10.f},{"fm_2to1",0.25f},{"filter_cutoff",7000.f},{"amp_sustain",0.8f}});
    add("Sub Growl", {{"osc1_table",0.1f},{"osc3_octave",-2.f},{"osc3_level",0.8f},{"fm_3to1",0.7f},{"filter_cutoff",600.f},{"filter_reso",0.65f},{"filter_drive",0.45f},{"master_drive",0.4f}});
    add("Glass Arp", {{"osc1_table",0.85f},{"osc1_warp",0.5f},{"unison_voices",3.f},{"filter_cutoff",9000.f},{"amp_attack",0.001f},{"amp_decay",0.2f},{"amp_sustain",0.f},{"delay_mix",0.4f},{"arp_on",1.f},{"arp_rate",4.f}});
    add("Wide Pad+", {{"osc1_table",0.6f},{"osc2_table",0.9f},{"unison_voices",7.f},{"unison_detune",25.f},{"unison_spread",1.f},{"amp_attack",1.2f},{"amp_release",3.f},{"delay_mix",0.45f},{"lfo_rate",0.2f},{"lfo_amount",0.25f}});
    add("Cyber Pluck", {{"osc1_warp",0.4f},{"filter_cutoff",5000.f},{"filter_env",0.6f},{"amp_attack",0.002f},{"amp_decay",0.25f},{"amp_sustain",0.f},{"delay_mix",0.25f}});
}

void SalekHightechAudioProcessor::loadFactoryPreset(int index)
{
    if (index < 0 || index >= (int)factoryPresets.size()) return;
    currentProgram = index;
    for (auto* param : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(param))
            rp->setValueNotifyingHost(rp->getDefaultValue());
    for (auto& kv : factoryPresets[(size_t)index].values)
        if (auto* p = apvts.getParameter(kv.first))
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p))
                rp->setValueNotifyingHost(rp->convertTo0to1(kv.second));
}

juce::StringArray SalekHightechAudioProcessor::getPresetNames() const
{
    juce::StringArray n; for (auto& pr : factoryPresets) n.add(pr.name); return n;
}
void SalekHightechAudioProcessor::setCurrentProgram(int index) { loadFactoryPreset(index); }
const juce::String SalekHightechAudioProcessor::getProgramName(int index)
{
    return (index >= 0 && index < (int)factoryPresets.size()) ? factoryPresets[(size_t)index].name : juce::String();
}

void SalekHightechAudioProcessor::prepareToPlay(double sr, int spb)
{
    synthEngine.prepareToPlay(sr, spb); delay.prepare(sr, spb); arpeggiator.prepare(sr);
    stepSequencer.prepare(sr); stepSequencer.initDefaultPattern();
}
bool SalekHightechAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto s = layouts.getMainOutputChannelSet();
    return s == juce::AudioChannelSet::mono() || s == juce::AudioChannelSet::stereo();
}

void SalekHightechAudioProcessor::applyParamsToEngine()
{
    auto g = [&](const char* id) -> float { if (auto* p = apvts.getRawParameterValue(id)) return p->load(); return 0.f; };
    synthEngine.setOsc1Level(g("osc1_level")); synthEngine.setOsc2Level(g("osc2_level")); synthEngine.setOsc3Level(g("osc3_level"));
    synthEngine.setOsc1TablePos(g("osc1_table")); synthEngine.setOsc2TablePos(g("osc2_table")); synthEngine.setOsc3TablePos(g("osc3_table"));
    synthEngine.setOsc1Warp(g("osc1_warp")); synthEngine.setOsc2Warp(g("osc2_warp")); synthEngine.setOsc3Warp(g("osc3_warp"));
    synthEngine.setOsc1Fold(g("osc1_fold")); synthEngine.setOsc2Fold(g("osc2_fold")); synthEngine.setOsc3Fold(g("osc3_fold"));
    synthEngine.setOsc1Drive(g("osc1_drive")); synthEngine.setOsc2Drive(g("osc2_drive")); synthEngine.setOsc3Drive(g("osc3_drive"));
    synthEngine.setOsc1Octave((int)g("osc1_octave")); synthEngine.setOsc2Octave((int)g("osc2_octave")); synthEngine.setOsc3Octave((int)g("osc3_octave"));
    synthEngine.setOsc1Semi((int)g("osc1_semi")); synthEngine.setOsc2Semi((int)g("osc2_semi")); synthEngine.setOsc3Semi((int)g("osc3_semi"));
    synthEngine.setOsc1Fine(g("osc1_fine")); synthEngine.setOsc2Fine(g("osc2_fine")); synthEngine.setOsc3Fine(g("osc3_fine"));
    synthEngine.setOsc1Detune(g("osc1_detune")); synthEngine.setOsc2Detune(g("osc2_detune")); synthEngine.setOsc3Detune(g("osc3_detune"));
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    float cut = g("filter_cutoff") * (0.35f + 0.65f*(1.f-g("macro1")) + g("macro1")*2.2f);
    synthEngine.setFilterCutoff(cut); synthEngine.setFilterResonance(g("filter_reso"));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setFilterEnvAmt(g("filter_env"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    delay.setTimeMs(g("delay_time")); delay.setFeedback(g("delay_fb")); delay.setMix(g("delay_mix"));
    synthEngine.setUnison((int)g("unison_voices"));
    synthEngine.setUnisonDetune(g("unison_detune"));
    synthEngine.setUnisonSpread(g("unison_spread"));
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd; buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);
    applyParamsToEngine();

    auto g = [&](const char* id) -> float { if (auto* p = apvts.getRawParameterValue(id)) return p->load(); return 0.f; };
    arpeggiator.setEnabled(g("arp_on") > 0.5f);
    arpeggiator.setRateDivisor((int) g("arp_rate"));
    arpeggiator.setOctaves((int) g("arp_octaves"));

    juce::MidiBuffer routed;
    const bool seqOn = g("seq_on") > 0.5f;
    const bool arpOn = g("arp_on") > 0.5f;

    stepSequencer.setEnabled(seqOn);
    stepSequencer.setRateDivisor((int) g("seq_rate"));

    if (seqOn)
    {
        for (const auto meta : midi)
        {
            auto m = meta.getMessage();
            if (m.isNoteOn()) stepSequencer.setRootNote(m.getNoteNumber());
            else if (! m.isNoteOff()) routed.addEvent(m, meta.samplePosition);
        }
        stepSequencer.process(buffer.getNumSamples(), routed);
        midi.swapWith(routed);
    }
    else if (arpOn)
    {
        for (const auto meta : midi)
        {
            auto m = meta.getMessage();
            if (m.isNoteOn()) arpeggiator.noteOn(m.getNoteNumber(), m.getFloatVelocity());
            else if (m.isNoteOff()) arpeggiator.noteOff(m.getNoteNumber());
            else routed.addEvent(m, meta.samplePosition);
        }
        arpeggiator.process(buffer.getNumSamples(), routed);
        midi.swapWith(routed);
    }

    synthEngine.processBlock(buffer, midi);
    float md = apvts.getRawParameterValue("master_drive")->load();
    float mg = apvts.getRawParameterValue("master_gain")->load();
    if (md > 1e-4f) {
        float gain = 1.f + md * 4.f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i) d[i] = std::tanh(d[i]*gain)*0.9f;
        }
    }
    buffer.applyGain(mg); delay.process(buffer);
}

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor() { return new SalekHightechAudioProcessorEditor(*this); }

void SalekHightechAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState(); state.setProperty("program", currentProgram, nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, destData);
}
void SalekHightechAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType())) {
        auto tree = juce::ValueTree::fromXml(*xml);
        currentProgram = (int)tree.getProperty("program", 0);
        apvts.replaceState(tree);
    }
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SalekHightechAudioProcessor(); }

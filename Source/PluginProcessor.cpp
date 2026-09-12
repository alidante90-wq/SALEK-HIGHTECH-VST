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
    F("chorus_mix","Chorus Mix",0,1,0); F("chorus_rate","Chorus Rate",0.05f,5,0.35f); F("chorus_depth","Chorus Depth",0,1,0.5f);
    F("reverb_mix","Reverb Mix",0,1,0); F("reverb_size","Reverb Size",0,1,0.5f); F("reverb_decay","Reverb Decay",0.1f,0.95f,0.55f);
    F("master_drive","Master Drive",0,1,0); F("master_gain","Master Gain",0,1,0.8f);
    B("arp_on","Arp On",false); I("arp_rate","Arp Rate",1,8,4); I("arp_octaves","Arp Oct",1,4,1);
    B("seq_on","Seq On",false); I("seq_rate","Seq Rate",1,8,4);
    F("comp_threshold","Comp Thresh",-40,0,-12); F("comp_ratio","Comp Ratio",1,20,4); F("comp_mix","Comp Mix",0,1,0);
    F("eq_low","EQ Low",-12,12,0); F("eq_mid","EQ Mid",-12,12,0); F("eq_high","EQ High",-12,12,0);
    F("spatial_azim","Spatial Azim",-180,180,0); F("spatial_dist","Spatial Dist",0,1,0);
    F("spatial_size","Spatial Size",0,1,0.5f); F("spatial_elev","Spatial Elev",-90,90,0);
    F("input_mix","Input Mix",0,1,0);
    return { p.begin(), p.end() };
}

#include "PluginProcessorPresets.inl"

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
    keyboardState.allNotesOff (0);
    synthEngine.allNotesOff();
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
    synthEngine.prepareToPlay(sr, spb);
    delay.prepare(sr, spb);
    chorus.prepare(sr, spb);
    reverb.prepare(sr, spb);
    compressor.prepare(sr, spb);
    eq.prepare(sr, spb);
    spatial.prepare(sr, spb);
    arpeggiator.prepare(sr);
    stepSequencer.prepare(sr);
    stepSequencer.initDefaultPattern();
    keyboardState.allNotesOff (0);
    synthEngine.allNotesOff();
    arpeggiator.setEnabled (false);
    stepSequencer.setEnabled (false);
    visualFifo.prepare ((int) sr * 2);
}

bool SalekHightechAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    auto in  = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    if (in != juce::AudioChannelSet::disabled()
        && in != juce::AudioChannelSet::mono()
        && in != juce::AudioChannelSet::stereo())
        return false;
    return true;
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
    synthEngine.setUnison((int)g("unison_voices"));
    synthEngine.setUnisonDetune(g("unison_detune"));
    synthEngine.setUnisonSpread(g("unison_spread"));
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    float cut = g("filter_cutoff") * (0.35f + 0.65f*(1.f-g("macro1")) + g("macro1")*2.2f);
    cut *= std::pow(2.0f, modMatrix.getModulation(salek::ModMatrix::Dest::FilterCutoff) * 2.0f);
    synthEngine.setFilterCutoff(juce::jlimit(20.f, 20000.f, cut));
    synthEngine.setFilterResonance(g("filter_reso"));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterEnvAmt(g("filter_env"));
    synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    modMatrix.setSourceValue(salek::ModMatrix::Source::LFO1, std::sin((float)juce::Time::getMillisecondCounter() * 0.001f * g("lfo_rate") * juce::MathConstants<float>::twoPi));
    modMatrix.setSourceValue(salek::ModMatrix::Source::Macro1, g("macro1") * 2.f - 1.f);
    modMatrix.setSourceValue(salek::ModMatrix::Source::Macro2, g("macro2") * 2.f - 1.f);
    modMatrix.setSourceValue(salek::ModMatrix::Source::Macro3, g("macro3") * 2.f - 1.f);
    modMatrix.setSourceValue(salek::ModMatrix::Source::Macro4, g("macro4") * 2.f - 1.f);
    delay.setMix(g("delay_mix")); delay.setTimeMs(g("delay_time")); delay.setFeedback(g("delay_fb"));
    chorus.setMix(g("chorus_mix")); chorus.setRate(g("chorus_rate")); chorus.setDepth(g("chorus_depth"));
    reverb.setMix(g("reverb_mix")); reverb.setSize(g("reverb_size")); reverb.setDecay(g("reverb_decay"));
    compressor.setThresholdDb(g("comp_threshold")); compressor.setRatio(g("comp_ratio")); compressor.setMix(g("comp_mix"));
    eq.setLowGainDb(g("eq_low")); eq.setMidGainDb(g("eq_mid")); eq.setHighGainDb(g("eq_high"));
    spatial.setAzimuth(g("spatial_azim")); spatial.setDistance(g("spatial_dist"));
    spatial.setSize(g("spatial_size")); spatial.setElevation(g("spatial_elev"));
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

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
            if (m.isNoteOn())
                stepSequencer.setRootNote(m.getNoteNumber());
            else if (! m.isNoteOff())
                routed.addEvent(m, meta.samplePosition);
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

    buffer.clear();
    synthEngine.processBlock(buffer, midi);

    if (auto* inBus = getBus (true, 0))
    {
        if (inBus->isEnabled())
        {
            auto inBuf = getBusBuffer (buffer, true, 0);
            float im = apvts.getRawParameterValue("input_mix")->load();
            if (im > 1e-4f && inBuf.getNumSamples() > 0)
            {
                for (int ch = 0; ch < juce::jmin (buffer.getNumChannels(), inBuf.getNumChannels()); ++ch)
                    buffer.addFrom (ch, 0, inBuf, ch, 0, buffer.getNumSamples(), im);
            }
        }
    }

    float drive = apvts.getRawParameterValue("master_drive")->load();
    if (drive > 1e-4f)
    {
        float dg = 1.f + drive * 4.f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                d[i] = std::tanh(d[i] * dg);
        }
    }

    chorus.process(buffer);
    delay.process(buffer);
    reverb.process(buffer);
    compressor.process(buffer);
    eq.process(buffer);
    spatial.process(buffer);

    float gain = apvts.getRawParameterValue("master_gain")->load();
    buffer.applyGain(gain);

    float peak = 0.f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    outputPeak.store (peak);
    visualFifo.pushStereo (buffer);
}

void SalekHightechAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}
void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor()
{
    return new SalekHightechAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SalekHightechAudioProcessor();
}

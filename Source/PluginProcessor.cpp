#include <cmath>
#include "PluginProcessor.h"
#include "PluginEditor.h"

SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

SalekHightechAudioProcessor::~SalekHightechAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout SalekHightechAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto F = [&](const char* id, const char* name, float minV, float maxV, float defV)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float> (minV, maxV), defV));
    };
    auto I = [&](const char* id, const char* name, int minV, int maxV, int defV)
    {
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id, 1 }, name, minV, maxV, defV));
    };
    auto B = [&](const char* id, const char* name, bool defV)
    {
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id, 1 }, name, defV));
    };

    F("osc1_level","OSC1 Level",0,1,0.7f); F("osc1_table","OSC1 Table",0,1,0);
    F("osc1_warp","OSC1 Warp",0,1,0); F("osc1_fold","OSC1 Fold",0,1,0);
    F("osc1_drive","OSC1 Drive",0,1,0); I("osc1_octave","OSC1 Oct",-2,2,0);
    F("osc2_level","OSC2 Level",0,1,0.5f); F("osc2_table","OSC2 Table",0,1,0.33f);
    F("osc2_warp","OSC2 Warp",0,1,0); F("osc2_fold","OSC2 Fold",0,1,0);
    F("osc2_drive","OSC2 Drive",0,1,0); I("osc2_octave","OSC2 Oct",-2,2,0);
    F("osc3_level","OSC3 Level",0,1,0.4f); F("osc3_table","OSC3 Table",0,1,0.66f);
    F("osc3_warp","OSC3 Warp",0,1,0); F("osc3_fold","OSC3 Fold",0,1,0);
    I("unison_voices","Unison",1,8,1); F("unison_detune","U Detune",0,50,12); F("unison_spread","Spread",0,1,0.7f);

    F("filter_cutoff","Cutoff",20,18000,8000); F("filter_reso","Reso",0,1,0.25f);
    F("filter_drive","F Drive",0,1,0); F("filter_env","F Env",0,1,0.35f);
    I("filter_mode","F Mode",0,3,0);

    F("amp_attack","Attack",0.001f,3,0.01f); F("amp_decay","Decay",0.001f,3,0.15f);
    F("amp_sustain","Sustain",0,1,0.75f); F("amp_release","Release",0.001f,5,0.25f);

    F("fm_2to1","FM 2>1",0,1,0); F("fm_3to1","FM 3>1",0,1,0); F("fm_3to2","FM 3>2",0,1,0);
    F("pm_2to1","PM 2>1",0,1,0); F("rm_2to1","RM 2>1",0,1,0); F("am_2to1","AM 2>1",0,1,0);
    F("lfo_rate","LFO Rate",0.05f,20,2); F("lfo_amount","LFO Amt",0,1,0); I("lfo_wave","LFO Wave",0,4,0);
    F("macro1","Macro 1",0,1,0); F("macro2","Macro 2",0,1,0); F("macro3","Macro 3",0,1,0); F("macro4","Macro 4",0,1,0);

    F("chorus_mix","Chorus Mix",0,1,0); F("chorus_rate","Chorus Rate",0.05f,5,0.35f); F("chorus_depth","Chorus Depth",0,1,0.5f);
    F("delay_mix","Delay Mix",0,1,0); F("delay_time","Delay Time",50,800,280); F("delay_fb","Delay FB",0,0.95f,0.35f);
    F("reverb_mix","Reverb Mix",0,1,0); F("reverb_size","Reverb Size",0,1,0.5f); F("reverb_decay","Reverb Decay",0.1f,0.95f,0.55f);
    F("master_drive","Master Drive",0,1,0); F("master_gain","Master Gain",0,1,0.8f);
    F("comp_threshold","C Thr",-40,0,-12); F("comp_ratio","C Ratio",1,20,4); F("comp_mix","C Mix",0,1,0);
    F("eq_low","EQ Low",-12,12,0); F("eq_mid","EQ Mid",-12,12,0); F("eq_high","EQ Hi",-12,12,0);
    F("spatial_azim","Azim",-1,1,0); F("spatial_dist","Dist",0,1,0); F("spatial_size","Size",0,1,0.5f); F("spatial_elev","Elev",-1,1,0);
    F("input_mix","In Mix",0,1,0);

    B("arp_on","Arp On",false); B("seq_on","Seq On",false);
    F("arp_rate","Arp Rate",1,32,8); I("arp_octaves","Arp Oct",1,4,1); F("seq_rate","Seq Rate",1,32,8);

    return layout;
}

void SalekHightechAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synthEngine.prepareToPlay (sampleRate, samplesPerBlock);
    delay.prepare (sampleRate, samplesPerBlock);
    chorus.prepare (sampleRate, samplesPerBlock);
    reverb.prepare (sampleRate, samplesPerBlock);
    compressor.prepare (sampleRate, samplesPerBlock);
    eq.prepare (sampleRate, samplesPerBlock);
    spatial.prepare (sampleRate, samplesPerBlock);
    arpeggiator.prepare (sampleRate);
    stepSequencer.prepare (sampleRate);
    visualFifo.prepare (sampleRate, samplesPerBlock);
    keyboardState.reset();
}

void SalekHightechAudioProcessor::releaseResources()
{
    synthEngine.releaseResources();
}

bool SalekHightechAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void SalekHightechAudioProcessor::updateParameters()
{
    auto g = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    synthEngine.setOsc1Level(g("osc1_level")); synthEngine.setOsc1TablePos(g("osc1_table"));
    synthEngine.setOsc1Warp(g("osc1_warp")); synthEngine.setOsc1Fold(g("osc1_fold")); synthEngine.setOsc1Drive(g("osc1_drive"));
    synthEngine.setOsc1Octave((int)g("osc1_octave"));
    synthEngine.setOsc2Level(g("osc2_level")); synthEngine.setOsc2TablePos(g("osc2_table"));
    synthEngine.setOsc2Warp(g("osc2_warp")); synthEngine.setOsc2Fold(g("osc2_fold")); synthEngine.setOsc2Drive(g("osc2_drive"));
    synthEngine.setOsc2Octave((int)g("osc2_octave"));
    synthEngine.setOsc3Level(g("osc3_level")); synthEngine.setOsc3TablePos(g("osc3_table"));
    synthEngine.setOsc3Warp(g("osc3_warp")); synthEngine.setOsc3Fold(g("osc3_fold"));
    synthEngine.setUnison((int)g("unison_voices")); synthEngine.setUnisonDetune(g("unison_detune")); synthEngine.setUnisonSpread(g("unison_spread"));
    synthEngine.setFilterCutoff(g("filter_cutoff")); synthEngine.setFilterResonance(g("filter_reso"));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterEnvAmt(g("filter_env"));
    synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    delay.setMix(g("delay_mix")); delay.setTimeMs(g("delay_time")); delay.setFeedback(g("delay_fb"));
    chorus.setMix(g("chorus_mix")); chorus.setRate(g("chorus_rate")); chorus.setDepth(g("chorus_depth"));
    reverb.setMix(g("reverb_mix")); reverb.setSize(g("reverb_size")); reverb.setDecay(g("reverb_decay"));
    compressor.setThreshold(g("comp_threshold")); compressor.setRatio(g("comp_ratio")); compressor.setMix(g("comp_mix"));
    eq.setLow(g("eq_low")); eq.setMid(g("eq_mid")); eq.setHigh(g("eq_high"));
    spatial.setAzimuth(g("spatial_azim")); spatial.setDistance(g("spatial_dist")); spatial.setSize(g("spatial_size")); spatial.setElevation(g("spatial_elev"));
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    updateParameters();

    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    if (apvts.getRawParameterValue("arp_on")->load() > 0.5f)
    {
        juce::MidiBuffer routed;
        arpeggiator.setRate (apvts.getRawParameterValue("arp_rate")->load());
        arpeggiator.setOctaves ((int) apvts.getRawParameterValue("arp_octaves")->load());
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
        float dg = 1.f + drive * 3.5f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = d[i] * dg;
                x = std::tanh (x);
                x = x - 0.15f * x * x * x;
                d[i] = x;
            }
        }
    }

    chorus.process(buffer);
    delay.process(buffer);
    reverb.process(buffer);
    compressor.process(buffer);
    eq.process(buffer);
    spatial.process(buffer);

    float gain = apvts.getRawParameterValue("master_gain")->load();
    buffer.applyGain(gain * 0.95f);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer (ch);
        const int n = buffer.getNumSamples();
        for (int i = 0; i < n; ++i)
        {
            float x = d[i];
            const float ax = std::abs (x);
            if (ax > 0.9f)
            {
                const float s = (x >= 0.0f) ? 1.0f : -1.0f;
                d[i] = s * (0.9f + 0.1f * std::tanh ((ax - 0.9f) * 8.0f));
            }
        }
    }

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


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
    C("scale_mode","Scale",{"Equal 12-TET","Shur","Segah","Homayun","Mahur","Free Koron"},0);
    F("koron_cents","Koron Cents",-50,50,0);
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

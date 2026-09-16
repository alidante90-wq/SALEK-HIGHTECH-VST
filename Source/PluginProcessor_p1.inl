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
    auto B = [&](const char* id, const char* n, bool d) {
        p.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1}, n, d)); };

    F("osc1_level","OSC1 Level",0,1,0.7f); F("osc1_table","OSC1 Table",0,1,0);
    F("osc1_warp","OSC1 Warp",0,1,0); F("osc1_fold","OSC1 Fold",0,1,0); F("osc1_drive","OSC1 Drive",0,1,0);
    I("osc1_octave","OSC1 Oct",-2,2,0); I("osc1_semi","OSC1 Semi",-12,12,0); F("osc1_fine","OSC1 Fine",-100,100,0);
    F("osc2_level","OSC2 Level",0,1,0.5f); F("osc2_table","OSC2 Table",0,1,0.33f);
    F("osc2_warp","OSC2 Warp",0,1,0); F("osc2_fold","OSC2 Fold",0,1,0); F("osc2_drive","OSC2 Drive",0,1,0);
    I("osc2_octave","OSC2 Oct",-2,2,0); I("osc2_semi","OSC2 Semi",-12,12,0); F("osc2_fine","OSC2 Fine",-100,100,0);
    F("osc3_level","OSC3 Level",0,1,0.4f); F("osc3_table","OSC3 Table",0,1,0.66f);
    F("osc3_warp","OSC3 Warp",0,1,0); F("osc3_fold","OSC3 Fold",0,1,0); F("osc3_drive","OSC3 Drive",0,1,0);
    I("osc3_octave","OSC3 Oct",-2,2,-1); I("osc3_semi","OSC3 Semi",-12,12,0); F("osc3_fine","OSC3 Fine",-100,100,0);
    I("unison_voices","Unison",1,8,1); F("unison_detune","U Detune",0,50,12); F("unison_spread","Spread",0,1,0.7f);
    F("filter_cutoff","Cutoff",20,18000,8000); F("filter_reso","Reso",0,1,0.25f);
    F("filter_drive","F Drive",0,1,0); F("filter_env","F Env",0,1,0.35f); I("filter_mode","F Mode",0,3,0);
    F("amp_attack","Attack",0.001f,3,0.01f); F("amp_decay","Decay",0.001f,3,0.15f);
    F("amp_sustain","Sustain",0,1,0.75f); F("amp_release","Release",0.001f,5,0.25f);
    F("fm_2to1","FM 2>1",0,1,0); F("fm_3to1","FM 3>1",0,1,0); F("fm_3to2","FM 3>2",0,1,0);
    F("pm_2to1","PM 2>1",0,1,0); F("pm_3to1","PM 3>1",0,1,0);
    F("rm_2to1","RM 2>1",0,1,0); F("am_2to1","AM 2>1",0,1,0);
    F("lfo_rate","LFO Rate",0.05f,20,2); F("lfo_amount","LFO Amt",0,1,0); I("lfo_wave","LFO Wave",0,4,0);
    F("macro1","Macro 1",0,1,0); F("macro2","Macro 2",0,1,0); F("macro3","Macro 3",0,1,0); F("macro4","Macro 4",0,1,0);
    F("chorus_mix","Chorus Mix",0,1,0); F("chorus_rate","Chorus Rate",0.05f,5,0.35f); F("chorus_depth","Chorus Depth",0,1,0.5f);
    F("delay_mix","Delay Mix",0,1,0); F("delay_time","Delay Time",50,800,280); F("delay_fb","Delay FB",0,0.95f,0.35f);
    F("reverb_mix","Reverb Mix",0,1,0); F("reverb_size","Reverb Size",0,1,0.5f); F("reverb_decay","Reverb Decay",0.1f,0.95f,0.55f);
    F("master_drive","Master Drive",0,1,0); F("master_gain","Master Gain",0,1,0.8f);
    B("arp_on","Arp On",false); I("arp_rate","Arp Rate",1,8,4); I("arp_octaves","Arp Oct",1,4,1);
    B("seq_on","Seq On",false); I("seq_rate","Seq Rate",1,8,4);
    F("comp_threshold","Comp Thresh",-40,0,-12); F("comp_ratio","Comp Ratio",1,20,4); F("comp_mix","Comp Mix",0,1,0);
    F("eq_low","EQ Low",-12,12,0); F("eq_mid","EQ Mid",-12,12,0); F("eq_high","EQ High",-12,12,0);
    F("spatial_azim","Azimuth",-1,1,0); F("spatial_dist","Distance",0,1,0);
    F("spatial_size","Size",0,1,0.5f); F("spatial_elev","Elevation",-1,1,0);
    F("input_mix","Input Mix",0,1,0);
    I("scale_mode","Scale",0,4,0); F("koron_cents","Koron",-50,50,-50);
    return { p.begin(), p.end() };
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
    keyboardState.reset();
}

bool SalekHightechAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void SalekHightechAudioProcessor::applyParamsToEngine()
{
    auto g = [this](const char* id) -> float {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return 0.f;
    };
    synthEngine.setOsc1Level(g("osc1_level")); synthEngine.setOsc1TablePos(g("osc1_table"));
    synthEngine.setOsc1Warp(g("osc1_warp")); synthEngine.setOsc1Fold(g("osc1_fold")); synthEngine.setOsc1Drive(g("osc1_drive"));
    synthEngine.setOsc1Octave((int)g("osc1_octave")); synthEngine.setOsc1Semi((int)g("osc1_semi")); synthEngine.setOsc1Fine(g("osc1_fine"));
    synthEngine.setOsc2Level(g("osc2_level")); synthEngine.setOsc2TablePos(g("osc2_table"));
    synthEngine.setOsc2Warp(g("osc2_warp")); synthEngine.setOsc2Fold(g("osc2_fold")); synthEngine.setOsc2Drive(g("osc2_drive"));
    synthEngine.setOsc2Octave((int)g("osc2_octave")); synthEngine.setOsc2Semi((int)g("osc2_semi")); synthEngine.setOsc2Fine(g("osc2_fine"));
    synthEngine.setOsc3Level(g("osc3_level")); synthEngine.setOsc3TablePos(g("osc3_table"));
    synthEngine.setOsc3Warp(g("osc3_warp")); synthEngine.setOsc3Fold(g("osc3_fold")); synthEngine.setOsc3Drive(g("osc3_drive"));
    synthEngine.setOsc3Octave((int)g("osc3_octave")); synthEngine.setOsc3Semi((int)g("osc3_semi")); synthEngine.setOsc3Fine(g("osc3_fine"));
    synthEngine.setFilterCutoff(g("filter_cutoff")); synthEngine.setFilterResonance(g("filter_reso"));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterMode((int)g("filter_mode")); synthEngine.setFilterEnvAmt(g("filter_env"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setPm3to1(g("pm_3to1"));
    synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    synthEngine.setUnison((int)g("unison_voices")); synthEngine.setUnisonDetune(g("unison_detune")); synthEngine.setUnisonSpread(g("unison_spread"));
    synthEngine.setScaleMode((int)g("scale_mode")); synthEngine.setKoronCents(g("koron_cents"));
    delay.setMix(g("delay_mix")); delay.setTimeMs(g("delay_time")); delay.setFeedback(g("delay_fb"));
    chorus.setMix(g("chorus_mix")); chorus.setRate(g("chorus_rate")); chorus.setDepth(g("chorus_depth"));
    reverb.setMix(g("reverb_mix")); reverb.setSize(g("reverb_size")); reverb.setDecay(g("reverb_decay"));
}

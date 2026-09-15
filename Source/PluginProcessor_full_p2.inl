
void SalekHightechAudioProcessor::applyParamsToEngine()
{
    auto g = [&](const char* id) -> float { if (auto* p = apvts.getRawParameterValue(id)) return p->load(); return 0.f; };
    float o1l = juce::jlimit(0.f,1.f, g("osc1_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Level)*0.5f);
    float o2l = juce::jlimit(0.f,1.f, g("osc2_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Level)*0.5f);
    float o3l = juce::jlimit(0.f,1.f, g("osc3_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Level)*0.5f);
    synthEngine.setOsc1Level(o1l); synthEngine.setOsc2Level(o2l); synthEngine.setOsc3Level(o3l);
    synthEngine.setOsc1TablePos(juce::jlimit(0.f,1.f, g("osc1_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Table)*0.5f));
    synthEngine.setOsc2TablePos(juce::jlimit(0.f,1.f, g("osc2_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Table)*0.5f));
    synthEngine.setOsc3TablePos(juce::jlimit(0.f,1.f, g("osc3_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Table)*0.5f));
    synthEngine.setOsc1Warp(juce::jlimit(0.f,1.f, g("osc1_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Warp)*0.5f));
    synthEngine.setOsc2Warp(juce::jlimit(0.f,1.f, g("osc2_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Warp)*0.5f));
    synthEngine.setOsc3Warp(juce::jlimit(0.f,1.f, g("osc3_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Warp)*0.5f));
    synthEngine.setOsc1Fold(g("osc1_fold")); synthEngine.setOsc2Fold(g("osc2_fold")); synthEngine.setOsc3Fold(g("osc3_fold"));
    synthEngine.setOsc1Drive(g("osc1_drive")); synthEngine.setOsc2Drive(g("osc2_drive")); synthEngine.setOsc3Drive(g("osc3_drive"));
    synthEngine.setOsc1Octave((int)g("osc1_octave")); synthEngine.setOsc2Octave((int)g("osc2_octave")); synthEngine.setOsc3Octave((int)g("osc3_octave"));
    synthEngine.setOsc1Semi((int)g("osc1_semi")); synthEngine.setOsc2Semi((int)g("osc2_semi")); synthEngine.setOsc3Semi((int)g("osc3_semi"));
    synthEngine.setUnison((int)g("unison_voices"));
    synthEngine.setUnisonDetune(g("unison_detune"));
    synthEngine.setUnisonSpread(g("unison_spread"));
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    float cut = g("filter_cutoff");
    const float macroAmt = g("macro1");
    cut *= (0.85f + 0.15f * (1.0f - macroAmt) + macroAmt * 1.35f);
    cut *= std::pow(2.0f, modMatrix.getModulation(salek::ModMatrix::Dest::FilterCutoff) * 1.0f);
    synthEngine.setFilterCutoff(juce::jlimit(20.f, 20000.f, cut));
    synthEngine.setFilterResonance(juce::jlimit(0.f,1.f, g("filter_reso")+modMatrix.getModulation(salek::ModMatrix::Dest::FilterReso)*0.5f));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterEnvAmt(g("filter_env"));
    synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    synthEngine.setScaleMode((int)g("scale_mode"));
    synthEngine.setKoronCents(g("koron_cents"));

    auto setLfo = [] (salek::LFO& lfo, float rate, float amt, int wave)
    {
        lfo.setRate (rate);
        lfo.setAmount (1.0f);
        static const salek::LFO::Wave waves[] = {
            salek::LFO::Wave::Sine, salek::LFO::Wave::Triangle, salek::LFO::Wave::Saw,
            salek::LFO::Wave::Square, salek::LFO::Wave::SAndH, salek::LFO::Wave::Custom
        };
        lfo.setWave (waves[juce::jlimit (0, 5, wave)]);
        juce::ignoreUnused (amt);
    };
    setLfo (lfo1, g("lfo_rate"), g("lfo_amount"), (int) g("lfo_wave"));
    setLfo (lfo2, g("lfo2_rate"), g("lfo2_amount"), (int) g("lfo2_wave"));
    setLfo (lfo3, g("lfo3_rate"), g("lfo3_amount"), (int) g("lfo3_wave"));

    float v1 = lfo1.process() * g("lfo_amount");
    float v2 = lfo2.process() * g("lfo2_amount");
    float v3 = lfo3.process() * g("lfo3_amount");
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO1, v1);
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO2, v2);
    modMatrix.setSourceValue (salek::ModMatrix::Source::LFO3, v3);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro1, g("macro1") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro2, g("macro2") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro3, g("macro3") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Macro4, g("macro4") * 2.f - 1.f);
    modMatrix.setSourceValue (salek::ModMatrix::Source::Random,
        juce::Random::getSystemRandom().nextFloat() * 2.f - 1.f);
    delay.setMix(g("delay_mix")); delay.setTimeMs(g("delay_time")); delay.setFeedback(g("delay_fb"));
    chorus.setMix(g("chorus_mix")); chorus.setRate(g("chorus_rate")); chorus.setDepth(g("chorus_depth"));
    reverb.setMix(g("reverb_mix")); reverb.setSize(g("reverb_size")); reverb.setDecay(g("reverb_decay"));
    reverb.setMode ((int) g("reverb_mode"));
    phaser.setMix(g("phaser_mix")); phaser.setRate(g("phaser_rate")); phaser.setDepth(g("phaser_depth"));
    distortion.setMix(g("dist_mix")); distortion.setDrive(g("dist_drive")); distortion.setBitcrush(g("dist_crush"));
    distortion.setMode ((int) g("dist_mode"));
    compressor.setThresholdDb(g("comp_threshold")); compressor.setRatio(g("comp_ratio")); compressor.setMix(g("comp_mix"));
    eq.setLowGainDb(g("eq_low")); eq.setMidGainDb(g("eq_mid")); eq.setHighGainDb(g("eq_high"));
    spatial.setAzimuth(g("spatial_azim")); spatial.setDistance(g("spatial_dist"));
    spatial.setSize(g("spatial_size")); spatial.setElevation(g("spatial_elev"));

    magic.setMode ((int) g("magic_mode"));
    magic.setXY (g("magic_x"), g("magic_y"));
    magic.setActive (g("magic_on") > 0.5f);
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    applyParamsToEngine();
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    juce::MidiBuffer routed;
    routed.addEvents (midi, 0, buffer.getNumSamples(), 0);

    const bool seqOn = apvts.getRawParameterValue("seq_on")->load() > 0.5f;
    const bool arpOn = apvts.getRawParameterValue("arp_on")->load() > 0.5f;
    stepSequencer.setEnabled (seqOn);
    arpeggiator.setEnabled (arpOn);

    if (seqOn)
    {
        stepSequencer.setRateDivisor ((int) apvts.getRawParameterValue("seq_rate")->load());
        for (const auto metadata : midi)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn()) stepSequencer.setRootNote (msg.getNoteNumber());
        }
        stepSequencer.process(buffer.getNumSamples(), routed);
    }

    if (arpOn)
    {
        arpeggiator.setRateDivisor ((int) apvts.getRawParameterValue("arp_rate")->load());
        arpeggiator.setOctaves ((int) apvts.getRawParameterValue("arp_octaves")->load());
        arpeggiator.process(buffer.getNumSamples(), routed);
    }

    midi.swapWith (routed);
    synthEngine.processBlock(buffer, midi);

    // Bassify: low-shelf-ish boost + soft grit (dubstep noise colour)
    const float bassify = apvts.getRawParameterValue("bassify")->load();
    if (bassify > 1e-4f)
    {
        static float lpL = 0.f, lpR = 0.f;
        const float coeff = 0.08f + bassify * 0.12f;
        const float grit = bassify * 0.35f;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float L = buffer.getSample (0, i);
            float R = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : L;
            lpL += coeff * (L - lpL);
            lpR += coeff * (R - lpR);
            float subL = lpL * (1.f + bassify * 1.8f);
            float subR = lpR * (1.f + bassify * 1.8f);
            // subtle even-order grit
            subL = subL + grit * subL * subL * (subL >= 0.f ? 1.f : -1.f);
            subR = subR + grit * subR * subR * (subR >= 0.f ? 1.f : -1.f);
            buffer.setSample (0, i, L * (1.f - bassify * 0.3f) + subL * bassify * 0.55f);
            if (buffer.getNumChannels() > 1)
                buffer.setSample (1, i, R * (1.f - bassify * 0.3f) + subR * bassify * 0.55f);
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
    phaser.process(buffer);
    distortion.process(buffer);
    eq.process(buffer);
    compressor.process(buffer);
    delay.process(buffer);
    reverb.process(buffer);
    spatial.process(buffer);
    magic.process(buffer);

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
    juce::XmlElement root ("SALEK_STATE");
    root.setAttribute ("version", 1);
    root.setAttribute ("program", currentProgram);
    if (auto ap = apvts.copyState().createXml())
        root.addChildElement (new juce::XmlElement (*ap));
    copyXmlToBinary (root, destData);
}
void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName ("SALEK_STATE"))
        {
            if (auto* ap = xml->getChildByName (apvts.state.getType()))
                apvts.replaceState (juce::ValueTree::fromXml (*ap));
            const int prog = xml->getIntAttribute ("program", -1);
            if (prog >= 0 && prog < (int) factoryPresets.size())
                currentProgram = prog;
        }
        else if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
    }
}

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor()
{
    return new SalekHightechAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SalekHightechAudioProcessor();
}

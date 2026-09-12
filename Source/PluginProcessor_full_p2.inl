
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
    synthEngine.setScaleMode((int)g("scale_mode"));
    synthEngine.setKoronCents(g("koron_cents"));
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

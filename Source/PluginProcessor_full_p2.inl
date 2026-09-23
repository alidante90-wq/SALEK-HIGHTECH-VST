
void SalekHightechAudioProcessor::applyParamsToEngine (int numSamples)
{
    auto g = [&](const char* id) -> float { if (auto* p = apvts.getRawParameterValue(id)) return p->load(); return 0.f; };

    // ---- Modulation sources first (then O(1) dest lookup) ----
    {
        auto setLfo = [] (salek::LFO& lfo, float rate, int wave)
        {
            lfo.setRate (rate);
            lfo.setAmount (1.0f);
            static const salek::LFO::Wave waves[] = {
                salek::LFO::Wave::Sine, salek::LFO::Wave::Triangle, salek::LFO::Wave::Saw,
                salek::LFO::Wave::Square, salek::LFO::Wave::SAndH, salek::LFO::Wave::Custom,
                salek::LFO::Wave::SmoothRnd, salek::LFO::Wave::Chaos, salek::LFO::Wave::Pulse,
                salek::LFO::Wave::Exp, salek::LFO::Wave::Sine3, salek::LFO::Wave::SoftSquare
            };
            lfo.setWave (waves[juce::jlimit (0, 11, wave)]);
        };
        setLfo (lfo1, g("lfo_rate"),  (int) g("lfo_wave"));
        setLfo (lfo2, g("lfo2_rate"), (int) g("lfo2_wave"));
        setLfo (lfo3, g("lfo3_rate"), (int) g("lfo3_wave"));
        const int ns = juce::jmax (1, numSamples);
        modMatrix.setSourceValue (salek::ModMatrix::Source::LFO1, lfo1.processBlock (ns) * g("lfo_amount"));
        modMatrix.setSourceValue (salek::ModMatrix::Source::LFO2, lfo2.processBlock (ns) * g("lfo2_amount"));
        modMatrix.setSourceValue (salek::ModMatrix::Source::LFO3, lfo3.processBlock (ns) * g("lfo3_amount"));
        modMatrix.setSourceValue (salek::ModMatrix::Source::Macro1, g("macro1") * 2.f - 1.f);
        modMatrix.setSourceValue (salek::ModMatrix::Source::Macro2, g("macro2") * 2.f - 1.f);
        modMatrix.setSourceValue (salek::ModMatrix::Source::Macro3, g("macro3") * 2.f - 1.f);
        modMatrix.setSourceValue (salek::ModMatrix::Source::Macro4, g("macro4") * 2.f - 1.f);
        modMatrix.setSourceValue (salek::ModMatrix::Source::Random,
            juce::Random::getSystemRandom().nextFloat() * 2.f - 1.f);
        {
            static float envApprox = 0.f;
            if (apvts.getRawParameterValue ("seq_on")->load() > 0.5f)
                envApprox = juce::jmax (envApprox * 0.985f, stepSequencer.getCurrentMod());
            else
                envApprox *= 0.985f;
            modMatrix.setSourceValue (salek::ModMatrix::Source::Env1, juce::jlimit (0.f, 1.f, envApprox));
        }
        mseg.setRateHz (g("mseg_rate"));
        mseg.setLoop (g("mseg_loop") > 0.5f);
        {
            static const salek::shae::MSEG::Curve curves[] = {
                salek::shae::MSEG::Curve::Linear, salek::shae::MSEG::Curve::Exp,
                salek::shae::MSEG::Curve::Log, salek::shae::MSEG::Curve::Smooth };
            mseg.setCurve (curves[juce::jlimit (0, 3, (int) g("mseg_curve"))]);
            const int sh = (int) g("mseg_shape");
            static int lastShape = -1;
            if (sh != lastShape)
            {
                lastShape = sh;
                if (sh == 0) mseg.loadADSRShape();
                else if (sh == 1) mseg.loadRampUp();
                else if (sh == 2) mseg.loadTriangle();
                else mseg.loadHitechBurst();
            }
            modMatrix.setSourceValue (salek::ModMatrix::Source::MSEG,
                mseg.processBlock (ns) * g("mseg_amount"));
        }
        modMatrix.setSourceValue (salek::ModMatrix::Source::Velocity, lastNoteVelocity);
        modMatrix.setSourceValue (salek::ModMatrix::Source::ModWheel, modWheelValue);
        modMatrix.finalizeBlock();
    }

    float o1l = juce::jlimit(0.f,1.f, g("osc1_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Level)*0.5f);
    float o2l = juce::jlimit(0.f,1.f, g("osc2_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Level)*0.5f);
    float o3l = juce::jlimit(0.f,1.f, g("osc3_level") + modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Level)*0.5f);
    synthEngine.setOsc1Level(o1l); synthEngine.setOsc2Level(o2l); synthEngine.setOsc3Level(o3l);
    synthEngine.setOsc1Pan(juce::jlimit(0.f,1.f, g("osc1_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Pan)*0.5f));
    synthEngine.setOsc2Pan(juce::jlimit(0.f,1.f, g("osc2_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Pan)*0.5f));
    synthEngine.setOsc3Pan(juce::jlimit(0.f,1.f, g("osc3_pan")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Pan)*0.5f));
    synthEngine.setOsc1TablePos(juce::jlimit(0.f,1.f, g("osc1_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Table)*0.5f));
    synthEngine.setOsc2TablePos(juce::jlimit(0.f,1.f, g("osc2_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Table)*0.5f));
    synthEngine.setOsc3TablePos(juce::jlimit(0.f,1.f, g("osc3_table")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Table)*0.5f));
    synthEngine.setOsc1Warp(juce::jlimit(0.f,1.f, g("osc1_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Warp)*0.5f));
    synthEngine.setOsc2Warp(juce::jlimit(0.f,1.f, g("osc2_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Warp)*0.5f));
    synthEngine.setOsc3Warp(juce::jlimit(0.f,1.f, g("osc3_warp")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Warp)*0.5f));
    synthEngine.setOsc1Fold(juce::jlimit(0.f,1.f,g("osc1_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Fold)*0.5f));
    synthEngine.setOsc2Fold(juce::jlimit(0.f,1.f,g("osc2_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Fold)*0.5f));
    synthEngine.setOsc3Fold(juce::jlimit(0.f,1.f,g("osc3_fold")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Fold)*0.5f));
    synthEngine.setOsc1Drive(juce::jlimit(0.f,1.f,g("osc1_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc1Drive)*0.5f));
    synthEngine.setOsc2Drive(juce::jlimit(0.f,1.f,g("osc2_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc2Drive)*0.5f));
    synthEngine.setOsc3Drive(juce::jlimit(0.f,1.f,g("osc3_drive")+modMatrix.getModulation(salek::ModMatrix::Dest::Osc3Drive)*0.5f));
    synthEngine.setOsc1Phase(g("osc1_phase")); synthEngine.setOsc2Phase(g("osc2_phase")); synthEngine.setOsc3Phase(g("osc3_phase"));
    synthEngine.setOsc1Rand(g("osc1_rand")); synthEngine.setOsc2Rand(g("osc2_rand")); synthEngine.setOsc3Rand(g("osc3_rand"));
    synthEngine.setOsc1Octave((int)g("osc1_octave")); synthEngine.setOsc2Octave((int)g("osc2_octave")); synthEngine.setOsc3Octave((int)g("osc3_octave"));
    synthEngine.setOsc1Semi((int)g("osc1_semi")); synthEngine.setOsc2Semi((int)g("osc2_semi")); synthEngine.setOsc3Semi((int)g("osc3_semi"));
    synthEngine.setUnison((int)g("unison_voices"));
    synthEngine.setUnisonDetune(g("unison_detune"));
    synthEngine.setUnisonSpread(g("unison_spread"));
    // Per-osc unison — always honour UNI/DET/SPR knobs (1..7)
    {
        const int u1 = juce::jlimit (1, 7, (int) std::lround (g("osc1_unison")));
        const int u2 = juce::jlimit (1, 7, (int) std::lround (g("osc2_unison")));
        const int u3 = juce::jlimit (1, 7, (int) std::lround (g("osc3_unison")));
        const float d1 = juce::jmax (0.f, g("osc1_udet"));
        const float d2 = juce::jmax (0.f, g("osc2_udet"));
        const float d3 = juce::jmax (0.f, g("osc3_udet"));
        const float s1 = juce::jlimit (0.f, 1.f, g("osc1_uspread"));
        const float s2 = juce::jlimit (0.f, 1.f, g("osc2_uspread"));
        const float s3 = juce::jlimit (0.f, 1.f, g("osc3_uspread"));
        synthEngine.setOsc1Unison (u1, d1, s1);
        synthEngine.setOsc2Unison (u2, d2, s2);
        synthEngine.setOsc3Unison (u3, d3, s3);
    }
    synthEngine.setFm2to1(g("fm_2to1")); synthEngine.setFm3to1(g("fm_3to1")); synthEngine.setFm3to2(g("fm_3to2"));
    synthEngine.setPm2to1(g("pm_2to1")); synthEngine.setRm2to1(g("rm_2to1")); synthEngine.setAm2to1(g("am_2to1"));
    float cut = g("filter_cutoff");
    float reso = g("filter_reso");
    // Smart macros (hardwired semantic + matrix sources):
    //   Macro1 BRIGHT  → filter cutoff/reso open
    //   Macro2 SPACE   → delay/reverb mix scale
    //   Macro3 DESTROY → distortion mix/drive/crush
    //   Macro4 WIDTH   → chorus mix scale
    // All four also feed ModMatrix as bipolar sources for free routing
    const float mBright = g("macro1");
    const float mSpace  = g("macro2");
    const float mDest   = g("macro3");
    const float mWidth  = g("macro4");
    cut *= (0.75f + mBright * 1.6f);
    reso = juce::jlimit (0.f, 0.98f, reso + mBright * 0.15f);
    // space → delay/reverb (applied later via param push if available)
    // push smart macros into FX (non-destructive scale)
    if (auto* p = apvts.getParameter ("delay_mix"))
        juce::ignoreUnused (p);
    // Smart macro scales are applied in processBlock's FX stage.
    cut *= std::pow(2.0f, modMatrix.getModulation(salek::ModMatrix::Dest::FilterCutoff) * 1.0f);
    synthEngine.setFilterCutoff(juce::jlimit(20.f, 20000.f, cut));
    synthEngine.setFilterResonance(juce::jlimit(0.f,1.f, g("filter_reso")+modMatrix.getModulation(salek::ModMatrix::Dest::FilterReso)*0.5f));
    synthEngine.setFilterDrive(g("filter_drive")); synthEngine.setFilterEnvAmt(juce::jlimit(0.f,1.f,g("filter_env")+modMatrix.getModulation(salek::ModMatrix::Dest::FilterEnv)*0.5f));
    synthEngine.setFilterMode((int)g("filter_mode"));
    synthEngine.setFilterRoute((int)g("filter_route"));
    synthEngine.setNoiseLevel(g("noise_level"));
    synthEngine.setSubLevel(g("sub_level"));
    synthEngine.setGlide(g("glide"));
    synthEngine.setAmpAttack(g("amp_attack")); synthEngine.setAmpDecay(g("amp_decay"));
    synthEngine.setAmpSustain(g("amp_sustain")); synthEngine.setAmpRelease(g("amp_release"));
    synthEngine.setLfoRate(g("lfo_rate")); synthEngine.setLfoAmount(g("lfo_amount")); synthEngine.setLfoWave((int)g("lfo_wave"));
    synthEngine.setScaleMode((int)g("scale_mode"));
    synthEngine.setKoronCents(g("koron_cents"));

    compressor.setRatio(g("comp_ratio")); compressor.setMix(g("comp_mix"));
    compressor.setDepth(g("comp_depth")); compressor.setAttackMs(g("comp_attack")); compressor.setReleaseMs(g("comp_release"));
    compressor.setMakeupDb(g("comp_gain"));
    // Per-band thresholds (explicit LO/MID/HI)
    compressor.setBandThresholdDb (0, g("comp_thr_lo"));
    compressor.setBandThresholdDb (1, g("comp_thr_mid"));
    compressor.setBandThresholdDb (2, g("comp_thr_hi"));
    // Also push global for makeup/display consistency
    compressor.setThresholdDb(g("comp_threshold"));
    eq.setLowGainDb(g("eq_low")); eq.setMidGainDb(g("eq_mid")); eq.setHighGainDb(g("eq_high"));
    spatial.setAzimuth(g("spatial_azim")); spatial.setDistance(g("spatial_dist"));
    spatial.setSize(g("spatial_size")); spatial.setElevation(g("spatial_elev"));

    magic.setMode ((int) g("magic_mode"));
    {
        float mx = g("magic_x") + modMatrix.getModulation (salek::ModMatrix::Dest::MagicX) * 0.5f;
        float my = g("magic_y") + modMatrix.getModulation (salek::ModMatrix::Dest::MagicY) * 0.5f;
        // SEQ → Magic modulation (step.modValue drives selected target)
        const int seqMag = (int) g("seq_magic_target");
        if (seqMag > 0 && g("seq_on") > 0.5f)
        {
            const float depth = g("seq_magic_depth");
            const float sm = stepSequencer.getCurrentMod(); // typically 0..1
            const float bipolar = sm * 2.f - 1.f;
            if (seqMag == 1 || seqMag == 3) mx += bipolar * depth;
            if (seqMag == 2 || seqMag == 3) my += bipolar * depth;
            if (seqMag == 4) // intensity → force active + scale XY toward corner
            {
                mx = mx * (1.f - depth) + sm * depth;
                my = my * (1.f - depth) + sm * depth;
                magic.setActive (true);
            }
        }
        mx = juce::jlimit (0.f, 1.f, mx);
        my = juce::jlimit (0.f, 1.f, my);
        magic.setXY (mx, my);
    }
    if (g("seq_magic_target") != 4.f)
        magic.setActive (g("magic_on") > 0.5f || (g("seq_on") > 0.5f && g("seq_magic_target") > 0.5f));
}

void SalekHightechAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    if (! SalekLicense::isLicensed())
    {
        // Early-access lock: silence until valid code for this machine
        buffer.clear();
        return;
    }

    juce::ScopedNoDenormals noDenormals;
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Capture performance MIDI before applying modulation so velocity/mod-wheel are current for this block.
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            lastNoteVelocity = msg.getFloatVelocity();
        else if (msg.isController() && msg.getControllerNumber() == 1)
            modWheelValue = msg.getControllerValue() / 127.f;
    }

    applyParamsToEngine (buffer.getNumSamples());
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
        juce::MidiBuffer arpIn;
        for (const auto metadata : routed)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                arpeggiator.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                arpeggiator.noteOff (msg.getNoteNumber());
            else
                arpIn.addEvent (msg, metadata.samplePosition);
        }
        routed.clear();
        routed.addEvents (arpIn, 0, buffer.getNumSamples(), 0);
        arpeggiator.process (buffer.getNumSamples(), routed);
    }

    midi.swapWith (routed);
    synthEngine.processBlock(buffer, midi);

    auto bypassed = [&](const char* id) -> bool {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load() > 0.5f;
        return false;
    };
    auto g = [&](const char* id) -> float {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return 0.f;
    };
   
    const float bassify = apvts.getRawParameterValue("bassify")->load();
    if (bassify > 1e-4f && ! bypassed ("bassify_bypass"))
    {
        const float a1 = 0.04f + bassify * 0.06f;
        const float a2 = 0.12f + bassify * 0.10f;
        const float amount = bassify;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float L = buffer.getSample (0, i);
            float R = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : L;
            bassLp1L += a1 * (L - bassLp1L); bassLp1R += a1 * (R - bassLp1R);
            bassLp2L += a2 * (bassLp1L - bassLp2L); bassLp2R += a2 * (bassLp1R - bassLp2R);
            float sub = 0.5f * (bassLp2L + bassLp2R);
            float grit = amount * 0.4f;
            sub = sub + grit * sub * sub * (sub >= 0.f ? 1.f : -1.f);
            sub = std::tanh (sub * (1.2f + amount * 1.6f));
            float dryKeep = 1.f - amount * 0.25f;
            float wet = sub * amount * 0.85f;
            buffer.setSample (0, i, L * dryKeep + wet);
            if (buffer.getNumChannels() > 1)
                buffer.setSample (1, i, R * dryKeep + wet);
        }
    }

    // Smart macro scales for the FX render stage.
    const float spaceScale = 0.15f + g ("macro2") * 0.85f;
    const float destScale  = juce::jlimit (0.f, 1.f, g ("macro3"));
    const float widthScale = 0.3f + juce::jlimit (0.f, 1.f, g ("macro4")) * 0.7f;

    // ---- Push all FX params (UI knobs were never wired → delay/reverb silent) ----
    {
        const float dMix = juce::jlimit (0.f, 1.f, g ("delay_mix") * spaceScale);
        delay.setMix (dMix);
        delay.setTimeMs (g ("delay_time"));
        delay.setTimeMsL (g ("delay_time_l"));
        delay.setTimeMsR (g ("delay_time_r"));
        delay.setFeedback (g ("delay_fb"));
        delay.setMode ((int) g ("delay_mode"));

        const float rMix = juce::jlimit (0.f, 1.f, g ("reverb_mix") * spaceScale);
        reverb.setMix (rMix);
        reverb.setSize (g ("reverb_size"));
        reverb.setDecay (g ("reverb_decay"));
        reverb.setDamping (g ("reverb_damping"));
        reverb.setMode ((int) g ("reverb_mode"));

        const float cMix = juce::jlimit (0.f, 1.f, g ("chorus_mix") * widthScale);
        chorus.setMix (cMix);
        chorus.setRate (g ("chorus_rate"));
        chorus.setDepth (g ("chorus_depth"));

        phaser.setMix (g ("phaser_mix"));
        phaser.setRate (g ("phaser_rate"));
        phaser.setDepth (g ("phaser_depth"));

        distortion.setMix (juce::jlimit (0.f, 1.f, g ("dist_mix") * (0.35f + destScale * 0.9f)));
        distortion.setDrive (juce::jlimit (0.f, 1.f, g ("dist_drive") * (0.5f + destScale)));
        distortion.setBitcrush (g ("dist_crush"));
        distortion.setMode ((int) g ("dist_mode"));

        formantFilter.setMorph (g ("formant_morph"));
        formantFilter.setAmount (g ("formant_amt"));

        resonator.setMix (g ("res_mix"));
        resonator.setDecay (g ("res_decay"));
        resonator.setBrightness (g ("res_bright"));
        resonator.setFrequency (g ("res_freq"));
        resonator.setMaterial ((int) g ("res_material"));

        spectralSmear.setMix (g ("spectral_mix"));
        spectralSmear.setAmount (g ("spectral_amt"));
        spectralSmear.setShift (g ("spectral_shift"));
        spectralSmear.setGate (g ("spectral_gate"));

        spatial.setAzimuth (g ("spatial_azim"));
        spatial.setDistance (g ("spatial_dist"));
        spatial.setSize (g ("spatial_size"));
        spatial.setElevation (g ("spatial_elev"));

        eq.setLowGainDb (g ("eq_low"));
        eq.setMidGainDb (g ("eq_mid"));
        eq.setHighGainDb (g ("eq_high"));
    }

    // Capture actual post-stage peaks for honest UI metering (no decorative fake waveform level).
    auto captureFx = [&] (int index)
    {
        float peak = 0.f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
        if (index >= 0 && index < 8) fxPeaks[(size_t) index].store (juce::jlimit (0.f, 1.f, peak));
    };
    for (auto& p : fxPeaks) p.store (0.f);

    // Mix=0 early-out saves a lot with 2+ instances in FL
    if (! bypassed ("chorus_bypass") && g ("chorus_mix") > 1e-4f)  chorus.process (buffer);
    captureFx (0);
    if (! bypassed ("phaser_bypass") && g ("phaser_mix") > 1e-4f)  phaser.process (buffer);
    captureFx (6);
    if (! bypassed ("dist_bypass") && g ("dist_mix") > 1e-4f)      distortion.process (buffer);
    captureFx (7);
    // Formant / vocal (SALEK signature)
    if (g ("formant_amt") > 1e-4f)
    {
        const int ns = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < ns; ++i)
        {
            float L = buffer.getSample (0, i);
            float R = ch > 1 ? buffer.getSample (1, i) : L;
            formantFilter.process (L, R);
            buffer.setSample (0, i, L);
            if (ch > 1) buffer.setSample (1, i, R);
        }
    }
    if (g ("res_mix") > 1e-4f)
        resonator.process (buffer);
    if (g ("spectral_mix") > 1e-4f)
        spectralSmear.process (buffer);
    if (! bypassed ("eq_bypass"))
    {
        if (std::abs (g ("eq_low")) > 0.05f || std::abs (g ("eq_mid")) > 0.05f || std::abs (g ("eq_high")) > 0.05f)
            eq.process (buffer);
    }
    captureFx (5);
    if (! bypassed ("comp_bypass") && g ("comp_mix") > 1e-4f)    compressor.process (buffer);
    captureFx (4);
    if (! bypassed ("delay_bypass") && g ("delay_mix") > 1e-4f)   delay.process (buffer);
    captureFx (1);
    if (! bypassed ("reverb_bypass") && g ("reverb_mix") > 1e-4f) reverb.process (buffer);
    captureFx (2);
    {
        const float az = std::abs (g ("spatial_azim"));
        const float ds = g ("spatial_dist");
        if (az > 0.5f || ds > 1e-3f || std::abs (g ("spatial_elev")) > 0.5f)
            spatial.process (buffer);
    }

    if (auto* gm = apvts.getRawParameterValue ("granular_mix"))
    {
        const float gMix = gm->load();
        if (gMix > 1e-4f)
        {
            granular.setMix (gMix);
            if (auto* p = apvts.getRawParameterValue ("granular_density")) granular.setDensity (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_size"))    granular.setGrainSize (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pos"))     granular.setPosition (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pitch"))   granular.setPitch (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_freeze"))  granular.setFreeze (p->load() > 0.5f);
            if (auto* p = apvts.getRawParameterValue ("granular_spray"))   granular.setSpray (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_pitchspray")) granular.setPitchSpray (p->load());
            if (auto* p = apvts.getRawParameterValue ("granular_feedback")) granular.setFeedback (p->load());
            granular.process (buffer);
        }
    }

    magic.process (buffer);

    float gain = apvts.getRawParameterValue("master_gain")->load();
    const float drive = apvts.getRawParameterValue("master_drive")->load();
    const float gMul = gain * (0.85f + drive * 0.1f);
    // SHAE Master Core: DC block → mono bass → soft clip → ceiling
    const float bassC = 0.08f; // ~120Hz @ 48k one-pole
    const int nS = buffer.getNumSamples();
    const int nCh = buffer.getNumChannels();
    if (nCh >= 2)
    {
        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getWritePointer (1);
        for (int i = 0; i < nS; ++i)
        {
            float xL = L[i] * gMul;
            float xR = R[i] * gMul;
            dcL += 0.0005f * (xL - dcL); xL -= dcL;
            dcR += 0.0005f * (xR - dcR); xR -= dcR;
            // Mono low end (stable club/system compatibility)
            masterLpL += bassC * (xL - masterLpL);
            masterLpR += bassC * (xR - masterLpR);
            float midBass = 0.5f * (masterLpL + masterLpR);
            float hiL = xL - masterLpL;
            float hiR = xR - masterLpR;
            xL = midBass + hiL;
            xR = midBass + hiR;
            xL = salek::shae::softClipComp (xL, drive * 0.6f);
            xR = salek::shae::softClipComp (xR, drive * 0.6f);
            const float aL = std::abs (xL), aR = std::abs (xR);
            if (aL > 0.88f) xL = std::copysign (0.88f + 0.1f * std::tanh ((aL - 0.88f) * 5.f), xL);
            if (aR > 0.88f) xR = std::copysign (0.88f + 0.1f * std::tanh ((aR - 0.88f) * 5.f), xR);
            L[i] = juce::jlimit (-0.97f, 0.97f, xL);
            R[i] = juce::jlimit (-0.97f, 0.97f, xR);
        }
    }
    else if (nCh == 1)
    {
        auto* d = buffer.getWritePointer (0);
        for (int i = 0; i < nS; ++i)
        {
            float x = d[i] * gMul;
            dcL += 0.0005f * (x - dcL); x -= dcL;
            x = salek::shae::softClipComp (x, drive * 0.6f);
            d[i] = juce::jlimit (-0.97f, 0.97f, x);
        }
    }

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer (ch);
        const int n = buffer.getNumSamples();
        for (int i = 0; i < n; ++i)
        {
            float x = d[i];
            x = std::tanh (x * 1.05f);
            const float ax = std::abs (x);
            if (ax > 0.88f)
            {
                const float s = (x >= 0.0f) ? 1.0f : -1.0f;
                x = s * (0.88f + 0.12f * std::tanh ((ax - 0.88f) * 6.0f));
            }
            d[i] = x;
        }
    }

    {
            const float coeff = 0.08f;
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float L = buffer.getSample (0, i);
            float R = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : L;
            airHpL += coeff * ((L - airHpL));
            airHpR += coeff * ((R - airHpR));
            float airL = (L - airHpL) * 0.18f;
            float airR = (R - airHpR) * 0.18f;
            buffer.setSample (0, i, L + airL);
            if (buffer.getNumChannels() > 1)
                buffer.setSample (1, i, R + airR);
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
    root.setAttribute ("version", 2);
    root.setAttribute ("program", currentProgram);

    if (auto ap = apvts.copyState().createXml())
        root.addChildElement (new juce::XmlElement (*ap));

    // Mod-matrix routes and custom LFO tables are DSP state, not APVTS parameters.
    auto* matrixXml = root.createNewChildElement ("MOD_MATRIX");
    for (const auto& route : modMatrix.getRoutes())
        if (route.active)
        {
            auto* x = matrixXml->createNewChildElement ("ROUTE");
            x->setAttribute ("src", (int) route.source);
            x->setAttribute ("dst", (int) route.dest);
            x->setAttribute ("amount", (double) route.amount);
        }

    auto* lfoXml = root.createNewChildElement ("LFO_TABLES");
    const salek::LFO* lfos[] = { &lfo1, &lfo2, &lfo3 };
    for (int li = 0; li < 3; ++li)
    {
        auto* x = lfoXml->createNewChildElement ("LFO");
        x->setAttribute ("index", li);
        for (int i = 0; i < salek::LFO::TableSize; ++i)
            x->setAttribute ("p" + juce::String (i), (double) lfos[li]->getCustomPoint (i));
    }

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

            modMatrix.clear();
            if (auto* mx = xml->getChildByName ("MOD_MATRIX"))
                for (auto* x : mx->getChildIterator())
                    if (x->hasTagName ("ROUTE"))
                        modMatrix.addRoute (
                            (salek::ModMatrix::Source) juce::jlimit (0, (int) salek::ModMatrix::Source::NumSources - 1, x->getIntAttribute ("src", 0)),
                            (salek::ModMatrix::Dest) juce::jlimit (0, (int) salek::ModMatrix::Dest::NumDests - 1, x->getIntAttribute ("dst", 0)),
                            (float) x->getDoubleAttribute ("amount", 0.0));

            if (auto* lx = xml->getChildByName ("LFO_TABLES"))
            {
                salek::LFO* lfos[] = { &lfo1, &lfo2, &lfo3 };
                for (auto* x : lx->getChildIterator())
                {
                    if (! x->hasTagName ("LFO")) continue;
                    const int li = juce::jlimit (0, 2, x->getIntAttribute ("index", 0));
                    for (int i = 0; i < salek::LFO::TableSize; ++i)
                        lfos[li]->setCustomPoint (i, (float) x->getDoubleAttribute (
                            "p" + juce::String (i), lfos[li]->getCustomPoint (i)));
                }
            }

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

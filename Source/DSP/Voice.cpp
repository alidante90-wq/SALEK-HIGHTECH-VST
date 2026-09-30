#include "Voice.h"

SalekVoice::SalekVoice (juce::AudioProcessorValueTreeState& vts, ModulationMatrix& matrix)
    : apvts (vts), modMatrix (matrix)
{
}

bool SalekVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SalekSound*> (sound) != nullptr;
}

void SalekVoice::prepare (double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;
    osc1.prepare (sampleRate);
    osc2.prepare (sampleRate);
    osc3.prepare (sampleRate);
    subOsc.prepare (sampleRate);

    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    filter.prepare (spec);

    ampEnv.prepare (sampleRate);
    filterEnv.prepare (sampleRate);
    modEnv.prepare (sampleRate);

    lfo1.prepare (sampleRate);
    lfo2.prepare (sampleRate);
    lfo3.prepare (sampleRate);
    lfo4.prepare (sampleRate);

    static const float centsTable[maxUnison] = { -18.f, -7.f, 0.f, 7.f, 18.f };
    for (int i = 0; i < maxUnison; ++i)
        unisonDetune[i] = std::pow (2.0f, centsTable[i] / 1200.0f) - 1.0f;

    isPrepared = true;
    paramsDirty = true;
}

void SalekVoice::startNote (int midiNoteNumber, float vel, juce::SynthesiserSound*, int)
{
    currentNote = midiNoteNumber;
    velocity = vel;
    noteHz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);

    osc1.reset(); osc2.reset(); osc3.reset(); subOsc.reset();
    filter.reset();

    ampEnv.noteOn();
    filterEnv.noteOn();
    modEnv.noteOn();

    lfo1.reset(); lfo2.reset(); lfo3.reset(); lfo4.reset();
    paramsDirty = true;
}

void SalekVoice::stopNote (float, bool allowTailOff)
{
    ampEnv.noteOff();
    filterEnv.noteOff();
    modEnv.noteOff();

    if (! allowTailOff || ! ampEnv.isActive())
        clearCurrentNote();
}

void SalekVoice::updateParameters()
{
    auto get = [this] (const juce::String& id) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return 0.0f;
    };

    auto calcFreq = [&] (const juce::String& pre) -> float
    {
        float oct  = get (pre + "octave");
        float semi = get (pre + "semitone");
        float det  = get (pre + "detune");
        return noteHz * std::pow (2.0f, (oct * 12.0f + semi + det) / 12.0f);
    };

    baseOsc1Freq = calcFreq ("osc1_");
    baseOsc2Freq = calcFreq ("osc2_");
    baseOsc3Freq = calcFreq ("osc3_");

    baseOsc1Level = get ("osc1_level");
    baseOsc2Level = get ("osc2_level");
    baseOsc3Level = get ("osc3_level");
    baseSubLevel  = get ("sub_level");
    baseNoiseLevel= get ("noise_level");

    baseOsc1WT    = get ("osc1_wtpos");
    baseOsc1Morph = get ("osc1_morph");
    baseOsc1Warp  = get ("osc1_warp");
    baseOsc1FM    = get ("osc1_fm");
    baseOsc1AM    = get ("osc1_am");
    baseOsc1RM    = get ("osc1_rm");

    baseOsc2WT    = get ("osc2_wtpos");
    baseOsc2Morph = get ("osc2_morph");
    baseOsc2Warp  = get ("osc2_warp");
    baseOsc2FM    = get ("osc2_fm");
    baseOsc2AM    = get ("osc2_am");
    baseOsc2RM    = get ("osc2_rm");

    baseOsc3WT    = get ("osc3_wtpos");
    baseOsc3Morph = get ("osc3_morph");
    baseOsc3Warp  = get ("osc3_warp");
    baseOsc3FM    = get ("osc3_fm");
    baseOsc3AM    = get ("osc3_am");
    baseOsc3RM    = get ("osc3_rm");

    float subOct = get ("sub_octave");
    baseSubFreq  = noteHz * std::pow (2.0f, subOct);

    baseCutoff   = get ("filter_cutoff");
    baseRes      = get ("filter_res");
    baseDrive    = get ("filter_drive");
    baseKeytrack = get ("filter_keytrack");
    int ftype    = (int) get ("filter_type");
    filter.setType (static_cast<MultiFilter::Type> (ftype));
    filter.setDrive (baseDrive);

    ampEnv.setAttack  (get ("env1_attack"));
    ampEnv.setDecay   (get ("env1_decay"));
    ampEnv.setSustain (get ("env1_sustain"));
    ampEnv.setRelease (get ("env1_release"));

    filterEnv.setAttack  (get ("env2_attack"));
    filterEnv.setDecay   (get ("env2_decay"));
    filterEnv.setSustain (get ("env2_sustain"));
    filterEnv.setRelease (get ("env2_release"));

    modEnv.setAttack  (get ("env3_attack"));
    modEnv.setDecay   (get ("env3_decay"));
    modEnv.setSustain (get ("env3_sustain"));
    modEnv.setRelease (get ("env3_release"));

    lfo1.setRate (get ("lfo1_rate")); lfo1.setDepth (get ("lfo1_depth"));
    lfo1.setShape (static_cast<LFO::Shape> ((int) get ("lfo1_shape")));
    lfo1.setOneShot (get ("lfo1_oneshot") > 0.5f);

    lfo2.setRate (get ("lfo2_rate")); lfo2.setDepth (get ("lfo2_depth"));
    lfo2.setShape (static_cast<LFO::Shape> ((int) get ("lfo2_shape")));

    lfo3.setRate (get ("lfo3_rate")); lfo3.setDepth (get ("lfo3_depth"));
    lfo3.setShape (static_cast<LFO::Shape> ((int) get ("lfo3_shape")));

    lfo4.setRate (get ("lfo4_rate")); lfo4.setDepth (get ("lfo4_depth"));
    lfo4.setShape (static_cast<LFO::Shape> ((int) get ("lfo4_shape")));

    unisonVoices = juce::jlimit (1, maxUnison, (int) get ("osc1_unison"));
    unisonDetuneAmt = get ("osc1_udet") * 0.01f;

    paramsDirty = false;
}

void SalekVoice::applyModulation (float* destOffsets)
{
    for (int i = 0; i < ModulationMatrix::numDests; ++i)
        destOffsets[i] = 0.0f;

    modSources[ModulationMatrix::LFO1] = lfo1.process();
    modSources[ModulationMatrix::LFO2] = lfo2.process();
    modSources[ModulationMatrix::LFO3] = lfo3.process();
    modSources[ModulationMatrix::LFO4] = lfo4.process();
    modSources[ModulationMatrix::Env1] = ampEnv.getLevel();
    modSources[ModulationMatrix::Env2] = filterEnv.getLevel();
    modSources[ModulationMatrix::Env3] = modEnv.getLevel();
    modSources[ModulationMatrix::Velocity]   = velocity * 2.0f - 1.0f;
    modSources[ModulationMatrix::NoteNumber] = (currentNote - 60) / 48.0f;

    modMatrix.process (modSources, destOffsets);
}

void SalekVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isPrepared || ! isVoiceActive())
        return;

    updateParameters();

    auto* left  = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : left;

    const float noteOffsetBase = (currentNote - 60) * baseKeytrack * 40.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        applyModulation (modDests);

        const float mOsc1Level = modDests[ModulationMatrix::Osc1Level];
        const float mOsc1WT    = modDests[ModulationMatrix::Osc1WTPos];
        const float mOsc1Morph = modDests[ModulationMatrix::Osc1Morph];
        const float mOsc1Warp  = modDests[ModulationMatrix::Osc1Warp];
        const float mOsc1FM    = modDests[ModulationMatrix::Osc1FM];
        const float mOsc1AM    = modDests[ModulationMatrix::Osc1AM];
        const float mOsc1RM    = modDests[ModulationMatrix::Osc1RM];

        const float mOsc2Level = modDests[ModulationMatrix::Osc2Level];
        const float mOsc2WT    = modDests[ModulationMatrix::Osc2WTPos];
        const float mOsc2Morph = modDests[ModulationMatrix::Osc2Morph];
        const float mOsc2Warp  = modDests[ModulationMatrix::Osc2Warp];
        const float mOsc2FM    = modDests[ModulationMatrix::Osc2FM];
        const float mOsc2AM    = modDests[ModulationMatrix::Osc2AM];
        const float mOsc2RM    = modDests[ModulationMatrix::Osc2RM];

        const float mOsc3Level = modDests[ModulationMatrix::Osc3Level];
        const float mOsc3WT    = modDests[ModulationMatrix::Osc3WTPos];
        const float mOsc3Morph = modDests[ModulationMatrix::Osc3Morph];
        const float mOsc3Warp  = modDests[ModulationMatrix::Osc3Warp];
        const float mOsc3FM    = modDests[ModulationMatrix::Osc3FM];
        const float mOsc3AM    = modDests[ModulationMatrix::Osc3AM];
        const float mOsc3RM    = modDests[ModulationMatrix::Osc3RM];

        const float mSubLevel  = modDests[ModulationMatrix::SubLevel];
        const float mNoise     = modDests[ModulationMatrix::NoiseLevel];
        const float mCutoff    = modDests[ModulationMatrix::FilterCutoff] * 5500.0f;
        const float mRes       = modDests[ModulationMatrix::FilterRes] * 0.45f;
        const float mDrive     = modDests[ModulationMatrix::FilterDrive] * 0.5f;
        const float mAmp       = modDests[ModulationMatrix::AmpLevel];

        osc1.setFrequency (baseOsc1Freq);
        osc1.setLevel (juce::jlimit (0.0f, 1.5f, (baseOsc1Level + mOsc1Level) * velocity));
        osc1.setWavetablePos (juce::jlimit (0.0f, 1.0f, baseOsc1WT + mOsc1WT));
        osc1.setMorph (juce::jlimit (0.0f, 1.0f, baseOsc1Morph + mOsc1Morph));
        osc1.setWarp (juce::jlimit (0.0f, 1.0f, baseOsc1Warp + mOsc1Warp));
        osc1.setFM (baseOsc1FM + mOsc1FM);
        osc1.setAM (juce::jlimit (0.0f, 1.0f, baseOsc1AM + mOsc1AM));
        osc1.setRM (juce::jlimit (0.0f, 1.0f, baseOsc1RM + mOsc1RM));
        osc1.setWarpMode (Oscillator::WarpMode::Fold);

        osc2.setFrequency (baseOsc2Freq);
        osc2.setLevel (juce::jlimit (0.0f, 1.5f, (baseOsc2Level + mOsc2Level) * velocity));
        osc2.setWavetablePos (juce::jlimit (0.0f, 1.0f, baseOsc2WT + mOsc2WT));
        osc2.setMorph (juce::jlimit (0.0f, 1.0f, baseOsc2Morph + mOsc2Morph));
        osc2.setWarp (juce::jlimit (0.0f, 1.0f, baseOsc2Warp + mOsc2Warp));
        osc2.setFM (baseOsc2FM + mOsc2FM);
        osc2.setAM (juce::jlimit (0.0f, 1.0f, baseOsc2AM + mOsc2AM));
        osc2.setRM (juce::jlimit (0.0f, 1.0f, baseOsc2RM + mOsc2RM));
        osc2.setWarpMode (Oscillator::WarpMode::PD);

        osc3.setFrequency (baseOsc3Freq);
        osc3.setLevel (juce::jlimit (0.0f, 1.5f, (baseOsc3Level + mOsc3Level) * velocity));
        osc3.setWavetablePos (juce::jlimit (0.0f, 1.0f, baseOsc3WT + mOsc3WT));
        osc3.setMorph (juce::jlimit (0.0f, 1.0f, baseOsc3Morph + mOsc3Morph));
        osc3.setWarp (juce::jlimit (0.0f, 1.0f, baseOsc3Warp + mOsc3Warp));
        osc3.setFM (baseOsc3FM + mOsc3FM);
        osc3.setAM (juce::jlimit (0.0f, 1.0f, baseOsc3AM + mOsc3AM));
        osc3.setRM (juce::jlimit (0.0f, 1.0f, baseOsc3RM + mOsc3RM));
        osc3.setWarpMode (Oscillator::WarpMode::Sync);

        subOsc.setFrequency (baseSubFreq);
        subOsc.setLevel (juce::jlimit (0.0f, 1.5f, (baseSubLevel + mSubLevel) * velocity));
        subOsc.setWavetablePos (0.0f);
        const float noiseAmt = juce::jlimit (0.0f, 1.5f, (baseNoiseLevel + mNoise) * velocity);

        filter.setCutoff (juce::jlimit (20.0f, 20000.0f, baseCutoff + mCutoff + noteOffsetBase));
        filter.setResonance (juce::jlimit (0.0f, 1.0f, baseRes + mRes));
        filter.setDrive (juce::jlimit (0.0f, 1.0f, baseDrive + mDrive));

        const float s2prev = osc2.getLastSample();
        const float s3prev = osc3.getLastSample();
        osc1.setPhaseMod (s2prev * 0.7f + s3prev * 0.3f + modSources[ModulationMatrix::LFO1] * 0.35f);
        osc2.setPhaseMod (s3prev * 0.5f + modSources[ModulationMatrix::LFO2] * 0.25f);
        osc3.setPhaseMod (modSources[ModulationMatrix::LFO3] * 0.2f);

        osc1.setRingModInput (s2prev);
        osc2.setRingModInput (s3prev);
        osc3.setRingModInput (s2prev);

        float s1 = osc1.process();
        float s2 = osc2.process();
        float s3 = osc3.process();
        float sub = subOsc.process();
        float n = (noiseRandom.nextFloat() * 2.0f - 1.0f) * noiseAmt;

        float mixed = s1 + s2 + s3 + sub + n;

        float e2 = filterEnv.process();
        float e1 = ampEnv.process();
        float e3 = modEnv.process();
        juce::ignoreUnused (e3);

        float y = filter.processSample (mixed * (1.0f + e2 * 0.3f));
        y *= e1 * juce::jlimit (0.0f, 1.5f, 1.0f + mAmp);

        y = std::tanh (y * 1.35f);

        left[i]  += y;
        right[i] += y;

        if (! ampEnv.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}

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

    // Default unison detunes
    for (int i = 0; i < maxUnison; ++i)
        unisonDetune[i] = (i - maxUnison / 2) * 0.07f;

    isPrepared = true;
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
}

void SalekVoice::stopNote (float, bool allowTailOff)
{
    ampEnv.noteOff();
    filterEnv.noteOff();
    modEnv.noteOff();

    if (!allowTailOff || !ampEnv.isActive())
        clearCurrentNote();
}

void SalekVoice::updateParameters()
{
    auto get = [this](const juce::String& id) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return p->load();
        return 0.0f;
    };

    // Base frequencies with octave/semi/detune
    auto calcFreq = [&](const juce::String& pre) -> float
    {
        float oct  = get (pre + "octave");
        float semi = get (pre + "semitone");
        float det  = get (pre + "detune");
        return noteHz * std::pow (2.0f, (oct * 12.0f + semi + det) / 12.0f);
    };

    osc1.setFrequency (calcFreq ("osc1_"));
    osc1.setLevel (get ("osc1_level") * velocity);
    osc1.setWavetablePos (get ("osc1_wtpos"));
    osc1.setMorph (get ("osc1_morph"));
    osc1.setWarp (get ("osc1_warp"));
    osc1.setFM (get ("osc1_fm"));
    osc1.setWarpMode (Oscillator::WarpMode::Fold);

    osc2.setFrequency (calcFreq ("osc2_"));
    osc2.setLevel (get ("osc2_level") * velocity);
    osc2.setWavetablePos (get ("osc2_wtpos"));
    osc2.setMorph (get ("osc2_morph"));
    osc2.setWarp (get ("osc2_warp"));
    osc2.setFM (get ("osc2_fm"));
    osc2.setWarpMode (Oscillator::WarpMode::PD);

    osc3.setFrequency (calcFreq ("osc3_"));
    osc3.setLevel (get ("osc3_level") * velocity);
    osc3.setWavetablePos (get ("osc3_wtpos"));
    osc3.setMorph (get ("osc3_morph"));
    osc3.setWarp (get ("osc3_warp"));
    osc3.setFM (get ("osc3_fm"));
    osc3.setWarpMode (Oscillator::WarpMode::Sync);

    float subOct = get ("sub_octave");
    subOsc.setFrequency (noteHz * std::pow (2.0f, subOct));
    subOsc.setLevel (get ("sub_level") * velocity);
    subOsc.setWavetablePos (0.0f);

    noiseLevel = get ("noise_level") * velocity;

    // Filter base
    float cutoff = get ("filter_cutoff");
    float keytrack = get ("filter_keytrack");
    float noteOffset = (currentNote - 60) * keytrack * 40.0f;
    filter.setCutoff (cutoff + noteOffset);
    filter.setResonance (get ("filter_res"));
    filter.setDrive (get ("filter_drive"));
    int ftype = (int) get ("filter_type");
    filter.setType (static_cast<MultiFilter::Type> (ftype));

    // Envelopes
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

    // LFOs
    lfo1.setRate (get ("lfo1_rate")); lfo1.setDepth (get ("lfo1_depth"));
    lfo1.setShape (static_cast<LFO::Shape> ((int) get ("lfo1_shape")));
    lfo1.setOneShot (get ("lfo1_oneshot") > 0.5f);

    lfo2.setRate (get ("lfo2_rate")); lfo2.setDepth (get ("lfo2_depth"));
    lfo2.setShape (static_cast<LFO::Shape> ((int) get ("lfo2_shape")));

    lfo3.setRate (get ("lfo3_rate")); lfo3.setDepth (get ("lfo3_depth"));
    lfo3.setShape (static_cast<LFO::Shape> ((int) get ("lfo3_shape")));

    lfo4.setRate (get ("lfo4_rate")); lfo4.setDepth (get ("lfo4_depth"));
    lfo4.setShape (static_cast<LFO::Shape> ((int) get ("lfo4_shape")));
}

void SalekVoice::applyModulation (float* destOffsets)
{
    // Zero
    for (int i = 0; i < ModulationMatrix::numDests; ++i)
        destOffsets[i] = 0.0f;

    // Fill sources
    modSources[ModulationMatrix::LFO1] = lfo1.process();
    modSources[ModulationMatrix::LFO2] = lfo2.process();
    modSources[ModulationMatrix::LFO3] = lfo3.process();
    modSources[ModulationMatrix::LFO4] = lfo4.process();
    modSources[ModulationMatrix::Env1] = ampEnv.getLevel();
    modSources[ModulationMatrix::Env2] = filterEnv.getLevel();
    modSources[ModulationMatrix::Env3] = modEnv.getLevel();
    // Macros / velocity / note filled by processor or here
    modSources[ModulationMatrix::Velocity] = velocity * 2.0f - 1.0f;
    modSources[ModulationMatrix::NoteNumber] = (currentNote - 60) / 48.0f;

    modMatrix.process (modSources, destOffsets);
}

void SalekVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isPrepared || !isVoiceActive())
        return;

    updateParameters();

    auto* left  = outputBuffer.getWritePointer (0, startSample);
    auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : left;

    for (int i = 0; i < numSamples; ++i)
    {
        // Modulation
        applyModulation (modDests);

        // Apply modulation from matrix (real)
        float wtMod1   = modDests[ModulationMatrix::Osc1WTPos];
        float warpMod1 = modDests[ModulationMatrix::Osc1Warp];
        float fmMod1   = modDests[ModulationMatrix::Osc1FM];
        float cutMod   = modDests[ModulationMatrix::FilterCutoff] * 5500.0f;
        float resMod   = modDests[ModulationMatrix::FilterRes] * 0.45f;

        // Re-read base and add mod (simple but effective)
        auto get = [this](const juce::String& id) -> float {
            if (auto* p = apvts.getRawParameterValue (id)) return p->load();
            return 0.0f;
        };
        osc1.setWavetablePos (juce::jlimit (0.0f, 1.0f, get("osc1_wtpos") + wtMod1));
        osc1.setWarp (juce::jlimit (0.0f, 1.0f, get("osc1_warp") + warpMod1));
        osc1.setFM (get("osc1_fm") + fmMod1);
        filter.setCutoff (juce::jlimit (20.0f, 20000.0f, get("filter_cutoff") + cutMod + (currentNote-60)*get("filter_keytrack")*40.0f));
        filter.setResonance (juce::jlimit (0.0f, 1.0f, get("filter_res") + resMod));

        // Classic FM stack for hi-tech / screech
        float s2 = osc2.getLastSample();
        float s3 = osc3.getLastSample();
        osc1.setPhaseMod (s2 * 0.7f + s3 * 0.3f + modSources[ModulationMatrix::LFO1] * 0.4f);
        osc2.setPhaseMod (s3 * 0.5f + modSources[ModulationMatrix::LFO2] * 0.3f);
        osc3.setPhaseMod (modSources[ModulationMatrix::LFO3] * 0.25f);

        // Generate
        float s1 = osc1.process();
        s2 = osc2.process();
        s3 = osc3.process();
        float sub = subOsc.process();
        float n = (noiseRandom.nextFloat() * 2.0f - 1.0f) * noiseLevel;

        float mixed = s1 + s2 + s3 + sub + n;

        // Filter with env + mod
        float e2 = filterEnv.process();
        float e1 = ampEnv.process();
        float e3 = modEnv.process();

        // Re-apply cutoff with env (strong for acid / psy)
        // (In a full version we would have continuous cutoff modulation)
        float y = filter.processSample (mixed * (1.0f + e2 * 0.3f));

        y *= e1;

        // Soft saturation for loudness and character
        y = std::tanh (y * 1.35f);

        left[i]  += y;
        right[i] += y;

        if (!ampEnv.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}

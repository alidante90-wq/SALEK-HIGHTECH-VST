#include "PluginProcessor.h"
#include "PluginEditor.h"

SalekHightechAudioProcessor::SalekHightechAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    osc1TablePos = apvts.getRawParameterValue ("osc1_table");
    osc2TablePos = apvts.getRawParameterValue ("osc2_table");
    osc3TablePos = apvts.getRawParameterValue ("osc3_table");
    osc1Level    = apvts.getRawParameterValue ("osc1_level");
    osc2Level    = apvts.getRawParameterValue ("osc2_level");
    osc3Level    = apvts.getRawParameterValue ("osc3_level");
    osc1Octave   = apvts.getRawParameterValue ("osc1_octave");
    osc2Octave   = apvts.getRawParameterValue ("osc2_octave");
    osc3Octave   = apvts.getRawParameterValue ("osc3_octave");
    osc1Semi     = apvts.getRawParameterValue ("osc1_semi");
    osc2Semi     = apvts.getRawParameterValue ("osc2_semi");
    osc3Semi     = apvts.getRawParameterValue ("osc3_semi");
    osc1Fine     = apvts.getRawParameterValue ("osc1_fine");
    osc2Fine     = apvts.getRawParameterValue ("osc2_fine");
    osc3Fine     = apvts.getRawParameterValue ("osc3_fine");
    osc1Detune   = apvts.getRawParameterValue ("osc1_detune");
    osc2Detune   = apvts.getRawParameterValue ("osc2_detune");
    osc3Detune   = apvts.getRawParameterValue ("osc3_detune");
    osc1Warp     = apvts.getRawParameterValue ("osc1_warp");
    osc2Warp     = apvts.getRawParameterValue ("osc2_warp");
    osc3Warp     = apvts.getRawParameterValue ("osc3_warp");
    osc1Phase    = apvts.getRawParameterValue ("osc1_phase");
    osc2Phase    = apvts.getRawParameterValue ("osc2_phase");
    osc3Phase    = apvts.getRawParameterValue ("osc3_phase");
    osc1Fold     = apvts.getRawParameterValue ("osc1_fold");
    osc2Fold     = apvts.getRawParameterValue ("osc2_fold");
    osc3Fold     = apvts.getRawParameterValue ("osc3_fold");
    osc1Drive    = apvts.getRawParameterValue ("osc1_drive");
    osc2Drive    = apvts.getRawParameterValue ("osc2_drive");
    osc3Drive    = apvts.getRawParameterValue ("osc3_drive");
    fm2to1       = apvts.getRawParameterValue ("fm_2to1");
    fm3to1       = apvts.getRawParameterValue ("fm_3to1");
    fm3to2       = apvts.getRawParameterValue ("fm_3to2");
    pm2to1       = apvts.getRawParameterValue ("pm_2to1");
    pm3to1       = apvts.getRawParameterValue ("pm_3to1");
    am2to1       = apvts.getRawParameterValue ("am_2to1");
    rm2to1       = apvts.getRawParameterValue ("rm_2to1");
    filterCutoff = apvts.getRawParameterValue ("filter_cutoff");
    filterReso   = apvts.getRawParameterValue ("filter_reso");
    filterDrive  = apvts.getRawParameterValue ("filter_drive");
    filterMode   = apvts.getRawParameterValue ("filter_mode");
    filterEnvAmt = apvts.getRawParameterValue ("filter_env");
    ampAttack    = apvts.getRawParameterValue ("amp_attack");
    ampDecay     = apvts.getRawParameterValue ("amp_decay");
    ampSustain   = apvts.getRawParameterValue ("amp_sustain");
    ampRelease   = apvts.getRawParameterValue ("amp_release");
    lfoRate      = apvts.getRawParameterValue ("lfo_rate");
    lfoAmount    = apvts.getRawParameterValue ("lfo_amount");
    lfoWave      = apvts.getRawParameterValue ("lfo_wave");
    macro1       = apvts.getRawParameterValue ("macro1");
    macro2       = apvts.getRawParameterValue ("macro2");
    macro3       = apvts.getRawParameterValue ("macro3");
    macro4       = apvts.getRawParameterValue ("macro4");
    delayTime    = apvts.getRawParameterValue ("delay_time");
    delayFb      = apvts.getRawParameterValue ("delay_fb");
    delayMix     = apvts.getRawParameterValue ("delay_mix");
    masterDrive  = apvts.getRawParameterValue ("master_drive");
    arpEnabled   = apvts.getRawParameterValue ("arp_enabled");
    arpRate      = apvts.getRawParameterValue ("arp_rate");
    arpOctaves   = apvts.getRawParameterValue ("arp_octaves");
    arpGate      = apvts.getRawParameterValue ("arp_gate");
    arpDir       = apvts.getRawParameterValue ("arp_dir");
    seqEnabled   = apvts.getRawParameterValue ("seq_enabled");
    seqRate      = apvts.getRawParameterValue ("seq_rate");
    seqSteps     = apvts.getRawParameterValue ("seq_steps");
    seqRoot      = apvts.getRawParameterValue ("seq_root");
    labMorph     = apvts.getRawParameterValue ("lab_morph");
    labFold      = apvts.getRawParameterValue ("lab_fold");
    labDrive     = apvts.getRawParameterValue ("lab_drive");
    labRegen     = apvts.getRawParameterValue ("lab_regen");
}

SalekHightechAudioProcessor::~SalekHightechAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout SalekHightechAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addFloat = [&] (const juce::String& id, const juce::String& name,
                         float min, float max, float def)
    {
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> (min, max), def));
    };

    auto addInt = [&] (const juce::String& id, const juce::String& name,
                       int min, int max, int def)
    {
        params.push_back (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id, 1 }, name, min, max, def));
    };

    // OSC 1
    addFloat ("osc1_table",  "OSC1 Table",  0.0f, 1.0f, 0.0f);
    addFloat ("osc1_level",  "OSC1 Level",  0.0f, 1.0f, 0.7f);
    addInt   ("osc1_octave", "OSC1 Octave", -4, 4, 0);
    addInt   ("osc1_semi",   "OSC1 Semi",   -12, 12, 0);
    addFloat ("osc1_fine",   "OSC1 Fine",   -100.0f, 100.0f, 0.0f);
    addFloat ("osc1_detune", "OSC1 Detune", -50.0f, 50.0f, 0.0f);
    addFloat ("osc1_warp",   "OSC1 Warp",   0.0f, 1.0f, 0.0f);
    addFloat ("osc1_phase",  "OSC1 Phase",  0.0f, 1.0f, 0.0f);
    addFloat ("osc1_fold",   "OSC1 Fold",   0.0f, 1.0f, 0.0f);
    addFloat ("osc1_drive",  "OSC1 Drive",  0.0f, 1.0f, 0.0f);

    // OSC 2
    addFloat ("osc2_table",  "OSC2 Table",  0.0f, 1.0f, 0.33f);
    addFloat ("osc2_level",  "OSC2 Level",  0.0f, 1.0f, 0.5f);
    addInt   ("osc2_octave", "OSC2 Octave", -4, 4, 0);
    addInt   ("osc2_semi",   "OSC2 Semi",   -12, 12, 0);
    addFloat ("osc2_fine",   "OSC2 Fine",   -100.0f, 100.0f, 0.0f);
    addFloat ("osc2_detune", "OSC2 Detune", -50.0f, 50.0f, 7.0f);
    addFloat ("osc2_warp",   "OSC2 Warp",   0.0f, 1.0f, 0.0f);
    addFloat ("osc2_phase",  "OSC2 Phase",  0.0f, 1.0f, 0.0f);
    addFloat ("osc2_fold",   "OSC2 Fold",   0.0f, 1.0f, 0.0f);
    addFloat ("osc2_drive",  "OSC2 Drive",  0.0f, 1.0f, 0.0f);

    // OSC 3
    addFloat ("osc3_table",  "OSC3 Table",  0.0f, 1.0f, 0.66f);
    addFloat ("osc3_level",  "OSC3 Level",  0.0f, 1.0f, 0.4f);
    addInt   ("osc3_octave", "OSC3 Octave", -4, 4, -1);
    addInt   ("osc3_semi",   "OSC3 Semi",   -12, 12, 0);
    addFloat ("osc3_fine",   "OSC3 Fine",   -100.0f, 100.0f, 0.0f);
    addFloat ("osc3_detune", "OSC3 Detune", -50.0f, 50.0f, -5.0f);
    addFloat ("osc3_warp",   "OSC3 Warp",   0.0f, 1.0f, 0.0f);
    addFloat ("osc3_phase",  "OSC3 Phase",  0.0f, 1.0f, 0.0f);
    addFloat ("osc3_fold",   "OSC3 Fold",   0.0f, 1.0f, 0.0f);
    addFloat ("osc3_drive",  "OSC3 Drive",  0.0f, 1.0f, 0.0f);

    addFloat ("fm_2to1", "FM 2→1", 0.0f, 1.0f, 0.0f);
    addFloat ("fm_3to1", "FM 3→1", 0.0f, 1.0f, 0.0f);
    addFloat ("fm_3to2", "FM 3→2", 0.0f, 1.0f, 0.0f);
    addFloat ("pm_2to1", "PM 2→1", 0.0f, 1.0f, 0.0f);
    addFloat ("pm_3to1", "PM 3→1", 0.0f, 1.0f, 0.0f);
    addFloat ("am_2to1", "AM 2→1", 0.0f, 1.0f, 0.0f);
    addFloat ("rm_2to1", "RM 2→1", 0.0f, 1.0f, 0.0f);

    addFloat ("filter_cutoff", "Filter Cutoff", 20.0f, 20000.0f, 8000.0f);
    addFloat ("filter_reso",   "Filter Reso",   0.0f, 1.0f, 0.3f);
    addFloat ("filter_drive",  "Filter Drive",  0.0f, 1.0f, 0.0f);
    addInt   ("filter_mode",   "Filter Mode",   0, 3, 0);
    addFloat ("filter_env",    "Filter Env",    0.0f, 1.0f, 0.4f);

    addFloat ("amp_attack",  "Amp Attack",  0.001f, 5.0f, 0.01f);
    addFloat ("amp_decay",   "Amp Decay",   0.001f, 5.0f, 0.1f);
    addFloat ("amp_sustain", "Amp Sustain", 0.0f, 1.0f, 0.8f);
    addFloat ("amp_release", "Amp Release", 0.001f, 5.0f, 0.2f);

    addFloat ("lfo_rate",   "LFO Rate",   0.01f, 40.0f, 2.0f);
    addFloat ("lfo_amount", "LFO Amount", 0.0f, 1.0f, 0.0f);
    addInt   ("lfo_wave",   "LFO Wave",   0, 4, 0);

    addFloat ("macro1", "Macro 1", 0.0f, 1.0f, 0.0f);
    addFloat ("macro2", "Macro 2", 0.0f, 1.0f, 0.0f);
    addFloat ("macro3", "Macro 3", 0.0f, 1.0f, 0.0f);
    addFloat ("macro4", "Macro 4", 0.0f, 1.0f, 0.0f);

    addFloat ("delay_time", "Delay Time", 1.0f, 1500.0f, 350.0f);
    addFloat ("delay_fb",   "Delay FB",   0.0f, 0.9f, 0.25f);
    addFloat ("delay_mix",  "Delay Mix",  0.0f, 1.0f, 0.0f);
    addFloat ("master_drive","Master Drive", 0.0f, 1.0f, 0.0f);

    addFloat ("arp_enabled", "Arp On", 0.0f, 1.0f, 0.0f);
    addFloat ("arp_rate",    "Arp Rate", 1.0f, 8.0f, 4.0f);
    addFloat ("arp_octaves", "Arp Octaves", 1.0f, 4.0f, 1.0f);
    addFloat ("arp_gate",    "Arp Gate", 0.1f, 1.0f, 0.6f);
    addFloat ("arp_dir",     "Arp Dir", 0.0f, 3.0f, 0.0f);

    addFloat ("seq_enabled", "Seq On", 0.0f, 1.0f, 0.0f);
    addFloat ("seq_rate",    "Seq Rate", 1.0f, 8.0f, 4.0f);
    addFloat ("seq_steps",   "Seq Steps", 1.0f, 16.0f, 16.0f);
    addFloat ("seq_root",    "Seq Root", 24.0f, 60.0f, 36.0f);

    addFloat ("lab_morph",   "Lab Morph", 0.0f, 1.0f, 0.0f);
    addFloat ("lab_fold",    "Lab Fold",  0.0f, 1.0f, 0.0f);
    addFloat ("lab_drive",   "Lab Drive", 0.0f, 1.0f, 0.0f);
    addFloat ("lab_regen",   "Lab Regen", 0.0f, 1.0f, 0.0f);

    return { params.begin(), params.end() };
}

const juce::String SalekHightechAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SalekHightechAudioProcessor::acceptsMidi() const { return true; }
bool SalekHightechAudioProcessor::producesMidi() const { return false; }
bool SalekHightechAudioProcessor::isMidiEffect() const { return false; }
double SalekHightechAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SalekHightechAudioProcessor::getNumPrograms() { return 1; }
int SalekHightechAudioProcessor::getCurrentProgram() { return 0; }
void SalekHightechAudioProcessor::setCurrentProgram (int) {}
const juce::String SalekHightechAudioProcessor::getProgramName (int) { return {}; }
void SalekHightechAudioProcessor::changeProgramName (int, const juce::String&) {}

void SalekHightechAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synthEngine.prepareToPlay (sampleRate, samplesPerBlock);
    delay.prepare (sampleRate, samplesPerBlock);
    arpeggiator.prepare (sampleRate);
    stepSequencer.prepare (sampleRate);
    stepSequencer.initDefaultPattern();
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

void SalekHightechAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Arpeggiator
    if (arpEnabled && arpEnabled->load() > 0.5f)
    {
        arpeggiator.setEnabled (true);
        if (arpRate)    arpeggiator.setRateDivisor (static_cast<int> (arpRate->load()));
        if (arpOctaves) arpeggiator.setOctaves (static_cast<int> (arpOctaves->load()));
        if (arpGate)    arpeggiator.setGate (arpGate->load());
        if (arpDir)
        {
            int d = static_cast<int> (arpDir->load());
            using D = salek::Arpeggiator::Direction;
            arpeggiator.setDirection (d == 1 ? D::Down : d == 2 ? D::UpDown : d == 3 ? D::Random : D::Up);
        }

        // Feed held notes from incoming MIDI and generate arp MIDI
        for (const auto metadata : midi)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
                arpeggiator.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
            else if (msg.isNoteOff())
                arpeggiator.noteOff (msg.getNoteNumber());
        }
        midi.clear();
        arpeggiator.process (buffer.getNumSamples(), midi);
    }
    else
    {
        arpeggiator.setEnabled (false);
    }

    // Step Sequencer
    if (seqEnabled && seqEnabled->load() > 0.5f)
    {
        stepSequencer.setEnabled (true);
        if (seqRate)  stepSequencer.setRateDivisor (static_cast<int> (seqRate->load()));
        if (seqSteps) stepSequencer.setNumSteps (static_cast<int> (seqSteps->load()));
        if (seqRoot)  stepSequencer.setRootNote (static_cast<int> (seqRoot->load()));

        // If arp is off we clear and let seq generate; if both on, seq adds on top
        if (!(arpEnabled && arpEnabled->load() > 0.5f))
            midi.clear();
        stepSequencer.process (buffer.getNumSamples(), midi);

        // Feed seq mod into matrix
        synthEngine.getModMatrix().setSourceValue (
            salek::ModMatrix::Source::Random, // reuse Random slot for seq mod for now
            stepSequencer.getCurrentModValue());
    }
    else
    {
        stepSequencer.setEnabled (false);
    }



    // Push parameters to engine (real-time safe atomics)
    if (osc1TablePos) synthEngine.setOsc1TablePos (osc1TablePos->load());
    if (osc2TablePos) synthEngine.setOsc2TablePos (osc2TablePos->load());
    if (osc3TablePos) synthEngine.setOsc3TablePos (osc3TablePos->load());
    if (osc1Level)    synthEngine.setOsc1Level (osc1Level->load());
    if (osc2Level)    synthEngine.setOsc2Level (osc2Level->load());
    if (osc3Level)    synthEngine.setOsc3Level (osc3Level->load());
    if (osc1Octave)   synthEngine.setOsc1Octave (static_cast<int> (osc1Octave->load()));
    if (osc2Octave)   synthEngine.setOsc2Octave (static_cast<int> (osc2Octave->load()));
    if (osc3Octave)   synthEngine.setOsc3Octave (static_cast<int> (osc3Octave->load()));
    if (osc1Semi)     synthEngine.setOsc1Semi (static_cast<int> (osc1Semi->load()));
    if (osc2Semi)     synthEngine.setOsc2Semi (static_cast<int> (osc2Semi->load()));
    if (osc3Semi)     synthEngine.setOsc3Semi (static_cast<int> (osc3Semi->load()));
    if (osc1Fine)     synthEngine.setOsc1Fine (osc1Fine->load());
    if (osc2Fine)     synthEngine.setOsc2Fine (osc2Fine->load());
    if (osc3Fine)     synthEngine.setOsc3Fine (osc3Fine->load());
    if (osc1Detune)   synthEngine.setOsc1Detune (osc1Detune->load());
    if (osc2Detune)   synthEngine.setOsc2Detune (osc2Detune->load());
    if (osc3Detune)   synthEngine.setOsc3Detune (osc3Detune->load());
    if (osc1Warp)     synthEngine.setOsc1Warp (osc1Warp->load());
    if (osc2Warp)     synthEngine.setOsc2Warp (osc2Warp->load());
    if (osc3Warp)     synthEngine.setOsc3Warp (osc3Warp->load());
    if (osc1Phase)    synthEngine.setOsc1Phase (osc1Phase->load());
    if (osc2Phase)    synthEngine.setOsc2Phase (osc2Phase->load());
    if (osc3Phase)    synthEngine.setOsc3Phase (osc3Phase->load());
    if (osc1Fold)     synthEngine.setOsc1Fold (osc1Fold->load());
    if (osc2Fold)     synthEngine.setOsc2Fold (osc2Fold->load());
    if (osc3Fold)     synthEngine.setOsc3Fold (osc3Fold->load());
    if (osc1Drive)    synthEngine.setOsc1Drive (osc1Drive->load());
    if (osc2Drive)    synthEngine.setOsc2Drive (osc2Drive->load());
    if (osc3Drive)    synthEngine.setOsc3Drive (osc3Drive->load());
    if (fm2to1)       synthEngine.setFm2to1 (fm2to1->load());
    if (fm3to1)       synthEngine.setFm3to1 (fm3to1->load());
    if (fm3to2)       synthEngine.setFm3to2 (fm3to2->load());
    if (pm2to1)       synthEngine.setPm2to1 (pm2to1->load());
    if (pm3to1)       synthEngine.setPm3to1 (pm3to1->load());
    if (am2to1)       synthEngine.setAm2to1 (am2to1->load());
    if (rm2to1)       synthEngine.setRm2to1 (rm2to1->load());
    if (filterCutoff) synthEngine.setFilterCutoff (filterCutoff->load());
    if (filterReso)   synthEngine.setFilterResonance (filterReso->load());
    if (filterDrive)  synthEngine.setFilterDrive (filterDrive->load());
    if (filterMode)   synthEngine.setFilterMode (static_cast<int> (filterMode->load()));
    if (filterEnvAmt) synthEngine.setFilterEnvAmt (filterEnvAmt->load());
    if (ampAttack)    synthEngine.setAmpAttack (ampAttack->load());
    if (ampDecay)     synthEngine.setAmpDecay (ampDecay->load());
    if (ampSustain)   synthEngine.setAmpSustain (ampSustain->load());
    if (ampRelease)   synthEngine.setAmpRelease (ampRelease->load());
    if (lfoRate)      synthEngine.setLfoRate (lfoRate->load());
    if (lfoAmount)    synthEngine.setLfoAmount (lfoAmount->load());
    if (lfoWave)      synthEngine.setLfoWave (static_cast<int> (lfoWave->load()));

    // Macros (simple real routings)
    const float m1 = macro1 ? macro1->load() : 0.0f;
    const float m2 = macro2 ? macro2->load() : 0.0f;
    const float m3 = macro3 ? macro3->load() : 0.0f;
    const float m4 = macro4 ? macro4->load() : 0.0f;

    if (filterCutoff) {
        float c = filterCutoff->load() * (0.3f + 0.7f * (1.0f - m1) + m1 * 2.5f);
        synthEngine.setFilterCutoff (c);
    }
    if (fm2to1) synthEngine.setFm2to1 (fm2to1->load() + m2 * 0.6f);
    if (osc1Fold) synthEngine.setOsc1Fold (juce::jlimit (0.0f, 1.0f, (osc1Fold ? osc1Fold->load() : 0.0f) + m3 * 0.7f));
    if (osc1Warp) synthEngine.setOsc1Warp (juce::jlimit (0.0f, 1.0f, (osc1Warp ? osc1Warp->load() : 0.0f) + m4 * 0.6f));
    // --- Modulation Matrix (real) ---
    auto& matrix = synthEngine.getModMatrix();
    // Feed sources
    matrix.setSourceValue (salek::ModMatrix::Source::Macro1, m1);
    matrix.setSourceValue (salek::ModMatrix::Source::Macro2, m2);
    matrix.setSourceValue (salek::ModMatrix::Source::Macro3, m3);
    matrix.setSourceValue (salek::ModMatrix::Source::Macro4, m4);
    if (lfoAmount) // approximate LFO depth as source strength
        matrix.setSourceValue (salek::ModMatrix::Source::LFO1, lfoAmount->load());

    // Ensure a few useful default routes exist (only once would be better, but cheap)
    static bool routesSeeded = false;
    if (!routesSeeded)
    {
        matrix.addRoute (salek::ModMatrix::Source::LFO1,   salek::ModMatrix::Dest::FilterCutoff, 0.4f);
        matrix.addRoute (salek::ModMatrix::Source::Macro1, salek::ModMatrix::Dest::FilterCutoff, 0.6f);
        matrix.addRoute (salek::ModMatrix::Source::Macro2, salek::ModMatrix::Dest::Fm2to1, 0.5f);
        matrix.addRoute (salek::ModMatrix::Source::Macro3, salek::ModMatrix::Dest::Osc1Fold, 0.5f);
        matrix.addRoute (salek::ModMatrix::Source::Macro4, salek::ModMatrix::Dest::Osc1Warp, 0.5f);
        routesSeeded = true;
    }

    // Apply matrix modulation on top of base values
    {
        float cut = filterCutoff ? filterCutoff->load() : 8000.0f;
        cut *= std::pow (2.0f, matrix.getModulation (salek::ModMatrix::Dest::FilterCutoff) * 2.0f);
        synthEngine.setFilterCutoff (cut);

        float fm = (fm2to1 ? fm2to1->load() : 0.0f) + matrix.getModulation (salek::ModMatrix::Dest::Fm2to1);
        synthEngine.setFm2to1 (juce::jlimit (0.0f, 1.0f, fm));

        float fold = (osc1Fold ? osc1Fold->load() : 0.0f) + matrix.getModulation (salek::ModMatrix::Dest::Osc1Fold);
        synthEngine.setOsc1Fold (juce::jlimit (0.0f, 1.0f, fold));

        float warp = (osc1Warp ? osc1Warp->load() : 0.0f) + matrix.getModulation (salek::ModMatrix::Dest::Osc1Warp);
        synthEngine.setOsc1Warp (juce::jlimit (0.0f, 1.0f, warp));
    }



    // Wavetable Lab actions
    static float lastMorph = -1.0f, lastFold = -1.0f, lastDrive = -1.0f, lastRegen = -1.0f;
    const float lm = labMorph ? labMorph->load() : 0.0f;
    const float lf = labFold  ? labFold->load()  : 0.0f;
    const float ld = labDrive ? labDrive->load() : 0.0f;
    const float lr = labRegen ? labRegen->load() : 0.0f;
    if (std::abs (lm - lastMorph) > 0.01f) { synthEngine.labMorph (lm); lastMorph = lm; }
    if (std::abs (lf - lastFold) > 0.02f || std::abs (ld - lastDrive) > 0.02f) {
        synthEngine.labProcessFrames (lf, ld); lastFold = lf; lastDrive = ld;
    }
    if (lr > 0.5f && lastRegen <= 0.5f) { synthEngine.labRegenerate(); }
    lastRegen = lr;

    synthEngine.processBlock (buffer, midi);

    // Master soft drive
    if (masterDrive)
    {
        const float d = masterDrive->load();
        if (d > 1.0e-4f)
        {
            const float g = 1.0f + d * 4.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                    data[i] = std::tanh (data[i] * g) * 0.9f;
            }
        }
    }

    // Delay
    if (delayTime) delay.setTimeMs (delayTime->load());
    if (delayFb)   delay.setFeedback (delayFb->load());
    if (delayMix)  delay.setMix (delayMix->load());
    delay.process (buffer);
}

bool SalekHightechAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SalekHightechAudioProcessor::createEditor()
{
    return new SalekHightechAudioProcessorEditor (*this);
}

void SalekHightechAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("salek_version", 1, nullptr); // versioned for future compatibility
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SalekHightechAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SalekHightechAudioProcessor();
}

#pragma once
#include <JuceHeader.h>
#include <array>
#include <random>

class GitiFxAudioProcessor : public juce::AudioProcessor
{
public:
    GitiFxAudioProcessor();
    ~GitiFxAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }
    int getNumPrograms() override { return 8; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int i) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getGranularActivity() const noexcept { return granularActivity.load(); }

private:
    struct Grain { float pos=0, age=0, len=1, rate=1, pan=0; bool active=false; };
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioBuffer<float> grainBuffer;
    std::array<Grain,64> grains;
    int writePos=0;
    double sampleRate=44100.0;
    int currentProgram=0;
    float delayBufL[192000]{}, delayBufR[192000]{};
    int delayPos=0;
    juce::dsp::IIR::Filter<float> filterL, filterR;
    juce::dsp::Reverb reverb;
    std::mt19937 rng{0x53414c45};
    std::uniform_real_distribution<float> uni{0.f,1.f};
    std::atomic<float> granularActivity{0.f};

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float p(const char* id, float d) const;
    void spawnGrain();
    static float window(float x);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GitiFxAudioProcessor)
};

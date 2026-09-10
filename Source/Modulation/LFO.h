#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class LFO {
public:
    enum class Wave { Sine, Triangle, Saw, Square, Random, SAndH };
    void prepare(double sampleRate){ sr=sampleRate>0?sampleRate:44100; phase=0; }
    void reset() noexcept { phase=0; lastSH=0; }
    void setRate(float hz) noexcept { rate=juce::jlimit(0.01f,40.f,hz); phaseInc=double(rate)/sr; }
    void setWave(Wave w) noexcept { wave=w; }
    void setAmount(float a) noexcept { amount=juce::jlimit(0.f,1.f,a); }
    float process() noexcept {
        float v=0, p=float(phase);
        switch(wave){
            case Wave::Sine: v=std::sin(p*juce::MathConstants<float>::twoPi); break;
            case Wave::Triangle: v=1-4*std::abs(p-0.5f); break;
            case Wave::Saw: v=2*p-1; break;
            case Wave::Square: v=p<0.5f?1.f:-1.f; break;
            default:
                if(phase<phaseInc) lastSH=juce::Random::getSystemRandom().nextFloat()*2-1;
                v=lastSH; break;
        }
        phase+=phaseInc; if(phase>=1) phase-=1;
        return v*amount;
    }
private:
    double sr=44100, phase=0, phaseInc=0; float rate=1, amount=0, lastSH=0;
    Wave wave=Wave::Sine;
};
}

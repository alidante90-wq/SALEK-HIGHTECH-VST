#pragma once
#include <JuceHeader.h>
#include <cmath>
namespace salek {
class StateVariableFilter {
public:
    enum class Mode { LowPass, HighPass, BandPass, Notch };
    void prepare(double sampleRate){ sr=sampleRate>0?sampleRate:44100; reset(); }
    void reset() noexcept { ic1eq=ic2eq=0; }
    void setCutoff(float hz) noexcept { cutoff=juce::jlimit(20.f,float(sr*0.45),hz); update(); }
    void setResonance(float r) noexcept { resonance=juce::jlimit(0.f,1.f,r); update(); }
    void setMode(Mode m) noexcept { mode=m; }
    void setDrive(float d) noexcept { drive=juce::jlimit(0.f,1.f,d); }
    float process(float x) noexcept {
        if(drive>1e-4f){ float g=1+drive*3; x=std::tanh(x*g); }
        float v3=x-ic2eq, v1=a1*ic1eq+a2*v3, v2=ic2eq+a2*ic1eq+a3*v3;
        ic1eq=2*v1-ic1eq; ic2eq=2*v2-ic2eq;
        switch(mode){
            case Mode::LowPass: return v2;
            case Mode::HighPass: return x-k*v1-v2;
            case Mode::BandPass: return v1;
            case Mode::Notch: return x-k*v1;
            default: return v2;
        }
    }
private:
    void update() noexcept {
        float g=std::tan(juce::MathConstants<float>::pi*cutoff/float(sr));
        k=2-1.8f*resonance; a1=1/(1+g*(g+k)); a2=g*a1; a3=g*a2;
    }
    double sr=44100; float cutoff=1000, resonance=0.2f, drive=0;
    Mode mode=Mode::LowPass; float ic1eq=0,ic2eq=0,k=0,a1=0,a2=0,a3=0;
};
}

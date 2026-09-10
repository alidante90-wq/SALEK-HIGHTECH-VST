#pragma once
#include <JuceHeader.h>
#include <vector>
namespace salek {
class SimpleDelay {
public:
    void prepare(double sampleRate, int maxBlock) {
        sr=sampleRate; buffer.assign(size_t(sampleRate*2+maxBlock)*2, 0.f); writePos=0;
    }
    void setTimeMs(float ms) noexcept { delaySamples=juce::jlimit(1.f,float(sr*1.8),ms*0.001f*float(sr)); }
    void setFeedback(float fb) noexcept { feedback=juce::jlimit(0.f,0.95f,fb); }
    void setMix(float m) noexcept { mix=juce::jlimit(0.f,1.f,m); }
    void process(juce::AudioBuffer<float>& buf) noexcept {
        if(mix<1e-4f) return;
        int n=buf.getNumSamples(), ch=buf.getNumChannels(), bs=int(buffer.size()/2);
        for(int i=0;i<n;++i){
            for(int c=0;c<juce::jmin(2,ch);++c){
                float* d=buf.getWritePointer(c);
                int rp=(writePos-int(delaySamples)+bs)%bs;
                float delayed=buffer[size_t(rp*2+c)], in=d[i];
                d[i]=in*(1-mix)+delayed*mix;
                buffer[size_t(writePos*2+c)]=in+delayed*feedback;
            }
            writePos=(writePos+1)%bs;
        }
    }
private:
    double sr=44100; std::vector<float> buffer; int writePos=0;
    float delaySamples=300, feedback=0.3f, mix=0;
};
}

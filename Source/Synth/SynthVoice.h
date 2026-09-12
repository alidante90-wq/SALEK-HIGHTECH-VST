#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"
#include "../Oscillators/WavetableOscillator.h"
#include "../Wavetables/WavetableData.h"
#include "../Filters/StateVariableFilter.h"
#include "../Modulation/LFO.h"
#include "../DSP/SmoothedValue.h"
#include "PersianScale.h"

namespace salek {

class SynthVoice : public juce::SynthesiserVoice {
public:
    SynthVoice() {
        osc1.setWavetable(&sharedWavetable); osc2.setWavetable(&sharedWavetable); osc3.setWavetable(&sharedWavetable);
        osc1.setLevel(0.7f); osc2.setLevel(0.5f); osc3.setLevel(0.4f); osc3Octave = -1;
        for (int u = 0; u < maxUnison; ++u) { uniOsc[u].setWavetable(&sharedWavetable); uniOsc[u].setLevel(0.7f); }
    }
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*>(s) != nullptr; }
    void prepareToPlay(double sr, int) {
        currentSampleRate = sr; osc1.prepare(sr); osc2.prepare(sr); osc3.prepare(sr);
        for (int u = 0; u < maxUnison; ++u) uniOsc[u].prepare(sr);
        filter.prepare(sr); lfo.prepare(sr); adsr.setSampleRate(sr); adsr.setParameters(adsrParams);
        cutoffSmoother.reset(baseCutoff); cutoffSmoother.setTimeMs(5.f, sr);
    }
    void startNote(int note, float vel, juce::SynthesiserSound*, int) override {
        currentMidiNote = note; currentVelocity = vel; isNoteOn = true;
        osc1.reset(); osc2.reset(); osc3.reset();
        for (int u = 0; u < maxUnison; ++u) uniOsc[u].reset();
        updateFrequencies(); adsr.noteOn();
    }
    void stopNote(float, bool allowTailOff) override {
        if (allowTailOff) adsr.noteOff();
        else { clearCurrentNote(); adsr.reset(); isNoteOn = false; }
    }
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>& out, int start, int num) override;

    void setOsc1TablePos(float v){osc1.setTablePosition(v); for(int u=0;u<maxUnison;++u)uniOsc[u].setTablePosition(v);}
    void setOsc2TablePos(float v){osc2.setTablePosition(v);} void setOsc3TablePos(float v){osc3.setTablePosition(v);}
    void setOsc1Level(float v){osc1.setLevel(v); for(int u=0;u<maxUnison;++u)uniOsc[u].setLevel(v);}
    void setOsc2Level(float v){osc2.setLevel(v);} void setOsc3Level(float v){osc3.setLevel(v);}
    void setOsc1Octave(int v){osc1Octave=v; updateFrequencies();} void setOsc2Octave(int v){osc2Octave=v; updateFrequencies();} void setOsc3Octave(int v){osc3Octave=v; updateFrequencies();}
    void setOsc1Semi(int v){osc1Semi=v; updateFrequencies();} void setOsc2Semi(int v){osc2Semi=v; updateFrequencies();} void setOsc3Semi(int v){osc3Semi=v; updateFrequencies();}
    void setOsc1Fine(float v){osc1Fine=v; updateFrequencies();} void setOsc2Fine(float v){osc2Fine=v; updateFrequencies();} void setOsc3Fine(float v){osc3Fine=v; updateFrequencies();}
    void setOsc1Detune(float v){osc1.setDetuneCents(v);} void setOsc2Detune(float v){osc2.setDetuneCents(v);} void setOsc3Detune(float v){osc3.setDetuneCents(v);}
    void setOsc1Warp(float v){osc1.setWarp(v); for(int u=0;u<maxUnison;++u)uniOsc[u].setWarp(v);}
    void setOsc2Warp(float v){osc2.setWarp(v);} void setOsc3Warp(float v){osc3.setWarp(v);}
    void setOsc1Fold(float v){osc1.setFold(v); for(int u=0;u<maxUnison;++u)uniOsc[u].setFold(v);}
    void setOsc2Fold(float v){osc2.setFold(v);} void setOsc3Fold(float v){osc3.setFold(v);}
    void setOsc1Drive(float v){osc1.setDrive(v); for(int u=0;u<maxUnison;++u)uniOsc[u].setDrive(v);}
    void setOsc2Drive(float v){osc2.setDrive(v);} void setOsc3Drive(float v){osc3.setDrive(v);}
    void setOsc1Phase(float v){osc1.setPhaseOffset(v);} void setOsc2Phase(float v){osc2.setPhaseOffset(v);} void setOsc3Phase(float v){osc3.setPhaseOffset(v);}
    void setUnison(int v){unisonVoices=juce::jlimit(1,maxUnison,v);}
    void setUnisonDetune(float c){unisonDetune=juce::jlimit(0.f,100.f,c);}
    void setUnisonSpread(float s){unisonSpread=juce::jlimit(0.f,1.f,s);}
    void setFm2to1(float v){fm2to1=v;} void setFm3to1(float v){fm3to1=v;} void setFm3to2(float v){fm3to2=v;}
    void setPm2to1(float v){pm2to1=v;} void setPm3to1(float v){pm3to1=v;} void setAm2to1(float v){am2to1=v;} void setRm2to1(float v){rm2to1=v;}
    void setFilterCutoff(float) {} void setFilterBaseCutoff(float hz){baseCutoff=hz; filter.setCutoff(hz);}
    void setFilterResonance(float r){filter.setResonance(r);} void setFilterDrive(float d){filter.setDrive(d);}
    void setFilterMode(int m){ using M=StateVariableFilter::Mode; filter.setMode(m==1?M::HighPass:m==2?M::BandPass:m==3?M::Notch:M::LowPass); }
    void setFilterEnvAmt(float a){filterEnvAmt=a;}
    void setAmpAttack(float s){adsrParams.attack=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setAmpDecay(float s){adsrParams.decay=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setAmpSustain(float s){adsrParams.sustain=juce::jlimit(0.f,1.f,s); adsr.setParameters(adsrParams);}
    void setAmpRelease(float s){adsrParams.release=juce::jmax(0.001f,s); adsr.setParameters(adsrParams);}
    void setLfoRate(float hz){lfo.setRate(hz);} void setLfoAmount(float a){lfo.setAmount(a);}
    void setLfoWave(int w){ using W=LFO::Wave; lfo.setWave(w==1?W::Triangle:w==2?W::Saw:w==3?W::Square:w==4?W::SAndH:W::Sine); }
    void labRegenerate(){sharedWavetable.generateDefaultTables();}
    void labProcessFrames(float fold, float drive){ for(int i=0;i<sharedWavetable.getNumFrames();++i) sharedWavetable.processFrame(i,fold,drive); }
    void labMorph(float t){ if(sharedWavetable.getNumFrames()>=4) sharedWavetable.morphFrames(0,3,1,t); }
    void setScaleMode(int m){ scaleMode = juce::jlimit(0, PersianScale::NumModes-1, m); updateFrequencies(); }
    void setKoronCents(float c){ koronCents = juce::jlimit(-50.f, 50.f, c); updateFrequencies(); }
    void setScaleRoot(int r){ scaleRoot = r; updateFrequencies(); }

private:
    void updateFrequencies();
    float noteToHz(int note, int oct, int semi, float fine) const {
        return PersianScale::noteToHz (note, oct, semi, fine, scaleMode, koronCents, scaleRoot);
    }
    static constexpr int maxUnison = 7;
    WavetableOscillator osc1, osc2, osc3;
    WavetableOscillator uniOsc[maxUnison];
    int unisonVoices = 1;
    float unisonDetune = 12.f, unisonSpread = 0.7f;
    Wavetable sharedWavetable;
    StateVariableFilter filter;
    LFO lfo;
    SmoothedValue cutoffSmoother;
    double currentSampleRate = 44100;
    float currentVelocity = 0;
    int currentMidiNote = 60;
    int scaleMode = 0;
    float koronCents = 0.f;
    int scaleRoot = 60;
    int osc1Octave=0, osc2Octave=0, osc3Octave=-1;
    int osc1Semi=0, osc2Semi=0, osc3Semi=0;
    float osc1Fine=0, osc2Fine=0, osc3Fine=0;
    float fm2to1=0, fm3to1=0, fm3to2=0, pm2to1=0, pm3to1=0, am2to1=0, rm2to1=0;
    float filterEnvAmt=0.5f, baseCutoff=8000;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams{0.01f, 0.1f, 0.8f, 0.2f};
    bool isNoteOn = false;
};
}

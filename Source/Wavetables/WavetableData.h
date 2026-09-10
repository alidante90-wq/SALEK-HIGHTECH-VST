#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <cmath>

namespace salek {

struct WavetableFrame {
    static constexpr int tableSize = 2048;
    static constexpr int tableMask = tableSize - 1;
    std::array<float, tableSize> samples {};
    float getSample (float phase) const noexcept {
        float pos = phase * float(tableSize);
        int i0 = int(pos) & tableMask;
        int i1 = (i0 + 1) & tableMask;
        float frac = pos - float(int(pos));
        return samples[size_t(i0)] * (1 - frac) + samples[size_t(i1)] * frac;
    }
    void normalize() noexcept {
        float peak = 0;
        for (float s : samples) peak = juce::jmax(peak, std::abs(s));
        if (peak > 1e-6f) { float inv = 1/peak; for (float& s : samples) s *= inv; }
    }
};

class Wavetable {
public:
    Wavetable() { generateDefaultTables(); }
    int getNumFrames() const noexcept { return int(frames.size()); }
    WavetableFrame& getFrame(int i) { return frames[size_t(juce::jlimit(0, getNumFrames()-1, i))]; }
    float getSample(float tablePos, float phase) const noexcept {
        int n = getNumFrames(); if (n <= 0) return 0; if (n == 1) return frames[0].getSample(phase);
        float scaled = juce::jlimit(0.f,1.f,tablePos) * float(n-1);
        int f0 = int(scaled), f1 = juce::jmin(f0+1, n-1);
        float frac = scaled - float(f0);
        return frames[size_t(f0)].getSample(phase) * (1-frac) + frames[size_t(f1)].getSample(phase) * frac;
    }
    void generateDefaultTables() {
        frames.resize(4);
        for (int i = 0; i < WavetableFrame::tableSize; ++i) {
            float t = float(i) / float(WavetableFrame::tableSize);
            float phase = t * juce::MathConstants<float>::twoPi;
            size_t idx = size_t(i);
            float saw = 0; for (int h=1;h<=24;++h) saw += std::sin(float(h)*phase)/float(h);
            frames[0].samples[idx] = saw * 0.5f;
            float sq = 0; for (int h=1;h<=23;h+=2) sq += std::sin(float(h)*phase)/float(h);
            frames[1].samples[idx] = sq * 0.6f;
            frames[2].samples[idx] = std::sin(phase);
            float alien = std::sin(phase)+0.4f*std::sin(3*phase)+0.2f*std::sin(5*phase)+0.1f*std::sin(7*phase);
            frames[3].samples[idx] = alien * 0.55f;
        }
        for (auto& f : frames) f.normalize();
    }
    void processFrame(int frameIdx, float foldAmt, float driveAmt) noexcept {
        if (frameIdx < 0 || frameIdx >= getNumFrames()) return;
        auto& frame = frames[size_t(frameIdx)];
        for (float& s : frame.samples) {
            if (foldAmt > 1e-4f) {
                float x = s * (1 + foldAmt * 3), thr = 1 - foldAmt * 0.7f;
                for (int k=0;k<2;++k) { if (x>thr) x=thr-(x-thr); else if (x<-thr) x=-thr-(x+thr); else break; }
                s = x;
            }
            if (driveAmt > 1e-4f) s = std::tanh(s * (1 + driveAmt * 4));
        }
        frame.normalize();
    }
    void morphFrames(int a, int b, int dest, float t) noexcept {
        int n = getNumFrames(); if (a<0||a>=n||b<0||b>=n||dest<0||dest>=n) return;
        t = juce::jlimit(0.f,1.f,t);
        for (int i=0;i<WavetableFrame::tableSize;++i)
            getFrame(dest).samples[size_t(i)] = getFrame(a).samples[size_t(i)]*(1-t) + getFrame(b).samples[size_t(i)]*t;
        getFrame(dest).normalize();
    }
private:
    std::vector<WavetableFrame> frames;
};
} // namespace salek

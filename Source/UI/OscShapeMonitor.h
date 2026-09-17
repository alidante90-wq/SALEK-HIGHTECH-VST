#pragma once
#include <JuceHeader.h>

/** Live mini wavetable-shape scope for one oscillator */
class OscShapeMonitor : public juce::Component, private juce::Timer
{
public:
    OscShapeMonitor() { startTimerHz (28); }

    void setAPVTS (juce::AudioProcessorValueTreeState* s) { apvts = s; }
    void setOscIndex (int i) { osc = juce::jlimit (0, 2, i); }
    void setAccent (juce::Colour c) { accent = c; }
    void setTitle (const juce::String& t) { title = t; }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 6.f);
        g.setColour (accent.withAlpha (0.45f));
        g.drawRoundedRectangle (r, 6.f, 1.1f);

        g.setColour (accent);
        g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.drawText (title, r.removeFromTop (14).reduced (4, 0), juce::Justification::centredLeft);

        auto plot = r.reduced (6.f, 4.f);
        float table = 0.f, warp = 0.f, fold = 0.f;
        if (apvts != nullptr)
        {
            auto gval = [&] (const char* id, float d) {
                if (auto* p = apvts->getRawParameterValue (id)) return p->load();
                return d;
            };
            const char* tid[] = { "osc1_table", "osc2_table", "osc3_table" };
            const char* wid[] = { "osc1_warp", "osc2_warp", "osc3_warp" };
            const char* fid[] = { "osc1_fold", "osc2_fold", "osc3_fold" };
            table = gval (tid[osc], 0.f);
            warp  = gval (wid[osc], 0.f);
            fold  = gval (fid[osc], 0.f);
        }

        juce::Path wave;
        const int N = 64;
        for (int i = 0; i < N; ++i)
        {
            float t = (float) i / (float) (N - 1);
            // base: morphing harmonic series from table position
            float harm = 1.f + table * 5.f;
            float y = std::sin (t * juce::MathConstants<float>::twoPi * harm);
            // warp: phase distortion
            float tw = t + warp * 0.35f * std::sin (t * juce::MathConstants<float>::twoPi);
            y = std::sin (tw * juce::MathConstants<float>::twoPi * harm);
            // fold
            if (fold > 0.01f)
                y = std::sin (y * juce::MathConstants<float>::pi * (1.f + fold * 2.f));
            y *= 0.85f;
            float px = plot.getX() + t * plot.getWidth();
            float py = plot.getCentreY() - y * plot.getHeight() * 0.42f;
            if (i == 0) wave.startNewSubPath (px, py); else wave.lineTo (px, py);
        }
        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (wave, juce::PathStrokeType (1.5f));

        // subtle phase motion line
        float scan = std::fmod (anim * 0.15f, 1.f);
        float sx = plot.getX() + scan * plot.getWidth();
        g.setColour (accent.withAlpha (0.25f));
        g.drawVerticalLine ((int) sx, plot.getY(), plot.getBottom());

        // Extra readouts: Table / Warp / Fold
        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.setColour (accent.withAlpha (0.85f));
        auto info = juce::String::formatted ("T:%.0f  W:%.0f  F:%.0f",
            table * 100.f, warp * 100.f, fold * 100.f);
        g.drawText (info, getLocalBounds().removeFromBottom (12).reduced (4, 0),
                    juce::Justification::centredLeft);
    }

    void timerCallback() override { anim += 0.1f; repaint(); }

private:
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    int osc = 0;
    float anim = 0.f;
    juce::Colour accent { 0xff00e8ff };
    juce::String title { "OSC" };
};

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SalekLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff00f0ff));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff080810));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0c0c18));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffaa00ff));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff0a0a14));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff3a0066));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a0a28));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00f0ff));
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff080812));
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff06060e));
        setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xff00f0ff));
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)width,(float)height).reduced(3.f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.48f;
        auto cx = bounds.getCentreX(), cy = bounds.getCentreY();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
        for (int i = 3; i >= 1; --i) {
            g.setColour(fill.withAlpha(0.05f*(float)i));
            g.drawEllipse(cx-radius-i*2.f, cy-radius-i*2.f, (radius+i*2.f)*2, (radius+i*2.f)*2, 1.5f);
        }
        juce::ColourGradient body(juce::Colour(0xff1a1a28), cx, cy-radius, juce::Colour(0xff08080f), cx, cy+radius, false);
        g.setGradientFill(body);
        g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);
        juce::Path track;
        track.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1e1e30));
        g.strokePath(track, juce::PathStrokeType(3.5f));
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, angle, true);
        g.setColour(fill.withAlpha(0.3f));
        g.strokePath(arc, juce::PathStrokeType(5.f));
        g.setColour(fill);
        g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path needle;
        needle.addRoundedRectangle(-1.5f, -radius*0.7f, 3.f, radius*0.45f, 1.f);
        g.setColour(juce::Colours::white);
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(fill.brighter(0.3f));
        g.fillEllipse(cx-4, cy-4, 8, 8);
    }
};

class WavetableDisplay : public juce::Component, private juce::Timer {
public:
    explicit WavetableDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (30); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (juce::Colour (0xff04040c));
        g.fillRoundedRectangle (r, 6.0f);
        juce::ColourGradient edge (juce::Colour (0xff2a1050), r.getX(), r.getY(),
                                   juce::Colour (0xff050510), r.getX(), r.getBottom(), false);
        g.setGradientFill (edge);
        g.drawRoundedRectangle (r, 6.0f, 1.5f);
        g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.25f));
        g.drawRoundedRectangle (r.reduced (2.0f), 5.0f, 1.0f);
        auto gval = [&](const char* id, float d) {
            if (auto* p = apvts.getRawParameterValue (id)) return p->load();
            return d;
        };
        const float table = gval ("osc1_table", 0.0f);
        const float warp  = gval ("osc1_warp", 0.0f);
        const float fold  = gval ("osc1_fold", 0.0f);
        const float drive = gval ("osc1_drive", 0.0f);
        juce::Path wave;
        const int N = 128;
        const float midY = r.getCentreY();
        const float amp = r.getHeight() * 0.38f;
        for (int i = 0; i < N; ++i) {
            float phase = (float) i / (float) N;
            if (warp > 1e-4f) {
                float amount = 1.0f + warp * 3.5f;
                phase = std::pow (phase, amount);
                if (warp > 0.55f && phase > 0.5f) {
                    float foldAmt = (warp - 0.55f) * 2.2f;
                    phase = phase - foldAmt * (phase - 0.5f);
                }
                phase = juce::jlimit (0.0f, 0.9999f, phase);
            }
            float s = 0.0f;
            const float morph = table;
            for (int h = 1; h <= 12; ++h) {
                float harm = std::sin (phase * juce::MathConstants<float>::twoPi * (float) h);
                float wSine = (h == 1) ? 1.0f : 0.0f;
                float wSaw  = 1.0f / (float) h * ((h % 2 == 1) ? 1.0f : 0.7f);
                float wSqr  = (h % 2 == 1) ? 1.0f / (float) h : 0.0f;
                float a = wSine * (1.0f - morph) * (1.0f - morph)
                        + wSaw * 2.0f * morph * (1.0f - morph)
                        + wSqr * morph * morph;
                s += harm * a;
            }
            s *= 0.45f;
            if (fold > 1e-4f) {
                float thresh = 1.0f - fold * 0.85f;
                float gain = 1.0f + fold * 4.0f;
                float x = s * gain;
                for (int k = 0; k < 3; ++k) {
                    if (x > thresh) x = thresh - (x - thresh);
                    else if (x < -thresh) x = -thresh - (x + thresh);
                    else break;
                }
                s = x / (1.0f + fold * 1.5f);
            }
            if (drive > 1e-4f) {
                float dg = 1.0f + drive * 6.0f;
                s = std::tanh (s * dg);
            }
            float px = r.getX() + 4.0f + ((float) i / (float) (N - 1)) * (r.getWidth() - 8.0f);
            float py = midY - s * amp;
            if (i == 0) wave.startNewSubPath (px, py);
            else wave.lineTo (px, py);
        }
        g.setColour (juce::Colour (0xffff00aa).withAlpha (0.2f));
        g.strokePath (wave, juce::PathStrokeType (4.0f));
        g.setColour (juce::Colour (0xff00f0ff));
        g.strokePath (wave, juce::PathStrokeType (1.6f));
        g.setColour (juce::Colour (0xff8899aa));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("WAVETABLE / SHAPE", r.reduced (6).removeFromTop (14), juce::Justification::centredLeft);
    }
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};

class ScopeDisplay : public juce::Component, private juce::Timer {
public:
    ScopeDisplay() { startTimerHz(30); }
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced(1.f);
        g.setColour(juce::Colour(0xff05050c));
        g.fillRoundedRectangle(r, 4.f);
        g.setColour(juce::Colour(0xff00f0ff).withAlpha(0.35f));
        g.drawRoundedRectangle(r, 4.f, 1.f);
        juce::Path wave;
        for (int i = 0; i < 80; ++i) {
            float t = (float)i/79.f;
            float ph = t * juce::MathConstants<float>::twoPi * 3.f + phase;
            float y = std::sin(ph)*0.5f + 0.2f*std::sin(ph*2.7f);
            float px = r.getX() + t * r.getWidth();
            float py = r.getCentreY() - y * r.getHeight() * 0.38f;
            if (i == 0) wave.startNewSubPath(px, py); else wave.lineTo(px, py);
        }
        g.setColour(juce::Colour(0xffff00aa));
        g.strokePath(wave, juce::PathStrokeType(1.8f));
    }
    void timerCallback() override { phase += 0.2f; repaint(); }
private:
    float phase = 0.f;
};

class StepGridComponent : public juce::Component, private juce::Timer {
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq) { startTimerHz (20); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff05050c));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.3f));
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2;
        const int play = sequencer.getCurrentStep();
        for (int i = 0; i < n; ++i) {
            auto cell = juce::Rectangle<float> (r.getX() + gap + i * (w + gap), r.getY() + gap, w, h);
            const auto& st = sequencer.getStep (i);
            if (st.active)
                g.setColour (i == play ? juce::Colour (0xffff00aa) : juce::Colour (0xff00f0ff).withAlpha (0.75f));
            else
                g.setColour (juce::Colour (0xff1a1a28));
            g.fillRoundedRectangle (cell, 3.0f);
            if (i == play) {
                g.setColour (juce::Colours::white.withAlpha (0.9f));
                g.drawRoundedRectangle (cell, 3.0f, 1.5f);
            }
            g.setColour (juce::Colours::black.withAlpha (0.5f));
            g.setFont (juce::FontOptions (9.0f));
            g.drawText (juce::String (i + 1), cell, juce::Justification::centred);
        }
    }
    void mouseDown (const juce::MouseEvent& e) override {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        int idx = (int) ((e.position.x - r.getX() - gap) / (w + gap));
        if (idx >= 0 && idx < n) {
            auto& st = sequencer.getStep (idx);
            st.active = ! st.active;
            repaint();
        }
    }
    void timerCallback() override { repaint(); }
private:
    salek::StepSequencer& sequencer;
};

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::ListBoxModel,
                                          private juce::Timer {
public:
    explicit SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
    std::unique_ptr<WavetableDisplay> wtDisplay;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    struct Knob { juce::Slider s; juce::Label name; };
    std::vector<std::unique_ptr<Knob>> knobs;
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    juce::Component oscTab, filterTab, modTab, fxTab, seqTab, presetTab;
    juce::ComboBox filterMode, lfoWave;
    juce::ToggleButton arpOn { "ARP ON" }, seqOn { "SEQ ON" };
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, initBtn { "INIT" };
    juce::Label presetLabel, title, tagline;
    juce::ListBox presetList { "presets", this };
    std::unique_ptr<StepGridComponent> stepGrid;
    juce::MidiKeyboardComponent keyboard;
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

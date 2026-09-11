#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SalekLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffc8c8d0));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff121218));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121218));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a3a48));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0e8));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff121218));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff2a3a50));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1a24));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4fc3f7));
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff101014));
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff0d0d10));
        setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xff4fc3f7));
        setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(0xff8888a0));
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)width,(float)height).reduced(4.f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.48f;
        auto cx = bounds.getCentreX(), cy = bounds.getCentreY();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(juce::Colour(0xff1a1a22));
        g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);
        g.setColour(juce::Colour(0xff2a2a35));
        g.drawEllipse(cx-radius, cy-radius, radius*2, radius*2, 1.5f);
        juce::Path track;
        track.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff2a2a38));
        g.strokePath(track, juce::PathStrokeType(3.0f));
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, angle, true);
        g.setColour(fill);
        g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path needle;
        needle.addRoundedRectangle(-1.2f, -radius*0.65f, 2.4f, radius*0.4f, 1.f);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(fill.brighter(0.2f));
        g.fillEllipse(cx-3.5f, cy-3.5f, 7.f, 7.f);
    }
};

class WavetableDisplay : public juce::Component, private juce::Timer {
public:
    explicit WavetableDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (24); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (juce::Colour (0xff101014));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0xff2a2a38));
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
        auto gval = [&](const char* id, float d) {
            if (auto* p = apvts.getRawParameterValue (id)) return p->load();
            return d;
        };
        const float table = gval ("osc1_table", 0.0f);
        const float warp  = gval ("osc1_warp", 0.0f);
        const float fold  = gval ("osc1_fold", 0.0f);
        const float drive = gval ("osc1_drive", 0.0f);
        juce::Path wave;
        const int N = 96;
        const float midY = r.getCentreY();
        const float amp = r.getHeight() * 0.36f;
        for (int i = 0; i < N; ++i) {
            float phase = (float) i / (float) N;
            if (warp > 1e-4f) {
                phase = std::pow (phase, 1.0f + warp * 3.5f);
                phase = juce::jlimit (0.0f, 0.9999f, phase);
            }
            float s = 0.0f;
            for (int h = 1; h <= 10; ++h) {
                float harm = std::sin (phase * juce::MathConstants<float>::twoPi * (float) h);
                float wSine = (h == 1) ? 1.0f : 0.0f;
                float wSaw  = 1.0f / (float) h;
                float wSqr  = (h % 2 == 1) ? 1.0f / (float) h : 0.0f;
                float a = wSine * (1.0f - table) * (1.0f - table) + wSaw * 2.0f * table * (1.0f - table) + wSqr * table * table;
                s += harm * a;
            }
            s *= 0.4f;
            if (fold > 1e-4f) {
                float thresh = 1.0f - fold * 0.85f;
                float x = s * (1.0f + fold * 4.0f);
                for (int k = 0; k < 2; ++k) {
                    if (x > thresh) x = thresh - (x - thresh);
                    else if (x < -thresh) x = -thresh - (x + thresh);
                    else break;
                }
                s = x / (1.0f + fold * 1.5f);
            }
            if (drive > 1e-4f) s = std::tanh (s * (1.0f + drive * 5.0f));
            float px = r.getX() + 4.0f + ((float) i / (float) (N - 1)) * (r.getWidth() - 8.0f);
            float py = midY - s * amp;
            if (i == 0) wave.startNewSubPath (px, py); else wave.lineTo (px, py);
        }
        g.setColour (juce::Colour (0xff4fc3f7));
        g.strokePath (wave, juce::PathStrokeType (1.5f));
    }
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};

class ScopeDisplay : public juce::Component, private juce::Timer {
public:
    ScopeDisplay() { startTimerHz(24); }
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced(1.f);
        g.setColour(juce::Colour(0xff101014));
        g.fillRoundedRectangle(r, 6.f);
        g.setColour(juce::Colour(0xff2a2a38));
        g.drawRoundedRectangle(r, 6.f, 1.f);
        juce::Path wave;
        for (int i = 0; i < 64; ++i) {
            float t = (float)i/63.f;
            float ph = t * juce::MathConstants<float>::twoPi * 2.5f + phase;
            float y = std::sin(ph)*0.5f + 0.15f*std::sin(ph*3.f);
            float px = r.getX() + t * r.getWidth();
            float py = r.getCentreY() - y * r.getHeight() * 0.35f;
            if (i == 0) wave.startNewSubPath(px, py); else wave.lineTo(px, py);
        }
        g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.85f));
        g.strokePath(wave, juce::PathStrokeType(1.4f));
    }
    void timerCallback() override { phase += 0.15f; repaint(); }
private:
    float phase = 0.f;
};

class StepGridComponent : public juce::Component, private juce::Timer {
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq) { startTimerHz (16); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff101014));
        g.fillRoundedRectangle (r, 6.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 3.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2;
        const int play = sequencer.getCurrentStep();
        for (int i = 0; i < n; ++i) {
            auto cell = juce::Rectangle<float> (r.getX() + gap + i * (w + gap), r.getY() + gap, w, h);
            const auto& st = sequencer.getStep (i);
            g.setColour (st.active ? (i == play ? juce::Colour (0xff4fc3f7) : juce::Colour (0xff3a7a9a)) : juce::Colour (0xff1a1a22));
            g.fillRoundedRectangle (cell, 3.0f);
            g.setColour (juce::Colours::white.withAlpha (0.4f));
            g.setFont (juce::FontOptions (9.0f));
            g.drawText (juce::String (i + 1), cell, juce::Justification::centred);
        }
    }
    void mouseDown (const juce::MouseEvent& e) override {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 3.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        int idx = (int) ((e.position.x - r.getX() - gap) / (w + gap));
        if (idx >= 0 && idx < n) { sequencer.getStep (idx).active = ! sequencer.getStep (idx).active; repaint(); }
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
    juce::Component oscTab, filterTab, envTab, modTab, fxTab, seqTab, presetTab;
    juce::ComboBox filterMode, lfoWave;
    juce::ToggleButton arpOn { "ARP ON" }, seqOn { "SEQ ON" };
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, initBtn { "INIT" };
    juce::Label presetLabel, title, tagline;
    juce::ListBox presetList { "presets", this };
    std::unique_ptr<StepGridComponent> stepGrid;
    juce::MidiKeyboardComponent keyboard;
    float phaseLights = 0.0f;
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

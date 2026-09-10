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

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::ListBoxModel {
public:
    explicit SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
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
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

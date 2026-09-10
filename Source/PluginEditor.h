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
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)width,(float)height).reduced(4.f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.48f;
        auto cx = bounds.getCentreX(), cy = bounds.getCentreY();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
        for (int i = 3; i >= 1; --i) {
            g.setColour(fill.withAlpha(0.06f * (float)i));
            g.drawEllipse(cx-radius-i*2.5f, cy-radius-i*2.5f, (radius+i*2.5f)*2, (radius+i*2.5f)*2, 2.f);
        }
        juce::ColourGradient body(juce::Colour(0xff1a1a28), cx, cy-radius, juce::Colour(0xff08080f), cx, cy+radius, false);
        g.setGradientFill(body);
        g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);
        juce::Path track;
        track.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1e1e30));
        g.strokePath(track, juce::PathStrokeType(4.f));
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, angle, true);
        g.setColour(fill.withAlpha(0.35f));
        g.strokePath(arc, juce::PathStrokeType(6.f));
        g.setColour(fill);
        g.strokePath(arc, juce::PathStrokeType(3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path needle;
        needle.addRoundedRectangle(-1.8f, -radius*0.72f, 3.6f, radius*0.48f, 1.f);
        g.setColour(juce::Colours::white);
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(fill.brighter(0.4f));
        g.fillEllipse(cx-5, cy-5, 10, 10);
        g.setColour(juce::Colour(0xff0a0a12));
        g.fillEllipse(cx-2.5f, cy-2.5f, 5, 5);
    }
};

class ScopeDisplay : public juce::Component, private juce::Timer {
public:
    ScopeDisplay() { startTimerHz(30); }
    void paint(juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xff05050c));
        g.fillRoundedRectangle(r, 6.f);
        g.setColour(juce::Colour(0xff00f0ff).withAlpha(0.4f));
        g.drawRoundedRectangle(r, 6.f, 1.f);
        juce::Path wave;
        for (int i = 0; i < 64; ++i) {
            float t = (float)i / 63.f;
            float phase = t * juce::MathConstants<float>::twoPi * 3.f + phaseOffset;
            float y = std::sin(phase)*0.55f + 0.15f*std::sin(phase*3.f+1.f);
            float px = r.getX() + t * r.getWidth();
            float py = r.getCentreY() - y * r.getHeight() * 0.4f;
            if (i == 0) wave.startNewSubPath(px, py); else wave.lineTo(px, py);
        }
        g.setColour(juce::Colour(0xffff00aa).withAlpha(0.9f));
        g.strokePath(wave, juce::PathStrokeType(2.f));
    }
    void timerCallback() override {
        phaseOffset += 0.18f;
        if (phaseOffset > juce::MathConstants<float>::twoPi) phaseOffset -= juce::MathConstants<float>::twoPi;
        repaint();
    }
private:
    float phaseOffset = 0.f;
};

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
    struct Knob { juce::Slider s; juce::Label name; };
    Knob osc1L, osc2L, osc3L, osc1T, osc2T, osc3T;
    Knob osc1W, osc2W, osc3W, osc1F, osc2F, osc3F;
    Knob osc1D, osc2D, osc3D;
    Knob cut, reso, fDrv, fEnv, lfoR, lfoA;
    Knob fm21, fm31, rm21, macro, delay, master;
    juce::ComboBox filterMode, lfoWave;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterAtt, lfoAtt;
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;
    juce::Label title, tagline, brand;
    void setup(Knob& k, const char* id, const char* label, juce::Colour c);
    void paintPanel(juce::Graphics& g, juce::Rectangle<float> r, const juce::String& header, juce::Colour accent);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

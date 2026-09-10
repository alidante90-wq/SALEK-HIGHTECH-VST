#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SalekLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SalekLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colours::cyan);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff0a0a14));
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff12121e));
        setColour (juce::ComboBox::outlineColourId, juce::Colours::magenta.darker (0.4f));
        setColour (juce::ComboBox::textColourId, juce::Colours::white);
        setColour (juce::Label::textColourId, juce::Colours::white);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0e0e18));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::magenta.darker (0.5f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId).withAlpha (0.25f));
        g.drawEllipse (centre.x - radius - 2, centre.y - radius - 2, (radius + 2) * 2, (radius + 2) * 2, 3.0f);

        g.setColour (juce::Colour (0xff101018));
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2, radius * 2);

        juce::Path track;
        track.addCentredArc (centre.x, centre.y, radius * 0.85f, radius * 0.85f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff2a2a3a));
        g.strokePath (track, juce::PathStrokeType (3.5f));

        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, radius * 0.85f, radius * 0.85f, 0.0f, rotaryStartAngle, angle, true);
        auto fill = slider.findColour (juce::Slider::rotarySliderFillColourId);
        g.setColour (fill);
        g.strokePath (valueArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path pointer;
        auto pr = radius * 0.7f;
        pointer.addRectangle (-1.5f, -pr, 3.0f, pr * 0.55f);
        g.setColour (juce::Colours::white);
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));

        g.setColour (fill.brighter (0.3f));
        g.fillEllipse (centre.x - 4, centre.y - 4, 8, 8);
    }
};

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel salekLnF;

    struct Knob { juce::Slider slider; juce::Label label; };

    Knob kOsc1Lvl, kOsc2Lvl, kOsc3Lvl;
    Knob kOsc1Tab, kOsc2Tab, kOsc3Tab;
    Knob kOsc1Warp, kOsc2Warp, kOsc3Warp;
    Knob kOsc1Fold, kOsc2Fold, kOsc3Fold;
    Knob kOsc1Drv, kOsc2Drv, kOsc3Drv;
    Knob kCutoff, kReso, kFDrive, kFEnv;
    Knob kLfoRate, kLfoAmt;
    Knob kFm21, kFm31, kRm21;
    Knob kMacro, kDelay, kMDrive;

    juce::ComboBox filterModeBox, lfoWaveBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterModeAtt, lfoWaveAtt;

    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;

    juce::Label title, subtitle;
    juce::Label hOsc, hFilter, hMod, hFx;

    void setupKnob (Knob& k, const juce::String& paramId, const juce::String& name, juce::Colour c);
    void drawPanel (juce::Graphics& g, juce::Rectangle<int> r);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalekHightechAudioProcessorEditor)
};


    auto k = std::make_unique<Knob>();
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    k->s.setColour (juce::Slider::thumbColourId, c.brighter());
    k->name.setText (label, juce::dontSendNotification);
    k->name.setJustificationType (juce::Justification::centred);
    k->name.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.85f));
    parent.addAndMakeVisible (k->s);
    parent.addAndMakeVisible (k->name);
    sliderAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.getAPVTS(), id, k->s));
    auto* raw = k.get();
    knobs.add (std::move (k));
    return *raw;

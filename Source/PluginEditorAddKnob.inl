
    auto k = std::make_unique<Knob>();
    k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 14);
    k->s.setNumDecimalPlacesToDisplay (2);   // short numbers (fixes "هزار رقمی")
    k->s.setColour (juce::Slider::rotarySliderFillColourId, c);
    // Cleaner value text: max 2 decimals, strip trailing zeros
    k->s.textFromValueFunction = [] (double v)
    {
        if (std::abs (v - std::round (v)) < 1e-4)
            return juce::String ((int) std::round (v));
        return juce::String (v, 2);
    };
    parent.addAndMakeVisible (k->s);
    atts.push_back (std::make_unique<SAtt> (processor.getAPVTS(), id, k->s));
    k->name.setText (label, juce::dontSendNotification);
    k->name.setJustificationType (juce::Justification::centred);
    k->name.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    k->name.setColour (juce::Label::textColourId, juce::Colour (0xffc0a0d0));
    parent.addAndMakeVisible (k->name);
    knobs.push_back (std::move (k));
    return *knobs.back();

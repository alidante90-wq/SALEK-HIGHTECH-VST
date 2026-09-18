class SalekLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SalekLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e8ff));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a1840));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff12081c));
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff1a1030));
        setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colour(0xff2a1840));
        setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colour(0xff00e8ff));
        setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xffffffff));
        setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(0xffa090c0));
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::ListBox::outlineColourId, juce::Colour(0xff1a1030));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffff00aa));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1030));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00e8ff));
        setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffff2d9b));
        setColour(juce::Label::textColourId, juce::Colour(0xffc0c0e0));
        // Popup / preset menu (Pigments-like dark + neon)
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff0c0616));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe8e0ff));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff2a0848));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffffff));
        setColour(juce::PopupMenu::headerTextColourId, juce::Colour(0xff00e8ff));
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff00e8ff).withAlpha(0.55f));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(3.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.40f;
        auto cx = bounds.getCentreX();
        auto cy = bounds.getCentreY() - 1.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
        const float v = juce::jlimit (0.f, 1.f, sliderPos);

        // Soft body (no expanding shadow)
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (cx - radius + 1.5f, cy - radius + 2.5f, radius * 2.f, radius * 2.f);
        juce::ColourGradient body (
            juce::Colour (0xff2a1a40), cx, cy - radius,
            juce::Colour (0xff0c0618), cx, cy + radius, false);
        g.setGradientFill (body);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.f, radius * 2.f);

        // Thin rim
        g.setColour (fill.withAlpha (0.25f + v * 0.35f));
        g.drawEllipse (cx - radius, cy - radius, radius * 2.f, radius * 2.f, 1.4f);

        // Value arc — clean neon, thickness grows slightly with value
        juce::Path arc;
        arc.addCentredArc (cx, cy, radius * 0.78f, radius * 0.78f, 0.f, rotaryStartAngle, angle, true);
        g.setColour (fill.withAlpha (0.2f + v * 0.25f));
        g.strokePath (arc, juce::PathStrokeType (3.2f + v * 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (fill.brighter (0.15f * v));
        g.strokePath (arc, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // LED ticks (only along arc, no blob glow)
        const int numLeds = 11;
        for (int i = 0; i < numLeds; ++i)
        {
            float t = (float) i / (float) (numLeds - 1);
            float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            bool on = t <= sliderPos + 0.02f;
            float lx = cx + std::cos (a - juce::MathConstants<float>::halfPi) * (radius + 4.5f);
            float ly = cy + std::sin (a - juce::MathConstants<float>::halfPi) * (radius + 4.5f);
            g.setColour (on ? fill.withAlpha (0.5f + v * 0.5f) : juce::Colour (0xff181028));
            g.fillEllipse (lx - 1.6f, ly - 1.6f, 3.2f, 3.2f);
        }

        // Pointer
        juce::Path needle;
        needle.addRectangle (-1.2f, -radius * 0.72f, 2.4f, radius * 0.42f);
        g.setColour (fill.brighter (0.3f));
        g.fillPath (needle, juce::AffineTransform::rotation (angle).translated (cx, cy));
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.fillEllipse (cx - 3.f, cy - 3.f, 6.f, 6.f);
        g.setColour (fill.withAlpha (0.9f));
        g.fillEllipse (cx - 1.8f, cy - 1.8f, 3.6f, 3.6f);

        // Bottom value bar
        auto bar = bounds.removeFromBottom (4.0f).reduced (6.0f, 0.0f);
        g.setColour (juce::Colour (0xff1a1028));
        g.fillRoundedRectangle (bar, 1.5f);
        g.setColour (fill.withAlpha (0.85f));
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * v), 1.5f);
    }
};

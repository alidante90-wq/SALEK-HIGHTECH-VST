class SalekLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SalekLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00e8ff));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a1840));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff12081c));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffff00aa));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a1030));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00e8ff));
        setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffff2d9b));
        setColour(juce::Label::textColourId, juce::Colour(0xffc0c0e0));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.42f;
        auto cx = bounds.getCentreX();
        auto cy = bounds.getCentreY() - 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);

        g.setColour(fill.withAlpha(0.22f));
        g.fillEllipse(cx - radius - 4.f, cy - radius - 4.f, (radius + 4.f) * 2.f, (radius + 4.f) * 2.f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(cx - radius + 2.f, cy - radius + 3.f, radius * 2.f, radius * 2.f);

        juce::ColourGradient body(
            juce::Colour(0xff2e2048), cx, cy - radius,
            juce::Colour(0xff0a0614), cx, cy + radius, false);
        g.setGradientFill(body);
        g.fillEllipse(cx - radius, cy - radius, radius * 2.f, radius * 2.f);

        const int numLeds = 12;
        for (int i = 0; i < numLeds; ++i)
        {
            float t = (float) i / (float) (numLeds - 1);
            float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            float on = (t <= sliderPos + 0.02f) ? 1.0f : 0.0f;
            float lx = cx + std::cos(a - juce::MathConstants<float>::halfPi) * (radius + 5.5f);
            float ly = cy + std::sin(a - juce::MathConstants<float>::halfPi) * (radius + 5.5f);
            g.setColour(on > 0.5f ? fill.withAlpha(0.95f) : juce::Colour(0xff1a1028));
            g.fillEllipse(lx - 2.0f, ly - 2.0f, 4.0f, 4.0f);
            if (on > 0.5f)
            {
                g.setColour(fill.withAlpha(0.35f));
                g.fillEllipse(lx - 3.5f, ly - 3.5f, 7.0f, 7.0f);
            }
        }

        g.setColour(fill.withAlpha(0.7f));
        g.drawEllipse(cx - radius, cy - radius, radius * 2.f, radius * 2.f, 1.6f);

        juce::Path arc;
        arc.addCentredArc(cx, cy, radius * 0.72f, radius * 0.72f, 0.f, rotaryStartAngle, angle, true);
        g.setColour(fill.withAlpha(0.3f));
        g.strokePath(arc, juce::PathStrokeType(5.5f));
        g.setColour(fill);
        g.strokePath(arc, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path pointer;
        auto pointerLength = radius * 0.72f;
        pointer.addRectangle(-1.5f, -pointerLength, 3.0f, pointerLength * 0.85f);
        g.setColour(juce::Colours::white.withAlpha(0.95f));
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));

        g.setColour(fill.withAlpha(0.9f));
        g.fillEllipse(cx - 3.5f, cy - 3.5f, 7.f, 7.f);
        g.setColour(juce::Colour(0xff0a0614));
        g.fillEllipse(cx - 2.0f, cy - 2.0f, 4.f, 4.f);

        float barY = cy + radius + 6.0f;
        float barW = radius * 1.85f;
        float barH = 5.5f;
        float barX = cx - barW * 0.5f;
        g.setColour(juce::Colour(0xff0a0614));
        g.fillRoundedRectangle(barX - 1.0f, barY - 1.0f, barW + 2.0f, barH + 2.0f, 2.5f);
        const int segs = 8;
        float gap = 1.2f;
        float segW = (barW - gap * (segs - 1)) / (float) segs;
        for (int i = 0; i < segs; ++i)
        {
            float t0 = (float) i / (float) segs;
            bool on = sliderPos > t0 + 0.02f;
            float sx = barX + i * (segW + gap);
            g.setColour(on ? fill.withAlpha(0.95f) : juce::Colour(0xff1a1028));
            g.fillRoundedRectangle(sx, barY, segW, barH, 1.2f);
            if (on)
            {
                g.setColour(fill.withAlpha(0.35f));
                g.fillRoundedRectangle(sx - 0.5f, barY - 0.5f, segW + 1.0f, barH + 1.0f, 1.5f);
            }
        }
    }
};

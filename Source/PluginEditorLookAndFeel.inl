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
        setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(0xffc8b8e8));
        setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xffffffff));
        setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colour(0xff00e8ff));
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
        // Unified type scale (readable, one family — no extra font loads)
        setDefaultSansSerifTypefaceName (juce::Font::getDefaultSansSerifFontName());
    }

    static juce::Font salekFont (float h, bool bold = true)
    {
        return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return salekFont (11.5f, true);
    }

    juce::Font getTextButtonFont (juce::TextButton& b, int buttonHeight) override
    {
        juce::ignoreUnused (b);
        return salekFont (juce::jlimit (10.0f, 13.0f, (float) buttonHeight * 0.42f), true);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return salekFont (12.0f, true);
    }

    juce::Font getPopupMenuFont() override
    {
        return salekFont (13.0f, false);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                          bool, bool) override
    {
        const auto t = button.getButtonText().trim().toUpperCase();
        const auto area = button.getLocalBounds().toFloat().reduced (4.f);
        const auto accent = button.findColour (juce::TextButton::textColourOffId);
        g.setColour (accent.withAlpha (0.95f));

        // Compact icon language for navigation / utility controls.
        // Text remains in the component for accessibility/tooltips, but the
        // visible control is an icon so the top bar and preset browser stay clean.
        juce::Path p;
        const auto c = area.getCentre();
        const float s = juce::jmin (area.getWidth(), area.getHeight()) * 0.28f;

        if (t == "<" || t == "PREV")
        {
            p.startNewSubPath (c.x + s, c.y - s * 1.25f);
            p.lineTo (c.x - s, c.y);
            p.lineTo (c.x + s, c.y + s * 1.25f);
            g.strokePath (p, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        else if (t == ">" || t == "NEXT")
        {
            p.startNewSubPath (c.x - s, c.y - s * 1.25f);
            p.lineTo (c.x + s, c.y);
            p.lineTo (c.x - s, c.y + s * 1.25f);
            g.strokePath (p, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        else if (t == "INIT")
        {
            g.drawEllipse (c.x - s, c.y - s, 2.f * s, 2.f * s, 1.7f);
            g.drawLine (c.x, c.y - s * 0.55f, c.x, c.y + s * 0.55f, 1.8f);
            g.drawLine (c.x - s * 0.55f, c.y, c.x + s * 0.55f, c.y, 1.8f);
        }
        else if (t == "SAVE")
        {
            auto rr = juce::Rectangle<float> (c.x - s, c.y - s, 2.f*s, 2.f*s);
            g.drawRoundedRectangle (rr, 2.f, 1.7f);
            g.fillRect (c.x - s * 0.55f, c.y - s * 0.78f, s * 1.1f, s * 0.42f);
            g.drawLine (c.x - s * 0.55f, c.y + s * 0.15f, c.x + s * 0.55f, c.y + s * 0.15f, 1.5f);
        }
        else if (t == "LOAD")
        {
            g.drawRoundedRectangle (c.x - s, c.y - s * 0.7f, 2.f*s, s * 1.5f, 2.f, 1.7f);
            p.startNewSubPath (c.x, c.y + s * 0.65f);
            p.lineTo (c.x, c.y - s * 0.15f);
            p.startNewSubPath (c.x - s * 0.4f, c.y + s * 0.15f);
            p.lineTo (c.x, c.y - s * 0.25f);
            p.lineTo (c.x + s * 0.4f, c.y + s * 0.15f);
            g.strokePath (p, juce::PathStrokeType (1.8f));
        }
        else if (t == "BANK")
        {
            g.drawRoundedRectangle (c.x - s * 1.15f, c.y - s * 0.75f, s * 2.3f, s * 1.5f, 2.f, 1.5f);
            g.drawLine (c.x - s * 0.65f, c.y - s * 0.25f, c.x + s * 0.65f, c.y - s * 0.25f, 1.4f);
            g.drawLine (c.x - s * 0.65f, c.y + s * 0.25f, c.x + s * 0.65f, c.y + s * 0.25f, 1.4f);
        }
        else if (t == "INSPIRE")
        {
            for (int i = 0; i < 8; ++i)
            {
                const float a = juce::MathConstants<float>::twoPi * (float) i / 8.f;
                g.drawLine (c.x + std::cos(a)*s*0.35f, c.y + std::sin(a)*s*0.35f,
                            c.x + std::cos(a)*s*1.15f, c.y + std::sin(a)*s*1.15f, 1.5f);
            }
            g.fillEllipse (c.x - s*0.35f, c.y - s*0.35f, s*0.7f, s*0.7f);
        }
        else if (t == "BG")
        {
            g.drawRoundedRectangle (c.x-s*1.1f,c.y-s*0.75f,s*2.2f,s*1.5f,2.f,1.5f);
            g.fillEllipse (c.x-s*0.55f,c.y-s*0.35f,s*0.38f,s*0.38f);
            p.startNewSubPath(c.x-s*0.9f,c.y+s*0.5f);
            p.lineTo(c.x-s*0.1f,c.y-s*0.05f);
            p.lineTo(c.x+s*0.25f,c.y+s*0.3f);
            p.lineTo(c.x+s*0.95f,c.y-s*0.45f);
            g.strokePath(p,juce::PathStrokeType(1.5f));
        }
        else if (t == "CHAR" || t == "MODEL")
        {
            g.drawEllipse (c.x-s*0.45f,c.y-s*0.9f,s*0.9f,s*0.9f,1.5f);
            g.drawRoundedRectangle(c.x-s*0.75f,c.y,s*1.5f,s*0.9f,3.f,1.5f);
        }
        else if (t == "BYP")
        {
            g.drawEllipse (c.x-s,c.y-s,2.f*s,2.f*s,1.5f);
            g.drawLine(c.x,c.y-s*0.75f,c.x,c.y+s*0.1f,2.f);
        }
        else
        {
            // Keep compact labels such as LFO1 / ARP / SEQ readable.
            g.setFont (salekFont (juce::jlimit (10.f, 12.f, area.getHeight() * 0.45f), true));
            g.drawText (button.getButtonText(), area.toNearestInt(), juce::Justification::centred);
        }
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

        // Deep 3D body + drop shadow
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillEllipse (cx - radius + 2.f, cy - radius + 3.5f, radius * 2.f, radius * 2.f);
        juce::ColourGradient body (
            juce::Colour (0xff3a2858), cx - radius * 0.3f, cy - radius,
            juce::Colour (0xff080410), cx + radius * 0.2f, cy + radius, false);
        g.setGradientFill (body);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.f, radius * 2.f);
        // Specular highlight (3D glass)
        juce::ColourGradient spec (
            juce::Colours::white.withAlpha (0.22f), cx - radius * 0.35f, cy - radius * 0.55f,
            juce::Colours::transparentWhite, cx, cy + radius * 0.1f, true);
        g.setGradientFill (spec);
        g.fillEllipse (cx - radius * 0.85f, cy - radius * 0.85f, radius * 1.3f, radius * 0.9f);
        // Neon rim
        g.setColour (fill.withAlpha (0.3f + v * 0.45f));
        g.drawEllipse (cx - radius, cy - radius, radius * 2.f, radius * 2.f, 1.6f);
        g.setColour (fill.brighter (0.4f).withAlpha (0.15f + v * 0.25f));
        g.drawEllipse (cx - radius - 1.5f, cy - radius - 1.5f, radius * 2.f + 3.f, radius * 2.f + 3.f, 1.f);

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

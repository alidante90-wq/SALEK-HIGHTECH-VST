#pragma once
#include <JuceHeader.h>

/** Hexagonal Kaossilator-style XY pad for MAGIC tab */
class MagicPad : public juce::Component, private juce::Timer
{
public:
    std::function<void(float,float,bool)> onChange; // x,y,active

    MagicPad() { startTimerHz (30); }

    void setPosition (float nx, float ny, bool act)
    {
        x = juce::jlimit (0.f, 1.f, nx);
        y = juce::jlimit (0.f, 1.f, ny);
        touching = act;
        repaint();
    }
    void setModeColour (juce::Colour c) { accent = c; repaint(); }
    void setModeName (const juce::String& n) { modeName = n; repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (6.f);
        g.setColour (juce::Colour (0xff080414));
        g.fillRoundedRectangle (r, 12.f);

        auto hex = makeHex (r.reduced (18.f));
        // glow fill
        g.setColour (accent.withAlpha (0.08f + (touching ? 0.12f : 0.f)));
        g.fillPath (hex);
        g.setColour (accent.withAlpha (0.55f));
        g.strokePath (hex, juce::PathStrokeType (2.2f));

        // inner grid rays
        auto c = hex.getBounds().getCentre();
        g.setColour (accent.withAlpha (0.12f));
        for (int i = 0; i < 6; ++i)
        {
            float a = (float) i / 6.f * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float rad = juce::jmin (hex.getBounds().getWidth(), hex.getBounds().getHeight()) * 0.48f;
            g.drawLine (c.x, c.y, c.x + std::cos (a) * rad, c.y + std::sin (a) * rad, 1.f);
        }

        // trail
        for (int i = 0; i < (int) trail.size(); ++i)
        {
            float a = (float) (i + 1) / (float) trail.size();
            g.setColour (accent.withAlpha (0.15f * a));
            auto p = toScreen (trail[(size_t) i], hex.getBounds());
            g.fillEllipse (p.x - 3.f, p.y - 3.f, 6.f, 6.f);
        }

        // finger point
        auto pt = toScreen ({ x, y }, hex.getBounds());
        g.setColour (accent.withAlpha (0.35f));
        g.fillEllipse (pt.x - 18.f, pt.y - 18.f, 36.f, 36.f);
        g.setColour (juce::Colours::white);
        g.fillEllipse (pt.x - 7.f, pt.y - 7.f, 14.f, 14.f);
        g.setColour (accent);
        g.drawEllipse (pt.x - 10.f, pt.y - 10.f, 20.f, 20.f, 2.f);

        // labels
        g.setColour (accent);
        g.setFont (juce::FontOptions (14.f, juce::Font::bold));
        g.drawText ("MAGIC  ·  " + modeName, r.removeFromTop (22).reduced (8, 0),
                    juce::Justification::centredLeft);
        g.setColour (juce::Colours::white.withAlpha (0.45f));
        g.setFont (juce::FontOptions (10.f));
        g.drawText ("drag inside hex  ·  release = off",
                    getLocalBounds().removeFromBottom (18).reduced (10, 0),
                    juce::Justification::centred);

        // XY readout
        g.setColour (accent.withAlpha (0.8f));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.drawText ("X " + juce::String (x, 2) + "   Y " + juce::String (y, 2),
                    getLocalBounds().removeFromBottom (36).removeFromTop (16).reduced (12, 0),
                    juce::Justification::centredRight);
    }

    void mouseDown (const juce::MouseEvent& e) override { updateFromPos (e.position, true); }
    void mouseDrag (const juce::MouseEvent& e) override { updateFromPos (e.position, true); }
    void mouseUp (const juce::MouseEvent&) override
    {
        touching = false;
        if (onChange) onChange (x, y, false);
        repaint();
    }

    void timerCallback() override
    {
        anim += 0.05f;
        if (touching)
        {
            trail.push_back ({ x, y });
            if (trail.size() > 24) trail.erase (trail.begin());
        }
        else if (! trail.empty())
            trail.erase (trail.begin());
        repaint();
    }

private:
    float x = 0.5f, y = 0.5f, anim = 0.f;
    bool touching = false;
    juce::Colour accent { 0xffff00dd };
    juce::String modeName { "LOOP" };
    std::vector<juce::Point<float>> trail;

    juce::Path makeHex (juce::Rectangle<float> r) const
    {
        juce::Path p;
        auto c = r.getCentre();
        float rad = juce::jmin (r.getWidth(), r.getHeight()) * 0.48f;
        for (int i = 0; i < 6; ++i)
        {
            float a = (float) i / 6.f * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float px = c.x + std::cos (a) * rad;
            float py = c.y + std::sin (a) * rad;
            if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
        }
        p.closeSubPath();
        return p;
    }

    juce::Point<float> toScreen (juce::Point<float> norm, juce::Rectangle<float> bounds) const
    {
        return { bounds.getX() + norm.x * bounds.getWidth(),
                 bounds.getY() + (1.f - norm.y) * bounds.getHeight() };
    }

    void updateFromPos (juce::Point<float> pos, bool act)
    {
        auto r = getLocalBounds().toFloat().reduced (24.f);
        auto hex = makeHex (r);
        // clamp to bounding box of hex for simplicity
        auto b = hex.getBounds().reduced (4.f);
        float nx = juce::jlimit (0.f, 1.f, (pos.x - b.getX()) / b.getWidth());
        float ny = juce::jlimit (0.f, 1.f, 1.f - (pos.y - b.getY()) / b.getHeight());
        x = nx; y = ny; touching = act;
        if (onChange) onChange (x, y, act);
        repaint();
    }
};

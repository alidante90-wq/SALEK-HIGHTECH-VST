#pragma once
#include <JuceHeader.h>

/** Hexagonal Kaossilator-style XY pad for MAGIC tab (v2 — smoother visual feedback) */
class MagicPad : public juce::Component, private juce::Timer
{
public:
    std::function<void(float,float,bool)> onChange; // x,y,active

    MagicPad() { startTimerHz (45); }

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
        auto r = getLocalBounds().toFloat().reduced (4.f);
        g.setColour (juce::Colour (0xff06030e));
        g.fillRoundedRectangle (r, 14.f);
        g.setColour (accent.withAlpha (0.35f));
        g.drawRoundedRectangle (r, 14.f, 1.5f);

        auto hex = makeHex (r.reduced (20.f));

        // outer glow pulse when active
        if (touching)
        {
            g.setColour (accent.withAlpha (0.06f + 0.04f * std::sin (anim * 3.f)));
            g.fillPath (hex);
        }
        else
        {
            g.setColour (accent.withAlpha (0.05f));
            g.fillPath (hex);
        }

        g.setColour (accent.withAlpha (0.65f));
        g.strokePath (hex, juce::PathStrokeType (2.4f));

        // concentric guide rings
        auto c = hex.getBounds().getCentre();
        float rad = juce::jmin (hex.getBounds().getWidth(), hex.getBounds().getHeight()) * 0.48f;
        for (int ring = 1; ring <= 3; ++ring)
        {
            float rr = rad * (float) ring / 3.f;
            g.setColour (accent.withAlpha (0.08f));
            g.drawEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f, 1.f);
        }

        // radial spokes
        g.setColour (accent.withAlpha (0.14f));
        for (int i = 0; i < 6; ++i)
        {
            float a = (float) i / 6.f * juce::MathConstants<float>::twoPi
                      - juce::MathConstants<float>::halfPi;
            g.drawLine (c.x, c.y, c.x + std::cos (a) * rad, c.y + std::sin (a) * rad, 1.f);
        }

        // motion trail
        for (size_t i = 0; i < trail.size(); ++i)
        {
            float a = (float) (i + 1) / (float) juce::jmax ((size_t) 1, trail.size());
            g.setColour (accent.withAlpha (0.12f * a));
            auto p = toScreen (trail[i], hex.getBounds());
            float s = 3.f + 4.f * a;
            g.fillEllipse (p.x - s, p.y - s, s * 2.f, s * 2.f);
        }

        // cursor
        auto pt = toScreen ({ x, y }, hex.getBounds());
        if (touching)
        {
            g.setColour (accent.withAlpha (0.28f));
            g.fillEllipse (pt.x - 22.f, pt.y - 22.f, 44.f, 44.f);
        }
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.fillEllipse (pt.x - 7.f, pt.y - 7.f, 14.f, 14.f);
        g.setColour (accent);
        g.drawEllipse (pt.x - 11.f, pt.y - 11.f, 22.f, 22.f, 2.2f);

        // header
        g.setColour (accent);
        g.setFont (juce::FontOptions (15.f, juce::Font::bold));
        g.drawText ("MAGIC  ·  " + modeName, r.removeFromTop (24).reduced (10, 0),
                    juce::Justification::centredLeft, false);

        // XY readout
        g.setColour (accent.withAlpha (0.85f));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.drawText ("X " + juce::String (x, 2) + "   Y " + juce::String (y, 2),
                    getLocalBounds().removeFromBottom (40).removeFromTop (16).reduced (14, 0),
                    juce::Justification::centredRight);

        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.setFont (juce::FontOptions (10.f));
        g.drawText (touching ? "ACTIVE  ·  drag to morph" : "touch hex  ·  release = bypass",
                    getLocalBounds().removeFromBottom (18).reduced (10, 0),
                    juce::Justification::centred);
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
        anim += 0.07f;
        if (touching)
        {
            trail.push_back ({ x, y });
            if (trail.size() > 28) trail.erase (trail.begin());
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
            float a = (float) i / 6.f * juce::MathConstants<float>::twoPi
                      - juce::MathConstants<float>::halfPi;
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
        auto b = hex.getBounds().reduced (4.f);
        float nx = juce::jlimit (0.f, 1.f, (pos.x - b.getX()) / juce::jmax (1.f, b.getWidth()));
        float ny = juce::jlimit (0.f, 1.f, 1.f - (pos.y - b.getY()) / juce::jmax (1.f, b.getHeight()));
        x = nx; y = ny; touching = act;
        if (onChange) onChange (x, y, act);
        repaint();
    }
};

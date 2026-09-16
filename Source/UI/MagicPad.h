#pragma once
#include <JuceHeader.h>

/** Hexagonal Kaossilator-style XY pad — cursor clamped inside hex */
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

        auto hex = makeHex (hexArea());

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

        auto c = hex.getBounds().getCentre();
        float rad = hexRadius (hexArea());
        for (int ring = 1; ring <= 3; ++ring)
        {
            float rr = rad * (float) ring / 3.f;
            g.setColour (accent.withAlpha (0.08f));
            g.drawEllipse (c.x - rr, c.y - rr, rr * 2.f, rr * 2.f, 1.f);
        }

        g.setColour (accent.withAlpha (0.14f));
        for (int i = 0; i < 6; ++i)
        {
            float a = (float) i / 6.f * juce::MathConstants<float>::twoPi
                      - juce::MathConstants<float>::halfPi;
            g.drawLine (c.x, c.y, c.x + std::cos (a) * rad, c.y + std::sin (a) * rad, 1.f);
        }

        for (size_t i = 0; i < trail.size(); ++i)
        {
            float a = (float) (i + 1) / (float) juce::jmax ((size_t) 1, trail.size());
            g.setColour (accent.withAlpha (0.12f * a));
            auto p = toScreen (trail[i]);
            float s = 3.f + 4.f * a;
            g.fillEllipse (p.x - s, p.y - s, s * 2.f, s * 2.f);
        }

        // cursor always inside hex
        auto pt = toScreen ({ x, y });
        if (touching)
        {
            g.setColour (accent.withAlpha (0.28f));
            g.fillEllipse (pt.x - 22.f, pt.y - 22.f, 44.f, 44.f);
        }
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.fillEllipse (pt.x - 7.f, pt.y - 7.f, 14.f, 14.f);
        g.setColour (accent);
        g.drawEllipse (pt.x - 11.f, pt.y - 11.f, 22.f, 22.f, 2.2f);

        g.setColour (accent);
        g.setFont (juce::FontOptions (15.f, juce::Font::bold));
        g.drawText ("MAGIC | " + modeName, r.removeFromTop (24).reduced (10, 0),
                    juce::Justification::centredLeft, false);

        g.setColour (accent.withAlpha (0.85f));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        g.drawText ("X " + juce::String (x, 2) + "   Y " + juce::String (y, 2),
                    getLocalBounds().removeFromBottom (40).removeFromTop (16).reduced (14, 0),
                    juce::Justification::centredRight);

        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.setFont (juce::FontOptions (10.f));
        g.drawText (touching ? "ACTIVE  |  drag to morph" : "touch inside hex  |  release = bypass",
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

    juce::Rectangle<float> hexArea() const
    {
        return getLocalBounds().toFloat().reduced (28.f, 36.f);
    }

    float hexRadius (juce::Rectangle<float> r) const
    {
        return juce::jmin (r.getWidth(), r.getHeight()) * 0.48f;
    }

    juce::Path makeHex (juce::Rectangle<float> r) const
    {
        juce::Path p;
        auto c = r.getCentre();
        float rad = hexRadius (r);
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

    /** Map normalised XY (0..1) to screen; uses inscribed square of hex for uniform mapping */
    juce::Point<float> toScreen (juce::Point<float> norm) const
    {
        auto area = hexArea();
        auto c = area.getCentre();
        float rad = hexRadius (area);
        // use flat-to-flat inscribed square (hex width = sqrt(3)*rad for pointy-top? 
        // Our hex is pointy-top: horizontal extent = rad * sqrt(3), vertical = 2*rad
        // Simpler: map through centre using radius * 0.85 so corners stay inside
        const float usable = rad * 0.82f;
        float px = c.x + (norm.x * 2.f - 1.f) * usable;
        float py = c.y - (norm.y * 2.f - 1.f) * usable; // y up
        return { px, py };
    }

    /** Clamp a screen point into the hex, return normalised 0..1 */
    void screenToNormClamped (juce::Point<float> pos, float& nx, float& ny) const
    {
        auto area = hexArea();
        auto c = area.getCentre();
        float rad = hexRadius (area);
        const float usable = rad * 0.82f;

        float dx = pos.x - c.x;
        float dy = c.y - pos.y; // y up

        // clamp to circle first (conservative inside hex)
        float len = std::sqrt (dx * dx + dy * dy);
        if (len > usable && len > 1e-6f)
        {
            dx *= usable / len;
            dy *= usable / len;
        }

        // also reject points outside actual hex path
        auto hex = makeHex (area);
        juce::Point<float> candidate { c.x + dx, c.y - dy };
        if (! hex.contains (candidate))
        {
            // pull toward centre until inside
            for (int i = 0; i < 8; ++i)
            {
                dx *= 0.85f;
                dy *= 0.85f;
                candidate = { c.x + dx, c.y - dy };
                if (hex.contains (candidate)) break;
            }
        }

        nx = juce::jlimit (0.f, 1.f, 0.5f + 0.5f * (dx / usable));
        ny = juce::jlimit (0.f, 1.f, 0.5f + 0.5f * (dy / usable));
    }

    void updateFromPos (juce::Point<float> pos, bool act)
    {
        float nx, ny;
        screenToNormClamped (pos, nx, ny);
        x = nx; y = ny; touching = act;
        if (onChange) onChange (x, y, act);
        repaint();
    }
};

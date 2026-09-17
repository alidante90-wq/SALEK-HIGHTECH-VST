#pragma once
#include "Modulation/LFO.h"

/** Mini live monitor for each FX section row */
class FxMonitor : public juce::Component, private juce::Timer
{
public:
    enum Kind { Chorus, Delay, Reverb, Master, Comp, EQ, Phaser, Dist };
    FxMonitor() { startTimerHz (24); }
    void setKind (Kind k) { kind = k; }
    void setLevel (float v) { level = juce::jlimit (0.f, 1.f, v); }
    void setAccent (juce::Colour c) { accent = c; }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 5.f);
        g.setColour (accent.withAlpha (0.45f));
        g.drawRoundedRectangle (r, 5.f, 1.f);

        const float t = phase;
        juce::Path path;
        const int N = 32;
        auto plot = r.reduced (4.f, 3.f);
        for (int i = 0; i < N; ++i)
        {
            float x = (float) i / (N - 1);
            float y = 0.f;
            switch (kind)
            {
                case Chorus:  y = std::sin (x * 12.f + t * 3.f) * 0.35f * (0.4f + level); break;
                case Delay:   y = std::sin (x * 6.f + t) * 0.3f * (0.3f + level)
                                  + std::sin (x * 6.f + t - 1.2f) * 0.2f * level; break;
                case Reverb:  y = (std::sin (x * 20.f + t * 2.f) * 0.15f
                                  + std::sin (x * 7.f + t) * 0.2f) * (0.3f + level); break;
                case Master:  y = juce::jlimit (-0.45f, 0.45f, level * 0.5f * std::sin (x * 8.f + t)); break;
                case Comp:    y = std::tanh (std::sin (x * 5.f + t) * (0.5f + level)) * 0.35f; break;
                case EQ:      y = std::sin (x * 4.f + t) * 0.2f
                                  + std::sin (x * 14.f + t * 1.5f) * 0.15f * level; break;
                case Phaser:  y = std::sin (x * 8.f + t * 2.f + std::sin (t) * 2.f) * 0.35f * (0.3f + level); break;
                case Dist:    {
                    float s = std::sin (x * 6.f + t);
                    y = std::tanh (s * (1.f + level * 4.f)) * 0.4f;
                    break;
                }
            }
            float px = plot.getX() + x * plot.getWidth();
            float py = plot.getCentreY() - y * plot.getHeight();
            if (i == 0) path.startNewSubPath (px, py); else path.lineTo (px, py);
        }
        g.setColour (accent.withAlpha (0.85f));
        g.strokePath (path, juce::PathStrokeType (1.3f));

        float bh = level * (plot.getHeight() - 2.f);
        g.setColour (accent.withAlpha (0.25f));
        g.fillRect (plot.getRight() - 4.f, plot.getBottom() - bh, 3.f, bh);
    }
    void timerCallback() override { phase += 0.08f + level * 0.1f; repaint(); }
private:
    Kind kind = Chorus;
    float phase = 0.f, level = 0.35f;
    juce::Colour accent { 0xff00e8ff };
};

/** Editable LFO shape (Serum-style modifiers)
 *  Drag = set point
 *  Shift = linear ramp from last anchor to current
 *  Alt  = smooth nearby points
 *  Ctrl = freehand stroke (paint across indices while dragging)
 */
class LfoShapeEditor : public juce::Component
{
public:
    LfoShapeEditor()
    {
        for (int i = 0; i < 16; ++i)
        {
            float t = (float) i / 15.f;
            points[i] = std::sin (t * juce::MathConstants<float>::twoPi);
        }
    }

    void setLfo (salek::LFO* l) { lfo = l; if (lfo) syncFromLfo(); }
    salek::LFO* getLfo() const noexcept { return lfo; }

    void syncFromLfo()
    {
        if (! lfo) return;
        for (int i = 0; i < 16; ++i)
            points[i] = lfo->getCustomPoint (i);
        repaint();
    }

    /** Copy current table into another LFO instance */
    void applyTo (salek::LFO& dest)
    {
        for (int i = 0; i < 16; ++i)
            dest.setCustomPoint (i, points[i]);
    }

    /** Snapshot points into a reusable bank slot (0..2) */
    void saveSlot (int slot)
    {
        if (slot < 0 || slot > 2) return;
        for (int i = 0; i < 16; ++i)
            bank[(size_t) slot][(size_t) i] = points[i];
        bankValid[(size_t) slot] = true;
    }

    void loadSlot (int slot)
    {
        if (slot < 0 || slot > 2 || ! bankValid[(size_t) slot]) return;
        for (int i = 0; i < 16; ++i)
        {
            points[i] = bank[(size_t) slot][(size_t) i];
            if (lfo) lfo->setCustomPoint (i, points[i]);
        }
        repaint();
    }

    bool slotValid (int slot) const
    {
        return slot >= 0 && slot <= 2 && bankValid[(size_t) slot];
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 8.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRoundedRectangle (r, 8.f, 1.2f);

        auto plot = r.reduced (10.f, 16.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.08f));
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());
        for (int i = 1; i < 4; ++i)
        {
            float x = plot.getX() + plot.getWidth() * (float) i / 4.f;
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        }

        juce::Path wave;
        for (int i = 0; i < 16; ++i)
        {
            float x = plot.getX() + (float) i / 15.f * plot.getWidth();
            float y = plot.getCentreY() - points[i] * plot.getHeight() * 0.45f;
            if (i == 0) wave.startNewSubPath (x, y); else wave.lineTo (x, y);
        }
        g.setColour (juce::Colour (0xff00e8ff));
        g.strokePath (wave, juce::PathStrokeType (2.0f));

        for (int i = 0; i < 16; ++i)
        {
            float x = plot.getX() + (float) i / 15.f * plot.getWidth();
            float y = plot.getCentreY() - points[i] * plot.getHeight() * 0.45f;
            g.setColour (juce::Colour (0xffff2d9b));
            g.fillEllipse (x - 4.f, y - 4.f, 8.f, 8.f);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.fillEllipse (x - 2.f, y - 2.f, 4.f, 4.f);
        }

        g.setColour (juce::Colour (0xffffd700).withAlpha (0.9f));
        g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        // EN + FA hint
        g.drawText ("DRAW freehand | Shift=line | Alt=smooth | drag to paint",
            getLocalBounds().removeFromTop (14).reduced (6, 0),
            juce::Justification::centredLeft);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        lastIdx = -1;
        dragAt (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override { dragAt (e); }

private:
    salek::LFO* lfo = nullptr;
    float points[16] {};
    float bank[3][16] {};
    bool bankValid[3] { false, false, false };
    int lastIdx = -1;

    void writePoint (int i, float v)
    {
        points[i] = v;
        if (lfo) lfo->setCustomPoint (i, v);
    }

    void dragAt (const juce::MouseEvent& e)
    {
        auto plot = getLocalBounds().toFloat().reduced (12.f, 18.f);
        if (plot.getWidth() < 1.f) return;

        int idx = juce::jlimit (0, 15,
            (int) std::round ((e.position.x - plot.getX()) / plot.getWidth() * 15.f));
        float v = juce::jlimit (-1.f, 1.f,
            (plot.getCentreY() - e.position.y) / (plot.getHeight() * 0.45f));

        if (e.mods.isCtrlDown() || e.mods.isCommandDown())
        {
            // Freehand: interpolate across all indices between last and current
            if (lastIdx < 0)
            {
                writePoint (idx, v);
            }
            else
            {
                int a = juce::jmin (lastIdx, idx);
                int b = juce::jmax (lastIdx, idx);
                float va = points[lastIdx];
                for (int i = a; i <= b; ++i)
                {
                    float t = (b == a) ? 1.f : (float) (i - a) / (float) (b - a);
                    // if moving right, blend from last value toward v
                    float pv = (lastIdx <= idx)
                        ? va * (1.f - t) + v * t
                        : v * (1.f - t) + va * t;
                    writePoint (i, pv);
                }
            }
            lastIdx = idx;
        }
        else if (e.mods.isShiftDown())
        {
            // Linear ramp from previous anchor
            int anchor = (lastIdx >= 0) ? lastIdx : juce::jmax (0, idx - 1);
            int a = juce::jmin (anchor, idx);
            int b = juce::jmax (anchor, idx);
            float va = points[anchor];
            for (int i = a; i <= b; ++i)
            {
                float t = (b == a) ? 1.f : (float) (i - a) / (float) (b - a);
                writePoint (i, va * (1.f - t) + v * t);
            }
            lastIdx = idx;
        }
        else if (e.mods.isAltDown())
        {
            // Soft smooth neighborhood
            for (int d = -2; d <= 2; ++d)
            {
                int j = idx + d;
                if (j < 0 || j > 15) continue;
                float w = 1.f - std::abs ((float) d) * 0.35f;
                writePoint (j, points[j] * (1.f - w * 0.55f) + v * (w * 0.55f));
            }
            lastIdx = idx;
        }
        else
        {
            // Default = freehand paint (Serum/Vital style) across indices
            if (lastIdx < 0)
            {
                writePoint (idx, v);
            }
            else
            {
                int a = juce::jmin (lastIdx, idx);
                int b = juce::jmax (lastIdx, idx);
                float va = points[lastIdx];
                for (int i = a; i <= b; ++i)
                {
                    float tt = (b == a) ? 1.f : (float) (i - a) / (float) (b - a);
                    float pv = (lastIdx <= idx) ? va * (1.f - tt) + v * tt
                                                : v * (1.f - tt) + va * tt;
                    writePoint (i, pv);
                }
            }
            lastIdx = idx;
        }
        repaint();
    }
};

#pragma once
#include "Modulation/LFO.h"
#include "FX/SimpleCompressor.h"

/** Mini live monitor for each FX section row */
class FxMonitor : public juce::Component, private juce::Timer
{
public:
    enum Kind { Chorus, Delay, Reverb, Master, Comp, EQ, Phaser, Dist };
    FxMonitor() { startTimerHz (24); }
    void setKind (Kind k) { kind = k; }
    void setLevel (float v) { level = juce::jlimit (0.f, 1.f, v); }
    void setAccent (juce::Colour c) { accent = c; }
    void setBandGR (float lo, float mid, float hi)
    {
        bandGR[0] = juce::jlimit (0.f, 1.f, lo);
        bandGR[1] = juce::jlimit (0.f, 1.f, mid);
        bandGR[2] = juce::jlimit (0.f, 1.f, hi);
    }
    void setEqBands (float lo, float mid, float hi)
    {
        eqBand[0] = juce::jlimit (-1.f, 1.f, lo);
        eqBand[1] = juce::jlimit (-1.f, 1.f, mid);
        eqBand[2] = juce::jlimit (-1.f, 1.f, hi);
    }
    void setThresholdNorm (float t)
    {
        if (draggingBand < 0)
            thrNorm[0] = thrNorm[1] = thrNorm[2] = juce::jlimit (0.f, 1.f, t);
    }
    void setBandThresholdNorms (float lo, float mid, float hi)
    {
        if (draggingBand < 0)
        {
            thrNorm[0] = juce::jlimit (0.f, 1.f, lo);
            thrNorm[1] = juce::jlimit (0.f, 1.f, mid);
            thrNorm[2] = juce::jlimit (0.f, 1.f, hi);
        }
    }
    void bindThresholdParam (juce::RangedAudioParameter* p) { thrParam = p; }
    void bindCompressor (salek::SimpleCompressor* c) { comp = c; }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (kind != Comp) return;
        auto plot = getLocalBounds().toFloat().reduced (5.f, 4.f);
        const float rowH = (plot.getHeight() - 4.f) / 3.f;
        for (int b = 0; b < 3; ++b)
        {
            auto row = juce::Rectangle<float> (plot.getX(), plot.getY() + b * rowH, plot.getWidth(), rowH);
            if (! row.contains (e.position)) continue;
            float tx = row.getX() + thrNorm[b] * row.getWidth();
            // whole row is draggable for that band threshold
            draggingBand = b;
            if (thrParam != nullptr) thrParam->beginChangeGesture();
            setBandFromX (e.position.x, plot);
            return;
        }
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (draggingBand < 0) return;
        auto plot = getLocalBounds().toFloat().reduced (5.f, 4.f);
        setBandFromX (e.position.x, plot);
        repaint();
    }
    void mouseUp (const juce::MouseEvent&) override
    {
        if (draggingBand >= 0 && thrParam != nullptr)
            thrParam->endChangeGesture();
        draggingBand = -1;
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 5.f);
        g.setColour (accent.withAlpha (0.45f));
        g.drawRoundedRectangle (r, 5.f, 1.f);

        auto plot = r.reduced (4.f, 3.f);

        if (kind == Comp)
        {
            juce::Colour cols[3] = {
                juce::Colour (0xff00e8ff), juce::Colour (0xffffd700), juce::Colour (0xffff2d9b)
            };
            const char* labs[3] = { "LO", "MID", "HI" };
            const float rowH = (plot.getHeight() - 4.f) / 3.f;
            for (int b = 0; b < 3; ++b)
            {
                dispGR[(size_t) b] += 0.25f * (bandGR[(size_t) b] - dispGR[(size_t) b]);
                auto row = juce::Rectangle<float> (
                    plot.getX(), plot.getY() + b * rowH, plot.getWidth(), rowH).reduced (1.f, 2.f);

                // track
                g.setColour (cols[b].withAlpha (0.14f));
                g.fillRoundedRectangle (row, 2.f);

                // GR fill from right
                float grW = dispGR[(size_t) b] * row.getWidth();
                g.setColour (cols[b].withAlpha (0.9f));
                g.fillRoundedRectangle (
                    juce::Rectangle<float> (row.getRight() - grW, row.getY(), grW, row.getHeight()), 2.f);

                // independent threshold line for THIS band
                float tx = row.getX() + thrNorm[b] * row.getWidth();
                const bool hot = (draggingBand == b);
                g.setColour (juce::Colours::white.withAlpha (hot ? 1.f : 0.85f));
                g.drawVerticalLine ((int) tx, row.getY(), row.getBottom());
                g.setColour (cols[b]);
                g.fillEllipse (tx - 3.5f, row.getCentreY() - 3.5f, 7.f, 7.f);

                // label + dB
                g.setFont (juce::FontOptions (8.f, juce::Font::bold));
                g.setColour (cols[b]);
                const float db = juce::jmap (thrNorm[b], 0.f, 1.f, -40.f, 0.f);
                g.drawText (juce::String (labs[b]) + " " + juce::String (db, 0),
                            row.toNearestInt().removeFromLeft (48),
                            juce::Justification::centredLeft);
            }
            return;
        }

        if (kind == EQ)
        {
            const float gap = 3.f;
            const float bw = (plot.getWidth() - gap * 2.f) / 3.f;
            const float midY = plot.getCentreY();
            const float maxH = plot.getHeight() * 0.42f;
            juce::Colour cols[3] = {
                juce::Colour (0xff00e8ff), juce::Colour (0xff39ff14), juce::Colour (0xffff2d9b)
            };
            const char* labs[3] = { "LO", "MID", "HI" };
            g.setColour (juce::Colours::white.withAlpha (0.15f));
            g.drawHorizontalLine ((int) midY, plot.getX(), plot.getRight());
            for (int b = 0; b < 3; ++b)
            {
                float v = eqBand[(size_t) b];
                float h = std::abs (v) * maxH;
                float x = plot.getX() + b * (bw + gap);
                auto bar = v >= 0.f
                    ? juce::Rectangle<float> (x, midY - h, bw, h)
                    : juce::Rectangle<float> (x, midY, bw, h);
                g.setColour (cols[b].withAlpha (0.85f));
                g.fillRoundedRectangle (bar, 2.f);
                g.setFont (juce::FontOptions (8.f, juce::Font::bold));
                g.setColour (cols[b]);
                g.drawText (labs[b], (int) x, (int) plot.getBottom() - 10, (int) bw, 10,
                            juce::Justification::centred);
            }
            return;
        }

        const float t = phase;
        juce::Path path;
        const int N = 32;
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
                case Comp:    break;
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
    void setBandFromX (float x, juce::Rectangle<float> plot)
    {
        if (draggingBand < 0 || draggingBand > 2) return;
        thrNorm[draggingBand] = juce::jlimit (0.f, 1.f,
            (x - plot.getX()) / juce::jmax (1.f, plot.getWidth()));
        const float db = juce::jmap (thrNorm[draggingBand], 0.f, 1.f, -40.f, 0.f);
        if (comp != nullptr)
            comp->setBandThresholdDb (draggingBand, db);
        // also nudge main threshold param to mid band for host automation
        if (thrParam != nullptr && draggingBand == 1)
            thrParam->setValueNotifyingHost (thrParam->convertTo0to1 (db));
    }

    Kind kind = Chorus;
    float phase = 0.f, level = 0.35f;
    float bandGR[3] {}, dispGR[3] {}, eqBand[3] {};
    float thrNorm[3] { 0.45f, 0.55f, 0.65f };
    int draggingBand = -1;
    juce::RangedAudioParameter* thrParam = nullptr;
    salek::SimpleCompressor* comp = nullptr;
    juce::Colour accent { 0xff00e8ff };
};

/** Editable LFO shape — freehand paint (Serum 2 style) */
class LfoShapeEditor : public juce::Component
{
public:
    static constexpr int N = salek::LFO::TableSize;

    LfoShapeEditor()
    {
        for (int i = 0; i < N; ++i)
        {
            float t = (float) i / (float) (N - 1);
            points[i] = std::sin (t * juce::MathConstants<float>::twoPi);
        }
    }

    void setLfo (salek::LFO* l) { lfo = l; if (lfo) syncFromLfo(); }
    salek::LFO* getLfo() const noexcept { return lfo; }

    void syncFromLfo()
    {
        if (! lfo) return;
        for (int i = 0; i < N; ++i)
            points[i] = lfo->getCustomPoint (i);
        repaint();
    }

    void applyTo (salek::LFO& dest)
    {
        for (int i = 0; i < N; ++i)
            dest.setCustomPoint (i, points[i]);
    }

    void saveSlot (int slot)
    {
        if (slot < 0 || slot > 2) return;
        for (int i = 0; i < N; ++i)
            bank[(size_t) slot][(size_t) i] = points[i];
        bankValid[(size_t) slot] = true;
    }

    void loadSlot (int slot)
    {
        if (slot < 0 || slot > 2 || ! bankValid[(size_t) slot]) return;
        for (int i = 0; i < N; ++i)
        {
            points[i] = bank[(size_t) slot][(size_t) i];
            if (lfo) lfo->setCustomPoint (i, points[i]);
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (juce::Colour (0xff080414));
        g.fillRoundedRectangle (r, 8.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.45f));
        g.drawRoundedRectangle (r, 8.f, 1.4f);

        auto plot = r.reduced (8.f, 18.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.07f));
        for (int i = 1; i < 8; ++i)
        {
            float x = plot.getX() + plot.getWidth() * (float) i / 8.f;
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
        }
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.2f));
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

        juce::Path fill, wave;
        for (int i = 0; i < N; ++i)
        {
            float x = plot.getX() + (float) i / (float) (N - 1) * plot.getWidth();
            float y = plot.getCentreY() - points[i] * plot.getHeight() * 0.45f;
            if (i == 0) { wave.startNewSubPath (x, y); fill.startNewSubPath (x, plot.getBottom()); fill.lineTo (x, y); }
            else { wave.lineTo (x, y); fill.lineTo (x, y); }
        }
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.closeSubPath();
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.12f));
        g.fillPath (fill);
        g.setColour (juce::Colour (0xff00e8ff));
        g.strokePath (wave, juce::PathStrokeType (2.2f));

        g.setColour (juce::Colour (0xffffd700));
        g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.drawText ("LFO SHAPE  |  drag = paint",
                    getLocalBounds().removeFromTop (16).reduced (8, 0),
                    juce::Justification::centredLeft);
    }

    void mouseDown (const juce::MouseEvent& e) override { lastIdx = -1; dragAt (e); }
    void mouseDrag (const juce::MouseEvent& e) override { dragAt (e); }

private:
    salek::LFO* lfo = nullptr;
    float points[N] {};
    float bank[3][N] {};
    bool bankValid[3] { false, false, false };
    int lastIdx = -1;

    void writePoint (int i, float v)
    {
        i = juce::jlimit (0, N - 1, i);
        points[i] = juce::jlimit (-1.f, 1.f, v);
        if (lfo)
        {
            lfo->setCustomPoint (i, points[i]);
            lfo->setWave (salek::LFO::Wave::Custom);
        }
    }

    void dragAt (const juce::MouseEvent& e)
    {
        auto plot = getLocalBounds().toFloat().reduced (10.f, 20.f);
        if (plot.getWidth() < 1.f) return;
        int idx = juce::jlimit (0, N - 1,
            (int) std::round ((e.position.x - plot.getX()) / plot.getWidth() * (float) (N - 1)));
        float v = juce::jlimit (-1.f, 1.f,
            (plot.getCentreY() - e.position.y) / (plot.getHeight() * 0.45f));
        if (lastIdx < 0) writePoint (idx, v);
        else
        {
            int a = juce::jmin (lastIdx, idx), b = juce::jmax (lastIdx, idx);
            float va = points[lastIdx];
            for (int i = a; i <= b; ++i)
            {
                float tt = (b == a) ? 1.f : (float) (i - a) / (float) (b - a);
                float pv = (lastIdx <= idx) ? va * (1.f - tt) + v * tt : v * (1.f - tt) + va * tt;
                writePoint (i, pv);
            }
        }
        lastIdx = idx;
        repaint();
    }
};

#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

/** Route-list mod matrix (Vital/Serum style) — no crowded grid */
class ModMatrixPanel : public juce::Component, private juce::Timer
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m)
    {
        startTimerHz (15);
        // Seed a few empty slots so the list is never blank
        if (countActive() == 0)
        {
            matrix.addRoute (salek::ModMatrix::Source::LFO1, salek::ModMatrix::Dest::FilterCutoff, 0.35f);
            matrix.addRoute (salek::ModMatrix::Source::LFO2, salek::ModMatrix::Dest::Osc1Table, 0.0f);
            matrix.addRoute (salek::ModMatrix::Source::LFO3, salek::ModMatrix::Dest::Osc1Warp, 0.0f);
        }
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff0a0614).withAlpha (0.92f));
        g.fillRoundedRectangle (bounds, 10.f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRoundedRectangle (bounds, 10.f, 1.4f);

        auto header = bounds.removeFromTop (26.f).reduced (8.f, 4.f);
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.setColour (juce::Colour (0xffffd700));
        g.drawText ("MOD ROUTES", header.removeFromLeft (110.f), juce::Justification::centredLeft);

        // + ADD / CLR hit zones
        addBtn = header.removeFromRight (52.f).toNearestInt();
        clrBtn = header.removeFromRight (52.f).reduced (4, 0).toNearestInt();
        drawChip (g, addBtn.toFloat(), "+ ADD", juce::Colour (0xff39ff14));
        drawChip (g, clrBtn.toFloat(), "CLR", juce::Colour (0xffff2d9b));

        g.setFont (juce::FontOptions (9.5f));
        g.setColour (juce::Colour (0xffa090c0));
        g.drawText ("drag bar / wheel = depth   |   arm LFO in header then click a knob",
                    header, juce::Justification::centredLeft);

        // Live LFO meters strip
        auto meters = bounds.removeFromTop (28.f).reduced (8.f, 2.f);
        const float mw = meters.getWidth() / 3.f;
        const juce::Colour cols[3] { juce::Colour (0xff00e8ff), juce::Colour (0xffff2d9b), juce::Colour (0xff39ff14) };
        for (int i = 0; i < 3; ++i)
        {
            auto cell = meters.withX (meters.getX() + i * mw).withWidth (mw - 4.f);
            float v = matrix.getSourceValue ((salek::ModMatrix::Source) i);
            g.setColour (juce::Colour (0xff12101c));
            g.fillRoundedRectangle (cell, 4.f);
            g.setColour (cols[i].withAlpha (0.9f));
            g.setFont (juce::FontOptions (10.f, juce::Font::bold));
            g.drawText ("LFO" + juce::String (i + 1), cell.removeFromLeft (36.f), juce::Justification::centred);
            auto bar = cell.reduced (4.f, 6.f);
            g.setColour (juce::Colour (0xff1a1028));
            g.fillRoundedRectangle (bar, 3.f);
            const float mid = bar.getCentreX();
            const float half = bar.getWidth() * 0.5f;
            g.setColour (cols[i].withAlpha (0.85f));
            if (v >= 0.f)
                g.fillRect (mid, bar.getY(), half * v, bar.getHeight());
            else
                g.fillRect (mid + half * v, bar.getY(), -half * v, bar.getHeight());
            g.setColour (juce::Colours::white.withAlpha (0.5f));
            g.drawVerticalLine ((int) mid, bar.getY(), bar.getBottom());
        }

        // Route rows
        rebuildRows (bounds.reduced (6.f, 4.f));
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));
        for (int i = 0; i < (int) rows.size(); ++i)
        {
            const auto& row = rows[(size_t) i];
            auto rr = row.bounds;
            const bool sel = (i == selected);
            g.setColour (sel ? juce::Colour (0xff1a1030) : juce::Colour (0xff100818));
            g.fillRoundedRectangle (rr, 6.f);
            g.setColour ((sel ? juce::Colour (0xff00e8ff) : juce::Colour (0xff3a2850)).withAlpha (0.7f));
            g.drawRoundedRectangle (rr, 6.f, sel ? 1.6f : 1.0f);

            auto left = rr.removeFromLeft (130.f).reduced (6.f, 0.f);
            g.setColour (juce::Colour (0xff00e8ff));
            g.drawText (salek::ModMatrix::sourceName (row.src), left.removeFromLeft (48.f),
                        juce::Justification::centredLeft);
            g.setColour (juce::Colour (0xff8060a0));
            g.drawText ("->", left.removeFromLeft (18.f), juce::Justification::centred);
            g.setColour (juce::Colour (0xffffd700));
            g.drawText (salek::ModMatrix::destName (row.dst), left, juce::Justification::centredLeft);

            auto xZone = rr.removeFromRight (28.f).reduced (4.f, 6.f);
            g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.8f));
            g.drawText ("X", xZone.toNearestInt(), juce::Justification::centred);

            auto amtLabel = rr.removeFromRight (48.f);
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.setFont (juce::FontOptions (10.f, juce::Font::bold));
            g.drawText (juce::String (row.amount, 2), amtLabel.toNearestInt(), juce::Justification::centred);

            auto bar = rr.reduced (6.f, 10.f);
            g.setColour (juce::Colour (0xff0a0614));
            g.fillRoundedRectangle (bar, 4.f);
            const float midX = bar.getCentreX();
            const float a = juce::jlimit (-1.f, 1.f, row.amount);
            if (a >= 0.f)
            {
                g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.9f));
                g.fillRect (midX, bar.getY(), bar.getWidth() * 0.5f * a, bar.getHeight());
            }
            else
            {
                g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.9f));
                g.fillRect (midX + bar.getWidth() * 0.5f * a, bar.getY(), -bar.getWidth() * 0.5f * a, bar.getHeight());
            }
            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.drawVerticalLine ((int) midX, bar.getY(), bar.getBottom());
        }

        if (rows.empty())
        {
            g.setColour (juce::Colour (0xff8060a0));
            g.setFont (juce::FontOptions (13.f));
            g.drawText ("No routes — press + ADD or arm LFO and click a knob",
                        bounds, juce::Justification::centred);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (addBtn.contains (e.getPosition()))
        {
            matrix.addRoute (salek::ModMatrix::Source::LFO1, salek::ModMatrix::Dest::FilterCutoff, 0.4f);
            repaint();
            return;
        }
        if (clrBtn.contains (e.getPosition()))
        {
            matrix.clear();
            repaint();
            return;
        }
        for (int i = 0; i < (int) rows.size(); ++i)
        {
            auto& row = rows[(size_t) i];
            if (! row.bounds.contains (e.position))
                continue;
            selected = i;
            auto xZone = row.bounds.withX (row.bounds.getRight() - 28.f).withWidth (28.f);
            if (xZone.contains (e.position))
            {
                matrix.removeRoute (row.src, row.dst);
                selected = -1;
                repaint();
                return;
            }
            dragging = true;
            dragIndex = i;
            setAmountFromX (i, e.position.x);
            return;
        }
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragging && dragIndex >= 0)
            setAmountFromX (dragIndex, e.position.x);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragging = false;
        dragIndex = -1;
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < (int) rows.size(); ++i)
        {
            auto& row = rows[(size_t) i];
            if (row.bounds.contains (e.position))
            {
                matrix.removeRoute (row.src, row.dst);
                repaint();
                return;
            }
        }
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        for (int i = 0; i < (int) rows.size(); ++i)
        {
            auto& row = rows[(size_t) i];
            if (! row.bounds.contains (e.position))
                continue;
            float a = row.amount + wheel.deltaY * 0.08f;
            matrix.addRoute (row.src, row.dst, juce::jlimit (-1.f, 1.f, a));
            repaint();
            return;
        }
    }

    void timerCallback() override { if (isShowing()) repaint(); }

private:
    struct Row
    {
        salek::ModMatrix::Source src {};
        salek::ModMatrix::Dest dst {};
        float amount = 0.f;
        juce::Rectangle<float> bounds;
    };

    salek::ModMatrix& matrix;
    std::vector<Row> rows;
    int selected = -1;
    bool dragging = false;
    int dragIndex = -1;
    juce::Rectangle<int> addBtn, clrBtn;

    int countActive() const
    {
        int n = 0;
        for (auto& r : matrix.getRoutes())
            if (r.active) ++n;
        return n;
    }

    void drawChip (juce::Graphics& g, juce::Rectangle<float> r, const char* txt, juce::Colour c)
    {
        g.setColour (c.withAlpha (0.2f));
        g.fillRoundedRectangle (r, 4.f);
        g.setColour (c);
        g.drawRoundedRectangle (r, 4.f, 1.f);
        g.setFont (juce::FontOptions (10.f, juce::Font::bold));
        g.drawText (txt, r.toNearestInt(), juce::Justification::centred);
    }

    void rebuildRows (juce::Rectangle<float> area)
    {
        rows.clear();
        const float rowH = 36.f;
        float y = area.getY();
        for (auto& r : matrix.getRoutes())
        {
            if (! r.active) continue;
            if (y + rowH > area.getBottom()) break;
            Row row;
            row.src = r.source;
            row.dst = r.dest;
            row.amount = r.amount;
            row.bounds = { area.getX(), y, area.getWidth(), rowH - 4.f };
            rows.push_back (row);
            y += rowH;
        }
    }

    void setAmountFromX (int index, float x)
    {
        if (index < 0 || index >= (int) rows.size()) return;
        auto& row = rows[(size_t) index];
        // amount bar is roughly center 55% of row
        auto bar = row.bounds.withTrimmedLeft (130.f).withTrimmedRight (76.f).reduced (6.f, 10.f);
        float t = (x - bar.getX()) / juce::jmax (1.f, bar.getWidth());
        float amt = juce::jlimit (-1.f, 1.f, t * 2.f - 1.f);
        if (std::abs (amt) < 0.03f) amt = 0.f;
        matrix.addRoute (row.src, row.dst, amt);
        repaint();
    }
};

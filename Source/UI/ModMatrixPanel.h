#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

/** Continuous-drag mod matrix (Serum-style): vertical drag sets amount -1..+1 */
class ModMatrixPanel : public juce::Component, private juce::Timer
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m)
    {
        startTimerHz (24);
        matrix.addRoute (salek::ModMatrix::Source::LFO1, salek::ModMatrix::Dest::FilterCutoff, 0.0f);
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff0a0814));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.45f));
        g.drawRoundedRectangle (r, 8.0f, 1.2f);

        g.setColour (juce::Colour (0xffc0a0e0));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("MOD MATRIX  (drag vertically: -1 .. +1 continuous)",
                    r.removeFromTop (18.0f).reduced (6, 0), juce::Justification::centredLeft);

        const int nSrc = 5, nDst = 8;
        auto srcs = sources();
        auto dsts = dests();
        const char* sLab[] = { "LFO", "ENV", "M1", "M2", "M3" };
        const char* dLab[] = { "CUT", "RES", "O1LVL", "O2LVL", "TABLE", "WARP", "FM2", "PITCH" };

        auto grid = r.reduced (4.0f);
        const float labelW = 34.0f, labelH = 14.0f;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;

        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        for (int d = 0; d < nDst; ++d)
        {
            g.setColour (juce::Colour (0xff8890a8));
            g.drawText (dLab[d], grid.getX() + labelW + d * cellW, grid.getY(), cellW, labelH, juce::Justification::centred);
        }

        for (int s = 0; s < nSrc; ++s)
        {
            g.setColour (juce::Colour (0xff8890a8));
            g.drawText (sLab[s], grid.getX(), grid.getY() + labelH + s * cellH, labelW, cellH, juce::Justification::centred);

            for (int d = 0; d < nDst; ++d)
            {
                auto cell = juce::Rectangle<float> (
                    grid.getX() + labelW + d * cellW + 2.0f,
                    grid.getY() + labelH + s * cellH + 2.0f,
                    cellW - 4.0f, cellH - 4.0f);

                float amt = 0.0f;
                bool active = false;
                for (const auto& route : matrix.getRoutes())
                    if (route.active && route.source == srcs[s] && route.dest == dsts[d])
                    { amt = route.amount; active = true; break; }

                g.setColour (juce::Colour (0xff12101c));
                g.fillRoundedRectangle (cell, 4.0f);

                if (active && std::abs (amt) > 0.01f)
                {
                    const float midY = cell.getCentreY();
                    const float fillH = std::abs (amt) * (cell.getHeight() * 0.5f);
                    juce::Rectangle<float> fill;
                    if (amt >= 0.0f)
                        fill = { cell.getX() + 1.0f, midY - fillH, cell.getWidth() - 2.0f, fillH };
                    else
                        fill = { cell.getX() + 1.0f, midY, cell.getWidth() - 2.0f, fillH };

                    juce::Colour col = amt >= 0.0f ? juce::Colour (0xff00e8ff) : juce::Colour (0xffff2d9b);
                    g.setColour (col.withAlpha (0.55f + std::abs (amt) * 0.4f));
                    g.fillRoundedRectangle (fill, 3.0f);

                    g.setColour (juce::Colours::white.withAlpha (0.25f));
                    g.drawHorizontalLine ((int) midY, cell.getX(), cell.getRight());
                }

                const bool isDrag = (dragSrc == s && dragDst == d);
                g.setColour (isDrag ? juce::Colour (0xffffd700) :
                             (active ? juce::Colour (0xff00e8ff).withAlpha (0.6f) : juce::Colour (0xff2a2840)));
                g.drawRoundedRectangle (cell, 4.0f, isDrag ? 1.8f : 1.0f);

                if (active && std::abs (amt) > 0.01f)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.9f));
                    g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
                    g.drawText (juce::String (amt, 2), cell, juce::Justification::centred);
                }
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! hitCell (e.position, dragSrc, dragDst))
        {
            dragSrc = dragDst = -1;
            return;
        }
        setAmountFromY (e.position.y);
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragSrc < 0 || dragDst < 0) return;
        setAmountFromY (e.position.y);
        repaint();
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragSrc = dragDst = -1;
        repaint();
    }

    void timerCallback() override { repaint(); }

private:
    salek::ModMatrix& matrix;
    int dragSrc = -1, dragDst = -1;

    static std::array<salek::ModMatrix::Source, 5> sources()
    {
        using S = salek::ModMatrix::Source;
        return { S::LFO1, S::Env1, S::Macro1, S::Macro2, S::Macro3 };
    }

    static std::array<salek::ModMatrix::Dest, 8> dests()
    {
        using D = salek::ModMatrix::Dest;
        return { D::FilterCutoff, D::FilterReso, D::Osc1Level, D::Osc2Level,
                 D::Osc1Table, D::Osc1Warp, D::Fm2to1, D::Pitch };
    }

    bool hitCell (juce::Point<float> pos, int& outS, int& outD) const
    {
        const int nSrc = 5, nDst = 8;
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        r.removeFromTop (18.0f);
        auto grid = r.reduced (4.0f);
        const float labelW = 34.0f, labelH = 14.0f;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;
        int d = (int) ((pos.x - grid.getX() - labelW) / cellW);
        int s = (int) ((pos.y - grid.getY() - labelH) / cellH);
        if (s < 0 || s >= nSrc || d < 0 || d >= nDst) return false;
        outS = s; outD = d;
        return true;
    }

    void setAmountFromY (float mouseY)
    {
        if (dragSrc < 0 || dragDst < 0) return;
        const int nSrc = 5;
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        r.removeFromTop (18.0f);
        auto grid = r.reduced (4.0f);
        const float labelH = 14.0f;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;
        const float cellTop = grid.getY() + labelH + dragSrc * cellH + 2.0f;
        const float cellBot = cellTop + cellH - 4.0f;
        float t = (mouseY - cellTop) / juce::jmax (1.0f, cellBot - cellTop);
        float amt = juce::jlimit (-1.0f, 1.0f, 1.0f - 2.0f * t);
        amt = std::round (amt * 100.0f) / 100.0f;
        matrix.addRoute (sources()[(size_t) dragSrc], dests()[(size_t) dragDst], amt);
    }
};

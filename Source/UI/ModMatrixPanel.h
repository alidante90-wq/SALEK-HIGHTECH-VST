#pragma once
#include <JuceHeader.h>
#include "Modulation/ModMatrix.h"

/** Continuous mod matrix: drag vertically in a cell to set amount -1..+1 */
class ModMatrixPanel : public juce::Component
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m) {}

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff0a0614));
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
        const char* dLab[] = { "CUT", "RES", "O1L", "O2L", "TAB", "WA", "FM2", "PIT" };

        auto grid = r.reduced (4.0f);
        const float labelW = 34.0f, labelH = 14.0f;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;

        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        for (int d = 0; d < nDst; ++d)
        {
            g.setColour (juce::Colour (0xff8890a8));
            g.drawText (dLab[d], juce::Rectangle<float> (grid.getX() + labelW + d * cellW, grid.getY(), cellW, labelH), juce::Justification::centred);
        }

        for (int s = 0; s < nSrc; ++s)
        {
            g.setColour (juce::Colour (0xff8890a8));
            g.drawText (sLab[s], juce::Rectangle<float> (grid.getX(), grid.getY() + labelH + s * cellH, labelW, cellH), juce::Justification::centred);

            for (int d = 0; d < nDst; ++d)
            {
                auto cell = juce::Rectangle<float> (
                    grid.getX() + labelW + d * cellW + 2.0f,
                    grid.getY() + labelH + s * cellH + 2.0f,
                    cellW - 4.0f, cellH - 4.0f);

                g.setColour (juce::Colour (0xff12081c));
                g.fillRoundedRectangle (cell, 3.0f);

                float amt = matrix.getAmount (srcs[(size_t) s], dsts[(size_t) d]);
                if (std::abs (amt) > 0.001f)
                {
                    float midY = cell.getCentreY();
                    float barH = cell.getHeight() * 0.5f * std::abs (amt);
                    auto bar = amt > 0
                        ? juce::Rectangle<float> (cell.getX() + 3, midY - barH, cell.getWidth() - 6, barH)
                        : juce::Rectangle<float> (cell.getX() + 3, midY, cell.getWidth() - 6, barH);
                    g.setColour (amt > 0 ? juce::Colour (0xff00e8ff) : juce::Colour (0xffff2d9b));
                    g.fillRoundedRectangle (bar, 2.0f);
                    g.setColour (juce::Colour (0xff3a3050));
                    g.drawHorizontalLine ((int) midY, cell.getX(), cell.getRight());
                    g.setColour (juce::Colours::white.withAlpha (0.85f));
                    g.setFont (juce::FontOptions (8.0f));
                    g.drawText (juce::String (amt, 2), cell, juce::Justification::centred);
                }
                else
                {
                    g.setColour (juce::Colour (0xff2a1840));
                    g.drawRoundedRectangle (cell, 3.0f, 1.0f);
                }
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override { dragAt (e); }
    void mouseDrag (const juce::MouseEvent& e) override { dragAt (e); }

private:
    salek::ModMatrix& matrix;
    int dragSrc = -1, dragDst = -1;

    static std::array<salek::ModSource, 5> sources()
    {
        using S = salek::ModSource;
        return { S::LFO1, S::Env1, S::Macro1, S::Macro2, S::Macro3 };
    }
    static std::array<salek::ModDest, 8> dests()
    {
        using D = salek::ModDest;
        return { D::FilterCutoff, D::FilterReso, D::Osc1Level, D::Osc2Level,
                 D::Osc1Table, D::Osc1Warp, D::Fm2to1, D::Osc1Pitch };
    }

    void dragAt (const juce::MouseEvent& e)
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        r.removeFromTop (18.0f);
        auto grid = r.reduced (4.0f);
        const float labelW = 34.0f, labelH = 14.0f;
        const int nSrc = 5, nDst = 8;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;

        float gx = e.position.x - grid.getX() - labelW;
        float gy = e.position.y - grid.getY() - labelH;
        int d = (int) (gx / cellW);
        int s = (int) (gy / cellH);
        if (s < 0 || s >= nSrc || d < 0 || d >= nDst) return;

        auto cellY0 = grid.getY() + labelH + s * cellH + 2.0f;
        auto cellH2 = cellH - 4.0f;
        float rel = 1.0f - juce::jlimit (0.0f, 1.0f, (e.position.y - cellY0) / cellH2);
        float amt = juce::jlimit (-1.0f, 1.0f, (rel - 0.5f) * 2.0f);

        auto srcs = sources();
        auto dsts = dests();
        matrix.setAmount (srcs[(size_t) s], dsts[(size_t) d], amt);
        repaint();
    }
};

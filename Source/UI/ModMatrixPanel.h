#pragma once
#include <JuceHeader.h>
#include "../Modulation/ModMatrix.h"

class ModMatrixPanel : public juce::Component, private juce::Timer
{
public:
    explicit ModMatrixPanel (salek::ModMatrix& m) : matrix (m)
    {
        startTimerHz (15);
        matrix.addRoute (salek::ModMatrix::Source::LFO1, salek::ModMatrix::Dest::FilterCutoff, 0.0f);
    }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff0a0814));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (juce::Colour (0xff00f0ff).withAlpha (0.4f));
        g.drawRoundedRectangle (r, 8.0f, 1.0f);
        g.setColour (juce::Colour (0xffa080c0));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText ("MOD MATRIX  (click cells: 0 / 0.5 / 1 / -0.5 / -1)", r.removeFromTop (18.0f).reduced (6, 0), juce::Justification::centredLeft);

        const int nSrc = 5, nDst = 6;
        salek::ModMatrix::Source srcs[] = {
            salek::ModMatrix::Source::LFO1, salek::ModMatrix::Source::Env1,
            salek::ModMatrix::Source::Macro1, salek::ModMatrix::Source::Macro2,
            salek::ModMatrix::Source::Macro3
        };
        salek::ModMatrix::Dest dsts[] = {
            salek::ModMatrix::Dest::FilterCutoff, salek::ModMatrix::Dest::FilterReso,
            salek::ModMatrix::Dest::Osc1Level, salek::ModMatrix::Dest::Osc1Table,
            salek::ModMatrix::Dest::Fm2to1, salek::ModMatrix::Dest::Pitch
        };
        const char* sLab[] = { "LFO", "ENV", "M1", "M2", "M3" };
        const char* dLab[] = { "CUT", "RES", "O1LVL", "TABLE", "FM2", "PITCH" };

        auto grid = r.reduced (4.0f);
        const float labelW = 36.0f, labelH = 14.0f;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;

        g.setFont (juce::FontOptions (9.0f));
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
                auto cell = juce::Rectangle<float> (grid.getX() + labelW + d * cellW + 2, grid.getY() + labelH + s * cellH + 2, cellW - 4, cellH - 4);
                float amt = 0.0f;
                bool active = false;
                for (const auto& route : matrix.getRoutes())
                {
                    if (route.active && route.source == srcs[s] && route.dest == dsts[d])
                    { amt = route.amount; active = true; break; }
                }
                juce::Colour c = juce::Colour (0xff1a1528);
                if (active && std::abs (amt) > 0.01f)
                    c = amt > 0 ? juce::Colour (0xff00f0ff).withAlpha (0.25f + 0.6f * amt)
                                : juce::Colour (0xffff2d6a).withAlpha (0.25f + 0.6f * (-amt));
                g.setColour (c);
                g.fillRoundedRectangle (cell, 4.0f);
                g.setColour (juce::Colour (0xff3a3050));
                g.drawRoundedRectangle (cell, 4.0f, 1.0f);
                if (active && std::abs (amt) > 0.01f)
                {
                    g.setColour (juce::Colours::white);
                    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
                    g.drawText (juce::String (amt, 1), cell, juce::Justification::centred);
                }
            }
        }
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        const int nSrc = 5, nDst = 6;
        salek::ModMatrix::Source srcs[] = {
            salek::ModMatrix::Source::LFO1, salek::ModMatrix::Source::Env1,
            salek::ModMatrix::Source::Macro1, salek::ModMatrix::Source::Macro2,
            salek::ModMatrix::Source::Macro3
        };
        salek::ModMatrix::Dest dsts[] = {
            salek::ModMatrix::Dest::FilterCutoff, salek::ModMatrix::Dest::FilterReso,
            salek::ModMatrix::Dest::Osc1Level, salek::ModMatrix::Dest::Osc1Table,
            salek::ModMatrix::Dest::Fm2to1, salek::ModMatrix::Dest::Pitch
        };
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        r.removeFromTop (18.0f);
        auto grid = r.reduced (4.0f);
        const float labelW = 36.0f, labelH = 14.0f;
        const float cellW = (grid.getWidth() - labelW) / (float) nDst;
        const float cellH = (grid.getHeight() - labelH) / (float) nSrc;
        int d = (int) ((e.position.x - grid.getX() - labelW) / cellW);
        int s = (int) ((e.position.y - grid.getY() - labelH) / cellH);
        if (s < 0 || s >= nSrc || d < 0 || d >= nDst) return;
        float amt = 0.0f;
        for (const auto& route : matrix.getRoutes())
            if (route.active && route.source == srcs[s] && route.dest == dsts[d])
            { amt = route.amount; break; }
        const float steps[] = { 0.0f, 0.5f, 1.0f, -0.5f, -1.0f };
        int idx = 0;
        for (int i = 0; i < 5; ++i)
            if (std::abs (amt - steps[i]) < 0.05f) { idx = i; break; }
        idx = (idx + 1) % 5;
        matrix.addRoute (srcs[s], dsts[d], steps[idx]);
        repaint();
    }
    void timerCallback() override { repaint(); }
private:
    salek::ModMatrix& matrix;
};

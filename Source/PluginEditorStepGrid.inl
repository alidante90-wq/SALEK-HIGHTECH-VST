/** 16-step grid: click = toggle, drag Y = velocity, alt+drag = gate, shift = note offset, right-click = clear */
class StepGridComponent : public juce::Component, private juce::Timer
{
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq)
    {
        startTimerHz (30);
        clearBtn.setButtonText ("CLR");
        randBtn.setButtonText ("RND");
        fillBtn.setButtonText ("FILL");
        clearBtn.onClick = [this] { clearPattern(); };
        randBtn.onClick = [this] { randomPattern(); };
        fillBtn.onClick = [this] { fillPattern(); };
        for (auto* b : { &clearBtn, &randBtn, &fillBtn })
        {
            b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1030));
            b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff00e8ff));
            addAndMakeVisible (*b);
        }
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto bar = r.removeFromTop (26).reduced (4, 2);
        clearBtn.setBounds (bar.removeFromLeft (52).reduced (2));
        randBtn.setBounds (bar.removeFromLeft (52).reduced (2));
        fillBtn.setBounds (bar.removeFromLeft (52).reduced (2));
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff080414));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f));
        g.drawRoundedRectangle (r, 10.0f, 1.3f);

        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);

        const int n = salek::StepSequencer::NumSteps;
        const float gap = 3.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2 - 16.0f;
        const int play = sequencer.getCurrentStep();

        for (int g4 = 0; g4 < n; g4 += 4)
        {
            auto band = juce::Rectangle<float> (
                r.getX() + gap + g4 * (w + gap) - 1.0f, r.getY() + gap - 1.0f,
                4 * (w + gap) - gap + 2.0f, h + 2.0f);
            g.setColour (juce::Colour (0xff120820).withAlpha (0.5f));
            g.fillRoundedRectangle (band, 4.0f);
        }

        for (int i = 0; i < n; ++i)
        {
            auto cell = juce::Rectangle<float> (r.getX() + gap + i * (w + gap), r.getY() + gap, w, h);
            const auto& st = sequencer.getStep (i);
            bool on = st.active;
            bool isPlay = (i == play && sequencer.isEnabled());

            g.setColour (on ? juce::Colour (0xff1a1050) : juce::Colour (0xff0c0818));
            g.fillRoundedRectangle (cell, 4.0f);

            float velH = h * juce::jlimit (0.06f, 1.0f, st.velocity);
            auto velBar = juce::Rectangle<float> (cell.getX() + 3, cell.getBottom() - velH, cell.getWidth() - 6, velH - 2);
            if (on)
            {
                juce::Colour vc = isPlay ? juce::Colour (0xffff2d9b) : juce::Colour (0xff00e8ff);
                juce::ColourGradient gr (vc, velBar.getCentreX(), velBar.getY(),
                                         vc.darker (0.4f), velBar.getCentreX(), velBar.getBottom(), false);
                g.setGradientFill (gr);
                g.fillRoundedRectangle (velBar, 3.0f);
            }

            if (on)
            {
                float gateW = cell.getWidth() * juce::jlimit (0.1f, 1.0f, st.gate);
                g.setColour (juce::Colour (0xffffd700).withAlpha (0.7f));
                g.fillRect (cell.getX() + 2, cell.getY() + 2, gateW - 4, 3.0f);
            }

            if (on && st.noteOffset != 0)
            {
                g.setColour (juce::Colours::white.withAlpha (0.7f));
                g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
                g.drawText ((st.noteOffset > 0 ? "+" : "") + juce::String (st.noteOffset),
                            cell.reduced (1, 6), juce::Justification::centredTop);
            }

            if (isPlay)
            {
                g.setColour (juce::Colour (0xffffd700).withAlpha (0.9f));
                g.drawRoundedRectangle (cell.expanded (1.5f), 5.0f, 2.2f);
                g.setColour (juce::Colour (0xffff2d9b).withAlpha (0.2f));
                g.fillRoundedRectangle (cell.expanded (3.0f), 6.0f);
            }
            else
            {
                g.setColour (on ? juce::Colour (0xff00e8ff).withAlpha (0.45f) : juce::Colour (0xff2a1840));
                g.drawRoundedRectangle (cell, 4.0f, 1.0f);
            }

            g.setColour (isPlay ? juce::Colour (0xffffd700) : juce::Colours::white.withAlpha (0.35f));
            g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
            g.drawText (juce::String (i + 1),
                        (int) (r.getX() + gap + i * (w + gap)), (int) (r.getBottom() - 14),
                        (int) w, 12, juce::Justification::centred);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        int idx = hitStep (e.position);
        if (idx < 0) return;
        auto& st = sequencer.getStep (idx);

        if (e.mods.isRightButtonDown())
        {
            st.active = false; st.velocity = 0.8f; st.gate = 0.6f; st.noteOffset = 0;
            repaint(); return;
        }
        if (e.mods.isShiftDown())
        {
            static const int offs[] = { 0, 12, -12, 7, -7, 5, -5 };
            int cur = 0;
            for (int i = 0; i < 7; ++i) if (st.noteOffset == offs[i]) { cur = i; break; }
            st.noteOffset = offs[(cur + 1) % 7];
            st.active = true;
            repaint(); return;
        }
        st.active = ! st.active;
        if (st.active && st.velocity < 0.05f) st.velocity = 0.8f;
        repaint();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        int idx = hitStep (e.position);
        if (idx < 0) return;
        auto& st = sequencer.getStep (idx);
        auto r = getLocalBounds().toFloat();
        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);
        const float h = r.getHeight() - 8.0f - 16.0f;
        float relY = 1.0f - juce::jlimit (0.f, 1.f, (e.position.y - r.getY() - 4.0f) / h);
        if (e.mods.isAltDown())
        {
            st.gate = juce::jlimit (0.05f, 1.0f, relY);
            st.active = true;
        }
        else
        {
            st.velocity = juce::jlimit (0.05f, 1.0f, relY);
            st.active = true;
        }
        repaint();
    }

    void timerCallback() override { repaint(); }

private:
    salek::StepSequencer& sequencer;
    juce::TextButton clearBtn, randBtn, fillBtn;

    int hitStep (juce::Point<float> pos) const
    {
        auto r = getLocalBounds().toFloat();
        r.removeFromTop (28.0f);
        r = r.reduced (4.0f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 3.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        int idx = (int) ((pos.x - r.getX() - gap) / (w + gap));
        if (idx < 0 || idx >= n) return -1;
        if (pos.y < r.getY() || pos.y > r.getBottom() - 14) return -1;
        return idx;
    }

    void clearPattern()
    {
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = false; st.velocity = 0.8f; st.gate = 0.6f; st.noteOffset = 0;
        }
        repaint();
    }

    void randomPattern()
    {
        juce::Random rng;
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = rng.nextFloat() > 0.45f;
            st.velocity = 0.4f + rng.nextFloat() * 0.6f;
            st.gate = 0.3f + rng.nextFloat() * 0.6f;
            st.noteOffset = 0;
            if (rng.nextFloat() > 0.85f) st.noteOffset = (rng.nextBool() ? 12 : -12);
        }
        repaint();
    }

    void fillPattern()
    {
        for (int i = 0; i < salek::StepSequencer::NumSteps; ++i)
        {
            auto& st = sequencer.getStep (i);
            st.active = true; st.velocity = 0.85f; st.gate = 0.6f;
        }
        repaint();
    }
};

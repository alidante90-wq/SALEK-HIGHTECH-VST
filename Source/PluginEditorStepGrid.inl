class StepGridComponent : public juce::Component, private juce::Timer {
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq) { startTimerHz (24); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (3.0f);
        g.setColour (juce::Colour (0xff0a0614));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.35f));
        g.drawRoundedRectangle (r, 10.0f, 1.2f);

        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.0f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2 - 18.0f;
        const int play = sequencer.getCurrentStep();

        for (int i = 0; i < n; ++i)
        {
            auto cell = juce::Rectangle<float> (r.getX() + gap + i * (w + gap), r.getY() + gap, w, h);
            const auto& st = sequencer.getStep (i);
            bool on = st.active;
            bool isPlay = (i == play);

            g.setColour (on ? juce::Colour (0xff1a1040) : juce::Colour (0xff0e0a1c));
            g.fillRoundedRectangle (cell, 4.0f);

            float velH = h * juce::jlimit (0.08f, 1.0f, st.velocity);
            auto bar = cell.removeFromBottom (velH).reduced (3.0f, 2.0f);
            if (on)
            {
                juce::Colour vc = isPlay ? juce::Colour (0xffff2d9b) : juce::Colour (0xff00e8ff);
                g.setColour (vc.withAlpha (0.85f));
                g.fillRoundedRectangle (bar, 3.0f);
                if (isPlay)
                {
                    g.setColour (vc.withAlpha (0.35f));
                    g.fillRoundedRectangle (cell.expanded (2.0f), 5.0f);
                }
            }
            else
            {
                g.setColour (juce::Colour (0xff1a1028));
                g.fillRoundedRectangle (bar.withHeight (4.0f), 2.0f);
            }

            g.setColour (isPlay ? juce::Colour (0xffffd700) : (on ? juce::Colour (0xff00e8ff).withAlpha (0.5f) : juce::Colour (0xff2a1840)));
            g.drawRoundedRectangle (juce::Rectangle<float> (r.getX() + gap + i * (w + gap), r.getY() + gap, w, h), 4.0f, isPlay ? 2.0f : 1.0f);

            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.setFont (juce::FontOptions (9.0f));
            g.drawText (juce::String (i + 1), (int) (r.getX() + gap + i * (w + gap)), (int) (r.getBottom() - 16), (int) w, 14, juce::Justification::centred);
        }
    }
    void mouseDown (const juce::MouseEvent& e) override {
        auto r = getLocalBounds().toFloat().reduced (3.f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        int idx = (int) ((e.position.x - r.getX() - gap) / (w + gap));
        if (idx >= 0 && idx < n)
        {
            auto& st = sequencer.getStep (idx);
            st.active = ! st.active;
            repaint();
        }
    }
    void mouseDrag (const juce::MouseEvent& e) override {
        auto r = getLocalBounds().toFloat().reduced (3.f);
        const int n = salek::StepSequencer::NumSteps;
        const float gap = 4.f;
        const float w = (r.getWidth() - gap * (n + 1)) / (float) n;
        const float h = r.getHeight() - gap * 2 - 18.0f;
        int idx = (int) ((e.position.x - r.getX() - gap) / (w + gap));
        if (idx >= 0 && idx < n)
        {
            float relY = 1.0f - juce::jlimit (0.f, 1.f, (e.position.y - r.getY() - gap) / h);
            sequencer.getStep (idx).velocity = relY;
            sequencer.getStep (idx).active = true;
            repaint();
        }
    }
    void timerCallback() override { repaint(); }
private:
    salek::StepSequencer& sequencer;
};

#pragma once
#include <JuceHeader.h>

class OpenGLGridBackdrop : public juce::Component,
                           private juce::OpenGLRenderer,
                           private juce::Timer
{
public:
    OpenGLGridBackdrop()
    {
        openGLContext.setOpenGLVersionRequired (juce::OpenGLContext::openGL3_2);
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
        openGLContext.setContinuousRepainting (true);
        startTimerHz (30);
    }
    ~OpenGLGridBackdrop() override
    {
        stopTimer();
        openGLContext.detach();
    }
    void setPulse (float p) { pulse = juce::jlimit (0.f, 1.f, p); }
    void setAccent (juce::Colour c) { accent = c; }

    void newOpenGLContextCreated() override { glReady = true; }
    void openGLContextClosing() override { glReady = false; }
    void renderOpenGL() override
    {
        juce::OpenGLHelpers::clear (juce::Colour::fromFloatRGBA (0.02f, 0.01f, 0.04f, 1.0f));
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float p = pulse;
        const float yHorizon = bounds.getHeight() * 0.42f;
        g.setColour (accent.withAlpha (0.05f + p * 0.12f));
        for (int i = 0; i < 16; ++i)
        {
            float t = (float) i / 15.0f;
            float y = yHorizon + t * t * (bounds.getHeight() - yHorizon);
            g.drawLine (0.0f, y, bounds.getWidth(), y, 1.0f);
        }
        const float cx = bounds.getCentreX() + std::sin (phase * 0.3f) * 20.0f * p;
        for (int i = -12; i <= 12; ++i)
        {
            float x1 = cx + (float) i * 8.0f;
            float x2 = cx + (float) i * 70.0f;
            g.setColour (accent.withAlpha (0.04f + p * 0.08f));
            g.drawLine (x1, yHorizon, x2, bounds.getHeight(), 1.0f);
        }
        for (int i = 0; i < 8; ++i)
        {
            float t = std::fmod (phase * 0.15f + (float) i * 0.12f, 1.0f);
            float depth = t;
            float sz = 4.0f + (1.0f - depth) * 18.0f;
            float x = bounds.getCentreX() + std::sin (phase + i) * bounds.getWidth() * 0.35f * depth;
            float y = yHorizon + depth * depth * (bounds.getHeight() - yHorizon - 40.0f);
            g.setColour (accent.withAlpha (0.15f + p * 0.25f));
            g.drawRect (x - sz * 0.5f, y - sz * 0.5f, sz, sz, 1.0f);
        }
        if (glReady)
        {
            g.setColour (accent.withAlpha (0.5f));
            g.setFont (juce::FontOptions (9.0f));
            g.drawText ("GL", bounds.getRight() - 28, 4, 24, 12, juce::Justification::centredRight);
        }
    }
    void timerCallback() override
    {
        phase += 0.04f + pulse * 0.06f;
        repaint();
    }
private:
    juce::OpenGLContext openGLContext;
    bool glReady = false;
    float phase = 0.f, pulse = 0.f;
    juce::Colour accent { 0xff00f0ff };
};

#include "PluginEditor.h"
#include "AssetsData.h"
#include <cmath>

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    // Full-window OpenGL is opaque black on Windows — disable backdrop
    glBackdrop = std::make_unique<OpenGLGridBackdrop>();
    glBackdrop->setVisible (false);
    glBackdrop->setInterceptsMouseClicks (false, false);
    addChildComponent (*glBackdrop);

    sonicCore = std::make_unique<SonicCoreGL> (processor.getVisualFifo());
    sonicCore->setOpaque (false);
    sonicCore->setInterceptsMouseClicks (false, false);
    addAndMakeVisible (*sonicCore);
    sonicCore->toBack();
    addAndMakeVisible (keyboard);
    vblank = juce::VBlankAttachment (this, [this] (double) {
        animPhase += 0.025f;
        if (sonicCore != nullptr)
        {
            sonicCore->setPulse (processor.getOutputPeak());
            const int th = themeBox.getSelectedId();
            juce::Colour a1 (0xff00e8ff), a2 (0xffff2d9b);
            if (th == 2) { a1 = juce::Colour (0xff39ff14); a2 = juce::Colour (0xffc0ff00); }
            if (th == 3) { a1 = juce::Colour (0xff4fc3f7); a2 = juce::Colour (0xff7c4dff); }
            sonicCore->setAccent (a1);
            sonicCore->setAccent2 (a2);
        }
        if (glBackdrop != nullptr)
            glBackdrop->setPulse (processor.getOutputPeak());
        repaint();
    });
    startTimerHz (15);

    setLookAndFeel (&lnf);
    logoImg   = SalekAssets::loadLogo();
    heroImg   = SalekAssets::loadToronowla();
    faceImg   = SalekAssets::loadFace();
    lianImg   = SalekAssets::loadLian();
    cyanImg   = SalekAssets::loadCyanGirl();
    setSize (1280, 820);
    setResizable (true, true);
    setResizeLimits (1020, 700, 1700, 1100);
    title.setText ("SALEK HIGHTECH", juce::dontSendNotification);
    addAndMakeVisible (title);
    tagline.setText ("ALIEN", juce::dontSendNotification);
    addAndMakeVisible (tagline);

#include "PluginEditorCtor.inl"
}

SalekHightechAudioProcessorEditor::~SalekHightechAudioProcessorEditor()
{
    stopTimer();
    vblank = {};
    sonicCore.reset();
    glBackdrop.reset();
    setLookAndFeel (nullptr);
}

void SalekHightechAudioProcessorEditor::timerCallback()
{
    static int ticks = 0;
    if (++ticks < 40) resized();
}

int SalekHightechAudioProcessorEditor::getNumRows() { return processor.getNumPrograms(); }
void SalekHightechAudioProcessorEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (selected)
        g.fillAll (juce::Colour (0xff2a1a4a));
    g.setColour (selected ? juce::Colour (0xff00e8ff) : juce::Colours::white.withAlpha (0.85f));
    g.setFont (12.0f);
    g.drawText (processor.getProgramName (row), 6, 0, width - 8, height, juce::Justification::centredLeft);
}
void SalekHightechAudioProcessorEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    processor.setCurrentProgram (row);
}

void SalekHightechAudioProcessorEditor::paint (juce::Graphics& g)
{
#include "PluginEditorPaint.inl"
}

SalekHightechAudioProcessorEditor::Knob& SalekHightechAudioProcessorEditor::addKnob (
    juce::Component& parent, const char* id, const char* label, juce::Colour c)
{
#include "PluginEditorAddKnob.inl"
}

void SalekHightechAudioProcessorEditor::resized()
{
    if (glBackdrop != nullptr)
        glBackdrop->setBounds (0, 0, 1, 1);

    auto full = getLocalBounds().reduced (6);
    if (sonicCore != nullptr)
    {
        auto core = full;
        core.removeFromTop (juce::jmax (0, core.getHeight() - 120));
        core = core.withSizeKeepingCentre (juce::jmin (480, core.getWidth()), 100);
        core.translate (0, -82);
        sonicCore->setBounds (core);
        sonicCore->toBack();
    }
#include "PluginEditorResized.inl"
}

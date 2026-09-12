#include "PluginEditor.h"
#include "AssetsData.h"
#include <cmath>

SalekHightechAudioProcessorEditor::SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    keyboard.setAvailableRange (21, 108);
    keyboard.setOctaveForMiddleC (4);
    // OpenGL full-window backdrop is OPAQUE on Windows — keep disabled so UI stays visible
    glBackdrop = std::make_unique<OpenGLGridBackdrop>();
    glBackdrop->setVisible (false);
    glBackdrop->setInterceptsMouseClicks (false, false);
    addChildComponent (*glBackdrop); // not visible

    // Visualizer: small, behind UI, non-interactive (neon only)
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
    addAndMakeVisible (scope);
    wtDisplay = std::make_unique<salek::WavetableDisplay>();
    addAndMakeVisible (*wtDisplay);
    tabs.addTab ("MAIN", juce::Colours::transparentBlack, &mainTab, false);
    tabs.addTab ("MOD", juce::Colours::transparentBlack, &modTab, false);
    tabs.addTab ("FX", juce::Colours::transparentBlack, &fxTab, false);
    tabs.addTab ("SEQ", juce::Colours::transparentBlack, &seqTab, false);
    addAndMakeVisible (tabs);
    tabs.setTabBarDepth (28);
    tabs.setOpaque (true);

    filterDisplay = std::make_unique<salek::FilterResponseDisplay>();
    adsrDisplay = std::make_unique<salek::ADSRDisplay>();
    lfoDisplay = std::make_unique<salek::LFODisplay>();
    matrixPanel = std::make_unique<salek::ModMatrixPanel>(processor.getModMatrix());
    stepGrid = std::make_unique<salek::StepGridComponent>(processor.getStepSequencer());

    {
        mainTab.addAndMakeVisible (oscTab);
        mainTab.addAndMakeVisible (filterTab);
        mainTab.addAndMakeVisible (envTab);
        mainTab.addAndMakeVisible (presetTab);
        if (filterDisplay != nullptr) filterTab.addAndMakeVisible (*filterDisplay);
        if (adsrDisplay != nullptr) envTab.addAndMakeVisible (*adsrDisplay);

        auto addK = [this] (juce::Component& parent, const char* id, const char* label, float minV, float maxV, float def) {
            auto* k = knobs.add (new KnobWithLabel());
            k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
            k->s.setRange (minV, maxV);
            k->s.setValue (def);
            k->name.setText (label, juce::dontSendNotification);
            k->name.setJustificationType (juce::Justification::centred);
            parent.addAndMakeVisible (k->s);
            parent.addAndMakeVisible (k->name);
            attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.getAPVTS(), id, k->s));
            return k;
        };

        // OSC row
        addK (oscTab, "osc1_level", "O1 LVL", 0, 1, 0.7f);
        addK (oscTab, "osc1_table", "O1 WT", 0, 1, 0);
        addK (oscTab, "osc1_warp", "O1 WRP", 0, 1, 0);
        addK (oscTab, "osc1_fold", "O1 FLD", 0, 1, 0);
        addK (oscTab, "osc2_level", "O2 LVL", 0, 1, 0.5f);
        addK (oscTab, "osc2_table", "O2 WT", 0, 1, 0.33f);
        addK (oscTab, "osc3_level", "O3 LVL", 0, 1, 0.4f);
        addK (oscTab, "unison_voices", "UNI", 1, 7, 1);
        addK (oscTab, "unison_detune", "DET", 0, 50, 12);
        addK (oscTab, "fm_2to1", "FM21", 0, 1, 0);

        addK (filterTab, "filter_cutoff", "CUT", 20, 20000, 8000);
        addK (filterTab, "filter_reso", "RES", 0, 1, 0.25f);
        addK (filterTab, "filter_drive", "DRV", 0, 1, 0);
        addK (filterTab, "filter_env", "ENV", 0, 1, 0.4f);

        addK (envTab, "amp_attack", "A", 0.001f, 5, 0.01f);
        addK (envTab, "amp_decay", "D", 0.001f, 5, 0.15f);
        addK (envTab, "amp_sustain", "S", 0, 1, 0.75f);
        addK (envTab, "amp_release", "R", 0.001f, 8, 0.25f);

        presetTab.addAndMakeVisible (presetList);
        presetTab.addAndMakeVisible (prevPreset);
        presetTab.addAndMakeVisible (nextPreset);
        presetTab.addAndMakeVisible (initBtn);
        presetTab.addAndMakeVisible (presetLabel);
        presetList.setModel (this);
        prevPreset.setButtonText ("<");
        nextPreset.setButtonText (">");
        initBtn.setButtonText ("INIT");
        prevPreset.onClick = [this] { int i = processor.getCurrentProgram(); if (i > 0) processor.setCurrentProgram (i - 1); presetList.selectRow (processor.getCurrentProgram()); };
        nextPreset.onClick = [this] { int i = processor.getCurrentProgram(); if (i + 1 < processor.getNumPrograms()) processor.setCurrentProgram (i + 1); presetList.selectRow (processor.getCurrentProgram()); };
        initBtn.onClick = [this] { processor.setCurrentProgram (0); presetList.selectRow (0); };
        presetLabel.setText ("PRESETS", juce::dontSendNotification);
    }

    {
        auto addK = [this] (juce::Component& parent, const char* id, const char* label, float minV, float maxV, float def) {
            auto* k = knobs.add (new KnobWithLabel());
            k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
            k->s.setRange (minV, maxV);
            k->s.setValue (def);
            k->name.setText (label, juce::dontSendNotification);
            k->name.setJustificationType (juce::Justification::centred);
            parent.addAndMakeVisible (k->s);
            parent.addAndMakeVisible (k->name);
            attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.getAPVTS(), id, k->s));
            return k;
        };
        if (lfoDisplay != nullptr) modTab.addAndMakeVisible (*lfoDisplay);
        if (matrixPanel != nullptr) modTab.addAndMakeVisible (*matrixPanel);
        addK (modTab, "lfo_rate", "RATE", 0.01f, 40, 2);
        addK (modTab, "lfo_amount", "AMT", 0, 1, 0);
        addK (modTab, "macro1", "M1", 0, 1, 0);
        addK (modTab, "macro2", "M2", 0, 1, 0);
        addK (modTab, "macro3", "M3", 0, 1, 0);
        addK (modTab, "macro4", "M4", 0, 1, 0);
    }

    {
        auto addK = [this] (juce::Component& parent, const char* id, const char* label, float minV, float maxV, float def) {
            auto* k = knobs.add (new KnobWithLabel());
            k->s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k->s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
            k->s.setRange (minV, maxV);
            k->s.setValue (def);
            k->name.setText (label, juce::dontSendNotification);
            k->name.setJustificationType (juce::Justification::centred);
            parent.addAndMakeVisible (k->s);
            parent.addAndMakeVisible (k->name);
            attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.getAPVTS(), id, k->s));
            return k;
        };
        addK (fxTab, "delay_mix", "DLY", 0, 1, 0);
        addK (fxTab, "delay_time", "TIME", 50, 800, 280);
        addK (fxTab, "delay_fb", "FB", 0, 0.95f, 0.35f);
        addK (fxTab, "chorus_mix", "CHO", 0, 1, 0);
        addK (fxTab, "reverb_mix", "REV", 0, 1, 0);
        addK (fxTab, "reverb_size", "SIZE", 0, 1, 0.5f);
        addK (fxTab, "master_drive", "DRV", 0, 1, 0);
        addK (fxTab, "master_gain", "GAIN", 0, 1, 0.8f);
    }

    {
        seqTab.addAndMakeVisible (arpOn);
        seqTab.addAndMakeVisible (seqOn);
        arpOn.setButtonText ("ARP");
        seqOn.setButtonText ("SEQ");
        arpAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "arp_on", arpOn);
        seqAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.getAPVTS(), "seq_on", seqOn);
        if (stepGrid != nullptr) seqTab.addAndMakeVisible (*stepGrid);
    }

    themeBox.addItem ("CYBER", 1);
    themeBox.addItem ("ACID", 2);
    themeBox.addItem ("NEON", 3);
    themeBox.setSelectedId (1);
    addAndMakeVisible (themeBox);

    // Keep visualizer behind all controls
    if (sonicCore != nullptr)
        sonicCore->toBack();
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

void SalekHightechAudioProcessorEditor::resized()
{
    // Keep OpenGL backdrop off-screen / unused (opaque black on Windows)
    if (glBackdrop != nullptr)
        glBackdrop->setBounds (0, 0, 1, 1);

    auto full = getLocalBounds().reduced (6);
    // Neon visualizer: strip above keyboard — never covers knobs/tabs
    if (sonicCore != nullptr)
    {
        auto core = full;
        core.removeFromTop (juce::jmax (0, core.getHeight() - 120));
        core = core.withSizeKeepingCentre (juce::jmin (480, core.getWidth()), 100);
        core.translate (0, -82);
        sonicCore->setBounds (core);
        sonicCore->toBack();
    }
    {
        auto kbArea = full.removeFromBottom (78).reduced (2, 2);
        const int whiteKeys = 52;
        int keyW = juce::jmax (12, kbArea.getWidth() / whiteKeys);
        keyboard.setKeyWidth ((float) keyW);
        keyboard.setBounds (kbArea);
    }
    full.removeFromBottom (4);

    auto a = full;
    a.removeFromLeft (182);
    title.setVisible (false);
    tagline.setVisible (false);

    auto header = a.removeFromTop (44);
    themeBox.setBounds (header.removeFromRight (150).reduced (2));
    scope.setBounds (header.removeFromRight (130).reduced (3));
    if (wtDisplay != nullptr)
        wtDisplay->setBounds (header.removeFromRight (260).reduced (2));

    tabs.setBounds (a);
    tabs.toFront (false);
    keyboard.toFront (false);

    // MAIN tab layout
    {
        auto b = mainTab.getLocalBounds().reduced (4);
        presetTab.setBounds (b.removeFromLeft (220));
        envTab.setBounds (b.removeFromBottom (150));
        filterTab.setBounds (b.removeFromRight (280));
        oscTab.setBounds (b);

        // preset column
        {
            auto r = presetTab.getLocalBounds().reduced (4);
            auto top = r.removeFromTop (28);
            prevPreset.setBounds (top.removeFromLeft (36).reduced (2));
            nextPreset.setBounds (top.removeFromLeft (36).reduced (2));
            initBtn.setBounds (top.removeFromLeft (56).reduced (2));
            presetLabel.setBounds (r.removeFromTop (20));
            presetList.setBounds (r.reduced (2));
        }

        auto layoutKnobs = [] (juce::Component& parent, juce::OwnedArray<KnobWithLabel>& all, int start, int count)
        {
            auto r = parent.getLocalBounds().reduced (6);
            if (count <= 0) return;
            int cols = juce::jmin (count, 5);
            int rows = (count + cols - 1) / cols;
            int cellW = r.getWidth() / cols;
            int cellH = r.getHeight() / juce::jmax (1, rows);
            for (int i = 0; i < count; ++i)
            {
                int idx = start + i;
                if (idx >= all.size()) break;
                auto* k = all[idx];
                int c = i % cols, row = i / cols;
                auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (4);
                k->name.setBounds (cell.removeFromBottom (16));
                k->s.setBounds (cell);
            }
        };

        // Approximate knob indices: first 10 osc, next 4 filter, next 4 env
        layoutKnobs (oscTab, knobs, 0, 10);
        layoutKnobs (filterTab, knobs, 10, 4);
        layoutKnobs (envTab, knobs, 14, 4);
        if (filterDisplay != nullptr)
            filterDisplay->setBounds (filterTab.getLocalBounds().removeFromTop (90).reduced (4));
        if (adsrDisplay != nullptr)
            adsrDisplay->setBounds (envTab.getLocalBounds().removeFromTop (70).reduced (4));
    }

    // MOD
    {
        auto r = modTab.getLocalBounds().reduced (4);
        if (lfoDisplay != nullptr)
            lfoDisplay->setBounds (r.removeFromTop (80).reduced (2));
        if (matrixPanel != nullptr)
            matrixPanel->setBounds (r.removeFromLeft (280).reduced (2));
        // remaining knobs 18..23 roughly
        int x = r.getX(), y = r.getY(), w = 90, h = 100;
        for (int i = 18; i < juce::jmin (24, knobs.size()); ++i)
        {
            auto* k = knobs[i];
            k->s.setBounds (x, y, w, h - 18);
            k->name.setBounds (x, y + h - 18, w, 16);
            x += w;
            if (x + w > r.getRight()) { x = r.getX(); y += h; }
        }
    }

    // FX knobs 24..31
    {
        auto r = fxTab.getLocalBounds().reduced (8);
        int cols = 4, i0 = 24;
        int n = juce::jmin (8, knobs.size() - i0);
        int cellW = r.getWidth() / cols;
        int cellH = 110;
        for (int i = 0; i < n; ++i)
        {
            auto* k = knobs[i0 + i];
            int c = i % cols, row = i / cols;
            auto cell = juce::Rectangle<int> (r.getX() + c * cellW, r.getY() + row * cellH, cellW, cellH).reduced (4);
            k->name.setBounds (cell.removeFromBottom (16));
            k->s.setBounds (cell);
        }
    }

    // SEQ
    {
        auto bounds = seqTab.getLocalBounds().reduced (4);
        auto top = bounds.removeFromTop (28);
        arpOn.setBounds (top.removeFromLeft (80).reduced (2));
        seqOn.setBounds (top.removeFromLeft (80).reduced (2));
        if (stepGrid != nullptr) stepGrid->setBounds (bounds.reduced (4));
    }
}

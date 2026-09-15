#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/ModMatrixPanel.h"
#include "UI/OpenGLGridBackdrop.h"
#include "UI/SonicCoreGL.h"
#include "UI/MagicPad.h"

#include "PluginEditorLookAndFeel.inl"
#include "PluginEditorFxLfo.inl"

class WavetableDisplay : public juce::Component, private juce::Timer {
public:
    explicit WavetableDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (24); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void WavetableDisplay::paint (juce::Graphics& g) {
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0c0818)); g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0xff00e8ff).withAlpha (0.4f)); g.drawRoundedRectangle (r, 6.0f, 1.f);
    auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
    float pos=gval("osc1_table",0.f);
    juce::Path wave; const int N=64;
    for(int i=0;i<N;++i){ float t=(float)i/(N-1); float y=std::sin(t*juce::MathConstants<float>::twoPi*(1.f+pos*4.f));
        float px=r.getX()+8+t*(r.getWidth()-16); float py=r.getCentreY()-y*(r.getHeight()*0.35f);
        if(i==0)wave.startNewSubPath(px,py); else wave.lineTo(px,py);}
    g.setColour(juce::Colour(0xff00e8ff)); g.strokePath(wave, juce::PathStrokeType(1.5f));
}

#include "PluginEditorVisualizers.inl"

class AdsrDisplay : public juce::Component, private juce::Timer {
public:
    explicit AdsrDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (20); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void AdsrDisplay::paint (juce::Graphics& g) {
    auto r=getLocalBounds().toFloat().reduced(2.f);
    g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,6.f);
    g.setColour(juce::Colour(0xff00e8ff).withAlpha(0.35f)); g.drawRoundedRectangle(r,6.f,1.f);
    auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
    float a=gval("amp_attack",0.01f), d=gval("amp_decay",0.2f), s=gval("amp_sustain",0.7f), rel=gval("amp_release",0.3f);
    float sum=a+d+0.4f+rel; auto plot=r.reduced(8.f,6.f);
    juce::Path env;
    float x0=plot.getX(), y0=plot.getBottom();
    float x1=x0+plot.getWidth()*(a/sum), y1=plot.getY();
    float x2=x1+plot.getWidth()*(d/sum), y2=plot.getY()+plot.getHeight()*(1.f-s);
    float x3=x2+plot.getWidth()*0.35f, y3=y2;
    float x4=plot.getRight(), y4=plot.getBottom();
    env.startNewSubPath(x0,y0); env.lineTo(x1,y1); env.lineTo(x2,y2); env.lineTo(x3,y3); env.lineTo(x4,y4);
    g.setColour(juce::Colour(0xff00e8ff)); g.strokePath(env, juce::PathStrokeType(1.6f));
}

class FilterCurveDisplay : public juce::Component, private juce::Timer {
public:
    explicit FilterCurveDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (24); }
    void paint (juce::Graphics& g) override;
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};
inline void FilterCurveDisplay::paint (juce::Graphics& g) {
    auto r=getLocalBounds().toFloat().reduced(2.f);
    g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,6.f);
    g.setColour(juce::Colour(0xffff2d9b).withAlpha(0.4f)); g.drawRoundedRectangle(r,6.f,1.f);
    auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
    float cut=gval("filter_cutoff",1000.f); float reso=gval("filter_reso",0.3f);
    float norm=juce::jlimit(0.f,1.f, std::log10(juce::jmax(20.f,cut)/20.f)/3.f);
    juce::Path curve; auto plot=r.reduced(6.f,4.f);
    for(int i=0;i<80;++i){ float t=(float)i/79.f; float x=plot.getX()+t*plot.getWidth();
        float dd=t-norm; float y=plot.getBottom()-4.f;
        if(t<norm) y=plot.getY()+plot.getHeight()*0.25f;
        else y=plot.getY()+plot.getHeight()*0.25f + juce::jmin(plot.getHeight()*0.7f, dd*dd*800.f*(1.f-reso*0.5f));
        if(i==0)curve.startNewSubPath(x,y); else curve.lineTo(x,y);}
    g.setColour(juce::Colour(0xffff2d9b)); g.strokePath(curve, juce::PathStrokeType(1.8f));
}

class LfoDisplay : public juce::Component, private juce::Timer {
public:
    explicit LfoDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (30); }
    void paint (juce::Graphics& g) override {
        auto bounds = getLocalBounds().toFloat().reduced (2.f);
        g.setColour (juce::Colour (0xff0c0818)); g.fillRoundedRectangle (bounds, 8.f);
        g.setColour (juce::Colour (0xff66ff99).withAlpha (0.4f)); g.drawRoundedRectangle (bounds, 8.f, 1.f);
        auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
        const char* rates[] = {"lfo_rate","lfo2_rate","lfo3_rate"};
        const char* amts[]  = {"lfo_amount","lfo2_amount","lfo3_amount"};
        const char* waves[] = {"lfo_wave","lfo2_wave","lfo3_wave"};
        juce::Colour cols[3] = { juce::Colour(0xff00e8ff), juce::Colour(0xffff2d9b), juce::Colour(0xff39ff14) };
        const char* labels[3] = { "LFO1", "LFO2", "LFO3" };
        float w = bounds.getWidth() / 3.f;
        for (int L = 0; L < 3; ++L)
        {
            auto r = bounds.withX (bounds.getX() + L * w).withWidth (w).reduced (4.f, 3.f);
            g.setColour (cols[L].withAlpha (0.25f)); g.drawRoundedRectangle (r, 6.f, 1.f);
            float rate = gval (rates[L], 1.f); int wave = (int) gval (waves[L], 0.f); float amt = gval (amts[L], 0.5f);
            g.setColour (cols[L]); g.setFont (juce::FontOptions (10.f, juce::Font::bold));
            g.drawText (juce::String (labels[L]) + "  " + juce::String (rate, 2) + "Hz",
                        r.removeFromTop (14).toNearestInt(), juce::Justification::centred);
            auto plot = r.reduced (2.f, 2.f);
            juce::Path curve; const int N = 64;
            float spd = juce::jmax (0.05f, rate) * 0.15f;
            for (int i = 0; i < N; ++i)
            {
                float t = (float) i / (N - 1);
                float ph = t * juce::MathConstants<float>::twoPi * 2.f + phase * spd * (1.f + L * 0.3f);
                float y = 0.f;
                if (wave == 0) y = std::sin (ph);
                else if (wave == 1) y = 1.f - 4.f * std::abs (std::fmod (ph / juce::MathConstants<float>::twoPi + 0.25f, 1.f) - 0.5f);
                else if (wave == 2) y = 2.f * (ph / juce::MathConstants<float>::twoPi - std::floor (ph / juce::MathConstants<float>::twoPi + 0.5f));
                else if (wave == 3) y = (std::sin (ph) >= 0.f ? 1.f : -1.f);
                else y = (std::sin (ph * 0.37f) > 0.f ? 1.f : -1.f);
                y *= (0.25f + 0.75f * amt);
                float px = plot.getX() + t * plot.getWidth();
                float py = plot.getCentreY() - y * plot.getHeight() * 0.4f;
                if (i == 0) curve.startNewSubPath (px, py); else curve.lineTo (px, py);
            }
            g.setColour (cols[L]); g.strokePath (curve, juce::PathStrokeType (1.6f));
        }
    }
    void timerCallback() override { phase += 0.12f; repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts; float phase = 0.f;
};

#include "PluginEditorStepGrid.inl"

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::Timer,
                                          public juce::ListBoxModel
{
public:
    explicit SalekHightechAudioProcessorEditor (SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
    SpectrumDisplay spectrum;
    std::unique_ptr<WavetableDisplay> wtDisplay;
    std::unique_ptr<AdsrDisplay> adsrDisplay;
    std::unique_ptr<FilterCurveDisplay> filterDisplay;
    std::unique_ptr<LfoDisplay> lfoDisplay;
    std::unique_ptr<ModMatrixPanel> matrixPanel;
    std::unique_ptr<OpenGLGridBackdrop> glBackdrop;
    std::unique_ptr<SonicCoreGL> sonicCore;
    juce::VBlankAttachment vblank;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    struct Knob { juce::Slider s; juce::Label name; };
    std::vector<std::unique_ptr<Knob>> knobs;
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    juce::Component mainTab, oscTab, filterTab, envTab, modTab, lfoTab, fxTab, magicTab, seqTab, presetTab, modularTab;
    juce::ComboBox themeBox;
    juce::ComboBox presetFilterBox;
    juce::ComboBox filterMode, lfoWave;
    juce::ToggleButton arpOn { "ARP ON" }, seqOn { "SEQ ON" };
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, initBtn { "INIT" }, savePresetBtn { "SAVE" }, loadPresetBtn { "LOAD" }, bankBtn { "BANK" };
    juce::Label presetLabel, title, tagline;
    juce::ListBox presetList { "presets", this };
    std::unique_ptr<StepGridComponent> stepGrid;
    juce::OwnedArray<juce::Label> fxSectionLabels;
    juce::OwnedArray<FxMonitor> fxMonitors;
    LfoShapeEditor lfoShapeEditor;
    juce::ComboBox lfo1WaveBox, lfo2WaveBox, lfo3WaveBox;
    juce::TextButton lfoPresetSine { "SIN" }, lfoPresetTri { "TRI" }, lfoPresetSaw { "SAW" },
                     lfoPresetSqr { "SQR" }, lfoPresetPulse { "PLS" }, lfoPresetCustom { "DRAW" };
    int lfoShapeTarget = 0;
    std::unique_ptr<MagicPad> magicPad;
    juce::TextButton magicLoopBtn, magicGlitchBtn, magicFlangeBtn, magicPsychBtn;
    juce::Label magicHint;
    juce::MidiKeyboardComponent keyboard;
    float phaseLights = 0.0f;
    float animPhase = 0.0f;
    juce::Colour themeAccent { 0xff00e8ff }, themeAccent2 { 0xffffd700 }, themePanelBg { 0xff0a0614 };
    juce::Image logoImg, heroImg, faceImg, lianImg, cyanImg;
    int heroIndex = 0;
    void applyHeroFromTheme();
    void cycleHero();
    struct PresetRow { bool isHeader = false; juce::String label; int programIndex = -1; };
    juce::Array<PresetRow> presetRows;
    void rebuildPresetRows();
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

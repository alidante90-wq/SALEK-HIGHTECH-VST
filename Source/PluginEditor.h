#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/ModMatrixPanel.h"
#include "UI/OpenGLGridBackdrop.h"

class SalekLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SalekLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff12081c));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff12081c));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffff00aa));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffe0e0ff));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff3a0066));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1a0a28));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xff00f0ff));
        setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff0a0614));
        setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xff00f0ff));
        setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(0xffa080c0));
    }
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
        auto bounds = juce::Rectangle<float>((float)x,(float)y,(float)width,(float)height).reduced(3.f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.46f;
        auto cx = bounds.getCentreX(), cy = bounds.getCentreY();
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillEllipse(cx-radius+2, cy-radius+3, radius*2, radius*2);
        juce::ColourGradient body(juce::Colour(0xff2a1a40), cx, cy-radius, juce::Colour(0xff0c0814), cx, cy+radius, false);
        g.setGradientFill(body);
        g.fillEllipse(cx-radius, cy-radius, radius*2, radius*2);
        g.setColour(fill.withAlpha(0.5f));
        g.drawEllipse(cx-radius, cy-radius, radius*2, radius*2, 1.5f);
        juce::Path track;
        track.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1e1028));
        g.strokePath(track, juce::PathStrokeType(3.2f));
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius*0.78f, radius*0.78f, 0, rotaryStartAngle, angle, true);
        g.setColour(fill.withAlpha(0.4f));
        g.strokePath(arc, juce::PathStrokeType(5.f));
        g.setColour(fill);
        g.strokePath(arc, juce::PathStrokeType(2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path needle;
        needle.addRoundedRectangle(-1.5f, -radius*0.68f, 3.f, radius*0.42f, 1.2f);
        g.setColour(juce::Colours::white);
        g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(fill.brighter(0.3f));
        g.fillEllipse(cx-4.5f, cy-4.5f, 9.f, 9.f);
    }
};

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
    g.setColour (juce::Colour (0xffff00aa).withAlpha (0.4f)); g.drawRoundedRectangle (r, 6.0f, 1.0f);
    auto gval = [&](const char* id, float d) { if (auto* p = apvts.getRawParameterValue (id)) return p->load(); return d; };
    float table = gval ("osc1_table", 0.f), warp = gval ("osc1_warp", 0.f), fold = gval ("osc1_fold", 0.f), drive = gval ("osc1_drive", 0.f);
    juce::Path wave; const int N = 96; float midY = r.getCentreY()+4.f, amp = r.getHeight()*0.32f;
    for (int i = 0; i < N; ++i) {
        float phase = (float)i/(float)N; if (warp>1e-4f) phase = juce::jlimit(0.f,0.9999f,std::pow(phase,1.f+warp*3.5f));
        float s=0.f; for (int h=1;h<=10;++h){ float harm=std::sin(phase*juce::MathConstants<float>::twoPi*(float)h);
            float wSine=(h==1)?1.f:0.f, wSaw=1.f/(float)h, wSqr=(h%2==1)?1.f/(float)h:0.f;
            s+=harm*(wSine*(1-table)*(1-table)+wSaw*2*table*(1-table)+wSqr*table*table);}
        s*=0.4f; if(fold>1e-4f){float th=1.f-fold*0.85f; float x=s*(1.f+fold*4.f); for(int k=0;k<2;++k){if(x>th)x=th-(x-th);else if(x<-th)x=-th-(x+th);else break;} s=x/(1.f+fold*1.5f);}
        if(drive>1e-4f)s=std::tanh(s*(1.f+drive*5.f));
        float px=r.getX()+4+((float)i/(N-1))*(r.getWidth()-8); float py=midY-s*amp;
        if(i==0)wave.startNewSubPath(px,py); else wave.lineTo(px,py);
    }
    g.setColour (juce::Colour (0xff00f0ff)); g.strokePath (wave, juce::PathStrokeType (1.6f));
}

class ScopeDisplay : public juce::Component, private juce::Timer {
public:
    ScopeDisplay() { startTimerHz(24); }
    void paint(juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(1.f);
        g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,6.f);
        juce::Path wave; for(int i=0;i<64;++i){float t=(float)i/63.f; float ph=t*juce::MathConstants<float>::twoPi*2.5f+phase;
            float y=std::sin(ph)*0.5f+0.15f*std::sin(ph*3.f); float px=r.getX()+t*r.getWidth(); float py=r.getCentreY()-y*r.getHeight()*0.35f;
            if(i==0)wave.startNewSubPath(px,py); else wave.lineTo(px,py);}
        g.setColour(juce::Colour(0xffff00aa)); g.strokePath(wave, juce::PathStrokeType(1.4f));
    }
    void timerCallback() override { phase += 0.15f; repaint(); }
private:
    float phase = 0.f;
};

class StepGridComponent : public juce::Component, private juce::Timer {
public:
    explicit StepGridComponent (salek::StepSequencer& seq) : sequencer (seq) { startTimerHz (16); }
    void paint (juce::Graphics& g) override {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff0c0818)); g.fillRoundedRectangle (r, 6.0f);
        const int n = salek::StepSequencer::NumSteps; const float gap=3.f; const float w=(r.getWidth()-gap*(n+1))/(float)n; const float h=r.getHeight()-gap*2;
        const int play = sequencer.getCurrentStep();
        for (int i=0;i<n;++i){ auto cell=juce::Rectangle<float>(r.getX()+gap+i*(w+gap),r.getY()+gap,w,h);
            const auto& st=sequencer.getStep(i);
            g.setColour(st.active?(i==play?juce::Colour(0xffff00aa):juce::Colour(0xff00f0ff).withAlpha(0.7f)):juce::Colour(0xff1a1028));
            g.fillRoundedRectangle(cell,3.f);}
    }
    void mouseDown (const juce::MouseEvent& e) override {
        auto r=getLocalBounds().toFloat().reduced(2.f); const int n=salek::StepSequencer::NumSteps; const float gap=3.f; const float w=(r.getWidth()-gap*(n+1))/(float)n;
        int idx=(int)((e.position.x-r.getX()-gap)/(w+gap)); if(idx>=0&&idx<n){sequencer.getStep(idx).active=!sequencer.getStep(idx).active; repaint();}
    }
    void timerCallback() override { repaint(); }
private:
    salek::StepSequencer& sequencer;
};

class AdsrDisplay : public juce::Component, private juce::Timer {
public:
    explicit AdsrDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (20); }
    void paint (juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,8.f);
        g.setColour(juce::Colour(0xff00f0ff).withAlpha(0.45f)); g.drawRoundedRectangle(r,8.f,1.f);
        auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
        float a=juce::jmax(0.001f,gval("amp_attack",0.01f)), d=juce::jmax(0.001f,gval("amp_decay",0.15f));
        float s=juce::jlimit(0.f,1.f,gval("amp_sustain",0.75f)), rel=juce::jmax(0.001f,gval("amp_release",0.25f));
        float total=a+d+0.4f+rel, xa=a/total, xd=d/total, xs=0.4f/total;
        auto plot=r.reduced(8.f,6.f); juce::Path curve; float x0=plot.getX(), y0=plot.getBottom(), w=plot.getWidth(), h=plot.getHeight();
        curve.startNewSubPath(x0,y0); curve.lineTo(x0+xa*w,plot.getY()); curve.lineTo(x0+(xa+xd)*w,plot.getY()+(1.f-s)*h);
        curve.lineTo(x0+(xa+xd+xs)*w,plot.getY()+(1.f-s)*h); curve.lineTo(x0+w,y0);
        g.setColour(juce::Colour(0xff00f0ff)); g.strokePath(curve, juce::PathStrokeType(2.f));
    }
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};

class FilterCurveDisplay : public juce::Component, private juce::Timer {
public:
    explicit FilterCurveDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (20); }
    void paint (juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,8.f);
        g.setColour(juce::Colour(0xffff00aa).withAlpha(0.4f)); g.drawRoundedRectangle(r,8.f,1.f);
        auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
        float cut=gval("filter_cutoff",8000.f), res=gval("filter_reso",0.25f); int mode=(int)gval("filter_mode",0.f);
        auto plot=r.reduced(8.f,6.f); juce::Path curve; const int N=80;
        for(int i=0;i<N;++i){ float t=(float)i/(N-1); float freq=20.f*std::pow(1000.f,t); float ratio=freq/juce::jmax(20.f,cut); float mag=1.f;
            if(mode==0)mag=1.f/std::sqrt(1.f+std::pow(ratio,4.f)); else if(mode==1)mag=1.f/std::sqrt(1.f+std::pow(1.f/juce::jmax(0.01f,ratio),4.f));
            else if(mode==2)mag=1.f/std::sqrt(1.f+std::pow((ratio-1.f/ratio)*2.f,2.f)); else mag=std::abs((ratio*ratio-1.f)/(ratio*ratio+1.f+0.001f));
            float peak=1.f+res*2.5f*std::exp(-std::pow((std::log(juce::jmax(0.01f,ratio)))*3.f,2.f)); mag=juce::jlimit(0.f,1.2f,mag*peak)/1.2f;
            float px=plot.getX()+t*plot.getWidth(); float py=plot.getBottom()-mag*plot.getHeight();
            if(i==0)curve.startNewSubPath(px,py); else curve.lineTo(px,py);}
        g.setColour(juce::Colour(0xffff00aa)); g.strokePath(curve, juce::PathStrokeType(2.f));
    }
    void timerCallback() override { repaint(); }
private:
    juce::AudioProcessorValueTreeState& apvts;
};

class LfoDisplay : public juce::Component, private juce::Timer {
public:
    explicit LfoDisplay (juce::AudioProcessorValueTreeState& s) : apvts (s) { startTimerHz (30); }
    void paint (juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xff0c0818)); g.fillRoundedRectangle(r,8.f);
        g.setColour(juce::Colour(0xff66ff99).withAlpha(0.45f)); g.drawRoundedRectangle(r,8.f,1.f);
        auto gval=[&](const char* id,float d){if(auto*p=apvts.getRawParameterValue(id))return p->load();return d;};
        float rate=gval("lfo_rate",2.f); int wave=(int)gval("lfo_wave",0.f); float amt=gval("lfo_amount",0.f);
        auto plot=r.reduced(8.f,4.f); juce::Path curve; const int N=100;
        for(int i=0;i<N;++i){ float t=(float)i/(N-1); float ph=t*juce::MathConstants<float>::twoPi*2.f+phase; float y=0.f;
            if(wave==0)y=std::sin(ph); else if(wave==1)y=1.f-4.f*std::abs(std::fmod(ph/juce::MathConstants<float>::twoPi+0.25f,1.f)-0.5f);
            else if(wave==2)y=2.f*(ph/juce::MathConstants<float>::twoPi-std::floor(ph/juce::MathConstants<float>::twoPi+0.5f));
            else if(wave==3)y=(std::sin(ph)>=0.f?1.f:-1.f); else y=(std::sin(ph*0.5f)>0.f?1.f:-1.f);
            y*=(0.35f+0.65f*amt); float px=plot.getX()+t*plot.getWidth(); float py=plot.getCentreY()-y*plot.getHeight()*0.42f;
            if(i==0)curve.startNewSubPath(px,py); else curve.lineTo(px,py);}
        g.setColour(juce::Colour(0xff66ff99)); g.strokePath(curve, juce::PathStrokeType(1.8f));
    }
    void timerCallback() override {
        float rate=2.f; if(auto*p=apvts.getRawParameterValue("lfo_rate")) rate=p->load();
        phase+=0.08f*juce::jlimit(0.2f,4.f,rate*0.25f); repaint();
    }
private:
    juce::AudioProcessorValueTreeState& apvts;
    float phase=0.f;
};

class SalekHightechAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::ListBoxModel,
                                          private juce::Timer {
public:
    explicit SalekHightechAudioProcessorEditor(SalekHightechAudioProcessor&);
    ~SalekHightechAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool selected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
private:
    SalekHightechAudioProcessor& processor;
    SalekLookAndFeel lnf;
    ScopeDisplay scope;
    std::unique_ptr<WavetableDisplay> wtDisplay;
    std::unique_ptr<AdsrDisplay> adsrDisplay;
    std::unique_ptr<FilterCurveDisplay> filterDisplay;
    std::unique_ptr<LfoDisplay> lfoDisplay;
    std::unique_ptr<ModMatrixPanel> matrixPanel;
    std::unique_ptr<OpenGLGridBackdrop> glBackdrop;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    struct Knob { juce::Slider s; juce::Label name; };
    std::vector<std::unique_ptr<Knob>> knobs;
    using SAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<SAtt>> atts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    juce::Component mainTab, oscTab, filterTab, envTab, modTab, fxTab, seqTab, presetTab, modularTab;
    juce::ComboBox themeBox;
    juce::ComboBox filterMode, lfoWave;
    juce::ToggleButton arpOn { "ARP ON" }, seqOn { "SEQ ON" };
    juce::TextButton prevPreset { "<" }, nextPreset { ">" }, initBtn { "INIT" };
    juce::Label presetLabel, title, tagline;
    juce::ListBox presetList { "presets", this };
    std::unique_ptr<StepGridComponent> stepGrid;
    juce::MidiKeyboardComponent keyboard;
    float phaseLights = 0.0f;
    float animPhase = 0.0f;
    juce::Image logoImg, heroImg;
    struct PresetRow { bool isHeader = false; juce::String label; int programIndex = -1; };
    juce::Array<PresetRow> presetRows;
    void rebuildPresetRows();
    Knob& addKnob(juce::Component& parent, const char* id, const char* label, juce::Colour c);
    void addCombo(juce::Component& parent, juce::ComboBox& box, const char* id, juce::StringArray items);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SalekHightechAudioProcessorEditor)
};

#include "GitiFxEditor.h"
GitiFxEditor::GitiFxEditor(GitiFxAudioProcessor& p):AudioProcessorEditor(p),processor(p)
{
    setSize(1280,760); setResizable(true,true);
    addKnob("grain_size","SIZE",35,210); addKnob("density","DENSITY",145,210); addKnob("position","POSITION",255,210); addKnob("spray","SPRAY",365,210);
    addKnob("pitch","PITCH",475,210); addKnob("stretch","STRETCH",585,210); addKnob("chaos","CHAOS",695,210);
    addKnob("destroy","DESTROY",35,470); addKnob("drive","DRIVE",145,470); addKnob("cutoff","FILTER",255,470); addKnob("resonance","RESO",365,470);
    addKnob("delay_mix","DELAY",475,470); addKnob("delay_time","TIME",585,470); addKnob("feedback","FEEDBACK",695,470); addKnob("reverb_mix","REVERB",805,470);
    modeBox.addItemList({"LIVE","LOOP","FREEZE","MUTATE"},1); addAndMakeVisible(modeBox);
    modeAtt=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getAPVTS(),"mode",modeBox);
    addAndMakeVisible(freezeBtn); freezeBtn.onClick=[this]{
        if (auto* parameter = processor.getAPVTS().getParameter("freeze"))
            parameter->setValueNotifyingHost(freezeBtn.getToggleState() ? 1.0f : 0.0f);
    }; freezeBtn.setClickingTogglesState(true);
    startTimerHz(30);
}
void GitiFxEditor::addKnob(const char* id,const char* text,int x,int y,int w)
{
    auto* s=knobs.add(new juce::Slider()); s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,18); addAndMakeVisible(s);
    auto* l=labels.add(new juce::Label()); l->setText(text,juce::dontSendNotification); l->setJustificationType(juce::Justification::centred); l->setColour(juce::Label::textColourId,cyan); addAndMakeVisible(l);
    atts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.getAPVTS(),id,*s));
    s->setBounds(x,y,w,80); l->setBounds(x,y-18,w,20);
}
void GitiFxEditor::paint(juce::Graphics& g)
{
    auto b=getLocalBounds().toFloat(); g.fillAll(juce::Colour(0xff05020d));
    for(int i=0;i<14;i++){float a=i/14.f; g.setColour(juce::Colour::fromHSV(.78f-a*.12f,.75f,.16f+a*.05f,1)); g.fillEllipse(b.getWidth()*.72f+a*80,b.getHeight()*.18f+a*18,260,180);}
    g.setColour(cyan); g.setFont(juce::FontOptions(48.f,juce::Font::bold)); g.drawText("GITI FX",30,18,500,60,juce::Justification::left);
    g.setColour(pink); g.setFont(juce::FontOptions(15.f,juce::Font::bold)); g.drawText("MULTI-FX  /  GRANULAR AUDIO PROCESSOR",34,72,620,28,juce::Justification::left);
    g.setColour(juce::Colours::white); g.setFont(juce::FontOptions(13.f)); g.drawText("BY SALEK HIGHTECH",36,99,350,24,juce::Justification::left);
    auto center=juce::Rectangle<float>(800,125,430,245); g.setColour(juce::Colour(0xff080b19)); g.fillRoundedRectangle(center,16);
    g.setColour(cyan.withAlpha(.65f)); g.drawRoundedRectangle(center,16,2);
    g.setColour(pink.withAlpha(.18f)); for(int i=0;i<9;i++) g.drawEllipse(juce::Rectangle<float>(center.getX()+20+i*42.0f,
                                          center.getY()+50.0f+std::sin(i*1.8f)*28.0f,
                                          100.0f, 100.0f), 1.0f);
    g.setColour(cyan); g.setFont(juce::FontOptions(13.f,juce::Font::bold)); g.drawText("GRANULAR CORE",center.getX()+18,center.getY()+18,180,22,juce::Justification::left);
    g.setColour(juce::Colours::white.withAlpha(.65f)); g.drawText("SIZE   DENSITY   POSITION   SPRAY   PITCH   STRETCH",center.getX()+18,center.getBottom()-38,390,22,juce::Justification::left);
    auto section=[&](int x,int y,int w,int h,const char* title,juce::Colour c){g.setColour(juce::Colour(0xff0a0715));g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,10);g.setColour(c.withAlpha(.65f));g.drawRoundedRectangle((float)x,(float)y,(float)w,(float)h,10,1.5f);g.setColour(c);g.setFont(juce::FontOptions(14.f,juce::Font::bold));g.drawText(title,x+12,y+10,w-24,20,juce::Justification::left);};
    section(20,155,760,250,"01  GRANULAR",cyan); section(20,415,930,230,"02  DESTROY / FILTER / SPACE",pink);
    section(965,390,295,255,"FX MATRIX  /  CHAOS",purple);
    g.setColour(cyan.withAlpha(.6f)); g.drawRect(985,440,250,150,1);
    for(int i=0;i<7;i++){g.setColour((i%2?pink:cyan).withAlpha(.35f));g.drawLine(995,455+i*18,1225,455+(i*18+int(std::sin(i*2.2f)*28)),1);}
    g.setColour(juce::Colours::white.withAlpha(.5f));g.setFont(juce::FontOptions(11.f));g.drawText("GRANULAR • DESTROY • FILTER • DELAY • REVERB • MOD • GLITCH",25,680,900,25,juce::Justification::left);
}
void GitiFxEditor::resized()
{
    auto area=getLocalBounds(); modeBox.setBounds(820,205,120,28); freezeBtn.setBounds(1080,205,120,28);
}
void GitiFxEditor::timerCallback(){repaint(800,120,450,260);}

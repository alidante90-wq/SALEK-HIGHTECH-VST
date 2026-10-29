#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

// Artist-specific animated identity strip for GITI. UI-only: no DSP/audio-thread work.
class ArtistVisualProfile : public juce::Component, private juce::Timer
{
public:
    struct Profile { const char* name; juce::Colour a, b, c; int motif; };
    static const std::array<Profile, 10>& profiles()
    {
        static const std::array<Profile, 10> p {{
            { "AVANAVA",       juce::Colour(0xffe8f5f7), juce::Colour(0xff62eee2), juce::Colour(0xff9caeb8), 0 },
            { "PSYELL",        juce::Colour(0xffbd45ff), juce::Colour(0xff6d1bdb), juce::Colour(0xffff4fcf), 1 },
            { "AUDIOLOGIST",   juce::Colour(0xff7acbff), juce::Colour(0xffdcecff), juce::Colour(0xff3488c9), 2 },
            { "ASHKANPASHA",   juce::Colour(0xffcda45d), juce::Colour(0xff9e172c), juce::Colour(0xffdce0e5), 3 },
            { "BRAIN POISON",  juce::Colour(0xffa5ff39), juce::Colour(0xff6d1bdb), juce::Colour(0xff36d77a), 4 },
            { "MIL.ODY",       juce::Colour(0xffc78cff), juce::Colour(0xffff83bd), juce::Colour(0xff58e9dc), 5 },
            { "IKKI",          juce::Colour(0xfff4f4f4), juce::Colour(0xffff303e), juce::Colour(0xff777b84), 6 },
            { "OSOM",          juce::Colour(0xff59baff), juce::Colour(0xff55f2e5), juce::Colour(0xffd5c9ff), 7 },
            { "anahita",       juce::Colour(0xff55e5d5), juce::Colour(0xffd6e9ed), juce::Colour(0xffa1b9c1), 0 },
            { "SALEK",         juce::Colour(0xffa66bff), juce::Colour(0xff37eee0), juce::Colour(0xffff354f), 1 }
        }};
        return p;
    }
    ArtistVisualProfile()
    {
        setOpaque(false);
        selector.addItem("01  AVANAVA", 1); selector.addItem("02  PSYELL", 2);
        selector.addItem("03  AUDIOLOGIST", 3); selector.addItem("04  ASHKANPASHA", 4);
        selector.addItem("05  BRAIN POISON", 5); selector.addItem("06  MIL.ODY", 6);
        selector.addItem("07  IKKI", 7); selector.addItem("08  OSOM", 8);
        selector.addItem("09  anahita", 9); selector.addItem("10  SALEK", 10);
        selector.setSelectedId(10, juce::dontSendNotification);
        selector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xee090b16));
        selector.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        selector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff37eee0));
        addAndMakeVisible(selector);
        selector.onChange = [this] { selected = juce::jlimit(0, 9, selector.getSelectedId()-1); if (onProfileChanged) onProfileChanged(profiles()[(size_t)selected]); repaint(); };
        startTimerHz(30);
    }
    std::function<void(const Profile&)> onProfileChanged;
    const Profile& current() const { return profiles()[(size_t)selected]; }
    int selectedIndex() const { return selected; }
    void setAudioPulse(float p) { audioPulse = juce::jlimit(0.f, 1.f, p); }
    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        const auto& p = current();
        const float pulse = juce::jmax(audioPulse, 0.10f + 0.08f * std::sin(phase*2.1f));
        auto panel = r.reduced(3.f);
        g.setColour(juce::Colour(0xe9080c18)); g.fillRoundedRectangle(panel, 10.f);
        g.setColour(p.b.withAlpha(0.40f + pulse*0.3f)); g.drawRoundedRectangle(panel, 10.f, 1.2f);
        auto cx = panel.getCentreX() + mouseX * 5.f;
        auto cy = panel.getBottom() - 34.f + mouseY * 3.f;
        const float rad = juce::jmin(panel.getWidth()*0.25f, 24.f) * (1.f + pulse*0.12f);
        juce::Path orbit;
        orbit.addEllipse(cx-rad, cy-rad*0.45f, rad*2.f, rad*0.9f);
        g.setColour(p.a.withAlpha(0.25f)); g.strokePath(orbit, juce::PathStrokeType(1.f));
        for (int i=0; i<8; ++i)
        {
            float ang = phase*(0.55f + (i%3)*0.16f) + i*juce::MathConstants<float>::twoPi/8.f;
            float x = cx + std::cos(ang)*rad;
            float y = cy + std::sin(ang)*rad*0.45f;
            float size = 2.2f + pulse*2.f + (i%3);
            g.setColour((i%2 ? p.b : p.c).withAlpha(0.65f + pulse*0.3f));
            g.fillEllipse(x-size/2.f,y-size/2.f,size,size);
        }
        // Distinct geometric signature per artist.
        juce::Path glyph;
        const float gx=cx, gy=cy;
        if (p.motif==0) { glyph.startNewSubPath(gx,gy-rad*.48f); glyph.lineTo(gx+rad*.45f,gy); glyph.lineTo(gx,gy+rad*.48f); glyph.lineTo(gx-rad*.45f,gy); glyph.closeSubPath(); }
        else if (p.motif==1 || p.motif==4) { for(int i=0;i<6;++i){float a=phase*.35f+i*juce::MathConstants<float>::twoPi/6.f; float x=gx+std::cos(a)*rad*.42f,y=gy+std::sin(a)*rad*.42f;if(i==0)glyph.startNewSubPath(x,y);else glyph.lineTo(x,y);} glyph.closeSubPath(); }
        else if (p.motif==2) { for(int i=0;i<5;++i){float x= gx-rad*.42f+i*rad*.21f; glyph.startNewSubPath(x,gy+rad*.38f); glyph.lineTo(x,gy-rad*(.2f+.06f*(i%2)));} }
        else if (p.motif==3) { glyph.addEllipse(gx-rad*.35f,gy-rad*.35f,rad*.7f,rad*.7f); glyph.addEllipse(gx-rad*.19f,gy-rad*.19f,rad*.38f,rad*.38f); }
        else if (p.motif==5 || p.motif==7) { glyph.startNewSubPath(gx-rad*.45f,gy); glyph.cubicTo(gx-rad*.2f,gy-rad*.7f,gx+rad*.2f,gy+rad*.7f,gx+rad*.45f,gy); }
        else { glyph.startNewSubPath(gx-rad*.4f,gy-rad*.35f); glyph.lineTo(gx+rad*.4f,gy+rad*.35f); glyph.startNewSubPath(gx-rad*.4f,gy+rad*.35f); glyph.lineTo(gx+rad*.4f,gy-rad*.35f); }
        g.setColour(p.a.withAlpha(0.95f)); g.strokePath(glyph, juce::PathStrokeType(1.4f));
        // The artist name is deliberately prominent and travels on a seamless horizontal loop.
        g.setColour(p.b.withAlpha(0.18f + pulse*.12f));
        g.fillRoundedRectangle(panel.reduced(5.f).withY(panel.getBottom()-29.f).withHeight(25.f), 5.f);
        g.setColour(p.a); g.setFont(juce::FontOptions(13.f, juce::Font::bold));
        auto nameArea = panel.reduced(8.f).withY(panel.getBottom()-28.f).withHeight(22.f);
        const float textW = juce::jmax(nameArea.getWidth()+28.f, (float)g.getCurrentFont().getStringWidth(p.name)+30.f);
        float x = nameArea.getX() - std::fmod(phase*24.f, textW);
        while (x < nameArea.getRight())
        {
            g.drawText(p.name, juce::Rectangle<float>(x,nameArea.getY(),textW,nameArea.getHeight()), juce::Justification::centred);
            x += textW;
        }
    }
    void resized() override { selector.setBounds(getLocalBounds().reduced(8).removeFromTop(24)); }
    void mouseMove(const juce::MouseEvent& e) override { auto b=getLocalBounds().toFloat(); mouseX=(e.position.x/b.getWidth()-.5f)*2.f; mouseY=(e.position.y/b.getHeight()-.5f)*2.f; }
    void mouseExit(const juce::MouseEvent&) override { mouseX=mouseY=0.f; }
private:
    juce::ComboBox selector;
    int selected=9;
    float phase=0.f, mouseX=0.f, mouseY=0.f, audioPulse=0.f;
    void timerCallback() override { phase += 0.035f; repaint(); }
};

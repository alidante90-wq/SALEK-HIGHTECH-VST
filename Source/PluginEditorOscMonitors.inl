// Three live OSC shape monitors on oscTab
{
    oscMon1 = std::make_unique<OscShapeMonitor>();
    oscMon2 = std::make_unique<OscShapeMonitor>();
    oscMon3 = std::make_unique<OscShapeMonitor>();
    oscMon1->setAPVTS (&processor.getAPVTS()); oscMon1->setOscIndex (0);
    oscMon1->setAccent (juce::Colour (0xff00e8ff)); oscMon1->setTitle ("OSC1 SHAPE");
    oscMon2->setAPVTS (&processor.getAPVTS()); oscMon2->setOscIndex (1);
    oscMon2->setAccent (juce::Colour (0xffff2d9b)); oscMon2->setTitle ("OSC2 SHAPE");
    oscMon3->setAPVTS (&processor.getAPVTS()); oscMon3->setOscIndex (2);
    oscMon3->setAccent (juce::Colour (0xff39ff14)); oscMon3->setTitle ("OSC3 SHAPE");
    oscTab.addAndMakeVisible (*oscMon1);
    oscTab.addAndMakeVisible (*oscMon2);
    oscTab.addAndMakeVisible (*oscMon3);
}

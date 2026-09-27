// Three live OSC shape monitors + ASCII shape presets (no broken Persian glyphs)
{
    oscMon1 = std::make_unique<OscShapeMonitor>();
    oscMon2 = std::make_unique<OscShapeMonitor>();
    oscMon3 = std::make_unique<OscShapeMonitor>();
    oscMon1->setAPVTS (&processor.getAPVTS()); oscMon1->setOscIndex (0);
    oscMon1->setAccent (juce::Colour (0xff00e8ff));
    oscMon1->setTitle ("OSC1");
    oscMon2->setAPVTS (&processor.getAPVTS()); oscMon2->setOscIndex (1);
    oscMon2->setAccent (juce::Colour (0xffff2d9b));
    oscMon2->setTitle ("OSC2");
    oscMon3->setAPVTS (&processor.getAPVTS()); oscMon3->setOscIndex (2);
    oscMon3->setAccent (juce::Colour (0xff39ff14));
    oscMon3->setTitle ("OSC3");
    oscTab.addAndMakeVisible (*oscMon1);
    oscTab.addAndMakeVisible (*oscMon2);
    oscTab.addAndMakeVisible (*oscMon3);

    auto shapeItems = juce::StringArray {
        "01 SINE","02 TRI","03 SQUARE","04 SAW","05 WARM SAW","06 HARD SAW","07 SOFT PULSE","08 FORMANT",
        "09 DIGITAL","10 METAL","11 GLASS","12 VOWEL","13 ALIEN","14 SCREAM","15 FOLD","16 DARK",
        "17 FAT SAW","18 BRIGHT SAW","19 SOFT SQ","20 HARD SQ","21 SAW+SQ","22 TRI+SAW","23 PULSE","24 NARROW PULSE",
        "25 FORMANT A","26 FORMANT B","27 METAL A","28 METAL B","29 CRUSH","30 HITECH","31 DARKPSY","32 CUSTOM"
    };

    auto applyShape = [this] (int osc, int idx)
    {
        const int frame[32] = { 0,3,7,11,16,18,22,25,30,36,42,48,54,60,66,72,78,84,88,92,96,100,104,108,112,116,120,123,125,126,127,127 };
        const float warp[32] = { 0.00f,0.02f,0.04f,0.06f,0.08f,0.10f,0.12f,0.14f,0.16f,0.18f,0.20f,0.22f,0.24f,0.26f,0.28f,0.30f,0.34f,0.38f,0.42f,0.46f,0.50f,0.54f,0.58f,0.62f,0.66f,0.70f,0.74f,0.78f,0.82f,0.86f,0.92f,0.50f };
        const float fold[32] = { 0.00f,0.00f,0.02f,0.04f,0.06f,0.08f,0.10f,0.12f,0.14f,0.16f,0.18f,0.20f,0.22f,0.24f,0.26f,0.30f,0.34f,0.38f,0.42f,0.46f,0.50f,0.54f,0.58f,0.62f,0.66f,0.70f,0.74f,0.78f,0.82f,0.86f,0.92f,0.15f };
        const char* tid[] = { "osc1_table", "osc2_table", "osc3_table" };
        const char* wid[] = { "osc1_warp",  "osc2_warp",  "osc3_warp" };
        const char* fid[] = { "osc1_fold",  "osc2_fold",  "osc3_fold" };
        idx = juce::jlimit (0, 31, idx);
        auto setP = [&] (const char* id, float v)
        {
            if (auto* p = processor.getAPVTS().getParameter (id))
                p->setValueNotifyingHost (p->convertTo0to1 (v));
        };
        setP (tid[osc], (float) frame[idx] / 127.0f);
        setP (wid[osc], warp[idx]);
        setP (fid[osc], fold[idx]);
    };

    auto setupBox = [&] (juce::ComboBox& box, int osc, juce::Colour col)
    {
        box.clear (juce::dontSendNotification);
        box.addItemList (shapeItems, 1);
        box.setSelectedItemIndex (0, juce::dontSendNotification);
        box.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1a0a30));
        box.setColour (juce::ComboBox::textColourId, col);
        oscTab.addAndMakeVisible (box);
        box.onChange = [this, &box, osc, applyShape]
        {
            applyShape (osc, box.getSelectedItemIndex());
        };
    };
    setupBox (osc1ShapeBox, 0, juce::Colour (0xff00e8ff));
    setupBox (osc2ShapeBox, 1, juce::Colour (0xffff2d9b));
    setupBox (osc3ShapeBox, 2, juce::Colour (0xff39ff14));
}

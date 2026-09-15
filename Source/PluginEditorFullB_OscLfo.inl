        // OSC monitors strip at top of osc area
        {
            auto ob = oscTab.getLocalBounds().reduced (4);
            auto monStrip = ob.removeFromTop (72);
            const int mw = monStrip.getWidth() / 3;
            if (oscMon1) oscMon1->setBounds (monStrip.removeFromLeft (mw).reduced (3));
            if (oscMon2) oscMon2->setBounds (monStrip.removeFromLeft (mw).reduced (3));
            if (oscMon3) oscMon3->setBounds (monStrip.reduced (3));
            place (ob, knobs, 0, 21, 6);
        }

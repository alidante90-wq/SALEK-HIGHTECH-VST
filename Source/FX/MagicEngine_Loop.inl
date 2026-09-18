// ---------- LOOP: true freeze — tight robotic high-tech stutter ----------
    // Y = loop length: bottom = ultra-tight (~8ms robotic), top = longer phrase
    // X = playback rate: left slow, right fast stutter
    void processLoopSample (float& outL, float& outR) noexcept
    {
        const int minLen = juce::jmax (32, (int) (sr * 0.008));
        const int maxLen = juce::jmax (minLen, freezeCapLen > 64 ? freezeCapLen : (int) (sr * 0.5));
        const float yCurve = ySmoothed * ySmoothed;
        const int len = juce::jlimit (minLen, maxLen,
            (int) (minLen + (maxLen - minLen) * (0.02f + yCurve * 0.98f)));
        freezeLen = len;
        const float rate = 0.15f + xSmoothed * 3.35f;

        if (freezeCapLen < 64)
        {
            const int liveLen = juce::jlimit (minLen, bufSize - 1,
                (int) (sr * (0.008 + ySmoothed * ySmoothed * 1.2)));
            float rp = (float) ((writePos - liveLen + bufSize) % bufSize) + (float) loopPhase;
            while (rp >= (float) bufSize) rp -= (float) bufSize;
            while (rp < 0.f) rp += (float) bufSize;
            const int i0 = ((int) rp) % bufSize;
            const int i1 = (i0 + 1) % bufSize;
            const float f = rp - (float) (int) rp;
            outL = bufL[(size_t) i0] * (1.f - f) + bufL[(size_t) i1] * f;
            outR = bufR[(size_t) i0] * (1.f - f) + bufR[(size_t) i1] * f;
            if (liveLen < (int) (sr * 0.03))
            {
                outL = std::tanh (outL * 1.35f);
                outR = std::tanh (outR * 1.35f);
            }
            loopPhase += (double) rate;
            if (loopPhase >= (double) liveLen) loopPhase -= (double) liveLen;
            return;
        }

        float rp = (float) loopPhase;
        while (rp >= (float) len) rp -= (float) len;
        while (rp < 0.f) rp += (float) len;
        const int i0 = ((int) rp) % len;
        const int i1 = (i0 + 1) % len;
        const float f = rp - (float) (int) rp;
        outL = freezeL[(size_t) i0] * (1.f - f) + freezeL[(size_t) i1] * f;
        outR = freezeR[(size_t) i0] * (1.f - f) + freezeR[(size_t) i1] * f;
        if (len < (int) (sr * 0.025))
        {
            const float t = rp / (float) len;
            const float env = 0.65f + 0.35f * std::sin (t * juce::MathConstants<float>::pi);
            outL = std::tanh (outL * 1.4f) * env;
            outR = std::tanh (outR * 1.4f) * env;
        }

        loopPhase += (double) rate;
        if (loopPhase >= (double) len) loopPhase -= (double) len;
        if (loopPhase < 0.0) loopPhase += (double) len;
    }

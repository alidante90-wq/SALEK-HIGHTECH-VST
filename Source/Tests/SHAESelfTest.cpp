#include "../SHAE/SHAECore.h"
#include <cmath>
#include <iostream>

int main()
{
    using namespace shae;

    if (!std::isfinite (softClip (100.0f, 8.0f))) return 1;
    if (std::abs (softClip (0.0f) ) > 1.0e-7f) return 2;

    ParameterSmoother smoother;
    smoother.prepare (48000.0, 5.0f, 0.0f);
    smoother.setTarget (1.0f);
    float v = 0.0f;
    for (int i = 0; i < 4096; ++i) v = smoother.next();
    if (!(v > 0.99f && v <= 1.0f)) return 3;

    SafetyStage safety;
    safety.prepare (48000.0, 2);
    juce::AudioBuffer<float> b (2, 512);
    b.clear();
    b.setSample (0, 17, std::numeric_limits<float>::infinity());
    b.setSample (1, 31, std::numeric_limits<float>::quiet_NaN());
    safety.process (b);

    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (!std::isfinite (b.getSample (ch, i)) ||
                std::abs (b.getSample (ch, i)) > 0.996f)
                return 4;

    std::cout << "SHAE self-test: PASS\n";
    return 0;
}

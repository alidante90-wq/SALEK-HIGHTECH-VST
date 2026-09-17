#pragma once
#include <cmath>
#include <JuceHeader.h>
namespace salek {
/** One-pole smoother. Default ~8-12 ms is musical for most synth params. */
class SmoothedValue {
public:
    void reset(float v = 0.f) noexcept { current = target = v; }
    void setTarget(float t) noexcept { target = t; }
    void setTimeMs(float ms, double sr) noexcept {
        if (ms <= 0.f || sr <= 0.0) { coeff = 1.f; return; }
        coeff = 1.f - std::exp (-1.f / (float (sr) * ms * 0.001f));
        coeff = juce::jlimit (0.0001f, 1.f, coeff);
    }
    void snapTo (float v) noexcept { current = target = v; }
    float getNext() noexcept {
        current += coeff * (target - current);
        if (std::abs (current) < 1.0e-15f) current = 0.f;
        return current;
    }
    float getCurrent() const noexcept { return current; }
    float getTarget()  const noexcept { return target; }
private:
    float current = 0.f, target = 0.f, coeff = 0.08f;
};
}

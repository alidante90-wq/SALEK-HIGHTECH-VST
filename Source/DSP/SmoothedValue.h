#pragma once

namespace salek
{

/** Very lightweight one-pole smoother. Real-time safe. */
class SmoothedValue
{
public:
    void reset (float initial = 0.0f) noexcept
    {
        current = target = initial;
    }

    void setTarget (float t) noexcept { target = t; }

    void setTimeMs (float ms, double sampleRate) noexcept
    {
        if (ms <= 0.0f || sampleRate <= 0.0)
            coeff = 1.0f;
        else
            coeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * ms * 0.001f));
    }

    float getNext() noexcept
    {
        current += coeff * (target - current);
        return current;
    }

    float getCurrent() const noexcept { return current; }

private:
    float current = 0.0f;
    float target = 0.0f;
    float coeff = 0.1f;
};

} // namespace salek

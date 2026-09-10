#pragma once
#include <cmath>
namespace salek {
class SmoothedValue {
public:
    void reset(float v=0) noexcept { current=target=v; }
    void setTarget(float t) noexcept { target=t; }
    void setTimeMs(float ms, double sr) noexcept {
        coeff = (ms<=0||sr<=0) ? 1.f : 1.f - std::exp(-1.f/(float(sr)*ms*0.001f));
    }
    float getNext() noexcept { current += coeff*(target-current); return current; }
private:
    float current=0, target=0, coeff=0.1f;
};
}

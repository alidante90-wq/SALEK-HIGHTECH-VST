#pragma once
#include <JuceHeader.h>
#include <array>
namespace salek {
class ModMatrix {
public:
    static constexpr int MaxRoutes=16;
    enum class Source:int{LFO1=0,Env1,Velocity,ModWheel,Macro1,Macro2,Macro3,Macro4,Random,NumSources};
    enum class Dest:int{FilterCutoff=0,FilterReso,Osc1Level,Osc2Level,Osc3Level,Osc1Table,Osc2Table,Osc3Table,Osc1Warp,Osc1Fold,Fm2to1,Fm3to1,Pitch,NumDests};
    struct Route{Source source=Source::LFO1;Dest dest=Dest::FilterCutoff;float amount=0;bool active=false;};
    void clear()noexcept{for(auto&r:routes)r.active=false;}
    int addRoute(Source src,Dest dst,float amount)noexcept{
        for(int i=0;i<MaxRoutes;++i) if(routes[i].active&&routes[i].source==src&&routes[i].dest==dst){routes[i].amount=juce::jlimit(-1.f,1.f,amount);return i;}
        for(int i=0;i<MaxRoutes;++i) if(!routes[i].active){routes[i]={src,dst,juce::jlimit(-1.f,1.f,amount),true};return i;}
        return -1;
    }
    void setSourceValue(Source s,float v)noexcept{sourceValues[size_t(s)]=v;}
    float getModulation(Dest d)const noexcept{
        float sum=0; for(auto&r:routes) if(r.active&&r.dest==d) sum+=sourceValues[size_t(r.source)]*r.amount; return sum;
    }
private:
    std::array<Route,MaxRoutes> routes{};
    std::array<float,size_t(Source::NumSources)> sourceValues{};
};
}

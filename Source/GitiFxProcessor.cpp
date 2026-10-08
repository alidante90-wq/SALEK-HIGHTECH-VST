#include "GitiFxProcessor.h"
#include "GitiFxEditor.h"

GitiFxAudioProcessor::GitiFxAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  apvts(*this, nullptr, "GITI_FX_STATE", createParameterLayout())
{
    for (auto& g : grains) g.active=false;
}

juce::AudioProcessorValueTreeState::ParameterLayout GitiFxAudioProcessor::createParameterLayout()
{
    using P=juce::AudioParameterFloat; using C=juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> v;
    auto f=[&](const char* id,const char* name,float lo,float hi,float d,float skew=1.f){
        v.push_back(std::make_unique<P>(id,name,juce::NormalisableRange<float>(lo,hi,0.001f,skew),d));
    };
    f("input_mix","INPUT MIX",0,1,1); f("grain_size","GRAIN SIZE",0.005f,1.5f,0.12f,0.45f);
    f("density","DENSITY",0,1,0.45f); f("position","POSITION",0,1,0.5f);
    f("spray","SPRAY",0,1,0.18f); f("pitch","PITCH",-24,24,0);
    f("stretch","STRETCH",0.25f,4,1,0.5f); f("freeze","FREEZE",0,1,0);
    f("reverse","REVERSE",0,1,0); f("destroy","DESTROY",0,1,0);
    f("drive","DRIVE",0,1,0.1f); f("cutoff","CUTOFF",40,20000,12000,0.35f);
    f("resonance","RESONANCE",0.05f,1,0.2f); f("delay_mix","DELAY MIX",0,1,0.12f);
    f("delay_time","DELAY TIME",0.005f,1.2f,0.24f); f("feedback","FEEDBACK",0,0.95f,0.35f);
    f("reverb_mix","REVERB MIX",0,1,0.14f); f("chaos","CHAOS",0,1,0.12f);
    f("output","OUTPUT",0,1.5f,0.85f);
    v.push_back(std::make_unique<C>("mode","GRANULAR MODE",juce::StringArray{"LIVE","LOOP","FREEZE","MUTATE"},0));
    return {v.begin(),v.end()};
}

float GitiFxAudioProcessor::p(const char* id,float d) const
{
    if(auto* x=apvts.getRawParameterValue(id)) return x->load();
    return d;
}
bool GitiFxAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();
}
void GitiFxAudioProcessor::prepareToPlay(double sr,int block)
{
    sampleRate=sr; writePos=0; delayPos=0;
    grainBuffer.setSize(2,(int)(sr*8.0)+4); grainBuffer.clear();
    for(auto& g:grains) g.active=false;
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,2};
    filterL.reset(); filterR.reset(); filterL.prepare(spec); filterR.prepare(spec);
    reverb.reset(); reverb.prepare(spec);
}
float GitiFxAudioProcessor::window(float x)
{
    x=juce::jlimit(0.f,1.f,x); return 0.5f-0.5f*std::cos(juce::MathConstants<float>::twoPi*x);
}
void GitiFxAudioProcessor::spawnGrain()
{
    const int N=grainBuffer.getNumSamples();
    float size=p("grain_size",.12f), spray=p("spray",.18f), pos=p("position",.5f);
    int len=juce::jlimit(32,(int)(sampleRate*1.5),juce::roundToInt(size*(float)sampleRate));
    int center=(writePos-(int)((1.f-pos)*N))%N; if(center<0) center+=N;
    int spread=(int)(spray*N*.25f); int offset=spread ? (int)((uni(rng)*2.f-1.f)*spread):0;
    Grain* best=&grains[0]; for(auto& g:grains) if(!g.active){best=&g;break;}
    best->pos=(float)((center+offset+N)%N); best->age=0; best->len=(float)len;
    best->rate=std::pow(2.f,p("pitch",0)/12.f)/p("stretch",1.f);
    if(p("reverse",0)>0.5f) best->rate=-best->rate;
    float chaos=p("chaos",0); best->rate*=1.f+(uni(rng)*2.f-1.f)*chaos*.35f;
    best->pan=uni(rng)*2.f-1.f; best->active=true;
}
void GitiFxAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    const int n=b.getNumSamples(); const int N=grainBuffer.getNumSamples();
    auto inL=b.getReadPointer(0), inR=b.getReadPointer(1);
    bool freeze=p("freeze",0)>0.5f || (int)p("mode",0)==2;
    for(int i=0;i<n;++i){
        if(!freeze){grainBuffer.setSample(0,writePos,inL[i]);grainBuffer.setSample(1,writePos,inR[i]);}
        writePos=(writePos+1)%N;
    }
    b.clear();
    float density=p("density",.45f), step=juce::jlimit(.0001f,.12f,density*.018f);
    float phase=0;
    for(int i=0;i<n;++i){
        phase+=step; if(phase>=1.f){phase-=1.f; int count=1+(p("chaos",0)*3.f>uni(rng)?1:0); while(count--) spawnGrain();}
        float l=0,r=0; int active=0;
        for(auto& g:grains) if(g.active){
            float x=g.age/g.len; if(x>=1.f){g.active=false;continue;}
            int a=((int)g.pos%N+N)%N, bpos=(a+1)%N; float frac=g.pos-std::floor(g.pos);
            float env=window(x); float sl=grainBuffer.getSample(0,a)*(1-frac)+grainBuffer.getSample(0,bpos)*frac;
            float sr=grainBuffer.getSample(1,a)*(1-frac)+grainBuffer.getSample(1,bpos)*frac;
            float panL=0.7071f*(1-g.pan), panR=0.7071f*(1+g.pan);
            l+=sl*env*panL; r+=sr*env*panR; g.pos+=g.rate; g.age++; active++;
        }
        float dry=p("input_mix",1.f); l*=1.25f; r*=1.25f;
        l+=inL[i]*dry*(1.f-p("destroy",0)*.7f); r+=inR[i]*dry*(1.f-p("destroy",0)*.7f);
        float drive=1.f+p("drive",0)*12.f, dest=p("destroy",0);
        l=std::tanh(l*drive)*(1.f-dest*.45f)+l*dest*.12f; r=std::tanh(r*drive)*(1.f-dest*.45f)+r*dest*.12f;
        float dms=p("delay_time",.24f); int ds=juce::jlimit(1,191999,(int)(dms*sampleRate));
        int rp=(delayPos-ds+192000)%192000; float dl=delayBufL[rp],dr=delayBufR[rp];
        delayBufL[delayPos]=l+dl*p("feedback",.35f); delayBufR[delayPos]=r+dr*p("feedback",.35f); delayPos=(delayPos+1)%192000;
        float dm=p("delay_mix",0); l=l*(1-dm)+dl*dm; r=r*(1-dm)+dr*dm;
        b.setSample(0,i,l); b.setSample(1,i,r);
        granularActivity.store((float)active/64.f);
    }
    auto coeff=juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate,p("cutoff",12000),juce::jlimit(.1f,20.f,p("resonance",.2f)));
    filterL.coefficients=coeff; filterR.coefficients=coeff;
    juce::dsp::AudioBlock<float> block(b);
    juce::dsp::AudioBlock<float> left(block.getSingleChannelBlock(0));
    juce::dsp::AudioBlock<float> right(block.getSingleChannelBlock(1));
    juce::dsp::ProcessContextReplacing<float> leftCtx(left);
    juce::dsp::ProcessContextReplacing<float> rightCtx(right);
    filterL.process(leftCtx);
    filterR.process(rightCtx);
    {
        auto rp = reverb.getParameters();
        const float wet = p("reverb_mix", 0.14f);
        rp.wetLevel = wet;
        rp.dryLevel = 1.0f - wet;
        reverb.setParameters(rp);

        juce::dsp::AudioBlock<float> reverbBlock(b);
        juce::dsp::ProcessContextReplacing<float> reverbContext(reverbBlock);
        reverb.process(reverbContext);
    }
    b.applyGain(p("output",.85f));
}
const juce::String GitiFxAudioProcessor::getProgramName(int i)
{
    static const char* names[]={"VOID EVOLUTION","GRAIN STORM","PSYCHOSIS","FROZEN SIGNAL","ALIEN BUFFER","METAL DUST","QUANTUM VOICE","SALEK CHAOS"};
    return names[juce::jlimit(0,7,i)];
}
void GitiFxAudioProcessor::getStateInformation(juce::MemoryBlock& d)
{
    if(auto xml=apvts.copyState().createXml()) copyXmlToBinary(*xml,d);
}
void GitiFxAudioProcessor::setStateInformation(const void* data,int size)
{
    if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new GitiFxAudioProcessor();}

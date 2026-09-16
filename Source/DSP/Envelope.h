#pragma once
#include <JuceHeader.h>

// Classic ADSR + simple multi-stage support skeleton
class Envelope
{
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        setAttack (0.01f);
        setDecay (0.2f);
        setSustain (0.7f);
        setRelease (0.3f);
    }

    void setAttack (float seconds)
    {
        attackInc = (seconds > 0.0f) ? (1.0f / (seconds * (float) sr)) : 1.0f;
    }
    void setDecay (float seconds)
    {
        decayInc = (seconds > 0.0f) ? (1.0f / (seconds * (float) sr)) : 1.0f;
    }
    void setSustain (float level) { sustainLevel = juce::jlimit (0.0f, 1.0f, level); }
    void setRelease (float seconds)
    {
        releaseInc = (seconds > 0.0f) ? (1.0f / (seconds * (float) sr)) : 1.0f;
    }

    void noteOn()
    {
        stage = Stage::Attack;
        // From current level for retrigger smoothness
    }

    void noteOff()
    {
        if (stage != Stage::Idle)
            stage = Stage::Release;
    }

    float process()
    {
        switch (stage)
        {
            case Stage::Attack:
                level += attackInc;
                if (level >= 1.0f)
                {
                    level = 1.0f;
                    stage = Stage::Decay;
                }
                break;
            case Stage::Decay:
                level -= decayInc * (1.0f - sustainLevel + 0.0001f);
                if (level <= sustainLevel)
                {
                    level = sustainLevel;
                    stage = Stage::Sustain;
                }
                break;
            case Stage::Sustain:
                level = sustainLevel;
                break;
            case Stage::Release:
                level -= releaseInc * (level + 0.0001f);
                if (level <= 0.0f)
                {
                    level = 0.0f;
                    stage = Stage::Idle;
                }
                break;
            case Stage::Idle:
            default:
                level = 0.0f;
                break;
        }
        return level;
    }

    bool isActive() const { return stage != Stage::Idle; }
    Stage getStage() const { return stage; }
    float getLevel() const { return level; }

private:
    double sr = 44100.0;
    Stage stage = Stage::Idle;
    float level = 0.0f;
    float attackInc = 0.01f;
    float decayInc = 0.01f;
    float sustainLevel = 0.7f;
    float releaseInc = 0.01f;
};
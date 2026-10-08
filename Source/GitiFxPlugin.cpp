#include "GitiFxProcessor.h"
#include "GitiFxEditor.h"
juce::AudioProcessorEditor* GitiFxAudioProcessor::createEditor(){return new GitiFxEditor(*this);}

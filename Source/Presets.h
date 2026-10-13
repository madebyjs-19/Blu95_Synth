#pragma once

#include <array>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"

namespace blu
{
struct OpPreset
{
    bool on;
    int coarse, fine, detune;
    float level;
    float attack, decay, sustain, release; // seconds, seconds, 0-100, seconds
};

/** Each factory preset doubles as a short lesson on one FM idea. */
struct FactoryPreset
{
    const char* name;
    int algorithm;
    float feedback, velBright, velVolume;
    std::array<OpPreset, kNumOps> ops;
    const char* lessonKo;
    const char* lessonEn;
};

const std::vector<FactoryPreset>& getFactoryPresets();

void applyPreset (const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state);
}

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace blu
{
constexpr int kNumOps        = 4;
constexpr int kNumAlgorithms = 8;
constexpr int kMaxCoarse     = 16;
constexpr float kCentsPerDetuneStep = 2.0f;

namespace pid
{
    inline constexpr const char* algorithm = "algorithm";
    inline constexpr const char* feedback  = "feedback";
    inline constexpr const char* volume    = "volume";
    inline constexpr const char* velBright = "velBright";
    inline constexpr const char* velVolume = "velVolume";
}

enum class OpParam { on, coarse, fine, detune, level, attack, decay, sustain, release, count };

constexpr int kNumOpParams = static_cast<int> (OpParam::count);

const char* opParamSuffix (OpParam p);
juce::String opParamID (int opIndex, OpParam p);

/** Splits an id like "op2_level" into op index 1 and OpParam::level. */
bool parseOpParamID (const juce::String& id, int& opIndex, OpParam& p);

/** Coarse 0 means 0.5, otherwise the integer ratio. Fine adds 0.00 - 0.99. */
float ratioFrom (int coarse, int fine);

/** Level 0-100 -> linear amplitude (squared curve). */
inline float levelToAmp (float level) { const float x = juce::jlimit (0.0f, 100.0f, level) / 100.0f; return x * x; }

juce::String formatTime (float seconds);

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}

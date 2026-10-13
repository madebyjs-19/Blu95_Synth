#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Algorithms.h"

namespace blu
{
struct OpSettings
{
    bool on = true;
    float ratio = 1.0f;
    float detune = 1.0f;     // frequency multiplier
    float level = 0.0f;      // 0-100
    float attack = 0.005f, decay = 1.0f, sustain = 1.0f, release = 0.3f;
};

/** Snapshot of all parameters, refreshed by the processor at the start of every block. */
struct SynthSettings
{
    int algorithm = 1;
    float feedback = 0.0f;   // 0-1
    float velBright = 0.0f;  // 0-1
    float velVolume = 0.0f;  // 0-1
    std::array<OpSettings, kNumOps> ops;
};

/** ADSR with a linear attack and exponential decay/release, like most FM envelopes sound. */
class Envelope
{
public:
    enum class Stage { idle, attack, decay, sustain, release };

    void setSampleRate (double sr) noexcept { sampleRate = static_cast<float> (sr); }
    void setParameters (float attackSec, float decaySec, float sustainLevel, float releaseSec) noexcept;

    void noteOn() noexcept   { stage = Stage::attack; }
    void noteOff() noexcept  { if (stage != Stage::idle) stage = Stage::release; }
    void reset() noexcept    { stage = Stage::idle; level = 0.0f; }

    bool isActive() const noexcept { return stage != Stage::idle; }
    float next() noexcept;

private:
    float coefFor (float seconds) const noexcept;

    Stage stage = Stage::idle;
    float sampleRate = 44100.0f;
    float level = 0.0f;
    float attackInc = 1.0f, decayCoef = 0.0f, sustain = 1.0f, releaseCoef = 0.0f;
};

class FMSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class FMVoice : public juce::SynthesiserVoice
{
public:
    explicit FMVoice (const SynthSettings& settingsToUse) : settings (settingsToUse) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<FMSound*> (s) != nullptr; }
    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int, int) override {}
    void setCurrentPlaybackSampleRate (double newRate) override;

    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>& output, int startSample, int numSamples) override;

    static constexpr float maxFeedback = 0.4f;  // in cycles
    static constexpr float voiceGain   = 0.4f;

private:
    void resetState() noexcept;

    const SynthSettings& settings;
    std::array<Envelope, kNumOps> envelopes;
    std::array<float, kNumOps> phases {};
    float noteHz = 440.0f, velocity = 1.0f, bend = 1.0f;
    float feedback1 = 0.0f, feedback2 = 0.0f;
};

/** Sine of a phase given in cycles (1.0 = one full turn), via a lookup table. */
float sineCycles (float cycles) noexcept;
}

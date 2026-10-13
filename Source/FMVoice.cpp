#include "FMVoice.h"

namespace blu
{
namespace
{
    constexpr int tableSize = 4096;

    struct SineTable
    {
        SineTable()
        {
            for (int i = 0; i <= tableSize; ++i)
                values[static_cast<size_t> (i)] = static_cast<float> (std::sin (juce::MathConstants<double>::twoPi * i / tableSize));
        }

        std::array<float, tableSize + 1> values {};
    };

    const SineTable sineTable;
}

float sineCycles (float cycles) noexcept
{
    const float p = cycles - std::floor (cycles);
    const float index = p * static_cast<float> (tableSize);
    const int i = static_cast<int> (index) & (tableSize - 1);
    const float frac = index - std::floor (index);
    const auto* v = sineTable.values.data();
    return v[i] + frac * (v[i + 1] - v[i]);
}

//==============================================================================
void Envelope::setParameters (float attackSec, float decaySec, float sustainLevel, float releaseSec) noexcept
{
    attackInc   = 1.0f / juce::jmax (1.0f, attackSec * sampleRate);
    decayCoef   = coefFor (decaySec);
    releaseCoef = coefFor (releaseSec);
    sustain     = juce::jlimit (0.0f, 1.0f, sustainLevel);
}

float Envelope::coefFor (float seconds) const noexcept
{
    // Reaches ~99% of the way to the target after `seconds`.
    return std::exp (-5.0f / juce::jmax (1.0f, seconds * sampleRate));
}

float Envelope::next() noexcept
{
    switch (stage)
    {
        case Stage::attack:
            level += attackInc;
            if (level >= 1.0f)
            {
                level = 1.0f;
                stage = Stage::decay;
            }
            break;

        case Stage::decay:
            level = sustain + (level - sustain) * decayCoef;
            if (std::abs (level - sustain) < 1.0e-4f)
            {
                level = sustain;
                if (sustain <= 0.0f)
                    reset();
                else
                    stage = Stage::sustain;
            }
            break;

        case Stage::sustain:
            level = sustain;   // follows the knob while the key is held
            break;

        case Stage::release:
            level *= releaseCoef;
            if (level < 1.0e-4f)
                reset();
            break;

        case Stage::idle:
            break;
    }

    return level;
}

//==============================================================================
void FMVoice::setCurrentPlaybackSampleRate (double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    for (auto& e : envelopes)
        e.setSampleRate (newRate);
}

void FMVoice::resetState() noexcept
{
    for (auto& e : envelopes)
        e.reset();
    phases.fill (0.0f);
    feedback1 = feedback2 = 0.0f;
}

void FMVoice::startNote (int midiNote, float vel, juce::SynthesiserSound*, int pitchWheelPosition)
{
    noteHz = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNote));
    velocity = vel;
    pitchWheelMoved (pitchWheelPosition);

    bool wasSounding = false;
    for (auto& e : envelopes)
        wasSounding = wasSounding || e.isActive();

    // Key sync: a fresh note always starts every operator at phase zero,
    // which makes the attack consistent (like the DX7).
    if (! wasSounding)
        resetState();

    for (auto& e : envelopes)
        e.noteOn();
}

void FMVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        for (auto& e : envelopes)
            e.noteOff();
    }
    else
    {
        resetState();
        clearCurrentNote();
    }
}

void FMVoice::pitchWheelMoved (int newValue)
{
    const float semitones = 2.0f * (static_cast<float> (newValue) - 8192.0f) / 8192.0f;
    bend = std::pow (2.0f, semitones / 12.0f);
}

void FMVoice::renderNextBlock (juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const auto& alg = getAlgorithm (settings.algorithm);
    const float sampleRate = static_cast<float> (getSampleRate());

    const float brightnessScale = 1.0f - settings.velBright * (1.0f - velocity);
    const float volumeScale     = 1.0f - settings.velVolume * (1.0f - velocity);
    const float carrierNorm     = 1.0f / std::sqrt (static_cast<float> (juce::jmax (1, alg.numCarriers())));
    const float feedbackAmount  = settings.feedback * maxFeedback * 0.5f;

    std::array<float, kNumOps> gain {}, increment {};

    for (int op = 0; op < kNumOps; ++op)
    {
        const auto& o = settings.ops[static_cast<size_t> (op)];
        envelopes[static_cast<size_t> (op)].setParameters (o.attack, o.decay, o.sustain, o.release);

        const float amp = o.on ? levelToAmp (o.level) : 0.0f;
        gain[static_cast<size_t> (op)] = amp * (alg.isCarrier (op) ? volumeScale : brightnessScale);
        increment[static_cast<size_t> (op)] = noteHz * bend * o.ratio * o.detune / sampleRate;
    }

    auto* out = output.getWritePointer (0, startSample);
    std::array<float, kNumOps> opOut {};

    for (int i = 0; i < numSamples; ++i)
    {
        float mix = 0.0f;

        for (int op = kNumOps - 1; op >= 0; --op)
        {
            const auto k = static_cast<size_t> (op);
            const float env = envelopes[k].next();

            // Phase modulation: every modulator's output (in cycles) is added to this operator's phase.
            float mod = 0.0f;
            const auto sources = alg.modulatorsOf[k];
            for (int src = op + 1; src < kNumOps; ++src)
                if ((sources >> src) & 1)
                    mod += opOut[static_cast<size_t> (src)];

            if (op == kNumOps - 1)
                mod += feedbackAmount * (feedback1 + feedback2);

            const float value = sineCycles (phases[k] + mod) * env * gain[k];
            opOut[k] = value;

            if (op == kNumOps - 1)
            {
                feedback2 = feedback1;
                feedback1 = value;
            }

            phases[k] += increment[k];
            phases[k] -= std::floor (phases[k]);

            if (alg.isCarrier (op))
                mix += value;
        }

        out[i] += mix * carrierNorm * voiceGain;
    }

    // The voice is finished once every carrier has gone quiet; modulators alone make no sound.
    bool anyCarrierActive = false;
    for (int op = 0; op < kNumOps; ++op)
        if (alg.isCarrier (op) && envelopes[static_cast<size_t> (op)].isActive())
            anyCarrierActive = true;

    if (! anyCarrierActive)
    {
        resetState();
        clearCurrentNote();
    }
}
}

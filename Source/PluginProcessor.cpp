#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace blu;

namespace
{
    constexpr int numVoices = 16;

    float volumeToGain (float volume)
    {
        if (volume <= 0.0f)
            return 0.0f;
        return juce::Decibels::decibelsToGain (-40.0f + 40.0f * volume / 100.0f);
    }
}

Blu95AudioProcessor::Blu95AudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    algorithmParam = apvts.getRawParameterValue (pid::algorithm);
    feedbackParam  = apvts.getRawParameterValue (pid::feedback);
    volumeParam    = apvts.getRawParameterValue (pid::volume);
    velBrightParam = apvts.getRawParameterValue (pid::velBright);
    velVolumeParam = apvts.getRawParameterValue (pid::velVolume);

    for (int op = 0; op < kNumOps; ++op)
        for (int p = 0; p < kNumOpParams; ++p)
            opParams[static_cast<size_t> (op)][static_cast<size_t> (p)] = apvts.getRawParameterValue (opParamID (op, static_cast<OpParam> (p)));

    for (int i = 0; i < numVoices; ++i)
        synth.addVoice (new FMVoice (settings));
    synth.addSound (new FMSound());
    synth.setNoteStealingEnabled (true);

    applyPreset (getFactoryPresets().front(), apvts);
}

void Blu95AudioProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    masterGain.reset (sampleRate, 0.05);
    masterGain.setCurrentAndTargetValue (volumeToGain (volumeParam->load()));
    dcCoefficient = 1.0f - juce::MathConstants<float>::twoPi * 10.0f / static_cast<float> (sampleRate);
    dcLastIn = dcLastOut = 0.0f;
    keyboardState.reset();
}

bool Blu95AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void Blu95AudioProcessor::updateSettings()
{
    settings.algorithm = juce::roundToInt (algorithmParam->load());
    settings.feedback  = feedbackParam->load() / 100.0f;
    settings.velBright = velBrightParam->load() / 100.0f;
    settings.velVolume = velVolumeParam->load() / 100.0f;

    for (size_t op = 0; op < kNumOps; ++op)
    {
        const auto& p = opParams[op];
        auto value = [&p] (OpParam which) { return p[static_cast<size_t> (which)]->load(); };

        auto& o = settings.ops[op];
        o.on      = value (OpParam::on) > 0.5f;
        o.ratio   = ratioFrom (juce::roundToInt (value (OpParam::coarse)), juce::roundToInt (value (OpParam::fine)));
        o.detune  = std::pow (2.0f, value (OpParam::detune) * kCentsPerDetuneStep / 1200.0f);
        o.level   = value (OpParam::level);
        o.attack  = value (OpParam::attack);
        o.decay   = value (OpParam::decay);
        o.sustain = value (OpParam::sustain) / 100.0f;
        o.release = value (OpParam::release);
    }
}

void Blu95AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    keyboardState.processNextMidiBuffer (midi, 0, numSamples, true);

    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
            lastNoteHz.store (static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (message.getNoteNumber())));
    }

    updateSettings();
    synth.renderNextBlock (buffer, midi, 0, numSamples);

    // Voices render mono into channel 0. Feedback FM produces a DC offset, so remove it.
    auto* left = buffer.getWritePointer (0);
    for (int i = 0; i < numSamples; ++i)
    {
        const float in = left[i];
        dcLastOut = in - dcLastIn + dcCoefficient * dcLastOut;
        dcLastIn = in;
        left[i] = dcLastOut;
    }

    tap.push (left, numSamples);

    masterGain.setTargetValue (volumeToGain (volumeParam->load()));
    for (int i = 0; i < numSamples; ++i)
        left[i] = juce::jlimit (-1.0f, 1.0f, left[i] * masterGain.getNextValue());

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, numSamples);
}

//==============================================================================
int Blu95AudioProcessor::getNumPrograms()
{
    return static_cast<int> (getFactoryPresets().size());
}

void Blu95AudioProcessor::setCurrentProgram (int index)
{
    const auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, static_cast<int> (presets.size())))
        return;

    currentProgram.store (index);
    applyPreset (presets[static_cast<size_t> (index)], apvts);
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

const juce::String Blu95AudioProcessor::getProgramName (int index)
{
    const auto& presets = getFactoryPresets();
    if (! juce::isPositiveAndBelow (index, static_cast<int> (presets.size())))
        return {};
    return juce::String (index + 1).paddedLeft ('0', 2) + " " + presets[static_cast<size_t> (index)].name;
}

//==============================================================================
void Blu95AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram.load(), nullptr);
    state.setProperty ("english", englishGuide.load(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void Blu95AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            currentProgram.store (juce::jlimit (0, getNumPrograms() - 1, static_cast<int> (state.getProperty ("program", 0))));
            englishGuide.store (static_cast<bool> (state.getProperty ("english", false)));
            apvts.replaceState (state);
        }
    }
}

juce::AudioProcessorEditor* Blu95AudioProcessor::createEditor()
{
    return new Blu95AudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Blu95AudioProcessor();
}

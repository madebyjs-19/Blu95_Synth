#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "AudioTap.h"
#include "FMVoice.h"

class Blu95AudioProcessor : public juce::AudioProcessor
{
public:
    Blu95AudioProcessor();
    ~Blu95AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    blu::AudioTap tap;

    std::atomic<float> lastNoteHz { 261.63f };
    std::atomic<int> currentProgram { 0 };
    std::atomic<bool> englishGuide { false };

private:
    void updateSettings();

    juce::Synthesiser synth;
    blu::SynthSettings settings;
    juce::SmoothedValue<float> masterGain;
    float dcCoefficient = 0.999f, dcLastIn = 0.0f, dcLastOut = 0.0f;

    std::atomic<float>* algorithmParam = nullptr;
    std::atomic<float>* feedbackParam  = nullptr;
    std::atomic<float>* volumeParam    = nullptr;
    std::atomic<float>* velBrightParam = nullptr;
    std::atomic<float>* velVolumeParam = nullptr;
    std::array<std::array<std::atomic<float>*, blu::kNumOpParams>, blu::kNumOps> opParams {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Blu95AudioProcessor)
};

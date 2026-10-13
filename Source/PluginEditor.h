#pragma once

#include "PluginProcessor.h"
#include "Components.h"

/** The whole front panel, laid out at a fixed design size and scaled by the editor. */
class MainPanel : public juce::Component, private juce::Timer
{
public:
    static constexpr int designWidth = 1100;
    static constexpr int designHeight = 800;

    explicit MainPanel (Blu95AudioProcessor&);
    ~MainPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void stepParameter (const juce::String& id, int delta, bool wrap);
    void selectProgram (int index);
    void updateDisplays();
    void updateLanguageButton();
    juce::String describeForLcd (const juce::String& id) const;
    juce::String hoveredInfoId() const;

    Blu95AudioProcessor& processor;
    blu::APVTS& apvts;

    blu::LedDisplay presetLed { 2 };
    juce::TextButton presetPrev { "<" }, presetNext { ">" };
    blu::LcdDisplay lcd;
    blu::Knob volume, velBright, velVolume;

    blu::AlgorithmDiagram algorithm;
    blu::LedDisplay algorithmLed { 1 };
    juce::TextButton algPrev { "<" }, algNext { ">" };
    blu::Knob feedback;

    blu::Visualizer visualizer;
    juce::TextButton scopeButton { "SCOPE" }, spectrumButton { "SPECTRUM" };

    blu::GuidePanel guide;
    juce::TextButton languageButton;
    juce::String guideInfoId;
    double lastHoverTime = 0.0;

    std::array<std::unique_ptr<blu::OperatorPanel>, blu::kNumOps> operators;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainPanel)
};

class Blu95AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Blu95AudioProcessorEditor (Blu95AudioProcessor&);
    ~Blu95AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    blu::RetroLookAndFeel lookAndFeel;
    MainPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Blu95AudioProcessorEditor)
};

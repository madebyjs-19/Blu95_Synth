#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "AudioTap.h"
#include "Parameters.h"
#include "RetroLookAndFeel.h"

namespace blu
{
using APVTS = juce::AudioProcessorValueTreeState;

/** Marks a component so the editor can show help for it when hovered. */
inline void setInfoId (juce::Component& c, const juce::String& id) { c.getProperties().set ("infoId", id); }

//==============================================================================
/** Rotary knob with a label above and the current value below. */
class Knob : public juce::Component
{
public:
    Knob (APVTS& state, const juce::String& paramId, const juce::String& label, juce::Colour accent);

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Slider slider;

private:
    juce::String label;
    std::unique_ptr<APVTS::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Knob)
};

//==============================================================================
/** Two-line backlit LCD, like the DX7's 2x16 character display. */
class LcdDisplay : public juce::Component
{
public:
    void setLines (const juce::String& top, const juce::String& bottom);
    void paint (juce::Graphics&) override;

private:
    juce::String line1, line2;
};

//==============================================================================
/** Red seven-segment LED number display. */
class LedDisplay : public juce::Component
{
public:
    explicit LedDisplay (int numDigits = 2) : digits (numDigits) {}
    void setNumber (int n);
    void paint (juce::Graphics&) override;

private:
    int digits;
    int number = 0;
};

//==============================================================================
/** Draws the current algorithm: operators as boxes, carriers in teal, modulators in maroon. */
class AlgorithmDiagram : public juce::Component, private juce::Timer
{
public:
    explicit AlgorithmDiagram (APVTS& state);

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    std::function<void()> onClick;

private:
    void timerCallback() override;
    int readState() const;

    APVTS& apvts;
    int lastState = -1;
};

//==============================================================================
/** Small picture of an operator's ADSR shape. */
class EnvelopeGraph : public juce::Component, private juce::Timer
{
public:
    EnvelopeGraph (APVTS& state, int opIndex);
    void paint (juce::Graphics&) override;
    void setAccent (juce::Colour c) { accent = c; repaint(); }

private:
    void timerCallback() override;

    APVTS& apvts;
    int op;
    std::array<float, 5> values {};
    juce::Colour accent = palette::teal;
};

//==============================================================================
/** Oscilloscope and harmonic spectrum of the synth output. */
class Visualizer : public juce::Component, private juce::Timer
{
public:
    enum class Mode { scope, spectrum };

    Visualizer (const AudioTap& tap, std::function<double()> sampleRate, const std::atomic<float>& noteHz);

    void setMode (Mode m) { mode = m; repaint(); }
    Mode getMode() const { return mode; }

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    void paintScope (juce::Graphics&, juce::Rectangle<float> area);
    void paintSpectrum (juce::Graphics&, juce::Rectangle<float> area);

    static constexpr int fftOrder = 13;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numHarmonics = 16;
    static constexpr float floorDb = -72.0f;

    const AudioTap& tap;
    std::function<double()> getSampleRate;
    const std::atomic<float>& lastNoteHz;

    Mode mode = Mode::spectrum;
    std::vector<float> samples;
    std::vector<float> fftData;
    std::vector<float> spectrumDb;    // per pixel column, smoothed
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { static_cast<size_t> (fftSize), juce::dsp::WindowingFunction<float>::hann, false };
};

//==============================================================================
/** Text panel for lessons and help. */
class GuidePanel : public juce::Component
{
public:
    void setText (const juce::String& title, const juce::String& body);
    void paint (juce::Graphics&) override;

private:
    juce::String title, body;
};

//==============================================================================
/** One operator column: on/off, ratio, fine, detune, level, envelope. */
class OperatorPanel : public juce::Component
{
public:
    OperatorPanel (APVTS& state, int opIndex);

    /** carrier = heard directly; targets = bitmask of the operators this one modulates. */
    void setRole (bool isCarrier, uint8_t targets);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int op;
    bool carrier = true;
    uint8_t targetMask = 0;

    juce::TextButton onButton { "ON" };
    std::unique_ptr<APVTS::ButtonAttachment> onAttachment;

    Knob ratio, fine, detune, level, attack, decay, sustain, release;
    EnvelopeGraph envelope;
};
}

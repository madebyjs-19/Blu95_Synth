#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace blu
{
/** Colours borrowed from the DX7 front panel, on a toned-down slate navy body:
    membrane buttons in maroon, teal-green and cream, a green backlit LCD and a red LED. */
namespace palette
{
    inline const juce::Colour body       { 0xff2b3546 };
    inline const juce::Colour bodyDark   { 0xff1c1816 };
    inline const juce::Colour section    { 0xff344055 };
    inline const juce::Colour line       { 0xff4a5870 };
    inline const juce::Colour cream      { 0xffe9dec3 };
    inline const juce::Colour creamDim   { 0xffa89f8a };
    inline const juce::Colour maroon     { 0xff9b3b55 };
    inline const juce::Colour teal       { 0xff4f9e8d };
    inline const juce::Colour lcdBack    { 0xffa6c06a };
    inline const juce::Colour lcdText    { 0xff1d2912 };
    inline const juce::Colour ledOn      { 0xffff3b2e };
    inline const juce::Colour ledOff     { 0xff3a1512 };
    inline const juce::Colour screen     { 0xff131922 };
}

juce::Font uiFont (float height, bool bold = true);
juce::Font lcdFont (float height);

class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool isHighlighted, bool isDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isHighlighted, bool isDown) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    /** Makes a TextButton look like a DX7 membrane switch in the given colour.
        With `withLed` a small LED shows the toggle state. */
    static void styleMembrane (juce::TextButton& button, juce::Colour colour, bool withLed = false);
};
}

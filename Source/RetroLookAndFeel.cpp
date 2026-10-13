#include "RetroLookAndFeel.h"

namespace blu
{
juce::Font uiFont (float height, bool bold)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Font lcdFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::bold));
}

RetroLookAndFeel::RetroLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, palette::body);
    setColour (juce::Slider::rotarySliderFillColourId, palette::cream);
    setColour (juce::TextButton::buttonColourId, palette::cream);
    setColour (juce::TextButton::buttonOnColourId, palette::cream);
    setColour (juce::TextButton::textColourOffId, palette::bodyDark);
    setColour (juce::TextButton::textColourOnId, palette::bodyDark);
}

void RetroLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 3.0f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Scale ring
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
    g.setColour (palette::bodyDark);
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, startAngle, angle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (value, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob cap
    const float capRadius = radius - 6.0f;
    const auto cap = juce::Rectangle<float> (capRadius * 2.0f, capRadius * 2.0f).withCentre (centre);

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (cap.translated (0.0f, 2.0f));

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a413b), cap.getX(), cap.getY(),
                                             juce::Colour (0xff171412), cap.getRight(), cap.getBottom(), false));
    g.fillEllipse (cap);
    g.setColour (juce::Colour (0xff5a5048));
    g.drawEllipse (cap.reduced (0.5f), 1.0f);

    // Pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -capRadius + 2.5f, 3.0f, capRadius * 0.6f, 1.5f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (palette::cream);
    g.fillPath (pointer);
}

void RetroLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool isHighlighted, bool isDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (1.5f);
    const float corner = 2.5f;

    auto base = backgroundColour;
    if (! button.isEnabled())
        base = base.withMultipliedSaturation (0.3f).darker (0.4f);
    else if (isDown)
        base = base.darker (0.25f);
    else if (isHighlighted)
        base = base.brighter (0.08f);

    if (! isDown)
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (r.translated (0.0f, 1.5f), corner);
    }
    else
    {
        r.translate (0.0f, 1.0f);
    }

    g.setGradientFill (juce::ColourGradient (base.brighter (0.12f), r.getX(), r.getY(),
                                             base.darker (0.15f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, corner);

    // Membrane highlight line along the top edge
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawHorizontalLine (juce::roundToInt (r.getY() + 1.0f), r.getX() + corner, r.getRight() - corner);

    g.setColour (base.darker (0.5f));
    g.drawRoundedRectangle (r, corner, 1.0f);

    if (static_cast<bool> (button.getProperties()["led"]))
    {
        const auto led = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ r.getX() + 9.0f, r.getCentreY() });
        const bool on = button.getToggleState();

        if (on)
        {
            g.setColour (palette::ledOn.withAlpha (0.35f));
            g.fillEllipse (led.expanded (3.0f));
        }
        g.setColour (on ? palette::ledOn : palette::ledOff);
        g.fillEllipse (led);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (led, 0.8f);
    }
}

juce::Font RetroLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return uiFont (juce::jmin (13.0f, static_cast<float> (buttonHeight) * 0.5f));
}

void RetroLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool isDown)
{
    const auto background = button.findColour (button.getToggleState() ? juce::TextButton::buttonOnColourId
                                                                        : juce::TextButton::buttonColourId);
    const auto textColour = background.getPerceivedBrightness() > 0.55f ? palette::bodyDark : palette::cream;

    auto area = button.getLocalBounds();
    if (static_cast<bool> (button.getProperties()["led"]))
        area.removeFromLeft (14);
    if (isDown)
        area.translate (0, 1);

    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.setColour (textColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
    g.drawText (button.getButtonText(), area, juce::Justification::centred, false);
}

void RetroLookAndFeel::styleMembrane (juce::TextButton& button, juce::Colour colour, bool withLed)
{
    button.setColour (juce::TextButton::buttonColourId, colour);
    button.setColour (juce::TextButton::buttonOnColourId, colour);
    button.getProperties().set ("led", withLed);
}
}

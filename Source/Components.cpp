#include "Components.h"
#include "Algorithms.h"

namespace blu
{
//==============================================================================
Knob::Knob (APVTS& state, const juce::String& paramId, const juce::String& labelText, juce::Colour accent)
    : label (labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (180);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    addAndMakeVisible (slider);

    attachment = std::make_unique<APVTS::SliderAttachment> (state, paramId, slider);

    if (auto* p = state.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));

    slider.onValueChange = [this] { repaint(); };
    setInfoId (*this, paramId);
}

void Knob::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (palette::cream);
    g.setFont (uiFont (11.5f));
    g.drawText (label, area.removeFromTop (14), juce::Justification::centred, false);

    g.setColour (palette::creamDim);
    g.setFont (uiFont (11.0f, false));
    g.drawText (slider.getTextFromValue (slider.getValue()), area.removeFromBottom (14), juce::Justification::centred, false);
}

void Knob::resized()
{
    slider.setBounds (getLocalBounds().withTrimmedTop (14).withTrimmedBottom (14));
}

//==============================================================================
void LcdDisplay::setLines (const juce::String& top, const juce::String& bottom)
{
    if (top != line1 || bottom != line2)
    {
        line1 = top;
        line2 = bottom;
        repaint();
    }
}

void LcdDisplay::paint (juce::Graphics& g)
{
    auto bezel = getLocalBounds().toFloat();
    g.setColour (juce::Colours::black);
    g.fillRoundedRectangle (bezel, 6.0f);

    auto glass = bezel.reduced (8.0f);
    g.setGradientFill (juce::ColourGradient (palette::lcdBack.brighter (0.12f), glass.getCentreX(), glass.getY(),
                                             palette::lcdBack.darker (0.18f), glass.getCentreX(), glass.getBottom(), false));
    g.fillRoundedRectangle (glass, 3.0f);

    // Faint dot-matrix texture
    g.setColour (palette::lcdText.withAlpha (0.045f));
    for (float x = glass.getX() + 2.0f; x < glass.getRight(); x += 3.0f)
        g.drawVerticalLine (juce::roundToInt (x), glass.getY(), glass.getBottom());

    auto text = glass.reduced (14.0f, 6.0f);
    const float lineHeight = text.getHeight() / 2.0f;
    g.setFont (lcdFont (lineHeight * 0.66f));
    g.setColour (palette::lcdText);
    g.drawText (line1, text.removeFromTop (lineHeight), juce::Justification::centredLeft, false);
    g.drawText (line2, text, juce::Justification::centredLeft, false);
}

//==============================================================================
void LedDisplay::setNumber (int n)
{
    if (n != number)
    {
        number = n;
        repaint();
    }
}

namespace
{
    juce::Path horizontalSegment (float x, float y, float length, float t)
    {
        juce::Path p;
        p.startNewSubPath (x, y);
        p.lineTo (x + t * 0.5f, y - t * 0.5f);
        p.lineTo (x + length - t * 0.5f, y - t * 0.5f);
        p.lineTo (x + length, y);
        p.lineTo (x + length - t * 0.5f, y + t * 0.5f);
        p.lineTo (x + t * 0.5f, y + t * 0.5f);
        p.closeSubPath();
        return p;
    }

    juce::Path verticalSegment (float x, float y, float length, float t)
    {
        juce::Path p;
        p.startNewSubPath (x, y);
        p.lineTo (x + t * 0.5f, y + t * 0.5f);
        p.lineTo (x + t * 0.5f, y + length - t * 0.5f);
        p.lineTo (x, y + length);
        p.lineTo (x - t * 0.5f, y + length - t * 0.5f);
        p.lineTo (x - t * 0.5f, y + t * 0.5f);
        p.closeSubPath();
        return p;
    }

    void drawSevenSegmentDigit (juce::Graphics& g, juce::Rectangle<float> r, int value)
    {
        static const uint8_t masks[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
        const uint8_t mask = value < 0 ? 0 : masks[value % 10];

        const float w = r.getWidth(), h = r.getHeight(), x = r.getX(), y = r.getY();
        const float t = w * 0.2f, gap = 1.2f;
        const float half = h * 0.5f - t * 0.5f - 2.0f * gap;

        const juce::Path segments[7] = {
            horizontalSegment (x + gap, y + t * 0.5f, w - 2.0f * gap, t),                  // a
            verticalSegment (x + w - t * 0.5f, y + t * 0.5f + gap, half, t),               // b
            verticalSegment (x + w - t * 0.5f, y + h * 0.5f + gap, half, t),               // c
            horizontalSegment (x + gap, y + h - t * 0.5f, w - 2.0f * gap, t),              // d
            verticalSegment (x + t * 0.5f, y + h * 0.5f + gap, half, t),                   // e
            verticalSegment (x + t * 0.5f, y + t * 0.5f + gap, half, t),                   // f
            horizontalSegment (x + gap, y + h * 0.5f, w - 2.0f * gap, t),                  // g
        };

        const auto italic = juce::AffineTransform::translation (-r.getCentreX(), -r.getCentreY())
                                .sheared (-0.1f, 0.0f)
                                .translated (r.getCentreX(), r.getCentreY());

        for (int i = 0; i < 7; ++i)
        {
            const bool lit = (mask >> i) & 1;
            if (lit)
            {
                g.setColour (palette::ledOn.withAlpha (0.25f));
                g.strokePath (segments[i], juce::PathStrokeType (3.0f), italic);
            }
            g.setColour (lit ? palette::ledOn : palette::ledOff);
            g.fillPath (segments[i], italic);
        }
    }
}

void LedDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0d0908));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colour (0xff3d1612).withAlpha (0.6f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

    auto inner = r.reduced (r.getHeight() * 0.18f, r.getHeight() * 0.16f);
    const float digitWidth = inner.getWidth() / static_cast<float> (digits);

    int divisor = 1;
    for (int i = 1; i < digits; ++i)
        divisor *= 10;

    for (int i = 0; i < digits; ++i)
    {
        const int value = (number / divisor) % 10;
        drawSevenSegmentDigit (g, inner.removeFromLeft (digitWidth).reduced (digitWidth * 0.12f, 0.0f), value);
        divisor = juce::jmax (1, divisor / 10);
    }
}

//==============================================================================
namespace
{
    // Grid positions (column, row) of OP1-OP4 for each algorithm. Row 0 holds the carriers.
    const juce::Point<float> diagramLayouts[kNumAlgorithms][kNumOps] = {
        { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 2.0f }, { 0.0f, 3.0f } },
        { { 0.5f, 0.0f }, { 0.5f, 1.0f }, { 0.0f, 2.0f }, { 1.0f, 2.0f } },
        { { 0.5f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 2.0f }, { 1.0f, 1.0f } },
        { { 0.5f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 2.0f } },
        { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 2.0f, 0.0f }, { 1.0f, 1.0f } },
        { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 2.0f, 0.0f }, { 2.0f, 1.0f } },
        { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 2.0f, 0.0f }, { 3.0f, 0.0f } },
    };

    void drawArrowHead (juce::Graphics& g, float x, float tipY, float size)
    {
        juce::Path p;
        p.addTriangle (x - size, tipY - size * 1.4f, x + size, tipY - size * 1.4f, x, tipY);
        g.fillPath (p);
    }
}

AlgorithmDiagram::AlgorithmDiagram (APVTS& state) : apvts (state)
{
    setInfoId (*this, pid::algorithm);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    startTimerHz (15);
}

int AlgorithmDiagram::readState() const
{
    int s = juce::roundToInt (apvts.getRawParameterValue (pid::algorithm)->load());
    for (int op = 0; op < kNumOps; ++op)
        if (apvts.getRawParameterValue (opParamID (op, OpParam::on))->load() > 0.5f)
            s |= 1 << (4 + op);
    if (apvts.getRawParameterValue (pid::feedback)->load() > 0.0f)
        s |= 1 << 8;
    return s;
}

void AlgorithmDiagram::timerCallback()
{
    const int s = readState();
    if (s != lastState)
    {
        lastState = s;
        repaint();
    }
}

void AlgorithmDiagram::mouseUp (const juce::MouseEvent& e)
{
    if (! e.mouseWasDraggedSinceMouseDown() && onClick)
        onClick();
}

void AlgorithmDiagram::paint (juce::Graphics& g)
{
    const int state = readState();
    const int number = state & 0xf;
    const auto& alg = getAlgorithm (number);
    const auto* layout = diagramLayouts[number - 1];
    const bool feedbackOn = (state >> 8) & 1;

    auto area = getLocalBounds().toFloat();
    g.setColour (palette::screen);
    g.fillRoundedRectangle (area, 4.0f);
    area = area.reduced (10.0f, 8.0f);

    float minCol = 99.0f, maxCol = -99.0f, maxRow = 0.0f;
    for (int op = 0; op < kNumOps; ++op)
    {
        minCol = juce::jmin (minCol, layout[op].x);
        maxCol = juce::jmax (maxCol, layout[op].x);
        maxRow = juce::jmax (maxRow, layout[op].y);
    }

    // Vertical budget: feedback loop on top, rows of operators, then the output bus and arrow.
    const float loopSpace = 20.0f, busGap = 10.0f, outSpace = 18.0f, rowGap = 14.0f;
    const float available = area.getHeight() - loopSpace - busGap - outSpace;
    const float box = juce::jmin (32.0f, (available - rowGap * maxRow) / (maxRow + 1.0f));
    const float spacingY = maxRow > 0.0f ? juce::jmin (54.0f, (available - box) / maxRow) : 0.0f;
    const float contentHeight = loopSpace + maxRow * spacingY + box + busGap + outSpace;
    const float bottomRowY = area.getCentreY() - contentHeight * 0.5f + loopSpace + maxRow * spacingY + box * 0.5f;
    const float busY = bottomRowY + box * 0.5f + busGap;

    const float colSpan = maxCol - minCol;
    const float spacingX = colSpan > 0.0f ? juce::jmin (60.0f, (area.getWidth() - box - 24.0f) / colSpan) : 0.0f;
    const float midCol = (minCol + maxCol) * 0.5f;

    auto centreOf = [&] (int op)
    {
        return juce::Point<float> (area.getCentreX() + (layout[op].x - midCol) * spacingX,
                                   bottomRowY - layout[op].y * spacingY);
    };

    const auto wire = palette::cream.withAlpha (0.75f);
    const juce::PathStrokeType stroke (1.6f);

    // Modulation connections
    g.setColour (wire);
    for (int dst = 0; dst < kNumOps; ++dst)
    {
        for (int src = dst + 1; src < kNumOps; ++src)
        {
            if (! ((alg.modulatorsOf[dst] >> src) & 1))
                continue;

            const auto s = centreOf (src);
            const auto d = centreOf (dst);
            const float startY = s.y + box * 0.5f;
            const float endY = d.y - box * 0.5f;
            const float midY = (startY + endY) * 0.5f;

            juce::Path p;
            p.startNewSubPath (s.x, startY);
            p.lineTo (s.x, midY);
            p.lineTo (d.x, midY);
            p.lineTo (d.x, endY - 4.0f);
            g.strokePath (p, stroke);
            drawArrowHead (g, d.x, endY, 3.5f);
        }
    }

    // Carriers to the output bus
    float busLeft = 1.0e6f, busRight = -1.0e6f;
    for (int op = 0; op < kNumOps; ++op)
    {
        if (! alg.isCarrier (op))
            continue;
        const auto c = centreOf (op);
        g.drawLine (c.x, c.y + box * 0.5f, c.x, busY, 1.6f);
        busLeft = juce::jmin (busLeft, c.x);
        busRight = juce::jmax (busRight, c.x);
    }
    const float busMid = (busLeft + busRight) * 0.5f;
    g.drawLine (busLeft, busY, busRight, busY, 1.6f);
    g.drawLine (busMid, busY, busMid, busY + 10.0f, 1.6f);
    drawArrowHead (g, busMid, busY + 15.0f, 3.5f);
    g.setFont (uiFont (10.0f));
    g.drawText ("OUT", juce::Rectangle<float> (busMid + 7.0f, busY + 5.0f, 30.0f, 12.0f), juce::Justification::centredLeft, false);

    // Feedback loop on OP4
    {
        const auto c = centreOf (3);
        const float right = c.x + box * 0.5f;
        const float top = c.y - box * 0.5f;
        juce::Path loop;
        loop.startNewSubPath (right, c.y);
        loop.lineTo (right + 9.0f, c.y);
        loop.lineTo (right + 9.0f, top - 9.0f);
        loop.lineTo (c.x, top - 9.0f);
        loop.lineTo (c.x, top - 4.0f);
        g.setColour (feedbackOn ? palette::cream : palette::creamDim.withAlpha (0.35f));
        g.strokePath (loop, stroke);
        drawArrowHead (g, c.x, top, 3.0f);
        g.setFont (uiFont (9.0f));
        g.drawText ("FB", juce::Rectangle<float> (c.x + 3.0f, top - 20.0f, right + 9.0f - c.x - 3.0f, 10.0f), juce::Justification::centred, false);
    }

    // Operator boxes
    for (int op = 0; op < kNumOps; ++op)
    {
        const auto r = juce::Rectangle<float> (box, box).withCentre (centreOf (op));
        const bool on = (state >> (4 + op)) & 1;
        const auto colour = alg.isCarrier (op) ? palette::teal : palette::maroon;

        if (on)
        {
            g.setGradientFill (juce::ColourGradient (colour.brighter (0.15f), r.getX(), r.getY(),
                                                     colour.darker (0.2f), r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, 3.0f);
            g.setColour (colour.darker (0.6f));
            g.drawRoundedRectangle (r, 3.0f, 1.0f);
            g.setColour (palette::cream);
        }
        else
        {
            g.setColour (palette::screen);
            g.fillRoundedRectangle (r, 3.0f);
            g.setColour (colour.withAlpha (0.7f));
            g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.2f);
        }

        g.setFont (uiFont (15.0f));
        g.drawText (juce::String (op + 1), r, juce::Justification::centred, false);
    }
}

//==============================================================================
EnvelopeGraph::EnvelopeGraph (APVTS& state, int opIndex) : apvts (state), op (opIndex)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (15);
}

void EnvelopeGraph::timerCallback()
{
    const OpParam ids[] = { OpParam::attack, OpParam::decay, OpParam::sustain, OpParam::release, OpParam::level };
    std::array<float, 5> now {};
    for (size_t i = 0; i < now.size(); ++i)
        now[i] = apvts.getRawParameterValue (opParamID (op, ids[i]))->load();

    if (now != values)
    {
        values = now;
        repaint();
    }
}

void EnvelopeGraph::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (palette::screen);
    g.fillRoundedRectangle (r, 3.0f);

    auto a = r.reduced (8.0f, 7.0f);

    auto proportion = [this] (OpParam p, float v)
    {
        auto* param = apvts.getParameter (opParamID (op, p));
        return param != nullptr ? param->convertTo0to1 (v) : 0.0f;
    };

    const float wA = 0.03f + 0.22f * proportion (OpParam::attack,  values[0]);
    const float wD = 0.03f + 0.22f * proportion (OpParam::decay,   values[1]);
    const float wS = 0.18f;
    const float wR = 0.03f + 0.22f * proportion (OpParam::release, values[3]);
    const float scale = a.getWidth() / (wA + wD + wS + wR);
    const float sustain = values[2] / 100.0f;
    const float peak = 0.15f + 0.85f * values[4] / 100.0f;

    const float x0 = a.getX();
    const float xA = x0 + wA * scale;
    const float xD = xA + wD * scale;
    const float xS = xD + wS * scale;
    const float xR = xS + wR * scale;
    const float bottom = a.getBottom();
    const float top = bottom - a.getHeight() * peak;
    const float sY = bottom - (bottom - top) * sustain;

    juce::Path p;
    p.startNewSubPath (x0, bottom);
    p.lineTo (xA, top);
    p.quadraticTo (xA + (xD - xA) * 0.15f, sY, xD, sY);
    p.lineTo (xS, sY);
    p.quadraticTo (xS + (xR - xS) * 0.15f, bottom, xR, bottom);

    juce::Path fill (p);
    fill.closeSubPath();
    g.setColour (accent.withAlpha (0.22f));
    g.fillPath (fill);
    g.setColour (accent.brighter (0.3f));
    g.strokePath (p, juce::PathStrokeType (1.6f));

    // Key-up marker
    g.setColour (palette::creamDim.withAlpha (0.35f));
    const float dash[] = { 2.0f, 3.0f };
    g.drawDashedLine (juce::Line<float> (xS, a.getY(), xS, bottom), dash, 2, 1.0f);

    g.setFont (uiFont (9.0f, false));
    g.setColour (palette::creamDim);
    g.drawText ("KEY UP", juce::Rectangle<float> (xS + 3.0f, a.getY() - 2.0f, 50.0f, 10.0f), juce::Justification::centredLeft, false);
}

//==============================================================================
Visualizer::Visualizer (const AudioTap& audioTap, std::function<double()> sampleRate, const std::atomic<float>& noteHz)
    : tap (audioTap), getSampleRate (std::move (sampleRate)), lastNoteHz (noteHz)
{
    samples.resize (static_cast<size_t> (AudioTap::size));
    fftData.resize (static_cast<size_t> (fftSize * 2));
    setInfoId (*this, "visualizer");
    startTimerHz (30);
}

void Visualizer::timerCallback()
{
    repaint();
}

void Visualizer::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (palette::screen);
    g.fillRoundedRectangle (r, 4.0f);

    auto area = r.reduced (10.0f, 8.0f);
    if (mode == Mode::scope)
        paintScope (g, area);
    else
        paintSpectrum (g, area);
}

void Visualizer::paintScope (juce::Graphics& g, juce::Rectangle<float> area)
{
    const double sr = juce::jmax (8000.0, getSampleRate());
    const float f0 = juce::jmax (20.0f, lastNoteHz.load());
    const int period = juce::jlimit (8, 4000, static_cast<int> (sr / f0));
    const int n = juce::jlimit (64, 8192, period * 3);
    const int total = juce::jmin (AudioTap::size, n + period * 2 + 2);

    tap.copyLatest (samples.data(), total);

    // Trigger on a rising zero crossing so a steady tone stands still.
    int start = total - n;
    for (int i = total - n; i > juce::jmax (1, total - n - period * 2); --i)
    {
        if (samples[static_cast<size_t> (i - 1)] < 0.0f && samples[static_cast<size_t> (i)] >= 0.0f)
        {
            start = i;
            break;
        }
    }

    float peak = 0.0f;
    for (int i = start; i < start + n; ++i)
        peak = juce::jmax (peak, std::abs (samples[static_cast<size_t> (i)]));

    // Grid
    g.setColour (palette::line.withAlpha (0.5f));
    for (int i = 1; i < 3; ++i)
        g.drawVerticalLine (juce::roundToInt (area.getX() + area.getWidth() * static_cast<float> (i) / 3.0f), area.getY(), area.getBottom());
    g.drawHorizontalLine (juce::roundToInt (area.getCentreY()), area.getX(), area.getRight());

    g.setFont (uiFont (10.0f));
    g.setColour (palette::creamDim);
    g.drawText ("3 CYCLES  /  AUTO GAIN", area.removeFromTop (12.0f), juce::Justification::centredRight, false);

    if (peak < 1.0e-4f)
        return;

    const float gain = 0.9f / peak;
    const float halfHeight = area.getHeight() * 0.5f;

    juce::Path wave;
    for (int i = 0; i < n; ++i)
    {
        const float x = area.getX() + area.getWidth() * static_cast<float> (i) / static_cast<float> (n - 1);
        const float y = area.getCentreY() - samples[static_cast<size_t> (start + i)] * gain * halfHeight;
        if (i == 0)
            wave.startNewSubPath (x, y);
        else
            wave.lineTo (x, y);
    }

    g.setColour (palette::teal.withAlpha (0.3f));
    g.strokePath (wave, juce::PathStrokeType (4.0f));
    g.setColour (palette::teal.brighter (0.5f));
    g.strokePath (wave, juce::PathStrokeType (1.5f));
}

void Visualizer::paintSpectrum (juce::Graphics& g, juce::Rectangle<float> area)
{
    const double sr = juce::jmax (8000.0, getSampleRate());
    const float f0 = juce::jmax (20.0f, lastNoteHz.load());
    const float nyquist = static_cast<float> (sr * 0.5);
    const float maxFreq = juce::jmin (f0 * (static_cast<float> (numHarmonics) + 0.5f), nyquist);
    const float binHz = static_cast<float> (sr) / static_cast<float> (fftSize);
    const float reference = static_cast<float> (fftSize) / 4.0f; // Hann window: a full-scale sine peaks at N/4

    tap.copyLatest (fftData.data(), fftSize);
    std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);
    window.multiplyWithWindowingTable (fftData.data(), static_cast<size_t> (fftSize));
    fft.performFrequencyOnlyForwardTransform (fftData.data(), true);

    auto labels = area.removeFromBottom (14.0f);
    auto header = area.removeFromTop (12.0f);

    const int width = juce::jmax (1, static_cast<int> (area.getWidth()));
    if (static_cast<int> (spectrumDb.size()) != width)
        spectrumDb.assign (static_cast<size_t> (width), floorDb);

    for (int x = 0; x < width; ++x)
    {
        const float fa = maxFreq * static_cast<float> (x) / static_cast<float> (width);
        const float fb = maxFreq * static_cast<float> (x + 1) / static_cast<float> (width);
        const int binA = juce::jlimit (0, fftSize / 2, static_cast<int> (fa / binHz));
        const int binB = juce::jlimit (binA, fftSize / 2, static_cast<int> (fb / binHz));

        float magnitude = 0.0f;
        for (int b = binA; b <= binB; ++b)
            magnitude = juce::jmax (magnitude, fftData[static_cast<size_t> (b)]);

        const float db = juce::Decibels::gainToDecibels (magnitude / reference, floorDb);
        auto& shown = spectrumDb[static_cast<size_t> (x)];
        shown = juce::jmax (db, shown - 2.5f);
    }

    // dB grid
    g.setColour (palette::line.withAlpha (0.45f));
    for (float db = -24.0f; db > floorDb; db -= 24.0f)
        g.drawHorizontalLine (juce::roundToInt (juce::jmap (db, floorDb, 0.0f, area.getBottom(), area.getY())), area.getX(), area.getRight());

    // Harmonic markers: multiples of the played note
    g.setFont (uiFont (9.5f));
    for (int h = 1; h <= numHarmonics; ++h)
    {
        const float freq = f0 * static_cast<float> (h);
        if (freq > maxFreq)
            break;
        const float x = area.getX() + area.getWidth() * freq / maxFreq;
        g.setColour (palette::line.withAlpha (h == 1 ? 0.9f : 0.5f));
        g.drawVerticalLine (juce::roundToInt (x), area.getY(), area.getBottom());
        g.setColour (palette::creamDim);
        g.drawText (juce::String (h), juce::Rectangle<float> (x - 10.0f, labels.getY() + 1.0f, 20.0f, 12.0f), juce::Justification::centred, false);
    }

    const int noteNumber = juce::roundToInt (12.0f * std::log2 (f0 / 440.0f) + 69.0f);
    g.setColour (palette::creamDim);
    g.setFont (uiFont (10.0f));
    g.drawText ("HARMONICS OF " + juce::MidiMessage::getMidiNoteName (noteNumber, true, true, 4)
                    + "  (" + juce::String (f0, 1) + " Hz)",
                header, juce::Justification::centredRight, false);

    juce::Path outline, fill;
    fill.startNewSubPath (area.getX(), area.getBottom());
    for (int x = 0; x < width; ++x)
    {
        const float y = juce::jmap (spectrumDb[static_cast<size_t> (x)], floorDb, 0.0f, area.getBottom(), area.getY());
        const float px = area.getX() + static_cast<float> (x);
        fill.lineTo (px, y);
        if (x == 0)
            outline.startNewSubPath (px, y);
        else
            outline.lineTo (px, y);
    }
    fill.lineTo (area.getRight(), area.getBottom());
    fill.closeSubPath();

    g.setColour (palette::teal.withAlpha (0.35f));
    g.fillPath (fill);
    g.setColour (palette::teal.brighter (0.5f));
    g.strokePath (outline, juce::PathStrokeType (1.3f));
}

//==============================================================================
void GuidePanel::setText (const juce::String& newTitle, const juce::String& newBody)
{
    if (newTitle != title || newBody != body)
    {
        title = newTitle;
        body = newBody;
        repaint();
    }
}

void GuidePanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (palette::screen);
    g.fillRoundedRectangle (r, 4.0f);

    auto inner = r.reduced (11.0f, 9.0f);

    g.setColour (palette::teal.brighter (0.4f));
    g.setFont (uiFont (13.0f));
    g.drawText (title, inner.removeFromTop (18.0f), juce::Justification::centredLeft, true);
    inner.removeFromTop (5.0f);

    // Shrink the text until it fits.
    for (float size = 12.5f; size >= 9.0f; size -= 0.5f)
    {
        juce::AttributedString text;
        text.setWordWrap (juce::AttributedString::byWord);
        text.setJustification (juce::Justification::topLeft);
        text.setLineSpacing (size * 0.25f);
        text.append (body, uiFont (size, false), palette::cream);

        juce::TextLayout layout;
        layout.createLayout (text, inner.getWidth());

        if (layout.getHeight() <= inner.getHeight() || size <= 9.0f)
        {
            layout.draw (g, inner);
            break;
        }
    }
}

//==============================================================================
OperatorPanel::OperatorPanel (APVTS& state, int opIndex)
    : op (opIndex),
      ratio   (state, opParamID (opIndex, OpParam::coarse),  "RATIO",   palette::teal),
      fine    (state, opParamID (opIndex, OpParam::fine),    "FINE",    palette::teal),
      detune  (state, opParamID (opIndex, OpParam::detune),  "DETUNE",  palette::teal),
      level   (state, opParamID (opIndex, OpParam::level),   "LEVEL",   palette::teal),
      attack  (state, opParamID (opIndex, OpParam::attack),  "ATTACK",  palette::teal),
      decay   (state, opParamID (opIndex, OpParam::decay),   "DECAY",   palette::teal),
      sustain (state, opParamID (opIndex, OpParam::sustain), "SUSTAIN", palette::teal),
      release (state, opParamID (opIndex, OpParam::release), "RELEASE", palette::teal),
      envelope (state, opIndex)
{
    RetroLookAndFeel::styleMembrane (onButton, palette::cream, true);
    onButton.setClickingTogglesState (true);
    onAttachment = std::make_unique<APVTS::ButtonAttachment> (state, opParamID (op, OpParam::on), onButton);
    setInfoId (onButton, opParamID (op, OpParam::on));
    addAndMakeVisible (onButton);

    for (auto* k : { &ratio, &fine, &detune, &level, &attack, &decay, &sustain, &release })
        addAndMakeVisible (k);
    addAndMakeVisible (envelope);
}

void OperatorPanel::setRole (bool isCarrier, uint8_t targets)
{
    if (isCarrier == carrier && targets == targetMask)
        return;

    carrier = isCarrier;
    targetMask = targets;

    const auto accent = carrier ? palette::teal : palette::maroon.brighter (0.25f);
    for (auto* k : { &ratio, &fine, &detune, &level, &attack, &decay, &sustain, &release })
        k->slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    envelope.setAccent (accent);
    repaint();
}

void OperatorPanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (palette::section);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (palette::line);
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);

    auto header = getLocalBounds().reduced (8, 6).removeFromTop (26).toFloat();

    g.setColour (palette::cream);
    g.setFont (uiFont (18.0f));
    g.drawText ("OP" + juce::String (op + 1), header.removeFromLeft (44.0f), juce::Justification::centredLeft, false);

    header.removeFromRight (static_cast<float> (onButton.getWidth()) + 8.0f);
    const auto badge = header.withSizeKeepingCentre (header.getWidth(), 18.0f);
    const auto colour = carrier ? palette::teal : palette::maroon;
    g.setColour (colour);
    g.fillRoundedRectangle (badge, 3.0f);

    juce::String text;
    if (carrier)
    {
        text = "CARRIER > OUT";
    }
    else
    {
        juce::StringArray targets;
        for (int dst = 0; dst < kNumOps; ++dst)
            if ((targetMask >> dst) & 1)
                targets.add (juce::String (dst + 1));
        text = "MOD > OP" + targets.joinIntoString (",");
    }

    g.setColour (palette::cream);
    g.setFont (uiFont (11.0f));
    g.drawText (text, badge, juce::Justification::centred, false);
}

void OperatorPanel::resized()
{
    auto r = getLocalBounds().reduced (8, 6);
    auto header = r.removeFromTop (26);
    onButton.setBounds (header.removeFromRight (54).withSizeKeepingCentre (54, 22));

    r.removeFromTop (6);
    auto row1 = r.removeFromTop (78);
    auto row2 = r.removeFromBottom (78);
    r.removeFromBottom (4);

    const int w = row1.getWidth() / 4;
    for (auto* k : { &ratio, &fine, &detune, &level })
        k->setBounds (row1.removeFromLeft (w));
    for (auto* k : { &attack, &decay, &sustain, &release })
        k->setBounds (row2.removeFromLeft (w));

    envelope.setBounds (r.reduced (2, 2));
}
}

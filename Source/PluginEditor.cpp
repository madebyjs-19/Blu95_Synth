#include "PluginEditor.h"
#include "Algorithms.h"
#include "InfoText.h"
#include "Presets.h"

using namespace blu;

namespace
{
    constexpr int lcdWidth = 30;

    // Front panel layout (design coordinates)
    const juce::Rectangle<int> presetArea    { 16, 74, 250, 96 };
    const juce::Rectangle<int> lcdArea       { 278, 74, 520, 96 };
    const juce::Rectangle<int> masterArea    { 810, 74, 274, 96 };
    const juce::Rectangle<int> algorithmArea { 16, 182, 330, 230 };
    const juce::Rectangle<int> visualArea    { 358, 182, 440, 230 };
    const juce::Rectangle<int> guideArea     { 810, 182, 274, 230 };
    const int operatorTop = 424, operatorHeight = 286, operatorWidth = 259, operatorGap = 10;
    const juce::Rectangle<int> keyboardArea  { 16, 720, 1068, 66 };

    juce::String lcdLine (const juce::String& left, const juce::String& right)
    {
        return left.paddedRight (' ', juce::jmax (0, lcdWidth - right.length())) + right;
    }
}

MainPanel::MainPanel (Blu95AudioProcessor& p)
    : processor (p),
      apvts (p.apvts),
      volume (apvts, pid::volume, "VOLUME", palette::cream),
      velBright (apvts, pid::velBright, "VEL>BRT", palette::teal),
      velVolume (apvts, pid::velVolume, "VEL>VOL", palette::teal),
      algorithm (apvts),
      feedback (apvts, pid::feedback, "FEEDBACK", palette::maroon.brighter (0.3f)),
      visualizer (p.tap, [&p] { return p.getSampleRate(); }, p.lastNoteHz),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Preset / lesson selector
    addAndMakeVisible (presetLed);
    for (auto* b : { &presetPrev, &presetNext })
    {
        RetroLookAndFeel::styleMembrane (*b, palette::maroon);
        setInfoId (*b, "preset");
        addAndMakeVisible (b);
    }
    setInfoId (presetLed, "preset");
    presetPrev.onClick = [this] { selectProgram (processor.getCurrentProgram() - 1); };
    presetNext.onClick = [this] { selectProgram (processor.getCurrentProgram() + 1); };

    addAndMakeVisible (lcd);

    for (auto* k : { &volume, &velBright, &velVolume, &feedback })
        addAndMakeVisible (k);

    // Algorithm
    addAndMakeVisible (algorithm);
    algorithm.onClick = [this] { stepParameter (pid::algorithm, 1, true); };
    addAndMakeVisible (algorithmLed);
    setInfoId (algorithmLed, pid::algorithm);
    for (auto* b : { &algPrev, &algNext })
    {
        RetroLookAndFeel::styleMembrane (*b, palette::teal);
        setInfoId (*b, pid::algorithm);
        addAndMakeVisible (b);
    }
    algPrev.onClick = [this] { stepParameter (pid::algorithm, -1, true); };
    algNext.onClick = [this] { stepParameter (pid::algorithm, 1, true); };

    // Scope / spectrum
    addAndMakeVisible (visualizer);
    for (auto* b : { &scopeButton, &spectrumButton })
    {
        RetroLookAndFeel::styleMembrane (*b, palette::cream, true);
        b->setClickingTogglesState (true);
        b->setRadioGroupId (1001);
        setInfoId (*b, "visualizer");
        addAndMakeVisible (b);
    }
    spectrumButton.setToggleState (true, juce::dontSendNotification);
    scopeButton.onClick    = [this] { visualizer.setMode (Visualizer::Mode::scope); };
    spectrumButton.onClick = [this] { visualizer.setMode (Visualizer::Mode::spectrum); };

    // Guide
    addAndMakeVisible (guide);
    RetroLookAndFeel::styleMembrane (languageButton, palette::cream);
    setInfoId (languageButton, "lang");
    languageButton.onClick = [this]
    {
        processor.englishGuide.store (! processor.englishGuide.load());
        updateLanguageButton();
        updateDisplays();
    };
    addAndMakeVisible (languageButton);
    updateLanguageButton();

    for (int op = 0; op < kNumOps; ++op)
    {
        operators[static_cast<size_t> (op)] = std::make_unique<OperatorPanel> (apvts, op);
        addAndMakeVisible (*operators[static_cast<size_t> (op)]);
    }

    // On-screen keyboard
    keyboard.setAvailableRange (36, 96);
    keyboard.setScrollButtonsVisible (false);
    keyboard.setKeyPressBaseOctave (5);
    keyboard.setOctaveForMiddleC (4);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, palette::cream);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, palette::bodyDark);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff8a806c));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, palette::teal.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, palette::teal);
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.4f));
    setInfoId (keyboard, "keyboard");
    addAndMakeVisible (keyboard);

    setSize (designWidth, designHeight);
    timerCallback();
    startTimerHz (20);
}

MainPanel::~MainPanel()
{
    stopTimer();
}

void MainPanel::paint (juce::Graphics& g)
{
    g.fillAll (palette::body);

    // Header
    auto header = juce::Rectangle<int> (0, 0, designWidth, 64);
    g.setGradientFill (juce::ColourGradient (palette::body.brighter (0.08f), 0.0f, 0.0f,
                                             palette::body, 0.0f, 64.0f, false));
    g.fillRect (header);
    g.setColour (palette::line);
    g.drawHorizontalLine (63, 0.0f, static_cast<float> (designWidth));

    g.setColour (palette::cream);
    g.setFont (juce::Font (juce::FontOptions (40.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("BLU95", juce::Rectangle<int> (22, 8, 140, 48), juce::Justification::centredLeft, false);

    const auto badge = juce::Rectangle<float> (160.0f, 19.0f, 40.0f, 26.0f);
    g.setColour (palette::maroon);
    g.fillRoundedRectangle (badge, 3.0f);
    g.setColour (palette::cream);
    g.setFont (uiFont (16.0f));
    g.drawText ("FM", badge, juce::Justification::centred, false);

    g.setColour (palette::cream);
    g.setFont (uiFont (12.0f));
    g.drawText ("DIGITAL PROGRAMMABLE ALGORITHM SYNTHESIZER", juce::Rectangle<int> (216, 15, 420, 16), juce::Justification::centredLeft, false);
    g.setColour (palette::creamDim);
    g.setFont (uiFont (11.0f, false));
    g.drawText (juce::CharPointer_UTF8 ("4 OPERATORS  \xc2\xb7  8 ALGORITHMS  \xc2\xb7  12 LESSONS"),
                juce::Rectangle<int> (216, 33, 420, 16), juce::Justification::centredLeft, false);

    // A row of membrane-coloured strips, a nod to the DX7 button colour scheme
    const juce::Colour strips[] = { palette::maroon, palette::maroon, palette::teal, palette::teal, palette::cream, palette::cream };
    for (int i = 0; i < 6; ++i)
    {
        const auto r = juce::Rectangle<float> (static_cast<float> (designWidth - 16 - (6 - i) * 34), 24.0f, 28.0f, 16.0f);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 2.0f);
        g.setColour (strips[i]);
        g.fillRoundedRectangle (r, 2.0f);
    }

    auto section = [&g] (juce::Rectangle<int> r, const juce::String& title, juce::Colour accent)
    {
        g.setColour (palette::section);
        g.fillRoundedRectangle (r.toFloat(), 5.0f);
        g.setColour (palette::line);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 5.0f, 1.0f);

        if (title.isNotEmpty())
        {
            g.setColour (accent);
            g.fillRect (r.getX() + 10, r.getY() + 8, 4, 12);
            g.setColour (palette::cream);
            g.setFont (uiFont (12.0f));
            g.drawText (title, r.getX() + 20, r.getY() + 6, 220, 16, juce::Justification::centredLeft, false);
        }
    };

    section (presetArea, "PRESET  /  LESSON", palette::maroon);
    section (masterArea, "MASTER", palette::cream);
    section (algorithmArea, "ALGORITHM", palette::teal);
    section (visualArea, "SOUND VIEW", palette::teal);
    section (guideArea, "GUIDE", palette::maroon);

    // Legend under the feedback knob
    auto legend = juce::Rectangle<int> (algorithmArea.getX() + 236, algorithmArea.getBottom() - 38, 90, 30);
    g.setFont (uiFont (10.0f));
    const std::pair<juce::Colour, const char*> items[] = { { palette::teal, "CARRIER" }, { palette::maroon, "MODULATOR" } };
    for (const auto& [colour, text] : items)
    {
        auto row = legend.removeFromTop (15);
        g.setColour (colour);
        g.fillRoundedRectangle (row.removeFromLeft (10).withSizeKeepingCentre (10, 10).toFloat(), 2.0f);
        row.removeFromLeft (5);
        g.setColour (palette::creamDim);
        g.drawText (text, row, juce::Justification::centredLeft, false);
    }

    // Keyboard bezel
    g.setColour (juce::Colours::black);
    g.fillRoundedRectangle (keyboardArea.expanded (3).toFloat(), 4.0f);
}

void MainPanel::resized()
{
    // Preset
    presetLed.setBounds (presetArea.getX() + 14, presetArea.getY() + 30, 72, 54);
    presetPrev.setBounds (presetArea.getX() + 102, presetArea.getY() + 40, 62, 34);
    presetNext.setBounds (presetArea.getX() + 172, presetArea.getY() + 40, 62, 34);

    lcd.setBounds (lcdArea);

    // Master
    {
        auto r = masterArea.withTrimmedTop (22).reduced (8, 2);
        const int w = r.getWidth() / 3;
        volume.setBounds (r.removeFromLeft (w));
        velBright.setBounds (r.removeFromLeft (w));
        velVolume.setBounds (r);
    }

    // Algorithm
    algorithm.setBounds (algorithmArea.getX() + 8, algorithmArea.getY() + 26, 218, algorithmArea.getHeight() - 34);
    const int column = algorithmArea.getX() + 234;
    algorithmLed.setBounds (column + 18, algorithmArea.getY() + 26, 52, 46);
    algPrev.setBounds (column + 2, algorithmArea.getY() + 78, 40, 26);
    algNext.setBounds (column + 46, algorithmArea.getY() + 78, 40, 26);
    feedback.setBounds (column, algorithmArea.getY() + 108, 88, 80);

    // Sound view
    spectrumButton.setBounds (visualArea.getRight() - 96, visualArea.getY() + 4, 88, 20);
    scopeButton.setBounds (visualArea.getRight() - 172, visualArea.getY() + 4, 72, 20);
    visualizer.setBounds (visualArea.withTrimmedTop (28).reduced (8, 0).withTrimmedBottom (8));

    // Guide
    languageButton.setBounds (guideArea.getRight() - 60, guideArea.getY() + 4, 52, 20);
    guide.setBounds (guideArea.withTrimmedTop (28).reduced (8, 0).withTrimmedBottom (8));

    for (int op = 0; op < kNumOps; ++op)
        operators[static_cast<size_t> (op)]->setBounds (16 + op * (operatorWidth + operatorGap), operatorTop, operatorWidth, operatorHeight);

    keyboard.setBounds (keyboardArea);
    keyboard.setKeyWidth (static_cast<float> (keyboardArea.getWidth()) / 36.0f); // C2..C7 = 36 white keys
}

//==============================================================================
void MainPanel::stepParameter (const juce::String& id, int delta, bool wrap)
{
    auto* param = apvts.getParameter (id);
    if (param == nullptr)
        return;

    const auto& range = param->getNormalisableRange();
    float value = range.convertFrom0to1 (param->getValue()) + static_cast<float> (delta);

    if (wrap)
    {
        if (value > range.end)   value = range.start;
        if (value < range.start) value = range.end;
    }

    param->beginChangeGesture();
    param->setValueNotifyingHost (range.convertTo0to1 (juce::jlimit (range.start, range.end, value)));
    param->endChangeGesture();
}

void MainPanel::selectProgram (int index)
{
    const int count = processor.getNumPrograms();
    processor.setCurrentProgram ((index % count + count) % count);
    updateDisplays();
}

void MainPanel::updateLanguageButton()
{
    languageButton.setButtonText (processor.englishGuide.load() ? "ENG" : "KOR");
}

juce::String MainPanel::hoveredInfoId() const
{
    auto* c = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

    while (c != nullptr && c != this)
    {
        if (c->getProperties().contains ("infoId"))
            return c->getProperties()["infoId"].toString();
        c = c->getParentComponent();
    }

    return {};
}

juce::String MainPanel::describeForLcd (const juce::String& id) const
{
    const int algNumber = juce::roundToInt (apvts.getRawParameterValue (pid::algorithm)->load());
    const auto& alg = getAlgorithm (algNumber);

    auto opRatio = [this] (int op)
    {
        return ratioFrom (juce::roundToInt (apvts.getRawParameterValue (opParamID (op, OpParam::coarse))->load()),
                          juce::roundToInt (apvts.getRawParameterValue (opParamID (op, OpParam::fine))->load()));
    };

    juce::String value;
    if (auto* param = apvts.getParameter (id))
        value = param->getCurrentValueAsText();

    const auto top = lcdLine (getLcdName (id), value.toUpperCase());
    juce::String bottom;

    int op = -1;
    OpParam which {};

    if (parseOpParamID (id, op, which))
    {
        const bool carrier = alg.isCarrier (op);
        const auto targets = alg.targetsOf (op);
        int target = 0;
        while (target < kNumOps - 1 && ! ((targets >> target) & 1))
            ++target;

        switch (which)
        {
            case OpParam::level:
            {
                const float level = apvts.getRawParameterValue (id)->load();
                bottom = carrier ? juce::String ("CARRIER > SETS VOLUME")
                                 : "MOD INDEX = " + juce::String (levelToAmp (level) * juce::MathConstants<float>::twoPi, 2) + " RAD";
                break;
            }
            case OpParam::coarse:
            case OpParam::fine:
                bottom = carrier ? "FREQ = KEY x " + juce::String (opRatio (op), 2)
                                 : "C:M = " + juce::String (opRatio (target), 2) + " : " + juce::String (opRatio (op), 2);
                break;
            case OpParam::detune:
                bottom = "PITCH OFFSET "
                         + juce::String (juce::roundToInt (apvts.getRawParameterValue (id)->load() * kCentsPerDetuneStep)) + " CENTS";
                break;
            case OpParam::on:
                bottom = carrier ? "CARRIER - YOU HEAR IT" : "MODULATOR - SHAPES TONE";
                break;
            case OpParam::attack:
            case OpParam::decay:
            case OpParam::sustain:
            case OpParam::release:
                bottom = carrier ? "SHAPES VOLUME OVER TIME" : "SHAPES BRIGHTNESS OVER TIME";
                break;
            case OpParam::count:
                break;
        }
    }
    else if (id == pid::algorithm)   bottom = alg.shortName;
    else if (id == pid::feedback)    bottom = "OP4 MODULATES ITSELF";
    else if (id == pid::volume)      bottom = "MASTER OUTPUT LEVEL";
    else if (id == pid::velBright)   bottom = "PLAY HARDER = BRIGHTER";
    else if (id == pid::velVolume)   bottom = "PLAY HARDER = LOUDER";
    else if (id == "visualizer")     bottom = visualizer.getMode() == Visualizer::Mode::scope ? "WAVEFORM" : "HARMONIC SPECTRUM";
    else if (id == "keyboard")       bottom = "CLICK OR TYPE A W S E D F";
    else if (id == "lang")           bottom = processor.englishGuide.load() ? "GUIDE: ENGLISH" : "GUIDE: KOREAN";
    else if (id == "preset")
    {
        const int index = processor.getCurrentProgram();
        return lcdLine ("INT " + juce::String (index + 1).paddedLeft ('0', 2), "LESSON") + "\n"
               + juce::String (getFactoryPresets()[static_cast<size_t> (index)].name);
    }

    return top + "\n" + bottom;
}

void MainPanel::updateDisplays()
{
    const bool english = processor.englishGuide.load();
    const auto id = hoveredInfoId();

    const int index = processor.getCurrentProgram();
    const auto& preset = getFactoryPresets()[static_cast<size_t> (index)];
    const int algNumber = juce::roundToInt (apvts.getRawParameterValue (pid::algorithm)->load());

    // LCD
    if (id.isNotEmpty())
    {
        const auto lines = juce::StringArray::fromLines (describeForLcd (id));
        lcd.setLines (lines[0], lines[1]);
    }
    else
    {
        lcd.setLines (lcdLine ("INT " + juce::String (index + 1).paddedLeft ('0', 2) + "  " + preset.name, ""),
                      "ALG " + juce::String (algNumber) + "  " + getAlgorithm (algNumber).shortName);
    }

    // Guide: shows help for the hovered control, and keeps it while you read it.
    const double now = juce::Time::getMillisecondCounterHiRes();
    if (id.isNotEmpty() && id != "preset")
    {
        guideInfoId = id;
        lastHoverTime = now;
    }
    else if (guide.isMouseOver (true) || id == "preset")
    {
        lastHoverTime = now;
        if (id == "preset")
            guideInfoId = {};
    }
    else if (now - lastHoverTime > 1500.0)
    {
        guideInfoId = {};
    }

    const auto info = getInfo (guideInfoId, english);
    if (guideInfoId.isNotEmpty() && info.title.isNotEmpty())
    {
        guide.setText (info.title, info.body);
    }
    else
    {
        guide.setText ((english ? "LESSON " : juce::String::fromUTF8 (u8"레슨 "))
                           + juce::String (index + 1).paddedLeft ('0', 2) + "  " + preset.name,
                       juce::String::fromUTF8 (english ? preset.lessonEn : preset.lessonKo));
    }
}

void MainPanel::timerCallback()
{
    const int algNumber = juce::roundToInt (apvts.getRawParameterValue (pid::algorithm)->load());
    const auto& alg = getAlgorithm (algNumber);

    for (int op = 0; op < kNumOps; ++op)
        operators[static_cast<size_t> (op)]->setRole (alg.isCarrier (op), alg.targetsOf (op));

    algorithmLed.setNumber (algNumber);
    presetLed.setNumber (processor.getCurrentProgram() + 1);
    updateDisplays();
}

//==============================================================================
Blu95AudioProcessorEditor::Blu95AudioProcessorEditor (Blu95AudioProcessor& p)
    : AudioProcessorEditor (&p), panel (p)
{
    panel.setLookAndFeel (&lookAndFeel);
    addAndMakeVisible (panel);

    constexpr double w = MainPanel::designWidth, h = MainPanel::designHeight;
    setResizable (true, true);
    setResizeLimits (juce::roundToInt (w * 0.6), juce::roundToInt (h * 0.6), juce::roundToInt (w * 1.6), juce::roundToInt (h * 1.6));
    getConstrainer()->setFixedAspectRatio (w / h);
    setSize (MainPanel::designWidth, MainPanel::designHeight);
}

Blu95AudioProcessorEditor::~Blu95AudioProcessorEditor()
{
    panel.setLookAndFeel (nullptr);
}

void Blu95AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (palette::body);
}

void Blu95AudioProcessorEditor::resized()
{
    const float scale = static_cast<float> (getWidth()) / static_cast<float> (MainPanel::designWidth);
    panel.setTransform (juce::AffineTransform::scale (scale));
    panel.setBounds (0, 0, MainPanel::designWidth, MainPanel::designHeight);
}

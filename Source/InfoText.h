#pragma once

#include <juce_core/juce_core.h>

namespace blu
{
struct InfoEntry
{
    juce::String title;
    juce::String body;
};

/** Help text for a control, looked up by parameter id ("op2_level") or a UI id ("preset", "visualizer", ...). */
InfoEntry getInfo (const juce::String& id, bool english);

/** Short upper-case name for the LCD, e.g. "OP2 LEVEL". */
juce::String getLcdName (const juce::String& id);
}

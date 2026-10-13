#include "Parameters.h"

namespace blu
{
const char* opParamSuffix (OpParam p)
{
    switch (p)
    {
        case OpParam::on:      return "on";
        case OpParam::coarse:  return "coarse";
        case OpParam::fine:    return "fine";
        case OpParam::detune:  return "detune";
        case OpParam::level:   return "level";
        case OpParam::attack:  return "attack";
        case OpParam::decay:   return "decay";
        case OpParam::sustain: return "sustain";
        case OpParam::release: return "release";
        case OpParam::count:   break;
    }
    return "";
}

juce::String opParamID (int opIndex, OpParam p)
{
    return "op" + juce::String (opIndex + 1) + "_" + opParamSuffix (p);
}

bool parseOpParamID (const juce::String& id, int& opIndex, OpParam& p)
{
    if (! id.startsWith ("op") || id.length() < 5 || id[3] != '_')
        return false;

    const int n = id.substring (2, 3).getIntValue();
    if (n < 1 || n > kNumOps)
        return false;

    const auto suffix = id.substring (4);
    for (int i = 0; i < kNumOpParams; ++i)
    {
        if (suffix == opParamSuffix (static_cast<OpParam> (i)))
        {
            opIndex = n - 1;
            p = static_cast<OpParam> (i);
            return true;
        }
    }
    return false;
}

float ratioFrom (int coarse, int fine)
{
    const float base = coarse <= 0 ? 0.5f : static_cast<float> (coarse);
    return base + static_cast<float> (fine) / 100.0f;
}

juce::String formatTime (float seconds)
{
    if (seconds < 1.0f)
        return juce::String (juce::roundToInt (seconds * 1000.0f)) + " ms";
    return juce::String (seconds, 2) + " s";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto asInt = [] (float v, int) { return String (roundToInt (v)); };

    auto percentParam = [&] (const String& id, const String& name, float def)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
                                                      NormalisableRange<float> (0.0f, 100.0f), def,
                                                      AudioParameterFloatAttributes().withStringFromValueFunction (asInt));
    };

    auto timeParam = [] (const String& id, const String& name, float def)
    {
        NormalisableRange<float> range (0.001f, 10.0f);
        range.setSkewForCentre (0.4f);
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, range, def,
                                                      AudioParameterFloatAttributes()
                                                          .withStringFromValueFunction ([] (float v, int) { return formatTime (v); }));
    };

    layout.add (std::make_unique<AudioParameterInt> (ParameterID { pid::algorithm, 1 }, "Algorithm", 1, kNumAlgorithms, 1));
    layout.add (percentParam (pid::feedback,  "Feedback", 0.0f));
    layout.add (percentParam (pid::volume,    "Volume", 80.0f));
    layout.add (percentParam (pid::velBright, "Velocity > Brightness", 30.0f));
    layout.add (percentParam (pid::velVolume, "Velocity > Volume", 40.0f));

    for (int op = 0; op < kNumOps; ++op)
    {
        const String prefix = "OP" + String (op + 1) + " ";

        // A two-state choice rather than AudioParameterBool, which keeps fractional values from hosts
        // and then fails to round-trip through saved state.
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { opParamID (op, OpParam::on), 1 }, prefix + "On",
                                                            StringArray { "Off", "On" }, 1));

        layout.add (std::make_unique<AudioParameterInt> (ParameterID { opParamID (op, OpParam::coarse), 1 }, prefix + "Ratio",
                                                         0, kMaxCoarse, 1,
                                                         AudioParameterIntAttributes().withStringFromValueFunction (
                                                             [] (int v, int) { return v == 0 ? String ("0.5") : String (v); })));

        layout.add (std::make_unique<AudioParameterInt> (ParameterID { opParamID (op, OpParam::fine), 1 }, prefix + "Fine",
                                                         0, 99, 0,
                                                         AudioParameterIntAttributes().withStringFromValueFunction (
                                                             [] (int v, int) { return "+." + String (v).paddedLeft ('0', 2); })));

        layout.add (std::make_unique<AudioParameterInt> (ParameterID { opParamID (op, OpParam::detune), 1 }, prefix + "Detune",
                                                         -7, 7, 0,
                                                         AudioParameterIntAttributes().withStringFromValueFunction (
                                                             [] (int v, int) { return v > 0 ? "+" + String (v) : String (v); })));

        layout.add (percentParam (opParamID (op, OpParam::level), prefix + "Level", op == 0 ? 90.0f : 0.0f));
        layout.add (timeParam (opParamID (op, OpParam::attack),  prefix + "Attack",  0.005f));
        layout.add (timeParam (opParamID (op, OpParam::decay),   prefix + "Decay",   1.0f));
        layout.add (percentParam (opParamID (op, OpParam::sustain), prefix + "Sustain", 100.0f));
        layout.add (timeParam (opParamID (op, OpParam::release), prefix + "Release", 0.3f));
    }

    return layout;
}
}

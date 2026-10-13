#pragma once

#include <cstdint>
#include "Parameters.h"

namespace blu
{
/** How the four operators are wired. Op indices are 0-based (0 = OP1).
    A modulator always has a higher index than the operator it modulates,
    so rendering from OP4 down to OP1 resolves every connection in one pass.
    OP4 is the operator with self-feedback in every algorithm. */
struct AlgorithmDef
{
    uint8_t modulatorsOf[kNumOps]; // bit s set => OP(s+1) modulates this operator
    uint8_t carrierMask;           // bit s set => OP(s+1) is heard
    const char* shortName;

    bool isCarrier (int op) const noexcept { return (carrierMask >> op) & 1; }

    /** Bitmask of operators that `op` modulates. */
    uint8_t targetsOf (int op) const noexcept
    {
        uint8_t mask = 0;
        for (int dst = 0; dst < kNumOps; ++dst)
            if ((modulatorsOf[dst] >> op) & 1)
                mask = static_cast<uint8_t> (mask | (1 << dst));
        return mask;
    }

    int numCarriers() const noexcept
    {
        int n = 0;
        for (int op = 0; op < kNumOps; ++op)
            n += isCarrier (op) ? 1 : 0;
        return n;
    }
};

inline const AlgorithmDef& getAlgorithm (int number1Based) noexcept
{
    static const AlgorithmDef algorithms[kNumAlgorithms] = {
        { { 0b0010, 0b0100, 0b1000, 0 }, 0b0001, "4>3>2>1  STACK" },
        { { 0b0010, 0b1100, 0, 0 },      0b0001, "(3+4)>2>1" },
        { { 0b1010, 0b0100, 0, 0 },      0b0001, "(3>2)+4 > 1" },
        { { 0b0110, 0, 0b1000, 0 },      0b0001, "(4>3)+2 > 1" },
        { { 0b0010, 0, 0b1000, 0 },      0b0101, "2>1  +  4>3" },
        { { 0b1000, 0b1000, 0b1000, 0 }, 0b0111, "4 > 1+2+3" },
        { { 0, 0, 0b1000, 0 },           0b0111, "4>3  +  1  +  2" },
        { { 0, 0, 0, 0 },                0b1111, "1+2+3+4  ADDITIVE" },
    };

    return algorithms[juce::jlimit (1, kNumAlgorithms, number1Based) - 1];
}
}

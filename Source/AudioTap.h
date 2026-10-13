#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace blu
{
/** Ring buffer the audio thread writes into and the UI reads from, for the scope and spectrum. */
class AudioTap
{
public:
    static constexpr int size = 16384; // power of two

    void push (const float* data, int numSamples) noexcept
    {
        auto w = writePos.load (std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i)
            buffer[(w + static_cast<uint32_t> (i)) & mask] = data[i];
        writePos.store (w + static_cast<uint32_t> (numSamples), std::memory_order_release);
    }

    /** Copies the most recent `numSamples` samples, oldest first. */
    void copyLatest (float* dest, int numSamples) const noexcept
    {
        const auto w = writePos.load (std::memory_order_acquire);
        const auto start = w - static_cast<uint32_t> (numSamples);
        for (int i = 0; i < numSamples; ++i)
            dest[i] = buffer[(start + static_cast<uint32_t> (i)) & mask];
    }

private:
    static constexpr uint32_t mask = size - 1;
    std::array<float, size> buffer {};
    std::atomic<uint32_t> writePos { 0 };
};
}

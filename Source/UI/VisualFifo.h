#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <vector>

/** Lock-free mono ring buffer for UI / OpenGL visualization. */
class VisualFifo
{
public:
    void prepare (int capacity)
    {
        size = juce::jmax (1024, capacity);
        buffer.assign ((size_t) size, 0.f);
        writePos.store (0);
        readPos.store (0);
    }

    void push (const float* samples, int num)
    {
        if (size <= 0 || samples == nullptr || num <= 0) return;
        int w = writePos.load (std::memory_order_relaxed);
        for (int i = 0; i < num; ++i)
        {
            buffer[(size_t) w] = samples[i];
            w = (w + 1) % size;
        }
        writePos.store (w, std::memory_order_release);
    }

    void pushStereo (const juce::AudioBuffer<float>& buf)
    {
        const int n = buf.getNumSamples();
        if (n <= 0 || size <= 0) return;
        const float* L = buf.getReadPointer (0);
        const float* R = buf.getNumChannels() > 1 ? buf.getReadPointer (1) : L;
        int w = writePos.load (std::memory_order_relaxed);
        for (int i = 0; i < n; ++i)
        {
            buffer[(size_t) w] = 0.5f * (L[i] + R[i]);
            w = (w + 1) % size;
        }
        writePos.store (w, std::memory_order_release);
    }

    /** Copy latest `count` samples into dest (oldest to newest). */
    int readLatest (float* dest, int count) const
    {
        if (size <= 0 || dest == nullptr || count <= 0) return 0;
        count = juce::jmin (count, size);
        const int w = writePos.load (std::memory_order_acquire);
        int start = w - count;
        if (start < 0) start += size;
        for (int i = 0; i < count; ++i)
            dest[i] = buffer[(size_t) ((start + i) % size)];
        return count;
    }

    int getCapacity() const { return size; }

private:
    std::vector<float> buffer;
    int size = 0;
    std::atomic<int> writePos { 0 };
    std::atomic<int> readPos { 0 };
};

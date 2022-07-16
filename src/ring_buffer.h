#pragma once

#include <stdint.h>

class RingBuffer
{
public:
    static constexpr uint8_t MaxBufferLenght = 50;

    RingBuffer();

    void append(const int32_t value);
    void clear() { m_filled = false; }
    float trend() const;

private:
    int32_t m_buffer[MaxBufferLenght];
    uint8_t m_lastPos = 0;
    bool m_filled = false;
    float K1;
    float K2;
};


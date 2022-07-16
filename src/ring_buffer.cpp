#include "ring_buffer.h"

RingBuffer::RingBuffer()
{
    float S = 0;
    K1 = 0;
    for (int i = 1; i <= MaxBufferLenght; i++)
    {
        S += i;
        K1 += i * i;
    }
    K2 = S / MaxBufferLenght;
    K1 = 5 / (K1 - K2 * S);
}

void RingBuffer::append(const int32_t value)
{
    if (m_filled)
    {
        m_buffer[m_lastPos++] = value;
        if (m_lastPos == MaxBufferLenght)
            m_lastPos = 0;
    }
    else
    {
        for (uint8_t i = 0; i < MaxBufferLenght; ++i)
            m_buffer[i] = value;
        m_filled = true;
    }
}

float RingBuffer::trend() const
{
    if (m_filled)
    {
        int32_t Ty = 0.0, Sy = 0.0;

        for (uint8_t i = 0; i < MaxBufferLenght; ++i)
        {
            int32_t value = m_buffer[(m_lastPos + i) % MaxBufferLenght];

            Sy += value;
            Ty += value * (i + 1);
        }

        return (Ty - K2 * Sy) * K1;
    }
    else
    {
        return 0;
    }
}

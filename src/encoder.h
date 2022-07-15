#pragma once

#include <stdint.h>

class Encoder
{
public:
    Encoder(const uint8_t _pin1, const uint8_t _pin2);
    void poll();
    int8_t ticks();

private:
    void getValue();
    uint8_t m_value;
    uint8_t m_lastValue;
    uint8_t m_fixState;
    uint8_t m_medState;
    uint8_t m_pin1;
    uint8_t m_pin2;
    int8_t m_counter = 0;
};

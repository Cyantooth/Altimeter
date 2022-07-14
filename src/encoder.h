#pragma once

#include <Arduino.h>

class Encoder
{
public:
    explicit Encoder(const uint8_t _pin1, const uint8_t _pin2, void(*callback)(int8_t));
    void poll();

private:
    void getValue();
    uint8_t m_value;
    uint8_t m_lastValue;
    uint8_t m_fixState;
    uint8_t m_medState;
    uint8_t m_pin1;
    uint8_t m_pin2;
    void(*m_callback)(int8_t);
};

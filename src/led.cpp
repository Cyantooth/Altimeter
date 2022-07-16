#include "led.h"

#include <Arduino.h>

LED::LED(const uint8_t pin):
    m_pin(pin)
{
    pinMode(m_pin, OUTPUT);
}

void LED::light(bool on)
{
    digitalWrite(m_pin, on ? HIGH : LOW);
}

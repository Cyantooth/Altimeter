#include "encoder.h"

#include <Arduino.h>

Encoder::Encoder(const uint8_t pin1, const uint8_t pin2):
    m_pin1(pin1),
    m_pin2(pin2)
{
    pinMode(m_pin1, INPUT_PULLUP);
    pinMode(m_pin2, INPUT_PULLUP);
    getValue();
    m_lastValue = m_value;
    m_fixState = m_value;
}

void Encoder::poll()
{
    getValue();
    if (m_value != m_lastValue)
    {
        switch (m_value)
        {
        case 0b00:
            m_fixState = m_value;
            break;

        case 0b11:
            if (m_value != m_fixState)
            {
                if (m_medState == 0b01)
                    m_counter++;
                else
                    m_counter--;
                m_fixState = m_value;
            }
            break;

        default:
            m_medState = m_value;
        }
        m_lastValue = m_value;
    }
}

int8_t Encoder::ticks()
{
    int8_t result = m_counter;
    m_counter = 0;
    return result;
}

void Encoder::getValue()
{
    m_value = (digitalRead(m_pin2) << 1) | digitalRead(m_pin1);
}

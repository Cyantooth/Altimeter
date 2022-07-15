#include "button.h"

namespace {
static constexpr uint16_t T_LongPress = 2000;
static constexpr uint16_t T_MedPress = 1000;
static constexpr uint16_t T_ShortPress = 10;
}

Button::Button(uint8_t pin):
    m_pin(pin),
    m_pressTime(0)
{
    pinMode(m_pin, INPUT_PULLUP);
}

void Button::poll()
{
    unsigned long btnTime = millis() - m_pressTime;
    if (digitalRead(m_pin))
    {
        // In released state
        if (m_event == ButtonEvent::beAcknowledge)
        {
            m_event = ButtonEvent::beNone;
            m_pressed = false;
        }
        else
        {
            if (m_pressed)
            {
                // Was pressed, just released
                m_pressed = false;
                if (m_event == ButtonEvent::beNone)
                {
                    if (btnTime >= T_ShortPress)
                        m_event = ButtonEvent::beShortPress;
                }
            }
        }
    }
    else
    {
        // In pushed state
        if (m_pressed)
        {
            // Remain pressed
            if (m_event == ButtonEvent::beMediumPress && btnTime >= T_LongPress)
                m_event = ButtonEvent::beLongPress;
            else if (m_event == ButtonEvent::beNone && btnTime >= T_MedPress)
                m_event = ButtonEvent::beMediumPress;
        }
        else
        {
            // Was released, just pressed
            m_pressed = true;
            m_event = ButtonEvent::beNone;
            m_pressTime = millis();
        }
    }
}

void Button::acknowledge()
{
    m_event = ButtonEvent::beAcknowledge;
}

void Button::clear()
{
    m_event = ButtonEvent::beNone;
}

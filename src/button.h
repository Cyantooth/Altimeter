#pragma once

#include <Arduino.h>

enum class ButtonEvent { beNone, beShortPress, beMediumPress, beLongPress, beAcknowledge };

class Button
{
public:
    explicit Button(uint8_t pin);
    void poll();
    bool pressed() const { return m_pressed; }
    ButtonEvent event() const { return m_event; }
    void acknowledge();
    void clear();

private:
    unsigned long m_pressTime = 0;
    uint8_t m_pin;
    bool m_pressed = false;
    ButtonEvent m_event = ButtonEvent::beNone;
};


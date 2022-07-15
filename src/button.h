#pragma once

#include <stdint.h>

enum class ButtonEvent { beNone = 0, beShortPress, beMediumPress, beLongPress, beAcknowledge };

class Button
{
public:
    explicit Button(uint8_t pin);
    void poll();
    inline bool pressed() const { return m_pressed; }
    inline ButtonEvent event() const { return m_event; }
    inline bool hasEvent() const { return m_event != ButtonEvent::beNone && m_event != ButtonEvent::beAcknowledge; }
    void acknowledge();
    void clear();

private:
    unsigned long m_pressTime = 0;
    uint8_t m_pin;
    bool m_pressed = false;
    ButtonEvent m_event = ButtonEvent::beNone;
};


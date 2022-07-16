#pragma once

#include <stdint.h>

class LED
{
public:
    LED(const uint8_t pin);
    void light(bool on);

private:
    uint8_t m_pin;
};

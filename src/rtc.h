#pragma once

#include <stdint.h>

struct Time
{
    int16_t year = 2020;
    int8_t mon = 1;
    int8_t day = 1;
    int8_t hour = 0;
    int8_t min = 0;
    int8_t sec = 0;
};

class RTC
{
public:
    static constexpr uint8_t DeviceAddr = 0x68;

    struct RawData
    {
        uint8_t year = 0;
        uint8_t month = 0;
        uint8_t date = 0;
        uint8_t day = 0;
        uint8_t hour = 0;
        uint8_t minutes = 0;
        uint8_t seconds = 0;
    };

    Time getTime(bool includeDate);
    void setTime(const Time& time);
    void setDate(const Time& time);
};

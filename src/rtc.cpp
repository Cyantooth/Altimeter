#include "rtc.h"

#include <Arduino.h>

#include "i2c.h"

Time RTC::getTime(bool includeDate)
{
    Time result;
    RawData rawData;
    if (includeDate)
    {
        readRegister(DeviceAddr, 0x00, &rawData, 7);
        result.day = ((rawData.date & 0b00110000) >> 4) * 10 + (rawData.date & 0b1111);
        result.mon = ((rawData.month & 0b00010000) >> 4) * 10 + (rawData.month & 0b1111);
        result.year = ((rawData.year & 0b11110000) >> 4) * 10 + (rawData.year & 0b1111) + 2000;
    }
    else
    {
        readRegister(DeviceAddr, 0x00, &rawData.hour, 3);
    }

    result.sec = ((rawData.seconds & 0b01110000) >> 4) * 10 + (rawData.seconds & 0b1111);
    result.min = ((rawData.minutes & 0b01110000) >> 4) * 10 + (rawData.minutes & 0b1111);
    result.hour = ((rawData.hour & 0b00110000) >> 4) *  10 + (rawData.hour & 0b1111);
    if (rawData.hour & (1 << 6))
    {
        // 12-hour mode
        if (result.hour >= 20)
            result.hour -= 8;
        if (result.hour == 12 || result.hour == 24)
            result.hour -= 12;
    }

    return result;
}

void RTC::setTime(const Time& time)
{
    RawData rawData;
    rawData.seconds = ((time.sec / 10) << 4) | (time.sec % 10);
    rawData.minutes = ((time.min / 10) << 4) | (time.min % 10);
    rawData.hour = ((time.hour / 10) << 4) | (time.hour % 10);
    writeData(DeviceAddr, 0x00, &rawData.hour, 3);
}

void RTC::setDate(const Time& time)
{
    RawData rawData;
    rawData.date = ((time.day / 10) << 4) | (time.day % 10);
    rawData.month = ((time.mon/ 10) << 4) | (time.mon % 10);
    uint8_t year = time.year - 2000;
    rawData.year = ((year / 10) << 4) | (year % 10);
    writeData(DeviceAddr, 0x04, &rawData.year, 3);
}

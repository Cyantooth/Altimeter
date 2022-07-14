#include "i2c.h"

#include <Wire.h>

uint8_t prepareTransmission(const uint8_t deviceAddr, const uint8_t regAddr)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(regAddr);
    return Wire.endTransmission();
}

uint8_t readRegister(
    const uint8_t deviceAddr, const uint8_t regAddr, void* data, const size_t size)
{
    if (uint8_t ret = prepareTransmission(deviceAddr, regAddr))
        return ret;

    uint8_t* ptr = (uint8_t*)data;
    Wire.requestFrom(deviceAddr, size);
    for (uint8_t i = size; i > 0; i--)
    {
        if (Wire.available())
            *(ptr + i - 1) = Wire.read();
        else
            return 100; // Data not available;
    }
    return Wire.endTransmission();
}

uint8_t writeRegister(const uint8_t deviceAddr, const uint8_t regAddr, const uint8_t data)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(regAddr);
    Wire.write(data);

    return Wire.endTransmission();
}

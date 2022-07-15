#pragma once

#include <stdint.h>

uint8_t prepareTransmission(const uint8_t deviceAddr, const uint8_t regAddr);
uint8_t readRegister(
    const uint8_t deviceAddr, const uint8_t regAddr, void* data, const uint8_t size = 1);
uint8_t writeRegister(const uint8_t deviceAddr, const uint8_t addr, const uint8_t data);
uint8_t writeData(const uint8_t deviceAddr, const uint8_t startAddr, void* data, const uint8_t size = 1);

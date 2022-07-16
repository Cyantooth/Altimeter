#pragma once

#include <stdint.h>

class RingBuffer;

class AltitudeSensor
{
public:
    enum SensorType { stUnknown, BMP085, BME280 };
//    enum State
//    {
//        NoError = 0,
//        DataTooLong,
//        NackOnAddress,
//        NackOnData,
//        OtherError,
//        Timeout,
//        IncorrectReturnCode,
//        DataNotAvailable
//    };
    
    AltitudeSensor();
    virtual ~AltitudeSensor();

    bool isTimeToGetPress() const;
    bool isTimeToGetTemp() const;
    bool isTimeToGetHum() const;

    uint8_t checkSensor();
    void checkTemperature();
    void checkPressure();
    void checkHumidity();

    bool isPressReady() const;
    bool isTempReady() const;
    bool isHumReady() const;

    void gndChanged();
    inline uint32_t getLastFilteredPress() { return m_filteredPressure; }

private:
    void calcTrend();
    int32_t pressToAlt(float press);
    int16_t pressToFL(float press);
    const uint16_t pressInterval() const;
    const uint16_t tempInterval() const;
    const uint16_t humInterval() const;

    void startReadPress();
    void finishReadPress();
    void startReadTemp();
    void finishReadTemp();
    void startReadHum();
    void finishReadHum();
    int32_t filtered(int32_t pressure);
    int32_t filtered2(int32_t pressure);

private:
    class PrivateSensor;
    PrivateSensor* d = nullptr;

    class PrivateBme280;
    class PrivateBmp085;
    
    SensorType m_sensorType = stUnknown;
    uint32_t m_lastTempTime = 0;
    uint32_t m_lastPressTime = 0;
    uint32_t m_lastHumTime = 0;

    RingBuffer* m_altBuffer;
    uint8_t m_pressBufferLenght = 0;
    int32_t m_filteredPressure = 0;
    uint8_t TrendMul = 0;
};

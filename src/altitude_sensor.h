#pragma once

#include <Arduino.h>

class AltitudeSensor
{
public:
    enum SensorType {stUnknown, BMP085, BME280};
    enum State 
    {
        NoError = 0,
        DataTooLong,
        NackOnAddress,
        NackOnData,
        OtherError,
        Timeout,
        IncorrectReturnCode,
        DataNotAvailable
    };
    
    static constexpr uint8_t MaxPressBufferLenght = 50;

    AltitudeSensor();
    bool isTimeToGetPress() const;
    bool isTimeToGetTemp() const;
    bool isTimeToGetHum() const;

    State checkSensor();
    void checkTemperature();
    void checkPressure();
    void checkHumidity();

    bool isPressReady() const;
    bool isTempReady() const;
    bool isHumReady() const;

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
    float K1 = 0.0;
    float K2 = 0.0;
    int16_t m_vSpeed = 0;
    uint32_t m_lastTempTime = 0;
    uint32_t m_lastPressTime = 0;
    uint32_t m_lastHumTime = 0;

    int32_t m_altBuffer[MaxPressBufferLenght];
    uint8_t m_pressBufferLenght = 0;
    uint8_t m_trendMul = 1;
    int32_t m_filteredPressure = 0;
};

//const double MeasureWeights[AltitudeSensor::MaxPressBufferLenght] =
//{
//    0.051240906232487,
//    0.048858048055584,
//    0.0465860000401071,
//    0.0444196091761143,
//    0.0423539620843181,
//    0.0403843738725299,
//    0.0385063775103139,
//    0.0367157136977525,
//    0.0350083212053448,
//    0.0333803276631285,
//    0.0318280407781367,
//    0.0303479399602691,
//    0.0289366683375857,
//    0.0275910251429143,
//    0.0263079584545027,
//    0.0250845582742539,
//    0.023918049927844,
//    0.0228057877717543,
//    0.0217452491929463,
//    0.0207340288875694,
//    0.0197698334057267,
//    0.0188504759499256,
//    0.0179738714154157,
//    0.0171380316611674,
//    0.0163410610007628,
//    0.0155811519029754,
//    0.0148565808922849,
//    0.0141657046400309,
//    0.0135069562373399,
//    0.0128788416413726,
//    0.0122799362868314,
//    0.0117088818550428,
//    0.0111643831932883,
//    0.0106452053773944,
//    0.0101501709109225,
//    0.00967815705460395,
//    0.009228093279965,
//    0.0087989588413659,
//    0.00838978046094748,
//    0.00799963012123482,
//    0.00762762296039156,
//    0.00727291526535126,
//    0.00693470255827434,
//    0.0066122177719905,
//    0.00630472951028874,
//    0.00601154038910907,
//    0.00573198545487395,
//    0.00546543067637209,
//    0.00521127150677426,
//    0.00496893151251972
//}; // Summ = 1

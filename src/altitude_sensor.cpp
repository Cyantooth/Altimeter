#include "altitude_sensor.h"

#include <Arduino.h>

#include "application.h"
#include "i2c.h"
#include "ring_buffer.h"

class AltitudeSensor::PrivateSensor
{
public:
    PrivateSensor() = default;
    virtual ~PrivateSensor() = default;

    uint8_t checkSensor();
    virtual uint8_t readCalibrateParams() = 0;
    virtual const uint16_t pressInterval() const = 0;
    virtual const uint16_t tempInterval() const = 0;
    virtual uint8_t upDelay() const = 0;
    virtual uint8_t utDelay() const = 0;
    virtual uint8_t calcPress() = 0;
    virtual uint8_t calcTemp() = 0;
    inline int16_t getT() const { return T; }
    inline int32_t getP() const { return P; }

protected:
    virtual const uint8_t deviceAddr() const = 0;
    virtual const uint8_t chipId() const = 0;

protected:
    int32_t T;
    int32_t P;
};

uint8_t AltitudeSensor::PrivateSensor::checkSensor()
{
    uint8_t check;
    if (uint8_t ret = readRegister(deviceAddr(), 0xD0, &check))
        return ret;
    if (check != chipId())
        return 0xFF;

    return 0;
}

// ====================================================================

class AltitudeSensor::PrivateBmp085 : public PrivateSensor
{
    friend class AltitudeSensor;
    enum Oversampling
    {
        LowPower = 0,
        Standard,
        HighRes,
        UltraHighRes
    };

    struct CalibratedParams 
    {
        // Reverse order due to big-endian transfer
        int16_t md = 0;
        int16_t mc = 0;
        int16_t mb = 0;
        int16_t b2 = 0;
        int16_t b1 = 0;
        uint16_t ac6 = 0;
        uint16_t ac5 = 0;
        uint16_t ac4 = 0;
        int16_t ac3 = 0;
        int16_t ac2 = 0;
        int16_t ac1 = 0;
    } __attribute__((packed));

public:
    uint8_t readCalibrateParams() override;
    inline const uint16_t pressInterval() const override { return 100; }
    inline const uint16_t tempInterval() const override { return 2000; }
    uint8_t upDelay() const override;
    inline uint8_t utDelay() const override { return 5; }
    uint8_t calcTemp() override;
    uint8_t calcPress() override;

protected:
    inline const uint8_t deviceAddr() const override { return 0x77; }
    inline const uint8_t chipId() const override { return 0x55; }

private:
    void startReadUP();
    void startReadUT();

private:
    CalibratedParams m_calibratedParams;
    int32_t b5 = 0;
    Oversampling m_rate = UltraHighRes;
};

uint8_t AltitudeSensor::PrivateBmp085::upDelay() const
{
    static const uint8_t delays_ms[4] = { 4, 7, 13, 25 };
    return delays_ms[m_rate];
}

uint8_t AltitudeSensor::PrivateBmp085::calcTemp()
{
    int32_t UT;
    {
        uint16_t tmp;
        if (uint8_t ret = readRegister(deviceAddr(), 0xF6, &tmp, 2))
        {
            UT = 0;
            T = 0;
            return ret;
        }
        UT = (int32_t)tmp;
    }
    int32_t X1 = (UT - m_calibratedParams.ac6) * m_calibratedParams.ac5 / 32768;
    int32_t X2 = (int32_t)m_calibratedParams.mc * 2048 / (X1 + m_calibratedParams.md);
    b5 = X1 + X2;
    T = (b5 + 8) / 16;
    return 0;
}

uint8_t AltitudeSensor::PrivateBmp085::calcPress()
{
    int32_t UP = 0;
    if (m_rate == 0)
    {
        if (uint8_t ret = readRegister(deviceAddr(), 0xF6, &UP, 2))
            return ret;
    }
    else
    {
        if (uint8_t ret = readRegister(deviceAddr(), 0xF6, &UP, 3))
            return ret;
        UP >>= (8 - m_rate);
    }
    int32_t b6 = b5 - 4000;
    int32_t X1 = ((int32_t)m_calibratedParams.b2 * (b6 * b6 / 4096)) / 2048;
    int32_t X2 = (int32_t)m_calibratedParams.ac2 * b6 / 2048;
    int32_t X3 = X1 + X2;
    int32_t b3 = ((((int32_t)m_calibratedParams.ac1 * 4 + X3) << m_rate) + 2) / 4;
    X1 = (int32_t)m_calibratedParams.ac3 * b6 / 8192;
    X2 = (m_calibratedParams.b1 * (b6 * b6 / 4096)) / 65536;
    X3 = ((X1 + X2) + 2) / 4;
    uint32_t b4 = (uint32_t)m_calibratedParams.ac4 * (uint32_t)(X3 + 32768) / 32768;
    int32_t b7 = ((uint32_t)UP - b3) * (50000 >> m_rate);
    if (b7 < 0)
        P = (b7 * 2) / b4;
    else
        P = (b7 / b4 * 2);
    X1 = (P / 256) * (P / 256);
    X1 = (X1 * 3038) / 65536;
    X2 = (-7357 * P) / 65536;
    P = P + (X1 + X2 + 3791) / 16;
    return 0;
}

void AltitudeSensor::PrivateBmp085::startReadUP()
{
    if (aApplication->tempState() == Querying)
        return;

    if (writeRegister(deviceAddr(), 0xF4, 0x34 | (m_rate << 6)))
    {
        aApplication->setFlag(ApplicationFlags::AltSensorError);
        aApplication->setPressState(NoAction);
    }
    else
    {
        aApplication->setPressState(Querying);
    }
}

void AltitudeSensor::PrivateBmp085::startReadUT()
{
    if (aApplication->pressState() == Querying)
        return;

    if (writeRegister(deviceAddr(), 0xF4, 0x2E))
    {
        aApplication->setFlag(ApplicationFlags::AltSensorError);
        aApplication->setTempState(NoAction);
    }
    else
    {
        aApplication->setTempState(Querying);
    }
}

uint8_t AltitudeSensor::PrivateBmp085::readCalibrateParams()
{
    return readRegister(deviceAddr(), 0xAA, &m_calibratedParams, sizeof(m_calibratedParams));
}

// ====================================================================

class AltitudeSensor::PrivateBme280 : public PrivateSensor
{
public:
    uint8_t readCalibrateParams() override;
    // TODO: Fix intervals according to sensor manual
    inline const uint16_t pressInterval() const override { return 100; }
    inline const uint16_t tempInterval() const override { return 2000; }
    inline const uint16_t humInterval() const { return 65535; }
    inline uint8_t upDelay() const override { return 255; }
    inline uint8_t utDelay() const override { return 255; }
    inline uint8_t uhDelay() const { return 255; }
    uint8_t calcPress() override;
    uint8_t calcTemp() override;

protected:
    const uint8_t deviceAddr() const override { return 0x76; }
    inline const uint8_t chipId() const override { return 0x60; }
};

uint8_t AltitudeSensor::PrivateBme280::readCalibrateParams()
{
    return 0;
}

uint8_t AltitudeSensor::PrivateBme280::calcPress()
{
    // TODO
    return 0;
}

uint8_t AltitudeSensor::PrivateBme280::calcTemp()
{
    // TODO
    return 0;
}

// ====================================================================

AltitudeSensor::AltitudeSensor()
{
    m_altBuffer = new RingBuffer;
    TrendMul = RingBuffer::MaxBufferLenght / 10;
    aApplication->clearFlag(ApplicationFlags::AltSensorError);
}

AltitudeSensor::~AltitudeSensor()
{
    delete m_altBuffer;
}

bool AltitudeSensor::isTimeToGetPress() const
{
    return millis() - m_lastPressTime >= pressInterval();
}

bool AltitudeSensor::isTimeToGetTemp() const
{
    return millis() - m_lastTempTime >= tempInterval();
}

bool AltitudeSensor::isTimeToGetHum() const
{
    return m_sensorType == BME280 &&
        millis() - m_lastHumTime >= ((PrivateBme280*)d)->humInterval();
}

uint8_t AltitudeSensor::checkSensor()
{
    if (d == nullptr)
    {
        uint8_t ret;
        m_sensorType = stUnknown;
        d = new PrivateBme280();
        if ((ret = d->checkSensor()))
        {
            delete d;

            d = new PrivateBmp085();
            if ((ret = d->checkSensor()))
                delete d;
            else
                m_sensorType = BMP085;
        }
        else
        {
            m_sensorType = BME280;
        }
        
        if (m_sensorType == stUnknown)
            return ret;
    }
    return d->readCalibrateParams();
}

void AltitudeSensor::checkTemperature()
{
    if (aApplication->tempState() == TimeToQuery)
        startReadTemp();
    else if (aApplication->tempState() == Ready)
        finishReadTemp();
}

void AltitudeSensor::checkPressure()
{
    if (aApplication->pressState() == TimeToQuery)
        startReadPress();
    else if (aApplication->pressState() == Ready)
        finishReadPress();
}

void AltitudeSensor::checkHumidity()
{
    if (aApplication->humState() == TimeToQuery)
        startReadHum();
    else if (aApplication->humState() == Ready)
        finishReadHum();
}

bool AltitudeSensor::isPressReady() const
{
    return m_sensorType != stUnknown &&
        millis() - m_lastPressTime >= d->upDelay();
}

bool AltitudeSensor::isTempReady() const
{
    return m_sensorType != stUnknown &&
        millis() - m_lastTempTime >= d->utDelay();
}

bool AltitudeSensor::isHumReady() const
{
    return m_sensorType == BME280 &&
        millis() - m_lastHumTime >= ((PrivateBme280*)d)->uhDelay();
}

void AltitudeSensor::gndChanged()
{
    m_altBuffer->clear();
}

void AltitudeSensor::calcTrend()
{
    float trend = m_altBuffer->trend() / TrendMul;
    // TODO: Calculate trent in feets per minutes
    aApplication->setVSpeed(trend);
}

const uint16_t AltitudeSensor::pressInterval() const
{
    if (m_sensorType != stUnknown)
        return d->pressInterval();

    return 0;
}

const uint16_t AltitudeSensor::tempInterval() const
{
    if (m_sensorType != stUnknown)
        return d->tempInterval();

    return 0;
}

void AltitudeSensor::startReadPress()
{
    if (m_sensorType == BMP085)
        ((PrivateBmp085*)d)->startReadUP();
    if (aApplication->pressState() == Querying)
        m_lastPressTime = millis();
}

void AltitudeSensor::finishReadPress()
{
    aApplication->setPressState(NoAction);

    if (uint8_t res = d->calcPress())
    {
        aApplication->setFlag(ApplicationFlags::AltSensorError);
        return;
    }
    int32_t pressure = d->getP();
    m_filteredPressure = filtered2(pressure);
    int32_t altitude = pressToAlt(m_filteredPressure);
    m_altBuffer->append(altitude);
    calcTrend();
    int16_t flightLevel = pressToFL(m_filteredPressure);
    aApplication->setAltitude(altitude);
    aApplication->setFlightLevel(flightLevel);
}

void AltitudeSensor::startReadTemp()
{
    if (m_sensorType == BMP085)
        ((PrivateBmp085*)d)->startReadUT();
    if (aApplication->tempState() == Querying)
        m_lastTempTime = millis();
}

void AltitudeSensor::finishReadTemp()
{
    aApplication->setTempState(NoAction);
    if (uint8_t res = d->calcTemp())
    {
        aApplication->setFlag(ApplicationFlags::AltSensorError);
        return;
    }

    aApplication->setTemperature(d->getT());
}

void AltitudeSensor::startReadHum()
{
    // TODO
    Serial.println("startReadHum()");
    m_lastHumTime = millis();
}

void AltitudeSensor::finishReadHum()
{
    // TODO
    Serial.println("finishReadHum()");
}

int32_t AltitudeSensor::filtered(int32_t pressure)
{
    static const float varVolt = 2.73;
    static const float varProcess = 0.05;
    static float Pc = 0.0;
    static float G = 0.0;
    static float P = 1.0;
    static float Xp = 0.0;
    static float Zp = 0.0;
    static float Xe = 0.0;

    Pc = P + varProcess;
    G = Pc / (Pc + varVolt);
    P = (1 - G) * Pc;
    Xp = Xe;
    Zp = Xp;
    Xe = G * (pressure - Zp) + Xp;
    return Xe;
}

int32_t AltitudeSensor::filtered2(int32_t pressure)
{
    static const float _err_measure = 2.73;
    static const float _q = 0.05;

    float _kalman_gain, _current_estimate;
    static float _err_estimate = _err_measure;
    static float _last_estimate = pressure;

    _kalman_gain = (float)_err_estimate / (_err_estimate + _err_measure);
    _current_estimate = _last_estimate + (float)_kalman_gain * (pressure - _last_estimate);
    _err_estimate = (1.0 - _kalman_gain) * _err_estimate + fabs(_last_estimate - _current_estimate) * _q;
    _last_estimate = _current_estimate;
    return _current_estimate;
}

int32_t AltitudeSensor::pressToAlt(float press)
{
    if (aApplication->gndPress() == 0)
        return 0;

    float res = 1.0 - pow(press / aApplication->gndPress(), 0.190295);
    if (aApplication->altUnit() == AltUnits::Feets)
        res *= 145439.633;
    else
        res *= 44330.0;
    return res * 100;
}

int16_t AltitudeSensor::pressToFL(float press)
{
    return (1.0 - pow(press / 101325, 0.190295)) * 1454.39633 + 0.5;
}

#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#include "display.h"
#include "altitude_sensor.h"

class Encoder;
class Button;

struct Time
{
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;      
};

enum ApplicationFlags
{
    AltSensorError       = 1 << 0,
    TimerError           = 1 << 1,
    TimeToPrintTime      = 1 << 2,
    TimeEditMode         = 1 << 3,
    TimerMode            = 1 << 4,
    PressUnitChangeReady = 1 << 5,
    PressUnitChanged     = 1 << 6,
    AltUnitChanged       = 1 << 7,
    AltSetChanged        = 1 << 8,
    AltitudeChanged      = 1 << 9,
    GndPressChanged      = 1 << 10,
    FlightLevelChanged   = 1 << 11,
    TempChanged          = 1 << 12,
    DateShowed           = 1 << 13,
//    Recording            = 1 << 14,
//    RecPause             = 1 << 15,
};

enum class AltUnits { Meters, Feets };
enum class PressUnits { HPA, mmHg };
enum SensorState { NoAction = 0, TimeToQuery, Querying, Ready };

class Application
{
public:
    struct EEPROMData 
    {
        uint32_t gndPress;
        int16_t altSet = 0;
        AltUnits altUnit = AltUnits::Meters;
        PressUnits pressUnit = PressUnits::HPA;
        bool timerRun = false;
        Time timer;
    } __attribute__((packed));

    Application();
    static Application* instance() { return s_instance; }

    void run();
    void interrupt();
    void int1();
    void int2();

    inline SensorState tempState() const { return m_tempState; }
    inline SensorState pressState() const { return m_pressState; }
    inline SensorState humState() const { return m_humState; }

    bool testFlag(uint16_t flag) const;
    void setFlag(uint16_t flag);
    void clearFlag(uint16_t flag);

    void setTempState(SensorState newTempState);
    void setPressState(SensorState newPressState);
    void setHumState(SensorState newHumState);

    inline uint32_t gndPress() const { return m_eepromData.gndPress; }
    void setGndPress(const uint32_t gndPress);

    inline PressUnits pressUnit() const { return m_eepromData.pressUnit; }
    void setPressUnit(const PressUnits pressUnit);

    int16_t altSet() const { return m_eepromData.altSet; }
    void setAltSet(const int16_t altSet);

    inline AltUnits altUnit() const { return m_eepromData.altUnit; }

    inline int16_t flightLevel() const { return m_flightLevel; }
    void setFlightLevel(int16_t newFlightLevel);

//    inline int16_t temperature() const { return m_temperature; }
    void setTemperature(int16_t newTemperature);
    void setAltitude(int32_t newAltitude);

private:
    void setupPins();
    void initSerial();
    void readEEPROMData();
    void writeEEPROMData(void *ptr, uint8_t length);
    void setupInterrupts();

    void reactEvents();
    void reactTemperature();
    void reactPressure();
    void reactHumidity();
    void reactVSpeed();
    void reactAltSet();
    void reactGndPress();
    void reactTimer();
    void reactAltUnitChanged();
    void reactPressUnitChanged();
    void reactButton2();

private:
    static Application* s_instance;
    Display* m_display;
    AltitudeSensor* m_altSensor;
    
    volatile SensorState m_tempState = NoAction;
    volatile SensorState m_pressState = NoAction;
    volatile SensorState m_humState = NoAction;

    volatile uint16_t m_flags = 0;
    volatile EEPROMData m_eepromData;
    int16_t m_flightLevel = 0;
    int16_t m_temperature = 0;
    int32_t m_altitude = 0;

    Encoder* m_encoder1;
    Encoder* m_encoder2;
    Button* m_button1;
    Button* m_button2;
    Button* m_buttonA;
    Button* m_buttonB;
};

template<typename T>
T limit(const T min, const T value, const T max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

#define aApplication Application::instance()

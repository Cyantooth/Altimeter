#pragma once

#include <stdint.h>

#include "rtc.h"

class Display;
class AltitudeSensor;
class RTC;
class Encoder;
class Button;
class LED;

enum ApplicationFlags
{
    AltSensorError       = 1 << 0,
    TimerError           = 1 << 1, //< Currently not used
    TimeToPrintTime      = 1 << 2,
    TimeEditMode         = 1 << 3,
    VertSpeedChanged     = 1 << 4,
    PressUnitChangeReady = 1 << 5,
    PressUnitChanged     = 1 << 6,
    AltUnitChanged       = 1 << 7,
    AltSetChanged        = 1 << 8,
    AltitudeChanged      = 1 << 9,
    GndPressChanged      = 1 << 10,
    FlightLevelChanged   = 1 << 11,
    TempChanged          = 1 << 12,
    DateShowed           = 1 << 13,
    Recording            = 1 << 14,
    RecPause             = 1 << 15,
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
    ~Application();

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
    void changeFlag(uint16_t flag);

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

    void setTemperature(int16_t newTemperature);

    inline int32_t altitude() const { return m_altitude; }
    void setAltitude(int32_t newAltitude);

    void setVSpeed(int16_t newVSpeed);

private:
    void initSerial();
    void readEEPROMData();
    void writeEEPROMData(void *ptr, uint8_t length);
    void setupInterrupts();

    void processEvents();
    void reactTemperature();
    void reactPressure();
    void reactHumidity();
    void reactVSpeed();
    void reactAltSet();
    void reactGndPress();
    void reactRTC();
    void reactTimer();
    void reactTimeSet();
    void reactAltUnitChanged();
    void reactPressUnitChanged();
    void reactButton2();
    void reactEncoders();

    int8_t daysInMonth(uint8_t month, uint8_t year);

private:
    static Application* s_instance;
    Display* m_display = nullptr;
    AltitudeSensor* m_altSensor = nullptr;
    RTC* m_rtc = nullptr;

    Encoder* m_encoder1 = nullptr;
    Encoder* m_encoder2 = nullptr;
    Button* m_button1 = nullptr;
    Button* m_button2 = nullptr;
    Button* m_buttonA = nullptr;
    Button* m_buttonB = nullptr;
    LED* m_ledAlt = nullptr;
    LED* m_ledRec = nullptr;
    
    volatile SensorState m_tempState = NoAction;
    volatile SensorState m_pressState = NoAction;
    volatile SensorState m_humState = NoAction;

    volatile uint16_t m_flags = 0;
    volatile EEPROMData m_eepromData;
    int16_t m_flightLevel = 0;
    int16_t m_temperature = 0;
    int32_t m_altitude = 0;
    int16_t m_vSpeed = 0;
    Time m_currentTime;
    Time m_editTime;
    Time m_recTimer;
    uint8_t m_timeEditPart;
    uint8_t m_lastTimerSec = 0;
    uint32_t m_timeEditStart;
    unsigned long m_timeShowMillis;
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

#include "application.h"

#include <EEPROM.h>
#include <avr/wdt.h>
#include <Wire.h>

#include "display.h"
#include "altitude_sensor.h"
#include "encoder.h"
#include "button.h"
#include "settings.h"

Application* Application::s_instance = nullptr;

Application::Application()
{
    s_instance = this;
    wdt_disable();
    setupPins();
    initSerial();
    m_display = new Display();
    m_display->clrScr();
    m_rtc = new RTC();
    m_altSensor = new AltitudeSensor();
    if (uint8_t check = m_altSensor->checkSensor())
    {
        Serial.print("Sensor error: ");
        Serial.println(check);
        // TODO: Error messages
    }
    else
    {
        readEEPROMData();
        m_encoder1 = new Encoder(2, 3);
        m_encoder2 = new Encoder(14, 15);
        m_button1 = new Button(4);
        m_button2 = new Button(13);
        m_buttonA = new Button(12);
        m_buttonB = new Button(16);

        m_display->drawFixedElements();
        m_timeShowMillis = millis();
        setupInterrupts();
        setFlag(AltSetChanged | GndPressChanged | AltUnitChanged | PressUnitChanged);
        setTempState(TimeToQuery); //< We need to measure temperature before pressure
    }
}

void Application::initSerial()
{
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
}

void Application::run()
{
    if (testFlag(AltSensorError))
    {
//        Serial.println("Sensor error!");
        while(1);
        // TODO: Improve
    }
    else
    {
        wdt_reset();

        m_altSensor->checkTemperature();
        m_altSensor->checkPressure();
        m_altSensor->checkHumidity();

        processEvents();
    }
}

void Application::interrupt()
{
    if (m_pressState == Querying && m_altSensor->isPressReady())
        m_pressState = Ready;
    else if (m_altSensor->isTimeToGetPress())
        m_pressState = TimeToQuery;

    if (m_tempState == Querying && m_altSensor->isTempReady())
        m_tempState = Ready;
    else if (m_altSensor->isTimeToGetTemp())
        m_tempState = TimeToQuery;

    if (m_humState == Querying && m_altSensor->isHumReady())
        m_humState = Ready;
    else if (m_altSensor->isTimeToGetHum())
        m_humState = TimeToQuery;

    m_button1->poll();
    m_button2->poll();
    m_buttonA->poll();
    m_buttonB->poll();

    if (!testFlag(TimerError) && millis() - m_timeShowMillis >= T_Time)
    {
        m_timeShowMillis += T_Time;
        setFlag(TimeToPrintTime);
    }
}

void Application::int1()
{
    m_encoder1->poll();
}

void Application::int2()
{
    m_encoder2->poll();
}

void Application::readEEPROMData()
{
    uint8_t* ptr = (uint8_t*)&m_eepromData;
    for (uint8_t i = 0; i < sizeof(EEPROMData); ++i)
    {
        *(ptr + i) = EEPROM.read(i);
    }

    if (m_eepromData.gndPress < 90000 || m_eepromData.gndPress > 110000)
        m_eepromData.gndPress = 101325;
    if (m_eepromData.altSet <= 0 || m_eepromData.altSet > 9990)
        m_eepromData.altSet = 300;
    if (m_eepromData.altUnit != AltUnits::Meters && m_eepromData.altUnit != AltUnits::Feets)
        m_eepromData.altUnit = AltUnits::Meters;
    if (m_eepromData.pressUnit != PressUnits::HPA && m_eepromData.pressUnit != PressUnits::mmHg)
        m_eepromData.pressUnit = PressUnits::HPA;
}

void Application::writeEEPROMData(void* ptr, uint8_t length)
{
    uint8_t offset = (uint8_t*)ptr - (uint8_t*)&m_eepromData;
    for (uint8_t i = 0; i < length; ++i)
    {
        EEPROM.write(offset + i, *((uint8_t*)ptr + i));
    }
}

void Application::setupInterrupts()
{
    cli();
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 8khz increments
    OCR2A = 255;// (must be <256)

    TCCR2A |= (1 << WGM21);  // turn on CTC mode
    TCCR2B |= (1 << CS22);   // Set CS21 bit for 64 prescaler | (1 << CS20)
    TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt

    PCICR = 1 << PCIE1;
    PCMSK1 = (1 << PCINT10) | (1 << PCINT9);

    EICRB = 1 << ISC50 | 1 << ISC40;
    EIMSK = 1 << INT5 | 1 << INT4;
    sei();  //allow interrupts
    wdt_enable(WDTO_2S);
}

void Application::processEvents()
{
    reactTemperature();
    reactPressure();
    reactHumidity();
    reactVSpeed();
    reactAltSet();
    reactGndPress();
    reactRTC();
    reactAltUnitChanged();
    reactPressUnitChanged();

    reactEncoders();

    reactTimeSet();
    reactTimer();
    reactButton2();
}

void Application::reactPressure()
{
    if (testFlag(FlightLevelChanged))
    {
        m_display->printFlightLevel(m_flightLevel);
        clearFlag(FlightLevelChanged);
    }

    if (testFlag(AltitudeChanged))
    {
        m_display->printAltitude(m_altitude);
        clearFlag(AltitudeChanged);
    }
}

void Application::reactHumidity()
{
    // TODO
}

void Application::reactVSpeed()
{
    // TODO
}

void Application::reactTemperature()
{
    if (testFlag(TempChanged))
    {
        clearFlag(TempChanged);
        m_display->printTemp(m_temperature);
    }
}

void Application::reactAltSet()
{
    if (testFlag(AltSetChanged))
    {
        clearFlag(AltSetChanged);
        m_display->printAltSet(altSet());
        m_display->drawLeveler(altSet());
        writeEEPROMData(&m_eepromData.altSet, sizeof(EEPROMData::altSet));
    }
}

void Application::reactGndPress()
{
    if (testFlag(GndPressChanged))
    {
        clearFlag(GndPressChanged);
        m_display->printGndPress(gndPress());
        writeEEPROMData(&m_eepromData.gndPress, sizeof(EEPROMData::gndPress));
    }
}

void Application::reactRTC()
{
    if (testFlag(TimeToPrintTime))
    {
        m_currentTime = m_rtc->getTime(false);
        if (testFlag(TimeEditMode))
        {
            m_display->printTime(
                m_timeEditPart <= 2 ? m_editTime : m_currentTime, m_timeEditPart);
            m_display->printDate(m_editTime, m_timeEditPart);
        }
        else
        {
            m_display->printTime(m_currentTime);
            if (testFlag(DateShowed))
            {
                m_display->hideDate();
            }
            else
            {
                if (testFlag(Recording))
                {
                    m_display->printTimer(m_recTimer);
                    if (!testFlag(RecPause))
                    {
                        uint8_t dt = m_currentTime.sec >= m_lastTimerSec ?
                            m_currentTime.sec - m_lastTimerSec :
                            m_currentTime.sec + 60 - m_lastTimerSec;
                        m_recTimer.sec += dt;
                        while (m_recTimer.sec >= 60)
                        {
                            m_recTimer.sec -= 60;
                            m_recTimer.min++;
                        }
                        while (m_recTimer.min >= 60)
                        {
                            m_recTimer.min -= 60;
                            m_recTimer.hour++;
                        }
                        m_lastTimerSec = m_currentTime.sec;
                    }
                }
                else if (testFlag(RecPause))
                {
                    m_display->hideTimer();
                    clearFlag(RecPause);
                }
            }
        }
        clearFlag(TimeToPrintTime);

//        uint16_t AltDif = abs(Altitude / 100 - AltSet);
//        bool OddSec = (m_currentTime.sec % 2);
//        // 30 метров ≈ 100 футов = 1 эшелон
//        if (AltDif <= 15 || (AltDif <= 30 && !OddSec))
//            digitalWrite(LED_FL, HIGH);
//        else
//            digitalWrite(LED_FL, LOW);

//        if (Flags.Recording && (!Flags.RecPause || OddSec))
//            digitalWrite(LED_REC, HIGH);
//        else
//            digitalWrite(LED_REC, LOW);
    }
}

void Application::reactTimer()
{
    if (!testFlag(TimeEditMode))
    {
        switch (m_buttonB->event())
        {
        case ButtonEvent::beShortPress:
            if (testFlag(Recording))
            {
                // Recording is in progress. Toggle pause
                changeFlag(RecPause);
                m_lastTimerSec = m_currentTime.sec;
            }
            else
            {
                // Start recording
                m_recTimer.hour = m_recTimer.min = m_recTimer.sec = 0;
                setFlag(Recording);
                clearFlag(RecPause);
                m_lastTimerSec = m_currentTime.sec;
                m_display->drawTimerBackground(true);
            }
            m_buttonB->acknowledge();
            break;

        case ButtonEvent::beLongPress:
            if (testFlag(Recording))
            {
                clearFlag(Recording);
                setFlag(RecPause);
            }
            m_buttonB->acknowledge();
            break;

        default:
            if (!m_buttonB->pressed() && m_buttonB->hasEvent())
                m_buttonB->acknowledge();
        }

//        // Запись изменения состояния в EEPROM
//        uint8_t TimerState = (Flags.Recording?1:0) | (Flags.RecPause?2:0);
//        if (LastTimerState != TimerState)
//        {
//            uint8_t ROMcnt = 7;
//            EEPROM.write(ROMcnt++, TimerState);
//            if (Flags.RecPause)
//            {
//                EEPROM.write(ROMcnt++, RecTimer.hour);
//                EEPROM.write(ROMcnt++, RecTimer.min);
//                EEPROM.write(ROMcnt++, RecTimer.sec);
//            }
//            else
//            {
//                EEPROM.write(ROMcnt++, CurrentTime.hour);
//                EEPROM.write(ROMcnt++, CurrentTime.min);
//                EEPROM.write(ROMcnt++, CurrentTime.sec);
//            }
//            LastTimerState = TimerState;
//        }
    }
    else
    {
        m_buttonB->acknowledge();
    }
}

void Application::reactTimeSet()
{
    if (testFlag(TimeEditMode))
    {
        auto exitTimeEditMode = [this]
        {
            clearFlag(TimeEditMode);
            m_display->hideDate();
            m_display->redrawAllTimeSegments();
        };

        // Time edit mode
        if (millis() - m_timeEditStart >= T_TimeEditTimeout)
        {
            exitTimeEditMode();
            return;
        }

        if (m_buttonA->event() == ButtonEvent::beShortPress)
        {
            m_buttonA->acknowledge();
            // Подготовим этот и следующий сегменты к перерисовке
            if (m_timeEditPart <= 2)
            {
                for (uint8_t j = 0; j < 10; j++)
                {
                    if ((TimeFixedSegments[j] == m_timeEditPart) || TimeFixedSegments[j] == m_timeEditPart + 1)
                        m_display->redrawTimeSegment(j);
                }
            }
            if (m_timeEditPart >= 2)
            {
                for (uint8_t j = 0; j < 10; j++)
                {
                    if ((DateFixedSegments[j] == m_timeEditPart) || DateFixedSegments[j] == m_timeEditPart + 1)
                        m_display->redrawTimeSegment(j);
                }
            }

            m_timeEditPart++;

            if (m_timeEditPart == 3)
            {
                m_rtc->setTime(m_editTime);
            }
            else if (m_timeEditPart == 6)
            {
                m_rtc->setDate(m_editTime);
                exitTimeEditMode();
            }
            m_timeEditStart = millis();
        }
        else if (m_buttonA->event() == ButtonEvent::beLongPress)
        {
            m_buttonA->acknowledge();
            exitTimeEditMode();
        }
        else if (m_button1->event() == ButtonEvent::beMediumPress)
        {
            m_button1->acknowledge();
            if (m_timeEditPart <= 2)
                m_editTime.sec = 0;
        }

        if (!m_buttonA->pressed())
        {
            if (int8_t dv = m_encoder1->ticks())
            {
                switch (m_timeEditPart)
                {
                case 0:
                    m_editTime.hour = (m_editTime.hour + dv) % 24;
                    // Remainders must always be positive, but who cares
                    while (m_editTime.hour < 0)
                        m_editTime.hour += 24;
                    break;

                case 1:
                    m_editTime.min = (m_editTime.min + dv) % 60;
                    while (m_editTime.min < 0)
                        m_editTime.min += 60;
                    break;

                case 2:
                    m_editTime.sec = (m_editTime.sec + dv) % 60;
                    while (m_editTime.sec < 0)
                        m_editTime.sec += 60;
                    break;

                case 3:
                    m_editTime.year = (m_editTime.year - 2000 + dv) % 1000 + 2000;
                    while (m_editTime.year < 2000)
                        m_editTime.year += 1000;
                    break;

                case 4:
                    m_editTime.mon = (m_editTime.mon - 1 + dv) % 12 + 1;
                    while (m_editTime.mon < 1)
                        m_editTime.mon += 12;
                    break;

                case 5:
                    uint8_t maxDay = daysInMonth(m_editTime.mon, m_editTime.year);
                    m_editTime.day = (m_editTime.day - 1 + dv) % maxDay + 1;
                    while (m_editTime.day < 1)
                        m_editTime.day += maxDay;
                    break;
                }
                m_timeEditStart = millis();
            }
        }
    }
    else if (m_buttonA->event() == ButtonEvent::beLongPress)
    {
        m_buttonA->acknowledge();
        if (!testFlag(Recording))
        {
            // Entering into time edit mode
            setFlag(TimeEditMode);
            m_timeEditPart = 0;
            setFlag(TimeToPrintTime);
            m_editTime = m_rtc->getTime(true);
            m_timeEditStart = millis();
            m_display->redrawTimeSegment(1);
            m_display->redrawTimeSegment(2);
            m_display->drawTimerBackground(false);
            m_display->redrawAllDateSegments();
        }
    }
    else
    {
        if (!m_buttonA->pressed() && m_buttonA->hasEvent())
            m_buttonA->acknowledge();
    }
}

void Application::reactAltUnitChanged()
{
    if (testFlag(AltUnitChanged))
    {
        clearFlag(AltUnitChanged);
        m_display->drawAltUnits(m_eepromData.altUnit);
    }
}

void Application::reactPressUnitChanged()
{
    if (testFlag(PressUnitChanged))
    {
        clearFlag(PressUnitChanged);
        m_display->drawPressUnit(m_eepromData.pressUnit);
    }
}

void Application::reactButton2()
{
    if (m_button2->pressed())
    {
        if (m_button2->event() == ButtonEvent::beMediumPress)
        {
            setFlag(PressUnitChangeReady);
            m_display->drawPressUnit(pressUnit());
        }
        else if (m_button2->event() == ButtonEvent::beLongPress)
        {
            clearFlag(PressUnitChangeReady);
            m_display->drawPressUnit(pressUnit());
            setGndPress(m_altSensor->getLastFilteredPress());
            m_button2->acknowledge();
        }
    }
    else if (m_button2->hasEvent())
    {
        if (m_button2->event() == ButtonEvent::beMediumPress)
        {
            setPressUnit(pressUnit() == PressUnits::HPA ? PressUnits::mmHg : PressUnits::HPA);
            writeEEPROMData(&m_eepromData.pressUnit, sizeof(EEPROMData::pressUnit));
            m_display->printGndPress(gndPress());
        }

        if (testFlag(PressUnitChangeReady))
        {
            clearFlag(PressUnitChangeReady);
            m_display->drawPressUnit(pressUnit());
        }
        m_button2->clear();
    }
}

void Application::reactEncoders()
{
    if (!testFlag(TimeEditMode))
        setAltSet(altSet() + m_encoder1->ticks() * (m_button1->pressed() ? 100 : 10));

    if (int8_t ticks = m_encoder2->ticks())
    {
        uint32_t value = gndPress();
        if (pressUnit() == PressUnits::mmHg)
            value = (((float)value / 133.322) + ticks) * 133.322;
        else
            value += ticks * 100;

        setGndPress(value);
    }
}

int8_t Application::daysInMonth(uint8_t month, uint8_t year)
{
    static const int8_t days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month > 12)
        return -1;

    int8_t result = days[month - 1];
    if (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
        result++;

    return result;
}

void Application::setAltitude(int32_t newAltitude)
{
    if (m_altitude == newAltitude)
        return;

    m_altitude = limit(-99900, newAltitude, 999900);
    setFlag(AltitudeChanged);
}

void Application::setupPins()
{
//    pinmode(17, OUTPUT);
//    pinmode(18, OUTPUT);
}

void Application::setHumState(SensorState newHumState)
{
    m_humState = newHumState;
}

void Application::setGndPress(const uint32_t gndPress)
{
    if (m_eepromData.gndPress == gndPress)
        return;

    m_eepromData.gndPress = limit((uint32_t)90000, gndPress, (uint32_t)110000);
    setFlag(GndPressChanged);
}

void Application::setPressUnit(const PressUnits pressUnit)
{
    if (m_eepromData.pressUnit == pressUnit)
        return;

    m_eepromData.pressUnit = pressUnit;
    setFlag(PressUnitChanged);
}

void Application::setAltSet(const int16_t altSet)
{
    if (m_eepromData.altSet == altSet)
        return;

    m_eepromData.altSet = limit(-990, altSet, 9990);
    setFlag(AltSetChanged);
}

void Application::setFlightLevel(int16_t newFlightLevel)
{
    if (m_flightLevel == newFlightLevel)
        return;

    m_flightLevel = newFlightLevel;
    setFlag(FlightLevelChanged);
}

void Application::setTemperature(int16_t newTemperature)
{
    if (m_temperature == newTemperature)
        return;

    m_temperature = newTemperature;
    setFlag(TempChanged);
}

void Application::setPressState(SensorState newPressState)
{
    m_pressState = newPressState;
}

void Application::setTempState(SensorState newTempState)
{
    m_tempState = newTempState;
}

bool Application::testFlag(uint16_t flag) const
{
    return (m_flags & flag) == flag;
}

void Application::setFlag(uint16_t flag)
{
    m_flags |= flag;
}

void Application::clearFlag(uint16_t flag)
{
    m_flags &= ~flag;
}

void Application::changeFlag(uint16_t flag)
{
    m_flags ^= flag;
}

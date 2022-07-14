#include "application.h"

#include <EEPROM.h>
#include <avr/wdt.h>
#include <Wire.h>

#include "encoder.h"
#include "button.h"

Application* Application::s_instance = nullptr;

Application::Application()
{
    s_instance = this;
    wdt_disable();
    setupPins();
    initSerial();
    m_display = new Display();
    m_display->clrScr();

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
        auto altSetCallback = [](int8_t dir)
        {
            aApplication->setAltSet(
                aApplication->altSet() + dir * (aApplication->m_button1->pressed() ? 100 : 10));
        };

        auto gndPressCallback = [](int8_t dir)
        {
            uint32_t value = aApplication->gndPress();
            if (aApplication->pressUnit() == PressUnits::mmHg)
                value = (((float)value / 133.322) + dir) * 133.322;
            else
                value += dir * 100;

            aApplication->setGndPress(value);
        };

        m_encoder1 = new Encoder(2, 3, altSetCallback);
        m_encoder2 = new Encoder(14, 15, gndPressCallback);
        m_button1 = new Button(4);
        m_button2 = new Button(13);
        m_buttonA = new Button(16);
        m_buttonB = new Button(12);

        m_display->drawFixedElements();
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

        reactEvents();
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

//    if (!Flags.TimerError && CurrentMillis - TimeShowMillis >= T_Time)
//    {
//        TimeShowMillis += T_Time;
//        Flags.TimeToPrintTime = true;
    //    }
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

void Application::reactEvents()
{
    reactTemperature();
    reactPressure();
    reactHumidity();
    reactVSpeed();
    reactAltSet();
    reactGndPress();
    reactTimer();
    reactAltUnitChanged();
    reactPressUnitChanged();

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

void Application::reactTimer()
{
    // TODO
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
    else
    {
        if (m_button2->event() != ButtonEvent::beNone)
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

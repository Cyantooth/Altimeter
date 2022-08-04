#include "display.h"

#include "settings.h"
#include "application.h"
#include "fonts/Tahoma42_neg.h"
#include "fonts/arial25bold.h"
#include "fonts/calibri14.h"

Display::Display()
{
    pinMode(38, OUTPUT);
    pinMode(39, OUTPUT);
    pinMode(40, OUTPUT);
    pinMode(41, OUTPUT);

    DDRA = 0xFF;
    DDRC = 0xFF;
        
    srst;
    delay(5);
    crst;
    delay(15);
    srst;
    delay(15);

    ccs;
    LCD_Write_COM(0x11);
    delay(20);
    LCD_Write_COM(0xD0);
    LCD_Write_DATA(0x07);
    LCD_Write_DATA(0x42);
    LCD_Write_DATA(0x18);

    LCD_Write_COM(0xD1);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x07);
    LCD_Write_DATA(0x10);

    LCD_Write_COM(0xD2);
    LCD_Write_DATA(0x01);
    LCD_Write_DATA(0x02);

    LCD_Write_COM(0xC0);
    LCD_Write_DATA(0x10);
    LCD_Write_DATA(0x3B);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x02);
    LCD_Write_DATA(0x11);

    LCD_Write_COM(0xC5);
    LCD_Write_DATA(0x03);

    LCD_Write_COM(0xC8);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x32);
    LCD_Write_DATA(0x36);
    LCD_Write_DATA(0x45);
    LCD_Write_DATA(0x06);
    LCD_Write_DATA(0x16);
    LCD_Write_DATA(0x37);
    LCD_Write_DATA(0x75);
    LCD_Write_DATA(0x77);
    LCD_Write_DATA(0x54);
    LCD_Write_DATA(0x0C);
    LCD_Write_DATA(0x00);

    LCD_Write_COM(0x36);
    #ifdef inverse
    LCD_Write_DATA(0x28);
    #else
    LCD_Write_DATA(0x2B);
    #endif

    LCD_Write_COM(0x3A);
    LCD_Write_DATA(0x55);

    LCD_Write_COM(0x2A);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x01);
    LCD_Write_DATA(0x3F);

    LCD_Write_COM(0x2B);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x00);
    LCD_Write_DATA(0x01);
    LCD_Write_DATA(0xE0);
    delay(120);
    LCD_Write_COM(0x29);
}

void Display::LCD_Write_COM(uint8_t value)
{
    crs;
    PORTA = 0;
    PORTC = value;
    lcd_strobe;
    srs;
}

void Display::LCD_Write_DATA(uint8_t value)
{
    PORTC = value;
    lcd_strobe;
}

void Display::setColumnAddr(uint16_t x1, uint16_t x2)
{
    LCD_Write_COM(0x2A);
    LCD_Write_DATA(x1 >> 8);
    LCD_Write_DATA(x1);
    LCD_Write_DATA(x2 >> 8);
    LCD_Write_DATA(x2);
}

void Display::setPageAddr(uint16_t y1, uint16_t y2)
{
    LCD_Write_COM(0x2B);
    LCD_Write_DATA(y1 >> 8);
    LCD_Write_DATA(y1);
    LCD_Write_DATA(y2 >> 8);
    LCD_Write_DATA(y2);
}

void Display::writeMemoryStart()
{
    LCD_Write_COM(0x2C);
}

void Display::clrScr()
{
    setArea(0, 0, disp_x_size - 1, disp_y_size - 1);
    prepareColor(C_Alt_BG_H, C_Alt_BG_L);
    for (uint16_t j = disp_y_size; j; j--)
        for (uint16_t i = disp_x_size; i; i--)
        {
            lcd_strobe;
        }
}

void Display::setArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    setColumnAddr(x1, x2);
    setPageAddr(y1, y2);
    writeMemoryStart();
}

void Display::prepareColor(uint8_t color_H, uint8_t color_L)
{
    srs;
    PORTA = color_H;
    PORTC = color_L;
}

void Display::fillSpace(uint8_t Color_H, uint8_t Color_L, uint16_t Count)
{
    prepareColor(Color_H, Color_L);
    for (uint16_t i = 0; i < Count; i++)
    {
        lcd_strobe;
    }
}

void Display::drawPackedBitmap(PGM_P bitmap, uint8_t FG_H, uint8_t FG_L, uint8_t BG_H, uint8_t BG_L, uint16_t Length)
{
    bool curBit = false;
    uint8_t C_H, C_L;

    srs;
    for (uint16_t j = 0; j < Length; j++)
    {
        if (curBit)
        {
            C_H = FG_H;
            C_L = FG_L;
        }
        else
        {
            C_H = BG_H;
            C_L = BG_L;
        }

        fillSpace(C_H, C_L, pgm_read_byte_near(bitmap + j));
        curBit = !curBit;
    }
}

void Display::drawFillRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint8_t C_Frame_H, uint8_t C_Frame_L, uint8_t C_BG_H, uint8_t C_BG_L, uint8_t Strength)
{
    // Frame
    uint16_t YS = Y + Strength;
    uint16_t YHS = Y + Height - Strength;
    uint16_t XW = X + Width;
    uint16_t WS = Width * Strength;
    uint16_t S2 = Strength << 1;
    uint16_t HZ = Height - S2;
    uint16_t HS = Strength * HZ;

    // Top
    setArea(X, Y, XW - 1, YS - 1);
    fillSpace(C_Frame_H, C_Frame_L, WS);

    // Left
    setArea(X, YS, X + Strength - 1, YHS - 1);
    fillSpace(C_Frame_H, C_Frame_L, HS);

    // Bottom
    setArea(X, YHS, XW - 1, Y + Height - 1);
    fillSpace(C_Frame_H, C_Frame_L, WS);

    // Right
    setArea(XW - Strength, YS, XW - 1, YHS - 1);
    fillSpace(C_Frame_H, C_Frame_L, HS);

    // Space
    setArea(X + Strength, YS, XW - Strength - 1, YHS - 1);
    fillSpace(C_BG_H, C_BG_L, (Width - S2) * HZ);
}

void Display::drawFixedElements()
{
     uint16_t dx = 30;
     uint16_t x = X_AltStart - dx;
     uint16_t dy = Y_AltEnd - Y_Middle + 7;
     uint16_t y;

    // Corner
    int t = -(dy >> 1);
    for (uint16_t i = 0; i <= dy; i++)
    {
        setColumnAddr(x, x + 2);
        y = Y_Middle - i;
        setPageAddr(y, y);
        writeMemoryStart();
        fillSpace(C_AltFrame_H, C_AltFrame_L, 3);

        y = Y_Middle + i;
        setPageAddr(y, y);
        writeMemoryStart();
        fillSpace(C_AltFrame_H, C_AltFrame_L, 3);

        t += dx;
        if (t >= 0)
        {
            x++;
            t -= dy;
        }
    }

    // Normal altitude
    uint16_t width = AltFixedDigits * altFontWidth - 5;
    setColumnAddr(x, x + width);
    y = Y_AltStart - 5;
    setPageAddr(y - 2, y);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, width * 3);
    y = Y_AltEnd + 5;
    setPageAddr(y, y + 2);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, width * 3);

    // Connectors
    x += width;
    setColumnAddr(x - 2, x);
    setPageAddr(Y_Ext_AltStart - 5, Y_AltStart - 5);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, (Y_AltStart - Y_Ext_AltStart + 10) * 3);
    setPageAddr(Y_AltEnd + 5, Y_Ext_AltEnd + 5);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, (Y_Ext_AltEnd - Y_AltEnd + 10) * 3);

    // Extended altitude
    uint16_t X_End = X_AltStart + (altFontWidth * 4) + 5;
    setColumnAddr(x - 2, X_End);
    width = X_End - x + 2;
    y = Y_Ext_AltStart - 5;
    setPageAddr(y - 2, y);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, width * 3);
    y = Y_Ext_AltEnd + 5;
    setPageAddr(y, y + 2);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, width * 3 + 3);

    // Ending
    x = X_End;
    setColumnAddr(x - 2, x);
    setPageAddr(Y_Ext_AltStart - 5, Y_Ext_AltEnd + 5);
    writeMemoryStart();
    fillSpace(C_AltFrame_H, C_AltFrame_L, (Y_Ext_AltEnd - Y_Ext_AltStart + 15) * 3);

    // Scale frame
    x = X_RulerStart - 2;
    width = RulerWidth + 2*2;
    setArea(x, Y_RulerStart - 2, x + width - 1, Y_RulerStart + RulerHeight + 2*2 - 1);
    uint16_t height = RulerHeight + 2*2;
    fillSpace(C_AltFrame_H, C_AltFrame_L, width * height);

    // Status bar
    drawFillRect(X_StatusBarStart, Y_StatusBarStart, StatusBarWidth, StatusBarHeight, C_StatusBarFrame_H, C_StatusBarFrame_L, C_StatusBar_BG_H, C_StatusBar_BG_L, 2);

    // Flight Level
    setArea(X_FlightLevelStart, Y_FlightLevelStart, X_FlightLevelStart + txtFLWidth - 1, Y_FlightLevelStart + txtFLHeight - 1);
    drawPackedBitmap(txtFL, C_FlightLevel_H, C_FlightLevel_L, C_Alt_BG_H, C_Alt_BG_L, sizeof(txtFL));

    drawVSpeedRuler();
}

void Display::printTemp(const int16_t temperature)
{
    auto value = temperature;
    if (value > 0)
    {
        if (value % 10 >= 5)
        {
            value += 5;
        }
    }
    else
    {
        if (value % 10 <= -5)
        {
            value -= 5;
        }
    }
    uint8_t strTemp[5];
    intToArray(value, &strTemp[0]);
    strTemp[3] = 14;
    strTemp[4] = 12;
    for (uint8_t i = 0; i < sizeof(strTemp) - 3; i++)
    {
        if (strTemp[i])
            break;
        strTemp[i] = 255;
    }

    setPageAddr(Y_TimeStart, Y_TimeStart + timeFontHeight - 1);
    uint16_t X = X_TempStart;
    for (uint8_t j = 0; j < sizeof(strTemp); j++)
    {
        uint8_t digit = strTemp[j];
        if (digit != m_lastTempArray[j])
        {
            setColumnAddr(X, X + timeFontWidth - 1);
            writeMemoryStart();
            srs;
            if (digit == 255)
            {
                fillSpace(C_StatusBar_BG_H, C_StatusBar_BG_L, timeFontHeight * timeFontWidth);
            }
            else
            {
                uint16_t charLen = pgm_read_word(&timeFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&timeFont_offsets[digit]);
                drawPackedBitmap(&timeFont[charOffset], C_Time_H, C_Time_L, C_StatusBar_BG_H, C_StatusBar_BG_L, charLen);
            }
            m_lastTempArray[j] = digit;
        }
        X += timeFontWidth;
    }
}

void Display::printFlightLevel(const int16_t flightLevel)
{
    uint8_t str[3];
    intToArray(flightLevel, &str[0], false, 3);

    setPageAddr(Y_FlightLevelStart, Y_FlightLevelStart + VSpeedFontHeight - 1);
    uint16_t x = X_FlightLevelStart + txtFLWidth + 6;
    for (uint8_t j = 0; j < sizeof(str); j++)
    {
        uint8_t digit = str[j];
        if (digit != m_lastFLArray[j])
        {
            setColumnAddr(x, x + VSpeedFontWidth - 1);
            writeMemoryStart();
            srs;
            if (digit == 255)
            {
                fillSpace(C_Pressure_BG_H, C_Pressure_BG_L, VSpeedFontHeight * VSpeedFontWidth);
            }
            else
            {
                uint16_t charLen = pgm_read_word(&VSpeedFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&VSpeedFont_offsets[digit]);
                drawPackedBitmap(&VSpeedFont[charOffset], C_FlightLevel_H, C_FlightLevel_L, C_Alt_BG_H, C_Alt_BG_L, charLen);
            }
            m_lastFLArray[j] = digit;
        }
        x += VSpeedFontWidth;
    }
}

void Display::printAltitude(const int32_t altitude)
{
    setAltDigitsArray(altitude);

    setPageAddr(Y_AltStart, Y_AltEnd);
    for (uint8_t i = 0; i < AltFixedDigits; i++)
    {
        if (m_altToDisplay[i] != m_altDisplayed[i])
        {
            printAltCharFixed(i);
            m_altDisplayed[i] = m_altToDisplay[i];
        }
    }

    setPageAddr(Y_Ext_AltStart, Y_Ext_AltEnd);
    for (uint8_t i = AltFixedDigits; i < 4; i++)
    {
        if ((m_altToDisplay[i] != m_altDisplayed[i]) || (m_altToDisplay[i + 2] != m_altDisplayed[i + 2]))
        {
            printAltCharFloated(i, altitude);
            m_altDisplayed[i] = m_altToDisplay[i];
            m_altDisplayed[i + 2] = m_altToDisplay[i + 2];
        }
    }

    // Ruler and leveler
    if (m_altToDraw != m_altDrawed)
    {
        drawRuler();
        drawLeveler(aApplication->altSet());
    }
}

void Display::printGndPress(const uint32_t gndPress)
{
    if (aApplication->pressUnit() == PressUnits::mmHg)
    {
        float value = gndPress / 133.322;
        if (value - (int)value >= 0.5)
            value += 0.5;
        printInfoNumber(X_GndPressStart, Y_GndPressStart, value, C_GndPress_H, C_GndPress_L, C_GndPress_BG_H, C_GndPress_BG_L);
    }
    else
    {
        printInfoNumber(X_GndPressStart, Y_GndPressStart, gndPress / 100, C_GndPress_H, C_GndPress_L, C_GndPress_BG_H, C_GndPress_BG_L);
    }
}

void Display::printAltSet(const int16_t altSet)
{
    printInfoNumber(X_AltSetStart, Y_AltSetStart, altSet, C_Leveler_H, C_Leveler_L, C_Leveler_BG_H, C_Leveler_BG_L);
}

void Display::printTime(const Time& curTime, uint8_t editPart)
{
    uint8_t strTime[10];
    strTime[0] = 255;
    strTime[1] = curTime.hour / 10;
    strTime[2] = curTime.hour % 10;
    strTime[3] = 11;
    strTime[4] = curTime.min / 10;
    strTime[5] = curTime.min % 10;
    strTime[6] = 11;
    strTime[7] = curTime.sec / 10;
    strTime[8] = curTime.sec % 10;
    strTime[9] = 255;

    setPageAddr(Y_TimeStart, Y_TimeStart + timeFontHeight - 1);
    uint16_t X = X_TimeStart;
    for (uint8_t j = 0; j < 10; j++)
    {
        uint8_t digit = strTime[j];
        if (digit != m_lastTimeArray[j])
        {
            setColumnAddr(X, X + timeFontWidth - 1);
            writeMemoryStart();
            srs;
            if (digit == 255)
            {
                fillSpace(C_StatusBar_BG_H, C_StatusBar_BG_L, timeFontHeight * timeFontWidth);
            }
            else
            {
                uint16_t charLen = pgm_read_word(&timeFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&timeFont_offsets[digit]);
                if (editPart == TimeFixedSegments[j])
                    drawPackedBitmap(&timeFont[charOffset], C_StatusBar_BG_H, C_StatusBar_BG_L, C_Time_H, C_Time_L, charLen);
                else
                    drawPackedBitmap(&timeFont[charOffset], C_Time_H, C_Time_L, C_StatusBar_BG_H, C_StatusBar_BG_L, charLen);
            }
            m_lastTimeArray[j] = digit;
        }
        X += timeFontWidth;
    }
}

void Display::printTimer(const Time& timer)
{
    uint8_t strTimer[10];
    strTimer[0] = 255;
    strTimer[1] = timer.hour / 10;
    strTimer[2] = timer.hour % 10;
    strTimer[3] = 11;
    strTimer[4] = timer.min / 10;
    strTimer[5] = timer.min % 10;
    strTimer[6] = 11;
    strTimer[7] = timer.sec / 10;
    strTimer[8] = timer.sec % 10;
    strTimer[9] = 255;

    setPageAddr(Y_DateStart, Y_DateStart + timeFontHeight - 1);
    uint16_t X = X_TimeStart;
    for (uint8_t j = 0; j < 10; j++)
    {
        uint8_t digit = strTimer[j];
        if (!aApplication->testFlag(DateShowed) || digit != m_lastDateArray[j])
        {
            setColumnAddr(X, X + timeFontWidth - 1);
            writeMemoryStart();
            srs;
            if (digit == 255)
            {
                fillSpace(C_Timer_BG_H, C_Timer_BG_L, timeFontHeight * timeFontWidth);
            }
            else
            {
                uint16_t charLen = pgm_read_word(&timeFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&timeFont_offsets[digit]);
                drawPackedBitmap(&timeFont[charOffset], C_Timer_H, C_Timer_L, C_Timer_BG_H, C_Timer_BG_L, charLen);
            }
            m_lastDateArray[j] = digit;
        }
        X += timeFontWidth;
    }
}

void Display::hideTimer()
{
    setArea(X_TimeStart - 4, StatusBarHeight, disp_x_size - 1, (StatusBarHeight << 1) - 1);
    fillSpace(C_Alt_BG_H, C_Alt_BG_L, (disp_x_size - X_TimeStart) * StatusBarHeight);
}

void Display::printDate(const Time &editTime, uint8_t editPart)
{
    if (aApplication->testFlag(DateShowed))
    {
        drawFillRect(X_TimeStart - 4, StatusBarHeight, disp_x_size - X_TimeStart + 4, StatusBarHeight, C_StatusBarFrame_H, C_StatusBarFrame_L, C_StatusBar_BG_H, C_StatusBar_BG_L, 2);
        aApplication->setFlag(DateShowed);
    }
    uint8_t strDate[10];
    strDate[0] = editTime.day / 10;
    strDate[1] = editTime.day % 10;
    strDate[2] = 15;
    strDate[3] = editTime.mon / 10;
    strDate[4] = editTime.mon % 10;
    strDate[5] = 15;
    intToArray(editTime.year, &strDate[6]);

    setPageAddr(Y_DateStart, Y_DateStart + timeFontHeight - 1);
    uint16_t X = X_TimeStart;
    for (uint8_t j = 0; j < 10; j++)
    {
        uint8_t digit = strDate[j];
        if (!aApplication->testFlag(DateShowed) || digit != m_lastDateArray[j])
        {
            setColumnAddr(X, X + timeFontWidth - 1);
            writeMemoryStart();
            srs;
            if (digit == 255)
            {
                fillSpace(C_StatusBar_BG_H, C_StatusBar_BG_L, timeFontHeight * timeFontWidth);
            }
            else
            {
                uint16_t charLen = pgm_read_word(&timeFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&timeFont_offsets[digit]);
                if (aApplication->testFlag(TimeEditMode) && editPart == DateFixedSegments[j])
                    drawPackedBitmap(&timeFont[charOffset], C_StatusBar_BG_H, C_StatusBar_BG_L, C_Time_H, C_Time_L, charLen);
                else
                    drawPackedBitmap(&timeFont[charOffset], C_Time_H, C_Time_L, C_StatusBar_BG_H, C_StatusBar_BG_L, charLen);
            }
            m_lastDateArray[j] = digit;
        }
        X += timeFontWidth;
    }
}

void Display::hideDate()
{
    setArea(X_TimeStart - 4, StatusBarHeight, disp_x_size - 1, (StatusBarHeight << 1));
    fillSpace(C_Alt_BG_H, C_Alt_BG_L, (disp_x_size - X_TimeStart + 4) * StatusBarHeight);
    aApplication->clearFlag(DateShowed);
}

void Display::printVSpeed(int16_t vSpeed)
{
    uint8_t digits[5];

    setPageAddr(Y_VSpeedStart, Y_VSpeedStart + VSpeedFontHeight - 1);
    intToArray(vSpeed, &digits[0]);
    digits[4] = digits[3];
    digits[3] = 11; // Dot

    bool showDigits = false;
    uint16_t X = X_VSpeedStart;
    for (uint8_t k = 0; k < sizeof(digits); k++)
    {
        uint8_t digit = digits[k];
        if (digit || (k == sizeof(digits) - 3))
            showDigits = true;
        if (digit != lastVSpeedArray[k])
        {
            setColumnAddr(X, X + VSpeedFontWidth - 1);
            writeMemoryStart();
            srs;

            if (showDigits)
            {
                uint16_t charLen = pgm_read_word(&VSpeedFont_sizes[digit]);
                uint16_t charOffset = pgm_read_word(&VSpeedFont_offsets[digit]);
                drawPackedBitmap(&VSpeedFont[charOffset], C_VSpeed_H, C_VSpeed_L, C_VSpeed_BG_H , C_VSpeed_BG_L, charLen);
            }
            else
            {
                fillSpace(C_VSpeed_BG_H , C_VSpeed_BG_L, VSpeedFontHeight * VSpeedFontWidth);
            }
            lastVSpeedArray[k] = digit;
        }
        X += VSpeedFontWidth;
    }
}

void Display::printFatal()
{
    clrScr();
    setCurrentFont(&Arial25BoldInfo);
    printText(10, 10, "Sensor reading error!");
    setCurrentFont(&Calibri14Info);
    printText(10, 45, "There is a problem with reading data.");
    printText(10, 64, "Please check cable or sensor.");
    printText(10, disp_y_size - m_currentFont->heightPages - 10, "Waiting WDT for reboot...");
}

void Display::drawAltUnits(const AltUnits altUnit)
{
    if (altUnit == AltUnits::Meters)
    {
        setArea(X_AltUnitStart, Y_AltUnitStart, X_AltUnitStart + UnitsWidth - 1, Y_AltUnitStart + UnitsHeight - 1);
        drawPackedBitmap(txtMeters, C_Alt_H, C_Alt_L, C_Alt_BG_H, C_Alt_BG_L, txtMetersLen);
        setArea(X_VSpeedUnitStart, Y_VSpeedUnitStart, X_VSpeedUnitStart + txtMSWidth - 1, Y_VSpeedUnitStart + txtMSHeight - 1);
        drawPackedBitmap(txtMS, C_VSpeed_H, C_VSpeed_L, C_VSpeed_BG_H , C_VSpeed_BG_L, txtMSLen);
    }
}

void Display::drawPressUnit(const PressUnits pressUnit)
{
    setArea(X_PressUnitStart, Y_PressUnitStart, X_PressUnitStart + txtPressUnitWidth - 1, Y_PressUnitStart + txtPressUnitHeight - 1);
    if (pressUnit == PressUnits::HPA)
    {
        if (aApplication->testFlag(ApplicationFlags::PressUnitChangeReady))
            drawPackedBitmap(txtHPA, C_GndPress_BG_H, C_GndPress_BG_L, C_GndPress_H, C_GndPress_L, txtHPALen);
        else
            drawPackedBitmap(txtHPA, C_GndPress_H, C_GndPress_L, C_GndPress_BG_H, C_GndPress_BG_L, txtHPALen);
    }
    else
    {
        if (aApplication->testFlag(ApplicationFlags::PressUnitChangeReady))
            drawPackedBitmap(txtMmHg, C_GndPress_BG_H, C_GndPress_BG_L, C_GndPress_H, C_GndPress_L, txtMmHgLen);
        else
            drawPackedBitmap(txtMmHg, C_GndPress_H, C_GndPress_L, C_GndPress_BG_H, C_GndPress_BG_L, txtMmHgLen);
    }
}

void Display::drawVSpeedRuler()
{
    uint16_t x, y, dx, dy;
    x = X_VSpeedArrowStart - 4;
    y = Y_VSpeedRulerStart + VSpeedRulerHeight + 8;
    dx = X_VSpeedRulerStart + VSpeedRulerWidth + 4 - x;
    dy = 1;
    setArea(x, y, x + dx - 1, y + dy - 1);
    fillSpace(C_StatusBarFrame_H, C_StatusBarFrame_L, dx * dy);

    setArea(X_VSpeedRulerStart, Y_VSpeedRulerStart, X_VSpeedRulerStart + VSpeedRulerWidth - 1, Y_VSpeedRulerStart + VSpeedRulerHeight - 1);
    drawPackedBitmap(VSpeedRuler, C_VSpeed_H, C_VSpeed_L, C_VSpeed_BG_H , C_VSpeed_BG_L, VSpeedRulerLen);
}

void Display::intToArray(const float value, uint8_t* array, bool mainAlt, uint8_t digits)
{
    int32_t val;
    int8_t c;
    for (c = 0; c < digits; c++)
    {
        array[c] = 0;
    }

    bool negativeValue = value < 0;
    if (negativeValue /*|| (mainAlt && m_negativeAlt)*/)
        val = -value;
    else
        val = value;
    while (val > 0)
    {
        uint8_t digit = val % 10;
        c--;
        array[c] = digit;
        val = (val - digit) / 10;
    }
    if (negativeValue /*|| (mainAlt && m_negativeAlt)*/)
    {
        c--;
        if (c <= 0)
            c = 0;
        else if (c > (AltFixedDigits - 1))
            c = AltFixedDigits - 1;
        array[c] = 10;
    }
}

void Display::setAltDigitsArray(const int32_t value)
{
    if (value == 0)
    {
        for (uint8_t i = 0; i < 6; i++)
            m_altToDisplay[i] = 0;
    }
    else
    {
      if (value >= 1000000)
      {
          m_altToDisplay[0] = m_altToDisplay[1] = m_altToDisplay[2] = m_altToDisplay[3] = 9;
          m_altToDisplay[4] = m_altToDisplay[5] = altFontHalfHeight;
      }
      else
      {
          m_altToDisplay[5] = (abs(value) % 100) * altFontHeight / 100;
          m_altToDraw = (float)value / 100.0;
          intToArray(m_altToDraw, &m_altToDisplay[0], true);
          if (abs(value) % 10000 >= 9950)
          {
              if (m_altToDisplay[1] == 10)
              {
                  m_altToDisplay[0] = 10;
                  m_altToDisplay[1] = 1;
              }
              else
              {
                  m_altToDisplay[1] = (m_altToDisplay[1] + 1) % 10;
                  if (m_altToDisplay[1] == 0)
                      m_altToDisplay[0]++;
              }
          }
          if (m_altToDisplay[3] == 9)
              m_altToDisplay[4] = m_altToDisplay[5];
          else
              m_altToDisplay[4] = 0;
      }
    }
}

void Display::printAltCharFloated(const uint8_t pos, const long& value)
{
    uint8_t digit = m_altToDisplay[pos];
    uint16_t X_Addr = pos * altFontWidth + X_AltStart;

    setColumnAddr(X_Addr, X_Addr + altFontWidth - 1);
    writeMemoryStart();
    srs;

    int8_t Y_charOffset = altFontHalfHeight - m_altToDisplay[pos + 2];
    bool shift = Y_charOffset <= 0;
    if (value < 0)
        Y_charOffset = -Y_charOffset;

    bool reverse = (digit == 0) && (value > -950) && (value < 950);
    int8_t scrollDigist[3];
    uint8_t countDigits;
    if (Y_charOffset == 0)
    {
        countDigits = 2;
        if (value < 0)
        {
            scrollDigist[0] = digit;
            scrollDigist[1] = digit + 1;
        }
        else
        {
            scrollDigist[0] = digit + 1;
            scrollDigist[1] = digit;
        }
    }
    else
    {
        countDigits = 3;
        if (Y_charOffset < 0)
        {
            Y_charOffset += altFontHeight;
            if (value < 0)
            {
                if (reverse)
                    scrollDigist[0] = digit + 1;
                else
                    scrollDigist[0] = digit - 1;
                scrollDigist[1] = digit;
                scrollDigist[2] = digit + 1;
            }
            else
            {
                scrollDigist[0] = digit + 2;
                scrollDigist[1] = digit + 1;
                scrollDigist[2] = digit;
            }
        }
        else
        {
            if (value < 0)
            {
                scrollDigist[0] = digit;
                scrollDigist[1] = digit + 1;
                scrollDigist[2] = digit + 2;
            }
            else
            {
                scrollDigist[0] = digit + 1;
                scrollDigist[1] = digit;
                if (reverse)
                    scrollDigist[2] = digit + 1;
                else
                    scrollDigist[2] = digit - 1;
            }
        }
    }
    for (uint8_t j = 0; j < 3; j++)
        if (scrollDigist[j] >= 10)
            scrollDigist[j] = scrollDigist[j] - 10;
        else if (scrollDigist[j] < 0)
            scrollDigist[j] = scrollDigist[j] + 10;

    int16_t pixelsToShow = Y_charOffset * altFontWidth;
    for (uint8_t j = 0; j < countDigits; j++)
    {

        uint8_t curDigit = scrollDigist[j];
        uint8_t color_H, color_L;
        if ((!shift && curDigit == digit) || (shift && curDigit == (digit + 1) % 10))
        {
            color_H = C_Alt_H;
            color_L = C_Alt_L;
        }
        else
        {
            color_H = C_Alt_Shaded_H;
            color_L = C_Alt_Shaded_L;
        }

        uint16_t charLen = pgm_read_word(&altFont_sizes[curDigit]);
        uint16_t charOffset = pgm_read_word(&altFont_offsets[curDigit]);
        int16_t pixelsToPass = Y_charOffset * altFontWidth;
        bool curBit = false;
        uint16_t mapOffset = 0;
        uint8_t curCount;
        while (pixelsToPass > 0)
        {
            curCount = pgm_read_byte(&altFont[charOffset + mapOffset++]);
            pixelsToPass -= curCount;
            curBit = !curBit;
        }
        if (curBit) // sic!
        {
            prepareColor(C_Alt_BG_H, C_Alt_BG_L);
        }
        else
        {
            prepareColor(color_H, color_L);
        }

        while (pixelsToPass < 0)
        {
            lcd_strobe;
            pixelsToPass++;
        }

        bool finished = false;

        for (uint16_t i = mapOffset; i < charLen; i++)
        {
            curCount = pgm_read_byte(&altFont[charOffset + i]);
            if (curBit)
            {
                prepareColor(color_H, color_L);
            }
            else
            {
                prepareColor(C_Alt_BG_H, C_Alt_BG_L);
            }

            for (uint8_t k = 0; k < curCount; k++)
            {
                lcd_strobe;
                if (j == 2)
                {
                    if (!(--pixelsToShow))
                    {
                        finished = true;
                        break;
                    }
                }
            }
            if (finished)
                break;
            curBit = !curBit;
        }
        Y_charOffset = 0; // Offset is only for the first symbol
    }
}

void Display::printAltCharFixed(const uint8_t pos)
{
    uint8_t digit = m_altToDisplay[pos];
    uint16_t X_Addr = pos * altFontWidth + X_AltStart;

    setColumnAddr(X_Addr, X_Addr + altFontWidth - 1);
    writeMemoryStart();
    srs;

    bool drawSpace = false;
    if (digit == 0)
    {
        drawSpace = true;
        for (uint8_t i = 0; i < pos; i++)
        {
            if (m_altToDisplay[i] != 0)
            {
                drawSpace = false;
                break;
            }
        }
    }

    if (drawSpace)
    {
        fillSpace(C_Alt_BG_H, C_Alt_BG_L, altFontMapSize);
    }
    else
    {
        uint16_t charLen = pgm_read_word(&altFont_sizes[digit]);
        uint16_t charOffset = pgm_read_word(&altFont_offsets[digit]);
        drawPackedBitmap(&altFont[charOffset], C_Alt_H, C_Alt_L, C_Alt_BG_H, C_Alt_BG_L, charLen);
    }
}

void Display::printInfoNumber(
    const uint16_t posX,
    const uint16_t posY,
    const int16_t value,
    const uint8_t FG_H,
    const uint8_t FG_L,
    const uint8_t BG_H,
    const uint8_t BG_L)
{
    uint8_t digits[4];

    setPageAddr(posY, posY + infoFontHeight - 1);
    intToArray(value, &digits[0]);

    bool showDigits = false;
    uint16_t X = posX;
    for (uint8_t k = 0; k < sizeof(digits); k++)
    {
        uint8_t digit = digits[k];
        setColumnAddr(X, X + infoFontWidth - 1);
        writeMemoryStart();
        srs;

        if (digit || (k == sizeof(digits) - 1))
            showDigits = true;
        if (showDigits)
        {
            uint16_t charLen = pgm_read_word(&infoFont_sizes[digit]);
            uint16_t charOffset = pgm_read_word(&infoFont_offsets[digit]);
            drawPackedBitmap(&infoFont[charOffset], FG_H, FG_L, BG_H, BG_L, charLen);
        }
        else
        {
            fillSpace(BG_H, BG_L, infoFontHeight * infoFontWidth);
        }
        X += infoFontWidth;
    }
}

void Display::drawRuler()
{
    enum colorNames { clBG = 0, clSky = 1, clGround = 2, clFrame = 3, clText = 4 };
    const uint16_t colorIndices[5] = {
        (C_Alt_BG_H << 8) | C_Alt_BG_L,
        (C_RulerSky_H << 8) | C_RulerSky_L,
        (C_RulerGround_H << 8) | C_RulerGround_L,
        (C_AltFrame_H << 8) | C_AltFrame_L,
        (C_Alt_H << 8) | C_Alt_L
    };
    const uint16_t frameBufferSize = RulerWidth * rulerFontHeight;

    setColumnAddr(X_RulerStart, X_RulerStart + RulerWidth - 1);
    setPageAddr(Y_RulerStart, Y_RulerStart + RulerHeight - 1);
    writeMemoryStart();
    srs;

    int16_t altOffset = m_altToDraw + (RulerHeight >> 1);
    int8_t tmpAlt = altOffset % 100;
    while (tmpAlt < rulerFontHalfHeight)
    {
        tmpAlt += 100;
    }

    uint8_t BG_H, BG_L;
    int8_t cnt100, cnt50 ,cnt10;
    uint8_t CurrentBG;
    if (altOffset < 0)
    {
        cnt100 = (altOffset % 100) + 101;
        cnt50  = (altOffset %  50) + 51;
        cnt10  = (altOffset %  10) + 11;
        BG_H = C_RulerGround_H;
        BG_L = C_RulerGround_L;
        CurrentBG = clGround;
    }
    else
    {
        cnt100 = (altOffset % 100) + 1;
        cnt50  = (altOffset %  50) + 1;
        cnt10  = (altOffset %  10) + 1;
        if (altOffset > 10000)
        {
            BG_H = C_Alt_BG_H;
            BG_L = C_Alt_BG_L;
            CurrentBG = clBG;
        }
        else
        {
            BG_H = C_RulerSky_H;
            BG_L = C_RulerSky_L;
            CurrentBG = clSky;
        }
    }
    uint8_t currentIndex = CurrentBG;

    int16_t altStartFrame = altOffset - tmpAlt + rulerFontHalfHeight;
    while (altStartFrame > 10000)
    {
        altStartFrame -= 100;
    }
    int16_t altEndFrame = altStartFrame - rulerFontHeight;
    int16_t altToRuler;
    bool inFrame = false;
    uint8_t frameBuffer[frameBufferSize];
    uint8_t digits[4];

    for (uint16_t y = 0; y < RulerHeight; y++)
    {
        int16_t RelAlt = altOffset - y;
        uint8_t Y_Frame;

        switch (RelAlt)
        {
        case 0:
            BG_H = C_RulerGround_H;
            BG_L = C_RulerGround_L;
            CurrentBG = clGround;
            break;

        case 10000:
            BG_H = C_RulerSky_H;
            BG_L = C_RulerSky_L;
            CurrentBG = clSky;
            break;
        }

        if (inFrame)
        {
            if (RelAlt == altEndFrame)
            {
                // Exit out of the frame
                inFrame = false;
                altEndFrame -= 100;
                altStartFrame -= 100;
                if (altEndFrame <= altOffset - RulerHeight)
                    altStartFrame -= 100; //< Go beyond the bounds
                intToArray(altToRuler, &digits[0]);

                bool showDigits = false;
                uint8_t x_Frame = 4;
                for (uint8_t i = 0; i < 4; i++)
                {
                    uint8_t digit = digits[i];
                    if (digit)
                        showDigits = true;
                    if (showDigits)
                    {
                        FONT_CHAR_INFO charInfo = rulerFontDescriptors[digit];
                        uint16_t fontCnt = charInfo.CharOffset;
                        for (uint8_t y_loc = 0; y_loc < rulerFontHeight; y_loc++)
                        {
                            uint8_t fontByte = pgm_read_byte(&rulerFont[fontCnt++]);
                            uint8_t fontMask = 0b10000000;
                            for (uint8_t x_loc = 0; x_loc < charInfo.CharWidth; x_loc++)
                            {
                                if (!fontMask)
                                {
                                    fontByte = pgm_read_byte(&rulerFont[fontCnt++]);
                                    fontMask = 0b10000000;
                                }
                                uint16_t bufferCnt = y_loc * RulerWidth + x_loc + x_Frame;
                                if (fontByte & fontMask)
                                    frameBuffer[bufferCnt] = clText;
                                else
                                    frameBuffer[bufferCnt] = CurrentBG;
                                fontMask >>= 1;
                            }
                        }
                        x_Frame += charInfo.CharWidth + 1;
                    }
                    else
                        x_Frame += 8;
                }

                uint8_t c_ind, cur_c_ind = 255;
                for (uint16_t i = 0; i < frameBufferSize; i++)
                {
                    c_ind = frameBuffer[i];
                    if (c_ind != cur_c_ind)
                    {
                        uint16_t color = colorIndices[c_ind];
                        prepareColor(color >> 8, color);
                        cur_c_ind = c_ind;
                    }
                    lcd_strobe;
                }
            }
            else
            {
                currentIndex = CurrentBG;
            }
        }
        else
        {
            if ((RelAlt > 0) && (RelAlt == altStartFrame))
                inFrame = true;
            else
                prepareColor(BG_H, BG_L);
        }
        if (inFrame)
            Y_Frame = altStartFrame - RelAlt;

        // Draw the ruler
        uint8_t x_Divider;
        cnt10--;
        cnt50--;
        cnt100--;
        if (!cnt10)
        {
            cnt10 = 10;
            if (!cnt50)
            {
                cnt50 = 50;
                if (!cnt100)
                {
                    cnt100 = 100;
                    x_Divider = 0;
                    altToRuler = RelAlt;
                }
                else
                {
                    x_Divider = RulerWidth - 16;
                }
            }
            else
            {
                x_Divider = RulerWidth - 8;
            }
        }
        else
            x_Divider = RulerWidth;
        if (RelAlt > 10000)
            x_Divider = RulerWidth;
        for (uint8_t x = 0; x < RulerWidth; x++)
        {
            if (inFrame)
            {
                if (x == x_Divider)
                    currentIndex = clFrame;
                frameBuffer[Y_Frame * RulerWidth + x] = currentIndex;
            }
            else
            {
                if (x == x_Divider)
                    prepareColor(C_AltFrame_H, C_AltFrame_L);
                lcd_strobe;
            }
        }
    }
    m_altDrawed = m_altToDraw;
}

void Display::printText(uint16_t x0, uint16_t y0, char* text)
{
    int stl = strlen(text);
    uint16_t x = x0;
    uint16_t width;
    uint16_t height = m_currentFont->heightPages;
    setPageAddr(y0, y0 + height - 1);
    for (int i = 0; i < stl; i++)
    {
        char curChar = *text++;
        EXT_FONT_CHAR_INFO efci = m_currentFont->charInfo[curChar - m_currentFont->startChar];
        if (curChar >= m_currentFont->startChar && curChar <= m_currentFont->endChar)
        {
            width = efci.CharWidth;
            setColumnAddr(x, x + width - 1);
            writeMemoryStart();
            drawPackedBitmap(&m_currentFont->data[efci.CharOffset], C_WarningText_H, C_WarningText_L, C_WarningText_BG_H, C_WarningText_BG_L, efci.CharLength);
        }
        else
        {
            width = m_currentFont->spacePixels * 2;
        }
        x += width + m_currentFont->spacePixels;
    }
}

void Display::setCurrentFont(const FONT_INFO* newFont)
{
    m_currentFont = newFont;
}

void Display::drawLeveler(int16_t _altSet)
{
    int16_t Y_new = m_altToDraw - _altSet + Y_Middle - LevelerHalfHeight;
    if (Y_new < 0)
        Y_new = 0;
    else if (Y_new > disp_y_size - LevelerHeight)
        Y_new = disp_y_size - LevelerHeight;


    if (Y_LastLeveler == Y_new)
    {
        return;
    }

    setColumnAddr(X_LevelerStart, X_LevelerStart + LevelerWidth - 1);

    // Erase previous symbol
    uint16_t Y_start, Y_end;
    if (Y_LastLeveler < Y_new)
    {
        Y_start = Y_LastLeveler;
        Y_end = min(Y_new - 1, Y_LastLeveler + LevelerHeight - 1);
    }
    else
    {
        Y_start = max(Y_LastLeveler, Y_new + LevelerHeight);
        Y_end = Y_LastLeveler + LevelerHeight - 1;
    }

    setPageAddr(Y_start, Y_end);
    writeMemoryStart();
    fillSpace(C_Alt_BG_H, C_Alt_BG_L, LevelerWidth * (Y_end - Y_start + 1));

    // Draw new symbol
    setPageAddr(Y_new, Y_new + LevelerHeight - 1);
    writeMemoryStart();
    drawPackedBitmap(Leveler, C_Leveler_H, C_Leveler_L, C_Leveler_BG_H, C_Leveler_BG_L, sizeof(Leveler));
    Y_LastLeveler = Y_new;
}

void Display::drawTimerBackground(bool inTimerColors)
{

    if (inTimerColors)
    {
        drawFillRect(
            X_TimeStart - 4,
            StatusBarHeight,
            disp_x_size - X_TimeStart + 4,
            StatusBarHeight - 1,
            C_TimerFrame_H,
            C_TimerFrame_L,
            C_Timer_BG_H,
            C_Timer_BG_L,
            2);
    }
    else
    {
        drawFillRect(
            X_TimeStart - 4,
            StatusBarHeight,
            disp_x_size - X_TimeStart + 4,
            StatusBarHeight - 1,
            C_StatusBarFrame_H,
            C_StatusBarFrame_L,
            C_StatusBar_BG_H,
            C_StatusBar_BG_L,
            2);
    }
}

void Display::drawVSpeed(const int16_t value, const uint8_t warningLevel)
{
    static uint8_t lastWarningLevel = 255;
    static uint16_t Y_LastBound = 0;

    bool neg = (value < 0);
    uint16_t LogIndex = neg ? -value : value;
    if (LogIndex > MaxVSpeed)
        LogIndex = MaxVSpeed;
    uint8_t Y_offset = pgm_read_byte(&Logarithms[LogIndex]);
    uint16_t Y_bound = neg ? Y_VSpeedZero + Y_offset : Y_VSpeedZero - Y_offset;

#define a Y_bound
#define b Y_LastBound
#define c Y_VSpeedZero
#define H VSpeedArrowHeight

    if (a == b && lastWarningLevel == warningLevel)
        return;

    int16_t Y_ClearStart, Y_ClearEnd;
    int16_t Y_DrawStart, Y_DrawEnd;

    if (a < b)
    {
        if (b <= c)
        {
            // a < b ≤ c
            Y_ClearStart = Y_ClearEnd = -1;
            Y_DrawStart = a;
            Y_DrawEnd = min(b + H - 1, c);
        }
        else if (a <= c)
        {
            // a ≤ c < b
            Y_ClearStart = c + 1;
            Y_ClearEnd = b;
            Y_DrawStart = a;
            Y_DrawEnd = c;
        }
        else
        {
            // c < a < b
            Y_ClearStart = a + 1;
            Y_ClearEnd = b;
            Y_DrawStart = a - H + 1;
            Y_DrawEnd = a;
        }
    }
    else // b < a
    {
        if (c <= b)
        {
            // c ≤ b < a
            Y_ClearStart = Y_ClearEnd = -1;
            Y_DrawStart = max(c, b - H + 1);
            Y_DrawEnd = a;
        }
        else if (a <= c)
        {
            // b < a ≤ c
            Y_ClearStart = b;
            Y_ClearEnd = a - 1;
            Y_DrawStart = a;
            Y_DrawEnd = min(b + H - 1, c);
        }
        else
        {
            // b ≤ c < a
            Y_ClearStart = b;
            Y_ClearEnd = c - 1;
            Y_DrawStart = c;
            Y_DrawEnd = a;
        }
    }
    if (lastWarningLevel != warningLevel)
    {
        if (a <= c)
        {
            Y_DrawStart = a;
            Y_DrawEnd = c;
        }
        else
        {
            Y_DrawStart = c;
            Y_DrawEnd = a;
        }
        lastWarningLevel = warningLevel;
    }

    setColumnAddr(X_VSpeedArrowStart, X_VSpeedArrowStart + VSpeedArrowWidth - 1);
    if (Y_ClearStart >= 0)
    {
        setPageAddr(Y_ClearStart, Y_ClearEnd);
        writeMemoryStart();
        fillSpace(C_VSpeed_BG_H, C_VSpeed_BG_L, VSpeedArrowWidth * (Y_ClearEnd - Y_ClearStart + 1));
    }

    if (value == 0)
    {
        setPageAddr(Y_VSpeedZero, Y_VSpeedZero);
        writeMemoryStart();
        fillSpace(C_VSpeed_H, C_VSpeed_L, VSpeedArrowWidth);
    }
    else
    {
        uint8_t Y_arrow;
        uint8_t Y_pointBack = 0;
        uint8_t Color_H;
        uint8_t Color_L;
        switch (warningLevel)
        {
        case 0:
            Color_H = C_VSpeed_H;
            Color_L = C_VSpeed_L;
            break;

        case 1:
            Color_H = C_VSpeedWarning_H;
            Color_L = C_VSpeedWarning_L;
            break;

        case 2:
            Color_H = C_VSpeedDanger_H;
            Color_L = C_VSpeedDanger_L;
            break;
        }

        if (neg)
        {
            // Descending
            int8_t Y_point = VSpeedArrowHeight - Y_offset - 1;
            if (Y_point < 0)
                Y_point = 0;
            Y_arrow = Y_VSpeedZero;
            setPageAddr(Y_VSpeedZero, Y_bound);
            writeMemoryStart();
            while (Y_arrow <= Y_bound)
            {
                for (uint8_t X_idx = 0; X_idx < VSpeedArrowWidthBytes; X_idx++)
                {
                    uint8_t BitMap = pgm_read_byte(&VSpeedArrowDn[Y_point * VSpeedArrowWidthBytes + X_idx]);
                    if ((LogIndex >= 10) && (Y_arrow <= Y_VSpeedDnLimit))
                    {
                        BitMap ^= pgm_read_byte(&VSpeedArrowDn[Y_pointBack * VSpeedArrowWidthBytes + X_idx]);
                    }
                    for (uint8_t mask = 0b10000000; mask > 0; mask >>= 1)
                    {
                        if (BitMap & mask)
                        {
                            prepareColor(Color_H, Color_L);
                        }
                        else
                        {
                            prepareColor(C_VSpeed_BG_H, C_VSpeed_BG_L);
                        }
                        lcd_strobe;
                    }
                }
                Y_pointBack++;
                Y_arrow++;
                if (Y_arrow >= Y_bound - VSpeedArrowHeight + 2)
                    Y_point++;
            }
        }
        else
        {
            // Climbing
            int8_t Y_point = 0;
            Y_arrow = Y_bound;
            setPageAddr(Y_arrow, Y_VSpeedZero);
            writeMemoryStart();
            while (Y_arrow <= Y_VSpeedZero)
            {
                for (uint8_t X_idx = 0; X_idx < VSpeedArrowWidthBytes; X_idx++)
                {
                    uint8_t BitMap = pgm_read_byte(&VSpeedArrowUp[Y_point * VSpeedArrowWidthBytes + X_idx]);
                    if ((LogIndex >= 10) && (Y_arrow >= Y_VSpeedUpLimit))
                    {
                        BitMap ^= pgm_read_byte(&VSpeedArrowUp[Y_pointBack * VSpeedArrowWidthBytes + X_idx]);
                    }
                    for (uint8_t mask = 0b10000000; mask > 0; mask >>= 1)
                    {
                        if (BitMap & mask)
                        {
                            prepareColor(Color_H, Color_L);
                        }
                        else
                        {
                            prepareColor(C_VSpeed_BG_H, C_VSpeed_BG_L);
                        }
                        lcd_strobe;
                    }
                }
                if (Y_arrow >= Y_VSpeedUpLimit)
                    Y_pointBack++;
                Y_arrow++;
                if (Y_point < VSpeedArrowHeight - 1)
                    Y_point++;
            }
        }
    }

    Y_LastBound = Y_bound;
}

void Display::redrawAllTimeSegments()
{
    for (int i = 0; i < sizeof(m_lastTimeArray); ++i)
        m_lastTimeArray[i] = 255;
}

void Display::redrawAllDateSegments()
{
    for (int i = 0; i < sizeof(m_lastDateArray); ++i)
        m_lastDateArray[i] = 255;
}

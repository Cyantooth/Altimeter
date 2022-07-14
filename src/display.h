#pragma once

#include <Arduino.h>

#define scs PORTG |=  0b00000010
#define ccs PORTG &= ~0b00000010
#define srs PORTD |=  0b10000000
#define crs PORTD &= ~0b10000000
#define srst PORTG |=  0b00000001
#define crst PORTG &= ~0b00000001
#define lcd_strobe PORTG &= ~0b00000100; PORTG |= 0b00000100

enum class AltUnits;
enum class PressUnits;

class Display
{
public:
    Display();
    
    void clrScr();
    void drawFixedElements();

    void printTemp(const int16_t temperature);
    void printFlightLevel(const int16_t flightLevel);
    void printAltitude(const int32_t altitude);
    void printGndPress(const uint32_t gndPress);
    void printAltSet(const int16_t altSet);
    void drawAltUnits(const AltUnits altUnit);
    void drawPressUnit(const PressUnits pressUnit);
    void drawLeveler(int16_t _altSet);

private:
    void LCD_Write_COM(uint8_t value);
    void LCD_Write_DATA(uint8_t value);
    void setArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void prepareColor(uint8_t color_H, uint8_t color_L);
    void fillSpace(uint8_t Color_H, uint8_t Color_L, uint16_t Count);
    void setColumnAddr(uint16_t x1, uint16_t x2);
    void setPageAddr(uint16_t y1, uint16_t y2);
    void writeMemoryStart();
    void drawFillRect(
        uint16_t X, 
        uint16_t Y, 
        uint16_t Width, 
        uint16_t Height, 
        uint8_t C_Frame_H, 
        uint8_t C_Frame_L, 
        uint8_t C_BG_H, 
        uint8_t C_BG_L, 
        uint8_t Strength);
    
    void drawPackedBitmap(
        PGM_P bitmap, 
        uint8_t FG_H, 
        uint8_t FG_L, 
        uint8_t BG_H, 
        uint8_t BG_L, 
        uint16_t Length);

    void drawVSpeedRuler();

    void intToArray(const float value, uint8_t* array, bool mainAlt = false, uint8_t digits = 4);
    void setAltDigitsArray(const int32_t value);
    void printAltCharFloated(const uint8_t pos, const long &value);
    void printAltCharFixed(const uint8_t pos);
    void printInfoNumber(
        const uint16_t posX,
        const uint16_t posY,
        const int16_t value,
        const uint8_t FG_H,
        const uint8_t FG_L,
        const uint8_t BG_H,
        const uint8_t BG_L);
    void drawRuler();

private:
//    bool m_negativeAlt = false;
    uint8_t m_lastTempArray[5] = {255,255,255,255,255};
    uint8_t lastFLArray[3] = {255,255,255};
    uint8_t altDisplayed[6] = {255,255,255,255,255,255};
    uint8_t altToDisplay[6] = {0,0,0,0,0,0};
    float altToDraw = 0.0;
    int32_t altDrawed = 32767;
    uint16_t Y_LastLeveler = 32767;
};

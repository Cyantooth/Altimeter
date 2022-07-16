#pragma once

#include "fonts\Tahoma82_neg.h"
#include "fonts\Arial25.h"
#include "fonts\Arial21.h"
#include "fonts\ArialNarrow20.h"
#include "inscriptions.h"

#include <stdint.h>

// *** [ Setting constants ] ***

static constexpr uint8_t AltFixedDigits = 2;
static constexpr uint16_t MaxVSpeed = 200;

// Тайминги
static constexpr uint16_t T_Time = 100;
static constexpr uint32_t T_TimeEditTimeout = 65000;

// Координаты и размеры элементов
static constexpr uint16_t disp_x_size = 480;
static constexpr uint16_t disp_y_size = 320;
static constexpr uint16_t Y_Middle = disp_y_size / 2;

static constexpr uint16_t altFontMapSize = altFontWidth * altFontHeight;
static constexpr uint16_t altFontHalfHeight = altFontHeight / 2;
static constexpr uint16_t X_AltStart = 200;
static constexpr uint16_t Y_AltStart = Y_Middle - altFontHalfHeight;
static constexpr uint16_t Y_AltEnd = Y_AltStart + altFontHeight - 1;
static constexpr uint16_t X_AltSetStart = 184;
static constexpr uint16_t Y_AltSetStart = 60;
static constexpr uint16_t Y_TimeStart = 6;
static constexpr uint16_t X_TimeStart = 336;
static constexpr uint16_t X_StatusBarStart = 184;
static constexpr uint16_t Y_StatusBarStart = 0;
static constexpr uint16_t StatusBarHeight = timeFontHeight + 2 * (Y_TimeStart - Y_StatusBarStart);
static constexpr uint16_t StatusBarWidth = disp_x_size - X_StatusBarStart;
static constexpr uint16_t X_PressureStart = 178;
static constexpr uint16_t Y_PressureStart = 285;
static constexpr uint16_t X_FlightLevelStart = 186;
static constexpr uint16_t Y_FlightLevelStart = 280;
static constexpr uint16_t Y_Ext_AltStart = Y_Middle - altFontHeight;
static constexpr uint16_t Y_Ext_AltEnd   = Y_Middle + altFontHeight - 1;
static constexpr uint16_t X_AltUnitStart = 225;
static constexpr uint16_t Y_AltUnitStart = 220;
static constexpr uint16_t X_PressUnitStart = 432;
static constexpr uint16_t Y_PressUnitStart = 265;
static constexpr uint16_t X_GndPressStart = 285;
static constexpr uint16_t Y_GndPressStart = 265;
static constexpr uint16_t X_TempStart = 220;
static constexpr uint16_t Y_DateStart = Y_TimeStart + StatusBarHeight;
static constexpr uint16_t X_DateStart = 332;
static constexpr uint8_t RulerWidth = 64;
static constexpr uint16_t X_RulerStart = X_AltStart - 30 - RulerWidth - 2;
static constexpr uint16_t Y_RulerStart = 16 + 2;
static constexpr uint16_t X_LevelerStart = X_RulerStart - LevelerWidth - 2;

static constexpr uint16_t X_VSpeedRulerStart = 62;
static constexpr uint16_t Y_VSpeedRulerStart = 13;
static constexpr uint16_t X_VSpeedUnitStart = (X_LevelerStart - txtMSWidth) >> 1;
static constexpr uint16_t Y_VSpeedUnitStart = disp_y_size - txtMSHeight - 1;
static constexpr uint16_t X_VSpeedStart = 1;
static constexpr uint16_t Y_VSpeedStart = disp_y_size - txtMSHeight - VSpeedFontHeight - 10;
static constexpr uint16_t Y_VSpeedZero = Y_VSpeedRulerStart + (VSpeedRulerHeight >> 1);
static constexpr uint16_t Y_VSpeedUpLimit = Y_VSpeedZero - VSpeedArrowHeight + 1;
static constexpr uint16_t Y_VSpeedDnLimit = Y_VSpeedZero + VSpeedArrowHeight - 1;
static constexpr uint16_t X_VSpeedArrowStart = 4;
static constexpr uint16_t Y_VSpeedArrowStart = Y_VSpeedRulerStart + 4;

// Линейка
static constexpr uint8_t rulerFontHalfHeight = rulerFontHeight / 2;
static constexpr int16_t RulerHeight = disp_y_size - 2 * (16 + 2);

// Уровень
static constexpr uint8_t LevelerHalfHeight = LevelerHeight / 2;

// Цвета элементов
#ifdef inverse
    static constexpr uint8_t C_Alt_H = ~0xFF;
    static constexpr uint8_t C_Alt_L = ~0xFF;
    static constexpr uint8_t C_Alt_Shaded_H = ~0x84;
    static constexpr uint8_t C_Alt_Shaded_L = ~0x10;
    static constexpr uint8_t C_Alt_BG_H = ~0x00;
    static constexpr uint8_t C_Alt_BG_L = ~0x00;
    static constexpr uint8_t C_AltFrame_H = ~0xFF;
    static constexpr uint8_t C_AltFrame_L = ~0xE0;
    static constexpr uint8_t C_RulerSky_H = ~0x00;
    static constexpr uint8_t C_RulerSky_L = ~0x0F;
    static constexpr uint8_t C_RulerGround_H = ~0x78;
    static constexpr uint8_t C_RulerGround_L = ~0xA0;
    static constexpr uint8_t C_Leveler_H = ~0xF8;
    static constexpr uint8_t C_Leveler_L = ~0x1F;
    static constexpr uint8_t C_Leveler_BG_H = ~0x00;
    static constexpr uint8_t C_Leveler_BG_L = ~0x00;
    static constexpr uint8_t C_GndPress_H = ~0x07;
    static constexpr uint8_t C_GndPress_L = ~0xE0;
    static constexpr uint8_t C_GndPress_BG_H = ~0x00;
    static constexpr uint8_t C_GndPress_BG_L = ~0x00;
    static constexpr uint8_t C_Time_H = ~0xDD;
    static constexpr uint8_t C_Time_L = ~0xF0;

    static constexpr uint8_t C_StatusBarFrame_H = ~0x18;
    static constexpr uint8_t C_StatusBarFrame_L = ~0x82;
    static constexpr uint8_t C_StatusBar_BG_H = ~0x08;
    static constexpr uint8_t C_StatusBar_BG_L = ~0x42;
    //static constexpr uint8_t C_StatusBar_BG_H = ~0x18;
    //static constexpr uint8_t C_StatusBar_BG_L = ~0xE4;

    static constexpr uint8_t C_Timer_H = ~0xCB;
    static constexpr uint8_t C_Timer_L = ~0xEF;
    static constexpr uint8_t C_Timer_BG_H = ~0x10;
    static constexpr uint8_t C_Timer_BG_L = ~0x00;
    static constexpr uint8_t C_TimerFrame_H = ~0x20;
    static constexpr uint8_t C_TimerFrame_L = ~0x00;

    static constexpr uint8_t C_VSpeed_H = ~0x7F;
    static constexpr uint8_t C_VSpeed_L = ~0xBE;
    static constexpr uint8_t C_VSpeedWarning_H = ~0xE6;
    static constexpr uint8_t C_VSpeedWarning_L = ~0x18;
    static constexpr uint8_t C_VSpeedDanger_H = ~0xF9;
    static constexpr uint8_t C_VSpeedDanger_L = ~0xA6;
    static constexpr uint8_t C_VSpeed_BG_H = ~0x00;
    static constexpr uint8_t C_VSpeed_BG_L = ~0x00;

    static constexpr uint8_t C_Pressure_H = ~0x00;
    static constexpr uint8_t C_Pressure_L = ~0x80;
    static constexpr uint8_t C_Pressure_BG_H = ~0x00;
    static constexpr uint8_t C_Pressure_BG_L = ~0x00;

    static constexpr uint8_t C_FlightLevel_H = ~0x53;
    static constexpr uint8_t C_FlightLevel_L = ~0x9C;

    static constexpr uint8_t C_WarningText_H = ~0xFF;
    static constexpr uint8_t C_WarningText_L = ~0xFF;
    static constexpr uint8_t C_WarningText_BG_H = ~0x00;
    static constexpr uint8_t C_WarningText_BG_L = ~0x00;
#else
    static constexpr uint8_t C_Alt_H = 0xFF;
    static constexpr uint8_t C_Alt_L = 0xFF;
    static constexpr uint8_t C_Alt_Shaded_H = 0x84;
    static constexpr uint8_t C_Alt_Shaded_L = 0x10;
    static constexpr uint8_t C_Alt_BG_H = 0x00;
    static constexpr uint8_t C_Alt_BG_L = 0x00;
    static constexpr uint8_t C_AltFrame_H = 0xFF;
    static constexpr uint8_t C_AltFrame_L = 0xE0;
    static constexpr uint8_t C_RulerSky_H = 0x00;
    static constexpr uint8_t C_RulerSky_L = 0x0F;
    static constexpr uint8_t C_RulerGround_H = 0x78;
    static constexpr uint8_t C_RulerGround_L = 0xA0;
    static constexpr uint8_t C_Leveler_H = 0xF8;
    static constexpr uint8_t C_Leveler_L = 0x1F;
    static constexpr uint8_t C_Leveler_BG_H = 0x00;
    static constexpr uint8_t C_Leveler_BG_L = 0x00;
    static constexpr uint8_t C_GndPress_H = 0x07;
    static constexpr uint8_t C_GndPress_L = 0xE0;
    static constexpr uint8_t C_GndPress_BG_H = 0x00;
    static constexpr uint8_t C_GndPress_BG_L = 0x00;
    static constexpr uint8_t C_Time_H = 0xEE;
    static constexpr uint8_t C_Time_L = 0xF0;

    static constexpr uint8_t C_StatusBarFrame_H = 0x18;
    static constexpr uint8_t C_StatusBarFrame_L = 0x82;
    static constexpr uint8_t C_StatusBar_BG_H = 0x08;
    static constexpr uint8_t C_StatusBar_BG_L = 0x42;
    //static constexpr uint8_t C_StatusBar_BG_H = 0x18;
    //static constexpr uint8_t C_StatusBar_BG_L = 0xE4;

    static constexpr uint8_t C_Timer_H = 0xFE;
    static constexpr uint8_t C_Timer_L = 0x99;
    static constexpr uint8_t C_Timer_BG_H = 0x10;
    static constexpr uint8_t C_Timer_BG_L = 0x00;
    static constexpr uint8_t C_TimerFrame_H = 0x20;
    static constexpr uint8_t C_TimerFrame_L = 0x00;

    static constexpr uint8_t C_VSpeed_H = 0x7F;
    static constexpr uint8_t C_VSpeed_L = 0xBE;
    static constexpr uint8_t C_VSpeedWarning_H = 0xE6;
    static constexpr uint8_t C_VSpeedWarning_L = 0x18;
    static constexpr uint8_t C_VSpeedDanger_H = 0xF9;
    static constexpr uint8_t C_VSpeedDanger_L = 0xA6;
    static constexpr uint8_t C_VSpeed_BG_H = 0x00;
    static constexpr uint8_t C_VSpeed_BG_L = 0x00;

    static constexpr uint8_t C_Pressure_H = 0x00;
    static constexpr uint8_t C_Pressure_L = 0x80;
    static constexpr uint8_t C_Pressure_BG_H = 0x00;
    static constexpr uint8_t C_Pressure_BG_L = 0x00;

    static constexpr uint8_t C_FlightLevel_H = 0x53;
    static constexpr uint8_t C_FlightLevel_L = 0x9C;

    static constexpr uint8_t C_WarningText_H = 0xFF;
    static constexpr uint8_t C_WarningText_L = 0xFF;
    static constexpr uint8_t C_WarningText_BG_H = 0x00;
    static constexpr uint8_t C_WarningText_BG_L = 0x00;
#endif

const uint8_t TimeFixedSegments[10] = {255,0,0,255,1,1,255,2,2,255};
const uint8_t DateFixedSegments[10] = {5,5,255,4,4,255,3,3,3,3};
const uint8_t Logarithms[] PROGMEM = {
    0,4,7,10,13,15,18,20,22,24,26,28,29,31,33,34,36,37,38,40,41,42,43,45,46,47,48,49,50,51,52,53,54,54,55,56,57,58,59,59,
    60,61,62,62,63,64,64,65,66,66,67,68,68,69,69,70,70,71,72,72,73,73,74,74,75,75,76,76,77,77,78,78,79,79,79,80,80,81,81,
    82,82,82,83,83,84,84,84,85,85,86,86,86,87,87,87,88,88,89,89,89,90,90,90,91,91,91,92,92,92,92,93,93,93,94,94,94,95,95,
    95,96,96,96,96,97,97,97,97,98,98,98,99,99,99,99,100,100,100,100,101,101,101,101,102,102,102,102,103,103,103,103,104,
    104,104,104,104,105,105,105,105,106,106,106,106,106,107,107,107,107,108,108,108,108,108,109,109,109,109,109,110,110,
    110,110,110,111,111,111,111,111,112,112,112,112,112,112,113,113,113,113,113,114,114
};

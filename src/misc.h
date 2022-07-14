#pragma once

#include <Arduino.h>
//#include "DS1302.h"
// #include <RTClib.h>

//enum AltUnits { Meters, Feets };
//enum PressUnits { HPA, mmHg };

// *** [ ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ] ***

#define LED_FL 17
#define LED_REC 18

//static long Altitude = 25000;
//static long lastAlt = -1;
//static int16_t AltSet;
//static int16_t lastAltSet = 32767;
//static int16_t VSpeed = -6;
//static int16_t lastVSpeed = 32767;
//static bool negativeAlt = false;

//static int16_t altToDraw, altDrawed = 32767;
//static int16_t FlightLevel = 0;

//static Encoder Encoder1(2, 3);
//static Button Btn1(4);

//static Encoder Encoder2(14, 15);
//static Button Btn2(13);

//static Button BtnA(12);
//static Button BtnB(19);

//static uint32_t GndPress;
//static uint32_t lastGndPress = 0;
static DS1302 rtc(5, 6, 7);
static unsigned long TimeShowMillis;
static uint8_t lastTimeArray[10] = {255,255,255,255,255,255,255,255,255,255};
static uint8_t lastDateArray[10] = {255,255,255,255,255,255,255,255,255,255};
static uint8_t TimeEditSegment;
static Time EditTime;
static Time CurrentTime;
static Time RecTimer;
static uint8_t LastTimerSec;
static uint8_t LastTimerState;
static uint32_t TimeEditStart;
static PressUnits CurrPressUnit;
static uint8_t WarningVSpeed = 0;

const uint8_t Logarithms[] PROGMEM = {
    0,4,7,10,13,15,18,20,22,24,26,28,29,31,33,34,36,37,38,40,41,42,43,45,46,47,48,49,50,51,52,53,54,54,55,56,57,58,59,59,
    60,61,62,62,63,64,64,65,66,66,67,68,68,69,69,70,70,71,72,72,73,73,74,74,75,75,76,76,77,77,78,78,79,79,79,80,80,81,81,
    82,82,82,83,83,84,84,84,85,85,86,86,86,87,87,87,88,88,89,89,89,90,90,90,91,91,91,92,92,92,92,93,93,93,94,94,94,95,95,
    95,96,96,96,96,97,97,97,97,98,98,98,99,99,99,99,100,100,100,100,101,101,101,101,102,102,102,102,103,103,103,103,104,
    104,104,104,104,105,105,105,105,106,106,106,106,106,107,107,107,107,108,108,108,108,108,109,109,109,109,109,110,110,
    110,110,110,111,111,111,111,111,112,112,112,112,112,112,113,113,113,113,113,114,114
};

void IntToArray(int32_t value, uint8_t *array, bool mainAlt, uint8_t digits);
int8_t getDays(uint8_t month, uint8_t year);

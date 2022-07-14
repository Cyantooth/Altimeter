#pragma once

#include <Arduino.h>

typedef uint8_t uint_8;

typedef struct
{
    const uint8_t CharWidth;
    const uint16_t CharOffset;
} FONT_CHAR_INFO;

typedef struct
{
    const uint8_t CharWidth;	// width, in bits (or pixels), of the character
    const uint16_t CharOffset;	// offset of the character's bitmap, in bytes, into the the FONT_INFO's data array
    const uint16_t CharLength;
} EXT_FONT_CHAR_INFO;

typedef struct
{
    const uint8_t heightPages;	// height, in pages (8 pixels), of the font's characters
    const uint8_t startChar;	// the first character in the font (e.g. in charInfo and data)
    const uint8_t endChar;		// the last character in the font
    const uint8_t spacePixels;	// number of pixels that a space character takes up
    const EXT_FONT_CHAR_INFO* charInfo;		// pointer to array of char information
    const uint8_t* data;		// pointer to generated array of character visual representation
} FONT_INFO;

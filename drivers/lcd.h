// ----------------------------------------------------------------------------
// lcd.h
//
// Copyright (C) 2015 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

#ifndef LCD_H_
#define LCD_H_

#include "iomodconfig.h"
#include "fonts.h"

#if LCDDriverUseILI9325 == 1
    #include "ili9325.h"
    #include "fsmc.h"
#endif
#if LCDDriverUseILI9341 == 1
    #include "ili9341.h"
    #include "fsmc.h"
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// LCD Size.
#define kLCDWidth 240
#define kLCDHeight 320

// LCD colors.
#define kLCDColor_Black 0x0000
#define kLCDColor_Brown 0xBBCA
#define kLCDColor_Blue 0x001F
#define kLCDColor_Blue2 0x051D
#define kLCDColor_Blue3 0x051F
#define kLCDColor_DarkBlue 0x0015
#define kLCDColor_Red 0xF800
#define kLCDColor_Green 0x07E0
#define kLCDColor_Green2 0xB723
#define kLCDColor_Cyan 0x07FF
#define kLCDColor_Cyan2 0x7FFF
#define kLCDColor_Magenta 0xF81F
#define kLCDColor_Magenta2 0xA254
#define kLCDColor_Yellow 0xFFE0
#define kLCDColor_Orange 0xFBE4
#define kLCDColor_Grey 0xF7DE
#define kLCDColor_Grey2 0x7BEF
#define kLCDColor_White 0xFFFF

// LCD Direction
typedef enum
{
    kLCDDirection_Horizontal,
    kLCDDirection_Vertical,
} LCDDirection_t;

typedef enum
{
    kLCDOrientation_Portrait1,
    kLCDOrientation_Landscape1,
    kLCDOrientation_Portrait2,
    kLCDOrientation_Landscape2
} LCDOrientation_t;

typedef enum
{
    kLCDTextWrapping_Word,
    kLCDTextWrapping_Character,
} LCDTextWrapping_t;

// LCD Lines depending on the chosen fonts
#define LCD_LINE_0  LINE(0)
#define LCD_LINE_1  LINE(1)
#define LCD_LINE_2  LINE(2)
#define LCD_LINE_3  LINE(3)
#define LCD_LINE_4  LINE(4)
#define LCD_LINE_5  LINE(5)
#define LCD_LINE_6  LINE(6)
#define LCD_LINE_7  LINE(7)
#define LCD_LINE_8  LINE(8)
#define LCD_LINE_9  LINE(9)
#define LCD_LINE_10 LINE(10)
#define LCD_LINE_11 LINE(11)
#define LCD_LINE_12 LINE(12)
#define LCD_LINE_13 LINE(13)
#define LCD_LINE_14 LINE(14)
#define LCD_LINE_15 LINE(15)
#define LCD_LINE_16 LINE(16)
#define LCD_LINE_17 LINE(17)
#define LCD_LINE_18 LINE(18)
#define LCD_LINE_19 LINE(19)
#define LCD_LINE_20 LINE(20)
#define LCD_LINE_21 LINE(21)
#define LCD_LINE_22 LINE(22)
#define LCD_LINE_23 LINE(23)
#define LCD_LINE_24 LINE(24)
#define LCD_LINE_25 LINE(25)
#define LCD_LINE_26 LINE(26)
#define LCD_LINE_27 LINE(27)
#define LCD_LINE_28 LINE(28)
#define LCD_LINE_29 LINE(29)

#define Line0 LCD_LINE_0
#define Line1 LCD_LINE_1
#define Line2 LCD_LINE_2
#define Line3 LCD_LINE_3
#define Line4 LCD_LINE_4
#define Line5 LCD_LINE_5
#define Line6 LCD_LINE_6
#define Line7 LCD_LINE_7
#define Line8 LCD_LINE_8
#define Line9 LCD_LINE_9

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint16_t driver;
    uint16_t width;
    uint16_t height;
    uint8_t orientation;
    uint16_t foreground;
    uint16_t background;
    uint8_t textWrap;
    uint8_t fontType;
    fonts_t* font;
} lcd_t;

typedef struct
{
    uint16_t startPosX;
    uint16_t startPosY;
    uint16_t cursorX;
    uint16_t cursorY;
} TextBox_t;

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define mLCDGetRGB565(R, G, B) ((((R)& 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))
#define mSwap(a, b) { int16_t t = a; a = b; b = t; }

#if LCDDriverUseILI9325 == 1
#define mLCDDriverSetup() ILI9325Setup()
#define mLCDSetCursor(inPositonX, inPositonY, inWidth, inHeight) ILI9325SetCursor(inPositonX, inPositonY, inWidth, inHeight)
#define mLCDSetPortrait1(inWidth, inHeight) ILI9325SetPortrait1(inWidth, inHeight)
#define mLCDSetLandscape1(inWidth, inHeight) ILI9325SetLandscape1(inWidth, inHeight)
#define mLCDSetPortrait2(inWidth, inHeight) ILI9325SetPortrait2(inWidth, inHeight)
#define mLCDSetLandscape2(inWidth, inHeight) ILI9325SetLandscape2(inWidth, inHeight)
#define mLCDAccessGRAM() (mFSMCWriteRegister(kLCDRegister_ILI9325_RW_GRAM))
#define mLCDWriteData(inData) mFSMCWriteData(inData)
#endif

#if LCDDriverUseILI9341 == 1
#define mLCDDriverSetup() ILI9341Setup()
#define mLCDSetCursor(inPositonX, inPositonY, inWidth, inHeight) ILI9341SetCursor(inPositonX, inPositonY, inWidth, inHeight)
#define mLCDSetPortrait1(inWidth, inHeight) ILI9341SetPortrait1(inWidth, inHeight)
#define mLCDSetLandscape1(inWidth, inHeight) ILI9341SetLandscape1(inWidth, inHeight)
#define mLCDSetPortrait2(inWidth, inHeight) ILI9341SetPortrait2(inWidth, inHeight)
#define mLCDSetLandscape2(inWidth, inHeight) ILI9341SetLandscape2(inWidth, inHeight)
#define mLCDAccessGRAM() (mFSMCWriteRegister(kLCDRegister_ILI9341_MEMORYWRITE))
#define mLCDWriteData(inData) mFSMCWriteData(inData)
#endif

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int LCDSetup(void);
///
uint16_t LCDGetWidth(void);
///
uint16_t LCDGetHeight(void);
///
void LCDPutPixel(int16_t x, int16_t y, uint16_t inColor);
///
void LCDDisplayOn(void);
///
void LCDDisplayOff(void);
///
void LCDSetTextWrapping(uint8_t wrap_mode);
///
void LCDSetColors(uint16_t inForegroundColor, uint16_t inBackgroundColor);
///
void LCDGetColors(uint16_t* outForegroundColor, uint16_t* outBackgroundColor);
///
void LCDSetFont(fonts_t* inFont);
///
fonts_t* LCDGetFont(void);
///
uint16_t LCDGetFontWidth(void);
///
uint16_t LCDGetFontHeight(void);
///
void LCDPutChar(uint16_t x, uint16_t y, char c);
///
void LCDPutText(uint16_t inColumn, uint16_t inLine, const char* inTextPointer);
///
void LCDAppendText(char* text_ptr);
///
void LCDPutTextSize(uint16_t inColumn, uint16_t inLine, const char* inTextPointer, uint32_t inSize);
///
void LCDAppendTextSize(const char* inTextPointer, uint32_t inSize);
///
void LCDSetTextStartPosition(uint16_t inColumn, uint16_t inLine);
///
void LCDDrawMonoImage(const uint32_t* pict);
///
void LCDDrawImage(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset);
///
void LCDDrawFade(uint16_t inColor);
///
void LCDFillScreen(uint16_t inColor);
///
void LCDClearScreen(void);
///
void LCDClearLine(uint16_t line, uint8_t line_with);
///
void LCDDrawLine(uint16_t inPositionX, uint16_t inPositionY, uint16_t inLength, uint8_t inDirection, uint16_t inColor);
///
void LCDDrawSegment(uint16_t inX0, uint16_t inY0, uint16_t inX1, uint16_t inY1, uint16_t inColor);
///
void LCDDrawTriangle(uint16_t inX0, uint16_t inY0, uint16_t inX1, uint16_t inY1, uint16_t inX2, uint16_t inY2, uint16_t inColor);
///
void LCDDrawRectangle(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight, uint16_t inForeground);
///
void LCDDrawFullRectangle(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight, uint16_t inForeground, uint16_t inBackground);
///
void LCDDrawCircle(uint16_t inPositionX, uint16_t inPositionY, uint16_t radius, uint16_t inColor);
///
void LCDDrawFullCircle(uint16_t inPositionX, uint16_t inPositionY, uint16_t radius, uint16_t inForeground, uint16_t inBackground);
///
void LCDDrawFullCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);

#endif // LCD_H_


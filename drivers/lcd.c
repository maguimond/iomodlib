// ----------------------------------------------------------------------------
// gLCD.c
//
// Copyright (C) 2015 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

// Standard includes.
#include <stdlib.h>

// HAL includes.
#include "lcd.h"

// Drivers includes.
#include "gpio.h"

// Lib includes.
#include "lcdlog.h"
#include "fonts.c"
#include "utils.h"

// ----------------------------------------------------------------------------
// Private variables.
static lcd_t gLCD;
static TextBox_t gTextBox;

// ----------------------------------------------------------------------------
void MenuDebugLog(void)
{
    LCDSetColors(kLCDColor_White, kLCDColor_Black);
    LCDClearScreen();
    LCDSetFont(FONT_7X10);
    LCDDataLoggerEnableDisplay(true);
}

// ----------------------------------------------------------------------------
void LCDSetOrientation(uint8_t inOrientation)
{
    gLCD.orientation = inOrientation;

    switch (gLCD.orientation)
    {
        case kLCDOrientation_Portrait1:
        {
            gLCD.width  = kLCDWidth;
            gLCD.height = kLCDHeight;
            mLCDSetPortrait1();
            break;
        }
        case kLCDOrientation_Landscape1:
        {
            gLCD.width  = kLCDHeight;
            gLCD.height = kLCDWidth;
            mLCDSetLandscape1();
            break;
        }
        case kLCDOrientation_Portrait2:
        {
            gLCD.width  = kLCDWidth;
            gLCD.height = kLCDHeight;
            mLCDSetPortrait2();
            break;
        }
        case kLCDOrientation_Landscape2:
        {
            gLCD.width  = kLCDHeight;
            gLCD.height = kLCDWidth;
            mLCDSetLandscape2();
            break;
        }
        default:
        {
            // TODO: Error.
            break;
        }
    }
}

// ----------------------------------------------------------------------------
void LCDDrawBMP(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset)
{
    uint32_t index = 0;
    /*TODO: create fade in function.
    for (index = 0; index < (10 * (inWidth * inHeight)); index ++)
    {
        uint16_t random_x = (Random16() % inWidth);
        uint16_t random_y = (Random16() % inHeight);
        mLCDSetCursor(random_x + inXOffset, random_y + inYOffset);
        mLCDAccessGRAM();
        mLCDWriteData(inImage[(random_y * inWidth) + (random_x - 1)]);
    }*/
    mLCDAccessGRAM();
    for (index = 0; index < (inWidth * inHeight); index ++)
    {
        mLCDWriteData(inImage[index]);
    }
}

// ----------------------------------------------------------------------------
void LCDDrawImage(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset)
{
    for (uint16_t xIndex = 0; xIndex < inWidth; xIndex ++)
    {
        for (uint16_t yIndex = 0; yIndex < inHeight; yIndex ++)
        {
            LCDPutPixel(xIndex + inXOffset, yIndex + inYOffset, inImage[(yIndex * inWidth) + (xIndex - 1)]);
        }
    }
}

// ----------------------------------------------------------------------------
void LCDDrawFade(uint16_t inColor)
{
    uint32_t index = 0;

    for (index = 0; index < (10 * (gLCD.width * gLCD.height)); index ++)
    {
        uint16_t random_x = Random16() % gLCD.width;
        uint16_t random_y = Random16() % gLCD.height;
        LCDPutPixel(random_x, random_y, inColor);
    }
}

// ----------------------------------------------------------------------------
int LCDSetup(void)
{
    int status = mLCDDriverSetup(kLCDWidth, kLCDHeight);

    if (status == 0)
    {
        // Apply background default settings.
        LCDSetColors(kLCDColor_White, kLCDColor_Black);
        LCDSetOrientation(kLCDOrientation_Landscape1);
        LCDClearScreen();
        // Apply test default settings.
        LCDSetTextWrapping(kLCDTextWrapping_Word);
        LCDSetFont(FONT_7X10);
    }
    else
    {
        status = -1;
    }

    return status;
}

// ----------------------------------------------------------------------------
void LCDFillScreen(uint16_t inColor)
{
    mLCDSetCursor(0, 0);
    mLCDAccessGRAM();
    for (uint32_t index = 0; index < kLCDHeight * kLCDWidth; index ++)
    {
        mLCDWriteData(inColor);
    }
}

// ----------------------------------------------------------------------------
void LCDClearScreen(void)
{
    mLCDSetCursor(0, 0);
    mLCDAccessGRAM();
    for (uint32_t index = 0; index < kLCDHeight * kLCDWidth; index ++)
    {
        mLCDWriteData(gLCD.background);
    }
}

// ----------------------------------------------------------------------------
void LCDClearLine(uint16_t line, uint8_t line_with)
{
    uint16_t ref_line = line;
    do
    {
        LCDDrawLine(0, ref_line, gLCD.width, kLCDDirection_Horizontal, gLCD.background);
    } while (++ ref_line < line_with);
}

// ----------------------------------------------------------------------------
void LCDDrawLine(uint16_t positonX, uint16_t positonY, uint16_t length, uint8_t direction, uint16_t inColor)
{
    mLCDSetCursor(positonX, positonY);
    if (direction == kLCDDirection_Horizontal)
    {
        mLCDAccessGRAM();
        for (uint32_t index = 0; index < length; index ++)
        {
            mLCDWriteData(inColor);
        }
    }
    else
    {
        for (uint32_t index = 0; index < length; index ++)
        {
            LCDPutPixel(positonX, positonY, inColor);
            positonY ++;
        }
    }
}

// ----------------------------------------------------------------------------
// TODO: Some functions could be moved to GFX lib.
void LCDDrawRectangle(uint16_t positonX, uint16_t positonY, uint16_t width, uint16_t height, uint16_t inForeground)
{
    LCDDrawLine(positonX, positonY, width, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(positonX, (positonY + height), width, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(positonX, positonY, height, kLCDDirection_Vertical, inForeground);
    LCDDrawLine((positonX + width - 1), positonY, height, kLCDDirection_Vertical, inForeground);
}
// ----------------------------------------------------------------------------
void LCDDrawFullRectangle(uint16_t positonX, uint16_t positonY, uint16_t width, uint16_t height, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawRectangle(positonX, positonY, width, height, inForeground);

    width -= 2;
    height --;
    positonX ++;
    positonY ++;

    while (height --)
    {
        LCDDrawLine(positonX, positonY ++, width, kLCDDirection_Horizontal, inBackground);
    }
}

// ----------------------------------------------------------------------------
void LCDDrawCircle(uint16_t positonX, uint16_t positonY, uint16_t radius, uint16_t inColor)
{
    int32_t decision;
    uint32_t currentX;
    uint32_t currentY;

    decision = 3 - (radius << 1);
    currentX = 0;
    currentY = radius;

    while (currentX <= currentY)
    {
        LCDPutPixel(positonX + currentX, positonY + currentY, inColor);
        LCDPutPixel(positonX + currentX, positonY - currentY, inColor);
        LCDPutPixel(positonX - currentX, positonY + currentY, inColor);
        LCDPutPixel(positonX - currentX, positonY - currentY, inColor);
        LCDPutPixel(positonX + currentY, positonY + currentX, inColor);
        LCDPutPixel(positonX + currentY, positonY - currentX, inColor);
        LCDPutPixel(positonX - currentY, positonY + currentX, inColor);
        LCDPutPixel(positonX - currentY, positonY - currentX, inColor);
        if (decision < 0)
        {
            decision += (currentX << 2) + 6;
        }
        else
        {
            decision += ((currentX - currentY) << 2) + 10;
            currentY --;
        }
        currentX ++;
    }
}

// ----------------------------------------------------------------------------
void LCDDrawFullCircle(uint16_t positonX, uint16_t positonY, uint16_t radius, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawLine(positonX, positonY - radius, 2 * radius + 1, kLCDDirection_Vertical, inBackground);
    LCDDrawFullCircleHelper(positonX, positonY, radius, 3, 0, inBackground);
    LCDDrawCircle(positonX, positonY, radius, inForeground);
}

// ----------------------------------------------------------------------------
// Used to do circles and roundrects
void LCDDrawFullCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y --;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1)
        {
            LCDDrawLine(x0+x, y0 - y, 2 * y + 1 + delta, kLCDDirection_Vertical, color);
            LCDDrawLine(x0+y, y0 - x, 2 * x + 1 + delta, kLCDDirection_Vertical, color);
        }
        if (cornername & 0x2)
        {
            LCDDrawLine(x0 - x, y0 - y, 2 * y + 1 + delta, kLCDDirection_Vertical, color);
            LCDDrawLine(x0 - y, y0 - x, 2 * x + 1 + delta, kLCDDirection_Vertical, color);
        }
    }
}

// ----------------------------------------------------------------------------
uint16_t LCDGetWidth(void)
{
    return gLCD.width;
}

// ----------------------------------------------------------------------------
uint16_t LCDGetHeight(void)
{
    return gLCD.height;
}

// ----------------------------------------------------------------------------
void LCDSetTextWrapping(uint8_t wrap_mode)
{
    gLCD.textWrap = wrap_mode;
}

// ----------------------------------------------------------------------------
void LCDSetColors(uint16_t foreground, uint16_t background)
{
    gLCD.foreground = foreground;
    gLCD.background = background;
}

// ----------------------------------------------------------------------------
void LCDGetColors(uint16_t* outForeground, uint16_t* outBackground)
{
    *outForeground = gLCD.foreground; *outBackground = gLCD.background;
}

// ----------------------------------------------------------------------------
void LCDSetFontColor(uint16_t color)
{
    gLCD.foreground = color;
}

// ----------------------------------------------------------------------------
void LCDSetBackgroundColor(uint16_t color)
{
    gLCD.background = color;
}

// ----------------------------------------------------------------------------
void LCDSetFont(uint8_t inFont)
{
    switch (inFont)
    {
        case FONT_7X10:
            gLCD.font = &font7x10;
            break;
        case FONT_8X8:
                gLCD.font = &font8x8;
                break;
        case FONT_8X12:
                gLCD.font = &font8x12;
                break;
        case FONT_11X18:
                gLCD.font = &font11x18;
                break;
        case FONT_12X12:
                gLCD.font = &font12x12;
                break;
        case FONT_16X24:
                gLCD.font = &font16x24;
                break;
        case FONT_16X26:
                gLCD.font = &font16x26;
                break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
fonts_t* LCDGetFont(void)
{
    return gLCD.font;
}

// ----------------------------------------------------------------------------
void LCDPutChar(uint16_t x, uint16_t y, char c)
{
    // Set coordinates
    gTextBox.cursorX = x;
    gTextBox.cursorY = y;

    if ((gTextBox.cursorY + gLCD.font->height) > gLCD.height)
    {   // If at bottom of display, go to start position
        gTextBox.cursorX = gTextBox.startPosX;
        gTextBox.cursorY = gTextBox.startPosY;
        LCDClearScreen();
    }

    uint8_t hIndex;
    uint16_t char_line = 0;
    mLCDSetCursor(gTextBox.cursorX, gTextBox.cursorY);
    for (hIndex = 0; hIndex < gLCD.font->height; hIndex ++)
    {
        uint16_t char_row_mask = gLCD.font->table[(c - 32) * gLCD.font->height + hIndex];
        uint16_t vIndex;
        mLCDAccessGRAM();
        for (vIndex = 0; vIndex < gLCD.font->width; vIndex ++)
        {
            if ((char_row_mask << vIndex) & 0x8000)
            {
                mLCDWriteData(gLCD.foreground);
            }
            else
            {
                mLCDWriteData(gLCD.background);
            }
        }
        char_line ++;
        mLCDSetCursor(gTextBox.cursorX, gTextBox.cursorY + char_line);
    }
}

// ----------------------------------------------------------------------------
void LCDPutText(uint16_t col_offset, uint16_t line, const char* inTextPtr)
{
    gTextBox.cursorX = col_offset;
    gTextBox.cursorY = line;

    // Send the string character by character on lCD
    while (*inTextPtr != 0)
    {
        if (*inTextPtr == '\r')
        {   // Return
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPtr ++;
            continue;
        }
        else if (*inTextPtr == '\n')
        {   // New line
            gTextBox.cursorY += gLCD.font->height;
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPtr ++;
            continue;
        }

        if (gLCD.textWrap == kLCDTextWrapping_Character)
        {
            if ((gTextBox.cursorX + gLCD.font->width) > gLCD.width)
            {   // Wrap on character screen width overflow
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPtr == ' ')
                {   // Skip overflow on space
                    inTextPtr ++;
                    continue;
                }
            }
        }
        else
        {   // Wrap on word
            uint8_t str_char_nb;
            // Count characters in a word
            for (str_char_nb = 1; (*(inTextPtr + str_char_nb) != ' ') && (*(inTextPtr + str_char_nb) != '\0') &&
                                  (*(inTextPtr + str_char_nb) != '\r') && (*(inTextPtr + str_char_nb) != '\n'); str_char_nb ++);

            if (gTextBox.cursorX + str_char_nb * gLCD.font->width > gLCD.width)
            {   // Wrap on word screen width overflow
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPtr == ' ')
                {   // Skip new line starting with space
                    inTextPtr ++;
                    continue;
                }
            }
        }

        // Display one character on LCD and point to next character
        LCDPutChar(gTextBox.cursorX, gTextBox.cursorY, *inTextPtr ++);
        // Increment horizontal position for next character
        gTextBox.cursorX += gLCD.font->width;
    }
}

// ----------------------------------------------------------------------------
void LCDAppendText(char* inTextPtr)
{
    LCDPutText(gTextBox.cursorX, gTextBox.cursorY, inTextPtr);
}

// ----------------------------------------------------------------------------
void LCDSetTextStartPosition(uint16_t col_offset, uint16_t line)
{
    gTextBox.startPosX = gTextBox.cursorX = col_offset;
    gTextBox.startPosY = gTextBox.cursorY = line;
}

// ----------------------------------------------------------------------------
void LCDDrawMonoImage(const uint32_t *pict)
{
    uint32_t index = 0, i = 0;
    mLCDSetCursor(0, (gLCD.width - 1));
    mLCDAccessGRAM();
    for (index = 0; index < 2400; index++)
    {
        for (i = 0; i < 32; i++)
        {
            if ((pict[index] & (1 << i)) == 0x00)
            {
                mLCDWriteData(gLCD.background);
            }
            else
            {
                mLCDWriteData(gLCD.foreground);
            }
        }
    }
}

// ----------------------------------------------------------------------------
void LCDPutPixel(int16_t x, int16_t y, uint16_t inColor)
{
    mLCDSetCursor(x, y);
    mLCDAccessGRAM();
    mLCDWriteData(inColor);
}

// ----------------------------------------------------------------------------
//TODO: lcd_display_on
void LCDDisplayOn(void)
{

}

// ----------------------------------------------------------------------------
//TODO: lcd_display_off
void LCDDisplayOff(void)
{

}


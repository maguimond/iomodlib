// ----------------------------------------------------------------------------
// lcd.c
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
static void LCDSetOrientation(uint8_t inOrientation)
{
    gLCD.orientation = inOrientation;

    switch (gLCD.orientation)
    {
        case kLCDOrientation_Portrait1:
        {
            gLCD.width = kLCDWidth;
            gLCD.height = kLCDHeight;
            mLCDSetPortrait1(gLCD.width, gLCD.height);
            break;
        }
        case kLCDOrientation_Landscape1:
        {
            gLCD.width = kLCDHeight;
            gLCD.height = kLCDWidth;
            mLCDSetLandscape1(gLCD.width, gLCD.height);
            break;
        }
        case kLCDOrientation_Portrait2:
        {
            gLCD.width = kLCDWidth;
            gLCD.height = kLCDHeight;
            mLCDSetPortrait2(gLCD.width, gLCD.height);
            break;
        }
        case kLCDOrientation_Landscape2:
        {
            gLCD.width = kLCDHeight;
            gLCD.height = kLCDWidth;
            mLCDSetLandscape2(gLCD.width, gLCD.height);
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
int LCDSetup(void)
{
    int status = mLCDDriverSetup();

    if (status == 0)
    {
        // Apply background default settings.
        LCDSetColors(kLCDColor_White, kLCDColor_Black);
        LCDSetOrientation(kLCDOrientation_Landscape1);
        LCDClearScreen();
        // Apply test default settings.
        LCDSetTextWrapping(kLCDTextWrapping_Word);
        LCDSetFont(&font7x10);
    }
    else
    {
        status = -1;
    }

    return status;
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
void LCDPutPixel(int16_t inPositionX, int16_t inPositionY, uint16_t inColor)
{
    mLCDSetCursor(inPositionX, inPositionY, gLCD.width, gLCD.height);
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

// ----------------------------------------------------------------------------
void LCDSetTextWrapping(uint8_t wrap_mode)
{
    gLCD.textWrap = wrap_mode;
}

// ----------------------------------------------------------------------------
void LCDSetColors(uint16_t inForegroundColor, uint16_t inBackgroundColor)
{
    gLCD.foreground = inForegroundColor;
    gLCD.background = inBackgroundColor;
}

// ----------------------------------------------------------------------------
void LCDGetColors(uint16_t* outForegroundColor, uint16_t* outBackgroundColor)
{
    *outForegroundColor = gLCD.foreground;
    *outBackgroundColor = gLCD.background;
}

// ----------------------------------------------------------------------------
void LCDSetFont(fonts_t* inFont)
{
    gLCD.font = inFont;
}

// ----------------------------------------------------------------------------
fonts_t* LCDGetFont(void)
{
    return gLCD.font;
}

// ----------------------------------------------------------------------------
uint16_t LCDGetFontWidth(void)
{
    return gLCD.font->width;
}

// ----------------------------------------------------------------------------
uint16_t LCDGetFontHeight(void)
{
    return gLCD.font->height;
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
    mLCDSetCursor(gTextBox.cursorX, gTextBox.cursorY, gLCD.width, gLCD.height);
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
        mLCDSetCursor(gTextBox.cursorX, gTextBox.cursorY + char_line, gLCD.width, gLCD.height);
    }
}

// ----------------------------------------------------------------------------
void LCDPutText(uint16_t inColumn, uint16_t inLine, const char* inTextPointer)
{
    gTextBox.cursorX = inColumn;
    gTextBox.cursorY = inLine;

    // TODO: Should input inSize.
    // Send the string character by character on lCD.
    while (*inTextPointer != 0)
    {
        if (*inTextPointer == '\r')
        {
            // Return.
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPointer ++;
            continue;
        }
        else if (*inTextPointer == '\n')
        {
            // New line.
            gTextBox.cursorY += gLCD.font->height;
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPointer ++;
            continue;
        }

        if (gLCD.textWrap == kLCDTextWrapping_Character)
        {
            if ((gTextBox.cursorX + gLCD.font->width) > gLCD.width)
            {
                // Wrap on character screen width overflow.
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPointer == ' ')
                {
                    // Skip overflow on space.
                    inTextPointer ++;
                    continue;
                }
            }
        }
        else
        {
            // Wrap on word.
            uint8_t str_char_nb;
            // Count characters in a word.
            for (str_char_nb = 1; (*(inTextPointer + str_char_nb) != ' ') && (*(inTextPointer + str_char_nb) != '\0') &&
                                  (*(inTextPointer + str_char_nb) != '\r') && (*(inTextPointer + str_char_nb) != '\n'); str_char_nb ++);

            if (gTextBox.cursorX + str_char_nb * gLCD.font->width > gLCD.width)
            {
                // Wrap on word screen width overflow.
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPointer == ' ')
                {
                    // Skip new line starting with space.
                    inTextPointer ++;
                    continue;
                }
            }
        }

        // Display one character on LCD and point to next character.
        LCDPutChar(gTextBox.cursorX, gTextBox.cursorY, *inTextPointer ++);
        // Increment horizontal position for next character.
        gTextBox.cursorX += gLCD.font->width;
    }
}

// ----------------------------------------------------------------------------
void LCDAppendText(char* inTextPointer)
{
    LCDPutText(gTextBox.cursorX, gTextBox.cursorY, inTextPointer);
}

// ----------------------------------------------------------------------------
void LCDPutTextSize(uint16_t inColumn, uint16_t inLine, const char* inTextPointer, uint32_t inSize)
{
    gTextBox.cursorX = inColumn;
    gTextBox.cursorY = inLine;

    // Send the string character by character on lCD.
    while ((inSize --) != 0)
    {
        if (*inTextPointer == '\r')
        {
            // Return.
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPointer ++;
            continue;
        }
        else if (*inTextPointer == '\n')
        {
            // New line.
            gTextBox.cursorY += gLCD.font->height;
            gTextBox.cursorX = gTextBox.startPosX;
            inTextPointer ++;
            continue;
        }

        if (gLCD.textWrap == kLCDTextWrapping_Character)
        {
            if ((gTextBox.cursorX + gLCD.font->width) > gLCD.width)
            {
                // Wrap on character screen width overflow.
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPointer == ' ')
                {
                    // Skip overflow on space.
                    inTextPointer ++;
                    continue;
                }
            }
        }
        else
        {
            // Wrap on word.
            uint8_t str_char_nb;
            // Count characters in a word.
            // FIXME: If none of the above, will loop indefinitely.
            for (str_char_nb = 1; (*(inTextPointer + str_char_nb) != ' ') && (*(inTextPointer + str_char_nb) != '\0') &&
                                  (*(inTextPointer + str_char_nb) != '\r') && (*(inTextPointer + str_char_nb) != '\n'); str_char_nb ++);

            if (gTextBox.cursorX + str_char_nb * gLCD.font->width > gLCD.width)
            {
                // Wrap on word screen width overflow.
                gTextBox.cursorX = gTextBox.startPosX;
                gTextBox.cursorY += gLCD.font->height;
                if (*inTextPointer == ' ')
                {
                    // Skip new line starting with space.
                    inTextPointer ++;
                    continue;
                }
            }
        }

        // Display one character on LCD and point to next character.
        LCDPutChar(gTextBox.cursorX, gTextBox.cursorY, *inTextPointer ++);
        // Increment horizontal position for next character.
        gTextBox.cursorX += gLCD.font->width;
    }
}

// ----------------------------------------------------------------------------
void LCDAppendTextSize(char* inTextPointer, uint32_t inSize)
{
    LCDPutTextSize(gTextBox.cursorX, gTextBox.cursorY, inTextPointer, inSize);
}

// ----------------------------------------------------------------------------
void LCDSetTextStartPosition(uint16_t inColumn, uint16_t inLine)
{
    gTextBox.startPosX = gTextBox.cursorX = inColumn;
    gTextBox.startPosY = gTextBox.cursorY = inLine;
}

// ----------------------------------------------------------------------------
void LCDDrawMonoImage(const uint32_t *pict)
{
    uint32_t index = 0, i = 0;
    mLCDSetCursor(0, (gLCD.width - 1), gLCD.width, gLCD.height);
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
void LCDDrawBMP(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset)
{
    uint32_t index = 0;
    /*TODO: create fade in function.
    for (index = 0; index < (10 * (inWidth * inHeight)); index ++)
    {
        uint16_t random_x = (Random16() % inWidth);
        uint16_t random_y = (Random16() % inHeight);
        mLCDSetCursor(random_x + inXOffset, random_y + inYOffset, gLCD.width, gLCD.height);
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
void LCDFillScreen(uint16_t inColor)
{
    mLCDSetCursor(0, 0, gLCD.width, gLCD.height);
    mLCDAccessGRAM();
    for (uint32_t index = 0; index < kLCDHeight * kLCDWidth; index ++)
    {
        mLCDWriteData(inColor);
    }
}

// ----------------------------------------------------------------------------
void LCDClearScreen(void)
{
    mLCDSetCursor(0, 0, gLCD.width, gLCD.height);
    mLCDAccessGRAM();
    for (uint32_t index = 0; index < kLCDHeight * kLCDWidth; index ++)
    {
        mLCDWriteData(gLCD.background);
    }
}

// ----------------------------------------------------------------------------
void LCDClearLine(uint16_t inLine, uint8_t inLineWidth)
{
    uint16_t ref_line = inLine;
    do
    {
        LCDDrawLine(0, ref_line, gLCD.width, kLCDDirection_Horizontal, gLCD.background);
    } while (++ ref_line < inLineWidth);
}

// ----------------------------------------------------------------------------
void LCDDrawLine(uint16_t inPositionX, uint16_t inPositionY, uint16_t inLength, uint8_t inDirection, uint16_t inColor)
{
    mLCDSetCursor(inPositionX, inPositionY, gLCD.width, gLCD.height);
    if (inDirection == kLCDDirection_Horizontal)
    {
        mLCDAccessGRAM();
        for (uint32_t index = 0; index < inLength; index ++)
        {
            mLCDWriteData(inColor);
        }
    }
    else
    {
        for (uint32_t index = 0; index < inLength; index ++)
        {
            LCDPutPixel(inPositionX, inPositionY, inColor);
            inPositionY ++;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------
void LCDDrawSegment(uint16_t inX0, uint16_t inY0, uint16_t inX1, uint16_t inY1, uint16_t inColor)
{
    int16_t steep = abs(inY1 - inY0) > abs(inX1 - inX0);

    if (steep)
    {
        mSwap(inX0, inY0);
        mSwap(inX1, inY1);
    }

    if (inX0 > inX1)
    {
        mSwap(inX0, inX1);
        mSwap(inY0, inY1);
    }

    int16_t dx, dy;
    dx = inX1 - inX0;
    dy = abs(inY1 - inY0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (inY0 < inY1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; inX0 <= inX1; inX0 ++)
    {
        if (steep)
        {
            LCDPutPixel(inY0, inX0, inColor);
        }
        else
        {
            LCDPutPixel(inX0, inY0, inColor);
        }

        err -= dy;
        if (err < 0)
        {
            inY0 += ystep;
            err += dx;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------
void LCDDrawTriangle(uint16_t inX0, uint16_t inY0, uint16_t inX1, uint16_t inY1, uint16_t inX2, uint16_t inY2, uint16_t inColor)
{
    LCDDrawSegment(inX0, inY0, inX1, inY1, inColor);
    LCDDrawSegment(inX1, inY1, inX2, inY2, inColor);
    LCDDrawSegment(inX2, inY2, inX0, inY0, inColor);
}

// ----------------------------------------------------------------------------
// TODO: Some functions could be moved to GFX lib.
void LCDDrawRectangle(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight, uint16_t inForeground)
{
    LCDDrawLine(inPositionX, inPositionY, inWidth, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(inPositionX, (inPositionY + inHeight), inWidth, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(inPositionX, inPositionY, inHeight, kLCDDirection_Vertical, inForeground);
    LCDDrawLine((inPositionX + inWidth - 1), inPositionY, inHeight, kLCDDirection_Vertical, inForeground);
}
// ----------------------------------------------------------------------------
void LCDDrawFullRectangle(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawRectangle(inPositionX, inPositionY, inWidth, inHeight, inForeground);

    inWidth -= 2;
    inHeight --;
    inPositionX ++;
    inPositionY ++;

    while (inHeight --)
    {
        LCDDrawLine(inPositionX, inPositionY ++, inWidth, kLCDDirection_Horizontal, inBackground);
    }
}

// ----------------------------------------------------------------------------
void LCDDrawCircle(uint16_t inPositionX, uint16_t inPositionY, uint16_t radius, uint16_t inColor)
{
    int32_t decision;
    uint32_t currentX;
    uint32_t currentY;

    decision = 3 - (radius << 1);
    currentX = 0;
    currentY = radius;

    while (currentX <= currentY)
    {
        LCDPutPixel(inPositionX + currentX, inPositionY + currentY, inColor);
        LCDPutPixel(inPositionX + currentX, inPositionY - currentY, inColor);
        LCDPutPixel(inPositionX - currentX, inPositionY + currentY, inColor);
        LCDPutPixel(inPositionX - currentX, inPositionY - currentY, inColor);
        LCDPutPixel(inPositionX + currentY, inPositionY + currentX, inColor);
        LCDPutPixel(inPositionX + currentY, inPositionY - currentX, inColor);
        LCDPutPixel(inPositionX - currentY, inPositionY + currentX, inColor);
        LCDPutPixel(inPositionX - currentY, inPositionY - currentX, inColor);
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
void LCDDrawFullCircle(uint16_t inPositionX, uint16_t inPositionY, uint16_t radius, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawLine(inPositionX, inPositionY - radius, 2 * radius + 1, kLCDDirection_Vertical, inBackground);
    LCDDrawFullCircleHelper(inPositionX, inPositionY, radius, 3, 0, inBackground);
    LCDDrawCircle(inPositionX, inPositionY, radius, inForeground);
}

// ----------------------------------------------------------------------------
// Used to do circles and roundrects
void LCDDrawFullCircleHelper(int16_t inX0, int16_t inY0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
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
            LCDDrawLine(inX0 + x, inY0 - y, 2 * y + 1 + delta, kLCDDirection_Vertical, color);
            LCDDrawLine(inX0 + y, inY0 - x, 2 * x + 1 + delta, kLCDDirection_Vertical, color);
        }
        if (cornername & 0x2)
        {
            LCDDrawLine(inX0 - x, inY0 - y, 2 * y + 1 + delta, kLCDDirection_Vertical, color);
            LCDDrawLine(inX0 - y, inY0 - x, 2 * x + 1 + delta, kLCDDirection_Vertical, color);
        }
    }
}

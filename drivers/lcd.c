// ----------------------------------------------------------------------------
// lcd.c
//
// Copyright (C) 2015 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This fille is encoded in UTF-8.
// ---------------------------------------------------------------------------

// Standard includes.
#include <stdlib.h>

// HAL includes.
#include "lcd.h"

// Drivers includes.
#include "fsmc.h"
#include "gpio.h"

// Common includes.
#include "utils.h"

// libgfx includes.
#include "fonts.c"

// ----------------------------------------------------------------------------
#define MAX_POLY_CORNERS 200
#define POLY_Y(z) ((int32_t)((points + z)->x))
#define POLY_X(z) ((int32_t)((points + z)->y))
#define ABS(X)  ((X) > 0 ? (X) : -(X))

#define LCD_TEST_DELAY 100
#define LCD_TEST_DELAY_LONG 100

// Private variables.
static lcd_t lcd;
static text_box_t text_box;

static const uint16_t ILI9325_reg_table[] =
{
    ILI9325_START_OSC        , 0x0001, // Start oscillator
    LCD_DELAY                , 50,     // 50 millisecond delay
    ILI9325_DRIV_OUT_CTRL    , 0x0100,
    ILI9325_DRIV_WAV_CTRL    , 0x0700,
    ILI9325_ENTRY_MOD        , 0x1030,
    ILI9325_RESIZE_CTRL      , 0x0000,
    ILI9325_DISP_CTRL2       , 0x0202,
    ILI9325_DISP_CTRL3       , 0x0000,
    ILI9325_DISP_CTRL4       , 0x0000,
    ILI9325_RGB_DISP_IF_CTRL1, 0x0,
    ILI9325_FRM_MARKER_POS   , 0x0,
    ILI9325_RGB_DISP_IF_CTRL2, 0x0,
    ILI9325_POW_CTRL1        , 0x0000,
    ILI9325_POW_CTRL2        , 0x0007,
    ILI9325_POW_CTRL3        , 0x0000,
    ILI9325_POW_CTRL4        , 0x0000,
    LCD_DELAY                , 200,
    ILI9325_POW_CTRL1        , 0x1690,
    ILI9325_POW_CTRL2        , 0x0227,
    LCD_DELAY                , 50,
    ILI9325_POW_CTRL3        , 0x001A,
    LCD_DELAY                , 50,
    ILI9325_POW_CTRL4        , 0x1800,
    ILI9325_POW_CTRL7        , 0x002A,
    LCD_DELAY                , 50,
    ILI9325_GAMMA_CTRL1      , 0x0000,
    ILI9325_GAMMA_CTRL2      , 0x0000,
    ILI9325_GAMMA_CTRL3      , 0x0000,
    ILI9325_GAMMA_CTRL4      , 0x0206,
    ILI9325_GAMMA_CTRL5      , 0x0808,
    ILI9325_GAMMA_CTRL6      , 0x0007,
    ILI9325_GAMMA_CTRL7      , 0x0201,
    ILI9325_GAMMA_CTRL8      , 0x0000,
    ILI9325_GAMMA_CTRL9      , 0x0000,
    ILI9325_GAMMA_CTRL10     , 0x0000,
    ILI9325_GRAM_HOR_AD      , 0x0000,
    ILI9325_GRAM_VER_AD      , 0x0000,
    ILI9325_HOR_START_AD     , 0x0000,
    ILI9325_HOR_END_AD       , 0x00EF,
    ILI9325_VER_START_AD     , 0X0000,
    ILI9325_VER_END_AD       , 0x013F,
    // Driver Output Control (R60h)
    ILI9325_GATE_SCAN_CTRL1  , 0xA700,
    // Driver Output Control (R61h)
    ILI9325_GATE_SCAN_CTRL2  , 0x0003,
    // Driver Output Control (R62h)
    ILI9325_GATE_SCAN_CTRL3  , 0x0000,
    // Panel Interface Control 1 (R90h)
    ILI9325_PANEL_IF_CTRL1   , 0X0010,
    ILI9325_PANEL_IF_CTRL2   , 0X0000,
    ILI9325_PANEL_IF_CTRL3   , 0X0003,
    ILI9325_PANEL_IF_CTRL4   , 0X1100,
    ILI9325_PANEL_IF_CTRL5   , 0X0000,
    ILI9325_PANEL_IF_CTRL6   , 0X0000,
    // Main screen turn on
    ILI9325_DISP_CTRL1       , 0x0133,
};

// ----------------------------------------------------------------------------
static void LCDSetRegisters(uint16_t driver)
{
    switch (driver)
    {
        case ILI9325:
        case ILI9328:
        {
            uint8_t reg_idx = 0;
            uint16_t addr, val;
            while(reg_idx < sizeof(ILI9325_reg_table) / sizeof(uint16_t))
            {
                addr = ILI9325_reg_table[reg_idx++];
                val = ILI9325_reg_table[reg_idx++];
                if(addr == LCD_DELAY)
                {
                    Delay(val);
                }
                else
                {
                    LCDWriteRegister(addr, val);
                }
            }
            break;
        }
        case SSD2119:
        {
            // Enter sleep mode (if we are not already there).
            LCDWriteRegister(SSD2119_SLEEP_MODE_1_REG, 0x0001);

            // Set initial power parameters.
            LCDWriteRegister(SSD2119_PWR_CTRL_5_REG, 0x00B2);
            LCDWriteRegister(SSD2119_VCOM_OTP_1_REG, 0x0006);

            // Start the oscillator.
            LCDWriteRegister(SSD2119_OSC_START_REG, 0x0001);

            // Set pixel format and basic display orientation (scanning direction).
            LCDWriteRegister(SSD2119_OUTPUT_CTRL_REG, 0x30EF);
            LCDWriteRegister(SSD2119_LCD_DRIVE_AC_CTRL_REG, 0x0600);

            // Exit sleep mode.
            LCDWriteRegister(SSD2119_SLEEP_MODE_1_REG, 0x0000);
            Delay(5);

            // Configure pixel color format and MCU interface parameters.
            LCDWriteRegister(SSD2119_ENTRY_MODE_REG, ENTRY_MODE_DEFAULT);

            // Set analog parameters
            LCDWriteRegister(SSD2119_SLEEP_MODE_2_REG, 0x0999);
            LCDWriteRegister(SSD2119_ANALOG_SET_REG, 0x3800);

            // Enable the display
            LCDWriteRegister(SSD2119_DISPLAY_CTRL_REG, 0x0033);

            // Set VCIX2 voltage to 6.1V.
            LCDWriteRegister(SSD2119_PWR_CTRL_2_REG, 0x0005);

            // Configure gamma correction.
            LCDWriteRegister(SSD2119_GAMMA_CTRL_1_REG, 0x0000);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_2_REG, 0x0303);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_3_REG, 0x0407);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_4_REG, 0x0301);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_5_REG, 0x0301);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_6_REG, 0x0403);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_7_REG, 0x0707);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_8_REG, 0x0400);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_9_REG, 0x0a00);
            LCDWriteRegister(SSD2119_GAMMA_CTRL_10_REG, 0x1000);

            // Configure Vlcd63 and VCOMl
            LCDWriteRegister(SSD2119_PWR_CTRL_3_REG, 0x000A);
            LCDWriteRegister(SSD2119_PWR_CTRL_4_REG, 0x2E00);

            // Set the display size and ensure that the GRAM window is set to allow access to the full display buffer.
            LCDWriteRegister(SSD2119_V_RAM_POS_REG, (kLCDWidth - 1) << 8);
            LCDWriteRegister(SSD2119_H_RAM_START_REG, 0x0000);
            LCDWriteRegister(SSD2119_H_RAM_END_REG, kLCDHeight - 1);

            LCDWriteRegister(SSD2119_X_RAM_ADDR_REG, 0x00);
            LCDWriteRegister(SSD2119_Y_RAM_ADDR_REG, 0x00);
            break;
        }
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
static void LCDSetCursor(uint16_t x_pos, uint16_t y_pos)
{
    uint16_t cursor_x;
    uint16_t cursor_y;

    switch (lcd.driver)
    {
        case ILI9325:
        case ILI9328:
        {
            switch (lcd.orientation)
            {
                case kLCDOrientation_Landscape1:
                    cursor_x = y_pos;
                    cursor_y = kLCDHeight - 1 - x_pos;
                    break;
                case kLCDOrientation_Portrait1:
                    cursor_x = kLCDWidth - 1 - x_pos;
                    cursor_y = kLCDHeight - 1 - y_pos;
                    break;
                case kLCDOrientation_Landscape2:
                    cursor_x = kLCDWidth - 1 - y_pos;
                    cursor_y = x_pos;
                    break;
                default:    // kLCDOrientation_Portrait2
                    cursor_x = x_pos;
                    cursor_y = y_pos;
                    break;
            }

            // Set the X address of the display cursor.
            LCDWriteRegister(ILI9325_GRAM_HOR_AD, cursor_x);
            // Set the Y address of the display cursor.
            LCDWriteRegister(ILI9325_GRAM_VER_AD, cursor_y);
            break;
        }
        case SSD2119:
        {   // Set the X address of the display cursor.
            LCDWriteRegister(SSD2119_X_RAM_ADDR_REG, x_pos);

            // Set the Y address of the display cursor.
            LCDWriteRegister(SSD2119_Y_RAM_ADDR_REG, y_pos);
            break;
        }
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
static void LCDSetAddressWindows(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
    switch (lcd.driver)
    {
        case ILI9325:
        case ILI9328:
        {
            // Set address window
            LCDWriteRegister(ILI9325_HOR_START_AD, x_pos);
            LCDWriteRegister(ILI9325_HOR_END_AD, width);
            LCDWriteRegister(ILI9325_VER_START_AD, y_pos);
            LCDWriteRegister(ILI9325_VER_END_AD, height);
            // Set address counter to top left
            LCDSetCursor(0, 0);
            break;
        }
        case SSD2119:
        {
            uint32_t value = 0;

            LCDWriteRegister(SSD2119_H_RAM_START_REG, x_pos);

            if ((x_pos + width) >= lcd.width)
            {
                LCDWriteRegister(SSD2119_H_RAM_END_REG, lcd.width - 1);
            }
            else
            {
                LCDWriteRegister(SSD2119_H_RAM_END_REG, x_pos + width);
            }

            if ((y_pos + height) >= lcd.height)
            {
                value = (lcd.height - 1) << 8;
            }
            else
            {
                value = (y_pos + height) << 8;
            }
            value |= x_pos;
            LCDWriteRegister(SSD2119_V_RAM_POS_REG, value);
            LCDSetCursor(x_pos, y_pos);
            break;
        }
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
static void LCDSetOrientation(uint8_t orientation)
{
    lcd.orientation = orientation;
    uint16_t entry_orientation = 0;

    switch (lcd.orientation)
    {
        case kLCDOrientation_Landscape1:
        {
            lcd.width  = kLCDHeight;
            lcd.height = kLCDWidth;
            entry_orientation = ILI9325_ENTRY_OR4;
            break;
        }
        case kLCDOrientation_Portrait1:
        {
            lcd.width  = kLCDWidth;
            lcd.height = kLCDHeight;
            entry_orientation = ILI9325_ENTRY_OR3;
            break;
        }
        case kLCDOrientation_Landscape2:
        {
            lcd.width  = kLCDHeight;
            lcd.height = kLCDWidth;
            entry_orientation = ILI9325_ENTRY_OR2;
            break;
        }
        case kLCDOrientation_Portrait2:
        {
            lcd.width  = kLCDWidth;
            lcd.height = kLCDHeight;
            entry_orientation = ILI9325_ENTRY_OR1;
            break;
        }
    }
    entry_orientation |= ILI9325_ENTRY_BGR;
    LCDWriteRegister(ILI9325_ENTRY_MOD, entry_orientation);
    // For ILI932x, init default full-screen address window
    LCDSetAddressWindows(0, 0, kLCDWidth, kLCDHeight);
}

// ----------------------------------------------------------------------------
void LCDDrawBMP(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset)
{
    uint32_t idx = 0;

    for (idx = 0; idx < (10 * (inWidth * inHeight)); idx ++)
    {
        uint16_t random_x = (Random16() % inWidth);
        uint16_t random_y = (Random16() % inHeight);
        LCDSetCursor(random_x + inXOffset, random_y + inYOffset);
        LCDWritePixel(inImage[(random_y * inWidth) + (random_x - 1)]);
    }
}

// ----------------------------------------------------------------------------
void LCDDrawImage(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset)
{
    for (uint16_t xIdx = 0; xIdx < inWidth; xIdx ++)
    {
        for (uint16_t yIdx = 0; yIdx < inHeight; yIdx ++)
        {
            LCDSetCursor(xIdx + inXOffset, yIdx + inYOffset);
            LCDWritePixel(inImage[(yIdx * inWidth) + (xIdx - 1)]);
        }
    }
}

// ----------------------------------------------------------------------------
void LCDDrawFade(uint16_t inColor)
{
    uint32_t idx = 0;

    for (idx = 0; idx < (10 * (lcd.width * lcd.height)); idx ++)
    {
        if (idx % 5000 == 0)
        {
            // Give some time to other tasks.
            vTaskDelay(10);
        }

        uint16_t random_x = Random16() % lcd.width;
        uint16_t random_y = Random16() % lcd.height;
        LCDSetCursor(random_x, random_y);
        LCDWritePixel(inColor);
    }
}

// ----------------------------------------------------------------------------
uint16_t LCDSetup(void)
{
    GPIOSetupLCD();

    FSMCSetup();

    GPIOResetLCD();

    lcd.driver = LCDReadID();

    LCDSetRegisters(lcd.driver);

    // Apply default settings.
    LCDClearScreen();
    LCDSetTextWrapping(kLCDTextWrapping_Word);
    LCDSetOrientation(kLCDOrientation_Landscape1);
    LCDSetColors(WHITE, BLACK);
    LCDSetFont(FONT_7X10);

    return lcd.driver;
}

// ----------------------------------------------------------------------------
uint16_t LCDReadID(void)
{
    uint16_t driver;
    driver = LCDReadRegister(0x0000);
    driver = LCDReadRAMData();
    return driver;
}

// ----------------------------------------------------------------------------
void LCDFillScreen(uint16_t inColor)
{
    uint32_t idx = 0;

    LCDSetCursor(0, 0);
    LCDAccessGRAM();
    for (idx = 0; idx < kLCDHeight * kLCDWidth; idx++)
    {
        LCDWriteRAMData(inColor);
    }
}

// ----------------------------------------------------------------------------
void LCDClearScreen(void)
{
    uint32_t idx = 0;

    LCDSetCursor(0, 0);
    LCDAccessGRAM();
    for (idx = 0; idx < kLCDHeight * kLCDWidth; idx++)
    {
        LCDWriteRAMData(lcd.background);
    }
}

// ----------------------------------------------------------------------------
void LCDClearLine(uint16_t line, uint8_t line_with)
{
    uint16_t ref_line = line;
    do
    {
        LCDDrawLine(0, ref_line, lcd.width, kLCDDirection_Horizontal, lcd.background);
    } while (++ ref_line < line_with);
}

// ----------------------------------------------------------------------------
void LCDDrawLine(uint16_t x_pos, uint16_t y_pos, uint16_t length, uint8_t direction, uint16_t inColor)
{
    uint32_t i = 0;

    LCDSetCursor(x_pos, y_pos);
    if (direction == kLCDDirection_Horizontal)
    {
        LCDAccessGRAM();
        for (i = 0; i < length; i++)
        {
            LCDWriteRAMData(inColor);
        }
    }
    else
    {
        for (i = 0; i < length; i++)
        {
            LCDAccessGRAM();
            LCDWriteRAMData(inColor);
            y_pos++;
            LCDSetCursor(x_pos, y_pos);
        }
    }
}

// ----------------------------------------------------------------------------
void LCDDrawRectangle(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height, uint16_t inForeground)
{
    LCDDrawLine(x_pos, y_pos, width, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(x_pos, (y_pos + height), width, kLCDDirection_Horizontal, inForeground);
    LCDDrawLine(x_pos, y_pos, height, kLCDDirection_Vertical, inForeground);
    LCDDrawLine((x_pos + width - 1), y_pos, height, kLCDDirection_Vertical, inForeground);
}
// ----------------------------------------------------------------------------
void LCDDrawFullRectangle(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawRectangle(x_pos, y_pos, width, height, inForeground);

    width -= 2;
    height --;
    x_pos ++;
    y_pos ++;

    while (height --)
    {
        LCDDrawLine(x_pos, y_pos ++, width, kLCDDirection_Horizontal, inBackground);
    }
}

// ----------------------------------------------------------------------------
void LCDDrawCircle(uint16_t x_pos, uint16_t y_pos, uint16_t radius, uint16_t inColor)
{
    int32_t  D;// Decision Variable
    uint32_t  current_x;// Current X Value
    uint32_t  current_y;// Current Y Value

    D = 3 - (radius << 1);
    current_x = 0;
    current_y = radius;

    while (current_x <= current_y)
    {
        LCDSetCursor(x_pos + current_x, y_pos + current_y);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos + current_x, y_pos - current_y);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos - current_x, y_pos + current_y);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos - current_x, y_pos - current_y);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos + current_y, y_pos + current_x);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos + current_y, y_pos - current_x);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos - current_y, y_pos + current_x);
        LCDWritePixel(inColor);
        LCDSetCursor(x_pos - current_y, y_pos - current_x);
        LCDWritePixel(inColor);
        if (D < 0)
        {
            D += (current_x << 2) + 6;
        }
        else
        {
            D += ((current_x - current_y) << 2) + 10;
            current_y--;
        }
        current_x++;
    }
}

// ----------------------------------------------------------------------------
void LCDDrawFullCircle(uint16_t x_pos, uint16_t y_pos, uint16_t radius, uint16_t inForeground, uint16_t inBackground)
{
    LCDDrawLine(x_pos, y_pos - radius, 2 * radius + 1, kLCDDirection_Vertical, inBackground);
    LCDDrawFullCircleHelper(x_pos, y_pos, radius, 3, 0, inBackground);
    LCDDrawCircle(x_pos, y_pos, radius, inForeground);
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
    return lcd.width;
}

// ----------------------------------------------------------------------------
uint16_t LCDGetHeight(void)
{
    return lcd.height;
}

// ----------------------------------------------------------------------------
void LCDSetTextWrapping(uint8_t wrap_mode)
{
    lcd.textWrap = wrap_mode;
}

// ----------------------------------------------------------------------------
void LCDSetColors(uint16_t foreground, uint16_t background)
{
    lcd.foreground = foreground;
    lcd.background = background;
}

// ----------------------------------------------------------------------------
void LCDGetColors(uint16_t* outForeground, uint16_t* outBackground)
{
    *outForeground = lcd.foreground; *outBackground = lcd.background;
}

// ----------------------------------------------------------------------------
void LCDSetFontColor(uint16_t color)
{
    lcd.foreground = color;
}

// ----------------------------------------------------------------------------
void LCDSetBackgroundColor(uint16_t color)
{
    lcd.background = color;
}

// ----------------------------------------------------------------------------
void LCDSetFont(uint8_t inFont)
{
    switch (inFont)
    {
        case FONT_7X10:
            lcd.font = &font7x10;
            break;
        case FONT_8X8:
                lcd.font = &font8x8;
                break;
        case FONT_8X12:
                lcd.font = &font8x12;
                break;
        case FONT_11X18:
                lcd.font = &font11x18;
                break;
        case FONT_12X12:
                lcd.font = &font12x12;
                break;
        case FONT_16X24:
                lcd.font = &font16x24;
                break;
        case FONT_16X26:
                lcd.font = &font16x26;
                break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------------
fonts_t* LCDGetFont(void)
{
    return lcd.font;
}

// ----------------------------------------------------------------------------
void LCDPutChar(uint16_t x, uint16_t y, char c)
{
    // Set coordinates
    text_box.cursor_x = x;
    text_box.cursor_y = y;

    if ((text_box.cursor_y + lcd.font->height) > lcd.height)
    {   // If at bottom of display, go to start position
        text_box.cursor_x = text_box.start_pos_x;
        text_box.cursor_y = text_box.start_pos_y;
        LCDClearScreen();
    }

    uint8_t h_idx;
    uint16_t char_line = 0;
    LCDSetCursor(text_box.cursor_x, text_box.cursor_y);
    for (h_idx = 0; h_idx < lcd.font->height; h_idx ++)
    {
        uint16_t char_row_mask = lcd.font->table[(c - 32) * lcd.font->height + h_idx];
        uint16_t v_idx;
        LCDAccessGRAM();
        for (v_idx = 0; v_idx < lcd.font->width; v_idx ++)
        {
            if ((char_row_mask << v_idx) & 0x8000)
            {
                LCDWriteRAMData(lcd.foreground);
            }
            else
            {
                LCDWriteRAMData(lcd.background);
            }
        }
        char_line ++;
        LCDSetCursor(text_box.cursor_x, text_box.cursor_y + char_line);
    }
}

// ----------------------------------------------------------------------------
void LCDPutText(uint16_t col_offset, uint16_t line, const char* text_ptr)
{
    text_box.cursor_x = col_offset;
    text_box.cursor_y = line;

    // Send the string character by character on lCD
    while (*text_ptr != 0)
    {
        if (*text_ptr == '\r')
        {   // Return
            text_box.cursor_x = text_box.start_pos_x;
            text_ptr ++;
            continue;
        }
        else if (*text_ptr == '\n')
        {   // New line
            text_box.cursor_y += lcd.font->height;
            text_box.cursor_x = text_box.start_pos_x;
            text_ptr ++;
            continue;
        }

        if (lcd.textWrap == kLCDTextWrapping_Character)
        {
            if ((text_box.cursor_x + lcd.font->width) > lcd.width)
            {   // Wrap on character screen width overflow
                text_box.cursor_x = text_box.start_pos_x;
                text_box.cursor_y += lcd.font->height;
                if (*text_ptr == ' ')
                {   // Skip overflow on space
                    text_ptr ++;
                    continue;
                }
            }
        }
        else
        {   // Wrap on word
            uint8_t str_char_nb;
            // Count characters in a word
            for (str_char_nb = 1; (*(text_ptr + str_char_nb) != ' ') && (*(text_ptr + str_char_nb) != '\0') &&
                                  (*(text_ptr + str_char_nb) != '\r') && (*(text_ptr + str_char_nb) != '\n'); str_char_nb ++);

            if (text_box.cursor_x + str_char_nb * lcd.font->width > lcd.width)
            {   // Wrap on word screen width overflow
                text_box.cursor_x = text_box.start_pos_x;
                text_box.cursor_y += lcd.font->height;
                if (*text_ptr == ' ')
                {   // Skip new line starting with space
                    text_ptr ++;
                    continue;
                }
            }
        }

        // Display one character on LCD and point to next character
        LCDPutChar(text_box.cursor_x, text_box.cursor_y, *text_ptr++);
        // Increment horizontal position for next character
        text_box.cursor_x += lcd.font->width;
    }
}

// ----------------------------------------------------------------------------
void LCDAppendText(char* text_ptr)
{
    LCDPutText(text_box.cursor_x, text_box.cursor_y, text_ptr);
}

// ----------------------------------------------------------------------------
void LCDSetTextStartPosition(uint16_t col_offset, uint16_t line)
{
    text_box.start_pos_x = text_box.cursor_x = col_offset;
    text_box.start_pos_y = text_box.cursor_y = line;
}

// ----------------------------------------------------------------------------
void LCDDrawMonoImage(const uint32_t *pict)
{
    uint32_t index = 0, i = 0;
    LCDSetCursor(0, (lcd.width - 1));
    LCDAccessGRAM();
    for (index = 0; index < 2400; index++)
    {
        for (i = 0; i < 32; i++)
        {
            if ((pict[index] & (1 << i)) == 0x00)
            {
                LCDWriteRAMData(lcd.background);
            }
            else
            {
                LCDWriteRAMData(lcd.foreground);
            }
        }
    }
}

// ----------------------------------------------------------------------------
void lcd_write_bmp(uint32_t bmp_addr)
{
#if 0
    uint32_t index = 0, size = 0;
    // Read bitmap size
    size = *(__IO uint16_t *) (bmp_addr + 2);
    size |= (*(__IO uint16_t *) (bmp_addr + 4)) << 16;
    // Get bitmap data address offset
    index = *(__IO uint16_t *) (bmp_addr + 10);
    index |= (*(__IO uint16_t *) (bmp_addr + 12)) << 16;
    size = (size - index)/2;
    bmp_addr += index;
    // Set GRAM write direction and BGR = 1
    // I/D=00 (Horizontal : decrement, Vertical : decrement)
    // AM=1 (address is updated in vertical writing direction)
    LCDWriteRegister(LCD_REG_3, 0x1008);

    LCDWriteRAMAddress();

    for (index = 0; index < size; index++)
    {
        LCDWriteRAMData(*(__IO uint16_t *)bmp_addr);
        bmp_addr += 2;
    }

    // Set GRAM write direction and BGR = 1
    // I/D = 01 (Horizontal : increment, Vertical : decrement)
    // AM = 1 (address is updated in vertical writing direction)
    LCDWriteRegister(LCD_REG_3, 0x1018);
#endif
}

// ----------------------------------------------------------------------------
void lcd_put_pixel(int16_t x, int16_t y, uint16_t inColor)
{
    LCDSetCursor(x, y);
    LCDWritePixel(inColor);
}

// ----------------------------------------------------------------------------
//TODO: lcd_display_on
void lcd_display_on(void)
{

}

// ----------------------------------------------------------------------------
//TODO: lcd_display_off
void lcd_display_off(void)
{

}


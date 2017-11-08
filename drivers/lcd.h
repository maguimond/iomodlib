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

#include "fonts.h"

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// LCD Size
#define kLCDWidth  240
#define kLCDHeight 320

// LCD colors
#define BLACK       0x0000
#define BROWN       0xBBCA
#define BLUE        0x001F
#define BLUE2       0x051D
#define BLUE3       0x051F
#define DARK_BLUE   0x0015
#define RED         0xF800
#define GREEN       0x07E0
#define GREEN2      0xB723
#define CYAN        0x07FF
#define CYAN2       0x7FFF
#define MAGENTA     0xF81F
#define MAGENTA2    0xA254
#define YELLOW      0xFFE0
#define ORANGE      0xFBE4
#define GREY        0xF7DE
#define GREY2       0x7BEF
#define WHITE       0xFFFF

#define LCD_DELAY   0xFF

// ILI9325 registers
#define ILI9325_START_OSC          0x00
#define ILI9325_DRIV_OUT_CTRL      0x01
#define ILI9325_DRIV_WAV_CTRL      0x02
#define ILI9325_ENTRY_MOD          0x03
#define ILI9325_RESIZE_CTRL        0x04
#define ILI9325_DISP_CTRL1         0x07
#define ILI9325_DISP_CTRL2         0x08
#define ILI9325_DISP_CTRL3         0x09
#define ILI9325_DISP_CTRL4         0x0A
#define ILI9325_RGB_DISP_IF_CTRL1  0x0C
#define ILI9325_FRM_MARKER_POS     0x0D
#define ILI9325_RGB_DISP_IF_CTRL2  0x0F
#define ILI9325_POW_CTRL1          0x10
#define ILI9325_POW_CTRL2          0x11
#define ILI9325_POW_CTRL3          0x12
#define ILI9325_POW_CTRL4          0x13
#define ILI9325_GRAM_HOR_AD        0x20
#define ILI9325_GRAM_VER_AD        0x21
#define ILI9325_RW_GRAM            0x22
#define ILI9325_POW_CTRL7          0x29
#define ILI9325_FRM_RATE_COL_CTRL  0x2B
#define ILI9325_GAMMA_CTRL1        0x30
#define ILI9325_GAMMA_CTRL2        0x31
#define ILI9325_GAMMA_CTRL3        0x32
#define ILI9325_GAMMA_CTRL4        0x35
#define ILI9325_GAMMA_CTRL5        0x36
#define ILI9325_GAMMA_CTRL6        0x37
#define ILI9325_GAMMA_CTRL7        0x38
#define ILI9325_GAMMA_CTRL8        0x39
#define ILI9325_GAMMA_CTRL9        0x3C
#define ILI9325_GAMMA_CTRL10       0x3D
#define ILI9325_HOR_START_AD       0x50
#define ILI9325_HOR_END_AD         0x51
#define ILI9325_VER_START_AD       0x52
#define ILI9325_VER_END_AD         0x53
#define ILI9325_GATE_SCAN_CTRL1    0x60
#define ILI9325_GATE_SCAN_CTRL2    0x61
#define ILI9325_GATE_SCAN_CTRL3    0x6A
#define ILI9325_PART_IMG1_DISP_POS 0x80
#define ILI9325_PART_IMG1_START_AD 0x81
#define ILI9325_PART_IMG1_END_AD   0x82
#define ILI9325_PART_IMG2_DISP_POS 0x83
#define ILI9325_PART_IMG2_START_AD 0x84
#define ILI9325_PART_IMG2_END_AD   0x85
#define ILI9325_PANEL_IF_CTRL1     0x90
#define ILI9325_PANEL_IF_CTRL2     0x92
#define ILI9325_PANEL_IF_CTRL3     0x93
#define ILI9325_PANEL_IF_CTRL4     0x95
#define ILI9325_PANEL_IF_CTRL5     0x97
#define ILI9325_PANEL_IF_CTRL6     0x98
#define ILI9325_ENTRY_OR1         0x0030
#define ILI9325_ENTRY_OR2         0x0028
#define ILI9325_ENTRY_OR3         0x0000
#define ILI9325_ENTRY_OR4         0x0018
#define ILI9325_ENTRY_RGB         0x0000
#define ILI9325_ENTRY_BGR         0x1000

// ILI9341 registers
#define ILI9341_SOFTRESET          0x01
#define ILI9341_SLEEPIN            0x10
#define ILI9341_SLEEPOUT           0x11
#define ILI9341_NORMALDISP         0x13
#define ILI9341_INVERTOFF          0x20
#define ILI9341_INVERTON           0x21
#define ILI9341_GAMMASET           0x26
#define ILI9341_DISPLAYOFF         0x28
#define ILI9341_DISPLAYON          0x29
#define ILI9341_COLADDRSET         0x2A
#define ILI9341_PAGEADDRSET        0x2B
#define ILI9341_MEMORYWRITE        0x2C
#define ILI9341_PIXELFORMAT        0x3A
#define ILI9341_FRAMECONTROL       0xB1
#define ILI9341_DISPLAYFUNC        0xB6
#define ILI9341_ENTRYMODE          0xB7
#define ILI9341_POWERCONTROL1      0xC0
#define ILI9341_POWERCONTROL2      0xC1
#define ILI9341_VCOMCONTROL1       0xC5
#define ILI9341_VCOMCONTROL2       0xC7
#define ILI9341_MEMCONTROL         0x36
#define ILI9341_MADCTL             0x36
#define ILI9341_MADCTL_MY          0x80
#define ILI9341_MADCTL_MX          0x40
#define ILI9341_MADCTL_MV          0x20
#define ILI9341_MADCTL_ML          0x10
#define ILI9341_MADCTL_RGB         0x00
#define ILI9341_MADCTL_BGR         0x08
#define ILI9341_MADCTL_MH          0x04

// HX8347G registers
#define HX8347G_COLADDRSTART_HI    0x02
#define HX8347G_COLADDRSTART_LO    0x03
#define HX8347G_COLADDREND_HI      0x04
#define HX8347G_COLADDREND_LO      0x05
#define HX8347G_ROWADDRSTART_HI    0x06
#define HX8347G_ROWADDRSTART_LO    0x07
#define HX8347G_ROWADDREND_HI      0x08
#define HX8347G_ROWADDREND_LO      0x09
#define HX8347G_MEMACCESS          0x16

// SD2119 registers
#define SSD2119_DEVICE_CODE_READ_REG    0x00
#define SSD2119_OSC_START_REG           0x00
#define SSD2119_OUTPUT_CTRL_REG         0x01
#define SSD2119_LCD_DRIVE_AC_CTRL_REG   0x02
#define SSD2119_PWR_CTRL_1_REG          0x03
#define SSD2119_DISPLAY_CTRL_REG        0x07
#define SSD2119_FRAME_CYCLE_CTRL_REG    0x0B
#define SSD2119_PWR_CTRL_2_REG          0x0C
#define SSD2119_PWR_CTRL_3_REG          0x0D
#define SSD2119_PWR_CTRL_4_REG          0x0E
#define SSD2119_GATE_SCAN_START_REG     0x0F
#define SSD2119_SLEEP_MODE_1_REG        0x10
#define SSD2119_ENTRY_MODE_REG          0x11
#define SSD2119_SLEEP_MODE_2_REG        0x12
#define SSD2119_GEN_IF_CTRL_REG         0x15
#define SSD2119_PWR_CTRL_5_REG          0x1E
#define SSD2119_RAM_DATA_REG            0x22
#define SSD2119_FRAME_FREQ_REG          0x25
#define SSD2119_ANALOG_SET_REG          0x26
#define SSD2119_VCOM_OTP_1_REG          0x28
#define SSD2119_VCOM_OTP_2_REG          0x29
#define SSD2119_GAMMA_CTRL_1_REG        0x30
#define SSD2119_GAMMA_CTRL_2_REG        0x31
#define SSD2119_GAMMA_CTRL_3_REG        0x32
#define SSD2119_GAMMA_CTRL_4_REG        0x33
#define SSD2119_GAMMA_CTRL_5_REG        0x34
#define SSD2119_GAMMA_CTRL_6_REG        0x35
#define SSD2119_GAMMA_CTRL_7_REG        0x36
#define SSD2119_GAMMA_CTRL_8_REG        0x37
#define SSD2119_GAMMA_CTRL_9_REG        0x3A
#define SSD2119_GAMMA_CTRL_10_REG       0x3B
#define SSD2119_V_RAM_POS_REG           0x44
#define SSD2119_H_RAM_START_REG         0x45
#define SSD2119_H_RAM_END_REG           0x46
#define SSD2119_X_RAM_ADDR_REG          0x4E
#define SSD2119_Y_RAM_ADDR_REG          0x4F

#define ENTRY_MODE_DEFAULT  0x6830
#define ENTRY_MODE_BMP      0x6810
#define MAKE_ENTRY_MODE(x)  ((ENTRY_MODE_DEFAULT & 0xFF00) | (x))

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

#define ILI9325 0x9325
#define ILI9328 0x9328
#define ILI9341 0x9341
#define SSD2119 0x2119

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    int16_t x;
    int16_t y;
} Point, * pPoint;

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
    uint16_t start_pos_x;
    uint16_t start_pos_y;
    uint16_t cursor_x;
    uint16_t cursor_y;
} text_box_t;

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define LCDGetRGB565(R, G, B) ((((R)& 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
void LCDDrawBMP(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset);
///
void LCDDrawImage(const uint16_t* inImage, uint16_t inWidth, uint16_t inHeight, uint16_t inXOffset, uint16_t inYOffset);
///
void LCDDrawFade(uint16_t inColor);
///
uint16_t LCDSetup(void);
///
uint16_t LCDReadID(void);
///
void LCDFillScreen(uint16_t Color);
///
void LCDClearScreen(void);
///
void LCDClearLine(uint16_t line, uint8_t line_with);
///
void LCDDrawLine(uint16_t x_pos, uint16_t y_pos, uint16_t length, uint8_t direction, uint16_t inColor);
///
void LCDDrawRectangle(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height, uint16_t inForeground);
///
void LCDDrawFullRectangle(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height, uint16_t inForeground, uint16_t inBackground);
///
void LCDDrawCircle(uint16_t x_pos, uint16_t y_pos, uint16_t radius, uint16_t inColor);
///
void LCDDrawFullCircle(uint16_t x_pos, uint16_t y_pos, uint16_t radius, uint16_t inForeground, uint16_t inBackground);
///
void LCDDrawFullCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
///
uint16_t LCDGetWidth(void);
///
uint16_t LCDGetHeight(void);
///
void LCDSetTextWrapping(uint8_t wrap_mode);
///
void LCDSetColors(uint16_t foreground, uint16_t background);
///
void LCDGetColors(uint16_t* outForeground, uint16_t* outBackground);
///
void LCDSetFontColor(uint16_t color);
///
void LCDSetBackgroundColor(uint16_t Color);
///
void LCDSetFont(uint8_t font);
///
fonts_t* LCDGetFont(void);
///
void LCDPutChar(uint16_t x, uint16_t y, char c);
///
void LCDPutText(uint16_t col_offset, uint16_t line, const char* text_ptr);
///
void LCDAppendText(char* text_ptr);
///
void LCDSetTextStartPosition(uint16_t col_offset, uint16_t line);
///
void LCDDRawMonoImage(const uint32_t* pict);
void lcd_write_bmp(uint32_t bmp_addr);
void lcd_put_pixel(int16_t x, int16_t y, uint16_t inColor);
void lcd_display_on(void);
void lcd_display_off(void);

#endif // LCD_H_


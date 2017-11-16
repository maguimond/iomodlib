// ----------------------------------------------------------------------------
// ili9325.h
//
// Copyright (C) 2015 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

#ifndef ILI9325_H_
#define ILI9325_H_

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// LCD drivers.
#define kLCDDriver_ILI9325 0x9325
#define kLCDDriver_ILI9328 0x9328

#define kLCDDelay 0xFF

#define kLCDRegister_ILI9325_ReadID 0x00

// ILI9325 registers.
typedef enum
{
    kLCDRegister_ILI9325_START_OSC = 0x00,
    kLCDRegister_ILI9325_DRIV_OUT_CTRL = 0x01,
    kLCDRegister_ILI9325_DRIV_WAV_CTRL = 0x02,
    kLCDRegister_ILI9325_ENTRY_MOD = 0x03,
    kLCDRegister_ILI9325_RESIZE_CTRL = 0x04,
    kLCDRegister_ILI9325_DISP_CTRL1 = 0x07,
    kLCDRegister_ILI9325_DISP_CTRL2 = 0x08,
    kLCDRegister_ILI9325_DISP_CTRL3 = 0x09,
    kLCDRegister_ILI9325_DISP_CTRL4 = 0x0A,
    kLCDRegister_ILI9325_RGB_DISP_IF_CTRL1 = 0x0C,
    kLCDRegister_ILI9325_FRM_MARKER_POS = 0x0D,
    kLCDRegister_ILI9325_RGB_DISP_IF_CTRL2 = 0x0F,
    kLCDRegister_ILI9325_POW_CTRL1 = 0x10,
    kLCDRegister_ILI9325_POW_CTRL2 = 0x11,
    kLCDRegister_ILI9325_POW_CTRL3 = 0x12,
    kLCDRegister_ILI9325_POW_CTRL4 = 0x13,
    kLCDRegister_ILI9325_GRAM_HOR_AD = 0x20,
    kLCDRegister_ILI9325_GRAM_VER_AD = 0x21,
    kLCDRegister_ILI9325_RW_GRAM = 0x22,
    kLCDRegister_ILI9325_POW_CTRL7 = 0x29,
    kLCDRegister_ILI9325_FRM_RATE_COL_CTRL = 0x2B,
    kLCDRegister_ILI9325_GAMMA_CTRL1 = 0x30,
    kLCDRegister_ILI9325_GAMMA_CTRL2 = 0x31,
    kLCDRegister_ILI9325_GAMMA_CTRL3 = 0x32,
    kLCDRegister_ILI9325_GAMMA_CTRL4 = 0x35,
    kLCDRegister_ILI9325_GAMMA_CTRL5 = 0x36,
    kLCDRegister_ILI9325_GAMMA_CTRL6 = 0x37,
    kLCDRegister_ILI9325_GAMMA_CTRL7 = 0x38,
    kLCDRegister_ILI9325_GAMMA_CTRL8 = 0x39,
    kLCDRegister_ILI9325_GAMMA_CTRL9 = 0x3C,
    kLCDRegister_ILI9325_GAMMA_CTRL10 = 0x3D,
    kLCDRegister_ILI9325_HOR_START_AD = 0x50,
    kLCDRegister_ILI9325_HOR_END_AD = 0x51,
    kLCDRegister_ILI9325_VER_START_AD = 0x52,
    kLCDRegister_ILI9325_VER_END_AD = 0x53,
    kLCDRegister_ILI9325_GATE_SCAN_CTRL1 = 0x60,
    kLCDRegister_ILI9325_GATE_SCAN_CTRL2 = 0x61,
    kLCDRegister_ILI9325_GATE_SCAN_CTRL3 = 0x6A,
    kLCDRegister_ILI9325_PART_IMG1_DISP_POS = 0x80,
    kLCDRegister_ILI9325_PART_IMG1_START_AD = 0x81,
    kLCDRegister_ILI9325_PART_IMG1_END_AD = 0x82,
    kLCDRegister_ILI9325_PART_IMG2_DISP_POS = 0x83,
    kLCDRegister_ILI9325_PART_IMG2_START_AD = 0x84,
    kLCDRegister_ILI9325_PART_IMG2_END_AD = 0x85,
    kLCDRegister_ILI9325_PANEL_IF_CTRL1 = 0x90,
    kLCDRegister_ILI9325_PANEL_IF_CTRL2 = 0x92,
    kLCDRegister_ILI9325_PANEL_IF_CTRL3 = 0x93,
    kLCDRegister_ILI9325_PANEL_IF_CTRL4 = 0x95,
    kLCDRegister_ILI9325_PANEL_IF_CTRL5 = 0x97,
    kLCDRegister_ILI9325_PANEL_IF_CTRL6 = 0x98,
} kLCDRegister_ILI9325_t;

#define kLCDRegister_ILI9325_ENTRY_OR1 0x0030
#define kLCDRegister_ILI9325_ENTRY_OR2 0x0028
#define kLCDRegister_ILI9325_ENTRY_OR3 0x0000
#define kLCDRegister_ILI9325_ENTRY_OR4 0x0018
#define kLCDRegister_ILI9325_ENTRY_RGB 0x0000
#define kLCDRegister_ILI9325_ENTRY_BGR 0x1000

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint16_t width;
    uint16_t height;
    uint8_t orientation;
} ILI9325_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int ILI9325Setup(uint16_t inWidth, uint16_t inHeight);
///
void ILI9325SetCursor(uint16_t inPositionX, uint16_t inPositionY);
///
void ILI9325SetOrientation(uint8_t inOrientation);
///
void ILI9325SetPortrait1(void);
///
void ILI9325SetLandscape1(void);
///
void ILI9325SetPortrait2(void);
///
void ILI9325SetLandscape2(void);

#endif // ILI9325_H_


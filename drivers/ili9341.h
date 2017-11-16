// ----------------------------------------------------------------------------
// ili9341.h
//
// Copyright (C) 2015 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

#ifndef ILI9341_H_
#define ILI9341_H_

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// LCD driver.
#define kLCDDriver_ILI9341 0x9341

// ILI9341 registers.
typedef enum
{
    kLCDRegister_ILI9341_SOFTRESET = 0x01,
    kLCDRegister_ILI9341_SLEEPIN = 0x10,
    kLCDRegister_ILI9341_SLEEPOUT = 0x11,
    kLCDRegister_ILI9341_NORMALDISP = 0x13,
    kLCDRegister_ILI9341_INVERTOFF = 0x20,
    kLCDRegister_ILI9341_INVERTON = 0x21,
    kLCDRegister_ILI9341_GAMMASET = 0x26,
    kLCDRegister_ILI9341_DISPLAYOFF = 0x28,
    kLCDRegister_ILI9341_DISPLAYON = 0x29,
    kLCDRegister_ILI9341_COLADDRSET = 0x2A,
    kLCDRegister_ILI9341_PAGEADDRSET = 0x2B,
    kLCDRegister_ILI9341_MEMORYWRITE = 0x2C,
    kLCDRegister_ILI9341_MEMCONTROL = 0x36,
    kLCDRegister_ILI9341_PIXELFORMAT = 0x3A,
    kLCDRegister_ILI9341_FRAMECONTROL = 0xB1,
    kLCDRegister_ILI9341_DISPLAYFUNC = 0xB6,
    kLCDRegister_ILI9341_ENTRYMODE = 0xB7,
    kLCDRegister_ILI9341_POWERCONTROL1 = 0xC0,
    kLCDRegister_ILI9341_POWERCONTROL2 = 0xC1,
    kLCDRegister_ILI9341_VCOMCONTROL1 = 0xC5,
    kLCDRegister_ILI9341_VCOMCONTROL2 = 0xC7,
    kLCDRegister_ILI9341_ReadID4 = 0xD3,
} kLCDRegister_ILI9341_t;

#define kLCDRegister_ILI9341_MADCTL_MY 0x80
#define kLCDRegister_ILI9341_MADCTL_MX 0x40
#define kLCDRegister_ILI9341_MADCTL_MV 0x20
#define kLCDRegister_ILI9341_MADCTL_ML 0x10
#define kLCDRegister_ILI9341_MADCTL_RGB 0x00
#define kLCDRegister_ILI9341_MADCTL_BGR 0x08
#define kLCDRegister_ILI9341_MADCTL_MH 0x04

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint16_t width;
    uint16_t height;
} ILI9341_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int ILI9341Setup(uint16_t inWidth, uint16_t inHeight);
///
void ILI9341SetCursor(uint16_t inPositonX, uint16_t inPositonY);
///
void ILI9341SetPortrait1(void);
///
void ILI9341SetLandscape1(void);
///
void ILI9341SetPortrait2(void);
///
void ILI9341SetLandscape2(void);

#endif // ILI9341_H_


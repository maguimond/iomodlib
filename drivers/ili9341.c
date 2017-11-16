// ----------------------------------------------------------------------------
// ili9341.c
//
// Copyright (C) 2017 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

// Standard includes.
#include <stdint.h>

// Drivers includes.
#include "ili9341.h"
#include "fsmc.h"
#include "gpio.h"
#include "utils.h"

// ----------------------------------------------------------------------------
// Private macros.
#define mLCDWriteRegister(inRegister) mFSMCWriteRegister(inRegister)
#define mLCDWriteData(inData) mFSMCWriteData(inData)
#define mLCDReadData() mFSMCReadData()

// ----------------------------------------------------------------------------
// Private variables.
ILI9341_t gILI9341;

// ----------------------------------------------------------------------------
static void LCDSetRegisters(void)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_SOFTRESET);
    DelayUs(10000);
    mLCDWriteRegister(kLCDRegister_ILI9341_DISPLAYOFF);
    mLCDWriteRegister(kLCDRegister_ILI9341_POWERCONTROL1);
    mLCDWriteData(0x23);
    mLCDWriteRegister(kLCDRegister_ILI9341_POWERCONTROL2);
    mLCDWriteData(0x10);
    mLCDWriteRegister(kLCDRegister_ILI9341_VCOMCONTROL1);
    mLCDWriteData(0x2B);
    mLCDWriteData(0x2B);
    mLCDWriteRegister(kLCDRegister_ILI9341_VCOMCONTROL2);
    mLCDWriteData(0xC0);
    mLCDWriteRegister(kLCDRegister_ILI9341_PIXELFORMAT);
    mLCDWriteData(0x55);
    mLCDWriteRegister(kLCDRegister_ILI9341_FRAMECONTROL);
    mLCDWriteData(0x00);
    mLCDWriteData(0x1B);
    mLCDWriteRegister(kLCDRegister_ILI9341_ENTRYMODE);
    mLCDWriteData(0x07);
    mLCDWriteRegister(kLCDRegister_ILI9341_SLEEPOUT);
    DelayUs(10000);
    mLCDWriteRegister(kLCDRegister_ILI9341_DISPLAYON);
    DelayUs(10000);
}

// ----------------------------------------------------------------------------
static void ILI9341SetAddressWindows(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_COLADDRSET);
    mLCDWriteData(x1 >> 8);
    mLCDWriteData(x1);
    mLCDWriteData(x2 >> 8);
    mLCDWriteData(x2);

    mLCDWriteRegister(kLCDRegister_ILI9341_PAGEADDRSET);
    mLCDWriteData(y1 >> 8);
    mLCDWriteData(y1);
    mLCDWriteData(y2 >> 8);
    mLCDWriteData(y2);
}

// ----------------------------------------------------------------------------
static uint16_t ILI9341ReadID(void)
{
    uint16_t driver;
    mLCDWriteRegister(kLCDRegister_ILI9341_ReadID4);
    // 1st parameter is dummy.
    driver = mLCDReadData();
    // 2nd parameter is the IC version (we don't care).
    driver = mLCDReadData();
    // The ID is in the 3rd and 4th parameters.
    driver = (mLCDReadData() << 8);
    driver |= mLCDReadData();

    return driver;
}

// ----------------------------------------------------------------------------
int ILI9341Setup(uint16_t inWidth, uint16_t inHeight)
{
    gILI9341.width = inWidth;
    gILI9341.height = inHeight;

    int status = 0;

    // TODO: Setup like in BLE HAL.
    GPIOSetupLCD();
    FSMCSetup();

    GPIOResetLCD();

    uint16_t lcdDriver = ILI9341ReadID();

    if (lcdDriver == kLCDDriver_ILI9341)
    {
        LCDSetRegisters();
    }
    else
    {
        status = -1;
    }

    return status;
}

// ----------------------------------------------------------------------------
void ILI9341SetCursor(uint16_t inPositonX, uint16_t inPositonY)
{
    ILI9341SetAddressWindows(inPositonX, inPositonY, gILI9341.width - 1, gILI9341.height - 1);
}

// ----------------------------------------------------------------------------
void ILI9341SetPortrait1(void)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_MEMCONTROL);
    mLCDWriteData(kLCDRegister_ILI9341_MADCTL_ML | kLCDRegister_ILI9341_MADCTL_MY | kLCDRegister_ILI9341_MADCTL_BGR);
    // Initialize default full-screen address window.
    // TODO: Is this necessary?
    ILI9341SetCursor(0, 0);
}

// ----------------------------------------------------------------------------
void ILI9341SetLandscape1(void)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_MEMCONTROL);
    mLCDWriteData(kLCDRegister_ILI9341_MADCTL_MX | kLCDRegister_ILI9341_MADCTL_MY | kLCDRegister_ILI9341_MADCTL_MV | kLCDRegister_ILI9341_MADCTL_BGR);
    // Initialize default full-screen address window.
    // TODO: Is this necessary?
    ILI9341SetCursor(0, 0);
}

// ----------------------------------------------------------------------------
void ILI9341SetPortrait2(void)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_MEMCONTROL);
    mLCDWriteData(kLCDRegister_ILI9341_MADCTL_MX | kLCDRegister_ILI9341_MADCTL_BGR);
    // Initialize default full-screen address window.
    // TODO: Is this necessary?
    ILI9341SetCursor(0, 0);
}

// ----------------------------------------------------------------------------
void ILI9341SetLandscape2(void)
{
    mLCDWriteRegister(kLCDRegister_ILI9341_MEMCONTROL);
    mLCDWriteData(kLCDRegister_ILI9341_MADCTL_MV | kLCDRegister_ILI9341_MADCTL_BGR);
    // Initialize default full-screen address window.
    // TODO: Is this necessary?
    ILI9341SetCursor(0, 0);
}

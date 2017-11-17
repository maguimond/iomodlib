// ----------------------------------------------------------------------------
// ili9325.c
//
// Copyright (C) 2017 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

// Standard includes.
#include <stdint.h>

// Drivers includes.
#include "ili9325.h"
#include "fsmc.h"

// ----------------------------------------------------------------------------
// Private macros.
#define mLCDWriteRegister(inRegister, inData) mFSMCWriteRegisterData(inRegister, inData)
#define mLCDReadData() mFSMCReadData()

// ----------------------------------------------------------------------------
// Private variables.
static uint8_t gILI9325Orientation;

static const uint16_t kLCDRegisterTable_ILI9325[] =
{
    // Start oscillator.
    kLCDRegister_ILI9325_START_OSC, 0x0001,
    // 50 millisecond delay.
    kLCDDelay, 50,
    kLCDRegister_ILI9325_DRIV_OUT_CTRL, 0x0100,
    kLCDRegister_ILI9325_DRIV_WAV_CTRL, 0x0700,
    kLCDRegister_ILI9325_ENTRY_MOD, 0x1030,
    kLCDRegister_ILI9325_RESIZE_CTRL, 0x0000,
    kLCDRegister_ILI9325_DISP_CTRL2, 0x0202,
    kLCDRegister_ILI9325_DISP_CTRL3, 0x0000,
    kLCDRegister_ILI9325_DISP_CTRL4, 0x0000,
    kLCDRegister_ILI9325_RGB_DISP_IF_CTRL1, 0x0,
    kLCDRegister_ILI9325_FRM_MARKER_POS, 0x0,
    kLCDRegister_ILI9325_RGB_DISP_IF_CTRL2, 0x0,
    kLCDRegister_ILI9325_POW_CTRL1, 0x0000,
    kLCDRegister_ILI9325_POW_CTRL2, 0x0007,
    kLCDRegister_ILI9325_POW_CTRL3, 0x0000,
    kLCDRegister_ILI9325_POW_CTRL4, 0x0000,
    kLCDDelay, 200,
    kLCDRegister_ILI9325_POW_CTRL1, 0x1690,
    kLCDRegister_ILI9325_POW_CTRL2, 0x0227,
    kLCDDelay, 50,
    kLCDRegister_ILI9325_POW_CTRL3, 0x001A,
    kLCDDelay, 50,
    kLCDRegister_ILI9325_POW_CTRL4, 0x1800,
    kLCDRegister_ILI9325_POW_CTRL7, 0x002A,
    kLCDDelay, 50,
    kLCDRegister_ILI9325_GAMMA_CTRL1, 0x0000,
    kLCDRegister_ILI9325_GAMMA_CTRL2, 0x0000,
    kLCDRegister_ILI9325_GAMMA_CTRL3, 0x0000,
    kLCDRegister_ILI9325_GAMMA_CTRL4, 0x0206,
    kLCDRegister_ILI9325_GAMMA_CTRL5, 0x0808,
    kLCDRegister_ILI9325_GAMMA_CTRL6, 0x0007,
    kLCDRegister_ILI9325_GAMMA_CTRL7, 0x0201,
    kLCDRegister_ILI9325_GAMMA_CTRL8, 0x0000,
    kLCDRegister_ILI9325_GAMMA_CTRL9, 0x0000,
    kLCDRegister_ILI9325_GAMMA_CTRL10, 0x0000,
    kLCDRegister_ILI9325_GRAM_HOR_AD, 0x0000,
    kLCDRegister_ILI9325_GRAM_VER_AD, 0x0000,
    kLCDRegister_ILI9325_HOR_START_AD, 0x0000,
    kLCDRegister_ILI9325_HOR_END_AD, 0x00EF,
    kLCDRegister_ILI9325_VER_START_AD, 0X0000,
    kLCDRegister_ILI9325_VER_END_AD, 0x013F,
    // Driver Output Control (R60h)
    kLCDRegister_ILI9325_GATE_SCAN_CTRL1, 0xA700,
    // Driver Output Control (R61h)
    kLCDRegister_ILI9325_GATE_SCAN_CTRL2, 0x0003,
    // Driver Output Control (R62h)
    kLCDRegister_ILI9325_GATE_SCAN_CTRL3, 0x0000,
    // Panel Interface Control 1 (R90h)
    kLCDRegister_ILI9325_PANEL_IF_CTRL1, 0X0010,
    kLCDRegister_ILI9325_PANEL_IF_CTRL2, 0X0000,
    kLCDRegister_ILI9325_PANEL_IF_CTRL3, 0X0003,
    kLCDRegister_ILI9325_PANEL_IF_CTRL4, 0X1100,
    kLCDRegister_ILI9325_PANEL_IF_CTRL5, 0X0000,
    kLCDRegister_ILI9325_PANEL_IF_CTRL6, 0X0000,
    // Main screen turn on
    kLCDRegister_ILI9325_DISP_CTRL1, 0x0133,
};

// ----------------------------------------------------------------------------
static void LCDSetRegisters(uint16_t driver)
{
    uint8_t registerIndex = 0;
    uint16_t registerAddress;
    uint16_t registerValue;
    while(registerIndex < sizeof(kLCDRegisterTable_ILI9325) / sizeof(uint16_t))
    {
        registerAddress = kLCDRegisterTable_ILI9325[registerIndex ++];
        registerValue = kLCDRegisterTable_ILI9325[registerIndex ++];
        if(registerAddress == kLCDDelay)
        {
            Delay(registerValue);
        }
        else
        {
            mLCDWriteRegister(registerAddress, registerValue);
        }
    }
}

// ----------------------------------------------------------------------------
static void ILI9325SetAddressWindows(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight)
{
    // Set address window.
    mLCDWriteRegister(kLCDRegister_ILI9325_HOR_START_AD, inPositionX);
    mLCDWriteRegister(kLCDRegister_ILI9325_HOR_END_AD, inWidth);
    mLCDWriteRegister(kLCDRegister_ILI9325_VER_START_AD, inPositionY);
    mLCDWriteRegister(kLCDRegister_ILI9325_VER_END_AD, inHeight);
    // Set address counter to top left
    ILI9325SetCursor(0, 0, inWidth, inHeight);
}

// ----------------------------------------------------------------------------
static uint16_t ILI9325ReadID(void)
{
    uint16_t driver;
    driver = LCDReadRegister(kLCDRegister_ILI9325_ReadID);
    driver = mLCDReadData();
    return driver;
}

// ----------------------------------------------------------------------------
int ILI9325Setup(void)
{
    int status = 0;

    // Setup like in BLE HAL.
    GPIOSetupLCD();
    FSMCSetup();

    ILI9325Reset();

    uint16_t lcdDriver = ILI9325ReadID();

    if (lcdDriver == kLCDDriver_ILI9325)
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
void ILI9325SetCursor(uint16_t inPositionX, uint16_t inPositionY, uint16_t inWidth, uint16_t inHeight)
{
    uint16_t cursorX;
    uint16_t cursorY;

    switch (gILI9325Orientation)
    {
        case 0:
            cursorX = inWidth - 1 - inPositionX;
            cursorY = inHeight - 1 - inPositionY;
            break;
        case 1:
            cursorX = inPositionY;
            cursorY = inHeight - 1 - inPositionX;
            break;
        case 2:
            cursorX = inPositionX;
            cursorY = inPositionY;
            break;
        case 3:
            cursorX = inWidth - 1 - inPositionY;
            cursorY = inPositionX;
            break;
    }

    // Set the X address of the display cursor.
    mLCDWriteRegister(kLCDRegister_ILI9325_GRAM_HOR_AD, cursorX);
    // Set the Y address of the display cursor.
    mLCDWriteRegister(kLCDRegister_ILI9325_GRAM_VER_AD, cursorY);
}

// ----------------------------------------------------------------------------
void ILI9325SetPortrait1(uint16_t inWidth, uint16_t inHeight)
{
    gILI9325Orientation = 0;
    mLCDWriteRegister(kLCDRegister_ILI9325_ENTRY_MOD, kLCDRegister_ILI9325_ENTRY_OR3 | kLCDRegister_ILI9325_ENTRY_BGR);
    // TODO: Is this necessary?
    // For ILI932x, initialize default full-screen address window.
    ILI9325SetAddressWindows(0, 0, inWidth, inHeight);
}

// ----------------------------------------------------------------------------
void ILI9325SetLandscape1(uint16_t inWidth, uint16_t inHeight)
{
    gILI9325Orientation = 1;
    mLCDWriteRegister(kLCDRegister_ILI9325_ENTRY_MOD, kLCDRegister_ILI9325_ENTRY_OR4 | kLCDRegister_ILI9325_ENTRY_BGR);
    // TODO: Is this necessary?
    // For ILI932x, initialize default full-screen address window.
    ILI9325SetAddressWindows(0, 0, inWidth, inHeight);
}

// ----------------------------------------------------------------------------
void ILI9325SetPortrait2(uint16_t inWidth, uint16_t inHeight)
{
    gILI9325Orientation = 2;
    mLCDWriteRegister(kLCDRegister_ILI9325_ENTRY_MOD, kLCDRegister_ILI9325_ENTRY_OR1 | kLCDRegister_ILI9325_ENTRY_BGR);
    // TODO: Is this necessary?
    // For ILI932x, initialize default full-screen address window.
    ILI9325SetAddressWindows(0, 0, inWidth, inHeight);
}

// ----------------------------------------------------------------------------
void ILI9325SetLandscape2(uint16_t inWidth, uint16_t inHeight)
{
    gILI9325Orientation = 3;
    mLCDWriteRegister(kLCDRegister_ILI9325_ENTRY_MOD, kLCDRegister_ILI9325_ENTRY_OR2 | kLCDRegister_ILI9325_ENTRY_BGR);
    // TODO: Is this necessary?
    // For ILI932x, initialize default full-screen address window.
    ILI9325SetAddressWindows(0, 0, inWidth, inHeight);
}

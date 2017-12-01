// ----------------------------------------------------------------------------
// iomdutils.h
//
// Copyright (C) 2017 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ----------------------------------------------------------------------------

#ifndef IOMOD_UTILS_
#define IOMOD_UTILS_

#include <stdint.h>

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

#define kIOMod_BusError 0x0100
#define kIOMod_DriverError 0x0200

// Error codes
typedef enum
{
    kIOModError_Argument = 1,
    kIOModError_FlashBusy,
    kIOModError_FlashErase,
    kIOModError_FlashProg,
    kIOModError_FlashWNE,
    kIOModError_UnsupportedDevice,
    kIOModError_DeviceNotFound,
    kIOModError_Unknown,
    kIOModError_Size,
} IOModError_Code_t;

#define kIOModError_Table \
{ \
    {kIOModError_Argument, "Invalid arg"}, \
    {kIOModError_FlashBusy, "Flash busy"}, \
    {kIOModError_FlashErase,"Flash erase"}, \
    {kIOModError_FlashProg, "Flash programming"}, \
    {kIOModError_FlashWNE, "Flash write not enabled"}, \
    {kIOModError_UnsupportedDevice, "Unsupported device"}, \
    {kIOModError_DeviceNotFound, "Device not found"}, \
    {kIOModError_Unknown, "Unknown"}, \
}

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define mIOValidateBus(returnStatus) if (returnStatus != 0) { return kIOMod_BusError | returnStatus; }
#define mIOValidateDriver(returnStatus) if (returnStatus != 0) { return kIOMod_DriverError | returnStatus; }
#define mIOAssertArg(expr) ((expr) ? (void)0: AssertFailure((uint8_t *)__FILE__, __LINE__, __FUNCTION__))
#define mHTONS(x) (((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    int errorNo;
    const char* description;
} IOModError_Table_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
/// Custom assert, including function context.
void AssertFailure(const uint8_t* inFile, uint32_t inLine, const char* inFunction);


#endif // IOMOD_UTILS_

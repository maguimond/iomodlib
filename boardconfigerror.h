// ----------------------------------------------------------------------------
// boardconfigerror.h
//
// Copyright (C) 2017 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ----------------------------------------------------------------------------

#ifndef BOARDCONFIGERROR_H_
#define BOARDCONFIGERROR_H_

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// Error codes
typedef enum
{
    kError_FlashBusy = 1,
    kError_FlashErase,
    kError_FlashProg,
    kError_FlashWNE,
    kError_UnsupportedDevice,
    kError_DeviceNotFound,
    kError_Unknown,
    kError_Size,
} ErrorCode;

#define kErrorTable \
{ \
    {kError_FlashBusy, "Flash busy"}, \
    {kError_FlashErase,"Flash erase"}, \
    {kError_FlashProg, "Flash programming"}, \
    {kError_FlashWNE,  "Flash write not enabled"}, \
    {kError_UnsupportedDevice,  "Unsupported device"}, \
    {kError_DeviceNotFound,  "Device not found"}, \
    {kError_Unknown,   "Unknown"}, \
}

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    int errorNo;
    const char* description;
} ErrorTable_t;

#ifdef __cplusplus
}
#endif

#endif // BOARDCONFIGERROR_H_

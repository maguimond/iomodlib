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
    kBoardConfigError_FlashBusy = 1,
    kBoardConfigError_FlashErase,
    kBoardConfigError_FlashProg,
    kBoardConfigError_FlashWNE,
    kBoardConfigError_UnsupportedDevice,
    kBoardConfigError_DeviceNotFound,
    kBoardConfigError_Unknown,
    kBoardConfigError_Size,
} BoardConfigError_Code_t;

#define kBoardConfigError_Table \
{ \
    {kBoardConfigError_FlashBusy, "Flash busy"}, \
    {kBoardConfigError_FlashErase,"Flash erase"}, \
    {kBoardConfigError_FlashProg, "Flash programming"}, \
    {kBoardConfigError_FlashWNE,  "Flash write not enabled"}, \
    {kBoardConfigError_UnsupportedDevice,  "Unsupported device"}, \
    {kBoardConfigError_DeviceNotFound,  "Device not found"}, \
    {kBoardConfigError_Unknown,   "Unknown"}, \
}

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    int errorNo;
    const char* description;
} BoardConfigError_Table_t;

#ifdef __cplusplus
}
#endif

#endif // BOARDCONFIGERROR_H_

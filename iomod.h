/* Copyright (C) 2016, Marc-Andre Guimond <guimond.marcandre@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is encoded in UTF-8.
 */

#ifndef IOMOD_H_
#define IOMOD_H_

// Standard includes.
#include <stdint.h>

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// TODO: define in config.
#define kIOModMaxIOs 8

// Status codes.
typedef enum
{
    kIOMod_Status_NotDetected,
    kIOMod_Status_Valid,
    kIOMod_Status_InvalidRange,
    kIOMod_Status_OverCurrent,
    // Reserved for future use, keep last.
    kIOMod_Status_Reserved,
} PortStatus_Status_t;

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------

typedef struct
{
    uint8_t status;
    uint8_t deviceID;
    uint8_t outputLevel;
    // Depending on the architecture, use signed 16-bits or float.
#ifdef GCC_ARMCM3
    int16_t data;
#else
    float data;
#endif
} IOModPort_t;

typedef struct
{
    uint8_t eepromLayout;
    uint8_t major;
    uint8_t minor;
} IOModVersion_t;

typedef struct
{
    IOModVersion_t version;
    uint8_t model;
    IOModPort_t io[kIOModMaxIOs];
} IOMod_t;

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
#define mIOModValidateStatus(returnStatus) if (returnStatus != 0) { return returnStatus; }

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int IOModADCInit(uint8_t inSlaveID);
///
int IOModGetTemperature(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData);

#endif // IOMOD_H_

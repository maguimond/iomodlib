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
// Status codes.
typedef enum
{
    kIOModPortStatus_Valid,
    kIOModPortStatus_NotDetected, // TODO
    kIOModPortStatus_DriverBusError,
    kIOModPortStatus_InvalidRange,
    kIOModPortStatus_OverLoad,
    kIOModPortStatus_OpenLoad, // TODO
    // Reserved for future use, keep last.
    kIOModPortStatus_Max,
} IOModPortStatus_e;

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------

typedef struct
{
    uint8_t eepromLayout;
    uint8_t major;
    uint8_t minor;
} IOModVersion_t;

typedef struct
{
    // TODO: Add enum for models.
    uint8_t model;
    IOModVersion_t version;
} IOMod_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
IOModPortStatus_e IOModADCInit(uint8_t inSlaveID);
///
IOModPortStatus_e IOModGetTemperature(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData);
///
IOModPortStatus_e IOModGetInternalTemperature(uint8_t inSlaveID, int32_t* outADCData);
///
IOModPortStatus_e IOModGetCurrent(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData);

#endif // IOMOD_H_

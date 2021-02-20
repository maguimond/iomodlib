/* Copyright (C) 2017, Marc-Andre Guimond <guimond.marcandre@gmail.com>.
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

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

// Standard includes.
#include <stdint.h>

// User config.
#include "boardconfiguser.h"

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint8_t pcbVersionMaj;
    uint8_t pcbVersionMin;
    uint8_t fwVersionMaj;
    uint8_t fwVersionMin;
} boardConfigInfo_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int BoardConfigCommit(void);

int BoardConfigResetFactory(void);
/// Initialize the master, retrieve configuration from non-volatile memory.
int BoardConfigInit(void);
/// Write configuration data to master card.
int BoardConfigWrite(uint8_t inAddress, uint8_t* inData, uint8_t inSize);
///
int BoardConfigRead(uint8_t inAddress, uint8_t* outData, uint8_t inSize);
///
uint8_t BoardConfigReadByte(uint8_t inAddress);

#endif // BOARDCONFIG_H_

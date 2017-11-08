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

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
#define kBoardConfigMagicNumber 0xDEAD
#define kMasterConfig_Size (kMasterConfig_CRC - kMasterConfig_Magic)
#define kMasterConfig_WriteQueueLenght 4
#define kSlaveConfig_Size (kSlaveConfig_CRC - kSlaveConfig_Magic)
#define kSlaveConfig_WriteQueueLenght 4

typedef enum
{
    SlaveID1,
    SlaveID2,
    SlaveID3,
    SlaveID4,
    SlaveID5,
    SlaveID6,
    SlaveID_Max,
    MasterID = 0x0A,
} BoardID_t;

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
/// Initialize the master, retrieve configuration from non-volatile memory.
int MasterConfigInit(uint8_t inSlaveCount);
/// Discover attached slave modules by reading EEPROM addresses.
uint8_t SlaveDiscovery(uint8_t* outSlaveAddressMap);
/// Initialize a slave card (selected by SlaveID), retrieve configuration from non-volatile memory.
int SlaveConfigInit(uint8_t inSlaveID);
/// Write configuration data to master card.
void MasterConfigWrite(uint8_t inAddress, uint8_t* inData, uint8_t inSize);
/// Write configuration data to a slave card (selected by inSlaveID).
void SlaveConfigWrite(uint8_t inSlaveID, uint8_t inAddress, uint8_t* inData, uint8_t inSize);
///
void MasterConfigRead(uint8_t inAddress, uint8_t* outData, uint8_t inSize);
///
void SlaveConfigRead(uint8_t inSlaveID, uint8_t inAddress, uint8_t* outData, uint8_t inSize);
///
uint8_t MasterConfigReadByte(uint8_t inAddress);
///
uint8_t SlaveConfigReadByte(uint8_t inSlaveID, uint8_t inAddress);
///
uint16_t SlaveConfigReadShort(uint8_t inSlaveID, uint8_t inAddress);
///
void TaskBoardConfig(void* args);

#endif // BOARDCONFIG_H_

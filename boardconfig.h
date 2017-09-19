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

#ifndef __CFG_H__
#define __CFG_H__

// Standard includes.
#include <stdint.h>

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
#define kBoardConfigMagicNumber 0xDEAD
#define kMasterConfig_FactorySize (kMasterConfigFactory_CRC - kMasterConfigFactory_Magic)
#define kMasterConfig_UserSize (kMasterConfigUser_CRC - kMasterConfigUser_IPAddr)
#define kMasterConfig_SerialNumberSize (kMasterConfigFactory_CRC - kMasterConfigFactory_SerialNumber)
#define kMasterConfig_WriteQueueLenght 4
#define kSlaveConfig_CRCSize (kSlaveConfig_CRC - kSlaveConfig_Magic)
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

// Master config non-volatile memory layout.
// All multibyte values are in big-endian format.
// All fields fit within a single page.
typedef enum
{
    // Factory Settings.
    kMasterConfigFactory_Magic = 0x0000,
    kMasterConfigFactory_FlashLayout = 0x0002,
    kMasterConfigFactory_SerialNumber = 0x0003,
    kMasterConfigFactory_CRC = 0x002c,
    // User Settings.
    kMasterConfigUser_IPAddr = 0x0030,
    kMasterConfigUser_IPMask = 0x0034,
    kMasterConfigUser_IPRouter = 0x0038,
    kMasterConfigUser_IPMode = 0x003c,
    kMasterConfigUser_DHCPSMode = 0x003d,
    kMasterConfigUser_DHCPSChain = 0x003e,
    kMasterConfigUser_BLEAdvTimeout = 0x0040,
    kMasterConfigUser_BLEAdvInterval = 0x0042,
    kMasterConfigUser_BLETxPower = 0x0044,
    kMasterConfigUser_SlavesStatus = 0x0045,
    kMasterConfigUser_CRC = 0x0046,
    // Keep at the end of user config space.
    kMasterConfigUser_End = 0x00FF,
    // End.
    kMasterConfig_Max = 0x1000
} masterConfig_t;

#define kMasterConfig_CRCValid 0x0
#define kMasterConfig_CRCInvalid 0xEE
#define kMasterConfig_IPModeDynamic 0x00
#define kMasterConfig_IPModeStatic 0x01
// Maximum advertising time in seconds (0 means infinite).
#define kMasterConfig_DefaultConnectionTimeout 300
// Advertising interval (multiple of 0.625ms).
#define kMasterConfig_DefaultAdvertisingInterval 80

// Slave config non-volatile memory layout.
// All multibyte values are in big-endian format.
// All fields fit within a single page.
typedef enum
{
    // Factory Settings.
    kSlaveConfig_Magic = 0x00,
    kSlaveConfig_VersionMajor = 0x02,
    kSlaveConfig_VersionMinor = 0x03,
    kSlaveConfig_Model = 0x04,
    kSlaveConfig_EEPROMLayout = 0x05,
    kSlaveConfig_DeviceID1 = 0x06,
    kSlaveConfig_DeviceID2 = 0x07,
    kSlaveConfig_DeviceID3 = 0x08,
    kSlaveConfig_DeviceID4 = 0x09,
    kSlaveConfig_TxConfig = 0x0A,
    kSlaveConfig_I2CADCAddress = 0x0B,
    kSlaveConfig_CRC = 0x0C,
    // Logging Settings.
    kSlaveConfig_Reflash = 0x20,
    // End.
    kSlaveConfig_Max = 0xFF
} slaveConfig_t;

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
int MasterConfigInit(uint8_t inSlavesCount);
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

#endif

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

// Standard includes
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Scheduler includes.
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

// App includes.
#include "boardConfig.h"
#include "CACIOModule.h"
#include "flashConfig.h"

// HAL includes.
#include "s25fl256.h"
#include "pca9500.h"
#include "adc128d818.h"

// Common includes.
#include "utils.h"
#include "crc16.h"
#include "error.h"
#include "version.h"

// ----------------------------------------------------------------------------
// Private constants.
// Map constants to HAL.
#define kMasterPageSize kS25FL256_PageSize256B
#define kMasterMemorySize kS25FL256_PageSize256B //kS25FL256_4KSectorSize
#define kSlavePageSize kPCA9500_EEPROMPageSize
#define kSlaveMemorySize kPCA9500_EEPROMTotSize

// TODO: Remove.
#define CONFIG_FACTORY 1

// Private variables.
static bool gIsBoardConfigInitDone = false;
static xQueueHandle BoardConfigWriteQueue;
static xSemaphoreHandle gBoardConfigMutex;
static uint16_t masterPageSize;

// Shadow RAM.
static uint8_t gMasterConfigShadowRAM[kMasterMemorySize];
static uint8_t gSlaveConfigShadowRAM[SlaveID_Max][kSlaveMemorySize];

// Private structs.
typedef struct
{
    uint8_t boardID;
    uint8_t address;
    uint8_t size;
    uint8_t data[kSlavePageSize];
    uint16_t crc;
} boardWriteMessage_t;

// Public variables.
extern xTaskHandle taskHandleBoardConfig;

// ----------------------------------------------------------------------------
static void MasterConfigSetFactoryDefaults(void)
{
    // Default the factory section to 0xFF.
    memset(gMasterConfigShadowRAM + kMasterConfigFactory_Magic, 0xFF, kMasterConfigUser_IPAddr - kMasterConfigFactory_Magic);
    PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Forcing default values");
#if CONFIG_FACTORY == 1
    // TODO: Use config tool.
    *(gMasterConfigShadowRAM + kMasterConfigFactory_Magic + 0) = (uint8_t)(kBoardConfigMagicNumber >> 8);
    *(gMasterConfigShadowRAM + kMasterConfigFactory_Magic + 1) = (uint8_t)(kBoardConfigMagicNumber & 0xFF);
    *(gMasterConfigShadowRAM + kMasterConfigFactory_FlashLayout + 0) = kVersionConfigLayout;
#endif
}

// ----------------------------------------------------------------------------
static void MasterConfigSetUserDefaults(uint8_t inSlavesStatus)
{
    memset(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr, 0x00, kMasterConfigUser_End - kMasterConfigUser_IPAddr);

    PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Forcing default values");

    // Server IP Address.
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr + 0) = 192;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr + 1) = 168;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr + 2) = 2;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr + 3) = 11;

    // IP Netmask.
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPMask + 0) = 255;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPMask + 1) = 255;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPMask + 2) = 255;
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPMask + 3) = 0;

    // IP Mode
    *(gMasterConfigShadowRAM + kMasterConfigUser_IPMode) = kMasterConfig_IPModeStatic;

    // BLE settings.
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLEAdvTimeout + 3) = 0;
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLEAdvTimeout + 0) = (uint8_t)(kMasterConfig_DefaultConnectionTimeout >> 8);
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLEAdvTimeout + 1) = (uint8_t)(kMasterConfig_DefaultConnectionTimeout & 0xFF);
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLEAdvInterval + 0) = (uint8_t)(kMasterConfig_DefaultAdvertisingInterval >> 8);
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLEAdvInterval + 1) = (uint8_t)(kMasterConfig_DefaultAdvertisingInterval & 0xFF);
    *(gMasterConfigShadowRAM + kMasterConfigUser_BLETxPower) = 0x03;

    // Slaves settings.
    *(gMasterConfigShadowRAM + kMasterConfigUser_SlavesStatus) = inSlavesStatus;
}

// ----------------------------------------------------------------------------
static void SlaveConfigSetDefaults(uint8_t inSlaveID)
{
    // Default the factory section to 0xFF.
    memset(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic, 0xFF, kSlaveConfig_Reflash - kSlaveConfig_Magic);
    PrintMessage("%s : %s - Warning: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Forcing default values", inSlaveID + 1);

    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic + 0) = (uint8_t)(kBoardConfigMagicNumber >> 8);
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic + 1) = (uint8_t)(kBoardConfigMagicNumber & 0xFF);

    // Undefined - use config tool.
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Model + 0) = kSlaveConfigHWModel_Unknown;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_VersionMajor + 0) = 0x00;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_VersionMinor + 0) = 0x00;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_EEPROMLayout + 0) = 0x00;

    // Device IDs.
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_DeviceID1) = kPortConfigDeviceID_None;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_DeviceID2) = kPortConfigDeviceID_None;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_DeviceID3) = kPortConfigDeviceID_None;
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_DeviceID4) = kPortConfigDeviceID_None;

    // Tx config.
    *(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_TxConfig) = 0;
}

// ----------------------------------------------------------------------------
// WARNING: does not acquire config mutex
// Write full User Settings to non-volatile memory. This is done only if the Factory Settings section is valid.
static void MasterConfigSave(void)
{
    // Compute CRC.
    uint16_t crc;
    // TODO: REMOVE, use config tool
#if CONFIG_FACTORY == 1
    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfigFactory_Magic), kMasterConfig_FactorySize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfigFactory_CRC)) = mHTONS(crc);
#endif
    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr), kMasterConfig_UserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfigUser_CRC)) = mHTONS(crc);

    S25FL256Erase64K(kMasterConfigFactory_Magic);
    // FIXME: Add sync here - erase command takes too long flash still busy after.
    DelayLoop(100000000);

    int status;
    // Write everything to non-volatile memory.
#if CONFIG_FACTORY == 1
    status = S25FL256PageWrite(kMasterConfigFactory_Magic, gMasterConfigShadowRAM + kMasterConfigFactory_Magic, kMasterConfigUser_IPAddr - kMasterConfigFactory_Magic);
    if (status != kSuccess)
    {
        PrintMessage("%s : %s - Error: %s (master)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status));
    }
#endif
    status = S25FL256PageWrite(kMasterConfigUser_IPAddr, gMasterConfigShadowRAM + kMasterConfigUser_IPAddr, kMasterConfigUser_End - kMasterConfigUser_IPAddr);
    if (status != kSuccess)
    {
        PrintMessage("%s : %s - Error: %s (master)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status));
    }
}

// ----------------------------------------------------------------------------
// WARNING: does not acquire config mutex
// Write full User Settings to non-volatile memory. This is done only if the Factory Settings section is valid.
static void SlaveConfigSave(uint8_t inSlaveID)
{
    uint16_t crc;

    // Compute CRC.
    crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kSlaveConfig_CRCSize);
    *((uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC)) = mHTONS(crc);

    int status;

    // Write everything to non-volatile memory.
    for (uint16_t addressIdx = kSlaveConfig_Magic; addressIdx < kSlaveConfig_Reflash; addressIdx += kSlavePageSize)
    {
        status = PCA9500EEPROMPageWrite(inSlaveID, addressIdx, gSlaveConfigShadowRAM[inSlaveID] + addressIdx, kSlavePageSize);
        if (status != kSuccess)
        {
            PrintMessage("%s : %s - Error: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status), inSlaveID + 1);
        }
    }
}

// ----------------------------------------------------------------------------
// WARNING: does not acquire config mutex.
int MasterConfigInit(uint8_t inSlavesStatus)
{
    // Setup HAL.
    int status = S25FL256Init();
    if (status != kSuccess)
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status));
        return status;
    }

    // Read non-volatile memory in Shadow RAM.
    masterPageSize = S25FL256GetPageSize();
    for (uint32_t addressIdx = 0; addressIdx < kMasterMemorySize; addressIdx += masterPageSize)
    {
        status = S25FL256PageRead(addressIdx, gMasterConfigShadowRAM + addressIdx, masterPageSize);
        if (status != kSuccess)
        {
            return status;
        }
    }

    // Validate factory settings.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfigFactory_Magic), kMasterConfig_FactorySize);
    bool isFactoryConfigValid = false;
    if (*(uint16_t*)(gMasterConfigShadowRAM + kMasterConfigFactory_Magic) != mHTONS(kBoardConfigMagicNumber))
    {
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Wrong magic value");
        isFactoryConfigValid = false;
    }
    else if (gMasterConfigShadowRAM[kMasterConfigFactory_FlashLayout] != kVersionConfigLayout)
    {
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Unsupported layout");
        isFactoryConfigValid = false;
    }
    else if (mHTONS(crc) != *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfigFactory_CRC)))
    {
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Bad factory CRC");
        isFactoryConfigValid = false;
    }
    else
    {
        isFactoryConfigValid = true;
    }

    // Validate user settings. Not valid if the factory settings are not valid too.
    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr), kMasterConfig_UserSize);
    bool isUserConfigValid = false;
    if (!isFactoryConfigValid || mHTONS(crc) != *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfigUser_CRC)))
    {
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Bad user CRC");
        isUserConfigValid = false;
    }
    else if (gMasterConfigShadowRAM[kMasterConfigUser_SlavesStatus] != inSlavesStatus)
    {
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Slave modules mismatch");
        isUserConfigValid = false;
    }
    else
    {
        isUserConfigValid = true;
    }

    // Overwrite shadow contents if non-volatile memory values were not valid.
    if (!isFactoryConfigValid)
    {
        MasterConfigSetFactoryDefaults();
    }

    if (!isUserConfigValid)
    {
        MasterConfigSetUserDefaults(inSlavesStatus);
    }

// TODO: REMOVE
#if CONFIG_FACTORY == 1
    isFactoryConfigValid = true;
#endif
    // Write changes back to non-volatile memory.
    if (isFactoryConfigValid && (!isUserConfigValid))
    {
        MasterConfigSave();
    }

    // Prepare IPC structures
    if ((gBoardConfigMutex = xSemaphoreCreateMutex()) == NULL)
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Can't create gBoardConfigMutex");
        return -1;
    }

    if ((BoardConfigWriteQueue = xQueueCreate(kMasterConfig_WriteQueueLenght, sizeof(boardWriteMessage_t))) == NULL)
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Can't create BoardConfigWriteQueue");
        return -1;
    }

    // Finished boot-time initialization
    gIsBoardConfigInitDone = true;

    return status;
}

// ----------------------------------------------------------------------------
uint8_t SlaveDiscovery(uint8_t* outSlaveAddressMap)
{
    // Setup HAL.
    PCA9500Init();

    bool isPreviousSlaveNotResponding = false;
    uint8_t slavesStatus = 0x00;
    uint8_t slavesCount = 0;

    PrintMessage("%s : %s - Info: Scanning attached slave modules...\n", __FILENAME__, __FUNCTION__);

    for (uint8_t slaveIdx = 0; slaveIdx < SlaveID_Max; slaveIdx ++)
    {
        // Check if the module responds to the command. If successful, it initialize it with all I/Os to 0.
        int status = PCA9500IOExpanderSetPort(slaveIdx, 0x00);

        if (status != kSuccess)
        {
            isPreviousSlaveNotResponding = true;
            // Force uninitialized address.
            *outSlaveAddressMap ++ = 0xFF;
            // No slave not responding, skip.
            continue;
        }
        else
        {
            if (isPreviousSlaveNotResponding)
            {
                PrintMessage("%s : %s - Warning: Inconsistent slave addressing. (Slave ID %d not present?).\n", __FILENAME__, __FUNCTION__, slaveIdx + 1);
            }
            PrintMessage("%s : %s - Info: Found slave ID %d.\n", __FILENAME__, __FUNCTION__, slaveIdx + 1);
            // Slave discovered. Get slave ID.
            *(outSlaveAddressMap) ++ = slaveIdx;
            // Use address index to count discovered slaves.
            slavesStatus |= 0x01 << slaveIdx;
            slavesCount ++;
        }
    }

    PrintMessage("%s : %s - Info: Discovered %d slave modules.\n", __FILENAME__, __FUNCTION__, slavesCount);

    return slavesStatus;
}

// ----------------------------------------------------------------------------
// WARNING: does not acquire EEPROM mutex.
int SlaveConfigInit(uint8_t inSlaveID)
{
    int status = kSuccess;
    // Read non-volatile memory in Shadow RAM.
    for (uint16_t addressIdx = 0; addressIdx < kSlaveMemorySize; addressIdx += kSlavePageSize)
    {
        status = PCA9500EEPROMPageRead(inSlaveID, (uint8_t)addressIdx, gSlaveConfigShadowRAM[inSlaveID] + addressIdx, kSlavePageSize);
        if (status != kSuccess)
        {
            PrintMessage("%s : %s - Error: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status), inSlaveID + 1);
            return status;
        }
    }

    // Validate settings.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kSlaveConfig_CRCSize);
    bool isConfigValid = false;
    if (*(uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic) != mHTONS(kBoardConfigMagicNumber))
    {
        PrintMessage("%s : %s - Warning: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Wrong magic value", inSlaveID + 1);
        isConfigValid = false;
    }
    else if (gSlaveConfigShadowRAM[inSlaveID][kSlaveConfig_EEPROMLayout] != kVersionConfigLayout)
    {
        PrintMessage("%s : %s - Warning: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Unsupported config layout", inSlaveID + 1);
        isConfigValid = false;
    }
    else if (*((uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC)) != mHTONS(crc))
    {
        PrintMessage("%s : %s - Warning: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Bad CRC", inSlaveID + 1);
        isConfigValid = false;
    }
    else
    {
        isConfigValid = true;
    }

    // Overwrite shadow contents if non-volatile memory values were not valid.
    if (!isConfigValid)
    {
        SlaveConfigSetDefaults(inSlaveID);
        // Write changes back to non-volatile memory.
        SlaveConfigSave(inSlaveID);
    }

    return status;
}

// ----------------------------------------------------------------------------
void MasterConfigWrite(uint8_t inAddress, uint8_t* inData, uint8_t inSize)
{
    uint16_t crc;
    boardWriteMessage_t configMessage;

    if (!gIsBoardConfigInitDone)
    {
        CACIOModuleSetServerState(kMasterStatusState_NoConfig);
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Initialization not complete");
        return;
    }

    // Validate address and size arguments.
    if (inSize > kMasterPageSize)
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Bad page size");
        return;
    }
    if ((inAddress < kMasterConfigUser_IPAddr) || (inAddress >= kMasterConfigUser_End))
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Can't write outside CFG_U section");
        return;
    }
    if (((inAddress / kMasterPageSize) != ((inAddress + inSize) / kMasterPageSize)) && ((inAddress + inSize) % kMasterPageSize))
    {
        PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Can't write across pages");
        return;
    }

    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);

    // Perform the write in Shadow RAM
    memcpy(gMasterConfigShadowRAM + inAddress, inData, inSize);

    // Recompute the CRC
    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfigUser_IPAddr), kMasterConfig_UserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfigUser_CRC)) = mHTONS(crc);

    // Send message to write task to commit changes to non-volatile memory.
    configMessage.boardID = MasterID;
    configMessage.address = inAddress;
    configMessage.size = inSize;
    memcpy(configMessage.data, inData, inSize);
    memcpy(&configMessage.crc, gMasterConfigShadowRAM + kMasterConfigUser_CRC, 2);
    xQueueSendToBack(BoardConfigWriteQueue, &configMessage, portMAX_DELAY);

    // Because we release the mutex after sending to the queue, the CRC's will always be in sync with the corresponding data
    xSemaphoreGive(gBoardConfigMutex);
}

// ----------------------------------------------------------------------------
void SlaveConfigWrite(uint8_t inSlaveID, uint8_t inAddress, uint8_t* inData, uint8_t inSize)
{
    uint16_t crc;
    boardWriteMessage_t configMessage;

    if (!gIsBoardConfigInitDone)
    {
        CACIOModuleSetServerState(kMasterStatusState_NoConfig);
        PrintMessage("%s : %s - Warning: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Initialization not complete", inSlaveID + 1);
        return;
    }

    // Validate address and size arguments.
    if (inSize > kSlavePageSize)
    {
        PrintMessage("%s : %s - Error: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Bad page size", inSlaveID + 1);
        return;
    }
    if ((inAddress < kSlaveConfig_VersionMajor) || (inAddress >= kSlaveConfig_Reflash))
    {
        PrintMessage("%s : %s - Error: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Can't write outside CFG_U section", inSlaveID + 1);
        return;
    }
    if (((inAddress / kSlavePageSize) != ((inAddress + inSize) / kSlavePageSize)) && ((inAddress + inSize) % kSlavePageSize))
    {
        PrintMessage("%s : %s - Error: %s (slave ID %d)\n", __FILENAME__, __FUNCTION__, "Can't write across pages", inSlaveID + 1);
        return;
    }

    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);

    // Perform the write in Shadow RAM
    memcpy(gSlaveConfigShadowRAM[inSlaveID] + inAddress, inData, inSize);

    // Recompute the CRC
    crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kSlaveConfig_CRCSize);
    *((uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC)) = mHTONS(crc);

    // Send message to write task to commit changes to non-volatile memory.
    configMessage.boardID = inSlaveID;
    configMessage.address = inAddress;
    configMessage.size = inSize;
    memcpy(configMessage.data, inData, inSize);
    memcpy(&configMessage.crc, gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC, 2);
    xQueueSendToBack(BoardConfigWriteQueue, &configMessage, portMAX_DELAY);

    // Because we release the mutex after sending to the queue, the CRC's will always be in sync with the corresponding data
    xSemaphoreGive(gBoardConfigMutex);
}

// ----------------------------------------------------------------------------
void MasterConfigRead(uint8_t inAddress, uint8_t* outData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        CACIOModuleSetServerState(kMasterStatusState_NoConfig);
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Initialization not complete");
        memset(outData, 0, inSize);
        return;
    }

    // Perform the read from Shadow RAM.
    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);
    memcpy(outData, gMasterConfigShadowRAM + inAddress, inSize);
    xSemaphoreGive(gBoardConfigMutex);
}

// ----------------------------------------------------------------------------
void SlaveConfigRead(uint8_t inSlaveID, uint8_t inAddress, uint8_t* outData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        CACIOModuleSetServerState(kMasterStatusState_NoConfig);
        PrintMessage("%s : %s - Warning: %s\n (slave ID %d)", __FILENAME__, __FUNCTION__, "Initialization not complete", inSlaveID + 1);
        memset(outData, 0, inSize);
        return;
    }

    // Perform the read from Shadow RAM.
    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);
    memcpy(outData, gSlaveConfigShadowRAM[inSlaveID] + inAddress, inSize);
    xSemaphoreGive(gBoardConfigMutex);
}

// ----------------------------------------------------------------------------
uint8_t MasterConfigReadByte(uint8_t inAddress)
{
    uint8_t data;
    MasterConfigRead(inAddress, &data, 1);
    return data;
}

// ----------------------------------------------------------------------------
uint8_t SlaveConfigReadByte(uint8_t inSlaveID, uint8_t inAddress)
{
    uint8_t data;
    SlaveConfigRead(inSlaveID, inAddress, &data, 1);
    return data;
}

// ----------------------------------------------------------------------------
uint16_t SlaveConfigReadShort(uint8_t inSlaveID, uint8_t inAddress)
{
    uint16_t data;
    SlaveConfigRead(inSlaveID, inAddress, (uint8_t*)&data, 2);
    return mHTONS(data);
}

// ----------------------------------------------------------------------------
void TaskBoardConfig(void* args)
{
    // Make sure that setup step was done, i.e. Shadow RAM is valid.
    if (!gIsBoardConfigInitDone)
    {
        CACIOModuleSetServerState(kMasterStatusState_NoConfig);
        PrintMessage("%s : %s - Warning: %s\n", __FILENAME__, __FUNCTION__, "Setup not completed");
        // Stop this tasks.
        vTaskSuspend(taskHandleBoardConfig);
    }

    boardWriteMessage_t configMessage;
    while (1)
    {
        xQueueReceive(BoardConfigWriteQueue, &configMessage, portMAX_DELAY);

        int status = kSuccess;
        if (configMessage.boardID == MasterID)
        {
            PrintMessage("%s : %s - Info: Writing to master\n", __FILENAME__, __FUNCTION__);

            /*// Store data to master.
            status = S25FL256PageWrite(configMessage.address, configMessage.data, configMessage.size);
            // Store new user CRC.
            status |= S25FL256PageWrite(kMasterConfigUser_CRC, (uint8_t*)&configMessage.crc, 2);
*/
            //FIXME: REIMPLEMENT FOR MASTER (I.E. DO NOT ERASE EVERYTIME).

        	//TODO: Serial read 4k sector
/*        	status = S25FL256PageRead(); // compare shadow ram data with flash data
        	if (status != kSuccess)
			{
				PrintMessage("%s : %s - Error: %s (master)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status));
			}

        	if (memcmp(gMasterConfigShadowRAM,)) // if not the same, erase and rewrite flash partition
        	{

        	}
*/
            S25FL256Erase4K(kPartition_1_FirstSector + kMasterConfigFactory_Magic);

            // Wait until erase is done
            do
            {
            	status = S25FL256BusyWait();
            }
            while (status == kError_FlashBusy);

            status = S25FL256PageWrite(kMasterConfigFactory_Magic, gMasterConfigShadowRAM + kMasterConfigFactory_Magic, kMasterConfigUser_End - kMasterConfigFactory_Magic);
            if (status != kSuccess)
            {
                PrintMessage("%s : %s - Error: %s (master)\n", __FILENAME__, __FUNCTION__, ParseErrorMessage(status));
            }
        }
        else
        {
            PrintMessage("%s : %s - Info: Writing to slave ID %d\n", __FILENAME__, __FUNCTION__, configMessage.boardID + 1);

            // Store data to slave.
            status = PCA9500EEPROMPageWrite(configMessage.boardID, configMessage.address, configMessage.data, configMessage.size);
            // Store new user CRC.
            status |= PCA9500EEPROMPageWrite(configMessage.boardID, kSlaveConfig_CRC, (uint8_t*)&configMessage.crc, 2);
        }

        if (status != kSuccess)
        {
            PrintMessage("%s : %s - Error: %s\n", __FILENAME__, __FUNCTION__, "Write error");
        }
    }
}

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
#include <string.h>
#include <stdbool.h>

// 3rd party includes.
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

// Drivers includes.
#include "s25fl256.h"
#include "pca9500.h"
#include "adc128d818.h"

// Lib includes.
#include "boardconfig.h"
#include "boardconfigerror.h"
#include "flashconfig.h"
#include "uprint.h"
#include "utils.h"
#include "crc16.h"
#include "version.h"

// TODO: See boardconfig.c in devpic16lf1718, should be the same file.

// ----------------------------------------------------------------------------
// Private constants.
// Map drivers constants.
#define kMasterPageSize kS25FL256_PageSize256B
#define kMasterConfigTotalSize kMasterConfig_End
#define kSlavePageSize kPCA9500_EEPROMPageSize
#define kSlaveConfigTotalSize kPCA9500_EEPROMTotSize

// Private variables.
static bool gIsBoardConfigInitDone = false;
static xQueueHandle BoardConfigWriteQueue;
static xSemaphoreHandle gBoardConfigMutex;
static uint16_t masterPageSize;

// Shadow RAM.
static uint8_t gMasterConfigShadowRAM[kMasterConfigTotalSize];
static uint8_t gSlaveConfigShadowRAM[SlaveID_Max][kSlaveConfigTotalSize];

// Private structs.
typedef struct
{
    uint8_t boardID;
    uint8_t address;
    uint8_t size;
    uint8_t data[8];
    uint16_t crc;
} boardWriteMessage_t;

// Public variables.
extern xTaskHandle taskHandleBoardConfig;

// ----------------------------------------------------------------------------
static void MasterConfigSave(void)
{
    // Compute CRC.
    uint16_t crc;
    // TODO: REMOVE, use config tool

    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)) = mHTONS(crc);

    S25FL256Erase64K(kMasterConfig_Magic);
    // FIXME: Add sync here - erase command takes too long flash still busy after.
    DelayLoop(100000000);

    int status;
    // Write everything to non-volatile memory.
    status = S25FL256PageWrite(kMasterConfig_Magic, gMasterConfigShadowRAM + kMasterConfig_Magic, kMasterConfigTotalSize);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s (master)\n", __FUNCTION__, ParseErrorMessage(status));
    }
}

// ----------------------------------------------------------------------------
static void SlaveConfigSave(uint8_t inSlaveID)
{
    uint16_t crc;

    // Compute CRC.
    crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kBoardConfig_SlaveUserSize);
    *((uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC)) = mHTONS(crc);

    int status;

    // Write everything to non-volatile memory.
    for (uint16_t addressIdx = kSlaveConfig_Magic; addressIdx < kSlaveConfig_Reflash; addressIdx += kSlavePageSize)
    {
        status = PCA9500EEPROMPageWrite(inSlaveID, addressIdx, gSlaveConfigShadowRAM[inSlaveID] + addressIdx, kSlavePageSize);
        if (status != 0)
        {
            PrintMessage("%s - Error: %s (slave ID %d)\n", __FUNCTION__, ParseErrorMessage(status), inSlaveID + 1);
        }
    }
}

// ----------------------------------------------------------------------------
int MasterConfigInit(uint8_t inSlaveCount)
{
    // Setup HAL.
    int status = S25FL256Init();
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
        return status;
    }

    // Read non-volatile memory in Shadow RAM.
    masterPageSize = S25FL256GetPageSize();
    for (uint32_t addressIdx = kPartition_1_FirstSector; addressIdx < kMasterConfigTotalSize; addressIdx += masterPageSize)
    {
        status = S25FL256PageRead(addressIdx, gMasterConfigShadowRAM + addressIdx, masterPageSize);
        if (status != 0)
        {
            return status;
        }
    }

    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    bool isConfigValid = false;

    // Validate config.
    if (*(uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic) != mHTONS(kBoardConfig_MagicNumber))
    {
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Bad magic");
        isConfigValid = false;
    }
    else if (gMasterConfigShadowRAM[kMasterConfig_FlashLayout] != kVersionConfigLayout)
    {
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Unsupported layout");
        isConfigValid = false;
    }
    else if (gMasterConfigShadowRAM[kMasterConfig_SlavesStatus] != inSlaveCount)
    {
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Slave config mismatch");
        isConfigValid = false;
    }
    else if (mHTONS(crc) != *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)))
    {
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Bad CRC");
        isConfigValid = false;
    }
    else
    {
        isConfigValid = true;
    }

    // Overwrite shadow contents if non-volatile memory values were not valid.
    if (!isConfigValid)
    {
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Forcing default values");
        MasterConfigSetDefaults(inSlaveCount, gMasterConfigShadowRAM);
        // Write changes back to non-volatile memory.
        MasterConfigSave();
    }

    if ((gBoardConfigMutex = xSemaphoreCreateMutex()) == NULL)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Can't create gBoardConfigMutex");
        return -1;
    }

    if ((BoardConfigWriteQueue = xQueueCreate(kBoardConfig_MasterWriteQueueLenght, sizeof(boardWriteMessage_t))) == NULL)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Can't create BoardConfigWriteQueue");
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

    PrintMessage("%s - Info: Scanning attached slave modules...\n", __FUNCTION__);

    for (uint8_t slaveIdx = 0; slaveIdx < SlaveID_Max; slaveIdx ++)
    {
        // Check if the module responds to the command. If successful, it initialize it with all I/Os to 0.
        int status = PCA9500IOExpanderSetPort(slaveIdx, 0x00);

        if (status != 0)
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
                PrintMessage("%s - Warning: Inconsistent slave addressing. (Slave ID %d not present?).\n", __FUNCTION__, slaveIdx + 1);
            }
            PrintMessage("%s - Info: Found slave ID %d.\n", __FUNCTION__, slaveIdx + 1);
            // Slave discovered. Get slave ID.
            *(outSlaveAddressMap) ++ = slaveIdx;
            // Use address index to count discovered slaves.
            slavesStatus |= 0x01 << slaveIdx;
            slavesCount ++;
        }
    }

    PrintMessage("%s - Info: Discovered %d slave modules.\n", __FUNCTION__, slavesCount);

    return slavesStatus;
}

// ----------------------------------------------------------------------------
int SlaveConfigInit(uint8_t inSlaveID)
{
    int status = 0;
    // Read non-volatile memory in Shadow RAM.
    for (uint16_t addressIdx = 0; addressIdx < kSlaveConfigTotalSize; addressIdx += kSlavePageSize)
    {
        status = PCA9500EEPROMPageRead(inSlaveID, (uint8_t)addressIdx, gSlaveConfigShadowRAM[inSlaveID] + addressIdx, kSlavePageSize);
        if (status != 0)
        {
            PrintMessage("%s - Error: %s (slave ID %d)\n", __FUNCTION__, ParseErrorMessage(status), inSlaveID + 1);
            return status;
        }
    }

    // Validate settings.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kBoardConfig_SlaveUserSize);
    bool isConfigValid = false;

    if (*(uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic) != mHTONS(kBoardConfig_MagicNumber))
    {
        PrintMessage("%s - Warning: %s (slave ID %d)\n", __FUNCTION__, "Wrong magic value", inSlaveID + 1);
        isConfigValid = false;
    }
    else if (gSlaveConfigShadowRAM[inSlaveID][kSlaveConfig_EEPROMLayout] != kVersionConfigLayout)
    {
        PrintMessage("%s - Warning: %s (slave ID %d)\n", __FUNCTION__, "Unsupported config layout", inSlaveID + 1);
        isConfigValid = false;
    }
    else if (*((uint16_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_CRC)) != mHTONS(crc))
    {
        PrintMessage("%s - Warning: %s (slave ID %d)\n", __FUNCTION__, "Bad CRC", inSlaveID + 1);
        isConfigValid = false;
    }
    else
    {
        isConfigValid = true;
    }

    // Overwrite shadow contents if non-volatile memory values were not valid.
    if (!isConfigValid)
    {
        PrintMessage("%s - Warning: %s (slave ID %d)\n", __FUNCTION__, "Forcing default values", inSlaveID + 1);
        SlaveConfigSetDefaults(inSlaveID, gSlaveConfigShadowRAM[inSlaveID]);
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
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Initialization not complete");
        return;
    }

    // Validate address and size arguments.
    if (inSize > kMasterPageSize)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Bad page size");
        return;
    }
    if ((inAddress < kMasterConfig_IPAddr) || (inAddress >= kMasterConfigTotalSize))
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Can't write outside config section");
        return;
    }
    if (((inAddress / kMasterPageSize) != ((inAddress + inSize) / kMasterPageSize)) && ((inAddress + inSize) % kMasterPageSize))
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Can't write across pages");
        return;
    }

    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);

    // Perform the write in Shadow RAM
    memcpy(gMasterConfigShadowRAM + inAddress, inData, inSize);

    // Recompute the CRC
    crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)) = mHTONS(crc);

    // Send message to write task to commit changes to non-volatile memory.
    configMessage.boardID = MasterID;
    configMessage.address = inAddress;
    configMessage.size = inSize;
    memcpy(configMessage.data, inData, inSize);
    memcpy(&configMessage.crc, gMasterConfigShadowRAM + kMasterConfig_CRC, 2);
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
        PrintMessage("%s - Warning: %s (slave ID %d)\n", __FUNCTION__, "Initialization not complete", inSlaveID + 1);
        return;
    }

    // Validate address and size arguments.
    if (inSize > kSlavePageSize)
    {
        PrintMessage("%s - Error: %s (slave ID %d)\n", __FUNCTION__, "Bad page size", inSlaveID + 1);
        return;
    }
    if ((inAddress < kSlaveConfig_VersionMajor) || (inAddress >= kSlaveConfig_Reflash))
    {
        PrintMessage("%s - Error: %s (slave ID %d)\n", __FUNCTION__, "Can't write outside config section", inSlaveID + 1);
        return;
    }
    if (((inAddress / kSlavePageSize) != ((inAddress + inSize) / kSlavePageSize)) && ((inAddress + inSize) % kSlavePageSize))
    {
        PrintMessage("%s - Error: %s (slave ID %d)\n", __FUNCTION__, "Can't write across pages", inSlaveID + 1);
        return;
    }

    xSemaphoreTake(gBoardConfigMutex, portMAX_DELAY);

    // Perform the write in Shadow RAM
    memcpy(gSlaveConfigShadowRAM[inSlaveID] + inAddress, inData, inSize);

    // Recompute the CRC
    crc = CRC16ComputeCRC((uint8_t*)(gSlaveConfigShadowRAM[inSlaveID] + kSlaveConfig_Magic), kBoardConfig_SlaveUserSize);
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
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Initialization not complete");
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
        PrintMessage("%s - Warning: %s\n (slave ID %d)", __FUNCTION__, "Initialization not complete", inSlaveID + 1);
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
        PrintMessage("%s - Warning: %s\n", __FUNCTION__, "Setup not completed");
        // Stop this tasks.
        vTaskSuspend(taskHandleBoardConfig);
    }

    boardWriteMessage_t configMessage;
    while (1)
    {
        xQueueReceive(BoardConfigWriteQueue, &configMessage, portMAX_DELAY);

        int status = 0;
        if (configMessage.boardID == MasterID)
        {
#if kDebugEnabled == 1
            PrintMessage("%s - Info: Writing to master\n", __FUNCTION__);
#endif
            // Copy config to shadow RAM.
            uint8_t masterConfigCache[kMasterConfigTotalSize] = { 0 };
            status = S25FL256PageRead(kPartition_1_FirstSector, masterConfigCache, kMasterConfigTotalSize);
            // Erase sector.
            S25FL256Erase64K(kPartition_1_FirstSector);
            // Wait until erase is done TODO: Move this into the erase function...
            do
            {
                status = S25FL256BusyWait();
            }
            while (status == kError_FlashBusy);

            // Write data cache.
            memcpy(masterConfigCache + configMessage.address, configMessage.data, configMessage.size);
            // Write CRC cache.
            memcpy(masterConfigCache + kMasterConfig_CRC, (uint8_t*)&configMessage.crc, 2);
            // Store config.
            status |= S25FL256PageWrite(kPartition_1_FirstSector, masterConfigCache, kMasterConfigTotalSize);
        }
        else
        {
            PrintMessage("%s - Info: Writing to slave ID %d\n", __FUNCTION__, configMessage.boardID + 1);

            // Store data to slave.
            status = PCA9500EEPROMPageWrite(configMessage.boardID, configMessage.address, configMessage.data, configMessage.size);
            // Store new user CRC.
            status |= PCA9500EEPROMPageWrite(configMessage.boardID, kSlaveConfig_CRC, (uint8_t*)&configMessage.crc, 2);
        }

        if (status != 0)
        {
            PrintMessage("%s - Error: %s\n", __FUNCTION__, "Write error");
        }
    }
}

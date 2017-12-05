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

// Lib includes.
#include "iomodconfig.h"
#include "boardconfig.h"
#include "crc16.h"
#include "version.h"

// ----------------------------------------------------------------------------
// Private variables.
static bool gIsBoardConfigInitDone = false;

// Shadow RAM.
static uint8_t gMasterConfigShadowRAM[kMasterConfigTotalSize];

// ----------------------------------------------------------------------------
static int MasterConfigCommit(void)
{
    // TODO: REMOVE, use config tool
    // Compute CRC.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)) = mHTONS(crc);

    int status = 0;
    // Write shadow RAM to NVM.
    for (uint32_t addressIndex = kMasterConfig_Magic; addressIndex < kMasterConfigTotalSize; addressIndex += kMasterConfigPageSize)
    {
        status = mMasterConfigPageWrite(addressIndex, gMasterConfigShadowRAM + addressIndex, kMasterConfigTotalSize);
        if (status != 0)
        {
            mBoardConfigPrintDriverError("Commit failed");
            return status;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
int MasterConfigInit(uint8_t inSlaveCount)
{
    // Setup HAL.
    int status = mMasterConfigSetup();
    if (status != 0)
    {
        mBoardConfigPrintDriverError("Status=%04x", status);
        return status;
    }

    // Read NVM in shadow RAM.
    for (uint32_t addressIndex = kMasterConfigStartAddress; addressIndex < kMasterConfigTotalSize; addressIndex += kMasterConfigPageSize)
    {
        status = mMasterConfigPageRead(addressIndex, gMasterConfigShadowRAM + addressIndex, kMasterConfigPageSize);
        if (status != 0)
        {
            mBoardConfigPrintDriverError("Status=%04x", status);
            return status;
        }
    }

    // Validate config retrieved from NVM.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    bool isConfigValid = false;

    // Validate config.
    if (*(uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic) != mHTONS(kBoardConfig_MagicNumber))
    {
        mBoardConfigPrintWarning("Bad magic");
        isConfigValid = false;
    }
    else if (gMasterConfigShadowRAM[kMasterConfig_FlashLayout] != kVersionConfigLayout)
    {
        mBoardConfigPrintWarning("Unsupported layout");
        isConfigValid = false;
    }
    else if (mHTONS(crc) != *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)))
    {
        mBoardConfigPrintWarning("Bad CRC");
        isConfigValid = false;
    }
    else
    {
        mBoardConfigPrintInfo("Config valid");
        isConfigValid = true;
    }

    // Overwrite shadow contents if NVM values were not valid.
    if (!isConfigValid)
    {
        mBoardConfigPrintInfo("Forcing default values");
        mMasterConfigSetDefaults(inSlaveCount, gMasterConfigShadowRAM);
        // Write changes back to NVM.
        status = MasterConfigCommit();
    }

    // Finished boot-time initialization.
    gIsBoardConfigInitDone = true;

    return status;
}

// ----------------------------------------------------------------------------
void MasterConfigWrite(uint8_t inAddress, uint8_t* inData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        mBoardConfigPrintInitError("Init incomplete");
        return;
    }

    // Validate address and size arguments.
    if (inSize > kMasterConfigPageSize)
    {
        mBoardConfigPrintPageError("Bad page size");
        return;
    }

    if (((inAddress / kMasterConfigPageSize) != ((inAddress + inSize) / kMasterConfigPageSize)) && ((inAddress + inSize) % kMasterConfigPageSize))
    {
        mBoardConfigPrintPageError("Write across pages");
        return;
    }

    mBoardConfigLock();

    // Perform the write in shadow RAM
    memcpy(gMasterConfigShadowRAM + inAddress, inData, inSize);

    // Recompute the CRC
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gMasterConfigShadowRAM + kMasterConfig_Magic), kBoardConfig_MasterUserSize);
    *((uint16_t*)(gMasterConfigShadowRAM + kMasterConfig_CRC)) = mHTONS(crc);

    mMasterConfigWrite(inAddress, inData, inSize, crc);

    mBoardConfigUnlock();
}

// ----------------------------------------------------------------------------
void MasterConfigRead(uint8_t inAddress, uint8_t* outData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        mBoardConfigPrintInitError("Init incomplete");
        memset(outData, 0, inSize);
        return;
    }

    mBoardConfigLock();

    // Perform the read from shadow RAM.
    memcpy(outData, gMasterConfigShadowRAM + inAddress, inSize);

    mBoardConfigUnlock();
}

// ----------------------------------------------------------------------------
uint8_t MasterConfigReadByte(uint8_t inAddress)
{
    uint8_t data;
    MasterConfigRead(inAddress, &data, 1);

    return data;
}

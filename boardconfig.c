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
#include "ufs.h" // TODO: Remove.

// ----------------------------------------------------------------------------
// Private variables.
static bool gIsBoardConfigInitDone = false;

// Shadow RAM.
static uint8_t gBoardConfigShadowRAM[kBoardConfigTotalSize];

// ----------------------------------------------------------------------------
static int BoardConfigCommit(void)
{
    // Erase sector.
    int status = S25FL256Erase4K(kPartition_Config_FirstSector);

    // TODO: REMOVE, use config tool
    // Compute CRCs.
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gBoardConfigShadowRAM + kBoardConfig_FactoryAddressOffset), kBoardConfig_FactorySize);
    *((uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_Factory_CRC)) = mHTONS(crc);
    crc = CRC16ComputeCRC((uint8_t*)(gBoardConfigShadowRAM + kBoardConfig_UserAddressOffset), kBoardConfig_UserSize);
    *((uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_User_CRC)) = mHTONS(crc);

    // Write shadow RAM to NVM.
    for (uint32_t addressIndex = kBoardConfig_Factory_Magic; addressIndex < kBoardConfigTotalSize; addressIndex += kBoardConfigPageSize)
    {
        status = mBoardConfigPageWrite(addressIndex, gBoardConfigShadowRAM + addressIndex, kBoardConfigPageSize);
        if (status != 0)
        {
            mBoardConfigPrintDriverError("Commit failed");
            return status;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
int BoardConfigResetFactory(void)
{
    mBoardConfigPrintInfo("Force defaults");
    // Load defaults in shadow RAM.
    mBoardConfigSetDefaults(gBoardConfigShadowRAM);
    // Commit changes to NVM.
    return BoardConfigCommit();
}

// ----------------------------------------------------------------------------
int BoardConfigInit(void)
{
    // Setup HAL.
    int status = mBoardConfigSetup();
    if (status != 0)
    {
        mBoardConfigPrintDriverError("Status=%04x", status);
        return status;
    }

    // Read NVM in shadow RAM.
    for (uint32_t addressIndex = kBoardConfigStartAddress; addressIndex < kBoardConfigTotalSize; addressIndex += kBoardConfigPageSize)
    {
        status = mBoardConfigPageRead(addressIndex, gBoardConfigShadowRAM + addressIndex, kBoardConfigPageSize);
        if (status != 0)
        {
            mBoardConfigPrintDriverError("Status=%04x", status);
            return status;
        }
    }

    // Validate config retrieved from NVM.
    uint16_t factoryCRC = CRC16ComputeCRC((uint8_t*)(gBoardConfigShadowRAM + kBoardConfig_FactoryAddressOffset), kBoardConfig_FactorySize);
    uint16_t userCRC = CRC16ComputeCRC((uint8_t*)(gBoardConfigShadowRAM + kBoardConfig_UserAddressOffset), kBoardConfig_UserSize);

    // Validate config.
    bool isConfigValid = false;
    if (*(uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_Factory_Magic) != mHTONS(kBoardConfig_MagicNumber))
    {
        mBoardConfigPrintWarning("Bad magic");
        isConfigValid = false;
    }
    else if (gBoardConfigShadowRAM[kBoardConfig_Factory_FlashLayout] != kVersionConfigLayout)
    {
        mBoardConfigPrintWarning("Bad layout");
        isConfigValid = false;
    }
    else if (mHTONS(factoryCRC) != *((uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_Factory_CRC)))
    {
        mBoardConfigPrintWarning("Bad factory CRC");
        isConfigValid = false;
    }
    else if (mHTONS(userCRC) != *((uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_User_CRC)))
    {
        mBoardConfigPrintWarning("Bad user CRC");
        isConfigValid = false;
    }
    else
    {
        mBoardConfigPrintInfo("Loaded config");
        isConfigValid = true;
    }

    // Overwrite shadow contents if NVM values were not valid.
    if (!isConfigValid)
    {
        mBoardConfigPrintInfo("Force defaults");
        // Load defaults in shadow RAM.
        mBoardConfigSetDefaults(gBoardConfigShadowRAM);
        // Commit changes to NVM.
        status = BoardConfigCommit();
    }

    // Finished boot-time initialization.
    gIsBoardConfigInitDone = true;

    return status;
}

// ----------------------------------------------------------------------------
int BoardConfigWrite(uint8_t inAddress, uint8_t* inData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        mBoardConfigPrintInitError("Init incomplete");
        return -1;
    }

    // Validate address and size arguments.
    if (inSize > kBoardConfigPageSize)
    {
        mBoardConfigPrintPageError("Bad page size");
        return -1;
    }

    if (((inAddress / kBoardConfigPageSize) != ((inAddress + inSize) / kBoardConfigPageSize)) && ((inAddress + inSize) % kBoardConfigPageSize))
    {
        mBoardConfigPrintPageError("Write across pages");
        return -1;
    }

    mBoardConfigLock();

    // Perform the write in shadow RAM
    memcpy(gBoardConfigShadowRAM + inAddress, inData, inSize);

    // Recompute the CRC
    uint16_t crc = CRC16ComputeCRC((uint8_t*)(gBoardConfigShadowRAM + kBoardConfig_UserAddressOffset), kBoardConfig_UserSize);
    *((uint16_t*)(gBoardConfigShadowRAM + kBoardConfig_User_CRC)) = mHTONS(crc);

    mBoardConfigWrite(inAddress, inData, inSize, crc);

    mBoardConfigUnlock();

    return 0;
}

// ----------------------------------------------------------------------------
int BoardConfigRead(uint8_t inAddress, uint8_t* outData, uint8_t inSize)
{
    if (!gIsBoardConfigInitDone)
    {
        mBoardConfigPrintInitError("Init incomplete");
        memset(outData, 0, inSize);
        return -1;
    }

    mBoardConfigLock();

    // Perform the read from shadow RAM.
    memcpy(outData, gBoardConfigShadowRAM + inAddress, inSize);

    mBoardConfigUnlock();

    return 0;
}

// ----------------------------------------------------------------------------
uint8_t BoardConfigReadByte(uint8_t inAddress)
{
    uint8_t data;
    BoardConfigRead(inAddress, &data, 1);

    return data;
}

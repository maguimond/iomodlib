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

// Standard includes.
#include <string.h>

// HAL includes.
#include "s25fl256.h"

// Drivers includes.
#include "spi.h"

// Lib includes.
#include "uprint.h" // TODO: remove prints, its a lib.
#include "boardconfigerror.h"

// ----------------------------------------------------------------------------
// Private variables.
static S25FL256_t gS25FL256;
static bool gIsMasterConfigDriverInitialized;

// Private constants.
#define kS25FL256Timeout 0x10000

// Private macros.
#define mValidateParam(inParam) if (!inParam) { return -1; }

// ----------------------------------------------------------------------------
static int S25FL256ReadInfo(void)
{
    uint8_t spiData[6] = { 0 };

    int status = SPI3ReadRegister(kS25FL256_RegisterRDID, spiData, 6);
    if (status == 0)
    {
        gS25FL256.manufacturerID = spiData[S25FL256_RegisterRDID_ManufacturerID];
        gS25FL256.memoryType = spiData[S25FL256_RegisterRDID_MemoryType];
        gS25FL256.capacity = spiData[S25FL256_RegisterRDID_Capacity];
        gS25FL256.IDCFI = spiData[S25FL256_RegisterRDID_IDCFI];
        gS25FL256.sectorArchitecture = spiData[S25FL256_RegisterRDID_SectorArchitecture];
        gS25FL256.familyID = spiData[S25FL256_RegisterRDID_FamilyID];

        if (gS25FL256.manufacturerID != 0x01)
        {
            status = kError_UnsupportedDevice;
        }

        if (gS25FL256.sectorArchitecture == kS25FL256_RegisterRDID_SectorArchitecture256KB)
        {
            gS25FL256.pageSize = kS25FL256_PageSize512B;
        }
        else if (kS25FL256_RegisterRDID_SectorArchitecture64KB)
        {
            gS25FL256.pageSize = kS25FL256_PageSize256B;
        }
        else
        {
            status = kError_UnsupportedDevice;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256BusyWait(void)
{
    uint8_t spiData = 0;
    int status = SPI3ReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
    }
    else
    {
        uint32_t timeout = 0;
        while (spiData & S25FL256_RegisterRDSR1_WIP)
        {
            status = SPI3ReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1);

            if ((timeout ++) > kS25FL256Timeout)
            {
                status = kError_FlashBusy;
                break;
            }

            if (status != 0)
            {
                PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
                break;
            }
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
static int S25FL256CheckStatus(uint8_t inStatusFlag)
{
    uint8_t spiData = 0;
    int status = SPI3ReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
    }
    else
    {
        uint32_t timeout = 0;
        while (spiData != 0)
        {
            if (!(spiData & S25FL256_RegisterRDSR1_WIP))
            {
                if (spiData & S25FL256_RegisterRDSR1_WEL)
                {
                    // If write enable is latched and flash is not busy, we are ok to continue for a write or an erase.
                    break;
                }
                else
                {
                    if (inStatusFlag == S25FL256_RegisterRDSR1_WEL)
                    {
                        status = kError_FlashWNE;
                    }
                }
            }

            if (spiData & S25FL256_RegisterRDSR1_E_ERR)
            {
                status = kError_FlashErase;
            }

            if (spiData & S25FL256_RegisterRDSR1_P_ERR)
            {
                status = kError_FlashProg;
            }

            if ((timeout ++) > kS25FL256Timeout)
            {
                status = kError_FlashBusy;
            }

            if (status != 0)
            {
                PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
                break;
            }

            status = SPI3ReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1);
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
static int S25FL256WriteEnable(void)
{
    uint8_t data;
    int status = SPI3WriteRegister(kS25FL256_RegisterWREN, &data, 0, kSPIPacketIsComplete);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
    }

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256Init(void)
{
    int status = 0;

    if (!gIsMasterConfigDriverInitialized)
    {
        // Setup SPI3 driver.
        SPI3Setup();

        status = S25FL256BusyWait();
        if (status == 0)
        {
            // Read device info.
            status = S25FL256ReadInfo();
        }
    }

    gIsMasterConfigDriverInitialized = true;

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256Erase4K(uint32_t inAddress)
{
    mValidateParam(inAddress >= 0 && inAddress <= kS25FL256_4KSectorLast);

    int status = S25FL256WriteEnable();
    status = S25FL256CheckStatus(S25FL256_RegisterRDSR1_WEL);

    uint8_t sectorAddress[4] = { 0 };
    sectorAddress[0] = inAddress >> 24;
    sectorAddress[1] = inAddress >> 16;
    sectorAddress[2] = inAddress >> 8;
    sectorAddress[3] = inAddress & 0xFF;

    status = SPI3WriteRegister(kS25FL256_Register4P4E, sectorAddress, sizeof(sectorAddress), kSPIPacketIsComplete);

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256Erase64K(uint32_t inAddress)
{
    int status = S25FL256WriteEnable();
    status = S25FL256CheckStatus(S25FL256_RegisterRDSR1_WEL);

    uint8_t sectorAddress[4] = { 0 };
    sectorAddress[0] = inAddress >> 24;
    sectorAddress[1] = inAddress >> 16;
    sectorAddress[2] = inAddress >> 8;
    sectorAddress[3] = inAddress & 0xFF;

    status = SPI3WriteRegister(kS25FL256_Register4SE, sectorAddress, sizeof(sectorAddress), kSPIPacketIsComplete);

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256PageWrite(uint32_t inAddress, uint8_t* inData, uint32_t inSize)
{
    // FIXME: S25FL256CheckStatus does not do the job.
    S25FL256BusyWait();
    int status = 0;
    if (inSize <= gS25FL256.pageSize)
    {
        uint8_t sectorAddress[4] = { 0 };
        sectorAddress[0] = inAddress >> 24;
        sectorAddress[1] = inAddress >> 16;
        sectorAddress[2] = inAddress >> 8;
        sectorAddress[3] = inAddress & 0xFF;

        status = S25FL256WriteEnable();
        status = S25FL256CheckStatus(S25FL256_RegisterRDSR1_WEL);

        // Send address.
        status = SPI3WriteRegister(kS25FL256_Register4PP, sectorAddress, sizeof(sectorAddress), kSPIPacketIsIncomplete);
        if (status != 0)
        {
            return status;
        }
        // Send data.
        status = SPI3WriteRegister(kS25FL256_Register4PP, inData, inSize, kSPIPacketIsComplete);
        if (status != 0)
        {
            return status;
        }
    }
    else
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, "Out of boundaries");
    }

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256PageRead(uint32_t inAddress, uint8_t* outData, uint32_t inSize)
{
    uint8_t sectorAddress[4] = { 0 };
    sectorAddress[0] = inAddress >> 24;
    sectorAddress[1] = inAddress >> 16;
    sectorAddress[2] = inAddress >> 8;
    sectorAddress[3] = inAddress & 0xFF;

    S25FL256BusyWait();
    // Send address.
    int status = SPI3WriteRegister(kS25FL256_Register4READ, sectorAddress, sizeof(sectorAddress), kSPIPacketIsIncomplete);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
    }

    status = SPI3ReadRegister(kS25FL256_Register4READ, outData, inSize);
    if (status != 0)
    {
        PrintMessage("%s - Error: %s\n", __FUNCTION__, ParseErrorMessage(status));
    }

    return status;
}

// ----------------------------------------------------------------------------
uint16_t S25FL256GetPageSize(void)
{
    return gS25FL256.pageSize;
}

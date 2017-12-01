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
#include "string.h"

// Lib includes.
#include "iomodutils.h"

// Driver includes.
#include "s25fl256.h"

// Bus includes.
#include "spi.h"

// ----------------------------------------------------------------------------
// Private variables.
static S25FL256_t gS25FL256;
static bool gIsMasterConfigDriverInitialized;

// Private constants.
#define kS25FL256Timeout 0x10000

// ----------------------------------------------------------------------------
static int S25FL256ReadInfo(void)
{
    uint8_t spiData[6] = { 0 };

    int status = SPIReadRegister(kS25FL256_RegisterRDID, spiData, 6);
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
            status = kIOMod_DriverError | kIOModError_UnsupportedDevice;
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
            status = kIOMod_DriverError | kIOModError_UnsupportedDevice;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256BusyWait(void)
{
    uint8_t spiData = 0;
    mIOValidateBus(SPIReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1));

    int status = 0;
    uint32_t timeout = 0;
    while (spiData & S25FL256_RegisterRDSR1_WIP)
    {
        mIOValidateBus(SPIReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1));

        if ((timeout ++) > kS25FL256Timeout)
        {
            status = kIOMod_DriverError | kIOModError_FlashBusy;
            break;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
static int S25FL256CheckStatus(uint8_t inStatusFlag)
{
    uint8_t spiData = 0;
    mIOValidateBus(SPIReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1));

    int status = 0;
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
                    status = kIOMod_DriverError | kIOModError_FlashWNE;
                    break;
                }
            }
        }

        if (spiData & S25FL256_RegisterRDSR1_E_ERR)
        {
            status = kIOMod_DriverError | kIOModError_FlashErase;
            break;
        }

        if (spiData & S25FL256_RegisterRDSR1_P_ERR)
        {
            status = kIOMod_DriverError | kIOModError_FlashProg;
            break;
        }

        if ((timeout ++) > kS25FL256Timeout)
        {
            status = kIOMod_DriverError | kIOModError_FlashBusy;
            break;
        }

        mIOValidateBus(SPIReadRegister(kS25FL256_RegisterRDSR1, &spiData, 1));
    }

    return status;
}

// ----------------------------------------------------------------------------
static int S25FL256WriteEnable(void)
{
    uint8_t data;
    mIOValidateBus(SPIWriteRegister(kS25FL256_RegisterWREN, &data, 0, kSPIPacketIsComplete));
    return 0;
}

// ----------------------------------------------------------------------------
int S25FL256Init(void)
{
    int status = 0;

    if (!gIsMasterConfigDriverInitialized)
    {
        // Setup SPI driver.
        SPISetup();

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
    mIOAssertArg(inAddress >= 0 && inAddress <= kS25FL256_4KSectorLast);

    int status = S25FL256WriteEnable();
    status = S25FL256CheckStatus(S25FL256_RegisterRDSR1_WEL);

    uint8_t sectorAddress[4] = { 0 };
    sectorAddress[0] = inAddress >> 24;
    sectorAddress[1] = inAddress >> 16;
    sectorAddress[2] = inAddress >> 8;
    sectorAddress[3] = inAddress & 0xFF;

    mIOValidateBus(SPIWriteRegister(kS25FL256_Register4P4E, sectorAddress, sizeof(sectorAddress), kSPIPacketIsComplete));

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

    mIOValidateBus(SPIWriteRegister(kS25FL256_Register4SE, sectorAddress, sizeof(sectorAddress), kSPIPacketIsComplete));

    // Wait until erase is done. TODO: Add timeout.
    do
    {
        status = S25FL256BusyWait();
    }
    while (status == kIOModError_FlashBusy);

    return status;
}

// ----------------------------------------------------------------------------
int S25FL256PageWrite(uint32_t inAddress, uint8_t* inData, uint32_t inSize)
{
    mIOAssertArg(inSize <= gS25FL256.pageSize);

    // FIXME: S25FL256CheckStatus does not do the job.
    S25FL256BusyWait();

    uint8_t sectorAddress[4] = { 0 };
    sectorAddress[0] = inAddress >> 24;
    sectorAddress[1] = inAddress >> 16;
    sectorAddress[2] = inAddress >> 8;
    sectorAddress[3] = inAddress & 0xFF;

    int status = S25FL256WriteEnable();
    status |= S25FL256CheckStatus(S25FL256_RegisterRDSR1_WEL);

    // Send address.
    mIOValidateBus(SPIWriteRegister(kS25FL256_Register4PP, sectorAddress, sizeof(sectorAddress), kSPIPacketIsIncomplete));
    // Send data.
    mIOValidateBus(SPIWriteRegister(kS25FL256_Register4PP, inData, inSize, kSPIPacketIsComplete));

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
    mIOValidateBus(SPIWriteRegister(kS25FL256_Register4READ, sectorAddress, sizeof(sectorAddress), kSPIPacketIsIncomplete));

    mIOValidateBus(SPIReadRegister(kS25FL256_Register4READ, outData, inSize));

    return 0;
}

// ----------------------------------------------------------------------------
uint16_t S25FL256GetPageSize(void)
{
    return gS25FL256.pageSize;
}

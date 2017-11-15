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

// Lib includes.
#include "iomod.h"
#include "boardconfig.h"

// Drivers includes.
#include "adc128d818.h"
#include "USP10973.h"

// ----------------------------------------------------------------------------
// Private variables.
IOMod_t gIOMod;
static uint8_t gADCI2CAddressTable[kADC128D818_MaxAddresses];

// ----------------------------------------------------------------------------
uint8_t IOModGetADCAddress(uint8_t inSlaveID)
{
    // TODO: Adapt when board is revised to support at least 6 ADC addresses: https://github.com/GeniALE/CarteAcquisitionControle/issues/41.
    // Set local devices addresses.
    uint8_t adcAddress;
    switch (inSlaveID)
    {
        case SlaveID1:
            adcAddress = kADC128D818_SlaveAddress1;
            break;
        case SlaveID2:
            adcAddress = kADC128D818_SlaveAddress2;
            break;
        case SlaveID3:
            adcAddress = kADC128D818_SlaveAddress3;
            break;
        case SlaveID4:
            adcAddress = kADC128D818_SlaveAddress4;
            break;
        case SlaveID5:
            adcAddress = kADC128D818_SlaveAddress5;
            break;
        case SlaveID6:
            adcAddress = kADC128D818_SlaveAddress6;
            break;
        default:
            adcAddress = kADC128D818_SlaveAddress7;
            break;
    }

    return adcAddress;
}

// ----------------------------------------------------------------------------
int IOModADCInit(uint8_t inSlaveID)
{
    // Get the slave ADC address.
    gADCI2CAddressTable[inSlaveID] = IOModGetADCAddress(inSlaveID);
    // Initialize ADC.
    mIOModValidateStatus(ADC128D818Init(gADCI2CAddressTable[inSlaveID]));
    // Start ADC continuous conversions.
    mIOModValidateStatus(ADC128D818StartConversion(gADCI2CAddressTable[inSlaveID], kADC128D818_ConversionRate_Continuous));

    // If we make it this far, its a success.
    return 0;
}

// ----------------------------------------------------------------------------
int IOModGetTemperature(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData)
{
    // TODO: Somehow from config determine on which channels are the temp sensors.

    int32_t temperature = 0;
    uint16_t adcRawData;

    mIOModValidateStatus(ADC128D818ReadChannel(gADCI2CAddressTable[inSlaveID], inChannelIdx, &adcRawData));

    // Convert thermistor value.
    if (!USP10973BetaComputeTemperature(adcRawData, &temperature))
    {
        gIOMod.io[inChannelIdx].status = kIOMod_Status_InvalidRange;
    }
    else
    {
        gIOMod.io[inChannelIdx].status = kIOMod_Status_Valid;
        *outADCData = temperature;
    }

    // If we make it this far, its a success.
    return 0;
}

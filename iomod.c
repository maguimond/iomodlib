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
#include "conversion.h"

// Drivers includes.
#include "adc128d818.h"
#include "usp10973.h"

// ----------------------------------------------------------------------------
// Private variables.
IOMod_t gIOMod;
static uint8_t gADCI2CAddressTable[kADC128D818_MaxAddresses];

// Private macros.
#define mIOModValidateDriverStatus(returnStatus) if (returnStatus != 0) { return kIOModPortStatus_DriverBusError; }

// ----------------------------------------------------------------------------
uint8_t IOModGetADCAddress(uint8_t inSlaveID)
{
    // TODO: Adapt when board is revised to support at least 6 ADC addresses: https://github.com/GeniALE/CarteAcquisitionControle/issues/41.
    // Set local devices addresses.
    uint8_t adcAddress;
    switch (inSlaveID)
    {
        case kBoardConfig_AuxID1:
            adcAddress = kADC128D818_SlaveAddress1;
            break;
        case kBoardConfig_AuxID2:
            adcAddress = kADC128D818_SlaveAddress2;
            break;
        case kBoardConfig_AuxID3:
            adcAddress = kADC128D818_SlaveAddress3;
            break;
        case kBoardConfig_AuxID4:
            adcAddress = kADC128D818_SlaveAddress4;
            break;
        case kBoardConfig_AuxID5:
            adcAddress = kADC128D818_SlaveAddress5;
            break;
        case kBoardConfig_AuxID6:
            adcAddress = kADC128D818_SlaveAddress6;
            break;
        default:
            adcAddress = kADC128D818_SlaveAddress7;
            break;
    }

    return adcAddress;
}

// ----------------------------------------------------------------------------
IOModPortStatus_t IOModADCInit(uint8_t inSlaveID)
{
    // Get the slave ADC address.
    gADCI2CAddressTable[inSlaveID] = IOModGetADCAddress(inSlaveID);
    // Initialize ADC.
    mIOModValidateDriverStatus(ADC128D818Init(gADCI2CAddressTable[inSlaveID]));
    // Start ADC continuous conversions.
    mIOModValidateDriverStatus(ADC128D818StartConversion(gADCI2CAddressTable[inSlaveID], kADC128D818_ConversionRate_Continuous));

    // If we make it this far, its a success.
    return 0;
}

// ----------------------------------------------------------------------------
IOModPortStatus_t IOModGetTemperature(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData)
{
    int status = 0;
    int32_t temperature = 0;
    uint16_t adcRawData;

    mIOModValidateDriverStatus(ADC128D818ReadChannel(gADCI2CAddressTable[inSlaveID], inChannelIdx, &adcRawData));

    // Convert thermistor value.
    if (USP10973BetaComputeTemperature(adcRawData, &temperature) != 0)
    {
        status = kIOModPortStatus_InvalidRange;
    }
    else
    {
        status = kIOModPortStatus_Valid;
        *outADCData = temperature;
    }

    // If we make it this far, its a success.
    return status;
}

// ----------------------------------------------------------------------------
IOModPortStatus_t IOModGetInternalTemperature(uint8_t inSlaveID, int32_t* outADCData)
{
    uint16_t adcRawData;

    mIOModValidateDriverStatus(ADC128D818SetMode(gADCI2CAddressTable[inSlaveID], kADC128D818_Mode_Temp));
    // TODO: Wait for conversion switch to stabilize.
    DelayUs(30000);
    mIOModValidateDriverStatus(ADC128D818ReadChannel(gADCI2CAddressTable[inSlaveID], kADC128D818_RegisterChannel7Read, &adcRawData));
    // TODO: Should put back in the config mode it was instead of forcing back single ended.
    mIOModValidateDriverStatus(ADC128D818SetMode(gADCI2CAddressTable[inSlaveID], kADC128D818_Mode_SingleEnded));

    // Shift right 3 as we pass from 12 bit to 9 bit result for internal temperature.
    adcRawData >>= 3;

    // 9-bit two's-complement conversions of the temperature (see datasheet p.28).
    // Note: Normal conversion x 1000 to add 3 float digits to be compatible with single-ended ADC conversions (however precision is 0.5C, thus 500).
    if (!(adcRawData & kADC128D818_TemperatureMSBMask))
    {
        // If temperature is positive.
        // If DOUT[MSb] = 0: + Temp(°C) = DOUT(dec) / 2.
        *outADCData = (adcRawData * 1000) >> 1;
    }
    else
    {
        // If temperature is negative.
        // If DOUT[MSb] = 1: – Temp(°C) = [29 – DOUT(dec)] / 2 .
        *outADCData = -(int32_t)(((0x0200 - adcRawData) * 1000) >> 1);
    }

    // If we make it this far, its a success.
    return 0;
}

// ----------------------------------------------------------------------------
IOModPortStatus_t IOModGetCurrent(uint8_t inSlaveID, uint8_t inChannelIdx, int32_t* outADCData)
{
    int status;
    int32_t current = 0;
    uint16_t adcRawData;

    mIOModValidateDriverStatus(ADC128D818ReadChannel(gADCI2CAddressTable[inSlaveID], inChannelIdx, &adcRawData));

    // Convert to mA: (300 / 2^12 - 1) * 2^16.
    current = ConversionDecode(adcRawData, 4801, 16, 1);

    // TODO: Make constants (should also be programmable).
    if (current > 300)
    {
        status = kIOModPortStatus_OverLoad;
    }
    else
    {
        status = kIOModPortStatus_Valid;
        *outADCData = current;
    }

    return status;
}

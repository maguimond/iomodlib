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
#include <stdbool.h>

// Lib includes.
#include "iomodutils.h"

// Driver includes.
#include "adc128d818.h"

// Bus includes.
#include "i2c.h"

// ----------------------------------------------------------------------------
// Private variables.
static ADC128D818_t gADC128D818[kADC128D818_MaxAddresses];

// Private constants.
#define kADCBusyTimeout 0x0010

// ----------------------------------------------------------------------------
static bool ADC128D818IsBusy(uint8_t inADCAddress, uint8_t inBusyFlag)
{
    bool isBusy = true;
    uint8_t i2cData = 0;

    int status = I2CReadRegister(inADCAddress, kADC128D818_RegisterBusyStatus, &i2cData, 1);
    if (status != 0)
    {
        return true;
    }
    else
    {
        // Mask the busy bits.
        i2cData &= inBusyFlag;
    }

    if (i2cData != 0)
    {
        isBusy = true;
    }
    else
    {
        isBusy = false;
    }

    return isBusy;
}

// ----------------------------------------------------------------------------
int ADC128D818Init(uint8_t inADCAddress)
{
    // TODO: Power on the device, then wait for at least 33ms.
    /*7. Program the Limit Registers (addresses 2Ah � 39h).
    8. Set the �START� bit of the Configuration Register (address 00h, bit 0) to 1.
    9. Set the 'INT_Clear' bit (address 00h, bit 3) to 0. If needed, program the 'INT_Enable' bit (address 00h, bit 1)
    to 1 to enable the INT output.
    The ADC128D818 then performs a round-robin monitoring of enabled voltage and temperature channels. The
    sequence of items being monitored corresponds to locations in the Channel Readings Registers (except for the
    temperature reading). Detailed descriptions of the register map can be found at the end of this data sheet.*/

    // Verify if the ADC is busy.
    uint32_t timeout = kADCBusyTimeout;
    while (ADC128D818IsBusy(inADCAddress, kADC128D818_RegisterBusyStatus_NotReady))
    {
        if ((-- timeout) == 0)
        {
            return -1;
        }
    }

    // Read manufacturing data.
    int status = I2CReadRegister(inADCAddress, kADC128D818_RegisterManufacturerID, &(gADC128D818[0].manufacturerID), 1);
    if (status != 0)
    {
        return status;
    }

    // Read revision data.
    status = I2CReadRegister(inADCAddress, kADC128D818_RegisterRevisionID, &(gADC128D818[0].revisionID), 1);
    if (status != 0)
    {
        return status;
    }

    const uint8_t gADC128D818RegisterConfigTable[] =
    {
        // Set ADC operation mode and VREF.
        kADC128D818_RegisterAdvancedConfiguration, (kADC128D818_Mode_SingleEnded << 1) | kADC128D818_RegisterAdvancedConfiguration_ExternalReferenceEnable,
        // Set Conversion Rate.
        kADC128D818_RegisterConversionRate, kADC128D818_ConversionRate_Continuous,
        // Enable channels.
        kADC128D818_RegisterChannelDisable, kADC128D818_RegisterChannelDisable_None,
        // Disable interrupts.
        kADC128D818_RegisterInterruptMask, kADC128D818_RegisterInterruptMask_All,
    };

    // Configure ADC with predefined registers.
    int adcRegisterTableSize = sizeof(gADC128D818RegisterConfigTable) / sizeof(uint8_t);
    for (uint8_t registerIdx = 0; registerIdx < adcRegisterTableSize; registerIdx ++)
    {
        uint8_t registerAddress = gADC128D818RegisterConfigTable[registerIdx ++];
        uint8_t registerValue = gADC128D818RegisterConfigTable[registerIdx];
        status = I2CWriteRegister(inADCAddress, registerAddress, &registerValue, 1);
        if (status != 0)
        {
            break;
        }
    }

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818SetMode(uint8_t inADCAddress, uint8_t inMode)
{
    mIOAssertArg(mADC128D818IsMode(inMode));

    uint8_t i2cData = (inMode << 1) | kADC128D818_RegisterAdvancedConfiguration_ExternalReferenceEnable;

    int status = I2CWriteRegister(inADCAddress, kADC128D818_RegisterAdvancedConfiguration, &i2cData, 1);

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818StartConversion(uint8_t inADCAddress, uint8_t inMode)
{
    mIOAssertArg(mADC128D818IsConversionRate(inMode));

    // Select conversion mode.
    uint8_t i2cData = inMode;
    int status = I2CWriteRegister(inADCAddress, kADC128D818_RegisterConversionRate, &i2cData, 1);

    // Enable startup of monitoring operations.
    // A voltage conversion takes 12.2 ms and a temperature conversion takes 3.6 ms
    i2cData = kADC128D818_RegisterConfiguration_Start;
    status |= I2CWriteRegister(inADCAddress, kADC128D818_RegisterConfiguration, &i2cData, 1);

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818StopConversion(uint8_t inADCAddress)
{
    // Disable monitoring operations.
    uint8_t i2cData = 0;

    int status = I2CWriteRegister(inADCAddress, kADC128D818_RegisterConfiguration, &i2cData, 1);

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818SingleConversion(uint8_t inADCAddress)
{
    uint8_t i2cData = kADC128D818_RegisterOneShot_OneShot;

    int status = I2CWriteRegister(inADCAddress, kADC128D818_RegisterOneShot, &i2cData, 1);

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818DeepShutdown(uint8_t inADCAddress, uint8_t inShutdownMode)
{
    mIOAssertArg(mADC128D818IsDeepShutdown(inShutdownMode));

    int status = I2CWriteRegister(inADCAddress, kADC128D818_RegisterDeepShutdown, &inShutdownMode, 1);

    return status;
}

// ----------------------------------------------------------------------------
int ADC128D818ReadChannel(uint8_t inADCAddress, uint8_t inChannel, uint16_t* outADCData)
{
    // Add channel address base.
    inChannel |= kADC128D818_RegisterChannel0Read;

    mIOAssertArg(mADC128D818IsChannelReadings(inChannel));

    uint8_t i2cData = 0;
    uint8_t* i2cDataPointer = &i2cData;
    int status = I2CReadRegister(inADCAddress, inChannel, i2cDataPointer, 2);

    uint16_t adcDataBuffer = *(uint16_t*)(i2cDataPointer);

    // ADC returns LSByte first [16..4]. Convert to MSByte first with result [12..0]
    *outADCData = mHTONS(adcDataBuffer) >> 4;

    return status;
}

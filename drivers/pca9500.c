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

// HAL includes.
#include "pca9500.h"

// Drivers includes.
#include "i2c.h"

// ----------------------------------------------------------------------------
// Private variables.
static PCA9500_t gPCA9500[kPCA9500_MaxAddresses];

// Private macros.
#define mValidateParam(inParam) if (!inParam) { return -1; }

// ----------------------------------------------------------------------------
void PCA9500Init(void)
{
    // Setup I2C1 driver.
    I2C1Setup(); // FIXME: Not modular. It references a specific function / driver.
}

// ----------------------------------------------------------------------------
int PCA9500IOExpanderSetIO(uint8_t inSlaveAddress, uint8_t inIOPin, uint8_t inState)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);

    if (inState == 0)
    {
        gPCA9500[inSlaveAddress].portState &= ~(1 << inIOPin);
    }
    else
    {
        gPCA9500[inSlaveAddress].portState |= 1 << inIOPin;
    }

    return I2C1Write(kPCA9500_IOExpanderBaseAddress | inSlaveAddress, &(gPCA9500[inSlaveAddress].portState), 1);
}

// ----------------------------------------------------------------------------
int PCA9500IOExpanderGetIO(uint8_t inSlaveAddress, uint8_t inIOPin, uint8_t* outState)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);

    int status = I2C1Read(kPCA9500_IOExpanderBaseAddress | inSlaveAddress, &(gPCA9500[inSlaveAddress].portState), 1);

    if (&(gPCA9500[inSlaveAddress].portState) && (1 << inIOPin))
    {
        *outState = 1;
    }
    else
    {
        *outState = 0;
    }

    return status;
}

// ----------------------------------------------------------------------------
int PCA9500IOExpanderSetPort(uint8_t inSlaveAddress, uint8_t inPortData)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);

    gPCA9500[inSlaveAddress].portState = inPortData;

    return I2C1Write(kPCA9500_IOExpanderBaseAddress | inSlaveAddress, &(gPCA9500[inSlaveAddress].portState), 1);
}

// ----------------------------------------------------------------------------
int PCA9500IOExpanderGetPort(uint8_t inSlaveAddress, uint8_t* outPortData)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);

    return I2C1Read(kPCA9500_IOExpanderBaseAddress | inSlaveAddress, outPortData, 1);
}

// ----------------------------------------------------------------------------
int PCA9500EEPROMPageWrite(uint8_t inSlaveAddress, uint8_t inMemoryAddress, uint8_t* inData, uint8_t inSize)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);
    // kPCA9500_EEPROMPageSize + 1, we are including the memory address.
    mValidateParam(inSize >= 1 && inSize <= kPCA9500_EEPROMPageSize + 1);

    uint8_t i2cData[kPCA9500_EEPROMPageSize + 1] = { 0 };
    i2cData[0] = inMemoryAddress;
    memcpy(i2cData + 1, inData, inSize);

    return I2C1Write(kPCA9500_EEPROMBaseAddress | inSlaveAddress, i2cData, inSize + 1);
}

// ----------------------------------------------------------------------------
int PCA9500EEPROMPageRead(uint8_t inSlaveAddress, uint8_t inMemoryAddress, uint8_t* outData, uint8_t inSize)
{
    mValidateParam(inSlaveAddress >= kPCA9500_SlaveAddress1 && inSlaveAddress < kPCA9500_MaxAddresses);
    mValidateParam(inSize >= 1 && inSize <= kPCA9500_EEPROMPageSize);

    return I2C1ReadRegister(kPCA9500_EEPROMBaseAddress | inSlaveAddress, inMemoryAddress, outData, inSize);
}

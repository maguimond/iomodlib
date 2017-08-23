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

#ifndef PCA9500_H_
#define PCA9500_H_

// Standard includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// Slave addresses.
#define kPCA9500_IOExpanderBaseAddress 0x20
#define kPCA9500_EEPROMBaseAddress 0x50
typedef enum
{
    kPCA9500_SlaveAddress1,
    kPCA9500_SlaveAddress2,
    kPCA9500_SlaveAddress3,
    kPCA9500_SlaveAddress4,
    kPCA9500_SlaveAddress5,
    kPCA9500_SlaveAddress6,
    kPCA9500_SlaveAddress7,
    kPCA9500_SlaveAddress8,
    kPCA9500_MaxAddresses,
} PCA9500_SlaveAddress_t;

// IO Expander constants
typedef enum
{
    kPCA9500_IOExpanderIO0,
    kPCA9500_IOExpanderIO1,
    kPCA9500_IOExpanderIO2,
    kPCA9500_IOExpanderIO3,
    kPCA9500_IOExpanderIO4,
    kPCA9500_IOExpanderIO5,
    kPCA9500_IOExpanderIO6,
    kPCA9500_IOExpanderIO7,
    kPCA9500_IOExpanderIOMax,
} PCA9500_IOExpanderIO_t;

// EEPROM constants
#define kPCA9500_EEPROMPageSize 4
#define kPCA9500_EEPROMTotSize 256

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint8_t portState;
} PCA9500_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
/// Initialize PCA9500 driver.
void PCA9500Init(void);
/// Set PCA9500 IO pin inIOPin to state inState (bit = 1 | 0) at slave address inSlaveAddress. Returns I2C level error code.
int PCA9500IOExpanderSetIO(uint8_t inSlaveAddress, uint8_t inIOPin, uint8_t inState);
/// Get PCA9500 IO pin outIOPin at slave address inSlaveAddress. Returns I2C level error code.
int PCA9500IOExpanderGetIO(uint8_t inSlaveAddress, uint8_t inIOPin, uint8_t* outState);
/// Set PCA9500 port inPortData to 8-bit value at slave address inSlaveAddress. Returns I2C level error code.
int PCA9500IOExpanderSetPort(uint8_t inSlaveAddress, uint8_t inPortData);
/// Get PCA9500 port ouPortData at slave address inSlaveAddress. Returns I2C level error code.
int PCA9500IOExpanderGetPort(uint8_t inSlaveAddress, uint8_t* outPortData);
/// Write PCA9500 EEPROM data inData of size inSize starting at memory address inMemoryAddress of slave address inSlaveAddress. Returns I2C level error code.
int PCA9500EEPROMPageWrite(uint8_t inSlaveAddress, uint8_t inMemoryAddress, uint8_t* inData, uint8_t inSize);
/// Read PCA9500 EEPROM data outData of size inSize starting at memory address inMemoryAddress of slave address inSlaveAddress. Returns I2C level error code.
int PCA9500EEPROMPageRead(uint8_t inSlaveAddress, uint8_t inMemoryAddress, uint8_t* outData, uint8_t inSize);

#ifdef __cplusplus
}
#endif

#endif // PCA9500_H_

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

#ifndef S25FL256_H_
#define S25FL256_H_

// Standard includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// Ordering constants.
#define kS25FL256_ManufacturerID 0x01
#define kS25FL256_DeviceID 0x18
#define kS25FL256_PageSize256B 256
#define kS25FL256_PageSize512B 512
// Sectors.
#define kS25FL256_4KSectorCount 32
#define kS25FL256_4KSectorSize 4096
#define kS25FL256_4KSectorFirst 0x00000000
#define kS25FL256_4KSectorLast 0x0001FFFF
#define kS25FL256_64KSectorCount 510
#define kS25FL256_64KSectorSize 65536
#define kS25FL256_64KSectorFirst 0x00020000
#define kS25FL256_64KSectorLast 0x01FFFFFF

/// Read Device Identification command set.

// Read Electronic Manufacturer Signature.
#define kS25FL256_RegisterREMS      0x90

// Read ID (JEDEC Manufacturer ID and JEDEC CFI).
#define kS25FL256_RegisterRDID      0x9F
typedef enum
{
    S25FL256_RegisterRDID_ManufacturerID,
    S25FL256_RegisterRDID_MemoryType,
    S25FL256_RegisterRDID_Capacity,
    S25FL256_RegisterRDID_IDCFI,
    S25FL256_RegisterRDID_SectorArchitecture,
    S25FL256_RegisterRDID_FamilyID,
} S25FL256_RegisterRDID_t;

typedef enum
{
    kS25FL256_RegisterRDID_SectorArchitecture256KB = 0x00,
    kS25FL256_RegisterRDID_SectorArchitecture64KB = 0x01,
} S25FL256_RegisterRDID_SectorArchitecture_t;

// Read Electronic Signature.
#define kS25FL256_RegisterRES       0xAB

/// Register Access.

// Read Status Register-1.
#define kS25FL256_RegisterRDSR1     0x05
typedef enum
{
    // Write in Progress.
    // 1 = Device Busy, a Write Registers (WRR), program, erase or other operation is in progress.
    // 0 = Ready Device is in standby mode and can accept commands.
    S25FL256_RegisterRDSR1_WIP = (1 << 0),
    // Write Enable Latch.
    // 1 = Device accepts Write Registers (WRR), program or erase commands.
    // 0 = Device ignores Write Registers (WRR), program or erase commands.
    // This bit is not affected by WRR, only WREN and WRDI commands affect this bit
    S25FL256_RegisterRDSR1_WEL = (1 << 1),
    // Block Protection.
    // Protects selected range of sectors (Block) from Program or Erase.
    S25FL256_RegisterRDSR1_BP0 = (1 << 2),
    S25FL256_RegisterRDSR1_BP1 = (1 << 3),
    S25FL256_RegisterRDSR1_BP2 = (1 << 4),
    // Erase Error Occurred.
    // 1 = Error occurred.
    // 0 = No Error.
    S25FL256_RegisterRDSR1_E_ERR = (1 << 5),
    // Programming Error Occurred.
    // 1 = Error occurred.
    // 0 = No Error.
    S25FL256_RegisterRDSR1_P_ERR = (1 << 6),
    // Status Register Write Disable.
    // 1 = Locks state of SRWD, BP, and configuration register bits when WP# is low by ignoring WRR command.
    // 0 = No protection, even when WP# is low.
    S25FL256_RegisterRDSR1_SRWD = (1 << 7),
} S25FL256_RegisterRDSR1_t;

// Read Status Register-2.
#define kS25FL256_RegisterRDSR2     0x07

typedef enum
{
    // Program Suspend.
    // 1 = In program suspend mode.
    // 0 = Not in program suspend mode.
    S25FL256_RegisterRDSR2_PS = (1 << 0),
    // Erase Suspend.
    // 1 = In erase suspend mode.
    // 0 = Not in erase suspend mode.
    S25FL256_RegisterRDSR2_ES = (1 << 1),
} S25FL256_RegisterRDSR2_t;

// Read Configuration Register-1.
#define kS25FL256_RegisterRDCR      0x35

typedef enum
{
    // Lock current state of BP2-0 bits in Status Register, TBPROT and TBPARM in Configuration Register, and OTP regions.
    // 1 = Block Protection and OTP locked.
    // 0 = Block Protection and OTP un-locked.
    S25FL256_RegisterRDCR_Freeze = (1 << 0),
    // Puts the device into Quad I/O operation.
    // 1 = Quad.
    // 0 = Dual or Serial.
    S25FL256_RegisterRDCR_QUAD = (1 << 1),
    // Configures Parameter Sectors location.
    // 1 = 4-kB physical sectors at top, (high address).
    // 0 = 4-kB physical sectors at bottom (Low address).
    // RFU in uniform sector devices.
    S25FL256_RegisterRDCR_TBPARM = (1 << 2),
    // Configures BP2-0 in Status Register.
    // 1 = Volatile.
    // 0 = Non-Volatile.
    S25FL256_RegisterRDCR_BPNV = (1 << 3),
    // Configures Start of Block Protection.
    // 1 = BP starts at bottom (Low address).
    // 0 = BP starts at top (High address).
    S25FL256_RegisterRDCR_TBPROT = (1 << 5),
    // Latency Code.
    // Selects number of initial read latency cycles.
    S25FL256_RegisterRDCR_LC0 = (1 << 6),
    S25FL256_RegisterRDCR_LC1 = (1 << 7),
} S25FL256_RegisterRDCR_t;

// Write Register (Status-1, Configuration-1).
#define kS25FL256_RegisterWRR       0x01

// Write Disable.
#define kS25FL256_RegisterWRDI      0x04

// Write Enable.
#define kS25FL256_RegisterWREN      0x06

// Bank Register Access (Legacy Command formerly used for Deep Power Down).
#define kS25FL256_RegisterBRAC      0xb9

/// Read Flash Array.

// Read (3- or 4-byte address)
#define kS25FL256_RegisterREAD      0x03

// Read (4-byte address)
#define kS25FL256_Register4READ     0x13

// Fast Read (3- or 4-byte address)
#define kS25FL256_RegisterFASTREAD  0x0B

// Fast Read (4-byte address)
#define kS25FL256_Register4FASTREAD 0x0C

// DDR Fast Read (3- or 4-byte address)
#define kS25FL256_RegisterDDRFR     0x0D

// DDR Fast Read (4-byte address)
#define kS25FL256_Register4DDRFR    0x0E

// Read Dual Out (3- or 4-byte address)
#define kS25FL256_RegisterDOR       0x3B

// Read Dual Out (4-byte address)
#define kS25FL256_Register4DOR      0x3C

/// Program Flash Array.

// Page Program (3- or 4-byte address)
#define kS25FL256_RegisterPP        0x02

// Page Program (4-byte address)
#define kS25FL256_Register4PP       0x12

/// Erase Flash Array.

// Parameter 4-kB, sector Erase (3- or 4-byte address).
#define kS25FL256_RegisterP4E       0x20
#define kS25FL256_Register4P4E      0x21
#define kS25FL256_RegisterSE        0xD8
#define kS25FL256_Register4SE       0xDC

// Bulk Erase (alternate command)
#define kS25FL256_RegisterBE        0xC7

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint8_t manufacturerID;
    uint8_t memoryType;
    uint8_t capacity;
    uint8_t IDCFI;
    uint8_t sectorArchitecture;
    uint8_t familyID;
    uint16_t pageSize;
} S25FL256_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int S25FL256Init(void);
///
int S25FL256BusyWait(void);
///
int S25FL256Erase4K(uint32_t inAddress);
///
int S25FL256Erase64K(uint32_t inAddress);
///
int S25FL256PageWrite(uint32_t inAddress, uint8_t* inData, uint32_t inSize);
///
int S25FL256PageRead(uint32_t inAddress, uint8_t* outData, uint32_t inSize);
///
uint16_t S25FL256GetPageSize(void);

#ifdef __cplusplus
}
#endif

#endif // S25FL256_H_

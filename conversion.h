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

#ifndef CONVERSION_H_
#define CONVERSION_H_

// Standard includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// --------------------------------------------------------------------------------------------------------------
// Data types
// --------------------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t multiplier;
    uint8_t scale;
    int8_t sign;
} ConversionEncode_t;

// --------------------------------------------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------------------------------------------
///
uint16_t ConversionDecode(uint16_t inData, uint16_t inMultiplier, uint8_t inScale, int8_t inSign);
///
uint16_t ComputeAmplitude(uint16_t inDataTable[], uint16_t inSize, uint16_t* outMin, uint16_t* outMax);

#ifdef __cplusplus
}
#endif

#endif // CONVERSION_H_

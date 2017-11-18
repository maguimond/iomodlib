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
#include <stdint.h>
#include <stdlib.h>

// Common includes.
#include "conversion.h"
#include "utils.h"

// --------------------------------------------------------------------------------------------------------------
uint16_t ConversionDecode(uint16_t inData, uint16_t inMultiplier, uint8_t inScale, int8_t inSign)
{
    uint32_t temp_product;
    uint32_t roundup;
    uint32_t result;

    if (!inScale)
    {
        return 0;
    }

    temp_product = inData * inMultiplier;
    result = temp_product >> inScale;
    // For negatives voltages monitoring.
    (inSign < 0)? result = abs(result - 31516): result;
    roundup = temp_product & (1 << (inScale - 1));

    if (result > UINT16_MAX)
    {
        return UINT16_MAX;
    }

    if (roundup && result != UINT16_MAX)
    {
        // Fract part >= 0.5, round up.
        result += 1;
    }

    return (uint16_t)result;
}

// --------------------------------------------------------------------------------------------------------------
uint16_t ComputeAmplitude(uint16_t inDataTable[], uint16_t inSize, uint16_t* outMin, uint16_t* outMax)
{
    // Initialize min & max to opposites.
    *outMax = 0;
    *outMin = UINT16_MAX;

    for (int tableIdx = 0; tableIdx < inSize; tableIdx ++)
    {
        *outMin = min(*outMin, inDataTable[tableIdx]);
        *outMax = max(*outMax, inDataTable[tableIdx]);
    }

    return *outMax - *outMin;
}

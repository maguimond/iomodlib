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

// Driver includes.
#include "bcd7seg.h"

// ----------------------------------------------------------------------------
// Private variables.

// Private constants.

// ----------------------------------------------------------------------------
// Display control - change 4-bit BCD to its 7-segment value.
static char BinToSeptSeg(char Data)
{
    // Tableau 0,1,2,3,4,5,6,7,8,9
    static char Arr[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x6F};
    // Masque des 4 bits MSB (Most Significant Bit) inutilisés
    return Arr[Data];
}

// ----------------------------------------------------------------------------
void BCDDisplayNumber(int inNumber, char* outDigits)
{
    char i, isNegativeNumber = 0;

    if (inNumber < 0)
    {
        inNumber = -inNumber;
        isNegativeNumber = 1;
    }

    for (i = 0; i < 3; i++)
    {
        outDigits[i] = BinToSeptSeg(inNumber % 10);
        inNumber = inNumber / 10;
        
        if (isNegativeNumber)
        {
            outDigits[3] = 0x40;
        }
        else
        {
            outDigits[3] = 0x00;
        }
    }
}




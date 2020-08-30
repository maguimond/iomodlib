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

#ifndef BCD7SEG_H_
#define BCD7SEG_H_

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------

/// Display control: Convert values (inNumber) to be displayed and put then to outDigits[].
void BCDDisplayNumber(int inNumber, char* outDigits);

/// BCD digit scan: Call this function in the timer interrupt to sweep digits.
void ChangeDigit(void);

#ifdef __cplusplus
}
#endif

#endif // BCD7SEG_H_

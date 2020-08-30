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
#include "7seg.h"

// ----------------------------------------------------------------------------
// Private variables.

// Private constants.
#define kSevenSegmentNumberOfDigits 4
#define kSevenSegmentDecimalOffset 16
#define kSevenSegmentASCIIOffset 32
#define kSevenSegmentASCIITableSize 96

const char SevenSegmentASCII[kSevenSegmentASCIITableSize] =
{
    0x00, // (space)
    0x86, // !
    0x22, // "
    0x7E, // #
    0x6D, // $
    0xD2, // %
    0x46, // &
    0x20, // '
    0x29, // (
    0x0B, // )
    0x21, // *
    0x70, // +
    0x10, // ,
    0x40, // -
    0x80, // .
    0x52, // /
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x09, // :
    0x0D, // ;
    0x61, // <
    0x48, // =
    0x43, // >
    0xD3, // ?
    0x5F, // @
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71, // F
    0x3D, // G
    0x76, // H
    0x30, // I
    0x1E, // J
    0x75, // K
    0x38, // L
    0x15, // M
    0x37, // N
    0x3F, // O
    0x73, // P
    0x6B, // Q
    0x33, // R
    0x6D, // S
    0x78, // T
    0x3E, // U
    0x3E, // V
    0x2A, // W
    0x76, // X
    0x6E, // Y
    0x5B, // Z
    0x39, // [
    0x64, // \
    0x0F, // ]
    0x23, // ^
    0x08, // _
    0x02, // `
    0x5F, // a
    0x7C, // b
    0x58, // c
    0x5E, // d
    0x7B, // e
    0x71, // f
    0x6F, // g
    0x74, // h
    0x10, // i
    0x0C, // j
    0x75, // k
    0x30, // l
    0x14, // m
    0x54, // n
    0x5C, // o
    0x73, // p
    0x67, // q
    0x50, // r
    0x6D, // s
    0x78, // t
    0x1C, // u
    0x1C, // v
    0x14, // w
    0x76, // x
    0x6E, // y
    0x5B, // z
    0x46, // {
    0x30, // |
    0x70, // }
    0x01, // ~
    0x00, // (del)
};

// ----------------------------------------------------------------------------
void DecimalTo7Seg(int inNumber, char* outDigits)
{
    char digitIndex, isNegativeNumber = 0;

    if (inNumber < 0)
    {
        inNumber = -inNumber;
        isNegativeNumber = 1;
    }

    for (digitIndex = 0; digitIndex < 3; digitIndex ++)
    {
        outDigits[digitIndex] = SevenSegmentASCII[inNumber % 10 + kSevenSegmentDecimalOffset];
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

// ----------------------------------------------------------------------------
void CharTo7Seg(const char* inString, char* outDigits)
{
    int digitIndex, asciiIndex;

    for (digitIndex = kSevenSegmentNumberOfDigits; digitIndex >= 0 ; digitIndex --)
    {
        asciiIndex = *inString - kSevenSegmentASCIIOffset;
        inString ++;
        if ((asciiIndex >= 0) && (asciiIndex < kSevenSegmentASCIITableSize))
        {
            outDigits[digitIndex] = SevenSegmentASCII[asciiIndex];
        }
    }
}

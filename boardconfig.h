/* Copyright (C) 2017, Marc-Andre Guimond <guimond.marcandre@gmail.com>.
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

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

// Standard includes.
#include <stdint.h>

typedef uint32_t (*BoardConfig_WriteToMedium)(uint32_t address, const uint8_t* data, uint32_t size);
typedef uint32_t (*BoardConfig_ReadFromMedium)(uint32_t address, uint8_t* destination, uint32_t size);

typedef void (*BoardConfig_Lock)(void* lock);
typedef void (*BoardConfig_Unlock)(void* lock);

typedef void (*BoardConfig_GetDefault)(uint8_t* destination, uint32_t destination_size);

typedef struct{
    BoardConfig_WriteToMedium write_to_medium;
    BoardConfig_ReadFromMedium read_from_medium;
    BoardConfig_GetDefault get_defaults;
    //Optionals
    void* shadow_lock;
    BoardConfig_Lock lock;
    BoardConfig_Unlock unlock;
}board_config_config_t;

//TODO: Remove the _ once all changes are done...

int BoardConfig_Init(board_config_config_t* config);

int BoardConfig_Commit(void);

int BoardConfig_Write(uint32_t inAddress, const uint8_t* inData, uint32_t inSize);

int BoardConfig_Read(uint32_t inAddress, uint8_t* outData, uint32_t inSize);

#endif // BOARDCONFIG_H_

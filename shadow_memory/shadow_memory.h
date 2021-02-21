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

#ifndef SHADOW_MEMORY_H
#define SHADOW_MEMORY_H

#include <stdint.h>

// Should return the actual size of the read/write (<= size)
typedef uint32_t (*SHADOW_MEMEORY_WriteToMedium)(uint32_t address, const uint8_t* data, uint32_t size);
typedef uint32_t (*SHADOW_MEMEORY_ReadFromMedium)(uint32_t address, uint8_t* destination, uint32_t size);

typedef void (*SHADOW_MEMORY_Lock)(void* lock);
typedef void (*SHADOW_MEMORY_Unlock)(void* lock);

typedef struct{
    //Mandatory
    uint8_t* memory;
    uint32_t memory_size;
    uint32_t offset_on_medium;
    SHADOW_MEMEORY_WriteToMedium write_to_medium;
    SHADOW_MEMEORY_ReadFromMedium read_from_medium;
    //Optionals
    void* shadow_lock;
    SHADOW_MEMORY_Lock lock;
    SHADOW_MEMORY_Unlock unlock;
}shadow_memory_t;

//Returns the actual read/write size (<= size)
uint32_t SHADOW_MEMORY_Sync(shadow_memory_t* shadow);   //Read all from medium to shadow
uint32_t SHADOW_MEMORY_Flush(shadow_memory_t* shadow);  //Write all to medium from shadow
uint32_t SHADOW_MEMORY_Write(shadow_memory_t* shadow, uint32_t offset, const uint8_t* data, uint32_t size);
uint32_t SHADOW_MEMORY_WriteThrough(shadow_memory_t* shadow, uint32_t offset, const uint8_t* data, uint32_t size);
uint32_t SHADOW_MEMORY_Read(shadow_memory_t* shadow, uint32_t offset, uint8_t* destination, uint32_t size);
uint32_t SHADOW_MEMORY_ReadThrough(shadow_memory_t* shadow, uint32_t offset, uint8_t* destination, uint32_t size);

#endif //SHADOW_MEMORY_H

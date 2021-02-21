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

#include "shadow_memory.h"
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

static bool Validate(shadow_memory_t* shadow);
static uint32_t GetOperationSize(uint32_t memory_size, uint32_t requested_offset, uint32_t requested_size);
static void Lock(SHADOW_MEMORY_Lock lock_fct, void* lock);
static void Unlock(SHADOW_MEMORY_Unlock unlock_fct, void* lock);

uint32_t SHADOW_MEMORY_Sync(shadow_memory_t* shadow){
    uint32_t operation_size = 0;
    if(Validate(shadow)){
        Lock(shadow->lock, shadow->shadow_lock);
        operation_size = shadow->read_from_medium(shadow->offset_on_medium, shadow->memory, shadow->memory_size);
        Unlock(shadow->unlock, shadow->shadow_lock);
    }
    return operation_size;
}

uint32_t SHADOW_MEMORY_Flush(shadow_memory_t* shadow){
    uint32_t operation_size = 0;
    if(Validate(shadow)){
        Lock(shadow->lock, shadow->shadow_lock);
        operation_size = shadow->write_to_medium(shadow->offset_on_medium, shadow->memory, shadow->memory_size);
        Unlock(shadow->unlock, shadow->shadow_lock);
    }
    return operation_size;
}

uint32_t SHADOW_MEMORY_Write(shadow_memory_t* shadow, uint32_t offset, const uint8_t* data, uint32_t size){
    uint32_t operation_size = 0;
    if(Validate(shadow) && data){
        operation_size = GetOperationSize(shadow->memory_size, offset, size);
        if(operation_size){
            Lock(shadow->lock, shadow->shadow_lock);
            memcpy(shadow->memory + offset, data, operation_size);
            Unlock(shadow->unlock, shadow->shadow_lock);
        }
    }
    return operation_size;
}

uint32_t SHADOW_MEMORY_WriteThrough(shadow_memory_t* shadow, uint32_t offset, const uint8_t* data, uint32_t size){
    uint32_t operation_size = 0;
    if(Validate(shadow) && data){
        operation_size = GetOperationSize(shadow->memory_size, offset, size);
        if(operation_size){
            uint32_t medium_operation_size;
            Lock(shadow->lock, shadow->shadow_lock);
            memcpy(shadow->memory + offset, data, operation_size);
            medium_operation_size = shadow->write_to_medium(shadow->offset_on_medium + offset, data, operation_size);
            if(medium_operation_size < operation_size){
                operation_size = medium_operation_size;
            }
            Unlock(shadow->unlock, shadow->shadow_lock);
        }
    }
    return operation_size;
}

uint32_t SHADOW_MEMORY_Read(shadow_memory_t* shadow, uint32_t offset, uint8_t* destination, uint32_t size){
    uint32_t operation_size = 0;
    if(Validate(shadow) && destination){
        operation_size = GetOperationSize(shadow->memory_size, offset, size);
        if(operation_size){
            Lock(shadow->lock, shadow->shadow_lock);
            memcpy(destination, shadow->memory + offset, operation_size);
            Unlock(shadow->unlock, shadow->shadow_lock);
        }
    }
    return operation_size;
}

uint32_t SHADOW_MEMORY_ReadThrough(shadow_memory_t* shadow, uint32_t offset, uint8_t* destination, uint32_t size){
    uint32_t operation_size = 0;
    if(Validate(shadow) && destination){
        operation_size = GetOperationSize(shadow->memory_size, offset, size);
        if(operation_size){
            uint32_t medium_operation_size;
            Lock(shadow->lock, shadow->shadow_lock);
            medium_operation_size = shadow->read_from_medium(shadow->offset_on_medium + offset, shadow->memory + offset, operation_size);
            if(medium_operation_size < operation_size){
                operation_size = medium_operation_size;
            }
            memcpy(destination, shadow->memory + offset, operation_size);
            Unlock(shadow->unlock, shadow->shadow_lock);
        }
    }
    return operation_size;
}

static bool Validate(shadow_memory_t* shadow){
    bool valid = shadow && shadow->memory && shadow->memory_size && shadow->read_from_medium && shadow->write_to_medium;
    if(valid){
        if(shadow->lock){
            if(shadow->unlock == NULL){
                valid = false;
            }
        }
    }
    return valid;
}

static uint32_t GetOperationSize(uint32_t memory_size, uint32_t requested_offset, uint32_t requested_size){
    uint32_t operation_size;
    if(requested_offset >= memory_size){
        operation_size = 0;
    }
    else if((memory_size - requested_offset) < requested_size){
        operation_size = memory_size - requested_offset;
    }
    else{
        operation_size = requested_size;
    }
    return operation_size;
}

static void Lock(SHADOW_MEMORY_Lock lock_fct, void* lock){
    if(lock_fct){
        lock_fct(lock);
    }
}

static void Unlock(SHADOW_MEMORY_Unlock unlock_fct, void* lock){
    if(unlock_fct){
        unlock_fct(lock);
    }
}

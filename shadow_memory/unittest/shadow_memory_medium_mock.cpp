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

#include "shadow_memory_medium_mock.hpp"

static ShadowMemoryMediumMock* mock_pointer = NULL;

void ShadowMemoryMediumMock_SetGlobalPointer(ShadowMemoryMediumMock* pointer){
    mock_pointer = pointer;
}

uint32_t ShadowMemoryMediumMock_WriteToMedium(uint32_t address, const uint8_t* data, uint32_t size){
    return mock_pointer->WriteToMedium(address, data, size);
}

uint32_t ShadowMemoryMediumMock_ReadFromMedium(uint32_t address, uint8_t* destination, uint32_t size){
    return mock_pointer->ReadFromMedium(address, destination, size);
}

void ShadowMemoryMediumMock_Lock(void* lock){
    mock_pointer->Lock(lock);
}

void ShadowMemoryMediumMock_Unlock(void* lock){
    mock_pointer->Unlock(lock);
}

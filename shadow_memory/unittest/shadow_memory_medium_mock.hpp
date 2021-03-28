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

#ifndef SHADOW_MEMORY_MOCK_HPP
#define SHADOW_MEMORY_MOCK_HPP

#include <gmock/gmock.h>

class ShadowMemoryMediumMock{
    public:
        ShadowMemoryMediumMock() = default;
        MOCK_METHOD(uint32_t, WriteToMedium, (uint32_t address, const uint8_t* data, uint32_t size));
        MOCK_METHOD(uint32_t, ReadFromMedium, (uint32_t address, uint8_t* destination, uint32_t size));
        MOCK_METHOD(void, Lock, (void* lock));
        MOCK_METHOD(void, Unlock, (void* lock));
};

void ShadowMemoryMediumMock_SetGlobalPointer(ShadowMemoryMediumMock* pointer);

uint32_t ShadowMemoryMediumMock_WriteToMedium(uint32_t address, const uint8_t* data, uint32_t size);
uint32_t ShadowMemoryMediumMock_ReadFromMedium(uint32_t address, uint8_t* destination, uint32_t size);
void ShadowMemoryMediumMock_Lock(void* lock);
void ShadowMemoryMediumMock_Unlock(void* lock);

#endif //SHADOW_MEMORY_MOCK_HPP

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

//TODO: Add test where we attempt to read/write more than the medium size (this need some more advanced mocking perhaps with matchers)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "shadow_memory_medium_mock.hpp"

extern "C" {
#include "shadow_memory.h"
};

using ::testing::NiceMock;
using ::testing::ReturnArg;
using ::testing::_;

TEST(GivenNullConfiguration, WhenSyncCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_Sync(NULL), 0);
}

TEST(GivenNullConfiguration, WhenFlushCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_Flush(NULL), 0);
}

TEST(GivenNullConfiguration, WhenWriteCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_Write(NULL, 0, NULL, 0), 0);
}

TEST(GivenNullConfiguration, WhenWriteThroughCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_WriteThrough(NULL, 0, NULL, 0), 0);
}

TEST(GivenNullConfiguration, WhenReadCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_Read(NULL, 0, NULL, 0), 0);
}

TEST(GivenNullConfiguration, WhenReadThroughCalledThenShouldReturnZero){
    EXPECT_EQ(SHADOW_MEMORY_ReadThrough(NULL, 0, NULL, 0), 0);
}

class GivenShadowMemoryTestBase : public ::testing::Test{
    protected:
        GivenShadowMemoryTestBase(){
            memset(&shadow, 0, sizeof(shadow));
            ShadowMemoryMediumMock_SetGlobalPointer(&medium_mock);
            shadow.memory_size = sizeof(test_data);
            shadow.memory = (uint8_t*)calloc(sizeof(test_data), 1);
            shadow.write_to_medium = ShadowMemoryMediumMock_WriteToMedium;
            shadow.read_from_medium = ShadowMemoryMediumMock_ReadFromMedium;
            shadow.lock = ShadowMemoryMediumMock_Lock;
            shadow.unlock = ShadowMemoryMediumMock_Unlock;
        }
        void SetUp() override {
            EXPECT_CALL(medium_mock, Lock(_)).Times(1);
            EXPECT_CALL(medium_mock, Unlock(_)).Times(1);
            ON_CALL(medium_mock, WriteToMedium(_, _, _)).WillByDefault(ReturnArg<2>());
            ON_CALL(medium_mock, ReadFromMedium(_, _, _)).WillByDefault(ReturnArg<2>());
        }
        ~GivenShadowMemoryTestBase(){
            free(shadow.memory);   
        }
        shadow_memory_t shadow;
        NiceMock<ShadowMemoryMediumMock> medium_mock;
        uint8_t test_data[100];
};

TEST_F(GivenShadowMemoryTestBase, WhenSyncCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(1);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(0);
    EXPECT_EQ(SHADOW_MEMORY_Sync(&shadow), shadow.memory_size);
}

TEST_F(GivenShadowMemoryTestBase, WhenFlushCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(0);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(1);
    EXPECT_EQ(SHADOW_MEMORY_Flush(&shadow), shadow.memory_size);
}

TEST_F(GivenShadowMemoryTestBase, WhenWriteCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(0);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(0);
    EXPECT_EQ(SHADOW_MEMORY_Write(&shadow, 0, test_data, sizeof(test_data)), shadow.memory_size);    
}

TEST_F(GivenShadowMemoryTestBase, WhenWriteThroughCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(0);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(1);
    EXPECT_EQ(SHADOW_MEMORY_WriteThrough(&shadow, 0, test_data, sizeof(test_data)), shadow.memory_size);    
}

TEST_F(GivenShadowMemoryTestBase, WhenReadCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(0);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(0);
    EXPECT_EQ(SHADOW_MEMORY_Read(&shadow, 0, test_data, sizeof(test_data)), shadow.memory_size);    
}

TEST_F(GivenShadowMemoryTestBase, WhenReadThroughCalledThenShouldReturnSize){
    EXPECT_CALL(medium_mock, ReadFromMedium(_, _, _)).Times(1);
    EXPECT_CALL(medium_mock, WriteToMedium(_, _, _)).Times(0);
    EXPECT_EQ(SHADOW_MEMORY_ReadThrough(&shadow, 0, test_data, sizeof(test_data)), shadow.memory_size);    
}

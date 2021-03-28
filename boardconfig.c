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

// Standard includes
#include <string.h>
#include <stdbool.h>

// Lib includes.
#include "shadow_memory.h"
#include "boardconfig.h"
#include "iomodconfig.h"
#include "version.h"
#include "crc16.h"

//TODO: Why does this module needs to know hoe many CRCs there is and where stuff is supposed to be? Should be generic

static bool init_done = false;
static shadow_memory_t shadow;
static uint8_t shadow_memory[kBoardConfigTotalSize];
static uint8_t double_buffer[kBoardConfigTotalSize];

static int Write(uint32_t inAddress, const uint8_t* inData, uint32_t inSize);
static int Commit(void);

int BoardConfig_Init(board_config_config_t* config){
    int status = -1;
    if(config && (config->get_defaults)){
        shadow.shadow_lock = config->shadow_lock;
        shadow.lock = config->lock;
        shadow.unlock = config->unlock;
        shadow.write_to_medium = config->write_to_medium;
        shadow.read_from_medium = config->read_from_medium;
        shadow.offset_on_medium = kBoardConfigStartAddress;
        shadow.memory = shadow_memory;
        shadow.memory_size = kBoardConfigTotalSize;
        if(SHADOW_MEMORY_Sync(&shadow) == kBoardConfigTotalSize){
            if(SHADOW_MEMORY_Read(&shadow, 0, double_buffer, kBoardConfigTotalSize) == kBoardConfigTotalSize){
                uint16_t factoryCRC = CRC16ComputeCRC(0, (uint8_t*)(double_buffer + kBoardConfig_FactoryAddressOffset), kBoardConfig_FactorySize);
                uint16_t userCRC = CRC16ComputeCRC(0, (uint8_t*)(double_buffer + kBoardConfig_UserAddressOffset), kBoardConfig_UserSize);
                bool config_valid = true;
                if (*(uint16_t*)(double_buffer + kBoardConfig_Factory_Magic) != mHTONS(kBoardConfig_MagicNumber))
                {
                    mBoardConfigPrintWarning("Bad magic");
                    config_valid = false;
                }
                else if (double_buffer[kBoardConfig_Factory_FlashLayout] != kVersionConfigLayout)
                {
                    mBoardConfigPrintWarning("Bad layout");
                    config_valid = false;
                }
                else if (mHTONS(factoryCRC) != *((uint16_t*)(double_buffer + kBoardConfig_Factory_CRC)))
                {
                    mBoardConfigPrintWarning("Bad factory CRC");
                    config_valid = false;
                }
                else if (mHTONS(userCRC) != *((uint16_t*)(double_buffer + kBoardConfig_User_CRC)))
                {
                    mBoardConfigPrintWarning("Bad user CRC");
                    config_valid = false;
                }
                else
                {
                    mBoardConfigPrintInfo("Loaded config");
                    config_valid = true;
                }

                if (config_valid == false)
                {
                    mBoardConfigPrintInfo("Force defaults");
                    config->get_defaults(double_buffer, sizeof(double_buffer));
                    Write(0, double_buffer, sizeof(double_buffer));
                    Commit();
                }
            }
            status = 0;
            init_done = true;
        }
    }
    return status;
}

int BoardConfig_Commit(void){
    int status = -1;
    if(init_done){
        status = Commit();
    }
    return status;
}

int BoardConfig_Write(uint32_t inAddress, const uint8_t* inData, uint32_t inSize){
    int status = -1;
    if(init_done){
        status = Write(inAddress, inData, inSize);
    }
    return status;
}

int BoardConfig_Read(uint32_t inAddress, uint8_t* outData, uint32_t inSize){
    int status = -1;
    if(init_done){
        if(SHADOW_MEMORY_Read(&shadow, inAddress, outData, inSize) == inSize){
            status = 0;
        }
    }
    return status;
}

static int Write(uint32_t inAddress, const uint8_t* inData, uint32_t inSize){
    int status = -1;
    if(SHADOW_MEMORY_Write(&shadow, inAddress, inData, inSize) == inSize){
        status = 0;
    }
    return status;
}

static int Commit(void){
    int status = -1;
    if(SHADOW_MEMORY_Read(&shadow, 0, double_buffer, kBoardConfigTotalSize) == kBoardConfigTotalSize){
        uint16_t crc = mHTONS(CRC16ComputeCRC(0, (uint8_t*)(double_buffer + kBoardConfig_FactoryAddressOffset), kBoardConfig_FactorySize));
        if(SHADOW_MEMORY_Write(&shadow, kBoardConfig_Factory_CRC, (uint8_t*)(&crc), 2) == 2){
            crc = mHTONS(CRC16ComputeCRC(0, (uint8_t*)(double_buffer + kBoardConfig_UserAddressOffset), kBoardConfig_UserSize));
            if(SHADOW_MEMORY_Write(&shadow, kBoardConfig_User_CRC, (uint8_t*)(&crc), 2) == 2){
                if(SHADOW_MEMORY_Flush(&shadow) == kBoardConfigTotalSize){
                    status = 0;
                }
            }
        }
    }
    return status;
}

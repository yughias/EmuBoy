#include "mbc.h"
#include "memory.h"
#include "info.h"
#include "ini.h"
#include "hardware.h"

#include "mbcs/all.h"

#include <stdio.h>
#include <string.h>

readFunc mbc_mapper_0000_3FFF;
readFunc mbc_mapper_4000_7FFF;
readFunc mbc_mapper_A000_BFFF_read;
writeFunc mbc_mapper_A000_BFFF_write;

writeFunc mbc_rom_write;

uint8_t MBC_0000_1FFF;
uint8_t MBC_2000_3FFF;
uint8_t MBC_4000_5FFF;
uint8_t MBC_6000_7FFF;

// advanced mbc registers
uint8_t MBC_2000_2FFF;
uint8_t MBC_3000_3FFF;

bool hasBattery;
bool mbcAlreadyWritten;
bool hasRtc;
bool hasCamera;

#define RAM_MAPPER(name) mbc_mapper_A000_BFFF_read =  name ## _read; mbc_mapper_A000_BFFF_write = name ## _write

uint8_t no_mapped_address_read(uint16_t addr){ return 0xFF; }
void no_mapped_address_write(uint16_t addr, uint8_t byte){}
uint8_t no_mbc_address(uint16_t addr){ return ROM[addr]; }
void mbc_no_write(uint16_t addr, uint8_t byte){}

void mbc_standard_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x2000){
        MBC_0000_1FFF = byte;
    } else if(addr < 0x4000) {
        MBC_2000_3FFF = byte;
    } else if(addr < 0x6000) {
        MBC_4000_5FFF = byte;
    } else {
        MBC_6000_7FFF = byte;
    }
}

void detectConsoleAndMbc(){
    mbc_rom_write = mbc_standard_registers;
    mbc_mapper_0000_3FFF = no_mbc_address;
    mbc_mapper_4000_7FFF = no_mbc_address;
    RAM_MAPPER(no_mapped_address);
    MBC_0000_1FFF = 0x00;
    MBC_2000_3FFF = 0x00;
    MBC_4000_5FFF = 0x00;
    MBC_6000_7FFF = 0x00;
    MBC_2000_2FFF = 0x00;
    MBC_3000_3FFF = 0x00;
    hasBattery = false;
    hasRtc = false;
    hasCamera = false;
    mbcAlreadyWritten = false; 

    console_type = CGB_TYPE;

    if(config_force_dmg_when_possible && ROM[0x143] != 0xC0)
        console_type = DMG_TYPE;

    // in emscripten we don't have config file
    // DMG is emulated if ROM doesn't have any CGB enhancements
    #ifdef __EMSCRIPTEN__
    if(ROM[0x143] < 0x80)
        console_type = DMG_TYPE;
    #endif

    // MBC for megaduck
    if(!containNintendoLogo(ROM)){
        console_type = MEGADUCK_TYPE;

        // default 32k megaduck rom
        if(ROM_SIZE == 1 << 15)
            return;

        uint16_t checksum = calculateRomChecksum(ROM, ROM_SIZE);

        if(
            checksum == SULEIMAN_TREASURE_CHECKSUM ||
            checksum == PUPPET_KNIGHT_CHECKSUM
        ){
            mbc_mapper_0000_3FFF = megaduck_special_mapper;
            mbc_mapper_4000_7FFF = megaduck_special_mapper;
            RAM_MAPPER(megaduck_special_registers);
            mbc_rom_write = mbc_no_write;
            return;
        }

        // megaduck mapper with register in 0x0001
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = megaduck_standard_mapper;
        mbc_rom_write =  megaduck_standard_registers;
        return;
    }

    if(detectM161(ROM)){
        printf("M161 DETECTED!\n");
        mbc_rom_write = m161_registers;
        mbc_mapper_0000_3FFF = m161_rom;
        mbc_mapper_4000_7FFF = m161_rom;
        return;
    }

    if(detectMMM01(ROM)){
        printf("MMM01 DETECTED!\n");
        MBC_3000_3FFF = 0x01;
        mbc_rom_write = mmm01_registers;
        mbc_mapper_0000_3FFF = mmm01_0000_3FFF;
        mbc_mapper_4000_7FFF = mmm01_4000_7FFF;
        RAM_MAPPER(mmm01_ram);
        return;
    }

    if(detectMBC1M(ROM)){
        printf("MBC1M DETECTED\n");
        mbc_mapper_0000_3FFF = mbc1m_0000_3FFF;
        mbc_mapper_4000_7FFF = mbc1m_4000_7FFF;
        return;
    }

    switch(ROM[0x147]){
        case 0x01:
        mbc_mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc_mapper_4000_7FFF = mbc1_4000_7FFF;
        printf("MBC1 ON!\n");
        break;

        case 0x03:
        mbc_mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc_mapper_4000_7FFF = mbc1_4000_7FFF;
        RAM_MAPPER(mbc1_ram);
        hasBattery = true;
        printf("MBC1 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x05:
        mbc_rom_write = mbc2_registers;
        MBC_2000_3FFF = 0x01;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc2_4000_7FFF;
        printf("MBC2 ONLY ON!\n");
        break;

        case 0x06:
        mbc_rom_write = mbc2_registers;
        MBC_2000_3FFF = 0x01;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc2_4000_7FFF;
        RAM_MAPPER(mbc2_ram);
        hasBattery = true;
        printf("MBC2 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x0F:
        case 0x10:
        hasRtc  = true;
        case 0x11:
        case 0x12:
        case 0x13:
        mbc_rom_write = mbc3_registers;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc3_4000_7FFF;
        RAM_MAPPER(mbc3_ram);
        hasBattery = true;
        printf("MBC3 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x19:
        mbc_rom_write = mbc5_registers;
        MBC_2000_2FFF = 0x01;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc5_4000_7FFF;
        hasBattery = false;
        printf("MBC5 ONLY ON!\n");
        break;

        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        mbc_rom_write = mbc5_registers;
        MBC_2000_2FFF = 0x01;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc5_4000_7FFF;
        RAM_MAPPER(mbc5_ram);
        hasBattery = true;
        printf("MBC5 WITH RAM ON!\n");
        break;

        case 0x22:
        mbc_rom_write = mbc5_registers;
        MBC_2000_2FFF = 0x01;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc5_4000_7FFF;
        RAM_MAPPER(no_mapped_address);
        printf("MBC7 ON!\n");
        break;

        case 0xFC:
        hasBattery = true;
        hasCamera = true;
        MBC_2000_3FFF = 0x1;
        mbc_mapper_0000_3FFF = no_mbc_address;
        mbc_mapper_4000_7FFF = mbc3_4000_7FFF;
        RAM_MAPPER(mbc_cam_ram);
        mbc_cam_init();
        printf("GAMEBOY CAMERA ON!\n");
        break;

        default:
        break;
    }
}

bool detectM161(const uint8_t* buf){
    if(!strncmp((const char*)&buf[0x134], "TETRIS SET", strlen("TETRIS SET")))
        if(buf[0x14D] == 0x3F && buf[0x14E] == 0x4C && buf[0x14F] == 0xB7)
            return true;
    
    return false;
}

bool detectMMM01(const uint8_t* buf){
    if(ROM_SIZE == (1 << 15)){
        return false;
    }

    if(containNintendoLogo(buf + ROM_SIZE - (1 << 15)))
        return true;
    return false;
}

bool detectMBC1M(const uint8_t* buf){
    int nintendo_logo_size = 48; 
    int nintendo_logo_postion = 0x10 << 14;
    if(ROM_SIZE < nintendo_logo_postion + 0x100 + nintendo_logo_size)
        return false;

    if(containNintendoLogo(buf + nintendo_logo_postion))
        return true;

    return false;
}

bool containNintendoLogo(const uint8_t* buffer){
    const uint8_t nintendo_logo[48] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };

    bool out = memcmp(&buffer[0x104], nintendo_logo, sizeof(nintendo_logo)); 
    

    return !out;
}
#include "mbc.h"
#include "memory.h"
#include "info.h"
#include "ini.h"
#include "gb.h"

#include "mbcs/all.h"

#include <stdio.h>
#include <string.h>

#define RAM_MAPPER(name) mbc->mapper_A000_BFFF_read =  name ## _read; mbc->mapper_A000_BFFF_write = name ## _write

static uint8_t no_mapped_address_read(gb_t* gb, uint16_t addr){ return 0xFF; }
static void no_mapped_address_write(gb_t* gb, uint16_t addr, uint8_t byte){}
static uint8_t no_mbc_address(gb_t* gb, uint16_t addr){ return gb->ROM[addr]; }
static void mbc_no_write(gb_t* gb, uint16_t addr, uint8_t byte){}

static void mbc_standard_registers(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if(addr < 0x2000){
        mbc->REG_0000_1FFF = byte;
    } else if(addr < 0x4000) {
        mbc->REG_2000_3FFF = byte;
    } else if(addr < 0x6000) {
        mbc->REG_4000_5FFF = byte;
    } else {
        mbc->REG_6000_7FFF = byte;
    }
}

void detectConsoleAndMbc(gb_t* gb){
    mbc_t* mbc = &gb->mbc;
    mbc->rom_write = mbc_standard_registers;
    mbc->mapper_0000_3FFF = no_mbc_address;
    mbc->mapper_4000_7FFF = no_mbc_address;
    RAM_MAPPER(no_mapped_address);
    mbc->REG_0000_1FFF = 0x00;
    mbc->REG_2000_3FFF = 0x00;
    mbc->REG_4000_5FFF = 0x00;
    mbc->REG_6000_7FFF = 0x00;
    mbc->REG_2000_2FFF = 0x00;
    mbc->REG_3000_3FFF = 0x00;
    mbc->hasBattery = false;
    mbc->hasRtc = false;
    mbc->hasCamera = false;
    mbc->mbcAlreadyWritten = false; 

    gb->console_type = CGB_TYPE;

    if(config_force_dmg_when_possible && gb->ROM[0x143] != 0xC0)
        gb->console_type = DMG_TYPE;

    // in emscripten we don't have config file
    // DMG is emulated if ROM doesn't have any CGB enhancements
    #ifdef __EMSCRIPTEN__
    if(ROM[0x143] < 0x80)
        gb->console_type = DMG_TYPE;
    #endif

    // MBC for megaduck
    if(!containNintendoLogo(gb->ROM)){
        gb->console_type = MEGADUCK_TYPE;

        // default 32k megaduck rom
        if(gb->ROM_SIZE == 1 << 15)
            return;

        uint16_t checksum = calculateRomChecksum(gb->ROM, gb->ROM_SIZE);

        if(
            checksum == SULEIMAN_TREASURE_CHECKSUM ||
            checksum == PUPPET_KNIGHT_CHECKSUM
        ){
            mbc->mapper_0000_3FFF = megaduck_special_mapper;
            mbc->mapper_4000_7FFF = megaduck_special_mapper;
            RAM_MAPPER(megaduck_special_registers);
            mbc->rom_write = mbc_no_write;
            return;
        }

        // megaduck mapper with register in 0x0001
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = megaduck_standard_mapper;
        mbc->rom_write =  megaduck_standard_registers;
        return;
    }

    if(detectM161(gb->ROM)){
        printf("M161 DETECTED!\n");
        mbc->rom_write = m161_registers;
        mbc->mapper_0000_3FFF = m161_rom;
        mbc->mapper_4000_7FFF = m161_rom;
        return;
    }

    if(detectMMM01(gb->ROM, gb->ROM_SIZE)){
        printf("MMM01 DETECTED!\n");
        mbc->REG_3000_3FFF = 0x01;
        mbc->rom_write = mmm01_registers;
        mbc->mapper_0000_3FFF = mmm01_0000_3FFF;
        mbc->mapper_4000_7FFF = mmm01_4000_7FFF;
        RAM_MAPPER(mmm01_ram);
        return;
    }

    if(detectMBC1M(gb->ROM, gb->ROM_SIZE)){
        printf("MBC1M DETECTED\n");
        mbc->mapper_0000_3FFF = mbc1m_0000_3FFF;
        mbc->mapper_4000_7FFF = mbc1m_4000_7FFF;
        return;
    }

    switch(gb->ROM[0x147]){
        case 0x01:
        mbc->mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc->mapper_4000_7FFF = mbc1_4000_7FFF;
        printf("MBC1 ON!\n");
        break;

        case 0x03:
        mbc->mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc->mapper_4000_7FFF = mbc1_4000_7FFF;
        RAM_MAPPER(mbc1_ram);
        mbc->hasBattery = true;
        printf("MBC1 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x05:
        mbc->rom_write = mbc2_registers;
        mbc->REG_2000_3FFF = 0x01;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc2_4000_7FFF;
        printf("MBC2 ONLY ON!\n");
        break;

        case 0x06:
        mbc->rom_write = mbc2_registers;
        mbc->REG_2000_3FFF = 0x01;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc2_4000_7FFF;
        RAM_MAPPER(mbc2_ram);
        mbc->hasBattery = true;
        printf("MBC2 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x0F:
        case 0x10:
        mbc->hasRtc = true;
        mbc->data = allocRtc();
        case 0x11:
        case 0x12:
        case 0x13:
        mbc->rom_write = mbc3_registers;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc3_4000_7FFF;
        RAM_MAPPER(mbc3_ram);
        mbc->hasBattery = true;
        printf("MBC3 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x19:
        mbc->rom_write = mbc5_registers;
        mbc->REG_2000_2FFF = 0x01;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc5_4000_7FFF;
        mbc->hasBattery = false;
        printf("MBC5 ONLY ON!\n");
        break;

        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        mbc->hasRumble = gb->ROM[0x147] >= 0x1C;
        mbc->rom_write = mbc5_registers;
        mbc->REG_2000_2FFF = 0x01;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc5_4000_7FFF;
        RAM_MAPPER(mbc5_ram);
        mbc->hasBattery = true;
        printf("MBC5 WITH RAM ON!\n");
        break;

        case 0x22:
        mbc->hasBattery = true;
        mbc->rom_write = mbc5_registers;
        mbc->REG_2000_2FFF = 0x01;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc5_4000_7FFF;
        RAM_MAPPER(mbc7_ram);
        mbc->data = mbc7_alloc();
        printf("MBC7 ON!\n");
        break;

        case 0xFC:
        mbc->hasBattery = true;
        mbc->hasCamera = true;
        mbc->REG_2000_3FFF = 0x1;
        mbc->mapper_0000_3FFF = no_mbc_address;
        mbc->mapper_4000_7FFF = mbc3_4000_7FFF;
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

bool detectMMM01(const uint8_t* buf, size_t size){
    if(size == (1 << 15)){
        return false;
    }

    if(containNintendoLogo(buf + size - (1 << 15)))
        return true;
    return false;
}

bool detectMBC1M(const uint8_t* buf, size_t size){
    int nintendo_logo_size = 48; 
    int nintendo_logo_position = 0x10 << 14;
    if(size < nintendo_logo_position + 0x100 + nintendo_logo_size)
        return false;

    if(containNintendoLogo(buf + nintendo_logo_position))
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
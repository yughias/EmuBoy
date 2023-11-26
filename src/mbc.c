#include <mbc.h>
#include <memory.h>

#include <stdio.h>
#include <string.h>

readFunc mbc_mapper_0000_3FFF;
readFunc mbc_mapper_4000_7FFF;
mbcFunc mbc_mapper_A000_BFFF;

writeFunc mbc_rom_write;

uint8_t MBC_0000_1FFF;
uint8_t MBC_2000_3FFF;
uint8_t MBC_4000_5FFF;
uint8_t MBC_6000_7FFF;

// advanced mbc registers
uint8_t MBC_2000_2FFF;
uint8_t MBC_3000_3FFF;

uint8_t RTC_08;
uint8_t RTC_09;
uint8_t RTC_0A;
uint8_t RTC_0B;
uint8_t RTC_0C;

bool hasBattery;
bool alreadyWrite;

#define MAP_RTC(addr) case 0x ## addr: return &RTC_ ## addr

uint8_t* noMappedAdress(uint16_t addr){ return &NOT_MAPPED; }
uint8_t noMbcAddress(uint16_t addr){ return ROM[addr]; }

uint8_t mbc1_0000_3FFF(uint16_t addr){
    addr &= (1 << 14) - 1;
    if(MBC_6000_7FFF & 0b1){
        size_t real_addr = addr;
        real_addr |= (MBC_4000_5FFF & 0b11) << 19;
        real_addr &= ROM_SIZE - 1;  
        return ROM[real_addr];
    } 
    
    return ROM[addr];
}

uint8_t mbc1_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = (MBC_2000_3FFF & 0b11111) == 0x00 ? 0x01 : (MBC_2000_3FFF & 0b11111); 
    real_addr |= bank << 14;
    real_addr |= (MBC_4000_5FFF & 0b11) << 19;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t* mbc1_ram(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A){
        NOT_MAPPED = 0xFF;
        return &NOT_MAPPED;
    }

    addr &= (1 << 13) - 1;
    if(MBC_6000_7FFF & 0b1){
        addr |= (MBC_4000_5FFF & 0b11) << 13;
        addr &= ERAM_SIZE - 1;
    }
    return ERAM + addr;
}

uint8_t mbc2_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = (MBC_2000_3FFF & 0b1111) == 0x00 ? 0x01 : (MBC_2000_3FFF & 0b1111); 
    real_addr |= bank << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t* mbc2_ram(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A){
        NOT_MAPPED = 0xFF;
        return &NOT_MAPPED;
    }

    uint16_t ram_addr = addr & (1 << 9) - 1;
    ERAM[ram_addr] |= 0xF0;
    return ERAM + ram_addr;
}

uint8_t mbc3_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = MBC_2000_3FFF == 0x00 ? 0x01 : (MBC_2000_3FFF & 0b1111111); 
    real_addr |= bank << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t* mbc3_ram(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A){
        NOT_MAPPED = 0xFF;
        return &NOT_MAPPED;
    }

    switch(MBC_4000_5FFF){
        MAP_RTC(08);
        MAP_RTC(09);
        MAP_RTC(0A);
        MAP_RTC(0B);
        MAP_RTC(0C);

        default:
        addr &= (1 << 13) - 1;
        if(MBC_6000_7FFF & 0b1){
            addr |= (MBC_4000_5FFF & 0b11) << 13;
            addr &= ERAM_SIZE - 1;
        }
        return ERAM + addr;
    }
}

uint8_t mbc5_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint16_t bank = ((MBC_3000_3FFF & 0b1) << 8) | MBC_2000_2FFF;
    real_addr |= (bank & 0x1FF) << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t* mbc5_ram(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A){
        NOT_MAPPED = 0xFF;
        return &NOT_MAPPED;
    }

    addr &= (1 << 13) - 1;
    addr |= (MBC_4000_5FFF & 0b1111) << 13;
    addr &= ERAM_SIZE - 1;
    return ERAM + addr;
}

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

void mbc2_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x4000)
        if(!(addr & 0x100))
            MBC_0000_1FFF = byte;
        else
            MBC_2000_3FFF = byte;
}


void mbc_advanced_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x2000)
        MBC_0000_1FFF = byte;
    else if(addr < 0x3000)
        MBC_2000_2FFF = byte;
    else if(addr < 0x4000)
        MBC_3000_3FFF = byte;
    else if(addr < 0x6000)
        MBC_4000_5FFF = byte;
    else
        MBC_6000_7FFF = byte;
}

uint8_t m161_rom(uint16_t addr){
    size_t real_addr = addr | (MBC_0000_1FFF << 15);
    return ROM[real_addr];
}

void m161_registers(uint16_t addr, uint8_t byte){
    if(!alreadyWrite){
        alreadyWrite = true;
        MBC_0000_1FFF = byte;
        MBC_0000_1FFF &= (ROM_SIZE - 1) >> 15;
    }
}

uint8_t mmm01_0000_3FFF(uint16_t addr){
    size_t real_addr = addr;
    
    if(alreadyWrite){
        real_addr += MBC_2000_2FFF << 14;
    } else {
        real_addr += ROM_SIZE - (1 << 15);
    }

    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mmm01_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    
     if(alreadyWrite){
        real_addr &= 0x3FFF;
        real_addr += MBC_2000_2FFF << 14;
        real_addr += MBC_3000_3FFF << 14;
    } else {
        real_addr += ROM_SIZE - (1 << 15);
    }

    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t* mmm01_ram(uint16_t addr){
    if((MBC_0000_1FFF & 0b1111) != 0xA){
        NOT_MAPPED = 0xFF;
        return &NOT_MAPPED;
    }

    size_t real_addr = (MBC_4000_5FFF << 13) + (addr & 0x1FFF);
    real_addr &= ERAM_SIZE - 1;
    return ERAM + real_addr;
}

void mmm01_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x2000){
        // MBC_0000_1FFF : ram_enable
        if(!alreadyWrite)
            alreadyWrite = true;
        else 
            MBC_0000_1FFF = byte;
    } else if(addr < 0x4000){
        // MBC_2000_2FFF : rom_base
        // MBC_3000_3FFF : rom_select
        if(!alreadyWrite){
            MBC_2000_2FFF = byte & 0x3F;
        } else {
            MBC_3000_3FFF = byte;
        }
    } else if(addr < 0x6000){
        // MBC_4000_5FFF : ram_select
        if(alreadyWrite){
            MBC_4000_5FFF = byte;
        }
    }
}

void detectMBC(){
    mbc_rom_write = mbc_standard_registers;
    MBC_0000_1FFF = 0x00;
    MBC_2000_3FFF = 0x00;
    MBC_4000_5FFF = 0x00;
    MBC_6000_7FFF = 0x00;
    MBC_2000_2FFF = 0x00;
    MBC_3000_3FFF = 0x00;
    RTC_08 = 0x00;
    RTC_09 = 0x00;
    RTC_0A = 0x00;
    RTC_0B = 0x00;
    RTC_0C = 0x00;
    hasBattery = false;
    alreadyWrite = false;

    if(detectM161(ROM)){
        printf("M161 DETECTED!\n");
        mbc_rom_write = m161_registers;
        mbc_mapper_0000_3FFF = m161_rom;
        mbc_mapper_4000_7FFF = m161_rom;
        mbc_mapper_A000_BFFF = noMappedAdress;
        return;
    }

    if(detectMMM01(ROM)){
        printf("MMM01 DETECTED!\n");
        MBC_3000_3FFF = 0x01;
        mbc_rom_write = mmm01_registers;
        mbc_mapper_0000_3FFF = mmm01_0000_3FFF;
        mbc_mapper_4000_7FFF = mmm01_4000_7FFF;
        mbc_mapper_A000_BFFF = mmm01_ram;
        return;
    }

    switch(ROM[0x147]){
        case 0x01:
        mbc_mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc_mapper_4000_7FFF = mbc1_4000_7FFF;
        mbc_mapper_A000_BFFF = noMappedAdress;
        printf("MBC1 ON!\n");
        break;

        case 0x03:
        mbc_mapper_0000_3FFF = mbc1_0000_3FFF;
        mbc_mapper_4000_7FFF = mbc1_4000_7FFF;
        mbc_mapper_A000_BFFF = mbc1_ram;
        hasBattery = true;
        printf("MBC1 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x05:
        mbc_rom_write = mbc2_registers;
        MBC_2000_3FFF = 0x01;
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = mbc2_4000_7FFF;
        mbc_mapper_A000_BFFF = noMappedAdress;
        printf("MBC2 ONLY ON!\n");
        break;

        case 0x06:
        mbc_rom_write = mbc2_registers;
        MBC_2000_3FFF = 0x01;
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = mbc2_4000_7FFF;
        mbc_mapper_A000_BFFF = mbc2_ram;
        hasBattery = true;
        printf("MBC2 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x10:
        case 0x13:
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = mbc3_4000_7FFF;
        mbc_mapper_A000_BFFF = mbc3_ram;
        hasBattery = true;
        printf("MBC3 WITH RAM AND BATTERY ON!\n");
        break;

        case 0x19:
        mbc_rom_write = mbc_advanced_registers;
        MBC_2000_2FFF = 0x01;
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = mbc5_4000_7FFF;
        mbc_mapper_A000_BFFF = noMappedAdress;
        hasBattery = false;
        printf("MBC5 ONLY ON!\n");
        break;

        case 0x1B:
        case 0x1E:
        mbc_rom_write = mbc_advanced_registers;
        MBC_2000_2FFF = 0x01;
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = mbc5_4000_7FFF;
        mbc_mapper_A000_BFFF = mbc5_ram;
        hasBattery = true;
        printf("MBC5 WITH RAM ON!\n");
        break;

        default:
        mbc_mapper_0000_3FFF = noMbcAddress;
        mbc_mapper_4000_7FFF = noMbcAddress;
        mbc_mapper_A000_BFFF = noMappedAdress;
        break;
    }
}

bool detectM161(const uint8_t* buf){
    if(!strncmp(&buf[0x134], "TETRIS SET", strlen("TETRIS SET")))
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

bool containNintendoLogo(const uint8_t* buffer){
    static uint8_t nintendo_logo[48] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
    };

    return !memcmp(&buffer[0x104], nintendo_logo, sizeof(nintendo_logo)) ? true : false;
}
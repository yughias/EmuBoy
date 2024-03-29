#include "mbc.h"
#include "memory.h"

uint8_t mmm01_0000_3FFF(uint16_t addr){
    size_t real_addr = addr;
    
    if(mbcAlreadyWritten){
        real_addr += MBC_2000_2FFF << 14;
    } else {
        real_addr += ROM_SIZE - (1 << 15);
    }

    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mmm01_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    
     if(mbcAlreadyWritten){
        real_addr &= 0x3FFF;
        real_addr += MBC_2000_2FFF << 14;
        real_addr += MBC_3000_3FFF << 14;
    } else {
        real_addr += ROM_SIZE - (1 << 15);
    }

    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mmm01_ram_read(uint16_t addr){
    if((MBC_0000_1FFF & 0b1111) != 0xA)
        return 0xFF;

    size_t real_addr = (MBC_4000_5FFF << 13) + (addr & 0x1FFF);
    real_addr &= ERAM_SIZE - 1;
    return ERAM[real_addr];
}

void mmm01_ram_write(uint16_t addr, uint8_t byte){
    if((MBC_0000_1FFF & 0b1111) != 0xA)
        return;

    size_t real_addr = (MBC_4000_5FFF << 13) + (addr & 0x1FFF);
    real_addr &= ERAM_SIZE - 1;
    ERAM[real_addr] = byte;
}

void mmm01_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x2000){
        // MBC_0000_1FFF : ram_enable
        if(!mbcAlreadyWritten)
            mbcAlreadyWritten = true;
        else 
            MBC_0000_1FFF = byte;
    } else if(addr < 0x4000){
        // MBC_2000_2FFF : rom_base
        // MBC_3000_3FFF : rom_select
        if(!mbcAlreadyWritten){
            MBC_2000_2FFF = byte & 0x3F;
        } else {
            MBC_3000_3FFF = byte;
        }
    } else if(addr < 0x6000){
        // MBC_4000_5FFF : ram_select
        if(mbcAlreadyWritten){
            MBC_4000_5FFF = byte;
        }
    }
}

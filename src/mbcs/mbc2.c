#include "mbc.h"
#include "memory.h"

uint8_t mbc2_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = (MBC_2000_3FFF & 0b1111) == 0x00 ? 0x01 : (MBC_2000_3FFF & 0b1111); 
    real_addr |= bank << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mbc2_ram_read(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    uint16_t ram_addr = addr & (1 << 9) - 1;
    return ERAM[ram_addr] | 0xF0;
}

void mbc2_ram_write(uint16_t addr, uint8_t byte){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return;

    uint16_t ram_addr = addr & (1 << 9) - 1;
    ERAM[ram_addr] = byte;
}

void mbc2_registers(uint16_t addr, uint8_t byte){
    if(addr < 0x4000){
        if(!(addr & 0x100))
            MBC_0000_1FFF = byte;
        else
            MBC_2000_3FFF = byte;
    }
}
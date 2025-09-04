#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t mbc2_4000_7FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = (mbc->REG_2000_3FFF & 0b1111) == 0x00 ? 0x01 : (mbc->REG_2000_3FFF & 0b1111); 
    real_addr |= bank << 14;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}

uint8_t mbc2_ram_read(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    uint16_t ram_addr = addr & (1 << 9) - 1;
    return gb->ERAM[ram_addr] | 0xF0;
}

void mbc2_ram_write(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return;

    uint16_t ram_addr = addr & (1 << 9) - 1;
    gb->ERAM[ram_addr] = byte;
}

void mbc2_registers(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if(addr < 0x4000){
        if(!(addr & 0x100))
            mbc->REG_0000_1FFF = byte;
        else
            mbc->REG_2000_3FFF = byte;
    }
}
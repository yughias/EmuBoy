#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t mbc1_0000_3FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    addr &= (1 << 14) - 1;
    if(mbc->REG_6000_7FFF & 0b1){
        size_t real_addr = addr;
        real_addr |= (mbc->REG_4000_5FFF & 0b11) << 19;
        real_addr &= gb->ROM_SIZE - 1;  
        return gb->ROM[real_addr];
    } 
    
    return gb->ROM[addr];
}

uint8_t mbc1_4000_7FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t bank = (mbc->REG_2000_3FFF & 0b11111) == 0x00 ? 0x01 : (mbc->REG_2000_3FFF & 0b11111); 
    real_addr |= bank << 14;
    real_addr |= (mbc->REG_4000_5FFF & 0b11) << 19;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}

uint8_t mbc1_ram_read(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    if(mbc->REG_6000_7FFF & 0b1){
        real_addr |= (mbc->REG_4000_5FFF & 0b11) << 13;
        real_addr &= gb->ERAM_SIZE - 1;
    }
    return gb->ERAM[real_addr];
}

void mbc1_ram_write(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return;
    
    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    if(mbc->REG_6000_7FFF & 0b1){
        real_addr |= (mbc->REG_4000_5FFF & 0b11) << 13;
        real_addr &= gb->ERAM_SIZE - 1;
    }
    gb->ERAM[real_addr] = byte;
}
#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t mbc1m_0000_3FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    addr &= (1 << 14) - 1;
    if(mbc->REG_6000_7FFF & 0b1){
        size_t real_addr = addr;
        uint8_t rom_bank = (mbc->REG_2000_3FFF & 0b1111) | ((mbc->REG_4000_5FFF & 0b11) << 5);
        rom_bank = ((rom_bank >> 1) & 0x30);
        real_addr |= rom_bank << 14;
        real_addr &= gb->ROM_SIZE - 1;  
        return gb->ROM[real_addr];
    } 
    
    return gb->ROM[addr];
}

uint8_t mbc1m_4000_7FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t rom_bank = (mbc->REG_2000_3FFF & 0b1111) | ((mbc->REG_4000_5FFF & 0b11) << 5);
    rom_bank = ((rom_bank >> 1) & 0x30) | (rom_bank & 0x0F);
    real_addr |= rom_bank << 14;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}
#include "mbc.h"
#include "memory.h"

uint8_t mbc1m_0000_3FFF(uint16_t addr){
    addr &= (1 << 14) - 1;
    if(MBC_6000_7FFF & 0b1){
        size_t real_addr = addr;
        uint8_t rom_bank = (MBC_2000_3FFF & 0b1111) | ((MBC_4000_5FFF & 0b11) << 5);
        rom_bank = ((rom_bank >> 1) & 0x30);
        real_addr |= rom_bank << 14;
        real_addr &= ROM_SIZE - 1;  
        return ROM[real_addr];
    } 
    
    return ROM[addr];
}

uint8_t mbc1m_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint8_t rom_bank = (MBC_2000_3FFF & 0b1111) | ((MBC_4000_5FFF & 0b11) << 5);
    rom_bank = ((rom_bank >> 1) & 0x30) | (rom_bank & 0x0F);
    real_addr |= rom_bank << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}
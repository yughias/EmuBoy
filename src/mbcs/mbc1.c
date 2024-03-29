#include "mbc.h"
#include "memory.h"

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

uint8_t mbc1_ram_read(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    if(MBC_6000_7FFF & 0b1){
        real_addr |= (MBC_4000_5FFF & 0b11) << 13;
        real_addr &= ERAM_SIZE - 1;
    }
    return ERAM[real_addr];
}

void mbc1_ram_write(uint16_t addr, uint8_t byte){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return;
    
    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    if(MBC_6000_7FFF & 0b1){
        real_addr |= (MBC_4000_5FFF & 0b11) << 13;
        real_addr &= ERAM_SIZE - 1;
    }
    ERAM[real_addr] = byte;
}
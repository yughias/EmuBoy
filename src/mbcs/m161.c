#include "mbc.h"
#include "memory.h"

uint8_t m161_rom(uint16_t addr){
    size_t real_addr = addr | (MBC_0000_1FFF << 15);
    return ROM[real_addr];
}

void m161_registers(uint16_t addr, uint8_t byte){
    if(!mbcAlreadyWritten){
        mbcAlreadyWritten = true;
        MBC_0000_1FFF = byte;
        MBC_0000_1FFF &= (ROM_SIZE - 1) >> 15;
    }
}
#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t m161_rom(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr = addr | (mbc->REG_0000_1FFF << 15);
    return gb->ROM[real_addr];
}

void m161_registers(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if(!mbc->mbcAlreadyWritten){
        mbc->mbcAlreadyWritten = true;
        mbc->REG_0000_1FFF = byte;
        mbc->REG_0000_1FFF &= (gb->ROM_SIZE - 1) >> 15;
    }
}
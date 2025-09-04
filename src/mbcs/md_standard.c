#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t megaduck_standard_mapper(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr;
    addr -= 0x4000;
    uint8_t bank = mbc->REG_0000_1FFF ? mbc->REG_0000_1FFF : 1;
    real_addr = addr + 0x4000 * bank;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}

void megaduck_standard_registers(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if(addr == 1)
        mbc->REG_0000_1FFF = byte;
}
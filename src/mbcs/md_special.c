#include "mbc.h"
#include "memory.h"
#include "gb.h"

uint8_t megaduck_special_mapper(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr;
    uint8_t bank = mbc->REG_0000_1FFF & 1;
    real_addr = addr + 0x8000 * bank;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}

uint8_t megaduck_special_registers_read(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    if(addr == 0xB000)
        return mbc->REG_0000_1FFF;
    
    return 0xFF;
}

void megaduck_special_registers_write(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if(addr == 0xB000)
        mbc->REG_0000_1FFF = byte;
}
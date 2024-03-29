#include "mbc.h"
#include "memory.h"

uint8_t megaduck_standard_mapper(uint16_t addr){
    size_t real_addr;
    addr -= 0x4000;
    uint8_t bank = MBC_0000_1FFF ? MBC_0000_1FFF : 1;
    real_addr = addr + 0x4000 * bank;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

void megaduck_standard_registers(uint16_t addr, uint8_t byte){
    if(addr == 1)
        MBC_0000_1FFF = byte;
}
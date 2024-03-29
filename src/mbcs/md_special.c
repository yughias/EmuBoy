#include "mbc.h"
#include "memory.h"

uint8_t megaduck_special_mapper(uint16_t addr){
    size_t real_addr;
    uint8_t bank = MBC_0000_1FFF & 1;
    real_addr = addr + 0x8000 * bank;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t megaduck_special_registers_read(uint16_t addr){
    if(addr == 0xB000)
        return MBC_0000_1FFF;
    
    return 0xFF;
}

void megaduck_special_registers_write(uint16_t addr, uint8_t byte){
    if(addr == 0xB000)
        MBC_0000_1FFF = byte;
}
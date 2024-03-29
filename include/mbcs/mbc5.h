#ifndef __MBC5_H__
#define __MBC5_H__

#include <stdint.h>

uint8_t mbc5_4000_7FFF(uint16_t addr);
uint8_t mbc5_ram_read(uint16_t addr);
void mbc5_ram_write(uint16_t addr, uint8_t byte);
void mbc5_registers(uint16_t addr, uint8_t byte);

#endif
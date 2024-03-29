#ifndef __MBC2_H__
#define __MBC2_H__

#include <stdint.h>

uint8_t mbc2_4000_7FFF(uint16_t addr);
uint8_t mbc2_ram_read(uint16_t addr);
void mbc2_ram_write(uint16_t addr, uint8_t byte);
void mbc2_registers(uint16_t addr, uint8_t byte);

#endif
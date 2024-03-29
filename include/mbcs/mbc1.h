#ifndef __MBC1_H__
#define __MBC1_H__

#include <stdint.h>

uint8_t mbc1_0000_3FFF(uint16_t addr);
uint8_t mbc1_4000_7FFF(uint16_t addr);
uint8_t mbc1_ram_read(uint16_t addr);
void mbc1_ram_write(uint16_t addr, uint8_t byte);

#endif
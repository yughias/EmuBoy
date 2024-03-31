#ifndef __MBC_7_H__
#define __MBC_7_H__

#include <stdint.h>

uint8_t mbc7_ram_read(uint16_t addr);
void mbc7_ram_write(uint16_t addr, uint8_t byte);

#endif
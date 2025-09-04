#ifndef __MBC2_H__
#define __MBC2_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t mbc2_4000_7FFF(gb_t* gb, uint16_t addr);
uint8_t mbc2_ram_read(gb_t* gb, uint16_t addr);
void mbc2_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);
void mbc2_registers(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
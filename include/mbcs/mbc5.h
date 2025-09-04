#ifndef __MBC5_H__
#define __MBC5_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t mbc5_4000_7FFF(gb_t* gb, uint16_t addr);
uint8_t mbc5_ram_read(gb_t* gb, uint16_t addr);
void mbc5_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);
void mbc5_registers(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
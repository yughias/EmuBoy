#ifndef __MBC1_H__
#define __MBC1_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t mbc1_0000_3FFF(gb_t* gb, uint16_t addr);
uint8_t mbc1_4000_7FFF(gb_t* gb, uint16_t addr);
uint8_t mbc1_ram_read(gb_t* gb, uint16_t addr);
void mbc1_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
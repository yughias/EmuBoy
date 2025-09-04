#ifndef __MBC_7_H__
#define __MBC_7_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t mbc7_ram_read(gb_t* gb, uint16_t addr);
void mbc7_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);
void* mbc7_alloc();

#endif
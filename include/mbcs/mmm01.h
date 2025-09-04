#ifndef __MMM01_H__
#define __MMM01_H__

#include <stdint.h>

typedef struct gb_t gb_t;

void mmm01_registers(gb_t* gb, uint16_t addr, uint8_t byte);
uint8_t mmm01_0000_3FFF(gb_t* gb, uint16_t addr);
uint8_t mmm01_4000_7FFF(gb_t* gb, uint16_t addr);
uint8_t mmm01_ram_read(gb_t* gb, uint16_t addr);
void mmm01_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
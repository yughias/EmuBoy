#ifndef __M161_H__
#define __M161_H__

#include <stdint.h>

typedef struct gb_t gb_t;

void m161_registers(gb_t* gb, uint16_t addr, uint8_t byte);
uint8_t m161_rom(gb_t* gb, uint16_t addr);

#endif
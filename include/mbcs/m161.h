#ifndef __M161_H__
#define __M161_H__

#include <stdint.h>

void m161_registers(uint16_t addr, uint8_t byte);
uint8_t m161_rom(uint16_t addr);

#endif
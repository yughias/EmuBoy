#ifndef __MMM01_H__
#define __MMM01_H__

#include <stdint.h>

void mmm01_registers(uint16_t addr, uint8_t byte);
uint8_t mmm01_0000_3FFF(uint16_t addr);
uint8_t mmm01_4000_7FFF(uint16_t addr);
uint8_t mmm01_ram_read(uint16_t addr);
void mmm01_ram_write(uint16_t addr, uint8_t byte);

#endif
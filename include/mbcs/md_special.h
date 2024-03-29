#ifndef __MD_SPECIAL_H__
#define __MD_SPECIAL_H__

#include <stdint.h>

uint8_t megaduck_special_mapper(uint16_t addr);
uint8_t megaduck_special_registers_read(uint16_t addr);
void megaduck_special_registers_write(uint16_t addr, uint8_t byte);

#endif
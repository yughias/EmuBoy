#ifndef __MD_STANDARD_H__
#define __MD_STANDARD_H__

#include <stdint.h>

uint8_t megaduck_standard_mapper(uint16_t addr);
void megaduck_standard_registers(uint16_t addr, uint8_t byte);

#endif
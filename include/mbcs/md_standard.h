#ifndef __MD_STANDARD_H__
#define __MD_STANDARD_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t megaduck_standard_mapper(gb_t* gb, uint16_t addr);
void megaduck_standard_registers(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
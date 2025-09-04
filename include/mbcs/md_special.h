#ifndef __MD_SPECIAL_H__
#define __MD_SPECIAL_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t megaduck_special_mapper(gb_t* gb, uint16_t addr);
uint8_t megaduck_special_registers_read(gb_t* gb, uint16_t addr);
void megaduck_special_registers_write(gb_t* gb, uint16_t addr, uint8_t byte);

#endif
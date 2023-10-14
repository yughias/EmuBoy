#ifndef __MBC_H__
#define __MBC_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t* (*mbcFunc)(uint16_t);

extern mbcFunc mbc_rom_write;

extern mbcFunc mbc_mapper_0000_3FFF;
extern mbcFunc mbc_mapper_4000_7FFF;
extern mbcFunc mbc_mapper_A000_BFFF;

extern bool hasBattery;

void detectMBC();

#endif
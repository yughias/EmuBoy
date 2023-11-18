#ifndef __MBC_H__
#define __MBC_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t* (*mbcFunc)(uint16_t);
typedef uint8_t (*readFunc)(uint16_t);
typedef void (*writeFunc)(uint16_t, uint8_t);

extern writeFunc mbc_rom_write;

extern readFunc mbc_mapper_0000_3FFF;
extern readFunc mbc_mapper_4000_7FFF;
extern mbcFunc mbc_mapper_A000_BFFF;

extern bool hasBattery;

void detectMBC();

#endif
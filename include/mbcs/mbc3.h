#ifndef __MBC3_H__
#define __MBC3_H__

#include <stdint.h>

uint8_t mbc3_4000_7FFF(uint16_t addr);
uint8_t mbc3_ram_read(uint16_t addr);
void mbc3_ram_write(uint16_t addr, uint8_t byte);
void mbc3_registers(uint16_t addr, uint8_t byte);
void stepRtc(uint64_t secs);
void saveRtc(const char* filename);
void loadRtc(const char* filename);

#endif
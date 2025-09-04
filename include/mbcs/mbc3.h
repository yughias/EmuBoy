#ifndef __MBC3_H__
#define __MBC3_H__

#include <stdint.h>

typedef struct gb_t gb_t;
typedef struct rtc_t rtc_t;

uint8_t mbc3_4000_7FFF(gb_t* gb, uint16_t addr);
uint8_t mbc3_ram_read(gb_t* gb, uint16_t addr);
void mbc3_ram_write(gb_t* gb, uint16_t addr, uint8_t byte);
void mbc3_registers(gb_t* gb, uint16_t addr, uint8_t byte);
void saveRtc(rtc_t* rtc, const char* filename);
void loadRtc(rtc_t* rtc, const char* filename);
rtc_t* allocRtc();

#endif
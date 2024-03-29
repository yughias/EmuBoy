#ifndef __MBC_H__
#define __MBC_H__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t (*readFunc)(uint16_t);
typedef void (*writeFunc)(uint16_t, uint8_t);

extern writeFunc mbc_rom_write;
extern readFunc mbc_mapper_0000_3FFF;
extern readFunc mbc_mapper_4000_7FFF;
extern readFunc mbc_mapper_A000_BFFF_read;
extern writeFunc mbc_mapper_A000_BFFF_write;

extern bool hasBattery;
extern bool hasRtc;
extern bool hasCamera;
extern bool mbcAlreadyWritten;

extern uint8_t MBC_0000_1FFF;
extern uint8_t MBC_2000_3FFF;
extern uint8_t MBC_4000_5FFF;
extern uint8_t MBC_6000_7FFF;
extern uint8_t MBC_2000_2FFF;
extern uint8_t MBC_3000_3FFF;

void detectConsoleAndMbc();
bool detectM161(const uint8_t*);
bool detectMMM01(const uint8_t*);
bool detectMBC1M(const uint8_t*);
bool containNintendoLogo(const uint8_t*);

#endif
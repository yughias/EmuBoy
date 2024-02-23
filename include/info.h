#ifndef __INFO_H__
#define __INFO_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SULEIMAN_TREASURE_CHECKSUM 0xEAC5
#define PUPPET_KNIGHT_CHECKSUM 0x41AB

bool isNewLinceseeCode(uint8_t*);
const char* getRomName(uint8_t*);
const char* getManufacturerName(uint8_t*);
const char* getCartridgeType(uint8_t*);
size_t getRomSize(uint8_t*);
size_t getRamSize(uint8_t*);
void printInfo(uint8_t*);
uint16_t calculateRomChecksum(uint8_t*, size_t);

#endif
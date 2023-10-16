#ifndef __INFO_H__
#define __INFO_H__

#include <stdint.h>
#include <stdbool.h>

bool containNintendoLogo(uint8_t*);
bool isNewLinceseeCode(uint8_t*);
const char* getRomName(uint8_t*);
const char* getManufacturerName(uint8_t*);
const char* getCartridgeType(uint8_t*);
size_t getRomSize(uint8_t*);
size_t getRamSize(uint8_t*);
void printInfo(uint8_t*);

#endif
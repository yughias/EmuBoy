#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BOOTROM_DISABLE_ADDR 0xFF50

#define VRAM_SIZE 0x4000
#define WRAM_SIZE 0x8000
#define OAM_SIZE  0xA0
#define HRAM_SIZE 0x7F
#define MAX_ERAM_SIZE 0x80000
#define CRAM_SIZE 64

#define BOOTROM_START_ADDR 0x0000
#define ROM_START_ADDR 0x0000
#define VRAM_START_ADDR 0x8000
#define WRAM_START_ADDR 0xC000
#define OAM_START_ADDR 0xFE00
#define HRAM_START_ADDR 0xFF80

extern char romName[FILENAME_MAX];
extern char savName[FILENAME_MAX];

typedef struct gb_t gb_t;

void initMemory(gb_t*, const char*);
void loadRom(gb_t*, const char*);
bool loadBootRom(gb_t*, const char*);
void loadSav(gb_t*, const char*);
void saveSav(gb_t*, const char*);
void freeMemory(gb_t* gb);

void setFilename(const char*);

uint8_t readByte(void*, uint16_t);
void writeByte(void*, uint16_t, uint8_t);

#endif
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

extern uint8_t BOOTROM_DISABLE_REG;

extern uint8_t NOT_MAPPED;

extern uint8_t* BOOTROM;
extern size_t BOOTROM_SIZE;

extern uint8_t* ROM;
extern size_t ROM_SIZE;

extern size_t ERAM_SIZE;
extern uint8_t ERAM[MAX_ERAM_SIZE];

extern uint8_t VRAM[VRAM_SIZE];
extern uint8_t WRAM[WRAM_SIZE];
extern uint8_t OAM[OAM_SIZE];
extern uint8_t HRAM[HRAM_SIZE];

extern uint8_t SVBK_REG;
extern uint8_t VBK_REG;
extern uint8_t KEY0_REG;
extern uint8_t KEY1_REG;

extern uint8_t BGP_CRAM[CRAM_SIZE];
extern uint8_t OBP_CRAM[CRAM_SIZE];

extern char romName[FILENAME_MAX];
extern char savName[FILENAME_MAX];

void initMemory(const char*);
void loadRom(const char*);
bool loadBootRom(const char*);
void loadSav(const char*);
void saveSav(const char*);
void freeMemory();

void setFilename(const char*);

uint8_t readByte(uint16_t);
void writeByte(uint16_t, uint8_t);

void writeToCRAM(uint8_t*, uint8_t, uint8_t*);
uint8_t readFromCRAM(uint8_t*, uint8_t*);

#endif
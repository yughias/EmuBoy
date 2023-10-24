#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BOOTROM_DISABLE_ADDR 0xFF50
#define IF_ADDR 0xFF0F
#define IE_ADDR 0xFFFF
#define SB_ADDR 0xFF01
#define SC_ADDR  0xFF02

#define VBLANK_IRQ                0b00000001
#define STAT_IRQ                  0b00000010
#define TIMER_IRQ                 0b00000100
#define SERIAL_IRQ                0b00001000
#define JOYPAD_IRQ                0b00010000

#define BOOTROM_SIZE 0x100
#define VRAM_SIZE 0x2000
#define WRAM_SIZE 0x2000
#define OAM_SIZE  0xA0
#define HRAM_SIZE 0x7F
#define ERAM_SIZE 0x80000

#define BOOTROM_START_ADDR 0x0000
#define ROM_START_ADDR 0x0000
#define VRAM_START_ADDR 0x8000
#define WRAM_START_ADDR 0xC000
#define OAM_START_ADDR 0xFE00
#define HRAM_START_ADDR 0xFF80

extern uint8_t BOOTROM_DISABLE_REG;
extern uint8_t IE_REG;
extern uint8_t IF_REG;
extern uint8_t SB_REG;
extern uint8_t SC_REG;

extern uint8_t NOT_MAPPED[0x10000];
extern uint8_t TEMP_REG;

extern uint8_t BOOTROM[BOOTROM_SIZE];
extern uint8_t* ROM;
extern size_t ROM_SIZE;

extern uint8_t ERAM[ERAM_SIZE];

extern bool bootromEnabled;

extern uint8_t VRAM[VRAM_SIZE];
extern uint8_t WRAM[WRAM_SIZE];
extern uint8_t OAM[OAM_SIZE];
extern uint8_t HRAM[HRAM_SIZE];

extern char romName[FILENAME_MAX];
extern char savName[FILENAME_MAX];

void initMemory();
void loadRom(const char*);
void loadBootRom(const char*);
void loadSav(const char*);
void saveSav(const char*);
void freeMemory();

void setFilename(const char*);

uint8_t* getReadAddress(uint16_t);
uint8_t* getWriteAddress(uint16_t);

#endif
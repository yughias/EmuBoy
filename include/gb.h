#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "sm83.h"
#include "ppu.h"
#include "memory.h"
#include "joypad.h"
#include "dma.h"
#include "mbc.h"
#include "apu.h"
#include "gb_timer.h"
#include "serial.h"

#include <stdint.h>

#define CYCLES_PER_FRAME 70224
#define DIV_INCREMENT_RATE 256
#define REFRESH_RATE 59.727500569606 

typedef enum {DMG_TYPE, CGB_TYPE, MEGADUCK_TYPE, DMG_ON_CGB_TYPE} CONSOLE_TYPE;

typedef struct gb_t {
    CONSOLE_TYPE console_type;
    sm83_t cpu;
    uint64_t startFrame_clock;
    gb_timer_t timer;   
    dma_t dma;
    joypad_t joypad;
    serial_t serial;
    ppu_t ppu;
    apu_t apu;
    mbc_t mbc;

    readGbFunc readTable[0x100];
    writeGbFunc writeTable[0x100];

    uint8_t BOOTROM_DISABLE_REG;

    uint8_t* BOOTROM;
    size_t BOOTROM_SIZE;
    
    uint8_t* ROM;
    size_t ROM_SIZE;
    
    size_t ERAM_SIZE;
    uint8_t ERAM[MAX_ERAM_SIZE];
    
    uint8_t VRAM[VRAM_SIZE];
    uint8_t WRAM[WRAM_SIZE];
    uint8_t OAM[OAM_SIZE];
    uint8_t HRAM[HRAM_SIZE];
    
    uint8_t BGP_CRAM[CRAM_SIZE];
    uint8_t OBP_CRAM[CRAM_SIZE];
    
    uint8_t SVBK_REG;
    uint8_t VBK_REG;
    uint8_t KEY0_REG;
    uint8_t KEY1_REG;
} gb_t;

void initConsole(gb_t*);
void emulateHardware(gb_t*);

#endif
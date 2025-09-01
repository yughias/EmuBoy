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
#define REFRESH_RATE 59.73

typedef enum {DMG_TYPE, CGB_TYPE, MEGADUCK_TYPE, DMG_ON_CGB_TYPE} CONSOLE_TYPE;

extern sm83_t cpu;
extern CONSOLE_TYPE console_type;
extern uint64_t startFrame_clock;

void emulateCpu(sm83_t*);
void emulateHardware(sm83_t*);
void updateTimer();
void tickHardware(int);
void initConsole();


#endif
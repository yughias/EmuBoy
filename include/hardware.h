#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <cpu.h>
#include <ppu.h>
#include <memory.h>
#include <joypad.h>
#include <dma.h>
#include <mbc.h>
#include <apu.h>
#include <gb_timer.h>

#define CYCLES_PER_FRAME 70224
#define SCALINE_CYCLES 456
#define DIV_INCREMENT_RATE 256
#define REFRESH_RATE 59.73

extern cpu_t cpu;

extern size_t master_cycle;

void emulateCpu(cpu_t*);
void emulateHardware(cpu_t*);
void updateTimer();
void updateDiv();
void startTimerCounter();

#endif
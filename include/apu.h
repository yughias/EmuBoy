#ifndef __APU_H__
#define __APU_H__

#include <stdint.h>
#include <stdbool.h>

#define AUDIO_FREQUENCY   44100
#define AUDIO_SAMPLES      1024
#define AUDIO_BUFFER_SIZE  (AUDIO_SAMPLES*4)

#define APU_FREQUENCY 4194304

#define NR52_ADDR 0xFF26
#define NR51_ADDR 0xFF25
#define NR50_ADDR 0xFF24
#define NR10_ADDR 0xFF10
#define NR11_ADDR 0xFF11
#define NR12_ADDR 0xFF12
#define NR13_ADDR 0xFF13
#define NR14_ADDR 0xFF14
#define NR21_ADDR 0xFF16
#define NR22_ADDR 0xFF17
#define NR23_ADDR 0xFF18
#define NR24_ADDR 0xFF19
#define NR30_ADDR 0xFF1A
#define NR31_ADDR 0xFF1B
#define NR32_ADDR 0xFF1C
#define NR33_ADDR 0xFF1D
#define NR34_ADDR 0xFF1E
#define NR41_ADDR 0xFF20
#define NR42_ADDR 0xFF21
#define NR43_ADDR 0xFF22
#define NR44_ADDR 0xFF23

#define WAVE_RAM_START_ADDR 0xFF30
#define WAVE_RAM_SIZE 0x10

extern uint8_t NR52_REG;
extern uint8_t NR51_REG;
extern uint8_t NR50_REG;

extern uint8_t NR10_REG;
extern uint8_t NR11_REG;
extern uint8_t NR12_REG;
extern uint8_t NR13_REG;
extern uint8_t NR14_REG;

extern uint8_t NR21_REG;
extern uint8_t NR22_REG;
extern uint8_t NR23_REG;
extern uint8_t NR24_REG;

extern uint8_t NR30_REG;
extern uint8_t NR31_REG;
extern uint8_t NR32_REG;
extern uint8_t NR33_REG;
extern uint8_t NR34_REG;

extern uint8_t NR41_REG;
extern uint8_t NR42_REG;
extern uint8_t NR43_REG;
extern uint8_t NR44_REG;

extern uint8_t WAVE_RAM[WAVE_RAM_SIZE];

extern bool ch1_check_timer;
extern bool ch2_check_timer;
extern bool ch3_check_timer;
extern bool ch4_check_timer;

void initAudio();
void freeAudio();
void convertAudio();
void emulateApu();

#endif
#ifndef __APU_H__
#define __APU_H__

#include <stdint.h>
#include <stdbool.h>

#include "SDL2/SDL.h"

#define AUDIO_FREQUENCY   44100
#define AUDIO_SAMPLES     4096
#define AUDIO_BUFFER_SIZE  (AUDIO_SAMPLES*2)
#define PUSH_RATE_RELOAD   ((float)APU_FREQUENCY/AUDIO_FREQUENCY)

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

typedef struct apu_t {
    uint8_t NR52_REG;
    uint8_t NR51_REG;
    uint8_t NR50_REG;
    uint8_t NR10_REG;
    uint8_t NR11_REG;
    uint8_t NR12_REG;
    uint8_t NR13_REG;
    uint8_t NR14_REG;
    uint8_t NR21_REG;
    uint8_t NR22_REG;
    uint8_t NR23_REG;
    uint8_t NR24_REG;
    uint8_t NR30_REG;
    uint8_t NR31_REG;
    uint8_t NR32_REG;
    uint8_t NR33_REG;
    uint8_t NR34_REG;
    uint8_t NR41_REG;
    uint8_t NR42_REG;
    uint8_t NR43_REG;
    uint8_t NR44_REG;

    uint8_t WAVE_RAM[WAVE_RAM_SIZE];

    size_t pulse_wave_counter;
    size_t length_timer_counter;

    // channel 1: pulse with sweep pace
    bool ch1_on;
    uint16_t ch1_period_val;
    uint16_t ch1_length_timer;
    uint8_t ch1_volume;
    bool ch1_envelope_dir;
    uint8_t ch1_envelope_pace;
    uint16_t ch1_sweep_shadow;
    size_t ch1_envelope_counter;
    size_t ch1_waveduty_idx;
    size_t ch1_wavelevel_idx;
    size_t ch1_sweep_counter;
    uint8_t ch1_sweep_pace;
    bool ch1_sweep_dir;
    uint8_t ch1_sweep_slope;
    bool ch1_wave_level;
    int8_t ch1_sample;

    // channel 2: pulse
    bool ch2_on;
    uint16_t ch2_period_val;
    uint16_t ch2_length_timer;
    uint8_t ch2_volume;
    bool ch2_envelope_dir;
    uint8_t ch2_envelope_pace;
    size_t ch2_envelope_counter;
    size_t ch2_waveduty_idx;
    size_t ch2_wavelevel_idx;
    bool ch2_wave_level;
    int8_t ch2_sample;

    // channel 3: wave sound
    bool ch3_on;
    uint16_t ch3_period_val;
    uint16_t ch3_length_timer;
    int8_t ch3_sample;
    size_t ch3_wave_idx;

    // channel 4: noise
    bool ch4_on;
    uint16_t ch4_lfsr;
    size_t ch4_shift_counter;
    uint16_t ch4_length_timer;
    uint8_t ch4_volume;
    bool ch4_envelope_dir;
    uint8_t ch4_envelope_pace;
    size_t ch4_envelope_counter;
    int8_t ch4_sample;

    SDL_AudioDeviceID audioDev;
    int16_t buffer[AUDIO_BUFFER_SIZE];
    size_t bufIdx;
    float push_rate_counter;
    float push_rate_reload;
} apu_t;

void initAudio(apu_t*);
void freeAudio(apu_t*);
void convertAudio(apu_t*);
void emulateApu(apu_t*);

uint8_t getNR52(apu_t* apu);
uint8_t getNR10(apu_t* apu);
uint8_t getNR11(apu_t* apu);
uint8_t getNR14(apu_t* apu);
uint8_t getNR21(apu_t* apu);
uint8_t getNR24(apu_t* apu);
uint8_t getNR30(apu_t* apu);
uint8_t getNR32(apu_t* apu);
uint8_t getNR34(apu_t* apu);
uint8_t getNR44(apu_t* apu);

void checkDAC1(apu_t* apu);
void checkDAC2(apu_t* apu);
void checkDAC3(apu_t* apu);
void checkDAC4(apu_t* apu);

void setChannel1Timer(apu_t* apu);
void setChannel2Timer(apu_t* apu);
void setChannel3Timer(apu_t* apu);
void setChannel4Timer(apu_t* apu);

void triggerChannel1(apu_t* apu);
void triggerChannel2(apu_t* apu);
void triggerChannel3(apu_t* apu);
void triggerChannel4(apu_t* apu);

uint8_t getWaveRamAddress(apu_t* apu, uint8_t);

#endif
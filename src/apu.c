#include "apu.h"
#include <SDL2/SDL.h>
#include <stdio.h>

// apu internal variables and functions
static size_t getWavedutyIdx(uint8_t);
static uint16_t getPeriodValue(uint8_t, uint8_t);
static uint8_t getLengthTimer(uint8_t);
static uint8_t getInitialVolume(uint8_t);
static bool getEnvelopeDirection(uint8_t);
static uint8_t getEnvelopePace(uint8_t);
static uint8_t getSweepPace(uint8_t);
static bool getSweepDirection(uint8_t);
static uint8_t getSweepSlope(uint8_t);
static size_t getShiftCounter(uint8_t);
static void resetApuRegisters(apu_t* apu);
static void apuMixer(apu_t*, int16_t*);

static bool waveforms[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 }
};

#define trigger_pulse_ch(n) \
apu->ch ## n ## _period_val = getPeriodValue(apu->NR ## n ## 3_REG, apu->NR ## n ## 4_REG); \
apu->ch ## n ## _waveduty_idx = getWavedutyIdx(apu->NR ## n ## 1_REG); \
apu->ch ## n ## _wave_level = 0; \
apu->ch ## n ## _wavelevel_idx = 0; \
apu->ch ## n ## _on = true; \
apu->ch ## n ## _volume = getInitialVolume(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_dir = getEnvelopeDirection(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_pace = getEnvelopePace(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_counter = 65536 * apu->ch ## n ## _envelope_pace; \
if(!apu->ch ## n ## _length_timer) \
    apu->ch ## n ## _length_timer = 64; \


#define trigger_sweep() \
apu->ch1_sweep_pace = getSweepPace(apu->NR10_REG); \
apu->ch1_sweep_dir = getSweepDirection(apu->NR10_REG); \
apu->ch1_sweep_slope = getSweepSlope(apu->NR10_REG); \
apu->ch1_sweep_counter = 32768 * apu->ch1_sweep_pace; \
apu->ch1_sweep_shadow = getPeriodValue(apu->NR13_REG, apu->NR14_REG);

#define trigger_noise_ch(n) \
apu->ch ## n ## _on = true; \
apu->ch ## n ## _shift_counter = getShiftCounter(apu->NR ## n ## 3_REG); \
apu->ch ## n ## _volume = getInitialVolume(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_dir = getEnvelopeDirection(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_pace = getEnvelopePace(apu->NR ## n ## 2_REG); \
apu->ch ## n ## _envelope_counter = 65536 * apu->ch ## n ## _envelope_pace; \
if(!apu->ch ## n ## _length_timer) \
    apu->ch ## n ## _length_timer = 64; \

#define trigger_wave_ch(n) \
apu->ch ## n ## _period_val = getPeriodValue(apu->NR ## n ## 3_REG, apu->NR ## n ## 4_REG); \
apu->ch ## n ## _on = true; \
apu->ch ## n ## _wave_idx = 1; \
if(!apu->ch ## n ## _length_timer) \
    apu->ch ## n ## _length_timer = 256; \

#define pulse_wave_ch(n) \
if(apu->ch ## n ## _on){ \
    apu->ch ## n ## _period_val++; \
    if(apu->ch ## n ## _period_val == 2048){ \
        apu->ch ## n ## _period_val = getPeriodValue(apu->NR ## n ## 3_REG, apu->NR ## n ## 4_REG); \
        apu->ch ## n ## _wave_level = waveforms[apu->ch ## n ## _waveduty_idx][apu->ch ## n ## _wavelevel_idx]; \
        apu->ch ## n ## _wavelevel_idx = (apu->ch ## n ## _wavelevel_idx + 1) % 8; \
        apu->ch ## n ## _sample = apu->ch ## n ## _wave_level * apu->ch ## n ## _volume; \
    } \
} \

#define ram_wave_ch(n) \
if(apu->ch ## n ## _on){ \
    apu->ch ## n ## _period_val++; \
    if(apu->ch ## n ## _period_val >= 2048){ \
        apu->ch ## n ## _period_val = getPeriodValue(apu->NR ## n ## 3_REG, apu->NR ## n ## 4_REG); \
        uint8_t ram_idx = apu->ch ## n ## _wave_idx >> 1; \
        uint8_t sample = apu->WAVE_RAM[ram_idx]; \
        if(apu->ch ## n ## _wave_idx & 0b1) \
            sample &= 0x0F; \
        else \
            sample >>= 4; \
        apu->ch ## n ## _wave_idx = (apu->ch ## n ## _wave_idx + 1) % 32; \
        uint8_t volume = (apu->NR ## n ## 2_REG >> 4) & 0b11; \
        if(!volume) \
            apu->ch ## n ## _sample = 0; \
        else \
            apu->ch ## n ## _sample = sample >> (volume-1); \
    } \
} \

#define timer_update_ch(n) \
if(apu->NR ## n ## 4_REG & 0x40){ \
    if(apu->ch ## n ## _length_timer) \
        apu->ch ## n ## _length_timer--; \
    if(!apu->ch ## n ## _length_timer){ \
        apu->ch ## n ## _on = false; \
    } \
} \

#define envelope_update_ch(n) \
if(!apu->ch ## n ## _envelope_pace) \
    apu->ch ## n ## _volume = getInitialVolume(apu->NR ## n ## 2_REG); \
else if(!apu->ch ## n ## _envelope_counter){ \
    if(apu->ch ## n ## _envelope_dir){ \
        if(apu->ch ## n ## _volume != 15) \
            apu->ch ## n ## _volume++; \
    } else { \
        if(apu->ch ## n ## _volume != 0) \
            apu->ch ## n ## _volume--; \
    } \
    apu->ch ## n ## _envelope_counter = 65536 * apu->ch ## n ## _envelope_pace; \
} \

void initAudio(apu_t* apu){
    SDL_AudioSpec audioSpec;
    SDL_memset(&audioSpec, 0, sizeof(audioSpec));
    audioSpec.freq = AUDIO_FREQUENCY;
    audioSpec.format = AUDIO_S16;
    audioSpec.channels = 2;
    audioSpec.samples = AUDIO_SAMPLES;
    audioSpec.callback = NULL;
    apu->audioDev = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &audioSpec, 0);

    apu->push_rate_reload = PUSH_RATE_RELOAD;
    apu->push_rate_counter = apu->push_rate_reload;

    SDL_PauseAudioDevice(apu->audioDev, 0);
    apu->ch4_lfsr = 0xFFFF;
}

void emulateApu(apu_t* apu){
    // APU DISABLED CHECK
    if(!(apu->NR52_REG & 0x80)){
        resetApuRegisters(apu);
        apu->ch1_on = false;
        apu->ch2_on = false;
        apu->ch4_on = false;
        return;
    }

    if(apu->pulse_wave_counter % 2 == 0){
        ram_wave_ch(3);
    }

    if(!apu->pulse_wave_counter){
        pulse_wave_ch(1);
        pulse_wave_ch(2);

        apu->pulse_wave_counter = 4;
    }

    if(!apu->length_timer_counter){
        timer_update_ch(1);
        timer_update_ch(2);
        timer_update_ch(3);
        timer_update_ch(4);

        apu->length_timer_counter = 16384;
    }

    if((apu->NR10_REG & 0b1110000) && !apu->ch1_sweep_pace && !apu->ch1_sweep_dir && !apu->ch1_sweep_slope){
        trigger_sweep();
    }

    if(!apu->ch1_sweep_counter && (apu->NR10_REG & 0b1110000)){
        uint16_t tmp = apu->ch1_sweep_shadow >> apu->ch1_sweep_slope;
        if(apu->ch1_sweep_dir){
            apu->ch1_sweep_shadow -= tmp;
            if(apu->ch1_sweep_shadow == 0)
                apu->ch1_on = false;
        } else {
            apu->ch1_sweep_shadow += tmp;
            if(apu->ch1_sweep_shadow >= 2048)
                apu->ch1_on = false;
        }
        
        apu->NR13_REG = apu->ch1_sweep_shadow & 0xFF;
        apu->NR14_REG &= 0b11111000;
        apu->NR14_REG |= (apu->ch1_sweep_shadow >> 8) & 0b111;

        trigger_sweep();
    }

    if(!apu->ch4_shift_counter){
        if(apu->ch4_on){
            bool sample = !(apu->ch4_lfsr & 0b1);
            apu->ch4_sample = sample * apu->ch4_volume;

            bool xor = (apu->ch4_lfsr & 0b1) ^ ((apu->ch4_lfsr & 0b10) >> 1);
            apu->ch4_lfsr = (xor << 14) | (apu->ch4_lfsr >> 1);
            if(apu->NR43_REG & 0b1000)
                apu->ch4_lfsr |= xor << 7;
        }

        apu->ch4_shift_counter = getShiftCounter(apu->NR43_REG);
    }

    envelope_update_ch(1);
    envelope_update_ch(2);
    envelope_update_ch(4);

    apu->ch1_envelope_counter--;
    apu->ch2_envelope_counter--;
    apu->ch4_envelope_counter--;

    apu->ch1_sweep_counter--;
    apu->ch4_shift_counter--;

    apu->pulse_wave_counter--;
    apu->length_timer_counter--;
}

void convertAudio(apu_t* apu){
    if(apu->push_rate_counter <= 0){
        apu->push_rate_counter += apu->push_rate_reload;
        int16_t sample[2];
        apuMixer(apu, sample);

        apu->buffer[apu->bufIdx++] = sample[0];
        apu->buffer[apu->bufIdx++] = sample[1];

        if(apu->bufIdx == AUDIO_BUFFER_SIZE){
            SDL_QueueAudio(apu->audioDev, apu->buffer, apu->bufIdx * 2);
            apu->bufIdx = 0;
        }
    }

    apu->push_rate_counter -= 1;
}

void freeAudio(apu_t* apu){
    SDL_CloseAudioDevice(apu->audioDev);
}

static size_t getWavedutyIdx(uint8_t reg){
    return reg >> 6;
}

static uint16_t getPeriodValue(uint8_t lo_reg, uint8_t hi_reg){
    return ((hi_reg & 0b111) << 8) | lo_reg;
}

static uint8_t getLengthTimer(uint8_t reg){
    return reg & 0b111111;
}

static uint8_t getInitialVolume(uint8_t reg){
    return reg >> 4;
}

static bool getEnvelopeDirection(uint8_t reg){
    return reg & 0b1000;
}

static uint8_t getEnvelopePace(uint8_t reg){
    return reg & 0b111;
}

static uint8_t getSweepPace(uint8_t reg){
    return (reg >> 4) & 0b111;
}

static bool getSweepDirection(uint8_t reg){
    return reg & 0b1000;
}

static uint8_t getSweepSlope(uint8_t reg){
    return reg & 0b111;
}

static size_t getShiftCounter(uint8_t reg){
    static uint8_t noise_divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};
    uint8_t r = reg & 0b111;
    uint8_t s = reg >> 4;
    
    return noise_divisor[r] << s;
}

static void apuMixer(apu_t* apu, int16_t* samples){
    samples[0] = 0;
    samples[1] = 0;

    if(apu->NR51_REG & 1)
        samples[1] += apu->ch1_sample;
    if(apu->NR51_REG & (1 << 1))
        samples[1] += apu->ch2_sample;
    if(apu->NR51_REG & (1 << 2))
        samples[1] += apu->ch3_sample;
    if(apu->NR51_REG & (1 << 3))
        samples[1] += apu->ch4_sample;
    
    if(apu->NR51_REG & (1 << 4))
        samples[0] += apu->ch1_sample;
    if(apu->NR51_REG & (1 << 5))
        samples[0] += apu->ch2_sample;
    if(apu->NR51_REG & (1 << 6))
        samples[0] += apu->ch3_sample;
    if(apu->NR51_REG & (1 << 7))
        samples[0] += apu->ch4_sample;
    
    uint8_t volume[2];
    volume[0] = ((apu->NR50_REG >> 4) & 0b111) + 1;
    volume[1] = (apu->NR50_REG & 0b111) + 1;

    samples[0] *= volume[0];
    samples[1] *= volume[1];

    samples[0] *= 64;
    samples[1] *= 64;
}

uint8_t getNR52(apu_t* apu){
    return (apu->NR52_REG & 0x80) | 0x70 | (apu->ch4_on << 3) | (apu->ch3_on << 2) | (apu->ch2_on << 1) | (apu->ch1_on); 
}

uint8_t getNR10(apu_t* apu){ return apu->NR10_REG | 0x80; }
uint8_t getNR11(apu_t* apu){ return apu->NR11_REG | 0x3F; }
uint8_t getNR14(apu_t* apu){ return apu->NR14_REG | 0xBF; }

uint8_t getNR21(apu_t* apu){ return apu->NR21_REG | 0x3F; }
uint8_t getNR24(apu_t* apu){ return apu->NR24_REG | 0xBF; }

uint8_t getNR30(apu_t* apu){ return apu->NR30_REG | 0x7F; }
uint8_t getNR32(apu_t* apu){ return apu->NR32_REG | 0x9F; }
uint8_t getNR34(apu_t* apu){ return apu->NR34_REG | 0xBF; }

uint8_t getNR44(apu_t* apu){ return apu->NR44_REG | 0xBF; } 

static void resetApuRegisters(apu_t* apu){
    apu->NR52_REG = 0x00;
    apu->NR51_REG = 0x00;
    apu->NR50_REG = 0x00;
    apu->NR10_REG = 0x00;
    apu->NR11_REG = 0x00;
    apu->NR12_REG = 0x00;
    apu->NR13_REG = 0x00;
    apu->NR14_REG = 0x00;
    apu->NR21_REG = 0x00;
    apu->NR22_REG = 0x00;
    apu->NR23_REG = 0x00;
    apu->NR24_REG = 0x00;
    apu->NR30_REG = 0x00;
    apu->NR31_REG = 0x00;
    apu->NR32_REG = 0x00;
    apu->NR33_REG = 0x00;
    apu->NR34_REG = 0x00;
    apu->NR41_REG = 0x00;
    apu->NR42_REG = 0x00;
    apu->NR43_REG = 0x00;
    apu->NR44_REG = 0x00;   
}

void checkDAC1(apu_t* apu){ if(!(apu->NR12_REG >> 3)) apu->ch1_on = false; }
void checkDAC2(apu_t* apu){ if(!(apu->NR22_REG >> 3)) apu->ch2_on = false; }
void checkDAC3(apu_t* apu){ if(!(apu->NR30_REG & 0x80)) apu->ch3_on = false; }
void checkDAC4(apu_t* apu){ if(!(apu->NR42_REG >> 3)) apu->ch4_on = false; }

void triggerChannel1(apu_t* apu){
    if(!(apu->NR52_REG & 0x80))
        return;

    if(apu->NR14_REG & 0x80){
        apu->NR14_REG &= 0x7F;
        if(apu->NR12_REG >> 3){
            trigger_pulse_ch(1);
            trigger_sweep();
        }
    }
}

void triggerChannel2(apu_t* apu){
    if(!(apu->NR52_REG & 0x80))
        return;

    if(apu->NR24_REG & 0x80){
        apu->NR24_REG &= 0x7F;
        if(apu->NR22_REG >> 3){
            trigger_pulse_ch(2);
        }
    }
}

void triggerChannel3(apu_t* apu){
    if(!(apu->NR52_REG & 0x80))
        return;
    
    if(apu->NR34_REG & 0x80){
        apu->NR34_REG &= 0x7F;
        if(apu->NR30_REG & 0x80){
            trigger_wave_ch(3);
        }
    }
}

void triggerChannel4(apu_t* apu){
    if(!(apu->NR52_REG & 0x80))
        return;

    if(apu->NR44_REG & 0x80){
        apu->NR44_REG &= 0x7F;
        if(apu->NR42_REG >> 3){
            trigger_noise_ch(4);
        }
    }
}

void setChannel1Timer(apu_t* apu){ apu->ch1_length_timer = 64 - getLengthTimer(apu->NR11_REG); }

void setChannel2Timer(apu_t* apu){ apu->ch2_length_timer = 64 - getLengthTimer(apu->NR21_REG); }

void setChannel3Timer(apu_t* apu){ apu->ch3_length_timer = 256 - apu->NR31_REG; }

void setChannel4Timer(apu_t* apu){ apu->ch4_length_timer = 64 - getLengthTimer(apu->NR41_REG); }

void turnOffAudioChannels(apu_t* apu){
    apu->ch1_on = false;
    apu->ch2_on = false;
    apu->ch3_on = false;
    apu->ch4_on = false;
}

uint8_t getWaveRamAddress(apu_t* apu, uint8_t address){
    return apu->ch3_on ? apu->ch3_wave_idx >> 1 : address;
}
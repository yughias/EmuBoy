#include <apu.h>
#include <SDL2/SDL.h>
#include <stdio.h>

SDL_AudioSpec audioSpec;
SDL_AudioDeviceID audioDev;

int16_t buffer[AUDIO_BUFFER_SIZE];
size_t bufIdx = 0;

size_t audio_request_rate;
size_t audio_counter;

size_t queued_samples;
size_t queued_check_counter;
size_t queued_check_rate; 

// gameboy registers
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

// apu internal variables and functions
size_t getWavedutyIdx(uint8_t);
uint16_t getPeriodValue(uint8_t, uint8_t);
uint8_t getLengthTimer(uint8_t);
uint8_t getInitialVolume(uint8_t);
bool getEnvelopeDirection(uint8_t);
uint8_t getEnvelopePace(uint8_t);
uint8_t getSweepPace(uint8_t);
bool getSweepDirection(uint8_t);
uint8_t getSweepSlope(uint8_t);
size_t getShiftCounter(uint8_t);

void apuMixer(int16_t*);

bool waveforms[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 }
};

size_t pulse_wave_counter;
size_t length_timer_counter;

// channel 1: pulse with sweep pace
bool ch1_check_timer;
bool ch1_on;
uint16_t ch1_period_val;
uint8_t ch1_length_timer;
uint8_t ch1_volume;
bool ch1_envelope_dir;
uint8_t ch1_envelope_pace;
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
bool ch2_check_timer;
bool ch2_on;
uint16_t ch2_period_val;
uint8_t ch2_length_timer;
uint8_t ch2_volume;
bool ch2_envelope_dir;
uint8_t ch2_envelope_pace;
size_t ch2_envelope_counter;
size_t ch2_waveduty_idx;
size_t ch2_wavelevel_idx;
bool ch2_wave_level;
int8_t ch2_sample;

// channel 3: wave sound
bool ch3_check_timer;
bool ch3_on;
uint16_t ch3_period_val;
uint8_t ch3_length_timer;
int8_t ch3_sample;
size_t ch3_wave_idx;

// channel 4: noise
bool ch4_check_timer;
bool ch4_on;
uint16_t ch4_lfsr = 0xFFFF;
size_t ch4_shift_counter;
uint8_t ch4_length_timer;
uint8_t ch4_volume;
bool ch4_envelope_dir;
uint8_t ch4_envelope_pace;
size_t ch4_envelope_counter;
int8_t ch4_sample;


#define trigger_pulse_ch(n) \
ch ## n ## _period_val = getPeriodValue(NR ## n ## 3_REG, NR ## n ## 4_REG); \
ch ## n ## _waveduty_idx = getWavedutyIdx(NR ## n ## 1_REG); \
ch ## n ## _wave_level = 0; \
ch ## n ## _wavelevel_idx = 0; \
ch ## n ## _on = true; \
ch ## n ## _volume = getInitialVolume(NR ## n ## 2_REG); \
ch ## n ## _envelope_dir = getEnvelopeDirection(NR ## n ## 2_REG); \
ch ## n ## _envelope_pace = getEnvelopePace(NR ## n ## 2_REG); \
ch ## n ## _envelope_counter = 65536 * ch ## n ## _envelope_pace; \

#define trigger_sweep() \
ch1_sweep_pace = getSweepPace(NR10_REG); \
ch1_sweep_dir = getSweepDirection(NR10_REG); \
ch1_sweep_slope = getSweepSlope(NR10_REG); \
ch1_sweep_counter = 32768 * ch1_sweep_pace; \

#define trigger_noise_ch(n) \
ch ## n ## _on = true; \
ch ## n ## _shift_counter = getShiftCounter(NR ## n ## 3_REG); \
ch ## n ## _volume = getInitialVolume(NR ## n ## 2_REG); \
ch ## n ## _envelope_dir = getEnvelopeDirection(NR ## n ## 2_REG); \
ch ## n ## _envelope_pace = getEnvelopePace(NR ## n ## 2_REG); \
ch ## n ## _envelope_counter = 65536 * ch ## n ## _envelope_pace; \

#define trigger_wave_ch(n) \
ch ## n ## _period_val = getPeriodValue(NR ## n ## 3_REG, NR ## n ## 4_REG); \
ch ## n ## _on = true; \
ch ## n ## _wave_idx = 1; \

#define timer_check_ch(n) \
if(ch ## n ## _check_timer){ \
    ch ## n ## _check_timer = false; \
    ch ## n ## _length_timer = getLengthTimer(NR ## n ## 1_REG); \
}

#define pulse_wave_ch(n) \
if(ch ## n ## _on){ \
    ch ## n ## _period_val++; \
    if(ch ## n ## _period_val == 2048){ \
        ch ## n ## _period_val = getPeriodValue(NR ## n ## 3_REG, NR ## n ## 4_REG); \
        ch ## n ## _wave_level = waveforms[ch ## n ## _waveduty_idx][ch ## n ## _wavelevel_idx]; \
        ch ## n ## _wavelevel_idx = (ch ## n ## _wavelevel_idx + 1) % 8; \
        ch ## n ## _sample = ch ## n ## _wave_level * ch ## n ## _volume; \
    } \
} \

#define ram_wave_ch(n) \
if(ch ## n ## _on){ \
    ch ## n ## _period_val++; \
    if(ch ## n ## _period_val >= 2048){ \
        ch ## n ## _period_val = getPeriodValue(NR ## n ## 3_REG, NR ## n ## 4_REG); \
        uint8_t ram_idx = ch ## n ## _wave_idx >> 1; \
        uint8_t sample = WAVE_RAM[ram_idx]; \
        if(ch ## n ## _wave_idx & 0b1) \
            sample &= 0x0F; \
        else \
            sample >>= 4; \
        ch ## n ## _wave_idx = (ch ## n ## _wave_idx + 1) % 32; \
        uint8_t volume = (NR ## n ## 2_REG >> 4) & 0b11; \
        if(!volume) \
            ch ## n ## _sample = 0; \
        else \
            ch ## n ## _sample = sample >> (volume-1); \
    } \
} \

#define timer_update_ch(n) \
if(NR ## n ## 4_REG & 0x40){ \
    ch ## n ## _length_timer++; \
    if(ch ## n ## _length_timer == 64){ \
        ch ## n ## _on = false; \
        ch ## n ## _length_timer = 0; \
    } \
} \

#define envelope_update_ch(n) \
if(!ch ## n ## _envelope_pace) \
    ch ## n ## _volume = getInitialVolume(NR ## n ## 2_REG); \
else if(!ch ## n ## _envelope_counter){ \
    if(ch ## n ## _envelope_dir){ \
        if(ch ## n ## _volume != 15) \
            ch ## n ## _volume++; \
    } else { \
        if(ch ## n ## _volume != 0) \
            ch ## n ## _volume--; \
    } \
    ch ## n ## _envelope_counter = 65536 * ch ## n ## _envelope_pace; \
} \

void initAudio(){
    SDL_memset(&audioSpec, 0, sizeof(audioSpec));
    audioSpec.freq = AUDIO_FREQUENCY;
    audioSpec.format = AUDIO_S16;
    audioSpec.channels = 2;
    audioSpec.samples = AUDIO_SAMPLES;
    audioSpec.callback = NULL;
    audioDev = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &audioSpec, 0);

    audio_request_rate = APU_FREQUENCY / AUDIO_FREQUENCY;
    audio_counter = 0;

    queued_check_rate = audio_request_rate * AUDIO_SAMPLES;
    queued_check_counter = 0;

    SDL_PauseAudioDevice(audioDev, 0);
}

void emulateApu(){
    // APU DISABLED CHECK

    if(!(NR52_REG & 0x80)){
        ch1_on = false;
        ch2_on = false;
        ch4_on = false;
        return;
    }

    // DAC DISABLED CHECK

    if(!(NR12_REG >> 3)){
        ch1_on = false;
    }

    if(!(NR22_REG) >> 3){
        ch2_on = false;
    }

    if(!(NR30_REG & 0x80)){
        ch3_on = false;
    }

    if(!(NR42_REG) >> 3){
        ch4_on = false;
    }

    // TRIGGER CHECK

    if(NR14_REG & 0x80){
        NR14_REG &= 0x7F;
        trigger_pulse_ch(1);
        trigger_sweep();
    }

    if(NR24_REG & 0x80){
        NR24_REG &= 0x7F;
        trigger_pulse_ch(2);
    }

    if(NR34_REG & 0x80){
        NR34_REG &= 0x7F;
        trigger_wave_ch(3);
    }

    if(NR44_REG & 0x80){
        NR44_REG &= 0x7F;
        trigger_noise_ch(4);
    }

    timer_check_ch(1);
    timer_check_ch(2);
    timer_check_ch(3);
    timer_check_ch(4);

    if(pulse_wave_counter % 2 == 0){
        ram_wave_ch(3);
    }

    if(!pulse_wave_counter){
        pulse_wave_ch(1);
        pulse_wave_ch(2);

        pulse_wave_counter = 4;
    }

    if(!length_timer_counter){
        timer_update_ch(1);
        timer_update_ch(2);
        timer_update_ch(3);
        timer_update_ch(4);

        length_timer_counter = 16384;
    }

    if((NR10_REG & 0b1110000) && !ch1_sweep_pace && !ch1_sweep_dir && !ch1_sweep_slope){
        trigger_sweep();
    }

    if(!ch1_sweep_counter && (NR10_REG & 0b1110000)){
        uint16_t tmp = ch1_period_val >> ch1_sweep_slope;
        if(ch1_sweep_dir){
            ch1_period_val -= tmp;
            if(ch1_period_val == 0)
                ch1_on = false;
        } else {
            ch1_period_val += tmp;
            if(ch1_period_val >= 2048)
                ch1_on = false;
        }
        
        NR13_REG = ch1_period_val & 0xFF;
        NR14_REG &= 0b11111000;
        NR14_REG |= (ch1_period_val >> 8) & 0b111;

        trigger_sweep();
    }

    if(!ch4_shift_counter && ch4_on){
        bool sample = !(ch4_lfsr & 0b1);
        ch4_sample = sample * ch4_volume;

        bool xor = (ch4_lfsr & 0b1) ^ ((ch4_lfsr & 0b10) >> 1);
        ch4_lfsr = (xor << 14) | (ch4_lfsr >> 1);
        if(NR43_REG & 0b1000){
            ch4_lfsr &= 0xFFDF;
            ch4_lfsr |= xor << 7;
        }

        ch4_shift_counter = getShiftCounter(NR43_REG);
    }

    envelope_update_ch(1);
    envelope_update_ch(2);
    envelope_update_ch(4);

    ch1_envelope_counter--;
    ch2_envelope_counter--;
    ch4_envelope_counter--;

    ch1_sweep_counter--;
    ch4_shift_counter--;

    pulse_wave_counter--;
    length_timer_counter--;
}

void convertAudio(){
    if(!audio_counter && bufIdx < AUDIO_BUFFER_SIZE){
        int16_t samples[2];
        apuMixer(samples);
        buffer[bufIdx++] = samples[0];
        buffer[bufIdx++] = samples[1];
        audio_counter = audio_request_rate;
    } 

    if(bufIdx >= AUDIO_SAMPLES && queued_samples < AUDIO_BUFFER_SIZE*2){
        SDL_QueueAudio(audioDev, &buffer, bufIdx*2);
        bufIdx = 0;
    }

    if(!queued_check_counter){
        queued_samples = SDL_GetQueuedAudioSize(audioDev);
        queued_check_counter = queued_check_rate;
    }

    if(audio_counter)
        audio_counter--;

    if(queued_check_counter)
        queued_check_counter--;
}

void freeAudio(){
    SDL_CloseAudioDevice(audioDev);
}

size_t getWavedutyIdx(uint8_t reg){
    return reg >> 6;
}

uint16_t getPeriodValue(uint8_t lo_reg, uint8_t hi_reg){
    return ((hi_reg & 0b111) << 8) | lo_reg;
}

uint8_t getLengthTimer(uint8_t reg){
    return reg & 0b111111;
}

uint8_t getInitialVolume(uint8_t reg){
    return reg >> 4;
}

bool getEnvelopeDirection(uint8_t reg){
    return reg & 0b1000;
}

uint8_t getEnvelopePace(uint8_t reg){
    return reg & 0b111;
}

uint8_t getSweepPace(uint8_t reg){
    return (reg >> 4) & 0b111;
}

bool getSweepDirection(uint8_t reg){
    return reg & 0b1000;
}

uint8_t getSweepSlope(uint8_t reg){
    return reg & 0b111;
}

size_t getShiftCounter(uint8_t reg){
    static uint8_t noise_divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};
    uint8_t r = reg & 0b111;
    uint8_t s = reg >> 4;
    
    return noise_divisor[r] << s;
}

void apuMixer(int16_t* samples){
    samples[0] = 0;
    samples[1] = 0;

    if(NR51_REG & 1)
        samples[1] += ch1_sample;
    if(NR51_REG & (1 << 1))
        samples[1] += ch2_sample;
    if(NR51_REG & (1 << 2))
        samples[1] += ch3_sample;
    if(NR51_REG & (1 << 3))
        samples[1] += ch4_sample;
    
    if(NR51_REG & (1 << 4))
        samples[0] += ch1_sample;
    if(NR51_REG & (1 << 5))
        samples[0] += ch2_sample;
    if(NR51_REG & (1 << 6))
        samples[0] += ch3_sample;
    if(NR51_REG & (1 << 7))
        samples[0] += ch4_sample;
    
    uint8_t volume[2];
    volume[0] = ((NR50_REG >> 4) & 0b111) + 1;
    volume[1] = (NR50_REG & 0b111) + 1;

    samples[0] *= volume[0];
    samples[1] *= volume[1];

    samples[0] *= 64;
    samples[1] *= 64;
}
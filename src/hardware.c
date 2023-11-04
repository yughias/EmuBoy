#include <hardware.h>
#include <stdio.h>
#include <stdlib.h>

size_t timer_counter = 0;

cpu_t cpu = {
    .readByte  = readByte,
    .writeByte = writeByte,
    .tickSystem = tickHardware 
};

void emulateCpu(cpu_t* cpu){
    if(!cpu->cycles){
        stepCPU(cpu);
        #ifdef DEBUG
        infoCPU(cpu);
        #endif
    }   

    if(cpu->cycles)
        cpu->cycles--;
}

void emulateHardware(cpu_t* cpu){
    while(cpu->cycles < CYCLES_PER_FRAME)
        stepCPU(cpu);
    cpu->cycles -= CYCLES_PER_FRAME;
}

void tickHardware(int ticks){
    cpu.cycles += ticks;

    if(writeToDMA){
        startDMA();
        writeToDMA = false;
    }

    if(gb_timer.tma_overwritten){
        TIMA_REG = TMA_REG;
        gb_timer.tma_overwritten = false;
    }

    for(int i = 0; i < ticks; i++){
        updateSerial();
        emulateApu();
        convertAudio();
        updatePPU();
        updateTimer();
    }
}
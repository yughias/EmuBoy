#include <hardware.h>
#include <stdio.h>
#include <stdlib.h>

size_t timer_counter = 0;

cpu_t cpu = {
    .readByte  = readByte,
    .writeByte = writeByte,
    .tickSystem = tickHardware 
};

void emulateHardware(cpu_t* cpu){
    while(cpu->cycles < CYCLES_PER_FRAME)
        stepCPU(cpu);
    cpu->cycles -= CYCLES_PER_FRAME;
}

void tickHardware(int ticks){
    cpu.cycles += ticks;

    for(int i = 0; i < ticks; i++){
        updateSerial();
        emulateApu();
        convertAudio();
        updatePPU();
        updateTimer();
    }
}
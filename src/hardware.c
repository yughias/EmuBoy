#include <hardware.h>
#include <stdio.h>
#include <stdlib.h>

size_t timer_counter = 0;

cpu_t cpu = {
    .readMemory  = getReadAddress,
    .writeMemory = getWriteAddress
};

void emulateCpu(cpu_t* cpu){
    if(!cpu->cycles){
        #ifdef DEBUG
        infoCPU(cpu);
        #endif
        stepCPU(cpu);

        if(writeToDMA){
            startDMA();
            writeToDMA = false;
        }

        if(gb_timer.tma_overwritten){
            TIMA_REG = TMA_REG;
            gb_timer.tma_overwritten = false;
        }
    
    }   

    if(cpu->cycles)
        cpu->cycles--;
}

void emulateHardware(cpu_t* cpu){
    for(size_t i = 0; i < CYCLES_PER_FRAME; i++){
        if((SC_REG & 0x80) && (SC_REG & 0x01)){
            SC_REG &= 0x7F;
            IF_REG |= SERIAL_IRQ;
        }

        emulateApu();
        convertAudio();
        updatePPU();
        updateTimer();
        emulateCpu(cpu);
    }
}
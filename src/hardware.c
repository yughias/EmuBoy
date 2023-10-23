#include <hardware.h>
#include <stdio.h>
#include <stdlib.h>

size_t timer_counter = 0;
size_t div_counter = 0;

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
    
    }   

    if(cpu->cycles)
        cpu->cycles--;
}

void startTimerCounter(){
    switch(TAC_REG & TIMER_CLOCK_MASK){
        case 0:
        timer_counter = 1024;
        break;

        case 1:
        timer_counter = 16;
        break;

        case 2:
        timer_counter = 64;
        break;

        case 3:
        timer_counter = 256;
        break;
    }
}

void updateTimer(){
    if(!timer_counter){
        if(TAC_REG & TIMER_ENABLE_MASK){
            if(TIMA_REG == 0xFF){
                    IF_REG |= TIMER_IRQ;
                    TIMA_REG = TMA_REG;
                } else
                    TIMA_REG++;
        }

        startTimerCounter();
    } else
        timer_counter--;
}

void updateDiv(){
    if(!div_counter){
        DIV_REG++;
        div_counter = DIV_INCREMENT_RATE;
    }

    div_counter--;
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
        updateDiv();
        emulateCpu(cpu);
    }
}
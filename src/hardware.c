#include <hardware.h>
#include <stdio.h>
#include <stdlib.h>

size_t timer_counter = 0;
size_t div_counter = 0;

cpu_t cpu = {
    .readMemory  = getReadAddress,
    .writeMemory = getWriteAddress
};

size_t master_cycle = 0;

void emulateCpu(cpu_t* cpu){
    if(!cpu->cycles){
        #ifdef DEBUG
        if(!bootromEnabled){
            infoCPU(cpu);
        }
        stepCPU(cpu);
        if(!bootromEnabled){
            master_cycle += cpu->cycles;
            fprintf(stderr, "%llu\n", master_cycle);
        }
        #else
        stepCPU(cpu);
        #endif

        if(writeToDMA){
            startDMA();
            writeToDMA = false;
        }
    
    }   

    if(cpu->cycles)
        cpu->cycles--;
}

void updateTimer(){
    TAC_REG |= 0b11111000;

    if(TAC_REG & TIMER_ENABLE_MASK){
        if(!timer_counter){
            if(TIMA_REG == 0xFF){
                IF_REG |= TIMER_IRQ;
                TIMA_REG = TMA_REG;
            } 
            TIMA_REG++;

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
        
        timer_counter--;
    }
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
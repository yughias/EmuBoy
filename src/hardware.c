#include "hardware.h"
#include "gameshark.h"

#include <stdio.h>
#include <stdlib.h>

cpu_t cpu = {
    .readByte  = readByte,
    .writeByte = writeByte,
    .tickSystem = tickHardware 
};

CONSOLE_TYPE console_type = DMG_TYPE;

void emulateHardware(cpu_t* cpu){
    while(cpu->cycles < CYCLES_PER_FRAME){
        #ifdef DEBUG
        infoCPU(cpu);
        #endif
        stepCPU(cpu);
    }
    cpu->cycles -= CYCLES_PER_FRAME;
}

void tickHardware(int ticks){
    if(KEY1_REG & 0x80)
        ticks /= 2;

    cpu.cycles += ticks;

    for(int i = 0; i < ticks; i++){
        updateSerial();
        emulateApu();
        convertAudio();
        updatePPU();
        updateTimer();
    }
}

void initConsole(){
    initCPU(&cpu);
    initMemory(romName);
    initHDMA();
    initLcdcMasks();
    initColorPalette();

    char gamesharkName[FILENAME_MAX];
    getAbsoluteDir(gamesharkName);
    strcat(gamesharkName, "data/gameshark.txt");
    loadGameShark(gamesharkName);

    renderLcdOff();
}
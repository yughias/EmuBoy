#include "gb.h"
#include "gameshark.h"

#include <stdio.h>
#include <stdlib.h>

void emulateHardware(gb_t* gb){
    sm83_t* cpu = &gb->cpu;
    while(cpu->cycles < CYCLES_PER_FRAME){
        #ifdef DEBUG
        infoCPU(cpu);
        #endif
        stepCPU(cpu);
    }
    gb->startFrame_clock += cpu->cycles;
    cpu->cycles -= CYCLES_PER_FRAME;
}

static void tickHardware(void* ctx, int ticks){
    gb_t* gb = (gb_t*)ctx;
    apu_t* apu = &gb->apu;
    bool fast_mode = gb->KEY1_REG & 0x80;

    if(fast_mode)
        ticks /= 2;
       
    gb->cpu.cycles += ticks;

    for(int i = 0; i < ticks; i++){
        updateSerial(gb);
        emulateApu(apu);
        convertAudio(apu);
        updatePPU(gb);
        updateTimer(gb);
    }
    
    if(fast_mode){
        for(int i = 0; i < ticks; i++){
            updateSerial(gb);
            updateTimer(gb);
        }
    }
}

void initConsole(gb_t* gb){
    memset(gb, 0, sizeof(gb_t));
    sm83_t* cpu = &gb->cpu;
    cpu->readByte = readByte;
    cpu->writeByte = writeByte;
    cpu->tickSystem = tickHardware;
    cpu->ctx = gb;
    initCPU(cpu);
    initMemory(gb, romName);
    initAudio(&gb->apu);
    initHDMA(&gb->dma);
    initLcdcMasks(gb);
    initColorPalette(gb);
    gb->ppu.workingBufferPtr = gb->ppu.workingBuffer;
    gb->ppu.renderBufferPtr = gb->ppu.renderBuffer;

    char gamesharkName[FILENAME_MAX];
    getAbsoluteDir(gamesharkName);
    strcat(gamesharkName, "data/gameshark.txt");
    loadGameShark(gamesharkName);

    renderLcdOff(&gb->ppu);
}
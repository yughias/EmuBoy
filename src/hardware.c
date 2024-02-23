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
    cpu.cycles += ticks;

    for(int i = 0; i < ticks; i++){
        updateSerial();
        emulateApu();
        convertAudio();
        updatePPU();
        updateTimer();
    }
}

void skipBootrom(){
    cpu.AF = 0x01B0;
    cpu.BC = 0x0013;
    cpu.DE = 0x00D8;
    cpu.HL = 0x014D;
    cpu.SP = 0xFFFE;
    cpu.PC = 0x0100;
    
    cpu.Z_FLAG = true;
    cpu.N_FLAG = false;
    cpu.H_FLAG = true;
    cpu.C_FLAG = true;

    cpu.HALTED = false;
    cpu.IME = false;
    cpu.EI_DELAY = false;
    
    SB_REG = 0x00;
    SC_REG = 0x7E;
    gb_timer.div = 0xAB;
    TIMA_REG = 0x00;
    TMA_REG = 0x00;
    TAC_REG = 0xF8;
    cpu.IF = 0xE1;
    NR10_REG = 0x80;
    NR11_REG = 0xBF;
    NR12_REG = 0xF3;
    NR13_REG = 0xFF;
    NR14_REG = 0xBF;
    NR21_REG = 0x3F;
    NR22_REG = 0x00;
    NR23_REG = 0xFF;
    NR24_REG = 0xBF;
    NR30_REG = 0x7F;
    NR31_REG = 0xFF;
    NR32_REG = 0x9F;
    NR33_REG = 0xFF;
    NR34_REG = 0xBF;
    NR41_REG = 0xFF;
    NR42_REG = 0x00;
    NR43_REG = 0x00;
    NR44_REG = 0xBF;
    NR50_REG = 0x77;
    NR51_REG = 0xF3;
    NR52_REG = 0xF1;
    LCDC_REG = 0x91;
    STAT_REG = 0x85;
    SCY_REG	= 0x00;
    SCX_REG	= 0x00;
    LY_REG	= 0x00;
    LYC_REG	= 0x00;
    DMA_REG	= 0xFF;
    BGP_REG = 0xFC;
    OBP0_REG = 0x00;
    OBP1_REG = 0x00;
    WY_REG = 0x00;
    WX_REG = 0x00;
    cpu.IE = 0x00;
}

void initConsole(){
    initCPU(&cpu);
    initMemory(romName);

    char bootromName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    strcat(bootromName, "data/dmg_boot.bin");
    if(console_type == DMG_TYPE)
        if(!loadBootRom(bootromName))
            skipBootrom();

    char gamesharkName[FILENAME_MAX];
    getAbsoluteDir(gamesharkName);
    strcat(gamesharkName, "data/gameshark.txt");
    loadGameShark(gamesharkName);

}
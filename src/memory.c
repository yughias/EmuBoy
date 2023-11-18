#include <hardware.h>
#include <info.h>

#include "memory_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t BOOTROM_DISABLE_REG;
uint8_t NOT_MAPPED;

uint8_t BOOTROM[BOOTROM_SIZE];
uint8_t* ROM;
size_t ROM_SIZE;

size_t ERAM_SIZE;
uint8_t ERAM[MAX_ERAM_SIZE];

uint8_t VRAM[VRAM_SIZE];
uint8_t WRAM[WRAM_SIZE];
uint8_t OAM[OAM_SIZE];
uint8_t HRAM[HRAM_SIZE];

char romName[FILENAME_MAX];
char savName[FILENAME_MAX];

void initMemory(const char* romName){
    loadRom(romName);
    LY_REG = 0;
    STAT_REG = 0;
    TIMA_REG = 0;
    TMA_REG = 0;
    TAC_REG = 0xF8;
    JOYP_REG = 0xFF;
    ppu_mode = OAM_SCAN_MODE;
    lyc_compare = false;
    internal_ly = 0;
    stat_irq = false;
    gb_timer.counter = 0x00;
    gb_timer.old_state = false;
    gb_timer.delay = 0x00;
    gb_timer.ignore_write = false;

    ERAM_SIZE = getRamSize(ROM);
    detectMBC();
    if(hasBattery)
        loadSav(savName);

    fillReadTable(0x00, 0x40, mbc_mapper_0000_3FFF);
    fillReadTable(0x40, 0x80, mbc_mapper_4000_7FFF);
    fillReadTable(0x80, 0xA0, readVram);
    fillReadTable(0xA0, 0xC0, readEram);
    fillReadTable(0xC0, 0xFE, readWram);
    fillReadTable(0xFE, 0xFE, readOam);
    fillReadTable(0xFF, 0xFF, readIO);

    fillWriteTable(0x00, 0x80, mbc_rom_write);
    fillWriteTable(0x80, 0xA0, writeVram);
    fillWriteTable(0xA0, 0xC0, writeEram);
    fillWriteTable(0xC0, 0xFE, writeWram);
    fillWriteTable(0xFE, 0xFE, writeOam);
    fillWriteTable(0xFF, 0xFF, writeIO);
}

void freeMemory(){
    free(ROM);
}

uint8_t readByte(uint16_t address){
    return (*readTable[address >> 8])(address);
}

void writeByte(uint16_t address, uint8_t byte){
    (*writeTable[address >> 8])(address, byte);
}

void loadRom(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    fseek(fptr, 0, SEEK_END);
    ROM_SIZE = ftell(fptr);
    rewind(fptr);

    ROM = (uint8_t*)malloc(ROM_SIZE);
    fread(ROM, 1, ROM_SIZE, fptr);
    fclose(fptr);
}

bool loadBootRom(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    if(!fptr)
        return false;
    fread(BOOTROM, 1, BOOTROM_SIZE, fptr);
    fclose(fptr);
    fillReadTable(0x00, 0x00, readBootrom);
    return true;
}

void loadSav(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    fread(ERAM, 1, ERAM_SIZE, fptr);
    fclose(fptr);
}

void saveSav(const char* filename){
    FILE* fptr = fopen(filename, "wb");
    fwrite(ERAM, 1, ERAM_SIZE, fptr);
    fclose(fptr);
}

void setFilename(const char* filename){
    size_t filename_len = strlen(filename);
    strcpy(romName, filename);
    strcpy(savName, filename);
    if(savName[filename_len-1] == 'b')
        strcpy(&savName[filename_len-2], "sav");
    else 
        strcpy(&savName[filename_len-3], "sav");
}
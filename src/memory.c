#include "hardware.h"
#include "info.h"
#include "bootrom_skip.h"
#include "memory_table.h"
#include "mbcs/mbc3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t BOOTROM_DISABLE_REG;

uint8_t* BOOTROM;
size_t BOOTROM_SIZE;

uint8_t* ROM;
size_t ROM_SIZE;

size_t ERAM_SIZE;
uint8_t ERAM[MAX_ERAM_SIZE];

uint8_t VRAM[VRAM_SIZE];
uint8_t WRAM[WRAM_SIZE];
uint8_t OAM[OAM_SIZE];
uint8_t HRAM[HRAM_SIZE];

uint8_t BGP_CRAM[CRAM_SIZE];
uint8_t OBP_CRAM[CRAM_SIZE];

uint8_t SVBK_REG;
uint8_t VBK_REG;
uint8_t KEY0_REG;
uint8_t KEY1_REG;

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
    SVBK_REG = 0x00;
    VBK_REG = 0x00;
    BCPS_REG = 0x00;
    OCPS_REG = 0x00;
    KEY0_REG = 0x00;
    KEY1_REG = 0x00;
    DMA_REG = 0x00;
    ppu_mode = OAM_SCAN_MODE;
    lyc_compare = false;
    internal_ly = 0;
    stat_irq = false;
    gb_timer.counter = 0x00;
    gb_timer.old_state = false;
    gb_timer.delay = 0x00;
    gb_timer.ignore_write = false;

    memset(OAM, 0, OAM_SIZE);
    memset(WRAM, 0, WRAM_SIZE);
    memset(VRAM, 0, VRAM_SIZE);

    ERAM_SIZE = getRamSize(ROM);
    detectConsoleAndMbc();
    if(hasBattery)
        loadSav(savName);
    if(hasRtc)
        loadRtc(savName);

    fillReadTable(0x00, 0x40, mbc_mapper_0000_3FFF);
    fillReadTable(0x40, 0x80, mbc_mapper_4000_7FFF);
    fillReadTable(0x80, 0xA0, readVram);
    fillReadTable(0xA0, 0xC0, mbc_mapper_A000_BFFF_read);
    fillReadTable(0xC0, 0xE0, readWram);
    fillReadTable(0xE0, 0xFE, readMirrorRam);
    fillReadTable(0xFE, 0xFE, readOam);
    fillReadTable(0xFF, 0xFF, readIO);

    fillWriteTable(0x00, 0x80, mbc_rom_write);
    fillWriteTable(0x80, 0xA0, writeVram);
    fillWriteTable(0xA0, 0xC0, mbc_mapper_A000_BFFF_write);
    fillWriteTable(0xC0, 0xE0, writeWram);
    fillWriteTable(0xE0, 0xFE, writeMirrorRam);
    fillWriteTable(0xFE, 0xFE, writeOam);
    fillWriteTable(0xFF, 0xFF, writeIO);

    char bootromName[FILENAME_MAX];
    char iniName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    if(console_type != MEGADUCK_TYPE){
        if(console_type == CGB_TYPE){
            strcat(bootromName, "data/cgb_boot.bin");
            if(!loadBootRom(bootromName))
                skipCgbBootrom();
        } else {
            strcat(bootromName, "data/dmg_boot.bin");
            if(!loadBootRom(bootromName))
                skipDmgBootrom();
        }
    }
}

void freeMemory(){
    free(ROM);
    free(BOOTROM);
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

    fseek(fptr, 0, SEEK_END);
    BOOTROM_SIZE = ftell(fptr);
    rewind(fptr);

    BOOTROM = malloc(BOOTROM_SIZE);

    fread(BOOTROM, 1, BOOTROM_SIZE, fptr);
    fclose(fptr);
    fillReadTable(0x00, 0x00, readBootrom);
    fillReadTable(0x02, 0x09, readBootrom);
    return true;
}

void loadSav(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    if(!fptr)
        return;
    fread(ERAM, 1, ERAM_SIZE, fptr);
    fclose(fptr);
}

void saveSav(const char* filename){
    FILE* fptr = fopen(filename, "wb");
    if(!fptr)
        return;
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
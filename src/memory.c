#include "gb.h"
#include "info.h"
#include "bootrom_skip.h"
#include "memory_table.h"
#include "mbcs/mbc3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char romName[FILENAME_MAX];
char savName[FILENAME_MAX];

void initMemory(gb_t* gb, const char* romName){
    loadRom(gb, romName);
    ppu_t* ppu = &gb->ppu;
    ppu->LY_REG = 0;
    ppu->STAT_REG = 0;
    ppu->mode = OAM_SCAN_MODE;
    ppu->lyc_compare = false;
    ppu->internal_ly = 0;
    ppu->stat_irq = false;
    ppu->BCPS_REG = 0x00;
    ppu->OCPS_REG = 0x00;
    gb->joypad.JOYP_REG = 0xFF;
    gb->SVBK_REG = 0x00;
    gb->VBK_REG = 0x00;
    gb->KEY0_REG = 0x00;
    gb->KEY1_REG = 0x00;
    gb->dma.DMA_REG = 0x00;
    gb_timer_t* tmr = &gb->timer;
    tmr->counter = 0x00;
    tmr->old_state = false;
    tmr->delay = 0x00;
    tmr->ignore_write = false;
    tmr->TIMA_REG = 0;
    tmr->TMA_REG = 0;
    tmr->TAC_REG = 0xF8;

    memset(gb->OAM, 0, OAM_SIZE);
    memset(gb->WRAM, 0, WRAM_SIZE);
    memset(gb->VRAM, 0, VRAM_SIZE);

    gb->ERAM_SIZE = getRamSize(gb->ROM);
    mbc_t* mbc = &gb->mbc;
    detectConsoleAndMbc(gb);
    if(mbc->hasBattery)
        loadSav(gb, savName);
    if(mbc->hasRtc)
        loadRtc(mbc->data, savName);

    readGbFunc* readTable = gb->readTable;
    fillReadTable(readTable, 0x00, 0x40, mbc->mapper_0000_3FFF);
    fillReadTable(readTable, 0x40, 0x80, mbc->mapper_4000_7FFF);
    fillReadTable(readTable, 0x80, 0xA0, readVram);
    fillReadTable(readTable, 0xA0, 0xC0, mbc->mapper_A000_BFFF_read);
    fillReadTable(readTable, 0xC0, 0xE0, readWram);
    fillReadTable(readTable, 0xE0, 0xFE, readMirrorRam);
    fillReadTable(readTable, 0xFE, 0xFE, readOam);
    fillReadTable(readTable, 0xFF, 0xFF, readIO);

    writeGbFunc* writeTable = gb->writeTable;
    fillWriteTable(writeTable, 0x00, 0x80, mbc->rom_write);
    fillWriteTable(writeTable, 0x80, 0xA0, writeVram);
    fillWriteTable(writeTable, 0xA0, 0xC0, mbc->mapper_A000_BFFF_write);
    fillWriteTable(writeTable, 0xC0, 0xE0, writeWram);
    fillWriteTable(writeTable, 0xE0, 0xFE, writeMirrorRam);
    fillWriteTable(writeTable, 0xFE, 0xFE, writeOam);
    fillWriteTable(writeTable, 0xFF, 0xFF, writeIO);

    char bootromName[FILENAME_MAX];
    char iniName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    if(gb->console_type != MEGADUCK_TYPE){
        if(gb->console_type == CGB_TYPE){
            strcat(bootromName, "data/cgb_boot.bin");
            if(!loadBootRom(gb, bootromName))
                skipCgbBootrom(gb);
        } else {
            strcat(bootromName, "data/dmg_boot.bin");
            if(!loadBootRom(gb, bootromName))
                skipDmgBootrom(gb);
        }
    }
}

void freeMemory(gb_t* gb){
    free(gb->ROM);
    free(gb->BOOTROM);
    free(gb->mbc.data);
}

uint8_t readByte(void* ctx, uint16_t address){
    gb_t* gb = (gb_t*)ctx;
    return (*gb->readTable[address >> 8])((gb_t*)ctx, address);
}

void writeByte(void* ctx, uint16_t address, uint8_t byte){
    gb_t* gb = (gb_t*)ctx;
    (*gb->writeTable[address >> 8])((gb_t*)ctx, address, byte);
}

void loadRom(gb_t* gb, const char* filename){
    FILE* fptr = fopen(filename, "rb");
    if(!fptr){
        printf("Failed to open ROM file\n");
        exit(EXIT_FAILURE);
    }
    fseek(fptr, 0, SEEK_END);
    gb->ROM_SIZE = ftell(fptr);
    rewind(fptr);

    gb->ROM = (uint8_t*)malloc(gb->ROM_SIZE);
    fread(gb->ROM, 1, gb->ROM_SIZE, fptr);
    fclose(fptr);
}

bool loadBootRom(gb_t* gb, const char* filename){
    FILE* fptr = fopen(filename, "rb");
    if(!fptr)
        return false;

    fseek(fptr, 0, SEEK_END);
    gb->BOOTROM_SIZE = ftell(fptr);
    rewind(fptr);

    gb->BOOTROM = malloc(gb->BOOTROM_SIZE);

    fread(gb->BOOTROM, 1, gb->BOOTROM_SIZE, fptr);
    fclose(fptr);
    readGbFunc* readTable = gb->readTable;
    fillReadTable(readTable, 0x00, 0x00, readBootrom);
    fillReadTable(readTable, 0x02, 0x09, readBootrom);
    return true;
}

void loadSav(gb_t* gb, const char* filename){
    FILE* fptr = fopen(filename, "rb");
    if(!fptr)
        return;
    fread(gb->ERAM, 1, gb->ERAM_SIZE, fptr);
    fclose(fptr);
}

void saveSav(gb_t* gb, const char* filename){
    FILE* fptr = fopen(filename, "wb");
    if(!fptr)
        return;
    fwrite(gb->ERAM, 1, gb->ERAM_SIZE, fptr);
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
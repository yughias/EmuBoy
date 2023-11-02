#include <hardware.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t BOOTROM_DISABLE_REG;
uint8_t SB_REG;
uint8_t SC_REG;

uint8_t NOT_MAPPED[0x10000];
uint8_t TEMP_REG;

#define MAP_REG(name) if(address == name ## _ADDR) return &name ## _REG
#define MAP_MEMORY(name) if(address >= name ## _START_ADDR && address < name ## _START_ADDR + name ## _SIZE) return name + (address - name ## _START_ADDR)

uint8_t BOOTROM[BOOTROM_SIZE];
uint8_t* ROM;
size_t ROM_SIZE;

uint8_t ERAM[ERAM_SIZE];

bool bootromEnabled;

uint8_t VRAM[VRAM_SIZE];
uint8_t WRAM[WRAM_SIZE];
uint8_t OAM[OAM_SIZE];
uint8_t HRAM[HRAM_SIZE];

char romName[FILENAME_MAX];
char savName[FILENAME_MAX];

void initMemory(){
    LY_REG = 0;
    STAT_REG = 0;
    TIMA_REG = 0;
    TMA_REG = 0;
    TAC_REG = 0xF8;
    JOYP_REG = 0xFF;
    ppu_mode = OAM_SCAN_MODE;
    writeToDMA = false;
    lyc_compare = false;
    internal_ly = 0;
    stat_irq = false;
    bootromEnabled = true;
    gb_timer.counter = 0x00;
    gb_timer.old_state = false;
    gb_timer.delay = 0x00;
    gb_timer.ignore_write = false;
    gb_timer.tma_overwritten = false;
}

void freeMemory(){
    free(ROM);
}

uint8_t* getReadAddress(uint16_t address){
    if(bootromEnabled)
        MAP_MEMORY(BOOTROM);

    if(address < 0x4000)
        return (*mbc_mapper_0000_3FFF)(address);
    else if(address < 0x8000)
        return (*mbc_mapper_4000_7FFF)(address);

    if(address >= 0xA000 && address < 0xC000)
        return (*mbc_mapper_A000_BFFF)(address);

    MAP_MEMORY(VRAM);
    MAP_MEMORY(WRAM);

    if(address >= 0xE000 && address < 0xFE00)
        return WRAM + (address - 0xE000);

    MAP_MEMORY(OAM);

    MAP_REG(LY);
    MAP_REG(LYC);
    MAP_REG(LCDC);
    MAP_REG(SCX);
    MAP_REG(SCY);
    MAP_REG(BGP);
    MAP_REG(BOOTROM_DISABLE);
    
    if(address == IE_ADDR){
        cpu.IE &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
        return &cpu.IE;
    }
    
    if(address == IF_ADDR){
        cpu.IF &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
        return &cpu.IF;
    }

    MAP_REG(TIMA);
    MAP_REG(TMA);
    
    if(address == TAC_ADDR){
        TAC_REG |= 0b11111000;
        return &TAC_REG;
    }

    MAP_REG(SC);
    MAP_REG(DMA);
    MAP_REG(OBP0);
    MAP_REG(OBP1);
    
    if(address == DIV_ADDR){
        return &gb_timer.div;
    }
    
    MAP_REG(WX);
    MAP_REG(WY);

    MAP_REG(NR52);
    MAP_REG(NR51);
    MAP_REG(NR50);
    MAP_REG(NR10);
    MAP_REG(NR11);
    MAP_REG(NR12);
    MAP_REG(NR13);
    MAP_REG(NR14);
    MAP_REG(NR21);
    MAP_REG(NR22);
    MAP_REG(NR23);
    MAP_REG(NR24);
    MAP_REG(NR30);
    MAP_REG(NR31);
    MAP_REG(NR32);
    MAP_REG(NR33);
    MAP_REG(NR34);
    MAP_REG(NR41);
    MAP_REG(NR42);
    MAP_REG(NR43);
    MAP_REG(NR44);

    MAP_MEMORY(WAVE_RAM);

    if(address == SB_ADDR){
        return &SB_REG;
    }

    if(address == STAT_ADDR){
        composeStatRegister();
        return &TEMP_REG;
    }

    if(address == JOYP_ADDR){
        composeJoypadRegister();
        return &TEMP_REG;
    }

    MAP_MEMORY(HRAM);

    return NOT_MAPPED + address;
}

uint8_t* getWriteAddress(uint16_t address){
    if(!bootromEnabled)
        if(address < 0x8000)  
            return (*mbc_rom_write)(address);

    if(address >= 0xA000 && address < 0xC000)
        return (*mbc_mapper_A000_BFFF)(address);

    MAP_MEMORY(VRAM);
    MAP_MEMORY(WRAM);

    if(address >= 0xE000 && address < 0xFE00)
        return WRAM + (address - 0xE000);

    MAP_MEMORY(OAM);

    MAP_REG(LYC);
    MAP_REG(LCDC);

    if(address == STAT_ADDR){
        if(!stat_irq && ppu_mode != OAM_SCAN_MODE)
            cpu.IF |= STAT_IRQ;
        stat_irq = true;
        return &STAT_REG;
    }

    MAP_REG(SCX);
    MAP_REG(SCY);
    MAP_REG(BGP);

    if(address == IE_ADDR)
        return &cpu.IE;

    if(address == IF_ADDR)
        return &cpu.IF;
    
    if(address == TIMA_ADDR){
        gb_timer.delay = 0;
        if(gb_timer.ignore_write)
            return NOT_MAPPED;
        return &TIMA_REG;
    }

    if(address == TMA_ADDR){
        if(gb_timer.ignore_write)
            gb_timer.tma_overwritten = true;
        return &TMA_REG;
    }

    MAP_REG(TAC);
    MAP_REG(SC);
    MAP_REG(SB);
    MAP_REG(JOYP);
    MAP_REG(OBP0);
    MAP_REG(OBP1);
    MAP_REG(WX);
    MAP_REG(WY);

    MAP_REG(NR52);
    MAP_REG(NR51);
    MAP_REG(NR50);
    MAP_REG(NR10);
    
    if(address == NR11_ADDR){
        ch1_check_timer = true;
        return &NR11_REG;
    }

    MAP_REG(NR12);
    MAP_REG(NR13);
    MAP_REG(NR14);

    if(address == NR21_ADDR){
        ch2_check_timer = true;
        return &NR21_REG;
    }

    MAP_REG(NR22);
    MAP_REG(NR23);
    MAP_REG(NR24);

    if(address == NR41_ADDR){
        ch4_check_timer = true;
        return &NR41_REG;
    }

    MAP_REG(NR30);
    
    if(address == NR31_ADDR){
        ch3_check_timer = true;
        return &NR31_REG;
    }

    MAP_REG(NR32);
    MAP_REG(NR33);
    MAP_REG(NR34);

    MAP_REG(NR42);
    MAP_REG(NR43);
    MAP_REG(NR44);

    MAP_MEMORY(WAVE_RAM);

    if(address == DIV_ADDR){
        gb_timer.counter = 0x00;
        return NOT_MAPPED;
    }

    if(address == DMA_ADDR){
        writeToDMA = true;
        return &DMA_REG;
    }

    if(address == BOOTROM_DISABLE_ADDR){
        bootromEnabled = false;
        return &BOOTROM_DISABLE_REG;
    }

    MAP_MEMORY(HRAM);

    return NOT_MAPPED + address;
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

void loadBootRom(const char* filename){
    FILE* fptr = fopen(filename, "rb");
    fread(BOOTROM, 1, BOOTROM_SIZE, fptr);
    fclose(fptr);
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
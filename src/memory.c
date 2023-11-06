#include <hardware.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t BOOTROM_DISABLE_REG;

uint8_t NOT_MAPPED;

#define READ_REG(name) if(address == name ## _ADDR) return name ## _REG
#define READ_MEMORY(name) if(address >= name ## _START_ADDR && address < name ## _START_ADDR + name ## _SIZE) return name[address - name ## _START_ADDR]

#define WRITE_REG(name) if(address == name ## _ADDR) { name ## _REG = byte; return; }
#define WRITE_MEMORY(name) if(address >= name ## _START_ADDR && address < name ## _START_ADDR + name ## _SIZE) { name[address - name ## _START_ADDR] = byte; return; }

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
    lyc_compare = false;
    internal_ly = 0;
    stat_irq = false;
    bootromEnabled = true;
    gb_timer.counter = 0x00;
    gb_timer.old_state = false;
    gb_timer.delay = 0x00;
    gb_timer.ignore_write = false;
}

void freeMemory(){
    free(ROM);
}

uint8_t readByte(uint16_t address){
    if(bootromEnabled)
        READ_MEMORY(BOOTROM);

    if(address < 0x4000)
        return *(*mbc_mapper_0000_3FFF)(address);
    else if(address < 0x8000)
        return *(*mbc_mapper_4000_7FFF)(address);

    if(address >= 0xA000 && address < 0xC000)
        return *(*mbc_mapper_A000_BFFF)(address);

    READ_MEMORY(VRAM);
    READ_MEMORY(WRAM);

    if(address >= 0xE000 && address < 0xFE00)
        return WRAM[address - 0xE000];

    READ_MEMORY(OAM);

    READ_REG(LY);
    READ_REG(LYC);
    READ_REG(LCDC);
    READ_REG(SCX);
    READ_REG(SCY);
    READ_REG(BGP);
    READ_REG(BOOTROM_DISABLE);
    
    if(address == IE_ADDR){
        cpu.IE &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
        return cpu.IE;
    }
    
    if(address == IF_ADDR){
        cpu.IF &= VBLANK_IRQ | STAT_IRQ | TIMER_IRQ | SERIAL_IRQ | JOYPAD_IRQ;
        return cpu.IF;
    }

    READ_REG(TIMA);
    READ_REG(TMA);
    
    if(address == TAC_ADDR){
        TAC_REG |= 0b11111000;
        return TAC_REG;
    }

    READ_REG(SC);
    READ_REG(DMA);
    READ_REG(OBP0);
    READ_REG(OBP1);
    
    if(address == DIV_ADDR){
        return gb_timer.div;
    }
    
    READ_REG(WX);
    READ_REG(WY);

    if(address == NR52_ADDR)
        return getNR52();

    READ_REG(NR51);
    READ_REG(NR50);
    
    if(address == NR10_ADDR)
        return getNR10();

    if(address == NR11_ADDR)
        return getNR11();

    READ_REG(NR12);
    
    if(address == NR13_ADDR)
        return 0xFF;

    if(address == NR14_ADDR)
        return getNR14();

    if(address == NR21_ADDR)
        return getNR21();

    READ_REG(NR22);
    
    if(address == NR23_ADDR)
        return 0xFF;

    if(address == NR24_ADDR)
        return getNR24();

    if(address == NR30_ADDR)
        return getNR30();

    if(address == NR31_ADDR)
        return 0xFF;

    if(address == NR32_ADDR)
        return getNR32();

    if(address == NR33_ADDR)
        return 0xFF;
    
    if(address == NR34_ADDR)
        return getNR34();

    if(address == NR41_ADDR)
        return 0xFF;

    READ_REG(NR42);
    READ_REG(NR43);
    
    if(address == NR44_ADDR)
        return getNR44();

    READ_MEMORY(WAVE_RAM);

    if(address == SB_ADDR){
        return SB_REG;
    }

    if(address == STAT_ADDR)
        return getStatRegister();

    if(address == JOYP_ADDR)
        return getJoypadRegister();

    READ_MEMORY(HRAM);

    return 0xFF;
}

void writeByte(uint16_t address, uint8_t byte){
    if(!bootromEnabled)
        if(address < 0x8000){ 
            *(*mbc_rom_write)(address) = byte;
            return;
        }

    if(address >= 0xA000 && address < 0xC000){
        *(*mbc_mapper_A000_BFFF)(address) = byte;
        return;
    }

    WRITE_MEMORY(VRAM);
    WRITE_MEMORY(WRAM);

    if(address >= 0xE000 && address < 0xFE00){
        WRAM[address - 0xE000] = byte;
        return;
    }

    WRITE_MEMORY(OAM);

    WRITE_REG(LYC);
    WRITE_REG(LCDC);

    if(address == STAT_ADDR){
        if(!stat_irq && ppu_mode != OAM_SCAN_MODE)
            cpu.IF |= STAT_IRQ;
        stat_irq = true;
        STAT_REG = byte & 0b01111000;
        return;
    }

    WRITE_REG(SCX);
    WRITE_REG(SCY);
    WRITE_REG(BGP);

    if(address == IE_ADDR){
        cpu.IE = byte;
        return;
    }

    if(address == IF_ADDR){
        cpu.IF = byte;
        return;
    }
    
    if(address == TIMA_ADDR){
        if(!gb_timer.ignore_write)
            TIMA_REG = byte;
        gb_timer.delay = 0;
        return;
    }

    if(address == TMA_ADDR){
        TMA_REG = byte;
        if(gb_timer.ignore_write)
            TIMA_REG = TMA_REG;
        return;
    }

    WRITE_REG(TAC);
    WRITE_REG(SC);
    WRITE_REG(SB);
    WRITE_REG(JOYP);
    WRITE_REG(OBP0);
    WRITE_REG(OBP1);
    WRITE_REG(WX);
    WRITE_REG(WY);

    WRITE_REG(NR52);
    WRITE_REG(NR51);
    WRITE_REG(NR50);
    WRITE_REG(NR10);
    
    if(address == NR11_ADDR){
        ch1_check_timer = true;
        NR11_REG = byte;
        return;
    }

    if(address == NR12_ADDR){
        NR12_REG = byte;
        checkDAC1();
        return;
    }

    WRITE_REG(NR13);

    if(address == NR14_ADDR){
        NR14_REG = byte;
        triggerChannel1();
        return;
    }

    if(address == NR21_ADDR){
        ch2_check_timer = true;
        NR21_REG = byte;
        return;
    }

    if(address == NR22_ADDR){
        NR22_REG = byte;
        checkDAC2();
        return;
    }

    WRITE_REG(NR22);
    WRITE_REG(NR23);

    if(address == NR24_ADDR){
        NR24_REG = byte;
        triggerChannel2();
        return;
    }

    if(address == NR30_ADDR){
        NR30_REG = byte;
        checkDAC3();
        return;
    }
    
    if(address == NR31_ADDR){
        ch3_check_timer = true;
        NR31_REG = byte;
        return;
    }

    WRITE_REG(NR32);
    WRITE_REG(NR33);

    if(address == NR34_ADDR){
        NR34_REG = byte;
        triggerChannel3();
        return;
    }

    if(address == NR41_ADDR){
        ch4_check_timer = true;
        NR41_REG = byte;
        return;
    }

    if(address == NR42_ADDR){
        NR42_REG = byte;
        checkDAC4();
        return;
    }

    WRITE_REG(NR43);

    if(address == NR44_ADDR){
        NR44_REG = byte;
        triggerChannel4();
        return;
    }

    WRITE_MEMORY(WAVE_RAM);

    if(address == DIV_ADDR){
        gb_timer.counter = 0x00;
        return;
    }

    if(address == DMA_ADDR){
        DMA_REG = byte;
        startDMA();
        return;
    }

    if(address == BOOTROM_DISABLE_ADDR){
        bootromEnabled = false;
        return;
    }

    WRITE_MEMORY(HRAM);
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
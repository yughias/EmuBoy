#include "mbc.h"
#include "memory.h"
#include "joypad.h"
#include "gb.h"

#include <SDL2/SDL.h>

uint8_t mbc5_4000_7FFF(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint16_t bank = ((mbc->REG_3000_3FFF & 0b1) << 8) | mbc->REG_2000_2FFF;
    real_addr |= (bank & 0x1FF) << 14;
    real_addr &= gb->ROM_SIZE - 1;
    return gb->ROM[real_addr];
}

uint8_t mbc5_ram_read(gb_t* gb, uint16_t addr){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    real_addr |= (mbc->REG_4000_5FFF & 0b1111) << 13;
    real_addr &= gb->ERAM_SIZE - 1;
    return gb->ERAM[real_addr];
}

void mbc5_ram_write(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    if((mbc->REG_0000_1FFF & 0x0F) != 0x0A)
        return;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    real_addr |= (mbc->REG_4000_5FFF & 0b1111) << 13;
    real_addr &= gb->ERAM_SIZE - 1;
    gb->ERAM[real_addr] = byte;
}

void mbc5_registers(gb_t* gb, uint16_t addr, uint8_t byte){
    mbc_t* mbc = &gb->mbc;
    static uint64_t rumbleTime = 0;
    static bool rumbleStarted = false;

    if(addr < 0x2000)
        mbc->REG_0000_1FFF = byte;
    else if(addr < 0x3000)
        mbc->REG_2000_2FFF = byte;
    else if(addr < 0x4000)
        mbc->REG_3000_3FFF = byte;
    else if(addr < 0x6000) {
        mbc->REG_4000_5FFF = byte;
        if(mbc->hasRumble && gameController){
            if(byte & 0b1000){
                SDL_GameControllerRumble(gameController, -1, -1, -1);
                rumbleTime = gb->startFrame_clock + gb->cpu.cycles;
                rumbleStarted = true;
            } else if(rumbleStarted){
                rumbleStarted = false;
                rumbleTime = (gb->startFrame_clock + gb->cpu.cycles) - rumbleTime ;
                rumbleTime /= (CYCLES_PER_FRAME * REFRESH_RATE) / 1000;
                SDL_GameControllerRumble(gameController, -1, -1, rumbleTime);
            }
        }
    }
}
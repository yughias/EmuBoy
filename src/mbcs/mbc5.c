#include "mbc.h"
#include "memory.h"
#include "joypad.h"
#include "gb.h"

#include <SDL2/SDL.h>

uint8_t mbc5_4000_7FFF(uint16_t addr){
    size_t real_addr = addr;
    real_addr &= (1 << 14) - 1;
    uint16_t bank = ((MBC_3000_3FFF & 0b1) << 8) | MBC_2000_2FFF;
    real_addr |= (bank & 0x1FF) << 14;
    real_addr &= ROM_SIZE - 1;
    return ROM[real_addr];
}

uint8_t mbc5_ram_read(uint16_t addr){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return 0xFF;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    real_addr |= (MBC_4000_5FFF & 0b1111) << 13;
    real_addr &= ERAM_SIZE - 1;
    return ERAM[real_addr];
}

void mbc5_ram_write(uint16_t addr, uint8_t byte){
    if((MBC_0000_1FFF & 0x0F) != 0x0A)
        return;

    size_t real_addr = addr;
    real_addr &= (1 << 13) - 1;
    real_addr |= (MBC_4000_5FFF & 0b1111) << 13;
    real_addr &= ERAM_SIZE - 1;
    ERAM[real_addr] = byte;
}

void mbc5_registers(uint16_t addr, uint8_t byte){
    static uint64_t rumbleTime = 0;
    static bool rumbleStarted = false;

    if(addr < 0x2000)
        MBC_0000_1FFF = byte;
    else if(addr < 0x3000)
        MBC_2000_2FFF = byte;
    else if(addr < 0x4000)
        MBC_3000_3FFF = byte;
    else if(addr < 0x6000) {
        MBC_4000_5FFF = byte;
        if(ROM[0x147] >= 0x1C && gameController){
            if(byte & 0b1000){
                SDL_GameControllerRumble(gameController, -1, -1, -1);
                rumbleTime = startFrame_clock + cpu.cycles;
                rumbleStarted = true;
            } else if(rumbleStarted){
                rumbleStarted = false;
                rumbleTime = (startFrame_clock + cpu.cycles) - rumbleTime ;
                rumbleTime /= (CYCLES_PER_FRAME * REFRESH_RATE) / 1000;
                SDL_GameControllerRumble(gameController, -1, -1, rumbleTime);
            }
        }
    }
}
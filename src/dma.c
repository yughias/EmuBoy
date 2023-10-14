#include <hardware.h>

uint8_t DMA_REG;
bool writeToDMA;

void startDMA(){
    uint16_t hi8 = DMA_REG << 8;
    for(uint8_t lo8 = 0x00; lo8 < 0xA0; lo8++){
        OAM[lo8] = *cpu.readMemory(hi8 | lo8);
    }
}
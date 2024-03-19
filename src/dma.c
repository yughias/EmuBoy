#include "hardware.h"

uint8_t DMA_REG;

uint8_t HDMA_REGS[5];
bool hblank_dma_started;

void initHDMA(){
    memset(HDMA_REGS, 0, sizeof(HDMA_REGS));
    hblank_dma_started = false;
}

void startDMA(){
    uint16_t hi8 = DMA_REG << 8;
    if(hi8 >> 12 == 0xF)
        hi8 = 0xD000 | (hi8 & 0xFFF);
    for(uint8_t lo8 = 0x00; lo8 < 0xA0; lo8++){
        OAM[lo8] = cpu.readByte(hi8 | lo8);
    }
}

void startHDMA(uint8_t byte){
    if(byte & 0x80){
        hblank_dma_started = true;
        HDMA_REGS[4] = byte & 0x7F;
    } else {
        if(hblank_dma_started){
            hblank_dma_started = false;
            HDMA_REGS[4] |= 0x80;
        } else {
            HDMA_REGS[4] = byte;

            uint16_t srcAddr;
            uint16_t dstAddr;
            uint16_t length;

            getInfoHDMA(&srcAddr, &dstAddr, &length);

            while(length){
                transferBlockHDMA(&srcAddr, &dstAddr);
                length--;
            }

            HDMA_REGS[0] = srcAddr >> 8;
            HDMA_REGS[1] = srcAddr;
            HDMA_REGS[2] = dstAddr >> 8;
            HDMA_REGS[3] = dstAddr;
            HDMA_REGS[4] = 0xFF;
        }
    }   
}

void stepHDMA(){
    uint16_t srcAddr;
    uint16_t dstAddr;
    uint16_t length;

    if(!hblank_dma_started)
        return;

    getInfoHDMA(&srcAddr, &dstAddr, &length);  

    transferBlockHDMA(&srcAddr, &dstAddr);
    length--;

    HDMA_REGS[0] = srcAddr >> 8;
    HDMA_REGS[1] = srcAddr;
    HDMA_REGS[2] = dstAddr >> 8;
    HDMA_REGS[3] = dstAddr;

    if(!length){
        hblank_dma_started = false;
        HDMA_REGS[4] = 0xFF;
    } else
        HDMA_REGS[4] = length - 1;
}


void getInfoHDMA(uint16_t* srcAddr, uint16_t* dstAddr, uint16_t* length){
    *srcAddr = ((HDMA_REGS[0] << 8) | (HDMA_REGS[1] & 0xF0));  
    *dstAddr = ((HDMA_REGS[2] & 0x1F) << 8) | (HDMA_REGS[3] & 0xF0) | 0x8000;
    *length =  ((HDMA_REGS[4] & 0x7F) + 1);
}

void transferBlockHDMA(uint16_t* srcAddr, uint16_t* dstAddr){
    for(int i = 0; i < 16; i++){
        uint8_t byte = cpu.readByte(*srcAddr);
        cpu.writeByte(*dstAddr, byte);
        (*srcAddr) += 1;
        (*dstAddr) += 1;
    }
}
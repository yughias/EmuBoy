#ifndef __DMA_H__
#define __DMA_H__

#include <stdint.h>

#define DMA_ADDR 0xFF46

extern uint8_t DMA_REG;
extern uint8_t HDMA_REGS[5];

void startDMA();

void startHDMA(uint8_t byte);
void initHDMA();
void stepHDMA();
void getInfoHDMA(uint16_t* srcAddr, uint16_t* dstAddr, uint16_t* length);
void transferBlockHDMA(uint16_t* srcAddr, uint16_t* dstAddr);

#endif
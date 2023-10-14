#ifndef __DMA_H__
#define __DMA_H__

#include <stdint.h>

#define DMA_ADDR 0xFF46

extern uint8_t DMA_REG;
extern bool writeToDMA;

void startDMA();

#endif
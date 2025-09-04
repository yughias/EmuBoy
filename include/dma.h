#ifndef __DMA_H__
#define __DMA_H__

#include <stdint.h>

#define DMA_ADDR 0xFF46

typedef struct dma_t {
    uint8_t DMA_REG;
    uint8_t HDMA_REGS[5];   
    bool hblank_dma_started; 
} dma_t;

typedef struct gb_t gb_t;

void initHDMA(dma_t*);
void startDMA(gb_t* gb);
void startHDMA(gb_t* gb, uint8_t byte);
void stepHDMA(gb_t*);

#endif
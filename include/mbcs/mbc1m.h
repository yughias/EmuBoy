#ifndef __MBC1M_H__
#define __MBC1M_H__

#include <stdint.h>

typedef struct gb_t gb_t;

uint8_t mbc1m_0000_3FFF(gb_t* gb, uint16_t addr);
uint8_t mbc1m_4000_7FFF(gb_t* gb, uint16_t addr);

#endif
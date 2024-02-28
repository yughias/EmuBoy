#ifndef __SKIP_BOOTROM_H__
#define __SKIP_BOOTROM_H__

#include <stdint.h>

void skipDmgBootrom();
void skipCgbBootrom();
void hleDmgColorization(uint8_t* rom);

#endif
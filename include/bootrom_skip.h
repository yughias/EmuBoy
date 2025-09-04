#ifndef __SKIP_BOOTROM_H__
#define __SKIP_BOOTROM_H__

#include <stdint.h>

typedef struct gb_t gb_t;

void skipDmgBootrom(gb_t*);
void skipCgbBootrom(gb_t*);
void hleDmgColorization(gb_t*);

#endif
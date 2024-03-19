#ifndef __MEMORY_TABLE_H__
#define __MEMORY_TABLE_H__

#include <stdint.h>

typedef uint8_t (*readFunc)(uint16_t);
typedef void (*writeFunc)(uint16_t, uint8_t);

extern readFunc readTable[0x100];
extern writeFunc writeTable[0x100];

void fillReadTable(uint8_t, uint8_t, readFunc);
void fillWriteTable(uint8_t, uint8_t, writeFunc);

uint8_t readBootrom(uint16_t);
uint8_t readVram(uint16_t);
uint8_t readWram(uint16_t);
uint8_t readEram(uint16_t);
uint8_t readMirrorRam(uint16_t);
uint8_t readOam(uint16_t);
uint8_t readIO(uint16_t);

void writeBootrom(uint16_t, uint8_t);
void writeVram(uint16_t, uint8_t);
void writeWram(uint16_t, uint8_t);
void writeEram(uint16_t, uint8_t);
void writeMirrorRam(uint16_t, uint8_t);
void writeOam(uint16_t, uint8_t);
void writeIO(uint16_t, uint8_t);

#endif
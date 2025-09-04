#ifndef __MEMORY_TABLE_H__
#define __MEMORY_TABLE_H__

#include <stdint.h>

typedef struct gb_t gb_t;
typedef uint8_t (*readGbFunc)(gb_t*, uint16_t);
typedef void (*writeGbFunc)(gb_t*, uint16_t, uint8_t);

void fillReadTable(readGbFunc*, uint8_t, uint8_t, readGbFunc);
void fillWriteTable(writeGbFunc*, uint8_t, uint8_t, writeGbFunc);

uint8_t readBootrom(gb_t*, uint16_t);
uint8_t readVram(gb_t*, uint16_t);
uint8_t readWram(gb_t*, uint16_t);
uint8_t readMirrorRam(gb_t*, uint16_t);
uint8_t readOam(gb_t*, uint16_t);
uint8_t readIO(gb_t*, uint16_t);
uint8_t readCRAM(uint8_t*, uint8_t*);

void writeVram(gb_t*, uint16_t, uint8_t);
void writeWram(gb_t*, uint16_t, uint8_t);
void writeMirrorRam(gb_t*, uint16_t, uint8_t);
void writeOam(gb_t*, uint16_t, uint8_t);
void writeIO(gb_t*, uint16_t, uint8_t);
void writeCRAM(uint8_t*, uint8_t, uint8_t*);

#endif
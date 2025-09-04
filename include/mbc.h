#ifndef __MBC_H__
#define __MBC_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct gb_t gb_t;

typedef uint8_t (*readGbFunc)(gb_t*, uint16_t);
typedef void (*writeGbFunc)(gb_t*, uint16_t, uint8_t);

typedef struct mbc_t {
    readGbFunc mapper_0000_3FFF;
    readGbFunc mapper_4000_7FFF;
    readGbFunc mapper_A000_BFFF_read;
    writeGbFunc mapper_A000_BFFF_write;

    writeGbFunc rom_write;

    uint8_t REG_0000_1FFF;
    uint8_t REG_2000_3FFF;
    uint8_t REG_4000_5FFF;
    uint8_t REG_6000_7FFF;

    // advanced mbc registers
    uint8_t REG_2000_2FFF;
    uint8_t REG_3000_3FFF;

    bool hasBattery;
    bool mbcAlreadyWritten;
    bool hasRtc;
    bool hasCamera;
    bool hasRumble;

    void* data;
} mbc_t;

void detectConsoleAndMbc(gb_t*);
bool detectM161(const uint8_t*);
bool detectMMM01(const uint8_t*, size_t);
bool detectMBC1M(const uint8_t*, size_t);
bool containNintendoLogo(const uint8_t*);

#endif
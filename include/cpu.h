#ifndef _CPU_H_
#define _CPU_H_

#include <stdbool.h>
#include <stdint.h>

#define IF_ADDR 0xFF0F
#define IE_ADDR 0xFFFF

#define VBLANK_IRQ                0b00000001
#define STAT_IRQ                  0b00000010
#define TIMER_IRQ                 0b00000100
#define SERIAL_IRQ                0b00001000
#define JOYPAD_IRQ                0b00010000

typedef uint8_t (*readFunc)(uint16_t);
typedef void (*writeFunc)(uint16_t, uint8_t);
typedef void (*tickFunc)(int);

typedef struct cpu_t {
    // interrupt vars
    bool     HALTED;
    bool     IME;
    bool     EI_DELAY;
    uint8_t IE;
    uint8_t IF;

    // registers
    union {
        uint16_t AF;
        struct {
            union {
                uint8_t F;
                struct {
                    uint8_t UNUSED_FLAG : 4;
                    bool C_FLAG : 1;
                    bool H_FLAG : 1;
                    bool N_FLAG : 1;
                    bool Z_FLAG : 1;
                };
            };
            uint8_t A;
        };
    };

    union {
        uint16_t BC;
        struct {
            uint8_t C;
            uint8_t B;
        };
    };

    union {
        uint16_t DE;
        struct {
            uint8_t E;
            uint8_t D;
        };
    };

    union {
        uint16_t HL;
        struct {
            uint8_t L;
            uint8_t H;
        };
    };

    uint16_t SP;
    uint16_t PC;

    // busFunc
    readFunc readByte;
    writeFunc writeByte;

    // tickFunc: used to update system and cpu cycles
    tickFunc tickSystem;

    uint64_t cycles;
} cpu_t;

void initCPU(cpu_t*);
void infoCPU(cpu_t*);
void stepCPU(cpu_t*);

#endif
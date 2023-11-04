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

    // 16 bit regs 
    uint16_t AF[1];
    uint16_t BC[1];
    uint16_t DE[1];
    uint16_t HL[1];
    uint16_t SP[1];
    uint16_t PC[1];

    // 8 bit regs
    uint8_t* A;
    uint8_t* F;
    uint8_t* B;
    uint8_t* C;
    uint8_t* D;
    uint8_t* E;
    uint8_t* H;
    uint8_t* L;

    // bool flags
    bool Z_FLAG;
    bool N_FLAG;
    bool H_FLAG;
    bool C_FLAG;

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
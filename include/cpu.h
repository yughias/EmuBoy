#ifndef _CPU_H_
#define _CPU_H_

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t* (*busFunc)(uint16_t);

typedef struct cpu_t {
    // interrupt vars
    bool     HALTED;
    bool     IME;
    bool     EI_DELAY;
    bool     WAIT_INTERRUPT_DISPATCH;

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

    // busFunc
    busFunc readMemory;
    busFunc writeMemory;

    uint64_t cycles;
} cpu_t;


void initCPU(cpu_t*);
void infoCPU(cpu_t*);
void stepCPU(cpu_t*);

#endif
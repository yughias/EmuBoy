#ifndef __GB_TIMER_H__
#define __GB_TIMER_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct gb_t gb_t;

typedef struct {
    uint8_t TIMA_REG;
    uint8_t TMA_REG;
    uint8_t TAC_REG;
    
    union {
        struct {
            uint8_t internal;
            uint8_t div;
        };
        uint16_t counter;
    };
    bool old_state;
    bool ignore_write;
    uint8_t delay;
} gb_timer_t;

#define TIMER_ENABLE_MASK         0b00000100
#define TIMER_CLOCK_MASK          0b00000011

#define TIMA_ADDR 0xFF05
#define TMA_ADDR 0xFF06
#define TAC_ADDR 0xFF07
#define DIV_ADDR 0xFF04

void updateTimer(gb_t*);

#endif
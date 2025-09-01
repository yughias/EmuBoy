#include "gb_timer.h"
#include "gb.h"

uint8_t TIMA_REG;
uint8_t TMA_REG;
uint8_t TAC_REG;

gb_timer_t gb_timer;

#define RESET_DELAY 4

bool isNegativeEdge(bool, bool);

void updateTimer(){
    gb_timer.counter++;
    gb_timer.ignore_write = false;

    if(gb_timer.delay && !(--gb_timer.delay)){
        cpu.IF |= TIMER_IRQ;
        TIMA_REG = TMA_REG;
        gb_timer.ignore_write = true;
    }

    uint16_t timer_masks[4] = {1 << 9, 1 << 3, 1 << 5, 1 << 7};
    uint16_t timer_mask = timer_masks[TAC_REG & TIMER_CLOCK_MASK];
    bool new_state = (gb_timer.counter & timer_mask) && (TAC_REG & TIMER_ENABLE_MASK);
    if(isNegativeEdge(gb_timer.old_state, new_state)){
        if(TIMA_REG == 0xFF){
            TIMA_REG = 0x00;
            gb_timer.delay = RESET_DELAY;
        } else {
            TIMA_REG++;
        }
    }

    gb_timer.old_state = new_state;
}

bool isNegativeEdge(bool old_state, bool new_state){
    return old_state && !new_state;
}
#include <SDL2/SDL.h>
#include "joypad.h"

uint8_t JOYP_REG;

uint8_t ARROW_BTN;
uint8_t ACTION_BTN;

const Uint8* keystate;

void initJoypad(){
    keystate = SDL_GetKeyboardState(NULL);
}

void emulateJoypad(){
    uint8_t new_arrow_btn = 0x0F;
    uint8_t new_action_btn = 0x0F;

    if(keystate[A_KEY])
        new_action_btn &= 0b1110;

    if(keystate[B_KEY])
        new_action_btn &= 0b1101;
    
    if(keystate[SELECT_KEY])
        new_action_btn &= 0b1011;

    if(keystate[START_KEY])
        new_action_btn &= 0b0111;

    if(keystate[RIGHT_KEY])
        new_arrow_btn &= 0b1110;

    if(keystate[LEFT_KEY])
        new_arrow_btn &= 0b1101;
    
    if(keystate[UP_KEY])
        new_arrow_btn &= 0b1011;

    if(keystate[DOWN_KEY])
        new_arrow_btn &= 0b0111;

    ARROW_BTN = new_arrow_btn;
    ACTION_BTN = new_action_btn;
}

uint8_t getJoypadRegister(){
    uint8_t output_val;

    emulateJoypad();

    uint8_t chosen = JOYP_REG >> 4;

    output_val = JOYP_REG & 0b110000;
    output_val |= 0b11000000;

    if(chosen == 0b10){
        output_val |= ARROW_BTN;
    }
    
    if(chosen == 0b01){
        output_val |= ACTION_BTN;
    }

    if(chosen == 0b11){
        output_val = 0xFF;
    }

    return output_val;
}
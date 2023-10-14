#include <hardware.h>

uint8_t JOYP_REG;

uint8_t ARROW_BTN;
uint8_t ACTION_BTN;

void emulateJoypad(const Uint8* keystate){
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

void composeJoypadRegister(){
    uint8_t chosen = JOYP_REG >> 4;

    TEMP_REG = JOYP_REG & 0b110000;
    TEMP_REG |= 0b11000000;

    if(chosen == 0b10){
        TEMP_REG |= ARROW_BTN;
    }
    
    if(chosen == 0b01){
        TEMP_REG |= ACTION_BTN;
    }

    if(chosen == 0b11){
        TEMP_REG = 0xFF;
    }
}
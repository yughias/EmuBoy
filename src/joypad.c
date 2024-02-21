#include <SDL2/SDL.h>
#include "joypad.h"

#define JOYSTICK_CHECK(btn) (joystick && SDL_JoystickGetButton(joystick, SDL_CONTROLLER_BUTTON_ ## btn)) 

uint8_t JOYP_REG;

uint8_t ARROW_BTN;
uint8_t ACTION_BTN;

const Uint8* keystate;
SDL_Joystick* joystick;

void initJoypad(){
    keystate = SDL_GetKeyboardState(NULL);
    joystick = SDL_JoystickOpen(0);
}

void emulateJoypad(){
    uint8_t new_arrow_btn = 0x0F;
    uint8_t new_action_btn = 0x0F;

    if(keystate[A_KEY] || JOYSTICK_CHECK(B))
        new_action_btn &= 0b1110;

    if(keystate[B_KEY] || JOYSTICK_CHECK(A))
        new_action_btn &= 0b1101;
    
    if(keystate[SELECT_KEY] || JOYSTICK_CHECK(X))
        new_action_btn &= 0b1011;

    if(keystate[START_KEY] || JOYSTICK_CHECK(Y))
        new_action_btn &= 0b0111;

    if(keystate[RIGHT_KEY] || JOYSTICK_CHECK(DPAD_RIGHT))
        new_arrow_btn &= 0b1110;

    if(keystate[LEFT_KEY] || JOYSTICK_CHECK(DPAD_LEFT))
        new_arrow_btn &= 0b1101;
    
    if(keystate[UP_KEY] || JOYSTICK_CHECK(DPAD_UP))
        new_arrow_btn &= 0b1011;

    if(keystate[DOWN_KEY] || JOYSTICK_CHECK(DPAD_DOWN))
        new_arrow_btn &= 0b0111;

    ARROW_BTN = new_arrow_btn;
    ACTION_BTN = new_action_btn;
}

uint8_t getJoypadRegister(){
    uint8_t output_val;

    emulateJoypad();

    uint8_t chosen = (JOYP_REG >> 4) & 0b11;

    output_val = JOYP_REG & 0b110000;
    output_val |= 0b11000000;

    if(chosen == 0b10)
        output_val |= ARROW_BTN;
    
    if(chosen == 0b01)
        output_val |= ACTION_BTN;

    if(chosen == 0b11)
        output_val = 0xFF;

    return output_val;
}
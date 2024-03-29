#include <SDL2/SDL.h>
#include "joypad.h"

uint8_t JOYP_REG;

uint8_t ARROW_BTN;
uint8_t ACTION_BTN;

const Uint8* keystate;
SDL_GameController* gameController;

bool a_turbo_btn = false;
bool b_turbo_btn = false;

void initJoypad(){
    keystate = SDL_GetKeyboardState(NULL);
    gameController = SDL_GameControllerOpen(0);
}

void emulateJoypad(){
    uint8_t new_arrow_btn = 0x0F;
    uint8_t new_action_btn = 0x0F;
    Sint16 x_axis = 0;
    Sint16 y_axis = 0;

    if(gameController){
        x_axis = SDL_GameControllerGetAxis(gameController, 0);
        y_axis = SDL_GameControllerGetAxis(gameController, 1);
        NORM_AXIS(x_axis);
        NORM_AXIS(y_axis);
    }

    if(keystate[A_KEY] || GAMECONTROLLER_CHECK(B))
        new_action_btn &= 0b1110;

    if(keystate[B_KEY] || GAMECONTROLLER_CHECK(A))
        new_action_btn &= 0b1101;
    
    if(keystate[SELECT_KEY] || GAMECONTROLLER_CHECK(BACK))
        new_action_btn &= 0b1011;

    // map select key to both left and right shift key for emscripten
    #ifdef __EMSCRIPTEN__
    if(keystate[SDL_SCANCODE_LSHIFT] || keystate[SDL_SCANCODE_RSHIFT])
        new_action_btn &= 0b1011;
    #endif

    if(keystate[START_KEY] || GAMECONTROLLER_CHECK(START))
        new_action_btn &= 0b0111;

    if(keystate[RIGHT_KEY] || GAMECONTROLLER_CHECK(DPAD_RIGHT) || x_axis == 1)
        new_arrow_btn &= 0b1110;

    if(keystate[LEFT_KEY] || GAMECONTROLLER_CHECK(DPAD_LEFT) || x_axis == -1)
        new_arrow_btn &= 0b1101;
    
    if(keystate[UP_KEY] || GAMECONTROLLER_CHECK(DPAD_UP) || y_axis == -1)
        new_arrow_btn &= 0b1011;

    if(keystate[DOWN_KEY] || GAMECONTROLLER_CHECK(DPAD_DOWN) || y_axis == 1)
        new_arrow_btn &= 0b0111;

    
    if(keystate[TURBO_A_KEY] || GAMECONTROLLER_CHECK(Y)){
        new_action_btn &= 0b1110;
        new_action_btn |= a_turbo_btn;
    }

    if(keystate[TURBO_B_KEY] || GAMECONTROLLER_CHECK(X)){
        new_action_btn &= 0b1110;
        new_action_btn |= b_turbo_btn;
    }

    ARROW_BTN = new_arrow_btn;
    ACTION_BTN = new_action_btn;
}

void emulateTurboButton(){
    a_turbo_btn ^= 1;
    b_turbo_btn ^= 1;
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
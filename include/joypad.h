#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include "SDL_MAINLOOP.h"

#define LEFT_KEY SDL_SCANCODE_LEFT
#define RIGHT_KEY SDL_SCANCODE_RIGHT
#define UP_KEY SDL_SCANCODE_UP
#define DOWN_KEY SDL_SCANCODE_DOWN
#define A_KEY SDL_SCANCODE_X
#define B_KEY SDL_SCANCODE_Z
#define SELECT_KEY SDL_SCANCODE_RSHIFT
#define START_KEY SDL_SCANCODE_RETURN

#define JOYP_ADDR 0xFF00

extern uint8_t JOYP_REG;

void initJoypad();
void emulateJoypad();
uint8_t getJoypadRegister();

#endif
#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include "SDL_MAINLOOP.h"

#define AXIS_DEAD_ZONE (1 << 13)
#define GAMECONTROLLER_CHECK(btn) (gameController && SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_ ## btn)) 
#define NORM_AXIS(axis) if(axis > AXIS_DEAD_ZONE) axis = 1; else if(axis < -AXIS_DEAD_ZONE) axis = -1; else axis = 0

#define LEFT_KEY SDL_SCANCODE_LEFT
#define RIGHT_KEY SDL_SCANCODE_RIGHT
#define UP_KEY SDL_SCANCODE_UP
#define DOWN_KEY SDL_SCANCODE_DOWN
#define A_KEY SDL_SCANCODE_X
#define B_KEY SDL_SCANCODE_Z
#define SELECT_KEY SDL_SCANCODE_RSHIFT
#define START_KEY SDL_SCANCODE_RETURN
#define TURBO_A_KEY SDL_SCANCODE_S 
#define TURBO_B_KEY SDL_SCANCODE_A

#define JOYP_ADDR 0xFF00

extern const Uint8* keystate;
extern SDL_GameController* gameController;

extern uint8_t JOYP_REG;

void initJoypad();
void emulateJoypad();
void emulateTurboButton();
uint8_t getJoypadRegister();
void setGameControllerLed();

#endif
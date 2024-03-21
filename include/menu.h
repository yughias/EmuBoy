#ifndef __MENU_H__
#define __MENU_H__

#include "SDL_MAINLOOP.h"

void createMenuBar();

extern buttonId menu_nearestBtn;
extern buttonId menu_linearBtn;
extern buttonId menu_matrixBtn;
extern buttonId menu_classicBtn;
extern buttonId menu_scale2xBtn;
extern buttonId menu_scale3xBtn;
extern buttonId menu_debugBtn;

extern buttonId menu_defaultPaletteBtn;
extern buttonId menu_greyPaletteBtn;
extern buttonId menu_superGameboyPaletteBtn;
extern buttonId menu_gameboyLightPaletteBtn;
extern buttonId menu_gameboyKioskPaletteBtn;

#endif
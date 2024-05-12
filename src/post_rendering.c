#include "post_rendering.h"
#include "SDL_MAINLOOP.h"
#include "hardware.h"
#include "menu.h"
#include "ini.h"
#include "upscaler.h"

#include <math.h>
#include <stdlib.h>

void noFilterDisplay();
void matrixFilterDisplay();
void dmgFilterDisplay();
void scale2xFilterDisplay();
void scale3xFilterDisplay();
void hq2xFilterDisplay();
void hq3xFilterDisplay();
void debugFilterDisplay();
void hq2xFilterDisplay();
void matrixAndGhostingDisplay(float);

void setupDisplayFilter(int width, int height, ScaleMode scale_mode, void (*filterFunc)(), buttonId radioBtn);

#define DETECT_UPSCALER(filterName, interp, rate) \
if(!strcmp(#filterName, config_render)){ \
        setupDisplayFilter(LCD_WIDTH*rate, LCD_HEIGHT*rate, interp, filterName ## FilterDisplay, menu_ ## filterName ## Btn); \
        return; \
} \

void (*renderDisplay)();

void setupWindow(){
    #ifdef __EMSCRIPTEN__
    setScaleMode(NEAREST);
    size(LCD_WIDTH, LCD_HEIGHT);
    renderDisplay = noFilterDisplay;
    return;
    #endif

    if(!strcmp("linear", config_render)){
        setupDisplayFilter(LCD_WIDTH, LCD_HEIGHT, LINEAR, noFilterDisplay, menu_linearBtn);
        return;
    }

    if(!strcmp("dmg", config_render)){
        setupDisplayFilter(LCD_WIDTH*5, LCD_HEIGHT*5, NEAREST, dmgFilterDisplay, menu_classicBtn);
        return;
    }

	if(!strcmp("debug", config_render)){
        setupDisplayFilter(800, 600, NEAREST, debugFilterDisplay, menu_debugBtn);
        return;
    }

	DETECT_UPSCALER(matrix, NEAREST, 5);
	DETECT_UPSCALER(scale2x, NEAREST, 2);
	DETECT_UPSCALER(scale3x, NEAREST, 3);
	DETECT_UPSCALER(hq2x, NEAREST, 2);
	DETECT_UPSCALER(hq3x, NEAREST, 3);

    setupDisplayFilter(LCD_WIDTH, LCD_HEIGHT, NEAREST, noFilterDisplay, menu_nearestBtn);
}


void setupDisplayFilter(int width, int height, ScaleMode scale_mode, void (*filterFunc)(), buttonId radioBtn){
    size(width, height);
    setScaleMode(scale_mode);
    renderDisplay = filterFunc;
    checkRadioButton(radioBtn);
}

void noFilterDisplay(){
    for(int y = 0; y < LCD_HEIGHT; y++)
        for(int x = 0; x < LCD_WIDTH; x++)
            pixels[x + y * width] = renderBufferPtr[x + y * LCD_WIDTH];
}

void matrixAndGhostingDisplay(float alpha){
    static float ghosting[LCD_WIDTH*LCD_HEIGHT][3] = {0};

    background(backgroundColor);

    for(int y = 0; y < LCD_HEIGHT; y++)
        for(int x = 0; x < LCD_WIDTH; x++){
            int idx = x + y * LCD_WIDTH;
            Uint8 nowRGB[3];
            getRGB(renderBufferPtr[idx], &nowRGB[0], &nowRGB[1], &nowRGB[2]);
            for(int i = 0; i < 3; i++)
                ghosting[idx][i] = alpha*nowRGB[i] + (1-alpha) * ghosting[idx][i]; 
            rect(x*5+1, y*5+1, 4, 4, color(ghosting[idx][0], ghosting[idx][1], ghosting[idx][2]));
        }
}

void matrixFilterDisplay(){
    matrixAndGhostingDisplay(1.0);
}

void dmgFilterDisplay(){
    matrixAndGhostingDisplay(0.3);
}

void scale2xFilterDisplay(){
    scale2x(renderBufferPtr, pixels, LCD_WIDTH, LCD_HEIGHT);
}

void scale3xFilterDisplay(){
	scale3x(renderBufferPtr, pixels, LCD_WIDTH, LCD_HEIGHT);
}


void hq2xFilterDisplay(){
	hq2x(renderBufferPtr, pixels, LCD_WIDTH, LCD_HEIGHT);
}


void hq3xFilterDisplay(){
	hq3x(renderBufferPtr, pixels, LCD_WIDTH, LCD_HEIGHT);
}

void debugFilterDisplay(){
    background(color(0, 0, 0));
    noFilterDisplay();
    
    drawBgRamAt(LCD_WIDTH + 16, 0);
    drawWinRamAt(LCD_WIDTH + 16, 256 + 16);

    bool bigSprite = LCDC_REG & OBJ_SIZE_MASK;
    int offY = bigSprite ? 16 : 8;
    for(int i = 0; i < 40; i++){
        drawOAMAt((i%8)*8, LCD_HEIGHT + 16 + (i/8)*offY, i);
    }

    int n_palette = console_type == CGB_TYPE ? 8 : 1;
    for(int palette = 0; palette < n_palette; palette++){
        for(int pal_color = 0; pal_color < 4; pal_color++){
            drawColorAt(LCD_WIDTH + 256 + 32 + pal_color*8, palette*8, palette, pal_color, BGP_CRAM);
            drawColorAt(LCD_WIDTH + 256 + 80 + pal_color*8, palette*8, palette, pal_color, OBP_CRAM);
        }
    }
}
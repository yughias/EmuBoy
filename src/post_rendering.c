#include "post_rendering.h"
#include "SDL_MAINLOOP.h"
#include "hardware.h"
#include "ini.h"

#include <stdlib.h>

void noFilterDisplay();
void matrixFilterDisplay();
void dmgFilterDisplay();
void scale2xFilterDisplay();
void scale3xFilterDisplay();
void debugFilterDisplay();

#define GET_3x3_PIXELS(x, y) \
int A = x - 1 >= 0 && y - 1 >= 0 ? renderBufferPtr[(x-1) + (y-1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int B = y - 1 >= 0 ? renderBufferPtr[x + (y-1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int C = x + 1 < LCD_WIDTH && y - 1 >= 0 ? renderBufferPtr[(x+1) + (y-1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int D = x - 1 >= 0 ? renderBufferPtr[(x-1) + y*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int E = renderBufferPtr[x + y*LCD_WIDTH]; \
int F = x + 1 < LCD_WIDTH ? renderBufferPtr[(x+1) + y*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int G = x - 1 >= 0 && y + 1 < LCD_HEIGHT ? renderBufferPtr[(x-1) + (y+1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int H = y + 1 < LCD_HEIGHT ? renderBufferPtr[x + (y+1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]; \
int I = x + 1 < LCD_WIDTH && y + 1 < LCD_HEIGHT ? renderBufferPtr[(x+1) + (y+1)*LCD_WIDTH] : renderBufferPtr[x + y * LCD_WIDTH]

void matrixAndGhostingDisplay(float);

void (*renderDisplay)();

void setupWindow(){
    #ifdef __EMSCRIPTEN__
    setScaleMode(NEAREST);
    size(LCD_WIDTH, LCD_HEIGHT);
    renderDisplay = noFilterDisplay;
    return;
    #endif

    if(!strcmp("linear", config_render)){
        setScaleMode(LINEAR);
        size(LCD_WIDTH, LCD_HEIGHT);
        renderDisplay = noFilterDisplay;
        return;
    }

    if(!strcmp("matrix", config_render)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*5, LCD_HEIGHT*5);
        renderDisplay = matrixFilterDisplay;
        return;
    }

    if(!strcmp("dmg", config_render)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*5, LCD_HEIGHT*5);
        renderDisplay = dmgFilterDisplay;
        return;
    }

    if(!strcmp("scale2x", config_render)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*2, LCD_HEIGHT*2);
        renderDisplay = scale2xFilterDisplay;
        return;
    }

    if(!strcmp("scale3x", config_render)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*3, LCD_HEIGHT*3);
        renderDisplay = scale3xFilterDisplay;
        return;
    }

    if(!strcmp("debug", config_render)){
        setScaleMode(NEAREST);
        size(800, 600);
        renderDisplay = debugFilterDisplay;
        return;
    }

    setScaleMode(NEAREST);
    size(LCD_WIDTH, LCD_HEIGHT);
    renderDisplay = noFilterDisplay;
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
    for(int y = 0; y < LCD_HEIGHT; y++){
        for(int x = 0; x < LCD_WIDTH; x++){
            GET_3x3_PIXELS(x, y);
            int E0, E1, E2, E3;
            if (B != H && D != F) {
                E0 = D == B ? D : E;
                E1 = B == F ? F : E;
                E2 = D == H ? D : E;
                E3 = H == F ? F : E;
            } else {
                E0 = E;
                E1 = E;
                E2 = E;
                E3 = E;
            }
            pixels[x*2 + y*2 * width] = E0;
            pixels[(x*2+1) + y*2 * width] = E1;
            pixels[x*2 + (y*2+1) * width] = E2;
            pixels[(x*2+1) + (y*2+1) * width] = E3;
        }
    }
}

void scale3xFilterDisplay(){
    for(int y = 0; y < LCD_HEIGHT; y++){
        for(int x = 0; x < LCD_WIDTH; x++){
            GET_3x3_PIXELS(x, y);
            int E_PIX[9];
            if (B != H && D != F) {
                E_PIX[0] = D == B ? D : E;
                E_PIX[1] = (D == B && E != C) || (B == F && E != A) ? B : E;
                E_PIX[2] = B == F ? F : E;
                E_PIX[3] = (D == B && E != G) || (D == H && E != A) ? D : E;
                E_PIX[4] = E;
                E_PIX[5] = (B == F && E != I) || (H == F && E != C) ? F : E;
                E_PIX[6] = D == H ? D : E;
                E_PIX[7] = (D == H && E != I) || (H == F && E != G) ? H : E;
                E_PIX[8] = H == F ? F : E;
            } else {
                E_PIX[0] = E;
                E_PIX[1] = E;
                E_PIX[2] = E;
                E_PIX[3] = E;
                E_PIX[4] = E;
                E_PIX[5] = E;
                E_PIX[6] = E;
                E_PIX[7] = E;
                E_PIX[8] = E;
            }
            for(int dy = 0; dy < 3; dy++)
                for(int dx = 0; dx < 3; dx++)
                    pixels[(x*3+dx) + (y*3+dy) * width] = E_PIX[dx + dy*3];
        }
    }
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
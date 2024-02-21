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

#define GET_3x3_PIXELS(x, y) \
int A = x - 1 >= 0 && y - 1 >= 0 ? renderBufferPtr[(x-1) + (y-1)*LCD_WIDTH] : colorRGB[0]; \
int B = y - 1 >= 0 ? renderBufferPtr[x + (y-1)*LCD_WIDTH] : colorRGB[0]; \
int C = x + 1 < LCD_WIDTH && y - 1 >= 0 ? renderBufferPtr[(x+1) + (y-1)*LCD_WIDTH] : colorRGB[0]; \
int D = x - 1 >= 0 ? renderBufferPtr[(x-1) + y*LCD_WIDTH] : colorRGB[0]; \
int E = renderBufferPtr[x + y*LCD_WIDTH]; \
int F = x + 1 < LCD_WIDTH ? renderBufferPtr[(x+1) + y*LCD_WIDTH] : colorRGB[0]; \
int G = x - 1 >= 0 && y + 1 < LCD_HEIGHT ? renderBufferPtr[(x-1) + (y+1)*LCD_WIDTH] : colorRGB[0]; \
int H = y + 1 < LCD_HEIGHT ? renderBufferPtr[x + (y+1)*LCD_WIDTH] : colorRGB[0]; \
int I = x + 1 < LCD_WIDTH && y + 1 < LCD_HEIGHT ? renderBufferPtr[(x+1) + (y+1)*LCD_WIDTH] : colorRGB[0]

void matrixAndGhostingDisplay(float);

void (*renderDisplay)() = dmgFilterDisplay;

void setupWindow(){
    #ifdef __EMSCRIPTEN__
    setScaleMode(NEAREST);
    size(LCD_WIDTH, LCD_HEIGHT);
    renderDisplay = noFilterDisplay;
    return;
    #endif

    char render_mode[128] = "";
    char ini_path[FILENAME_MAX];
    getAbsoluteDir(ini_path);
    strcat(ini_path, "data/config.ini");

    FILE* ini_ptr = INI_open(ini_path);

    if(ini_ptr){
        INI_getString(ini_ptr, "render", render_mode);
        INI_close(ini_ptr);
    }

    if(!strcmp("linear", render_mode)){
        setScaleMode(LINEAR);
        size(LCD_WIDTH, LCD_HEIGHT);
        return;
    }

    if(!strcmp("matrix", render_mode)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*5, LCD_HEIGHT*5);
        renderDisplay = matrixFilterDisplay;
        return;
    }

    if(!strcmp("dmg", render_mode)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*5, LCD_HEIGHT*5);
        renderDisplay = dmgFilterDisplay;
        return;
    }

    if(!strcmp("scale2x", render_mode)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*2, LCD_HEIGHT*2);
        renderDisplay = scale2xFilterDisplay;
        return;
    }

    if(!strcmp("scale3x", render_mode)){
        setScaleMode(NEAREST);
        size(LCD_WIDTH*3, LCD_HEIGHT*3);
        renderDisplay = scale3xFilterDisplay;
        return;
    }

    setScaleMode(NEAREST);
    size(LCD_WIDTH, LCD_HEIGHT);
    renderDisplay = noFilterDisplay;
}

void noFilterDisplay(){
    memcpy(pixels, renderBufferPtr, sizeof(int)*width*height);
}

void matrixAndGhostingDisplay(float alpha){
    static float ghosting[LCD_WIDTH*LCD_HEIGHT][3] = {0};

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
#include "SDL_MAINLOOP.h"
#include "gameshark.h"
#include "hardware.h"

#include <stdlib.h>

int workingBuffer[LCD_WIDTH*LCD_HEIGHT];
int renderBuffer[LCD_WIDTH*LCD_HEIGHT];
int* workingBufferPtr = &workingBuffer[0];
int* renderBufferPtr = &renderBuffer[0];

int colorRGB[4];
PPU_MODE ppu_mode;
bool lyc_compare;
bool stat_irq;
size_t ppu_counter;
uint8_t windowY_counter;
uint8_t internal_ly;
bool frameSkip;
bool lastFrameOn;

uint8_t LY_REG;
uint8_t LYC_REG;
uint8_t LCDC_REG;
uint8_t STAT_REG;
uint8_t SCX_REG;
uint8_t SCY_REG;
uint8_t BGP_REG;
uint8_t OBP0_REG;
uint8_t OBP1_REG;
uint8_t WX_REG;
uint8_t WY_REG;

uint8_t LCD_ENABLE_MASK;            
uint8_t WIN_TILE_MAP_AREA_MASK;   
uint8_t WIN_ENABLE_MASK;    
uint8_t BG_WIN_TILE_DATA_AREA_MASK;
uint8_t BG_TILE_MAP_AREA_MASK;
uint8_t OBJ_SIZE_MASK;     
uint8_t OBJ_ENABLE_MASK;            
uint8_t BG_WIN_ENABLE_MASK; 

#define SET_LCDC_MASKS(name) \
LCD_ENABLE_MASK = name ## _LCD_ENABLE_MASK; \
WIN_TILE_MAP_AREA_MASK = name ## _WIN_TILE_MAP_AREA_MASK; \
WIN_ENABLE_MASK = name ## _WIN_ENABLE_MASK; \
BG_WIN_TILE_DATA_AREA_MASK = name ## _BG_WIN_TILE_DATA_AREA_MASK; \
BG_TILE_MAP_AREA_MASK = name ## _BG_TILE_MAP_AREA_MASK; \
OBJ_SIZE_MASK = name ## _OBJ_SIZE_MASK; \
OBJ_ENABLE_MASK = name ## _OBJ_ENABLE_MASK; \
BG_WIN_ENABLE_MASK = name ## _BG_WIN_ENABLE_MASK

typedef struct {
    uint8_t idx;
    int x;
    int y;
} sprite_info;

void swapBuffers();

int sprite_order_compare(const void* a, const void* b){
    sprite_info* sprite_a = (sprite_info*)a;
    sprite_info* sprite_b = (sprite_info*)b;
    
    if(sprite_a->x != sprite_b->x)
        return sprite_b->x - sprite_a->x;
    else
        return sprite_b->idx - sprite_a->idx;
}

void initLcdcMasks(){
    switch(console_type){
        case DMG_TYPE:
        SET_LCDC_MASKS(DMG);
        break;

        case MEGADUCK_TYPE:
        SET_LCDC_MASKS(MEGADUCK);
        break;
    }
}

void initPaletteRGB(){
    colorRGB[3] = color(46, 65, 57);
    colorRGB[2] = color(64, 89, 74);
    colorRGB[1] = color(95, 120, 66);
    colorRGB[0] = color(123, 129, 17);
}

void drawBgRamAt(int screenX, int screenY){
    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
            int col = getBackgroundPixelRGB(x, y);
            pixels[screenX+x + (screenY+y)*width] = col;
        }
    }

    for(int y = 0; y < LCD_HEIGHT; y++){
        int offX1 = screenX + (SCX_REG % 256);
        int offX2 = screenX + ((SCX_REG + LCD_WIDTH) % 256);
        int offY = screenY + ((SCY_REG + y) % 256);
        pixels[offX1 + offY * width] = color(255, 0, 0);
        pixels[offX2 + offY * width] = color(255, 0, 0);
    }

    for(int x = 0; x < LCD_WIDTH; x++){
        int offX = screenX + ((SCX_REG + x) % 256);
        int offY1 = screenY + (SCY_REG % 256);
        int offY2 = screenY + ((SCY_REG + LCD_HEIGHT) % 256);
        pixels[offX + offY1 * width] = color(255, 0, 0);
        pixels[offX + offY2 * width] = color(255, 0, 0);
    }
}

void drawWinRamAt(int screenX, int screenY){
    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
            int col = getWindowPixelRGB(x, y);
            pixels[screenX+x + (screenY+y)*width] = col;
        }
    }
}

uint8_t* getTileMap(bool addressMode){
    return addressMode ? VRAM + 0x1C00 : VRAM + 0x1800;
}

uint8_t* getTileData(uint8_t tileIdx){
    if(LCDC_REG & BG_WIN_TILE_DATA_AREA_MASK){
        return VRAM + (tileIdx*16);
    } else {
        return VRAM + 0x1000 + ((int8_t)tileIdx)*16;
    }
}

int getColorFromTileDataRGB(uint8_t* tilePtr, uint8_t tilePX, uint8_t tilePY, uint8_t colorREG){
    uint8_t colorGB = 0;
    colorGB = !!(tilePtr[tilePY*2] & (1 << (7 - tilePX)));
    colorGB |= !!(tilePtr[1+tilePY*2] & (1 << (7 - tilePX))) << 1;

    return convertGB2RGB(colorGB, colorREG);
}

int convertGB2RGB(uint8_t colorGB, uint8_t colorREG){
    uint8_t idx = (colorREG >> (2 * colorGB)) & 0b11 ;
    return colorRGB[idx];
}

int getBackgroundPixelRGB(uint8_t x, uint8_t y){
    uint8_t* tileMapPtr = getTileMap(LCDC_REG & BG_TILE_MAP_AREA_MASK);
    uint8_t byteX = x >> 3;
    uint8_t byteY = y >> 3;
    uint8_t tilePX = x & 0b111;
    uint8_t tilePY = y & 0b111;
    uint8_t tileIdx = tileMapPtr[byteX+byteY*32];

    uint8_t* tilePtr = getTileData(tileIdx);

    return getColorFromTileDataRGB(tilePtr, tilePX, tilePY, BGP_REG);
}

int getWindowPixelRGB(uint8_t x, uint8_t y){
    uint8_t* tileMapPtr = getTileMap(LCDC_REG & WIN_TILE_MAP_AREA_MASK);
    uint8_t byteX = x >> 3;
    uint8_t byteY = y >> 3;
    uint8_t tilePX = x & 0b111;
    uint8_t tilePY = y & 0b111;
    uint8_t tileIdx = tileMapPtr[byteX+byteY*32];

    uint8_t* tilePtr = getTileData(tileIdx);

    return getColorFromTileDataRGB(tilePtr, tilePX, tilePY, BGP_REG);
}

void renderLine(uint8_t y){
    int col;

    for(uint8_t x = 0; x < LCD_WIDTH; x++){
        if(!(LCDC_REG & BG_WIN_ENABLE_MASK))
            col = colorRGB[0];
        else
            col = getBackgroundPixelRGB((SCX_REG + x % 256), (SCY_REG + y) % 256);
        
        workingBufferPtr[x + y * LCD_WIDTH] = col;
    }

    if(console_type == MEGADUCK_TYPE)
        if((LCDC_REG & (BG_WIN_TILE_DATA_AREA_MASK | WIN_TILE_MAP_AREA_MASK | BG_TILE_MAP_AREA_MASK)) == 0b01100)
            return;

    if((LCDC_REG & WIN_ENABLE_MASK) && (LCDC_REG & BG_WIN_ENABLE_MASK)){
        int winX = WX_REG - 7;
        if(y >= WY_REG){
            if(winX < LCD_WIDTH){
                for(int offX = 0; winX < LCD_WIDTH; offX = (offX + 1) % 256){
                    if(winX >= 0)
                        workingBufferPtr[winX + y * LCD_WIDTH] = getWindowPixelRGB(offX % 256, windowY_counter % 256);
                    winX++;
                }
                windowY_counter++;
            }
        }
    }

    if(!(LCDC_REG & OBJ_ENABLE_MASK))
        return;

    sprite_info sprites[10];
    uint8_t drawnSprites = 0;
    bool bigSprite = LCDC_REG & OBJ_SIZE_MASK;
    for(int i = 0; i < 40 && drawnSprites < 10; i++){
        // these value can be negative!
        int spriteTopY = getSpriteRealY(i);
        if(y >= spriteTopY && ((y < spriteTopY + 8) || (y < spriteTopY + 16 && bigSprite))){
            int spriteTopX = getSpriteRealX(i);
            sprites[drawnSprites].idx = i;
            sprites[drawnSprites].x = spriteTopX;
            sprites[drawnSprites].y = spriteTopY;
            drawnSprites++;
        }
    }

    qsort(sprites, drawnSprites, sizeof(sprite_info), sprite_order_compare);

    for(int i = 0; i < drawnSprites; i++){
        int spriteTopX = sprites[i].x;
        int spriteTopY = sprites[i].y;
        if(spriteTopX < 168){
            int screenX = spriteTopX;
            int spriteX = 0;
            int spriteY = y - spriteTopY;
            if(screenX < 0){
                screenX = 0;
                spriteX = -spriteTopX;
            }
            uint8_t* spriteData = &OAM[sprites[i].idx*4];
            while(screenX < LCD_WIDTH && spriteX < 8){
                bool flipX = spriteData[3] & 0b100000;
                bool flipY = spriteData[3] & 0b1000000;
                bool backgroundOver = spriteData[3] & 0b10000000;
                uint8_t palette = spriteData[3] & 0b10000 ? OBP1_REG : OBP0_REG; 
                uint8_t tileIdx = spriteData[2];
                if(bigSprite)
                    tileIdx &= 0b11111110;
                uint8_t* tilePtr = VRAM + tileIdx*16;

                col = getSpritePixelRGB(tilePtr, spriteX, spriteY, palette, flipX, flipY, bigSprite);
                if(col != -1)
                    if(!backgroundOver || (backgroundOver && workingBufferPtr[screenX + y * LCD_WIDTH] == convertGB2RGB(0, BGP_REG)))
                        workingBufferPtr[screenX + y * LCD_WIDTH] = col;

                screenX++;
                spriteX++;
            }
        }
    }
}

int getSpritePixelRGB(uint8_t* tilePtr, uint8_t x, uint8_t y, uint8_t palette, bool flipX, bool flipY, bool bigSprite){
    if(flipX)
        x = 7 - x;
    
    if(flipY){
        if(bigSprite)
            y = 15 - y;
        else
            y = 7 - y;
    }

    uint8_t colorGB = 0;
    colorGB = !!(tilePtr[y*2] & (1 << (7 - x)));
    colorGB |= !!(tilePtr[1+y*2] & (1 << (7 - x))) << 1;

    // white color = transparent in Sprite
    if(colorGB == 0)
        return -1;

    return convertGB2RGB(colorGB, palette);
}

void drawOAMAt(int screenX, int screenY, uint8_t spriteIdx){
    uint8_t* spriteData = &OAM[spriteIdx * 4];
    
    bool bigSprite = LCDC_REG & OBJ_SIZE_MASK;
    bool flipX = spriteData[3] & 0b100000;
    bool flipY = spriteData[3] & 0b1000000;
    uint8_t palette = spriteData[3] & 0b10000 ? OBP1_REG : OBP0_REG; 
    uint8_t tileIdx = spriteData[2];
    uint8_t* tilePtr = VRAM + tileIdx*16;
    uint8_t height = bigSprite ? 16 : 8;
    for(uint8_t y = 0; y < height; y++)
        for(uint8_t x = 0; x < 8; x++)
            pixels[(screenX + x) + (screenY + y)*width] = getSpritePixelRGB(tilePtr, x, y, palette, flipX, flipY, OBJ_SIZE_MASK);
}

int getSpriteRealX(uint8_t spriteIdx){
    uint8_t x = OAM[spriteIdx*4 + 1];
    return x - 8;
}

int getSpriteRealY(uint8_t spriteIdx){
    uint8_t y = OAM[spriteIdx*4];
    return y - 16;
}

uint8_t getStatRegister(){
    uint8_t output_val;
    output_val = STAT_REG & 0b11111000;
    output_val |= ppu_mode;
    output_val |= lyc_compare << 2;
    output_val |= 0b10000000;
    return output_val;
}

void updatePPU(){
    if(!(LCDC_REG & LCD_ENABLE_MASK)){
        LY_REG = 0;
        internal_ly = 0;
        ppu_mode = 0;
        ppu_counter = 0;
        windowY_counter = 0;
        frameSkip = true;

        if(lastFrameOn){
            for(int i = 0; i < LCD_WIDTH*LCD_HEIGHT; i++)
                workingBufferPtr[i] = colorRGB[0];
            swapBuffers();
        }

        lastFrameOn = false;
        return;
    }

    lastFrameOn = true;
    
    bool old_stat_irq = stat_irq;
    stat_irq = false;

    if(internal_ly < LCD_HEIGHT && ppu_counter == 80 && !frameSkip)
        renderLine(LY_REG);

    // search more about exact ppu_counter
    if(internal_ly == 144 && ppu_counter == 4){
        cpu.IF |= VBLANK_IRQ;

        if(STAT_REG & (1 << 5))
            stat_irq = true;

        windowY_counter = 0;
        frameSkip = false;

        swapBuffers();

        emulateGameShark();
    }

    // old stat mode timing
    if(internal_ly < LCD_HEIGHT){
        if(ppu_counter < 80)
            ppu_mode = OAM_SCAN_MODE;
        else if(ppu_counter < 80 + 172)
            ppu_mode = DRAW_MODE;
        else
            ppu_mode = HBLANK_MODE;
    } else {
        ppu_mode = VBLANK_MODE;
    }

    if(internal_ly == 153 && ppu_counter == 8)
        LY_REG = 0;

    uint8_t stat_interruput_source = (STAT_REG >> 3) & 0b111;
    if((stat_interruput_source & (1 << ppu_mode)))
        stat_irq = true;

    lyc_compare = LY_REG == LYC_REG;
    if((lyc_compare == true) && (STAT_REG & 0x40))
        stat_irq = true;

    if(!old_stat_irq && stat_irq)
        cpu.IF |= STAT_IRQ;

    if(ppu_counter == SCANLINE_CYCLE){        
        internal_ly = (internal_ly + 1) % SCANLINE_NUMBER;
        LY_REG = internal_ly;
        ppu_counter = 0;
    }   else
        ppu_counter++;
}

void swapBuffers(){
    int* tmp = renderBufferPtr;
    renderBufferPtr = workingBufferPtr;
    workingBufferPtr = tmp;
}
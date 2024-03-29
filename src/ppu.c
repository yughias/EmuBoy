#include "SDL_MAINLOOP.h"
#include "gameshark.h"
#include "hardware.h"
#include "ini.h"
#include "menu.h"

#include <stdlib.h>

int workingBuffer[LCD_WIDTH*LCD_HEIGHT];
int renderBuffer[LCD_WIDTH*LCD_HEIGHT];
int* workingBufferPtr = &workingBuffer[0];
int* renderBufferPtr = &renderBuffer[0];

int dmgColors[4];
int backgroundColor;
PPU_MODE ppu_mode;
bool lyc_compare;
bool stat_irq;
size_t ppu_counter;
uint8_t windowY_counter;
uint8_t internal_ly;
bool frameSkip;
bool lastFrameOn;

uint8_t colorCorrection[32] = {
    0, 62, 64, 89, 90, 109, 111, 127, 128, 142, 143, 156, 156, 168, 169, 180, 181, 191, 192, 201, 202, 211, 212, 221, 221, 230, 230, 238, 239, 247, 247, 255
};

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

uint8_t BCPS_REG;
uint8_t OCPS_REG;

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

int sprite_order_compare(const void* a, const void* b){
    sprite_info* sprite_a = (sprite_info*)a;
    sprite_info* sprite_b = (sprite_info*)b;
    
    if(sprite_a->x != sprite_b->x)
        return sprite_b->x - sprite_a->x;
    else
        return sprite_b->idx - sprite_a->idx;
}

void initLcdcMasks(){
    if(console_type == MEGADUCK_TYPE){
        SET_LCDC_MASKS(MEGADUCK);
        return;
    }

    SET_LCDC_MASKS(DMG);
}

int CgbToRgb(uint8_t lo_byte, uint8_t hi_byte){
    int out_r, out_g, out_b;
    uint8_t red = lo_byte & 0x1F;
    uint8_t green = (lo_byte >> 5) | ((hi_byte & 0b11) << 3);
    uint8_t blue = (hi_byte >> 2) & 0x1F;

    red = colorCorrection[red];
    green = colorCorrection[green];
    blue = colorCorrection[blue];

    out_r = (13*red + 2*green + 1*blue) / 16;
    out_g = (3*green + 1*blue) / 4;
    out_b = (2*green + 14*blue) / 16;

    return color(out_r, out_g, out_b);
}

void writeColorToCRAM(uint8_t red, uint8_t green, uint8_t blue, uint8_t* cram, uint8_t idx){
    red >>= 3;
    green >>= 3;
    blue >>= 3;
    cram[idx*2] = red | (green << 5);
    cram[idx*2+1] = (green >> 3) | (blue << 2);
}

void initColorPalette(){
    if(!strcmp(config_gb_palette, "grey")){
        dmgColors[3] = color(16, 16, 16);
        dmgColors[2] = color(88, 88, 88);
        dmgColors[1] = color(160, 160, 160);
        dmgColors[0] = color(232, 232, 232);
        checkRadioButton(menu_greyPaletteBtn);
    } else if(!strcmp(config_gb_palette, "gameboy_light")){ 
        dmgColors[3] = color(0, 79, 59);
        dmgColors[2] = color(0, 105, 74);
        dmgColors[1] = color(0, 154, 113);
        dmgColors[0] = color(0, 181, 129);
        checkRadioButton(menu_gameboyLightPaletteBtn);
    } else if(!strcmp(config_gb_palette, "gameboy_kiosk")){
        dmgColors[3] = color(16, 55, 0);
        dmgColors[2] = color(107, 110, 0);
        dmgColors[1] = color(187, 187, 24);
        dmgColors[0] = color(236, 237, 176); 
        checkRadioButton(menu_gameboyKioskPaletteBtn);
    } else if(!strcmp(config_gb_palette, "super_gameboy")){
        dmgColors[3] = color(49, 24, 82);
        dmgColors[2] = color(173, 41, 33);
        dmgColors[1] = color(222, 148, 74);
        dmgColors[0] = color(255, 239, 206);
        checkRadioButton(menu_superGameboyPaletteBtn);
    } else {
        dmgColors[3] = color(46, 65, 57);
        dmgColors[2] = color(64, 89, 74);
        dmgColors[1] = color(95, 120, 66);
        dmgColors[0] = color(123, 129, 17);
        checkRadioButton(menu_defaultPaletteBtn);
    }

   color(255, 0, 0);

    if(console_type == CGB_TYPE || console_type == DMG_ON_CGB_TYPE)
        backgroundColor = color(255, 255, 255);
    else
        backgroundColor = dmgColors[0];
}

void copyDefaultCgbPalette(){
    const uint8_t default_bgp_cram[CRAM_SIZE] = {
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F,
        0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F
    };
    
    const uint8_t default_obp_cram[CRAM_SIZE] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    memcpy(BGP_CRAM, default_bgp_cram, CRAM_SIZE);
    memcpy(OBP_CRAM, default_obp_cram, CRAM_SIZE);
}

void drawBgRamAt(int screenX, int screenY){
    uint8_t* bgTileMap = getTileMap(LCDC_REG & BG_TILE_MAP_AREA_MASK);

    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
            bool priority;
            int col = getTileMapPixelRGB(bgTileMap, x, y, &priority, &priority);
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
    uint8_t* winTileMap = getTileMap(LCDC_REG & WIN_TILE_MAP_AREA_MASK);

    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
            bool priority;
            int col = getTileMapPixelRGB(winTileMap, x, y, &priority, &priority);
            pixels[screenX+x + (screenY+y)*width] = col;
        }
    }
}

void drawColorAt(int x, int y, int palette, int pal_color, uint8_t* cram){
    uint8_t lo_byte = cram[palette*8 + pal_color*2];
    uint8_t hi_byte = cram[palette*8 + pal_color*2 + 1];
    int col = CgbToRgb(lo_byte, hi_byte);
    rect(x, y, 8, 8, col);
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
    uint8_t colorGB = getColorGb(tilePtr, tilePX, tilePY);

    if(console_type == CGB_TYPE)
        return convertCGB2RGB(colorGB, colorREG, BGP_CRAM);
    else
        return convertDMG2RGB(colorGB, colorREG, BGP_CRAM);
}

uint8_t getColorGb(uint8_t* tilePtr, uint8_t tilePX, uint8_t tilePY){
    uint8_t colorGB = 0;
    colorGB = !!(tilePtr[tilePY*2] & (1 << (7 - tilePX)));
    colorGB |= !!(tilePtr[1+tilePY*2] & (1 << (7 - tilePX))) << 1;

    return colorGB;
}

int convertCGB2RGB(uint8_t colorGB, uint8_t colorREG, uint8_t* cram_palette){
    cram_palette += colorREG*8;
    return CgbToRgb(cram_palette[colorGB*2], cram_palette[colorGB*2+1]);
}

int convertDMG2RGB(uint8_t colorGB, uint8_t colorREG, uint8_t* cram_palette){
    uint8_t idx = (colorREG >> (2 * colorGB)) & 0b11;
    if(console_type == DMG_ON_CGB_TYPE)
        return CgbToRgb(cram_palette[idx*2], cram_palette[idx*2+1]);
    else
        return dmgColors[idx];
}

int getTileMapPixelRGB(uint8_t* tileMapPtr, uint8_t x, uint8_t y, bool* dmgPrio, bool* cgbPrio){
    uint8_t byteX = x >> 3;
    uint8_t byteY = y >> 3;
    uint8_t tilePX = x & 0b111;
    uint8_t tilePY = y & 0b111;
    uint8_t tileIdx = tileMapPtr[byteX+byteY*32];
    uint8_t* tilePtr = getTileData(tileIdx);
    uint8_t paletteReg = BGP_REG;
    uint8_t bg_attributes;

    if(console_type == CGB_TYPE){
        bg_attributes = tileMapPtr[byteX+byteY*32 + 0x2000];
        if(bg_attributes & BG_ATTRIB_BANK_MASK)
            tilePtr += 0x2000;
        if(bg_attributes & BG_ATTRIB_FLIP_X_MASK)
            tilePX = 7 - tilePX;
        if(bg_attributes & BG_ATTRIB_FLIP_Y_MASK)
            tilePY = 7 - tilePY;
        paletteReg = bg_attributes & BG_ATTRIB_PALETTE_MASK;
    }

    uint8_t colorGB = getColorGb(tilePtr, tilePX, tilePY);
    *dmgPrio |= colorGB;
    if(console_type == CGB_TYPE)
        *cgbPrio |= (bg_attributes & BG_ATTRIB_PRIORITY_MASK) && colorGB;

    return getColorFromTileDataRGB(tilePtr, tilePX, tilePY, paletteReg);
}

void renderLine(uint8_t y){
    int col;
    uint8_t* bgTileMap = getTileMap(LCDC_REG & BG_TILE_MAP_AREA_MASK);
    uint8_t* winTileMap = getTileMap(LCDC_REG & WIN_TILE_MAP_AREA_MASK);

    bool bg_win_enabled = LCDC_REG & BG_WIN_ENABLE_MASK;

    bool cgbPrio[LCD_WIDTH] = {false};
    bool dmgPrio[LCD_WIDTH] = {false};
    
    for(uint8_t x = 0; x < LCD_WIDTH; x++){
        if(console_type != CGB_TYPE && !bg_win_enabled)
            col = backgroundColor;
        else
            col = getTileMapPixelRGB(bgTileMap, (SCX_REG + x % 256), (SCY_REG + y) % 256, &dmgPrio[x], &cgbPrio[x]);
        
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
                        workingBufferPtr[winX + y * LCD_WIDTH] = getTileMapPixelRGB(winTileMap, offX % 256, windowY_counter % 256, &dmgPrio[winX], &cgbPrio[winX]);
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
        // these values can be negative!
        int spriteTopY = getSpriteRealY(i);
        if(y >= spriteTopY && ((y < spriteTopY + 8) || (y < spriteTopY + 16 && bigSprite))){
            int spriteTopX = getSpriteRealX(i);
            sprites[drawnSprites].idx = i;
            sprites[drawnSprites].x = spriteTopX;
            sprites[drawnSprites].y = spriteTopY;
            drawnSprites++;
        }
    }

    int startVal;
    int step;
    int limit;

    if(console_type != CGB_TYPE){
        startVal = 0;
        limit = drawnSprites;
        step = 1;
        qsort(sprites, drawnSprites, sizeof(sprite_info), sprite_order_compare);
    } else {
        startVal = drawnSprites - 1;
        limit = -1;
        step = -1;
    }

    for(int i = startVal; i != limit; i += step){
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
                bool flipX, flipY;
                bool backgroundOver;
                bool obp_n;
                uint8_t palette;
                uint8_t* tilePtr;

                getSpriteAttribute(spriteData, &flipX, &flipY, &backgroundOver, &obp_n, &palette, &tilePtr);

                bool transparent;
                col = getSpritePixelRGB(tilePtr, spriteX, spriteY, obp_n, palette, flipX, flipY, bigSprite, &transparent);
                if(!transparent)
                    if(
                        // basic dmg background priority condition
                        (!backgroundOver || (backgroundOver && !dmgPrio[screenX])) &&
                        // cgb priority condition
                        (console_type != CGB_TYPE || !cgbPrio[screenX] || !bg_win_enabled)
                    )
                        workingBufferPtr[screenX + y * LCD_WIDTH] = col;

                screenX++;
                spriteX++;
            }
        }
    }
}

void getSpriteAttribute(uint8_t* spriteData, bool* flipX, bool* flipY, bool* backgroundOver, bool* obp_n, uint8_t* palette, uint8_t** tilePtr){
    *flipX = spriteData[3] & 0b100000;
    *flipY = spriteData[3] & 0b1000000;
    *backgroundOver = spriteData[3] & 0b10000000;
    *obp_n = spriteData[3] & 0b10000;
    if(console_type == CGB_TYPE)
        *palette = spriteData[3] & 0b111;
    else
        *palette = *obp_n ? OBP1_REG : OBP0_REG; 
    uint8_t tileIdx = spriteData[2];
    if(LCDC_REG & OBJ_SIZE_MASK)
        tileIdx &= 0b11111110;
    *tilePtr = VRAM + tileIdx*16;
    if(console_type == CGB_TYPE && (spriteData[3] & 0b1000))
        *tilePtr += 0x2000;
}

int getSpritePixelRGB(uint8_t* tilePtr, uint8_t x, uint8_t y, bool obp_n, uint8_t palette, bool flipX, bool flipY, bool bigSprite, bool* transparent){
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

    if(colorGB == 0){
        *transparent = true;
        return -1;
    } else {
        *transparent = false;
    }

    if(console_type == CGB_TYPE)
        return convertCGB2RGB(colorGB, palette, OBP_CRAM);
    else
        return convertDMG2RGB(colorGB, palette, OBP_CRAM);
}

void drawOAMAt(int screenX, int screenY, uint8_t spriteIdx){
    uint8_t* spriteData = &OAM[spriteIdx * 4];
    
    bool flipX, flipY;
    bool backgroundOver;
    bool obp_n;
    uint8_t palette;
    uint8_t* tilePtr;

    getSpriteAttribute(spriteData, &flipX, &flipY, &backgroundOver, &obp_n, &palette, &tilePtr);

    uint8_t height = LCDC_REG & OBJ_SIZE_MASK ? 16 : 8;
    for(uint8_t y = 0; y < height; y++)
        for(uint8_t x = 0; x < 8; x++){
            bool transparent;
            pixels[(screenX + x) + (screenY + y)*width] = getSpritePixelRGB(tilePtr, x, y, obp_n, palette, flipX, flipY, height, &transparent);
        }
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

        if(lastFrameOn)
            renderLcdOff();

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
        PPU_MODE old_mode = ppu_mode;

        if(ppu_counter < 80)
            ppu_mode = OAM_SCAN_MODE;
        else if(ppu_counter < 80 + 172)
            ppu_mode = DRAW_MODE;
        else
            ppu_mode = HBLANK_MODE;

        if(console_type == CGB_TYPE)
            if(old_mode != HBLANK_MODE && ppu_mode == HBLANK_MODE && !cpu.HALTED)
                stepHDMA();
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

void renderLcdOff(){
    for(int i = 0; i < LCD_WIDTH*LCD_HEIGHT; i++){
        workingBufferPtr[i] = backgroundColor;
        renderBufferPtr[i] = backgroundColor;
    }
    swapBuffers();
    lastFrameOn = true;
}
#ifndef __PPU_H__
#define __PPU_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH 160
#define LCD_HEIGHT 144
#define SCANLINE_NUMBER 154
#define SCANLINE_CYCLE 456

#define LY_ADDR 0xFF44
#define LYC_ADDR 0xFF45
#define LCDC_ADDR 0xFF40
#define STAT_ADDR 0xFF41
#define SCX_ADDR 0xFF43
#define SCY_ADDR 0xFF42
#define BGP_ADDR 0xFF47
#define OBP0_ADDR 0xFF48
#define OBP1_ADDR 0xFF49
#define WX_ADDR 0xFF4B
#define WY_ADDR 0xFF4A

#define DMG_LCD_ENABLE_MASK            0b10000000
#define DMG_WIN_TILE_MAP_AREA_MASK     0b01000000
#define DMG_WIN_ENABLE_MASK            0b00100000
#define DMG_BG_WIN_TILE_DATA_AREA_MASK 0b00010000
#define DMG_BG_TILE_MAP_AREA_MASK      0b00001000
#define DMG_OBJ_SIZE_MASK              0b00000100
#define DMG_OBJ_ENABLE_MASK            0b00000010
#define DMG_BG_WIN_ENABLE_MASK         0b00000001

#define BG_ATTRIB_PALETTE_MASK         0b00000111
#define BG_ATTRIB_BANK_MASK            0b00001000
#define BG_ATTRIB_FLIP_X_MASK          0b00100000
#define BG_ATTRIB_FLIP_Y_MASK          0b01000000
#define BG_ATTRIB_PRIORITY_MASK        0b10000000

#define MEGADUCK_LCD_ENABLE_MASK            0b10000000
#define MEGADUCK_WIN_TILE_MAP_AREA_MASK     0b00001000
#define MEGADUCK_WIN_ENABLE_MASK            0b00100000
#define MEGADUCK_BG_WIN_TILE_DATA_AREA_MASK 0b00010000
#define MEGADUCK_BG_TILE_MAP_AREA_MASK      0b00000100
#define MEGADUCK_OBJ_SIZE_MASK              0b00000010
#define MEGADUCK_OBJ_ENABLE_MASK            0b00000001
#define MEGADUCK_BG_WIN_ENABLE_MASK         0b01000000

typedef struct gb_t gb_t;

typedef enum {HBLANK_MODE = 0, VBLANK_MODE, OAM_SCAN_MODE, DRAW_MODE} PPU_MODE;

typedef struct ppu_t {
    uint8_t LCD_ENABLE_MASK;            
    uint8_t WIN_TILE_MAP_AREA_MASK;   
    uint8_t WIN_ENABLE_MASK;    
    uint8_t BG_WIN_TILE_DATA_AREA_MASK;
    uint8_t BG_TILE_MAP_AREA_MASK;
    uint8_t OBJ_SIZE_MASK;     
    uint8_t OBJ_ENABLE_MASK;            
    uint8_t BG_WIN_ENABLE_MASK;         

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

    PPU_MODE mode;
    bool lyc_compare;
    size_t counter;
    bool stat_irq;
    uint8_t internal_ly;

    int workingBuffer[LCD_WIDTH*LCD_HEIGHT];
    int renderBuffer[LCD_WIDTH*LCD_HEIGHT];
    int* workingBufferPtr;
    int* renderBufferPtr;

    int dmgColors[4];
    int backgroundColor;
    uint8_t windowY_counter;
    bool frameSkip;
    bool lastFrameOn;
} ppu_t;

void initColorPalette(gb_t*);
void copyDefaultCgbPalette(gb_t*);
void drawBgRamAt(gb_t*, int, int);
void drawWinRamAt(gb_t*, int, int);
void drawOAMAt(gb_t*, int, int, uint8_t);
void drawColorAt(int, int, int, int, uint8_t*);

uint8_t* getTileMap(gb_t*, bool);
uint8_t* getTileData(gb_t*, uint8_t);

int getColorFromTileDataRGB(gb_t*, uint8_t*, uint8_t, uint8_t, uint8_t);
int convertDMG2RGB(gb_t*, uint8_t, uint8_t, uint8_t*);
int convertCGB2RGB(gb_t*, uint8_t, uint8_t, uint8_t*);
uint8_t getColorGb(uint8_t* tilePtr, uint8_t tilePX, uint8_t tilePY);
int getTileMapPixelRGB(gb_t*, uint8_t* tileMapPtr, uint8_t x, uint8_t y, bool* dmgPrio, bool* cgbPrio);
int getSpritePixelRGB(gb_t*, uint8_t* tilePtr, uint8_t x, uint8_t y, bool obp_n, uint8_t palette, bool flipX, bool flipY, bool bigSprite, bool* transparent);
void getSpriteAttribute(gb_t*, uint8_t* spriteData, bool* flipX, bool* flipY, bool* backgroundOver, bool* obp_n, uint8_t* palette, uint8_t** tilePtr);

int getSpriteRealX(gb_t*, uint8_t);
int getSpriteRealY(gb_t*, uint8_t);

void renderLine(gb_t*, uint8_t);
void renderLcdOff(ppu_t*);
void initLcdcMasks(gb_t*);
void updatePPU(gb_t*);
uint8_t getStatRegister(ppu_t*);

void writeColorToCRAM(uint8_t, uint8_t, uint8_t, uint8_t*, uint8_t);
int CgbToRgb(uint8_t, uint8_t);

void swapBuffers(ppu_t*);

#endif
#include "menu.h"
#include "post_rendering.h"
#include "ini.h"
#include "serial.h"
#include "ppu.h"

#define generateVideoMenuCallback(func_name, filter_name) void use ## func_name ## Menu(){ strcpy(config_render, filter_name); setupWindow(); }
#define generatePaletteMenuCallback(func_name, palette_name) void set ## func_name ## Palette(){ strcpy(config_gb_palette, palette_name); initColorPalette(); }

generateVideoMenuCallback(Nearest, "nearest");
generateVideoMenuCallback(Linear, "linear");
generateVideoMenuCallback(Matrix, "matrix");
generateVideoMenuCallback(Classic, "dmg");
generateVideoMenuCallback(Scale2x, "scale2x");
generateVideoMenuCallback(Scale3x, "scale3x");
generateVideoMenuCallback(Hq2x, "hq2x");
generateVideoMenuCallback(Hq3x, "hq3x");
generateVideoMenuCallback(Debug, "debug");

generatePaletteMenuCallback(Default, "");
generatePaletteMenuCallback(Grey, "grey");
generatePaletteMenuCallback(SuperGameboy, "super_gameboy");
generatePaletteMenuCallback(GameboyLight, "gameboy_light");
generatePaletteMenuCallback(GameboyKiosk, "gameboy_kiosk");

buttonId menu_nearestBtn;
buttonId menu_linearBtn;
buttonId menu_matrixBtn;
buttonId menu_classicBtn;
buttonId menu_scale2xBtn;
buttonId menu_scale3xBtn;
buttonId menu_hq2xBtn;
buttonId menu_hq3xBtn;
buttonId menu_debugBtn;

buttonId menu_defaultPaletteBtn;
buttonId menu_greyPaletteBtn;
buttonId menu_superGameboyPaletteBtn;
buttonId menu_gameboyLightPaletteBtn;
buttonId menu_gameboyKioskPaletteBtn;


void showIniAndReload();

void createMenuBar(){
    addButtonTo(-1, L"configs", showIniAndReload);

    menuId videoMenu = addMenuTo(-1, L"video", false);

    menuId filterMenu = addMenuTo(videoMenu, L"filter", true);
    menu_nearestBtn = addButtonTo(filterMenu, L"nearest", useNearestMenu);
    menu_linearBtn = addButtonTo(filterMenu, L"linear", useLinearMenu);
    menu_matrixBtn = addButtonTo(filterMenu, L"matrix", useMatrixMenu);
    menu_classicBtn = addButtonTo(filterMenu, L"classic", useClassicMenu);
    menu_scale2xBtn = addButtonTo(filterMenu, L"scale2x", useScale2xMenu);
    menu_scale3xBtn = addButtonTo(filterMenu, L"scale3x", useScale3xMenu);
    menu_hq2xBtn = addButtonTo(filterMenu, L"hq2x", useHq2xMenu);
    menu_hq3xBtn = addButtonTo(filterMenu, L"hq3x", useHq3xMenu);
    menu_debugBtn = addButtonTo(filterMenu, L"debug", useDebugMenu);
    
    menuId paletteMenu = addMenuTo(videoMenu, L"GB palette", true);
    menu_defaultPaletteBtn = addButtonTo(paletteMenu, L"default", setDefaultPalette);
    menu_greyPaletteBtn = addButtonTo(paletteMenu, L"grey", setGreyPalette);
    menu_superGameboyPaletteBtn = addButtonTo(paletteMenu, L"super gameboy", setSuperGameboyPalette);
    menu_gameboyLightPaletteBtn = addButtonTo(paletteMenu, L"gameboy light", setGameboyLightPalette);
    menu_gameboyKioskPaletteBtn = addButtonTo(paletteMenu, L"gameboy kiosk", setGameboyKioskPalette);
}

void showIniAndReload(){ 
    char ini_path[FILENAME_MAX];
    char system_cmd[FILENAME_MAX];
    getAbsoluteDir(ini_path);
    strcat(ini_path, "data/config.ini");
    sprintf(system_cmd, "notepad \"%s\"", ini_path);
    
    system(system_cmd);

    loadConfigFile(ini_path);
    initSerial();
    setupWindow();
}
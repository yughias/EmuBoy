#include "menu.h"
#include "post_rendering.h"
#include "ini.h"
#include "serial.h"

#define generateVideoMenuCallback(func_name, filter_name) void use ## func_name ## Menu(){ strcpy(config_render, filter_name); setupWindow(); }

generateVideoMenuCallback(Nearest, "nearest");
generateVideoMenuCallback(Linear, "linear");
generateVideoMenuCallback(Matrix, "matrix");
generateVideoMenuCallback(Classic, "dmg");
generateVideoMenuCallback(Scale2x, "scale2x");
generateVideoMenuCallback(Scale3x, "scale3x");
generateVideoMenuCallback(Debug, "debug");

buttonId menu_nearestBtn;
buttonId menu_linearBtn;
buttonId menu_matrixBtn;
buttonId menu_classicBtn;
buttonId menu_scale2xBtn;
buttonId menu_scale3xBtn;
buttonId menu_debugBtn;

void showIniAndReload();

void createMenuBar(){
    addButtonTo(-1, L"configs", showIniAndReload);
    menuId filterMenu = addMenuTo(-1, L"video", true);
    menu_nearestBtn = addButtonTo(filterMenu, L"nearest", useNearestMenu);
    menu_linearBtn = addButtonTo(filterMenu, L"linear", useLinearMenu);
    menu_matrixBtn = addButtonTo(filterMenu, L"matrix", useMatrixMenu);
    menu_classicBtn = addButtonTo(filterMenu, L"classic", useClassicMenu);
    menu_scale2xBtn = addButtonTo(filterMenu, L"scale2x", useScale2xMenu);
    menu_scale3xBtn = addButtonTo(filterMenu, L"scale3x", useScale3xMenu);
    menu_debugBtn = addButtonTo(filterMenu, L"debug", useDebugMenu);
}

void showIniAndReload(){ 
    char ini_path[FILENAME_MAX];
    char system_cmd[FILENAME_MAX];
    getAbsoluteDir(ini_path);
    strcat(ini_path, "data/config.ini");
    sprintf(system_cmd, "notepad \'%s\'", ini_path);
    
    system(system_cmd);

    loadConfigFile(ini_path);
    initSerial();
    setupWindow();
}
#include <SDL_MAINLOOP.h>
#include <hardware.h>
#include <info.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void closeEmulator(){
    if(hasBattery)
        saveSav(savName);
    freeMemory();
    freeAudio();
}

void setup(){
    size(LCD_WIDTH, LCD_HEIGHT);
    setTitle(u8"ゲーム　ボーイ　ちゃん");
    frameRate(REFRESH_RATE);

    char logoPath[FILENAME_MAX];
    getAbsoluteDir(logoPath);
    strcat(logoPath, "data/logo.bmp");
    setWindowIcon(logoPath);

    setScaleMode(NEAREST);

    if(getArgc() != 2){
        printf("emulator.exe <rom name>\n");
        exit(1);
    }

    setFilename(getArgv(1));

    initCPU(&cpu);
    initMemory();
    initAudio();

    char bootromName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    strcat(bootromName, "data/dmg_boot.bin");
    loadBootRom(bootromName);

    loadRom(romName);
    detectMBC();
    if(hasBattery)
        loadSav(savName);

    #ifdef DEBUG
    freopen("log.txt", "wb", stderr);
    #endif

    printInfo(ROM);

    onExit = closeEmulator;
    noRender();
}

void loop(){
    char string[64];
    sprintf(string, "%d\n", (int)(1000.0f / deltaTime + 0.5f));
    setTitle(string);

    if(frameCount == 1)
        initPaletteRGB();
    
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    emulateJoypad(keystate);

    int speed = keystate[SDL_SCANCODE_TAB] ? 8 : 1;
    for(int i = 0; i < speed; i++)
        emulateHardware(&cpu);

    //drawBgRamAt(width/2, 0);
    //drawWinRamAt(width/2, height/2);

    bool bigSprite = LCDC_REG && OBJ_SIZE_MASK;
    int offY = bigSprite ? 16 : 8;
    for(int i = 0; i < 40; i++){
        //drawOAMAt((i%8)*8, height/2+(i/8)*offY, i);
    }
}
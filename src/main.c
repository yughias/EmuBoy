#include "SDL_MAINLOOP.h"
#include "hardware.h"
#include "gameshark.h"
#include "info.h"
#include "post_rendering.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void closeEmulator(){
    if(hasBattery)
        saveSav(savName);
    freeGameShark();
    freeMemory();
    freeAudio();
    freeSerial();
}

void setup(){
    setupWindow();
    setTitle(u8"エミュボーイ");
    frameRate(REFRESH_RATE);

    char logoPath[FILENAME_MAX];
    getAbsoluteDir(logoPath);
    strcat(logoPath, "data/logo.bmp");
    setWindowIcon(logoPath);

    if(getArgc() != 2){
        printf("emulator.exe <rom name>\n");
        exit(1);
    }

    setFilename(getArgv(1));

    initCPU(&cpu);
    initMemory(romName);
    initAudio();
    initSerial();
    initJoypad();
    initPaletteRGB();
    background(colorRGB[0]);

    char bootromName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    strcat(bootromName, "data/dmg_boot.bin");
    if(!loadBootRom(bootromName))
        skipBootrom();

    char gamesharkName[FILENAME_MAX];
    getAbsoluteDir(gamesharkName);
    strcat(gamesharkName, "data/gameshark.txt");
    loadGameShark(gamesharkName);

    #ifdef DEBUG
    freopen("log.txt", "wb", stderr);
    #endif

    printInfo(ROM);

    onExit = closeEmulator;
}

void loop(){
    #ifdef SPEED_TEST
    static float avg = 0.0f;
    if(frameCount > 5){
        avg += 1000.0f / deltaTime; 
    }

    if(cpu.PC >= 0x100){
        avg /= frameCount - 5;
        printf("SPEED TEST FINISHED\n");
        printf("ideal speedup: x%d\n", SPEED_TEST);
        printf("real speedup: x%f\n", avg / REFRESH_RATE * SPEED_TEST);
        printf("average fps: %f\n", avg * SPEED_TEST);
        system("pause");
        closeEmulator();
        exit(0);
    }
    #endif

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    #ifndef SPEED_TEST
    int speed = keystate[SDL_SCANCODE_TAB] ? 16 : 1;
    #else
    int speed = SPEED_TEST;
    #endif
    for(int i = 0; i < speed; i++)
        emulateHardware(&cpu);

    (*renderDisplay)();

    /*
    drawBgRamAt(width/2, 0);
    drawWinRamAt(width/2, height/2);

    bool bigSprite = LCDC_REG & OBJ_SIZE_MASK;
    int offY = bigSprite ? 16 : 8;
    for(int i = 0; i < 40; i++){
        drawOAMAt((i%8)*8, height/2+(i/8)*offY, i);
    }
    */
}

#ifdef __EMSCRIPTEN__

void emscripten_loadRom(const char* filename){
    setFilename(filename);

    turnOffAudioChannels();
    freeMemory();
    initMemory(romName);
    
    skipBootrom();
}

#endif
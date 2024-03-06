#include "SDL_MAINLOOP.h"
#include "hardware.h"
#include "gameshark.h"
#include "info.h"
#include "ini.h"
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
    char ini_path[FILENAME_MAX];
    getAbsoluteDir(ini_path);
    strcat(ini_path, "data/config.ini");
    loadConfigFile(ini_path);

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

    initAudio();
    initSerial();
    initJoypad();

    initConsole();

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
}

#ifdef __EMSCRIPTEN__

void emscripten_loadRom(const char* filename){
    setFilename(filename);

    turnOffAudioChannels();
    freeMemory();

    initConsole();
}

#endif
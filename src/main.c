#include "SDL_MAINLOOP.h"
#include "gb.h"
#include "gameshark.h"
#include "info.h"
#include "ini.h"
#include "post_rendering.h"
#include "menu.h"
#include "mbcs/mbc3.h"
#include "mbcs/mbc_cam.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gb_t gb;

void closeEmulator(){
    mbc_t* mbc = &gb.mbc;
    if(mbc->hasBattery)
        saveSav(&gb, savName);
    if(mbc->hasRtc)
        saveRtc((rtc_t*)gb.mbc.data, savName);
    if(mbc->hasCamera)
        mbc_cam_free();
    freeGameShark();
    freeMemory(&gb);
    freeAudio(&gb.apu);
    freeSerial();
}

void setup(){
    createMenuBar();

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

    initSerial();
    initJoypad();
    initConsole(&gb);

    #ifdef DEBUG
    freopen("log.txt", "wb", stderr);
    #endif

    printInfo(gb.ROM, gb.ROM_SIZE);

    onExit = closeEmulator;
}

void loop(){
    #ifdef SPEED_TEST
    static float avg = 0.0f;
    if(frameCount > 5){
        avg += 1000.0f / deltaTime; 
    }

    if(gb.cpu.PC >= 0x100){
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

    if(gameController)
        setGameControllerLed();

    emulateTurboButton();
    #ifndef SPEED_TEST
    int speed = keystate[SDL_SCANCODE_TAB] || GAMECONTROLLER_CHECK(RIGHTSHOULDER) ? 16 : 1;
    #else
    int speed = SPEED_TEST;
    #endif
    gb.apu.push_rate_reload = speed*PUSH_RATE_RELOAD;
    for(int i = 0; i < speed; i++)
        emulateHardware(&gb);

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
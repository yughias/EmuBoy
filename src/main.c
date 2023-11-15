#include <SDL_MAINLOOP.h>
#include <hardware.h>
#include <gameshark.h>
#include <info.h>

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
    size(LCD_WIDTH, LCD_HEIGHT);
    setTitle(u8"エミュボーイ");
    #ifndef SPEED_TEST
    frameRate(REFRESH_RATE);
    #else
    frameRate(2000);
    #endif

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
    initMemory(romName);
    initAudio();
    initSerial();
    initJoypad();

    char bootromName[FILENAME_MAX];
    getAbsoluteDir(bootromName);
    strcat(bootromName, "data/dmg_boot.bin");
    if(!loadBootRom(bootromName))
        skipBootrom();

    detectMBC();
    if(hasBattery)
        loadSav(savName);

    char gamesharkName[FILENAME_MAX];
    getAbsoluteDir(gamesharkName);
    strcat(gamesharkName, "data/gameshark.txt");
    loadGameShark(gamesharkName);

    #ifdef DEBUG
    freopen("log.txt", "wb", stderr);
    #endif

    printInfo(ROM);

    onExit = closeEmulator;
    noRender();
}

void loop(){
    #ifdef SPEED_TEST
    static float avg = 0.0f;
    if(frameCount > 1){
        avg += 1000.0f / deltaTime; 
    }

    if(*cpu.PC >= 0x100){
        avg /= frameCount - 1;
        printf("%f\n", avg);
        system("pause");
        closeEmulator();
        exit(0);
    }
    #endif

    if(frameCount == 1)
        initPaletteRGB();

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
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
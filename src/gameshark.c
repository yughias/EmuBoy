#include "gameshark.h"
#include "gb.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t type;
    uint8_t byte;
    uint16_t addr;
} gameshark_code_t;

gameshark_code_t* codes = NULL;
size_t n_code = 0;

void loadGameShark(const char* filename){
    FILE* fptr = fopen(filename, "r");
    if(!fptr)
        return;

    int data;
    while(fscanf(fptr, "%2X", &data) != EOF){
        if(!n_code)
            codes = malloc(sizeof(gameshark_code_t) * (++n_code));
        else
            codes = realloc(codes, sizeof(gameshark_code_t) * (++n_code));

        codes[n_code-1].type = data;
        fscanf(fptr, "%2X", &data);
        codes[n_code-1].byte = data;
        fscanf(fptr, "%2X", &data);
        codes[n_code-1].addr = data;
        fscanf(fptr, "%2X", &data);
        codes[n_code-1].addr |= data << 8;
    }
}

void emulateGameShark(){
    for(size_t i = 0; i < n_code; i++){
        switch(codes[i].type){
            case 0x01:
            cpu.writeByte(codes[i].addr, codes[i].byte);
            break;
        }
    }
}

void freeGameShark(){
    free(codes);
    n_code = 0;
}
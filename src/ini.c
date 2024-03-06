#include "ini.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int config_listen_port;
int config_connect_port;
char config_connect_ip[CONFIG_STRING_SIZE];
char config_render[CONFIG_STRING_SIZE];
int config_force_dmg_when_possible;

#define READ_INT(var) if(!INI_getInt(ini_ptr, #var, &config_ ## var)) config_ ## var = 0
#define READ_STRING(var) if(!INI_getString(ini_ptr, #var, config_ ## var)) config_ ## var[0] = 0

FILE* INI_open(const char*);
void INI_close(FILE*);
bool INI_getString(FILE*, const char*, char*);
bool INI_getInt(FILE*, const char*, int*);
bool INI_iterate(FILE* ini_ptr, const char* var_name, void* dst, char* format);

FILE* INI_open(const char* filename){
    return fopen(filename, "r");
}

void INI_close(FILE* ini_ptr){
    fclose(ini_ptr);
}

bool INI_getString(FILE* ini_ptr, const char* var_name, char* dst){
    return INI_iterate(ini_ptr, var_name, (void*)dst, "%s");
}

bool INI_getInt(FILE* ini_ptr, const char* var_name, int* dst){
    return INI_iterate(ini_ptr, var_name, (void*)dst, "%d");
}

bool INI_iterate(FILE* ini_ptr, const char* var_name, void* dst, char* format){
    rewind(ini_ptr);
    
    char search_buf[FILENAME_MAX];
    
    while(fscanf(ini_ptr, "%s", search_buf) != EOF){
        if(!strncmp(var_name, search_buf, strlen(var_name))){
            return sscanf(search_buf + strlen(var_name) + 1, format, dst) > 0 ? true : false;
        }
    }
    return false;
}

void loadConfigFile(const char* filename){
    FILE* ini_ptr = INI_open(filename);
    if(!ini_ptr)
        return;
    
    READ_INT(connect_port);
    READ_INT(listen_port);
    READ_STRING(connect_ip);
    READ_STRING(render);
    READ_INT(force_dmg_when_possible);

    INI_close(ini_ptr);
}
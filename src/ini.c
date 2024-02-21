#include "ini.h"

#include <string.h>

FILE* INI_open(const char* filename){
    return fopen(filename, "r");
}

void INI_close(FILE* ini_ptr){
    fclose(ini_ptr);
}

bool INI_getString(FILE* ini_ptr, const char* var_name, char* dst){
    rewind(ini_ptr);
    
    char search_buf[FILENAME_MAX];
    
    while(fscanf(ini_ptr, "%s", search_buf) != EOF){
        if(!strncmp(var_name, search_buf, strlen(var_name))){
            return sscanf(search_buf + strlen(var_name) + 1, "%s", dst) > 0 ? true : false;
        }
    }
    return false;
}

bool INI_getInt(FILE* ini_ptr, const char* var_name, int* dst){
    rewind(ini_ptr);
    
    char search_buf[FILENAME_MAX];

    while(fscanf(ini_ptr, "%s", search_buf)){
        if(!strncmp(var_name, search_buf, strlen(var_name))){
            return sscanf(search_buf + strlen(var_name) + 1, "%d", dst) > 0 ? true : false;
        }
    }
    return false;
}
#include "ini.h"

#include <string.h>

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

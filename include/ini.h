#ifndef __INI_H__
#define __INI_H__

#include <stdio.h>
#include <stdbool.h>

FILE* INI_open(const char*);
void INI_close(FILE*);
bool INI_getString(FILE*, const char*, char*);
bool INI_getInt(FILE*, const char*, int*);

#endif
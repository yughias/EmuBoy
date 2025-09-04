#ifndef __GAMESHARK_H__
#define __GAMESHARK_H__

typedef struct gb_t gb_t;

void loadGameShark(const char*);
void freeGameShark();
void emulateGameShark(gb_t*);

#endif
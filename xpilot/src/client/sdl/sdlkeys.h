#ifndef SDLKEYS_H
#define SDLKEYS_H

#include "SDL.h"
#include "keys.h"


SDLKey Get_key_by_name(const char* name);
char *Get_name_by_key(SDLKey key);

typedef struct {
    keys_t key;
    void *next;
} keylist;

keylist     	*keyMap[SDLK_LAST];   /* maps SDLKeys to keys_t */
keys_t          buttonMap[5];        /* maps mouse buttons to keys_t */
void freeKeyMap(void);
#endif

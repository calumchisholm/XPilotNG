#ifndef SDLKEYS_H
#define SDLKEYS_H

#include "SDL.h"

#define NUM_MOUSE_BUTTONS 5

SDLKey Get_key_by_name(const char* name);
char *Get_name_by_key(SDLKey key);

#endif

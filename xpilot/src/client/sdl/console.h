#ifndef CONSOLE_H
#define CONSOLE_H

#include "SDL.h"

int Console_init(void);
void Console_paint(void);
void Console_show(void);
void Console_hide(void);
int Console_isVisible(void);
int Console_process(SDL_Event *e);
void Console_cleanup(void);
void Console_print(const char *str, ...);

#endif

#ifndef SDLWINDOW_H
#define SDLWINDOW_H

#include "SDL.h"

typedef struct {
    int tx_id;
    SDL_Surface *surface;
    int x, y, w, h;
} sdl_window_t;

int sdl_window_init(sdl_window_t *win, int x, int y, int w, int h);
void sdl_window_move(sdl_window_t *win, int x, int y);
int sdl_window_resize(sdl_window_t *win, int w, int h);
void sdl_window_refresh(sdl_window_t *win);
void sdl_window_paint(sdl_window_t *win);
void sdl_window_destroy(sdl_window_t *win);

#endif

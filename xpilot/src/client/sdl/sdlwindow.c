#include <GL/gl.h>
#include "SDL.h"
#include "sdlwindow.h"
#include "error.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

static int next_p2(int t) 
{
    int r = 1;
    while (r < t) r <<= 1;
    return r;   
}

int sdl_window_init(sdl_window_t *win, int x, int y, int w, int h)
{
    glGenTextures(1, &win->tx_id);
    win->surface = NULL;
    if (sdl_window_resize(win, w, h)) {
	warn("failed to resize window");
	return -1;
    }
    sdl_window_move(win, x, y);
    return 0;
}

void sdl_window_move(sdl_window_t *win, int x, int y)
{
    win->x = x;
    win->y = y;
}

int sdl_window_resize(sdl_window_t *win, int width, int height)
{
    SDL_Surface *surface = 
	SDL_CreateRGBSurface(SDL_SWSURFACE, 
			     next_p2(width), next_p2(height), 
			     32, RMASK, GMASK, BMASK, AMASK);
    if (!surface) {
	error("failed to create SDL surface: %s", SDL_GetError());
	return -1;
    }

    if (win->surface != NULL)
	SDL_FreeSurface(win->surface);

    win->surface = surface;
    win->w = width;
    win->h = height;
    return 0;
}

void sdl_window_refresh(sdl_window_t *win)
{
    glBindTexture(GL_TEXTURE_2D, win->tx_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
		 win->surface->w, win->surface->h, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 
		 win->surface->pixels);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                    GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST);
}

void sdl_window_paint(sdl_window_t *win)
{
    glBindTexture(GL_TEXTURE_2D, win->tx_id);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(255, 255, 255, 255);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); 
    glVertex2i(win->x, win->y);
    glTexCoord2f((GLfloat)win->w / win->surface->w, 0); 
    glVertex2i(win->x + win->w , win->y);
    glTexCoord2f((GLfloat)win->w / win->surface->w, 
		 (GLfloat)win->h / win->surface->h);
    glVertex2i(win->x + win->w , win->y + win->h);
    glTexCoord2f(0, (GLfloat)win->h / win->surface->h);
    glVertex2i(win->x, win->y + win->h);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void sdl_window_destroy(sdl_window_t *win)
{
    glDeleteTextures(1, &win->tx_id);
    if (win->surface != NULL)
	SDL_FreeSurface(win->surface);
}


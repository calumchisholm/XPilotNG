#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"

int draw_width;
int draw_height;

int Init_playing_windows(void)
{
    const SDL_VideoInfo *info;
    Uint8  video_bpp;
    Uint32 videoflags;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    videoflags = SDL_HWSURFACE|SDL_OPENGL;//|SDL_FULLSCREEN;

    draw_width=1024;
    draw_height=768;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (SDL_SetVideoMode(draw_width, draw_height, 24, videoflags) == NULL) {
        error("failed to set video mode: %s", SDL_GetError());
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, draw_width, draw_height);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, draw_width, draw_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); 

    /* Use alpha blending */
    /*    SDL_SetAlpha(Surface, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);*/
	
    /* Set title for window */
    SDL_WM_SetCaption(TITLE, NULL);
    
    /*
    sdl_init_colors();
    Init_spark_colors();
    Radar_init();
    */
}

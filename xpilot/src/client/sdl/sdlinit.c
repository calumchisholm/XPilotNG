#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"

int draw_width;
int draw_height;
int draw_depth;
int num_spark_colors;

int Init_playing_windows(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    draw_width=1024;
    draw_height=768;
    draw_depth=24;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (SDL_SetVideoMode(draw_width, 
			 draw_height, 
			 draw_depth, 
			 SDL_HWSURFACE|SDL_OPENGL //|SDL_FULLSCREEN
			 ) == NULL) {
        error("failed to set video mode: %s", SDL_GetError());
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, draw_width, draw_height);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, draw_width, 0, draw_height);
    glMatrixMode(GL_MODELVIEW);

    /* Set title for window */
    SDL_WM_SetCaption(TITLE, NULL);
    
    /*
    sdl_init_colors();
    Init_spark_colors();
    */
    Radar_init();
    return 0;
}

void Quit(void) 
{
    Radar_cleanup();
    SDL_Quit();
}

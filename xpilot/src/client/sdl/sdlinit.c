#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"
#include "text.h"
#include "console.h"
#include "sdlkeys.h"

int draw_depth;

int Init_playing_windows(void)
{
    char defaultfont[] = "defaultfont.bmp"; /* TODO make bmp fonts work */
    char testfont[] = "Test.ttf";
    int fontsize = 12;
    int mapfontsize = 12;
    /*char testfont[] = "/doze/windows/fonts/trebuc.ttf";*/
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    draw_depth=24;
    num_spark_colors=8;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (SDL_SetVideoMode(draw_width, 
			 draw_height, 
			 draw_depth, 
			 SDL_HWSURFACE|SDL_OPENGL|SDL_RESIZABLE //|SDL_FULLSCREEN
			 ) == NULL) {
        error("failed to set video mode: %s", SDL_GetError());
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, draw_width, draw_height);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, draw_width, 0, draw_height);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* Set title for window */
    SDL_WM_SetCaption(TITLE, NULL);
    
    if (fontinit(&gamefont,testfont,fontsize)) {
    	error("fontinit failed with %s, reverting to default font %s",testfont,defaultfont);
	if (fontinit(&gamefont,defaultfont,fontsize))
	    error("Default font failed! gamefont not available!");
    }
    if (fontinit(&messagefont,testfont,fontsize)) {
    	error("fontinit failed with %s, reverting to default font %s",testfont,defaultfont);
	if (fontinit(&gamefont,defaultfont,fontsize))
	    error("Default font failed! messagefont not available!");
    }
    if (fontinit(&mapfont,testfont,mapfontsize)) {
    	error("fontinit failed with %s, reverting to default font %s",testfont,defaultfont);
	if (fontinit(&mapfont,defaultfont,mapfontsize))
	    error("Default font failed! messagefont not available!");
    }
        
    /*
    sdl_init_colors();
    Init_spark_colors();
    */
    if (Radar_init()) {
	error("radar initialization failed");
	return -1;
    }
    if (Console_init()) {
	error("console initialization failed");
	return -1;
    }
    return 0;
}

void Quit(void) 
{
    Console_cleanup();
    Radar_cleanup();
    fontclean(&gamefont);
    fontclean(&messagefont);
    freeKeyMap();
    SDL_Quit();
}

/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client. Copyright (C) 2003-2004 by 
 *
 *      Juha Lindström       <juhal@users.sourceforge.net>
 *      Erik Andersson       <deity_at_home.se>
 *      Darel Cullen         <darelcullen@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient_sdl.h"

#include "text.h"
#include "console.h"
#include "sdlkeys.h"
#include "glwidgets.h"
#include "sdlpaint.h"

/* These are only needed for the polygon tessellation */
/* I'd like to move them to Paint_init/cleanup but because it */
/* is called before the map is ready I need separate functions */
/* for now.. */
extern int Gui_init(void);
extern void Gui_cleanup(void);

int draw_depth;

/* This holds video information assigned at initialise */
const SDL_VideoInfo *videoInfo;

/* Flags to pass to SDL_SetVideoMode */
int videoFlags;
SDL_Surface  *MainSDLSurface = NULL;

font_data gamefont;
font_data mapfont;
int gameFontSize;
int mapFontSize;

/* ugly kps hack */
bool file_exists(const char *path) 
{ 
  FILE *fp;

  if (!path) {
    return false; 
  } else {
    fp = fopen(path ? path : "", "r");
    if (fp) { 
      fclose(fp); 
      return true;
    }
    return false; 
  }
}

int Init_playing_windows(void)
{
    /*
    sdl_init_colors();
    Init_spark_colors();
    */
    if ( !AppendGLWidgetList(&MainWidget,Init_MainWidget(&gamefont)) ) {
	error("widget initialization failed");
	return -1;
    }
    if (Console_init()) {
	error("console initialization failed");
	return -1;
    }
    if (Gui_init()) {
	error("gui initialization failed");
	return -1;
    }

    return 0;
}


int Init_window(void)
{
    char gamefontname[] = CONF_FONTDIR "Test.ttf";
    int value;

    if (TTF_Init()) {
    	error("SDL_ttf initialization failed: %s", SDL_GetError());
    	return -1;
    }
    xpprintf("SDL_ttf initialized\n");

    Conf_print();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    /* Fetch the video info */
    videoInfo = SDL_GetVideoInfo( );

    num_spark_colors=8;

    /* the flags to pass to SDL_SetVideoMode */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL          */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering       */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
#ifndef _WINDOWS
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing        */
#else
	videoFlags |= SDL_FULLSCREEN;
#endif

    /** This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
        videoFlags |= SDL_HWSURFACE;
    else
        videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
        videoFlags |= SDL_HWACCEL;

    draw_depth =  videoInfo->vfmt->BitsPerPixel;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if ((MainSDLSurface = SDL_SetVideoMode(draw_width,
			 draw_height,
			 draw_depth,
			 videoFlags )) == NULL) {
      error("Could not find a valid GLX visual for your display");
    }

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    printf("RGB bpp %d/", value);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,&value);
    printf("%d/", value);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    printf("%d ", value);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    printf("Bit Depth is %d\n",value);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, draw_width, draw_height);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, draw_width, 0, draw_height);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Set title for window */
    SDL_WM_SetCaption(TITLE, NULL);
    
    /* this prevents a freetype crash if you pass non existant fonts */
    if (!file_exists(gamefontname)) {
      error("cannot find your game font '%s'.\n" \
            "Please check you have run make install, if you have:\n" \
            "ensure that your fonts and texures are located in\n" \
	    "%s\n",gamefontname,CONF_FONTDIR);
      return -1;
    }
      
    if (fontinit(&gamefont,gamefontname,gameFontSize)) {
    	error("Font initialization failed with %s", gamefontname);
	return -1;
    }
    if (fontinit(&mapfont,gamefontname,mapFontSize)) {
    	error("Font initialization failed with %s", gamefontname);
	return -1;
    }

    return 0;
}

void Quit(void)
{
    Close_Widget(&MainWidget);
    Gui_cleanup();
    Console_cleanup();
    fontclean(&gamefont);
    fontclean(&mapfont);
    TTF_Quit();
    SDL_Quit();
}

static bool Set_geometry(xp_option_t *opt, const char *s)
{
    int w = 0, h = 0;

    if (s[0] == '=') {
	sscanf(s, "%*c%d%*c%d", &w, &h);
    } else {
	sscanf(s, "%d%*c%d", &w, &h);
    }
    if (w == 0 || h == 0) return false;
    if (MainSDLSurface != NULL) {
	Resize_Window(w, h);
    } else {
	draw_width = w;
	draw_height = h;
    }
    return true;
}

static const char* Get_geometry(xp_option_t *opt)
{
    static char buf[20]; /* should be enough */
    snprintf(buf, 20, "%dx%d", draw_width, draw_height);
    return buf;
}

static xp_option_t sdlinit_options[] = {
    XP_STRING_OPTION(
	"geometry",
	"1280x1024",
	NULL,
	0,
	Set_geometry, NULL, Get_geometry,
	XP_OPTFLAG_DEFAULT,
	"Set the initial window geometry.\n"),
    
     XP_INT_OPTION(
        "gameFontSize",
	16, 12, 32,
	&gameFontSize,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Height of font used for game strings.\n"),

    XP_INT_OPTION(
        "mapFontSize",
	16, 12, 64,
	&mapFontSize,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Height of font used for message strings.\n")

};

void Store_sdlinit_options(void)
{
    STORE_OPTIONS(sdlinit_options);
}

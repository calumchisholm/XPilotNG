/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client. Copyright (C) 2003-2004 by 
 *
 *     Juha Lindström <juhal@users.sourceforge.net>
 *     Erik Andersson <deity_at_home.se>
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

#include "sdlkeys.h"
#include "console.h"
#include "sdlpaint.h"
#include "glwidgets.h"

char sdlevent_version[] = VERSION;

/* TODO: remove these from client.h and put them in *event.h */
bool            initialPointerControl = false;
bool            pointerControl = false;

/* horizontal mouse movement. */
int	mouseMovement;
struct timeval next_time = {0,0};

GLWidget *target[NUM_MOUSE_BUTTONS];
GLWidget *hovertarget = NULL;

int Process_event(SDL_Event *evt);

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

#ifndef HAVE_GETTIMEOFDAY
struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};

__inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;

}
#endif /* HAVE_GETTIMEOFDAY */

bool Key_press_swap_scalefactor(void)
{
    double tmp;
    
    tmp = scaleFactor;
    scaleFactor = scaleFactor_s;
    scaleFactor_s = tmp;

    scale = 1.0 / scaleFactor;
    return false;
}

bool Key_press_talk(void)
{
    Console_show();
    return false;	/* server doesn't need to know */
}

bool Key_press_pointer_control(void)
{
    pointerControl = !pointerControl;
    if (pointerControl) {
    	MainWidget_ShowMenu(MainWidget,false);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(SDL_DISABLE);
    } else {
    	MainWidget_ShowMenu(MainWidget,true);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(SDL_ENABLE);
    }
    return false;	/* server doesn't need to know */
}

bool Key_press_toggle_record(void)
{
    /* TODO: implement if you think it is worth it 
    Record_toggle();
    */
    return false;	/* server doesn't need to know */
}

bool Key_press_toggle_radar_score(void)
{
    /* TODO */
    return false;
}

static bool find_size(int *w, int *h)
{
    extern int videoFlags;
    SDL_Rect **modes;
    int i;

    modes = SDL_ListModes(NULL, videoFlags);
    if (modes == NULL) return false;
    if (modes == (SDL_Rect**)-1) return true;

    if (!modes[1]) {
	*w = modes[0]->w;
	*h = modes[0]->h;
    } else {
	for (i = 1; modes[i]; i++) {
	    if (*w > modes[i]->w) {
		*w = modes[i - 1]->w;
		*h = modes[i - 1]->h;
		break;
	    }
	}
    }

    return true;
}

#ifndef _WINDOWS
bool Key_press_toggle_fullscreen(void)
{
    extern int videoFlags;
    static int initial_w = -1, initial_h = -1;
    int w, h;

    if (initial_w == -1) {
	initial_w = draw_width;
	initial_h = draw_height;
    }

    if (videoFlags & SDL_FULLSCREEN) {
	videoFlags ^= SDL_FULLSCREEN;
	Resize_Window(initial_w, initial_h);
	return false;
    }

    w = initial_w = draw_width;
    h = initial_h = draw_height;

    videoFlags ^= SDL_FULLSCREEN;
    if (find_size(&w, &h)
	&& Resize_Window(w, h) == 0)
	return false;
    
    videoFlags ^= SDL_FULLSCREEN;
    Resize_Window(initial_w, initial_h);
    Add_message("Failed to change video mode [*Client reply*]");
    return false;
}
#else
bool Key_press_toggle_fullscreen(void)
{
	Add_message("Changing mode does not work in windows [*Client reply*]");
	return false;
}
#endif

int Process_event(SDL_Event *evt)
{
    int button;
    static struct timeval now = {0,0};

    if (Console_process(evt)) return 1;
    
    switch (evt->type) {
	
    case SDL_QUIT:
        Net_cleanup(); 
        Quit();
		exit(0);
        break;
	
    case SDL_KEYDOWN:
	if (Console_isVisible()) break;
	Keyboard_button_pressed((xp_keysym_t)evt->key.keysym.sym);
	break;
	
    case SDL_KEYUP:
	if (Console_isVisible()) break;
	Keyboard_button_released((xp_keysym_t)evt->key.keysym.sym);
	break;
	
    case SDL_MOUSEBUTTONDOWN:
	button = evt->button.button;
	if (!pointerControl) {
	    if ( (target[button-1] = FindGLWidget(MainWidget,evt->button.x,evt->button.y)) ) {
	    	if (target[button-1]->button) {
		    target[button-1]->button(button,evt->button.state,
		    	    	    	    evt->button.x,evt->button.y,
					    target[button-1]->buttondata);
		}
	    }
	    
	} else {
	    Pointer_button_pressed(button);
	}
	break;
	
    case SDL_MOUSEMOTION:
	if (pointerControl) {
	    mouseMovement += evt->motion.xrel;
	} else {
	    /*xpprintf("mouse motion xrel=%i yrel=%i\n",evt->motion.xrel,evt->motion.yrel);*/
	    /*for (i = 0;i<NUM_MOUSE_BUTTONS;++i)*/ /* dragdrop for all mouse buttons*/
	    if (target[0]) { /*is button one pressed?*/
	    	/*xpprintf("SDL_MOUSEBUTTONDOWN drag: area found!\n");*/
	    	if (target[0]->motion) {
		    target[0]->motion(evt->motion.xrel,evt->motion.yrel,
		    	    	    	evt->button.x,evt->button.y,
					target[0]->motiondata);
		}
	    } else {
    	    	GLWidget *tmp = FindGLWidget(MainWidget,evt->button.x,evt->button.y);
		if (tmp != hovertarget) {
		    if (tmp && tmp->hover)
    	    	    	tmp->hover(true,evt->button.x,evt->button.y,tmp->hoverdata);
    	    	    if (hovertarget && hovertarget->hover) {
		    	hovertarget->hover(false,evt->button.x,evt->button.y,hovertarget->hoverdata);
		    }
		    hovertarget = tmp;
		}
	    }
	}
	break;
	
    case SDL_MOUSEBUTTONUP:
	button = evt->button.button;
	if (pointerControl) {
	    Pointer_button_released(button);
	} else {
	    if ( target[button-1] ) {
	    	if (target[button-1]->button) {
		    target[button-1]->button(button,evt->button.state,
		    	    	    	    	evt->button.x,evt->button.y,
						target[button-1]->buttondata);
		}
		target[button-1] = NULL;
	    }
	}
	break;

    case SDL_VIDEORESIZE:     
        Resize_Window(evt->resize.w, evt->resize.h);          
        break;

    default:
      break;
    }
    
    if (mouseMovement) {
    	gettimeofday(&now,NULL);
    	if (!movement_interval || (now.tv_sec > next_time.tv_sec) || (now.tv_usec > next_time.tv_usec)) {
	    next_time.tv_sec = now.tv_sec;
	    next_time.tv_usec = now.tv_usec + movement_interval;
	    while ( next_time.tv_usec > 1000000 ) {
	    	++next_time.tv_sec;
		next_time.tv_usec -= 1000000;
	    }
	    Send_pointer_move(mouseMovement);
	    Net_flush();
    	    mouseMovement = 0;
	}
    }
    return 1;
}

/* kps - just here so that this can link to generic client files */
void Config_redraw(void)
{

}

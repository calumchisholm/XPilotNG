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

#include "sdlinit.h"
#include "sdlkeys.h"
#include "console.h"
#include "sdlpaint.h"
#include "glwidgets.h"

char sdlevent_version[] = VERSION;

/* TODO: remove these from client.h and put them in *event.h */
bool            initialPointerControl = false;
bool            pointerControl = false;

static int	mouseMovement;	/* horizontal mouse movement. */

GLWidget *clicktarget[NUM_MOUSE_BUTTONS];
GLWidget *hovertarget = NULL;

int Process_event(SDL_Event *evt);

void Swap_scalefactor(void)
{
    double tmp;
    
    tmp = scaleFactor;
    scaleFactor = scaleFactor_s;
    scaleFactor_s = tmp;

    scale = 1.0 / scaleFactor;
}

bool Key_press_talk(void)
{
    Console_show();
    return false;	/* server doesn't need to know */
}

void Pointer_control_set_state(bool on)
{
    if (pointerControl == on)
	return;

    if (on) {
    	MainWidget_ShowMenu(MainWidget, false);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(SDL_DISABLE);
    } else {
    	MainWidget_ShowMenu(MainWidget, true);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(SDL_ENABLE);
    }
    
#ifdef HAVE_XF86MISC
    {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) > 0)
	    Disable_emulate3buttons(on, info.info.x11.display);
    }
#endif

    pointerControl = on;
}

void Record_toggle(void)
{
    /* TODO: implement if you think it is worth it */
    Add_message("Can't record with this client. [*Client reply*]");
}

void Toggle_radar_and_scorelist(void)
{
    /* TODO */
    return;
}

#ifndef _WINDOWS
extern int videoFlags;
void Toggle_fullscreen(void)
{
    static int initial_w = -1, initial_h = -1;
    int w, h;

    if (initial_w == -1) {
	initial_w = draw_width;
	initial_h = draw_height;
    }

    if (videoFlags & SDL_FULLSCREEN) {
	videoFlags ^= SDL_FULLSCREEN;
	Resize_Window(initial_w, initial_h);
	return;
    }

    w = initial_w = draw_width;
    h = initial_h = draw_height;

    videoFlags ^= SDL_FULLSCREEN;
    if (Resize_Window(w, h) == 0)
	return;

    videoFlags ^= SDL_FULLSCREEN;
    Resize_Window(initial_w, initial_h);
    Add_message("Failed to change video mode. [*Client reply*]");
}
#else
void Toggle_fullscreen(void)
{
    Add_message("Changing mode does not work in Windows. [*Client reply*]");
}
#endif

int Process_event(SDL_Event *evt)
{
    int button;

    mouseMovement = 0;

    if (Console_process(evt)) return 1;
    
    switch (evt->type) {
	
    case SDL_QUIT:
	Client_exit(0);
	break;
	
    case SDL_KEYDOWN:
	if (Console_isVisible()) break;
	Keyboard_button_pressed((xp_keysym_t)evt->key.keysym.sym);
	break;
	
    case SDL_KEYUP:
        /* letting release events through to prevent some keys from locking */
	/*if (Console_isVisible()) break;*/
	Keyboard_button_released((xp_keysym_t)evt->key.keysym.sym);
	break;
	
    case SDL_MOUSEBUTTONDOWN:
	button = evt->button.button;
	if (!pointerControl) {
	    if ( (clicktarget[button-1] = FindGLWidget(MainWidget,evt->button.x,evt->button.y)) ) {
	    	if (clicktarget[button-1]->button) {
		    clicktarget[button-1]->button(button,evt->button.state,
		    	    	    	    evt->button.x,evt->button.y,
					    clicktarget[button-1]->buttondata);
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
	    if (clicktarget[0]) { /*is button one pressed?*/
	    	/*xpprintf("SDL_MOUSEBUTTONDOWN drag: area found!\n");*/
	    	if (clicktarget[0]->motion) {
		    clicktarget[0]->motion(evt->motion.xrel,evt->motion.yrel,
		    	    	    	evt->button.x,evt->button.y,
					clicktarget[0]->motiondata);
		}
	    } else {
    	    	GLWidget *tmp = FindGLWidget(MainWidget,evt->button.x,evt->button.y);
		if (tmp != hovertarget) {
    	    	    if (hovertarget && hovertarget->hover) {
		    	hovertarget->hover(false,evt->button.x,evt->button.y,hovertarget->hoverdata);
		    }
		    tmp = FindGLWidget(MainWidget,evt->button.x,evt->button.y);
		    if (tmp && tmp->hover)
    	    	    	tmp->hover(true,evt->button.x,evt->button.y,tmp->hoverdata);
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
	    if ( clicktarget[button-1] ) {
	    	if (clicktarget[button-1]->button) {
		    clicktarget[button-1]->button(button,evt->button.state,
		    	    	    	    	evt->button.x,evt->button.y,
						clicktarget[button-1]->buttondata);
		}
		clicktarget[button-1] = NULL;
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
	Client_pointer_move(mouseMovement);
	Net_flush();
    }
    return 1;
}

/* kps - just here so that this can link to generic client files */
void Config_redraw(void)
{

}

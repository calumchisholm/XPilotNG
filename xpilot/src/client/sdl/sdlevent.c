/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xpclient.h"
#include "sdlkeys.h"
#include "SDL.h"
#include "console.h"
#include "sdlpaint.h"
#include "glwidgets.h"

char sdlevent_version[] = VERSION;

/* TODO: remove these from client.h and put them in *event.h */
bool            initialPointerControl = false;
bool            pointerControl = false;

static int	movement;	/* horizontal mouse movement. */
static GLWidget *target[NUM_MOUSE_BUTTONS];
static GLWidget *hovertarget;

int Process_event(SDL_Event *evt);

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
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(SDL_DISABLE);
    } else {
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
    movement = 0;
    
    if (Console_process(evt)) return 1;
    
    switch (evt->type) {
	
    case SDL_QUIT:
	return 0;
	
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
	    if ( (target[button-1] = FindGLWidget(evt->button.x,evt->button.y)) ) {
	    	if (target[button-1]->button) {
		    target[button-1]->button(button,evt->button.state,
		    	    	    	    evt->button.x,evt->button.y,
					    target[button-1]->buttondata);
#ifdef test
    	    	    xpprintf("target[button-1]->buttondata:%i->%i\n",(int)target[button-1],(int)target[button-1]->buttondata);
#endif
		}
	    }
	    
	} else {
	    Pointer_button_pressed(button);
	}
	break;
	
    case SDL_MOUSEMOTION:
	if (pointerControl)
	    movement += evt->motion.xrel;
	else {
	    /*xpprintf("mouse motion xrel=%i yrel=%i\n",evt->motion.xrel,evt->motion.yrel);*/
	    /*for (i = 0;i<NUM_MOUSE_BUTTONS;++i)*/ /* dragdrop for all mouse buttons*/
	    if (target[0]) { /*is button one pressed?*/
	    	/*xpprintf("SDL_MOUSEBUTTONDOWN drag: area found!\n");*/
	    	if (target[0]->motion) {
		    target[0]->motion(evt->motion.xrel,evt->motion.yrel,
		    	    	    	evt->button.x,evt->button.y,
					target[0]->motiondata);
#ifdef test
    	    	    xpprintf("target[0]->motiondata:%i->%i\n",(int)target[0],(int)target[0]->motiondata);
#endif
		}
	    } else {
    	    	GLWidget *tmp = FindGLWidget(evt->button.x,evt->button.y);
		if (tmp != hovertarget) {
		    if (tmp && tmp->hover)
    	    	    	tmp->hover(true,evt->button.x,evt->button.y,tmp->hoverdata);
    	    	    if (hovertarget && hovertarget->hover) {
		    	hovertarget->hover(false,evt->button.x,evt->button.y,hovertarget->hoverdata);
#ifdef test
		    	xpprintf("hovertarget->hoverdata:%i->%i\n",(int)hovertarget,(int)hovertarget->hoverdata);
#endif
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
#ifdef test
    	    	    xpprintf("target[button-1]->buttondata:%i->%i\n",(int)target[button-1],(int)target[button-1]->buttondata);
#endif
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
    
    if (movement) {
	Send_pointer_move(movement);
	Net_flush();
    }
    return 1;
}

/* kps - just here so that this can link to generic client files */
void Config_redraw(void)
{

}

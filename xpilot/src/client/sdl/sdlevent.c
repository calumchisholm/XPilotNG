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

char sdlevent_version[] = VERSION;

/* TODO: remove these from client.h and put them in *event.h */
bool            initialPointerControl = false;
bool            pointerControl = false;

static int	movement;	/* horizontal mouse movement. */
static guiarea_t *target[NUM_MOUSE_BUTTONS];

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

int Process_event(SDL_Event *evt)
{
    int key_change = 0;
    movement = 0;
    keylist *temp;
    int button,i=0;
    
    if (Console_process(evt)) return 1;
    
    switch (evt->type) {
	
    case SDL_QUIT:
	return 0;
	
    case SDL_KEYDOWN:
	if (Console_isVisible()) break;
    	temp = keyMap[evt->key.keysym.sym];
    	while (temp) {
    	    key_change |= Key_press(temp->key);
	    temp = (keylist *)temp->next;
    	}
	break;
	
    case SDL_KEYUP:
	if (Console_isVisible()) break;
    	temp = keyMap[evt->key.keysym.sym];
    	while (temp) {
    	    key_change |= Key_release(temp->key);
	    temp = (keylist *)temp->next;
    	}
	break;
	
    case SDL_MOUSEBUTTONDOWN:
	if (!pointerControl) {
	    button = evt->button.button;
	    if ( (target[button-1] = find_guiarea(evt->button.x,evt->button.y)) ) {
	    	if (target[button-1]->button) {
		    target[button-1]->button(button,evt->button.state,evt->button.x,evt->button.y);
		}
	    }
	    
	} else {
    	    temp = buttonMap[evt->button.button - 1];
    	    while (temp) {
    	    	key_change |= Key_press(temp->key);
	    	temp = (keylist *)temp->next;
    	    }
	}
	break;
	
    case SDL_MOUSEMOTION:
	if (pointerControl)
	    movement += evt->motion.xrel;
	else {
	    /*xpprintf("mouse motion xrel=%i yrel=%i\n",evt->motion.xrel,evt->motion.yrel);*/
	    /*for (i = 0;i<NUM_MOUSE_BUTTONS;++i)*/ /* dragdrop for all mouse buttons*/
	    if (target[i]) { /*is button one pressed?*/
	    	/*xpprintf("SDL_MOUSEBUTTONDOWN drag: area found!\n");*/
	    	if (target[i]->motion)
		    target[i]->motion(evt->motion.xrel,evt->motion.yrel,evt->button.x,evt->button.y);
	    }
	}
	break;
	
    case SDL_MOUSEBUTTONUP:
	if (pointerControl) {
    	    temp = buttonMap[evt->button.button - 1];
    	    while (temp) {
    	    	key_change |= Key_release(temp->key);
    	    	temp = (keylist *)temp->next;
    	    }
	} else {
	    button = evt->button.button;
	    if ( target[button-1] ) {
	    	if (target[button-1]->button) {
		    target[button-1]->button(button,evt->button.state,evt->button.x,evt->button.y);
		    target[button-1] = NULL;
		}
	    }
	}
	break;

    case SDL_VIDEORESIZE:     
        Resize_Window(evt->resize.w, evt->resize.h);          
        break;

    default:
      break;
    }
    
    if (key_change) Net_key_change();
    if (movement) Send_pointer_move(movement);
    if (key_change || movement) Net_flush();
    return 1;
}

/* kps - just here so that this can link to generic client files */
void Config_redraw(void)
{

}

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
#include "keys.h"
#include "SDL.h"
#include "console.h"

char sdlevent_version[] = VERSION;

/* TODO: remove these from client.h and put them in *event.h */
bool            initialPointerControl = false;
bool            pointerControl = false;

keys_t          keyMap[SDLK_LAST];   /* maps SDLKeys to keys_t */
keys_t          buttonMap[5];        /* maps mouse buttons to keys_t */

static int	movement;	/* horizontal mouse movement. */





bool Key_press_swap_scalefactor(void)
{
    /* TODO */
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

    if (Console_process(evt)) return 1;
    
    switch (evt->type) {
	
    case SDL_QUIT:
	return 0;
	
    case SDL_KEYDOWN:
	if (Console_isVisible()) break;
	key_change |= Key_press(keyMap[evt->key.keysym.sym]);
	break;
	
    case SDL_KEYUP:
	if (Console_isVisible()) break;
	key_change |= Key_release(keyMap[evt->key.keysym.sym]);
	break;
	
    case SDL_MOUSEBUTTONDOWN:
	if (!pointerControl) {
	    break;
	} else {
	    key_change |= Key_press(buttonMap[evt->button.button - 1]);
	    break;
	}
	
    case SDL_MOUSEMOTION:
	if (!pointerControl) break;
	movement += evt->motion.xrel;
	break;
	
    case SDL_MOUSEBUTTONUP:
	if (!pointerControl) break;
	key_change |= Key_release(buttonMap[evt->button.button - 1]);
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

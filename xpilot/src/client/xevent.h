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

#ifndef XEVENT_H
#define XEVENT_H

typedef struct {
    KeySym	keysym;			/* Keysym-to-action array */
    keys_t	key;
} keydefs_t;

extern keydefs_t	*keyDefs;

extern char* Get_keyHelpString(keys_t key);
extern const char *Get_keyResourceString(keys_t key);

keys_t Lookup_key(XEvent *event, KeySym ks, bool reset);
void Key_event(XEvent *event);
void Talk_event(XEvent *event);
void xevent_keyboard(int queued);
void xevent_pointer(void);
int x_event(int new_input);

#ifdef _WINDOWS
int win_xevent(XEvent event);
#endif

#endif

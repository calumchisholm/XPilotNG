/*
* Map Xpress, the XPilot Map Editor for Windows 95/98/NT.  Copyright (C) 1999, 2000 by
*
*      Jarrod L. Miller           <jlmiller@ctitech.com>
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
*
* See the file COPYRIGHT.TXT for current copyright information.
*
*/
#ifdef _WINDOWS
#define COLOR_SELECT		RGB(0, 255, 255)
#define COLOR_HIDDEN		RGB(102, 102, 102)

#define COLOR_WALL			RGB(0,0,255)
#define COLOR_DECOR			RGB(255,128,0)
#define COLOR_FUEL			RGB(255,0,0)

#define COLOR_BALL			RGB(0, 255, 0)//RGB(255,255,255)
#define COLOR_BALLAREA		RGB(255,0,0)
#define COLOR_BALLTARGET	RGB(255,0,255)
#define COLOR_TARGET		RGB(255,0,0)
#define COLOR_ITEM_CONC		RGB(255,0,0)

#define COLOR_GRAVITY		RGB(0,255,0)
#define COLOR_CURRENT		RGB(0,255,0)
#define COLOR_WORMHOLE		RGB(0,255,0)

#define COLOR_BASE			RGB(255,255,255)
#define COLOR_CANNON		RGB(255,255,255)

//#define COLOR_CHECKPOINT    RGB(102,102,102)
#define COLOR_CHECKPOINT    RGB(255,255,255)

#define COLOR_CANVAS		RGB(255, 255, 255)
#define COLOR_SHIPVIEW		RGB(0, 0, 255)
#define COLOR_SHIPVIEW2		RGB(0, 255, 255)
#define COLOR_SHIPSMALL		RGB(255, 255, 255)
#define COLOR_REFERENCE		RGB(0, 128, 0)
#define COLOR_GRID			RGB(102, 102, 102)
#define COLOR_VERTEX		RGB(255, 0, 0)
#define COLOR_ENGINE		RGB(0, 0, 0)
#define COLOR_GUN			RGB(255, 0, 0)
#define COLOR_MISSLERACK	RGB(0, 0, 0)
#define COLOR_LEFTLIGHT		RGB(255, 0, 0)
#define COLOR_RIGHTLIGHT	RGB(0, 0, 255)
#else
#define COLOR_BACKGROUND "black"
#define COLOR_FOREGROUND  "gray"
#define COLOR_HIGHLIGHT  "white"
#define COLOR_SHADOW     "black"

#define COLOR_WALL      "blue"
#define COLOR_DECOR     "green"
#define COLOR_FUEL      "red"
#define COLOR_TREASURE  "white"
#define COLOR_TARGET    "red"
#define COLOR_ITEM_CONC "red"

#define COLOR_GRAVITY   "red"
#define COLOR_CURRENT   "red"
#define COLOR_WORMHOLE  "red"

#define COLOR_BASE      "white"
#define COLOR_CANNON    "white"
#endif

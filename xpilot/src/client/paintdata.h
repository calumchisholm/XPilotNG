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

#ifndef PAINTTYPES_H
#define PAINTTYPES_H

/* need u_byte */
#ifndef	TYPES_H
#include "types.h"
#endif

/* need MAX_COLORS */
#ifdef _WINDOWS
#define MAX_COLORS  16   /* Max. switched colors ever */
#define MAX_COLOR_LEN 32 /* Max. length of a color name */
#endif

extern unsigned long	current_foreground;

static inline void RESET_FG(void)
{
    current_foreground = -1;
}

static inline void SET_FG(unsigned long fg)
{
    if (fg != current_foreground)
	XSetForeground(dpy, gameGC, current_foreground = fg);
}

extern XRectangle	*rect_ptr[MAX_COLORS];
extern int		num_rect[MAX_COLORS], max_rect[MAX_COLORS];
extern XArc		*arc_ptr[MAX_COLORS];
extern int		num_arc[MAX_COLORS], max_arc[MAX_COLORS];
extern XSegment		*seg_ptr[MAX_COLORS];
extern int		num_seg[MAX_COLORS], max_seg[MAX_COLORS];

extern void Rectangle_start(void);
extern void Rectangle_end(void);
extern int Rectangle_add(int color, int x, int y, int width, int height);
extern void Arc_start(void);
extern void Arc_end(void);
extern int Arc_add(int color,
		   int x, int y,
		   int width, int height,
		   int angle1, int angle2);
extern void Segment_start(void);
extern void Segment_end(void);
extern int Segment_add(int color, int x_1, int y_1, int x_2, int y_2);

#endif

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

#define RESET_FG()	(current_foreground = -1)
#define SET_FG(PIXEL)				\
    if ((PIXEL) == current_foreground) ;	\
    else XSetForeground(dpy, gameGC, current_foreground = (PIXEL))

extern unsigned long	current_foreground;


#define ERASE_INITIALIZED	(1 << 0)

#define MAX_LINE_WIDTH	10

typedef struct {
    int			flags;
    XRectangle		*rect_ptr;
    int			num_rect,
			max_rect;
    XArc		*arc_ptr;
    int			num_arc,
			max_arc;
    XSegment		*seg_ptr[MAX_LINE_WIDTH + 1];
    int			num_seg[MAX_LINE_WIDTH + 1],
			max_seg[MAX_LINE_WIDTH + 1];
} erase_t;

extern erase_t		erase[2],
			*erp;

extern XRectangle	*rect_ptr[MAX_COLORS];
extern int		num_rect[MAX_COLORS], max_rect[MAX_COLORS];
extern XArc		*arc_ptr[MAX_COLORS];
extern int		num_arc[MAX_COLORS], max_arc[MAX_COLORS];
extern XSegment		*seg_ptr[MAX_COLORS];
extern int		num_seg[MAX_COLORS], max_seg[MAX_COLORS];

extern void Erase_do_start(void);
extern void Erase_do_end(void);
extern void Erase_do_rectangle(int x, int y, int width, int height);
extern void Erase_do_rectangles(XRectangle *rectp, int n);
extern void Erase_do_arc(int x, int y, int width, int height,
                      int angle1, int angle2);
extern void Erase_do_arcs(XArc *arcp, int n);
extern void Erase_do_segment(int width, int x_1, int y_1, int x_2, int y_2);
extern void Erase_do_segments(XSegment *segp, int n);
extern void Erase_do_points(int width, XPoint *pointp, int n);
extern void Erase_do_4point(int x, int y, int width, int height);

#define Erase_start() \
	    ((useErase) ? Erase_do_start() : (void)0 )
#define Erase_end() \
	    ((useErase) ? Erase_do_end() : (void)0 )
#define Erase_rectangle( _A, _B, _C, _D ) \
	    ((useErase) ? Erase_do_rectangle((_A), (_B), (_C), (_D)) : (void)0 )
#define Erase_rectangles(_A, _B) \
	    ((useErase) ? Erase_do_rectangles((_A), (_B)) : (void)0 )
#define Erase_arc(_A, _B, _C, _D, _E, _F) \
	    ((useErase) ? Erase_do_arc((_A), (_B), (_C), (_D), (_E), (_F)):(void)0 )
#define Erase_arcs(_A, _B) \
	    ((useErase) ? Erase_do_arcs((_A), (_B)) : (void)0 )
#define Erase_segment(_A, _B, _C, _D, _E) \
	    ((useErase) ? Erase_do_segment((_A), (_B), (_C), (_D), (_E)) : (void)0 )
#define Erase_segments(_A, _B) \
	    ((useErase) ? Erase_do_segments((_A), (_B)) : (void)0 )
#define Erase_points(_A, _B, _C) \
	    ((useErase) ? Erase_do_points((_A), (_B), (_C)) : (void)0 )
#define Erase_4point(_A, _B, _C, _D) \
	    ((useErase) ? Erase_do_4point((_A), (_B), (_C), (_D)) : (void)0 )

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

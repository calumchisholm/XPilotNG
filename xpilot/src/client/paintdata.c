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

#include "xpclient_x11.h"

char paintdata_version[] = VERSION;

XRectangle	*rect_ptr[MAX_COLORS];
int		num_rect[MAX_COLORS], max_rect[MAX_COLORS];
XArc		*arc_ptr[MAX_COLORS];
int		num_arc[MAX_COLORS], max_arc[MAX_COLORS];
XSegment	*seg_ptr[MAX_COLORS];
int		num_seg[MAX_COLORS], max_seg[MAX_COLORS];

unsigned long	current_foreground;

void Rectangle_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++)
	num_rect[i] = 0;
}

void Rectangle_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_rect[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.fillRectangles(dpy, drawPixmap, gameGC,
			      rect_ptr[i], num_rect[i]);
	    RELEASE(rect_ptr[i], num_rect[i], max_rect[i]);
	}
    }
}

int Rectangle_add(int color, int x, int y, int width, int height)
{
    XRectangle		t;

    t.x = WINSCALE(x);
    t.y = WINSCALE(y);
    t.width = WINSCALE(width);
    t.height = WINSCALE(height);

    STORE(XRectangle, rect_ptr[color], num_rect[color], max_rect[color], t);
    return 0;
}

void Arc_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++)
	num_arc[i] = 0;
}

void Arc_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_arc[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.drawArcs(dpy, drawPixmap, gameGC, arc_ptr[i], num_arc[i]);
	    RELEASE(arc_ptr[i], num_arc[i], max_arc[i]);
	}
    }
}

int Arc_add(int color,
	    int x, int y,
	    int width, int height,
	    int angle1, int angle2)
{
    XArc t;

    t.x = WINSCALE(x);
    t.y = WINSCALE(y);
    t.width = WINSCALE(width+x) - t.x;
    t.height = WINSCALE(height+y) - t.y;

    t.angle1 = angle1;
    t.angle2 = angle2;
    STORE(XArc, arc_ptr[color], num_arc[color], max_arc[color], t);
    return 0;
}

void Segment_start(void)
{
    int i;

    for (i = 0; i < maxColors; i++)
	num_seg[i] = 0;
}

void Segment_end(void)
{
    int i;

    for (i = 0; i < maxColors; i++) {
	if (num_seg[i] > 0) {
	    SET_FG(colors[i].pixel);
	    rd.drawSegments(dpy, drawPixmap, gameGC,
			    seg_ptr[i], num_seg[i]);
	    RELEASE(seg_ptr[i], num_seg[i], max_seg[i]);
	}
    }
}

int Segment_add(int color, int x_1, int y_1, int x_2, int y_2)
{
    XSegment t;

    t.x1 = WINSCALE(x_1);
    t.y1 = WINSCALE(y_1);
    t.x2 = WINSCALE(x_2);
    t.y2 = WINSCALE(y_2);
    STORE(XSegment, seg_ptr[color], num_seg[color], max_seg[color], t);
    return 0;
}

void paintdataCleanup(void)
{
    int i;

    for (i = 0; i < MAX_COLORS; i++) {
	if (max_rect[i] > 0 && rect_ptr[i]) {
	    max_rect[i] = 0;
	    free(rect_ptr[i]);
	}
	if (max_arc[i] > 0 && arc_ptr[i]) {
	    max_arc[i] = 0;
	    free(arc_ptr[i]);
	}
	if (max_seg[i] > 0 && seg_ptr[i]) {
	    max_seg[i] = 0;
	    free(seg_ptr[i]);
	}
    }
}

#define SCALE_ARRAY_SIZE	32768
short	scaleArray[SCALE_ARRAY_SIZE];

void Init_scale_array(void)
{
    int		i, start, end, n;
    double	scaleMultFactor;

    if (scaleFactor == 0.0)
	scaleFactor = 1.0;
    if (scaleFactor < 0.1)
	scaleFactor = 0.1;
    if (scaleFactor > 10.0)
	scaleFactor = 10.0;
    scaleMultFactor = 1.0 / scaleFactor;

    scaleArray[0] = 0;

    for (i = 1; i < NELEM(scaleArray); i++) {
	n = floor(i * scaleMultFactor + 0.5);
	if (n == 0)
	    /* keep values for non-zero indices at least 1. */
	    scaleArray[i] = 1;
	else
	    break;
    }
    start = i;

    for (i = NELEM(scaleArray) - 1; i >= 0; i--) {
	n = floor(i * scaleMultFactor + 0.5);
	if (n > 32767)
	    /* keep values lower or equal to max short. */
	    scaleArray[i] = 32767;
	else
	    break;
    }
    end = i;

    for (i = start; i <= end; i++)
	scaleArray[i] = floor(i * scaleMultFactor + 0.5);

    /*
     * verify correct calculations, because of reported gcc optimization
     * bugs.
     */
    for (i = 1; i < NELEM(scaleArray); i++) {
	if (scaleArray[i] < 1)
	    break;
    }

    if (i != SCALE_ARRAY_SIZE) {
	fprintf(stderr,
		"Error: Illegal value %d in scaleArray[%d].\n"
		"\tThis error may be due to a bug in your compiler.\n"
		"\tEither try a lower optimization level,\n"
		"\tor a different compiler version or vendor.\n",
		scaleArray[i], i);
	exit(1);
    }
}

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

#ifndef CLICK_H
#define CLICK_H

#ifndef DRAW_H
# include "draw.h"
#endif

/*
 * The wall collision detection routines depend on repeatability
 * (getting the same result even after some "neutral" calculations)
 * and an exact determination whether a point is in space,
 * inside the wall (crash!) or on the edge.
 * This will be hard to achieve if only floating point would be used.
 * However, a resolution of a pixel is a bit rough and ugly.
 * Therefore a fixed point sub-pixel resolution is used called clicks.
 */

/* calculate the click coordinate of the center of a block */
#define BLOCK_CENTER(B)		((int)((B) * BLOCK_CLICKS) + BLOCK_CLICKS / 2)

/*
 * Two macros for edge wrap of x and y coordinates measured in clicks.
 * Note that the correction needed should never be bigger than the size of the map.
 */
#define WRAP_XCLICK(world_, x_)	\
	(BIT((world_)->rules->mode, WRAP_PLAY) \
	    ? ((x_) < 0 \
		? (x_) + (world_)->cwidth \
		: ((x_) >= (world_)->cwidth \
		    ? (x_) - (world_)->cwidth \
		    : (x_))) \
	    : (x_))

#define WRAP_YCLICK(world_, y_)	\
	(BIT((world_)->rules->mode, WRAP_PLAY) \
	    ? ((y_) < 0 \
		? (y_) + (world_)->cheight \
		: ((y_) >= (world_)->cheight \
		    ? (y_) - (world_)->cheight \
		    : (y_))) \
	    : (y_))

/*
 * Two macros for edge wrap of differences in position.
 * If the absolute value of a difference is bigger than
 * half the map size then it is wrapped.
 */
#define WRAP_DCX(world_, dcx)	\
	(BIT((world_)->rules->mode, WRAP_PLAY) \
	    ? ((dcx) < - ((world_)->cwidth >> 1) \
		? (dcx) + (world_)->cwidth \
		: ((dcx) > ((world_)->cwidth >> 1) \
		    ? (dcx) - (world_)->cwidth \
		    : (dcx))) \
	    : (dcx))

#define WRAP_DCY(world_, dcy)	\
	(BIT((world_)->rules->mode, WRAP_PLAY) \
	    ? ((dcy) < - ((world_)->cheight >> 1) \
		? (dcy) + (world_)->cheight \
		: ((dcy) > ((world_)->cheight >> 1) \
		    ? (dcy) - (world_)->cheight \
		    : (dcy))) \
	    : (dcy))

#define TWRAP_XCLICK(world_, x_) \
     ((x_) > 0 ? (x_) % (world_)->cwidth : \
      ((x_) % (world_)->cwidth + (world_)->cwidth))

#define TWRAP_YCLICK(world_, y_) \
     ((y_) > 0 ? (y_) % (world_)->cheight : \
      ((y_) % (world_)->cheight + (world_)->cheight))


#define CENTER_XCLICK(world_, X) \
        (((X) < -((world_)->cwidth >> 1)) ? \
             (X) + (world_)->cwidth : \
             (((X) >= ((world_)->cwidth >> 1)) ? \
                 (X) - (world_)->cwidth : \
                 (X)))

#define CENTER_YCLICK(world_, X) \
        (((X) < -((world_)->cheight >> 1)) ? \
	     (X) + (world_)->cheight : \
	     (((X) >= ((world_)->cheight >> 1)) ? \
	         (X) - (world_)->cheight : \
	         (X)))

#if 0 /* kps -moved to common/draw.h because shipshapes needs this */
typedef int click_t;

typedef struct {
    click_t		cx, cy;
} clpos;
#endif

typedef struct {
    click_t		cx, cy;
} clvec;

/*
 * Return the block position this click position is in.
 */
static inline blpos Clpos_to_blpos(clpos pos)
{
    blpos bpos;

    bpos.bx = CLICK_TO_BLOCK(pos.cx);
    bpos.by = CLICK_TO_BLOCK(pos.cy);

    return bpos;
}

#endif

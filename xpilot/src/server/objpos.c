/* 
 *
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
# include <windows.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "object.h"
#include "objpos.h"
#include "walls.h"

char objpos_version[] = VERSION;


void Object_position_set_clicks(object *obj, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&obj->pos;

    if (cx < 0 || cx >= World.cwidth || cy < 0 || cy >= World.cheight) {
	if (0) {
	    printf("BUG!  Illegal object position %d,%d\n", cx, cy);
	    printf("      Type = %d (%s)\n", obj->type, Object_typename(obj));
	    *(double *)(-1) = 4321.0;
	    abort();
	} else {
	    struct move mv;

	    Object_crash(obj, &mv, CrashUnknown, -1);
	    return;
	}
    }

    pos->cx = cx;
    pos->px = CLICK_TO_PIXEL(cx);
    pos->bx = CLICK_TO_BLOCK(cx);
    pos->cy = cy;
    pos->py = CLICK_TO_PIXEL(cy);
    pos->by = CLICK_TO_BLOCK(cy);
}

void Object_position_init_clicks(object *obj, int cx, int cy)
{
    Object_position_set_clicks(obj, cx, cy);
    Object_position_remember(obj);
    obj->collmode = 0;
}

void Player_position_restore(player *pl)
{
    Player_position_set_clicks(pl, pl->prevpos.cx, pl->prevpos.cy);
}

void Player_position_set_clicks(player *pl, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&pl->pos;

    if (cx < 0 || cx >= World.cwidth || cy < 0 || cy >= World.cheight) {
	if (0) {
	    printf("BUG!  Illegal player position %d,%d\n", cx, cy);
	    *(double *)(-1) = 4321.0;
	    abort();
	} else {
	    struct move mv;

	    Player_crash(pl, &mv, CrashUnknown, -1, 1);
	    return;
	}
    }

    pos->cx = cx;
    pos->px = CLICK_TO_PIXEL(cx);
    pos->bx = CLICK_TO_BLOCK(cx);
    pos->cy = cy;
    pos->py = CLICK_TO_PIXEL(cy);
    pos->by = CLICK_TO_BLOCK(cy);
}

void Player_position_init_clicks(player *pl, int cx, int cy)
{
    Player_position_set_clicks(pl, cx, cy);
    Player_position_remember(pl);
    pl->collmode = 0;
}

void Player_position_limit(player *pl)
{
    int			x = pl->pos.px, ox = x;
    int			y = pl->pos.py, oy = y;

    LIMIT(x, 0, World.width - 1);
    LIMIT(y, 0, World.height - 1);
    if (x != ox || y != oy) {
	Player_position_set_clicks(pl, PIXEL_TO_CLICK(x), PIXEL_TO_CLICK(y));
    }
}

void Player_position_debug(player *pl, const char *msg)
{
#if DEVELOPMENT
    int			i;

    printf("pl %s pos dump: ", pl->name);
    if (msg) printf("(%s)", msg);
    printf("\n");
    printf("\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
	   pl->pos.bx,
	   pl->pos.by,
	   pl->pos.x,
	   pl->pos.y,
	   pl->pos.cx,
	   pl->pos.cy,
	   pl->prevpos.x,
	   pl->prevpos.y);
    for (i = 0; i < pl->ship->num_points; i++) {
	printf("\t%2d\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
		i,
	       (int)((pl->pos.x + pl->ship->pts[i][pl->dir].x) / BLOCK_SZ),
	       (int)((pl->pos.y + pl->ship->pts[i][pl->dir].y) / BLOCK_SZ),
	       (int)(pl->pos.x + pl->ship->pts[i][pl->dir].x),
	       (int)(pl->pos.y + pl->ship->pts[i][pl->dir].y),
	       (int)(pl->pos.cx + FLOAT_TO_CLICK(pl->ship->pts[i][pl->dir].x)),
	       (int)(pl->pos.cy + FLOAT_TO_CLICK(pl->ship->pts[i][pl->dir].y)),
	       (int)(pl->prevpos.x + pl->ship->pts[i][pl->dir].x),
	       (int)(pl->prevpos.y + pl->ship->pts[i][pl->dir].y));
    }
#endif
}


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

#include "xpserver.h"

char objpos_version[] = VERSION;


void Object_position_set_clicks(object *obj, int cx, int cy)
{
    if (!INSIDE_MAP(cx, cy)) {
	if (0) {
	    printf("BUG!  Illegal object position %d,%d\n", cx, cy);
	    printf("      Type = %d (%s)\n", obj->type, Object_typename(obj));
	    *(double *)(-1) = 4321.0;
	    abort();
	} else {
	    Object_crash(obj, CrashUnknown, NO_IND);
	    return;
	}
    }

    obj->pos.cx = cx;
    obj->pos.cy = cy;
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
    if (!INSIDE_MAP(cx, cy)) {
	if (0) {
	    printf("BUG!  Illegal player position %d,%d\n", cx, cy);
	    *(double *)(-1) = 4321.0;
	    abort();
	} else {
	    Player_crash(pl, CrashUnknown, NO_IND, 1);
	    return;
	}
    }

    pl->pos.cx = cx;
    pl->pos.cy = cy;
}

void Player_position_init_clicks(player *pl, int cx, int cy)
{
    Player_position_set_clicks(pl, cx, cy);
    Player_position_remember(pl);
    pl->collmode = 0;
}

void Player_position_limit(player *pl)
{
    int			cx = pl->pos.cx, ox = cx;
    int			cy = pl->pos.cy, oy = cy;

    LIMIT(cx, 0, World.cwidth - 1);
    LIMIT(cy, 0, World.cheight - 1);
    if (cx != ox || cy != oy)
	Player_position_set_clicks(pl, cx, cy);
}

#ifdef DEVELOPMENT
void Player_position_debug(player *pl, const char *msg)
{
    int			i;

    printf("pl %s pos dump: ", pl->name);
    if (msg) printf("(%s)", msg);
    printf("\n");
    printf("\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
	   CLICK_TO_BLOCK(pl->pos.cx),
	   CLICK_TO_BLOCK(pl->pos.cy),
	   CLICK_TO_PIXEL(pl->pos.cx),
	   CLICK_TO_PIXEL(pl->pos.cy),
	   pl->pos.cx,
	   pl->pos.cy,
	   pl->prevpos.cx,
	   pl->prevpos.cy);
    for (i = 0; i < pl->ship->num_points; i++) {
	clpos pts = Ship_get_point_clpos(pl->ship, i, pl->dir);
	clpos pt;

	pt.cx = pl->pos.cx + pts.cx;
	pt.cy = pl->pos.cy + pts.cy;

	printf("\t%2d\tB %d, %d, P %d, %d, C %d, %d, O %d, %d\n",
	       i,
	       CLICK_TO_BLOCK(pt.cx),
	       CLICK_TO_BLOCK(pt.cy),
	       CLICK_TO_PIXEL(pt.cx),
	       CLICK_TO_PIXEL(pt.cy),
	       pt.cx,
	       pt.cy,
	       pl->prevpos.cx + pts.cx,
	       pl->prevpos.cy + pts.cy);
    }
}
#endif

/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef	_WINDOWS
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "object.h"
#include "objpos.h"
#include "click.h"

char objpos_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id$";
#endif

void Object_position_set_clicks(object *obj, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&obj->pos;

    pos->cx = cx;
    pos->bx = pos->cx / BLOCK_CLICKS;
    pos->cy = cy;
    pos->by = pos->cy / BLOCK_CLICKS;
    pos->px = CLICK_TO_PIXEL(cx);
    pos->py = CLICK_TO_PIXEL(cy);
}

void Object_position_init_clicks(object *obj, int x, int y)
{
  Object_position_set_clicks(obj, x, y);
  Object_position_remember(obj);
  obj->collmode = 0;
}

void Player_position_set_clicks(player *pl, int cx, int cy)
{
    struct _objposition		*pos = (struct _objposition *)&pl->pos;

    pos->cx = cx;
    pos->px = CLICK_TO_PIXEL(cx);
    pos->bx = pos->cx / BLOCK_CLICKS;
    pos->cy = cy;
    pos->py = CLICK_TO_PIXEL(cy);
    pos->by = pos->cy / BLOCK_CLICKS;
}

void Player_position_init_clicks(player *pl, int x, int y)
{
    Player_position_set_clicks(pl, x, y);
    Player_position_remember(pl);
    pl->collmode = 0;
}

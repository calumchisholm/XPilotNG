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

char shipshape_c_version[] = VERSION;

extern int	Get_shape_keyword(char *keyw);
extern void	Make_table(void);

void Rotate_point(shapepos pt[RES])
{
    int			i;

    for (i = 1; i < RES; i++) {
	pt[i].pxl.x = (tcos(i) * pt[0].pxl.x - tsin(i) * pt[0].pxl.y) + .5;
	pt[i].pxl.y = (tsin(i) * pt[0].pxl.x + tcos(i) * pt[0].pxl.y) + .5;
    }
}


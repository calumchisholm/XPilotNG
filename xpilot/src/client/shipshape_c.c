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

bool do_sshack = false;

void Rotate_point(shapepos pt[RES])
{
    int			i;

    for (i = 1; i < RES; i++) {
	pt[i].pxl.x = (tcos(i) * pt[0].pxl.x - tsin(i) * pt[0].pxl.y) + .5;
	pt[i].pxl.y = (tsin(i) * pt[0].pxl.x + tcos(i) * pt[0].pxl.y) + .5;
    }
}

/*
 * Return a pointer to a default ship.
 * This function should always succeed,
 * therefore no malloc()ed memory is used.
 */
shipobj *Default_ship(void)
{
    static shipobj	sh;
    static shapepos	pts[6][RES];

    if (!sh.num_points) {
	sh.num_points = 3;
	sh.num_orig_points = 3;
	sh.pts[0] = &pts[0][0];
	sh.pts[0][0].pxl.x = 15;
	sh.pts[0][0].pxl.y = 0;
	sh.pts[1] = &pts[1][0];
	sh.pts[1][0].pxl.x = -9;
	sh.pts[1][0].pxl.y = 8;
	sh.pts[2] = &pts[2][0];
	sh.pts[2][0].pxl.x = -9;
	sh.pts[2][0].pxl.y = -8;

	sh.engine[0].pxl.x = -9;
	sh.engine[0].pxl.y = 0;

	sh.m_gun[0].pxl.x = 15;
	sh.m_gun[0].pxl.y = 0;

	sh.num_l_light = 1;
	sh.l_light[0] = &pts[3][0];
	sh.l_light[0][0].pxl.x = -9;
	sh.l_light[0][0].pxl.y = 8;

	sh.num_r_light = 1;
	sh.r_light[0] = &pts[4][0];
	sh.r_light[0][0].pxl.x = -9;
	sh.r_light[0][0].pxl.y = -8;

	sh.num_m_rack = 1;
	sh.m_rack[0] = &pts[5][0];
	sh.m_rack[0][0].pxl.x = 15;
	sh.m_rack[0][0].pxl.y = 0;

	sh.num_l_gun = sh.num_r_gun = sh.num_l_rgun = sh.num_r_rgun = 0;

	Make_table();

	Rotate_ship(&sh);
    }

    return &sh;
}

void Calculate_shield_radius(shipobj *w)
{
    int			i;
    int			radius2, max_radius = 0;

    for (i = 0; i < w->num_points; i++) {
	position pti = Ship_get_point_position(w, i, 0);
	radius2 = (int)(sqr(pti.x) + sqr(pti.y));
	if (radius2 > max_radius) {
	    max_radius = radius2;
	}
    }
    max_radius = (int)(2.0 * sqrt(max_radius));
    w->shield_radius = (max_radius + 2 <= 34)
			? 34
			: (max_radius + 2 - (max_radius & 1));
}

shapepos ipos2shapepos(ipos pos)
{
    shapepos shpos;

    shpos.pxl.x = pos.x;
    shpos.pxl.y = pos.y;

    return shpos;
}

position Ship_get_point_position(shipobj *ship, int i, int dir)
{
    return ((ship)->pts[i][dir].pxl);
}
position Ship_get_engine_position(shipobj *ship, int dir)
{
    return ((ship)->engine[dir].pxl);
}
position Ship_get_m_gun_position(shipobj *ship, int dir)
{
    return ((ship)->m_gun[dir].pxl);
}
position Ship_get_l_gun_position(shipobj *ship, int gun, int dir)
{
    return ((ship)->l_gun[gun][dir].pxl);
}
position Ship_get_r_gun_position(shipobj *ship, int gun, int dir)
{
    return ((ship)->r_gun[gun][dir].pxl);
}
position Ship_get_l_rgun_position(shipobj *ship, int gun, int dir)
{
    return ((ship)->l_rgun[gun][dir].pxl);
}
position Ship_get_r_rgun_position(shipobj *ship, int gun, int dir)
{
    return ((ship)->r_rgun[gun][dir].pxl);
}
position Ship_get_l_light_position(shipobj *ship, int l, int dir)
{
    return ((ship)->l_light[l][dir].pxl);
}
position Ship_get_r_light_position(shipobj *ship, int l, int dir)
{
    return ((ship)->r_light[l][dir].pxl);
}
position Ship_get_m_rack_position(shipobj *ship, int rack, int dir)
{
    return ((ship)->m_rack[rack][dir].pxl);
}


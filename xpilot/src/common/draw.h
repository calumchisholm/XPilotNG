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

#ifndef	DRAW_H
#define	DRAW_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#include "const.h"
#endif

/*
 * Abstract (non-display system specific) drawing definitions.
 *
 * This file should not contain any X window stuff.
 */

/*
 * The server supports only 4 colors, except for spark/debris, which
 * may have 8 different colors.
 */
#define NUM_COLORS	    4

#define BLACK		    0
#define WHITE		    1
#define BLUE		    2
#define RED		    3

/*
 * Windows deals in Pens, not Colors.  So each pen has to have all of its
 * attributes defined.
 */
#if defined(_WINDOWS) && !defined(PENS_OF_PLENTY)
#define	CLOAKCOLOROFS	15	/* colors 16 and 17 are dashed white/blue */
#define	MISSILECOLOR	18	/* wide white pen */
#define	LASERCOLOR	19	/* wide red pen */
#define	LASERTEAMCOLOR	20	/* wide blue pen */
#define	FUNKCOLORS	6	/* 6 funky colors here (15-20) */
#endif

/*
 * The minimum and maximum playing window sizes supported by the server.
 */
#define MIN_VIEW_SIZE	    384
#define MAX_VIEW_SIZE	    1024
#define DEF_VIEW_SIZE	    768

/*
 * Spark rand limits.
 */
#define MIN_SPARK_RAND	    0		/* Not display spark */
#define MAX_SPARK_RAND	    0x80	/* Always display spark */
#define DEF_SPARK_RAND	    0x55	/* 66% */

#define DSIZE		    4	    /* Size of diamond (on radar) */

#define	UPDATE_SCORE_DELAY	(FPS)

/*
 * Polygon style flags
 */
#define STYLE_FILLED          (1U << 0)
#define STYLE_TEXTURED        (1U << 1)
#define STYLE_INVISIBLE       (1U << 2)
#define STYLE_INVISIBLE_RADAR (1U << 3)

/*
 * Please don't change any of these maxima.
 * It will create incompatibilities and frustration.
 */
#define MIN_SHIP_PTS	    3
#define MAX_SHIP_PTS	    24
/* SSHACK needs to double the vertices */
#define MAX_SHIP_PTS2	    (MAX_SHIP_PTS * 2)
#define MAX_GUN_PTS	    3
#define MAX_LIGHT_PTS	    3
#define MAX_RACK_PTS	    4

/* kps - fix this somehow */
#include "../server/click.h"

/* clk used on server, pxl on client */
typedef union {
    clpos clk;
    position pxl;
} shapepos;

typedef struct {
    shapepos	*pts[MAX_SHIP_PTS2];	/* the shape rotated many ways */
    int		num_points;		/* total points in object */
    int		num_orig_points;	/* points before SSHACK */
    shapepos	cashed_pts[MAX_SHIP_PTS2];
    int		cashed_dir;
} shape_t;

typedef struct {			/* Defines wire-obj, i.e. ship */
    shapepos	*pts[MAX_SHIP_PTS2];	/* the shape rotated many ways */
    int		num_points;		/* total points in object */
    int		num_orig_points;	/* points before SSHACK */
    shapepos	cashed_pts[MAX_SHIP_PTS2];
    int		cashed_dir;

    shapepos	engine[RES];		/* Engine position */
    shapepos	m_gun[RES];		/* Main gun position */
    int		num_l_gun,
		num_r_gun,
		num_l_rgun,
		num_r_rgun;		/* number of additional cannons */
    shapepos	*l_gun[MAX_GUN_PTS],	/* Additional cannon positions, left*/
		*r_gun[MAX_GUN_PTS],	/* Additional cannon positions, right*/
		*l_rgun[MAX_GUN_PTS],	/* Additional rear cannon positions, left*/
		*r_rgun[MAX_GUN_PTS];	/* Additional rear cannon positions, right*/
    int		num_l_light,		/* Number of lights */
		num_r_light;
    shapepos	*l_light[MAX_LIGHT_PTS], /* Left and right light positions */
		*r_light[MAX_LIGHT_PTS];
    int		num_m_rack;		/* Number of missile racks */
    shapepos	*m_rack[MAX_RACK_PTS];
    int		shield_radius;		/* Radius of shield used by client. */

#ifdef	_NAMEDSHIPS
    char*	name;
    char*	author;
#endif
} shipshape_t;

extern shipshape_t *Default_ship(void);
extern void Free_ship_shape(shipshape_t *ship);
extern shipshape_t *Parse_shape_str(char *str);
extern shipshape_t *Convert_shape_str(char *str);
extern void Calculate_shield_radius(shipshape_t *ship);
extern int Validate_shape_str(char *str);
extern void Convert_ship_2_string(shipshape_t *ship, char *buf, char *ext,
				  unsigned shape_version);
extern void Rotate_point(shapepos pt[RES]);
extern void Rotate_position(position pt[RES]);
extern shapepos ipos2shapepos(ipos pos);
extern position shapepos2position(shapepos pt);
extern shapepos *Shape_get_points(shape_t *s, int dir);

shapepos Ship_get_point(shipshape_t *ship, int i, int dir);
shapepos Ship_get_engine(shipshape_t *ship, int dir);
shapepos Ship_get_m_gun(shipshape_t *ship, int dir);
shapepos Ship_get_l_gun(shipshape_t *ship, int gun, int dir);
shapepos Ship_get_r_gun(shipshape_t *ship, int gun, int dir);
shapepos Ship_get_l_rgun(shipshape_t *ship, int gun, int dir);
shapepos Ship_get_r_rgun(shipshape_t *ship, int gun, int dir);
shapepos Ship_get_l_light(shipshape_t *ship, int l, int dir);
shapepos Ship_get_r_light(shipshape_t *ship, int l, int dir);
shapepos Ship_get_m_rack(shipshape_t *ship, int rack, int dir);

position Ship_get_point_position(shipshape_t *ship, int i, int dir);
position Ship_get_engine_position(shipshape_t *ship, int dir);
position Ship_get_m_gun_position(shipshape_t *ship, int dir);
position Ship_get_l_gun_position(shipshape_t *ship, int gun, int dir);
position Ship_get_r_gun_position(shipshape_t *ship, int gun, int dir);
position Ship_get_l_rgun_position(shipshape_t *ship, int gun, int dir);
position Ship_get_r_rgun_position(shipshape_t *ship, int gun, int dir);
position Ship_get_l_light_position(shipshape_t *ship, int l, int dir);
position Ship_get_r_light_position(shipshape_t *ship, int l, int dir);
position Ship_get_m_rack_position(shipshape_t *ship, int rack, int dir);

#ifdef SERVER

static inline clpos Ship_get_point_clpos(shipshape_t *ship, int i, int dir)
{
    return Ship_get_point(ship, i, dir).clk;
}

static inline clpos Ship_get_engine_clpos(shipshape_t *ship, int dir)
{
    return Ship_get_engine(ship, dir).clk;
}
static inline clpos Ship_get_m_gun_clpos(shipshape_t *ship, int dir)
{
    return Ship_get_m_gun(ship, dir).clk;
}
static inline clpos Ship_get_l_gun_clpos(shipshape_t *ship, int gun, int dir)
{
    return Ship_get_l_gun(ship, gun, dir).clk;
}
static inline clpos Ship_get_r_gun_clpos(shipshape_t *ship, int gun, int dir)
{
    return Ship_get_r_gun(ship, gun, dir).clk;
}
static inline clpos Ship_get_l_rgun_clpos(shipshape_t *ship, int gun, int dir)
{
    return Ship_get_l_rgun(ship, gun, dir).clk;
}
static inline clpos Ship_get_r_rgun_clpos(shipshape_t *ship, int gun, int dir)
{
    return Ship_get_r_rgun(ship, gun, dir).clk;
}
static inline clpos Ship_get_l_light_clpos(shipshape_t *ship, int l, int dir)
{
    return Ship_get_l_light(ship, l, dir).clk;
}
static inline clpos Ship_get_r_light_clpos(shipshape_t *ship, int l, int dir)
{
    return Ship_get_r_light(ship, l, dir).clk;
}
static inline clpos Ship_get_m_rack_clpos(shipshape_t *ship, int rack, int dir)
{
    return Ship_get_m_rack(ship, rack, dir).clk;
}

#endif

void Ship_set_point(shipshape_t *ship, int i, ipos pos);
void Ship_set_engine(shipshape_t *ship, ipos pos);
void Ship_set_m_gun(shipshape_t *ship, ipos pos);
void Ship_set_l_gun(shipshape_t *ship, int i, ipos pos);
void Ship_set_r_gun(shipshape_t *ship, int i, ipos pos);
void Ship_set_l_rgun(shipshape_t *ship, int i, ipos pos);
void Ship_set_r_rgun(shipshape_t *ship, int i, ipos pos);
void Ship_set_l_light(shipshape_t *ship, int i, ipos pos);
void Ship_set_r_light(shipshape_t *ship, int i, ipos pos);
void Ship_set_m_rack(shipshape_t *ship, int i, ipos pos);

#endif

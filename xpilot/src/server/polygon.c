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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
#endif

#ifdef _WINDOWS
# include <windows.h>
# include <io.h>
# define read(__a, __b, __c)	_read(__a, __b, __c)
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "defaults.h"
#include "map.h"
#include "error.h"
#include "types.h"
#include "commonproto.h"


char polygon_version[] = VERSION;



/* polygon map format related stuff */
/*static char	*FileName;*/


/* kps - if this is too small, the server will probably say
 * "xpilots: Polygon 2501 (4 points) doesn't start and end at the same place"
 */
static int edg[20000 * 2]; /* !@# change pointers in poly_t when realloc poss.*/
extern int num_polys; /* old name polyc */
extern int num_groups;

int *edges = edg;
int *estyleptr;
int ptscount, ecount;
char *mapd;

struct polystyle pstyles[256];
struct edgestyle estyles[256] =
{{"internal", 0, 0, 0}};	/* Style 0 is always this special style */
struct bmpstyle  bstyles[256];
poly_t *pdata;

int num_pstyles, num_bstyles, num_estyles = 1; /* "Internal" edgestyle */
int max_bases, max_balls, max_polys,max_echanges; /* !@# make static after testing done */
static int current_estyle, current_group, is_decor;

#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	warn("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)
/* !@# add a final realloc later to free wasted memory */


void P_edgestyle(char *id, int width, int color, int style)
{
    strlcpy(estyles[num_estyles].id, id, sizeof(estyles[0].id));
    estyles[num_estyles].color = color;
    estyles[num_estyles].width = width;
    estyles[num_estyles].style = style;
    num_estyles++;
}

void P_polystyle(char *id, int color, int texture_id, int defedge_id,
		   int flags)
{
    /* kps - add sanity checks ??? */
    if (defedge_id == 0) {
	warn("Polygon default edgestyle cannot be omitted or set "
	     "to 'internal'!");
	exit(1);
    }

    strlcpy(pstyles[num_pstyles].id, id, sizeof(pstyles[0].id));
    pstyles[num_pstyles].color = color;
    pstyles[num_pstyles].texture_id = texture_id;
    pstyles[num_pstyles].defedge_id = defedge_id;
    pstyles[num_pstyles].flags = flags;
    num_pstyles++;
}


void P_bmpstyle(char *id, char *filename, int flags)
{
    strlcpy(bstyles[num_bstyles].id, id, sizeof(bstyles[0].id));
    strlcpy(bstyles[num_bstyles].filename, filename,
	    sizeof(bstyles[0].filename));
    bstyles[num_bstyles].flags = flags;
    num_bstyles++;
}

/* current vertex */
static clpos P_cv;

void P_start_polygon(int cx, int cy, int style)
{
    poly_t t;

    if (cx < 0 || cx >= World.cwidth || cy < 0 || cy > World.cheight) {
	warn("Polygon start point (%d, %d) is not inside the map"
	     "(0 <= x < %d, 0 <= y < %d)",
	     cx, cy, World.cwidth, World.cheight);
	exit(1);
    }
    if (style == -1) {
	warn("Currently you must give polygon style, no default");
	exit(1);
    }

    ptscount = 0;
    P_cv.cx = cx;
    P_cv.cy = cy;
    t.x = cx;
    t.y = cy;
    t.group = current_group;
    t.edges = edges;
    t.style = style;
    t.estyles_start = ecount;
    t.is_decor = is_decor;
    current_estyle = pstyles[style].defedge_id;
    STORE(poly_t, pdata, num_polys, max_polys, t);
}


void P_offset(int offcx, int offcy, int edgestyle)
{
    /* don't add duplicates */
    if (offcx == 0 && offcy == 0)
	return;

    if (ABS(offcx) > POLYGON_MAX_OFFSET || ABS(offcy) > POLYGON_MAX_OFFSET) {
	warn("Offset component absolute value exceeds %d (x=%d, y=%d)",
	     POLYGON_MAX_OFFSET, offcx, offcy);
	exit(1);
    }

    *edges++ = offcx;
    *edges++ = offcy;
    if (edgestyle != -1 && edgestyle != current_estyle) {
	STORE(int, estyleptr, ecount, max_echanges, ptscount);
	STORE(int, estyleptr, ecount, max_echanges, edgestyle);
	current_estyle = edgestyle;
    }
    ptscount++;
    P_cv.cx += offcx;
    P_cv.cy += offcy;
}

void P_vertex(int cx, int cy, int edgestyle)
{
    P_offset(cx - P_cv.cx, cy - P_cv.cy, edgestyle);
}

void P_end_polygon(void)
{
    pdata[num_polys - 1].num_points = ptscount;
    pdata[num_polys - 1].num_echanges
	= ecount -pdata[num_polys - 1].estyles_start;
    STORE(int, estyleptr, ecount, max_echanges, INT_MAX);
}

void P_start_ballarea(void)
{
    current_group = ++num_groups;
    groups[current_group].type = TREASURE;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hit_mask = BALL_BIT;
    groups[current_group].hit_func = NULL;
    groups[current_group].item_id = -1;
}

void P_end_ballarea(void)
{
    current_group = 0; 
}

void P_start_balltarget(int team)
{
    current_group = ++num_groups;
    groups[current_group].type = TREASURE;
    groups[current_group].team = team;
    groups[current_group].hit_mask = NONBALL_BIT;
    /*= NONBALL_BIT | (((NOTEAM_BIT << 1) - 1) & ~(1 << team));*/
    groups[current_group].hit_func = Balltarget_hit_func;
    groups[current_group].item_id = -1;
}

void P_end_balltarget(void)
{
    current_group = 0; 
}

void P_start_target(int team, int ind)
{
    current_group = ++num_groups;
    groups[current_group].type = TARGET;
    groups[current_group].team = team;
    groups[current_group].hit_mask = HITMASK(team);
    groups[current_group].hit_func = NULL;
    groups[current_group].item_id = ind;
}

void P_end_target(void)
{
    current_group = 0; 
}

void P_start_cannon(int cx, int cy, int dir, int team, int ind)
{
    current_group = ++num_groups;
    groups[current_group].type = CANNON;
    groups[current_group].team = team;
    groups[current_group].hit_mask = HITMASK(team);
    groups[current_group].hit_func = NULL;
    groups[current_group].item_id = ind;
}

void P_end_cannon(void)
{
    current_group = 0; 
}

void P_start_wormhole(int ind)
{
    current_group = ++num_groups;
    groups[current_group].type = WORMHOLE;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hit_mask = 0;
    groups[current_group].hit_func = NULL;
    groups[current_group].item_id = ind;
}

void P_end_wormhole(void)
{
    current_group = 0; 
}

void P_start_frictionarea(void)
{
    current_group = ++num_groups;
    groups[current_group].type = FRICTION;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hit_mask = 0xFFFFFFFF; /* kps - hack */
    groups[current_group].hit_func = NULL;
    groups[current_group].item_id = -1;
}

void P_end_frictionarea(void)
{
    current_group = 0; 
}

void P_start_decor(void)
{
    is_decor = 1;
}

void P_end_decor(void)
{
    is_decor = 0;
}

int P_get_bmp_id(const char *s)
{
    int i;

    for (i = 0; i < num_bstyles; i++)
	if (!strcmp(bstyles[i].id, s))
	    return i;
    warn("Undeclared bmpstyle %s", s);
    return -1;
}


int P_get_edge_id(const char *s)
{
    int i;

    for (i = 0; i < num_estyles; i++)
	if (!strcmp(estyles[i].id, s))
	    return i;
    warn("Undeclared edgestyle %s", s);
    return -1;
}


int P_get_poly_id(const char *s)
{
    int i;

    for (i = 0; i < num_pstyles; i++)
	if (!strcmp(pstyles[i].id, s))
	    return i;
    warn("Undeclared polystyle %s", s);
    return -1;
}





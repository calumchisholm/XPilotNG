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
#include "xpconfig.h"
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


int num_edges, max_edges;
extern int num_polys; /* old name polyc */
extern int num_groups;

int *edgeptr;
int *estyleptr;
static int ptscount, ecount;

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


static void Check_groupcount(void)
{
    if (current_group == 1000) {
	warn("Server didn't allocate enough space for groups");
	exit(1);
    }
}

void P_edgestyle(char *id, int width, int color, int style)
{
    if (num_estyles > 255) {
	warn("Too many edgestyles");
	exit(1);
    }

    strlcpy(estyles[num_estyles].id, id, sizeof(estyles[0].id));
    estyles[num_estyles].color = color;
    estyles[num_estyles].width = width;
    estyles[num_estyles].style = style;
    num_estyles++;
}

void P_polystyle(char *id, int color, int texture_id, int defedge_id,
		   int flags)
{
    if (num_pstyles > 255) {
	warn("Too many polygon styles");
	exit(1);
    }
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
    if (num_bstyles > 255) {
	warn("Too many bitmap styles");
	exit(1);
    }
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

    if (!INSIDE_MAP(cx, cy)) {
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
    t.cx = cx;
    t.cy = cy;
    t.group = current_group;
    t.edges = num_edges;
    t.style = style;
    t.estyles_start = ecount;
    t.is_decor = is_decor;
    current_estyle = pstyles[style].defedge_id;
    STORE(poly_t, pdata, num_polys, max_polys, t);
}


void P_offset(int offcx, int offcy, int edgestyle)
{
    int i;

    if (offcx == 0 && offcy == 0) {
	warn("Edge with zero length");
	if (edgestyle != -1 && edgestyle != current_estyle) {
	    warn("Refusing to change edgestyle with zero-length edge");
	    exit(1);
	}
	return;
    }

    if (edgestyle != -1 && edgestyle != current_estyle) {
	STORE(int, estyleptr, ecount, max_echanges, ptscount);
	STORE(int, estyleptr, ecount, max_echanges, edgestyle);
	current_estyle = edgestyle;
    }

    P_cv.cx += offcx;
    P_cv.cy += offcy;

    i = (MAX(ABS(offcx), ABS(offcy)) - 1) / POLYGON_MAX_OFFSET + 1;
    for (;i > 0;i--) {
	STORE(int, edgeptr, num_edges, max_edges, offcx / i);
	STORE(int, edgeptr, num_edges, max_edges, offcy / i);
	offcx -= offcx / i;
	offcy -= offcy / i;
	ptscount++;
    }
}

void P_vertex(int cx, int cy, int edgestyle)
{
    P_offset(cx - P_cv.cx, cy - P_cv.cy, edgestyle);
}

void P_end_polygon(void)
{
    if (ptscount < 3) {
	warn("Polygon with less than 3 edges?? (start %d, %d)",
	     pdata[num_polys - 1].cx, pdata[num_polys - 1].cy);
	exit(-1);
    }
    pdata[num_polys - 1].num_points = ptscount;
    pdata[num_polys - 1].num_echanges
	= ecount -pdata[num_polys - 1].estyles_start;
    STORE(int, estyleptr, ecount, max_echanges, INT_MAX);
}

int P_start_ballarea(void)
{
    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = TREASURE;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hitmask = BALL_BIT;
    groups[current_group].hitfunc = NULL;
    groups[current_group].item_id = -1;
    return current_group;
}

void P_end_ballarea(void)
{
    current_group = 0;
}

int P_start_balltarget(int team)
{
    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = TREASURE;
    groups[current_group].team = team;
    groups[current_group].hitmask = NONBALL_BIT;
    /*= NONBALL_BIT | (((NOTEAM_BIT << 1) - 1) & ~(1 << team));*/
    groups[current_group].hitfunc = Balltarget_hitfunc;
    groups[current_group].item_id = -1;
    return current_group;
}

void P_end_balltarget(void)
{
    current_group = 0;
}

int P_start_target(int team, int ind)
{
    target_t *targ = &World.targets[ind];

    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = TARGET;
    groups[current_group].team = team;
    groups[current_group].hitmask = Target_hitmask(targ);
    groups[current_group].hitfunc = NULL /*Target_hitfunc*/;
    groups[current_group].item_id = ind;
    targ->group = current_group;
    return current_group;
}

void P_end_target(void)
{
    current_group = 0;
}

int P_start_cannon(int team, int ind)
{
    cannon_t *cannon = &World.cannon[ind];

    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = CANNON;
    groups[current_group].team = team;
    groups[current_group].hitmask = Cannon_hitmask(cannon);
    groups[current_group].hitfunc = Cannon_hitfunc;
    groups[current_group].item_id = ind;
    cannon->group = current_group;
    return current_group;
}

void P_end_cannon(void)
{
    current_group = 0;
}

int P_start_wormhole(int ind)
{
    wormhole_t		*wormhole = &World.wormHoles[ind];

    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = WORMHOLE;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hitmask = Wormhole_hitmask(wormhole);
    groups[current_group].hitfunc = Wormhole_hitfunc;
    groups[current_group].item_id = ind;
    return current_group;
}

void P_end_wormhole(void)
{
    current_group = 0;
}

int P_start_frictionarea(void)
{
    current_group = ++num_groups;
    Check_groupcount();
    groups[current_group].type = FRICTION;
    groups[current_group].team = TEAM_NOT_SET;
    groups[current_group].hitmask = 0xFFFFFFFF; /* kps - hack */
    groups[current_group].hitfunc = NULL;
    groups[current_group].item_id = -1;
    return current_group;
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
    warn("Broken map: Undeclared bmpstyle %s", s);
    exit(-1);
}


int P_get_edge_id(const char *s)
{
    int i;

    for (i = 0; i < num_estyles; i++)
	if (!strcmp(estyles[i].id, s))
	    return i;
    warn("Broken map: Undeclared edgestyle %s", s);
    exit(-1);
}


int P_get_poly_id(const char *s)
{
    int i;

    for (i = 0; i < num_pstyles; i++)
	if (!strcmp(pstyles[i].id, s))
	    return i;
    warn("Broken map: Undeclared polystyle %s", s);
    exit(-1);
}

/*
 * Call given function f with group item id as argument for
 * all groups of grouptype type.
 *
 * kps - which group numbers are ok ???
 * Is it 1 to num_groups ???
 */
void P_grouphack(int type, void (*f)(int))
{
    int group;

    for (group = 0; group <= num_groups; group++) {
	if (groups[group].type == type) {
	    (*f)(groups[group].item_id);
	}
    }
}

void P_set_hitmask(int group, int hitmask)
{
    if (group < 0 || group > num_groups) {
	xpprintf("P_set_hitmask: BUG: group out of range.\n");
	return;
    }
    groups[group].hitmask = hitmask;
}

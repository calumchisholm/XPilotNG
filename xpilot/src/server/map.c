/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
#include <sys/file.h>
#else
#include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "bit.h"
#include "error.h"
#include "click.h"
#include "commonproto.h"

char map_version[] = VERSION;

#define GRAV_RANGE  10

/*
 * Globals.
 */
World_map World;


void Init_map(void)
{
    int i;
    for (i = 0; i < MAX_TEAMS; i++) {
	World.teams[i].NumMembers = 0;
	World.teams[i].NumBases = 0;
	World.teams[i].NumTreasures = 0;
	World.teams[i].TreasuresDestroyed = 0;
	World.teams[i].TreasuresLeft = 0;
	World.teams[i].SwapperId = -1;
    }
    World.NumBases = 0;
    World.NumTreasures = 0;
    World.NumFuels = 0;
    World.NumChecks = 0;

    if (World.NumChecks > 0
	&& (World.check = malloc(World.NumChecks * sizeof(ipos))) ==NULL) {
	error("Out of memory - checkpoints");
	exit(-1);
    }
    World.NumGravs	= 0;
    World.NumCannons	= 0;
    World.NumWormholes	= 0;
    World.NumTargets	= 0;
    World.NumItemConcentrators	= 0;
}


void Free_map(void)
{
    if (World.gravity) {
	free(World.gravity);
	World.gravity = NULL;
    }
    if (World.grav) {
	free(World.grav);
	World.grav = NULL;
    }
    if (World.base) {
	free(World.base);
	World.base = NULL;
    }
    if (World.cannon) {
	free(World.cannon);
	World.cannon = NULL;
    }
    if (World.fuel) {
	free(World.fuel);
	World.fuel = NULL;
    }
    if (World.wormHoles) {
	free(World.wormHoles);
	World.wormHoles = NULL;
    }
    if (World.itemConcentrators) {
	free(World.itemConcentrators);
	World.itemConcentrators = NULL;
    }
}


void Alloc_map(void)
{
    int x;

    if (World.gravity)
	Free_map();

    World.gravity =
	(vector **)malloc(sizeof(vector *)*World.x
			  + World.x*sizeof(vector)*World.y);
    World.grav = NULL;
    World.cannon = NULL;
    World.wormHoles = NULL;
    World.itemConcentrators = NULL;
    if (World.gravity == NULL) {
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
	      World.x * (World.y * (sizeof(unsigned char) + sizeof(vector))
			 + sizeof(vector*)
			 + sizeof(unsigned char*)));
	exit(-1);
    } else {
	vector *grav_line;
	vector **grav_pointer;

	grav_pointer = World.gravity;
	grav_line = (vector*) ((vector**)grav_pointer + World.x);

	for (x=0; x<World.x; x++) {
	    *grav_pointer = grav_line;
	    grav_pointer += 1;
	    grav_line += World.y;
	}
    }
}


void Grok_map(void)
{
    Init_map();

#if 0
    /* !@# */
    if (mapWidth > MAX_MAP_SIZE || mapHeight > MAX_MAP_SIZE) {
	errno = 0;
	error("mapWidth or mapHeight exceeds map size limit %d", MAX_MAP_SIZE);
    } else {
	World.x = mapWidth;
	World.y = mapHeight;
    }
#endif
    World.width = mapWidth;
    World.height = mapHeight;
    World.x = (mapWidth - 1) / BLOCK_SZ + 1; /* !@# */
    World.y = (mapHeight - 1) / BLOCK_SZ + 1;
    World.diagonal = (int) LENGTH(World.x, World.y);
    World.cwidth = World.width * CLICK;
    World.cheight = World.height * CLICK;
    World.hypotenuse = (int) LENGTH(World.width, World.height);
    strncpy(World.name, mapName, sizeof(World.name) - 1);
    World.name[sizeof(World.name) - 1] = '\0';
    strncpy(World.author, mapAuthor, sizeof(World.author) - 1);
    World.author[sizeof(World.author) - 1] = '\0';
    strncpy(World.dataURL, dataURL, sizeof(World.dataURL) - 1);
    World.dataURL[sizeof(World.dataURL) - 1] = 0;

    Alloc_map();

    Set_world_rules();
    Set_world_items();

    if (BIT(World.rules->mode, TEAM_PLAY|TIMING) == (TEAM_PLAY|TIMING)) {
	error("Cannot teamplay while in race mode -- ignoring teamplay");
	CLR_BIT(World.rules->mode, TEAM_PLAY);
    }

#ifndef	SILENT
    xpprintf("World....: %s\nBases....: %d\nMapsize..: %dx%d\nTeam play: %s\n",
	   World.name, World.NumBases, World.x, World.y,
	   BIT(World.rules->mode, TEAM_PLAY) ? "on" : "off");
#endif
    return;
}

#if 0
    int i, x, y, c;
    char *s;

    x = -1;
    y = World.y - 1;

    s = mapData;
    while (y >= 0) {

	x++;

	c = *s;
	if (c == '\0' || c == EOF) {
	    if (x < World.x) {
		/* not enough map data on this line */
		Map_error(World.y - y);
		c = ' ';
	    } else {
		c = '\n';
	    }
	} else {
	    if (c == '\n' && x < World.x) {
		/* not enough map data on this line */
		Map_error(World.y - y);
		c = ' ';
	    } else {
		s++;
	    }
	}
	if (x >= World.x || c == '\n') {
	    y--; x = -1;
	    if (c != '\n') {			/* Get rest of line */
		error("Map file contains extranous characters");
		while (c != '\n' && c != EOF)	/* from file. */
		    fputc(c = *s++, stderr);
	    }
	    continue;
	}

	switch (World.block[x][y] = c) {
	case 'r':
	case 'd':
	case 'f':
	case 'c':
	    World.NumCannons++;
	    break;
	case '!':
	    World.NumTargets++;
	    break;
	case '%':
	    World.NumItemConcentrators++;
	    break;
	case '+':
	case '-':
	case '>':
	case '<':
        case 'i':
        case 'm':
        case 'j':
        case 'k':
	    World.NumGravs++;
	    break;
	case '@':
	case '(':
	case ')':
	    World.NumWormholes++;
	    break;
	default:
	    break;
	}
    }

    free(mapData);
    mapData = NULL;

    /*
     * Get space for special objects.
     */
    if (World.NumCannons > 0
	&& (World.cannon = (cannon_t *)
	    malloc(World.NumCannons * sizeof(cannon_t))) == NULL) {
	error("Out of memory - cannons");
	exit(-1);
    }
    if (World.NumGravs > 0
	&& (World.grav = (grav_t *)
	    malloc(World.NumGravs * sizeof(grav_t))) == NULL) {
	error("Out of memory - gravs");
	exit(-1);
    }
    if (World.NumWormholes > 0
	&& (World.wormHoles = (wormhole_t *)
	    malloc(World.NumWormholes * sizeof(wormhole_t))) == NULL) {
	error("Out of memory - wormholes");
	exit(-1);
    }
    if (World.NumTargets > 0
	&& (World.targets = (target_t *)
	    malloc(World.NumTargets * sizeof(target_t))) == NULL) {
	error("Out of memory - targets");
	exit(-1);
    }
    if (World.NumItemConcentrators > 0
	&& (World.itemConcentrators = (item_concentrator_t *)
	    malloc(World.NumItemConcentrators * sizeof(item_concentrator_t))) == NULL) {
	error("Out of memory - item concentrators");
	exit(-1);
    }

    World.NumCannons = 0;		/* Now reset all counters since */
    World.NumGravs = 0;			/* (and reuse these counters) */
    World.NumWormholes = 0;		/* while inserting the objects */
                       ;		/* into structures. */
    World.NumTargets = 0;
    World.NumItemConcentrators = 0;

    /*
     * Change read tags to internal data, create objects
     */
    {
	int	worm_in = 0,
		worm_out = 0,
		worm_norm = 0;

	for (x=0; x<World.x; x++) {
	    u_byte *line = World.block[x];
	    u_short *itemID = World.itemID[x];

	    for (y=0; y<World.y; y++) {
		char c = line[y];

		itemID[y] = (u_short) -1;

		switch (c) {
		case ' ':
		case '.':
		default:
		    line[y] = SPACE;
		    break;

		case 'x':
		    line[y] = FILLED;
		    break;
		case 's':
		    line[y] = REC_LU;
		    break;
		case 'a':
		    line[y] = REC_RU;
		    break;
		case 'w':
		    line[y] = REC_LD;
		    break;
		case 'q':
		    line[y] = REC_RD;
		    break;

		case 'r':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_UP;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].clk_pos.x =
						(x + 0.5) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].clk_pos.y =
						(y + 0.333) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'd':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_LEFT;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].clk_pos.x =
						(x + 0.667) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].clk_pos.y =
						(y + 0.5) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'f':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_RIGHT;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].clk_pos.x =
						(x + 0.333) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].clk_pos.y =
						(y + 0.5) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;
		case 'c':
		    line[y] = CANNON;
		    itemID[y] = World.NumCannons;
		    World.cannon[World.NumCannons].dir = DIR_DOWN;
		    World.cannon[World.NumCannons].blk_pos.x = x;
		    World.cannon[World.NumCannons].blk_pos.y = y;
		    World.cannon[World.NumCannons].clk_pos.x =
						(x + 0.5) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].clk_pos.y =
						(y + 0.667) * BLOCK_CLICKS;
		    World.cannon[World.NumCannons].dead_time = 0;
		    World.cannon[World.NumCannons].conn_mask = (unsigned)-1;
		    World.cannon[World.NumCannons].team = TEAM_NOT_SET;
		    Cannon_init(World.NumCannons);
		    World.NumCannons++;
		    break;

		case '!':
		    line[y] = TARGET;
		    itemID[y] = World.NumTargets;
		    World.targets[World.NumTargets].pos.x = x;
		    World.targets[World.NumTargets].pos.y = y;
		    /*
		     * Determining which team it belongs to is done later,
		     * in Find_closest_team().
		     */
		    World.targets[World.NumTargets].team = 0;
		    World.targets[World.NumTargets].dead_time = 0;
		    World.targets[World.NumTargets].damage = TARGET_DAMAGE;
		    World.targets[World.NumTargets].conn_mask = (unsigned)-1;
		    World.targets[World.NumTargets].update_mask = 0;
		    World.targets[World.NumTargets].last_change = frame_loops;
		    World.NumTargets++;
		    break;
		case '%':
		    line[y] = ITEM_CONCENTRATOR;
		    itemID[y] = World.NumItemConcentrators;
		    World.itemConcentrators[World.NumItemConcentrators].pos.x = x;
		    World.itemConcentrators[World.NumItemConcentrators].pos.y = y;
		    World.NumItemConcentrators++;
		    break;
		case '$':
		    line[y] = BASE_ATTRACTOR;
		    break;

		case '+':
		    line[y] = POS_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '-':
		    line[y] = NEG_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '>':
		    line[y]= CWISE_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
		case '<':
		    line[y] = ACWISE_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'i':
		    line[y] = UP_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'm':
		    line[y] = DOWN_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;
	        case 'k':
		    line[y] = RIGHT_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = GRAVS_POWER;
		    World.NumGravs++;
		    break;
                case 'j':
		    line[y] = LEFT_GRAV;
		    itemID[y] = World.NumGravs;
		    World.grav[World.NumGravs].pos.x = x;
		    World.grav[World.NumGravs].pos.y = y;
		    World.grav[World.NumGravs].force = -GRAVS_POWER;
		    World.NumGravs++;
		    break;

		case '@':
		case '(':
		case ')':
		    World.wormHoles[World.NumWormholes].pos.x = x;
		    World.wormHoles[World.NumWormholes].pos.y = y;
		    World.wormHoles[World.NumWormholes].countdown = 0;
		    World.wormHoles[World.NumWormholes].lastdest = -1;
		    World.wormHoles[World.NumWormholes].lastplayer = -1;
		    World.wormHoles[World.NumWormholes].temporary = 0;
		    if (c == '@') {
			World.wormHoles[World.NumWormholes].type = WORM_NORMAL;
			worm_norm++;
		    } else if (c == '(') {
			World.wormHoles[World.NumWormholes].type = WORM_IN;
			worm_in++;
		    } else {
			World.wormHoles[World.NumWormholes].type = WORM_OUT;
			worm_out++;
		    }
		    line[y] = WORMHOLE;
		    itemID[y] = World.NumWormholes;
		    World.NumWormholes++;
		    break;

		case 'b':
		    line[y] = DECOR_FILLED;
		    break;
		case 'h':
		    line[y] = DECOR_LU;
		    break;
		case 'g':
		    line[y] = DECOR_RU;
		    break;
		case 'y':
		    line[y] = DECOR_LD;
		    break;
		case 't':
		    line[y] = DECOR_RD;
		    break;
		}
	    }
	}

	if (!wormTime) {
	    for (i = 0; i < World.NumWormholes; i++) {
		int j = (int)(rfrac() * World.NumWormholes);
		while (World.wormHoles[j].type == WORM_IN)
		    j = (int)(rfrac() * World.NumWormholes);
		World.wormHoles[i].lastdest = j;
	    }
	}

	/*
	 * Determine which team a treasure belongs to.
	 * NOTE: Should check so that all teams have one, and only one,
	 * treasure.
	 */
	if (BIT(World.rules->mode, TEAM_PLAY)) {
	    u_short team = TEAM_NOT_SET;
	    for (i=0; i<World.NumTargets; i++) {
		team = Find_closest_team(World.targets[i].pos.x,
					 World.targets[i].pos.y);
		if (team == TEAM_NOT_SET) {
		    error("Couldn't find a matching team for the target.");
		}
		World.targets[i].team = team;
	    }
	    if (teamCannons) {
		for (i=0; i<World.NumCannons; i++) {
		    team = Find_closest_team(World.cannon[i].blk_pos.x,
					     World.cannon[i].blk_pos.y);
		    if (team == TEAM_NOT_SET) {
			error("Couldn't find a matching team for the target.");
		    }
		    World.cannon[i].team = team;
		}
	    }
	}
    }
}
#endif


DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}

DFLOAT Wrap_cfindDir(int dx, int dy)
{
    dx = CENTER_XCLICK(dx);
    dy = CENTER_YCLICK(dy);
    return findDir(dx, dy);
}

DFLOAT Wrap_length(int dx, int dy)
{
    dx = CENTER_XCLICK(dx);
    dy = CENTER_YCLICK(dy);
    return LENGTH(dx, dy);
}


static void Compute_global_gravity(void)
{
    int			xi, yi, dx, dy;
    DFLOAT		xforce, yforce, strength;
    double		theta;
    vector		*grav;


    if (gravityPointSource == false) {
	theta = (gravityAngle * PI) / 180.0;
	xforce = cos(theta) * Gravity;
	yforce = sin(theta) * Gravity;
	for (xi=0; xi<World.x; xi++) {
	    grav = World.gravity[xi];

	    for (yi=0; yi<World.y; yi++, grav++) {
		grav->x = xforce;
		grav->y = yforce;
	    }
	}
    } else {
	for (xi=0; xi<World.x; xi++) {
	    grav = World.gravity[xi];
	    dx = (xi - gravityPoint.x) * BLOCK_SZ;
	    dx = WRAP_DX(dx);

	    for (yi=0; yi<World.y; yi++, grav++) {
		dy = (yi - gravityPoint.y) * BLOCK_SZ;
		dy = WRAP_DX(dy);

		if (dx == 0 && dy == 0) {
		    grav->x = 0.0;
		    grav->y = 0.0;
		    continue;
		}
		strength = Gravity / LENGTH(dx, dy);
		if (gravityClockwise) {
		    grav->x =  dy * strength;
		    grav->y = -dx * strength;
		}
		else if (gravityAnticlockwise) {
		    grav->x = -dy * strength;
		    grav->y =  dx * strength;
		}
		else {
		    grav->x =  dx * strength;
		    grav->y =  dy * strength;
		}
	    }
	}
    }
}


static void Compute_grav_tab(vector grav_tab[GRAV_RANGE+1][GRAV_RANGE+1])
{
    int			x, y;
    double		strength;

    grav_tab[0][0].x = grav_tab[0][0].y = 0;
    for (x = 0; x < GRAV_RANGE+1; x++) {
	for (y = (x == 0); y < GRAV_RANGE+1; y++) {
	    strength = pow((double)(sqr(x) + sqr(y)), -1.5);
	    grav_tab[x][y].x = x * strength;
	    grav_tab[x][y].y = y * strength;
	}
    }
}


static void Compute_local_gravity(void)
{
    int			xi, yi, g, gx, gy, ax, ay, dx, dy, gtype;
    int			first_xi, last_xi, first_yi, last_yi, mod_xi, mod_yi;
    int			min_xi, max_xi, min_yi, max_yi;
    DFLOAT		force, fx, fy;
    vector		*v, *grav, *tab, grav_tab[GRAV_RANGE+1][GRAV_RANGE+1];


    Compute_grav_tab(grav_tab);

    min_xi = 0;
    max_xi = World.x - 1;
    min_yi = 0;
    max_yi = World.y - 1;
    if (BIT(World.rules->mode, WRAP_PLAY)) {
	min_xi -= MIN(GRAV_RANGE, World.x);
	max_xi += MIN(GRAV_RANGE, World.x);
	min_yi -= MIN(GRAV_RANGE, World.y);
	max_yi += MIN(GRAV_RANGE, World.y);
    }
    for (g=0; g<World.NumGravs; g++) {
	gx = World.grav[g].pos.x;
	gy = World.grav[g].pos.y;
	force = World.grav[g].force;

	if ((first_xi = gx - GRAV_RANGE) < min_xi) {
	    first_xi = min_xi;
	}
	if ((last_xi = gx + GRAV_RANGE) > max_xi) {
	    last_xi = max_xi;
	}
	if ((first_yi = gy - GRAV_RANGE) < min_yi) {
	    first_yi = min_yi;
	}
	if ((last_yi = gy + GRAV_RANGE) > max_yi) {
	    last_yi = max_yi;
	}
	/* !@#	gtype = World.block[gx][gy]; */
	mod_xi = (first_xi < 0) ? (first_xi + World.x) : first_xi;
	dx = gx - first_xi;
	fx = force;
	for (xi = first_xi; xi <= last_xi; xi++, dx--) {
	    if (dx < 0) {
		fx = -force;
		ax = -dx;
	    } else {
		ax = dx;
	    }
	    mod_yi = (first_yi < 0) ? (first_yi + World.y) : first_yi;
	    dy = gy - first_yi;
	    grav = &World.gravity[mod_xi][mod_yi];
	    tab = grav_tab[ax];
	    fy = force;
	    for (yi = first_yi; yi <= last_yi; yi++, dy--) {
		if (dx || dy) {
		    if (dy < 0) {
			fy = -force;
			ay = -dy;
		    } else {
			ay = dy;
		    }
		    v = &tab[ay];
		    if (gtype == CWISE_GRAV || gtype == ACWISE_GRAV) {
			grav->x -= fy * v->y;
			grav->y += fx * v->x;
		    } else if (gtype == UP_GRAV || gtype == DOWN_GRAV) {
			grav->y += force * v->x;
		    } else if (gtype == RIGHT_GRAV || gtype == LEFT_GRAV) {
			grav->x += force * v->y;
		    } else {
			grav->x += fx * v->x;
			grav->y += fy * v->y;
		    }
		}
		else {
		    if (gtype == UP_GRAV || gtype == DOWN_GRAV) {
			grav->y += force;
		    }
		    else if (gtype == LEFT_GRAV || gtype == RIGHT_GRAV) {
			grav->x += force;
		    }
		}
		mod_yi++;
		grav++;
		if (mod_yi >= World.y) {
		    mod_yi = 0;
		    grav = World.gravity[mod_xi];
		}
	    }
	    if (++mod_xi >= World.x) {
		mod_xi = 0;
	    }
	}
    }
    /*
     * We may want to free the World.gravity memory here
     * as it is not used anywhere else.
     * e.g.: free(World.gravity);
     *       World.gravity = NULL;
     *       World.NumGravs = 0;
     * Some of the more modern maps have quite a few gravity symbols.
     */
}


void Compute_gravity(void)
{
    Compute_global_gravity();
    Compute_local_gravity();
}

/* !@# */
#if 0
void add_temp_wormholes(int xin, int yin, int xout, int yout, int ind)
{
    wormhole_t inhole, outhole, *wwhtemp;

    if ((wwhtemp = (wormhole_t *)realloc(World.wormHoles,
					 (World.NumWormholes + 2)
					 * sizeof(wormhole_t)))
	== NULL) {
	error("No memory for temporary wormholes.");
	return;
    }
    World.wormHoles = wwhtemp;

    inhole.pos.x = xin;
    inhole.pos.y = yin;
    outhole.pos.x = xout;
    outhole.pos.y = yout;
    inhole.countdown = outhole.countdown = wormTime * FPS;
    inhole.lastdest = World.NumWormholes + 1;
    inhole.lastplayer = outhole.lastplayer = ind;
    inhole.temporary = outhole.temporary = 1;
    inhole.type = WORM_IN;
    outhole.type = WORM_OUT;
    World.wormHoles[World.NumWormholes] = inhole;
    World.wormHoles[World.NumWormholes + 1] = outhole;
    World.block[xin][yin] = World.block[xout][yout] = WORMHOLE;
/*
    World.itemID[xin][yin] = World.NumWormholes;
    World.itemID[xout][yout] = World.NumWormholes + 1;
*/
    World.NumWormholes += 2;
}


void remove_temp_wormhole(int ind)
{
    wormhole_t hole;

    hole = World.wormHoles[ind];
    World.block[hole.pos.x][hole.pos.y] = SPACE;
/*
    World.itemID[hole.pos.x][hole.pos.y] = (u_short) -1;
*/
    World.NumWormholes--;
    if (ind != World.NumWormholes) {
	World.wormHoles[ind] = World.wormHoles[World.NumWormholes];
    }
    World.wormHoles = (wormhole_t *)realloc(World.wormHoles,
					    World.NumWormholes
					    * sizeof(wormhole_t));
}
#endif

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <sys/file.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "bit.h"
#include "error.h"
#include "commonproto.h"

char map_version[] = VERSION;

#define GRAV_RANGE  10


#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	warn("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)
/* !@# add a final realloc later to free wasted memory */
int max_asteroidconcs = 0, max_bases = 0, max_cannons = 0, max_checks = 0,
    max_fuels = 0, max_gravs = 0, max_itemconcs = 0,
    max_targets = 0, max_treasures = 0, max_wormholes = 0;

/*
 * Globals.
 */
World_map World;
bool is_polygon_map = false;

static void Generate_random_map(void);

static void Find_base_order(void);

static void Reset_map_object_counters(void);
static void Reset_itemid_array(void);


static void shrink(void **pp, size_t size)
{
    void *p;

    p = realloc(*pp, size);
    if (!p) {
	warn("Realloc failed!");
	exit(1);
    }
    *pp = p;
}

#define SHRINK(T,P,N,M) { \
if ((M) > (N)) { \
  shrink((void **)&(P), (N) * sizeof(T)); \
  M = (N); \
} } \


static void Realloc_map_objects(void)
{
    SHRINK(cannon_t, World.cannon, World.NumCannons, max_cannons);
    SHRINK(fuel_t, World.fuel, World.NumFuels, max_fuels);
    SHRINK(grav_t, World.grav, World.NumGravs, max_gravs);
    SHRINK(wormhole_t, World.wormHoles, World.NumWormholes, max_wormholes);
    SHRINK(treasure_t, World.treasures, World.NumTreasures, max_treasures);
    SHRINK(target_t, World.targets, World.NumTargets, max_targets);
    SHRINK(base_t, World.base, World.NumBases, max_bases);
    SHRINK(item_concentrator_t, World.itemConcentrators,
	   World.NumItemConcentrators, max_itemconcs);
    SHRINK(asteroid_concentrator_t, World.asteroidConcs,
	   World.NumAsteroidConcs, max_asteroidconcs);
}



#include <ctype.h>
void asciidump(void *p, size_t size)
{
    int i;
    unsigned char *up = p;
    char c;

    for (i = 0; i < size; i++) {
       if (!(i % 64))
           printf("\n%08x ", i);
       c = *(up + i);
       if (isprint(c))
           printf("%c", c);
       else
           printf(".");
    }
    printf("\n\n");
}


void hexdump(void *p, size_t size)
{
    int i;
    unsigned char *up = p;

    for (i = 0; i < size; i++) {
	if (!(i % 16))
	    printf("\n%08x ", i);
	printf("%02x ", *(up + i));
    }
    printf("\n\n");
}

/* kps - tmp hacks to get rid of explicit refrences to World.itemID */
void Map_set_itemid(int x, int y, int ind)
{
    if (x < 0 || x >= World.x || y < 0 || y >= World.y)
	return;
    World.itemID[x][y] = (unsigned short)ind;
}

int Map_get_itemid(int x, int y)
{
    if (x < 0 || x >= World.x || y < 0 || y >= World.y)
	return -1;
    return (int)World.itemID[x][y];
}


int Map_place_cannon(int cx, int cy, int dir, int team)
{
    cannon_t t;
    int ind = World.NumCannons;

    t.pos.cx = cx;
    t.pos.cy = cy;
    t.dir = dir;
    t.team = team;
    t.dead_time = 0;
    t.conn_mask = (unsigned)-1;
    t.group = -1;
    STORE(cannon_t, World.cannon, World.NumCannons, max_cannons, t);
    Cannon_init(ind);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_fuel(int cx, int cy, int team)
{
    fuel_t t;
    int ind = World.NumFuels;

    t.pos.cx = cx;
    t.pos.cy = cy;
    t.fuel = START_STATION_FUEL;
    t.conn_mask = (unsigned)-1;
    t.last_change = frame_loops;
    t.team = team;
    STORE(fuel_t, World.fuel, World.NumFuels, max_fuels, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_base(int cx, int cy, int dir, int team)
{
    base_t t;
    int ind = World.NumBases;

    t.pos.cx = cx;
    t.pos.cy = cy;
    /*
     * The direction of the base should be so that it points
     * up with respect to the gravity in the region.  This
     * is fixed in Find_base_dir() when the gravity has
     * been computed.
     */
    t.dir = dir;
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (team < 0 || team >= MAX_TEAMS)
	    team = 0;
	t.team = team;
	World.teams[team].NumBases++;
	if (World.teams[team].NumBases == 1)
	    World.NumTeamBases++;
    } else {
	t.team = TEAM_NOT_SET;
    }
    STORE(base_t, World.base, World.NumBases, max_bases, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_treasure(int cx, int cy, int team, bool empty)
{
    treasure_t t;
    int ind = World.NumTreasures;

    t.pos.cx = cx;
    t.pos.cy = cy;
    t.have = false;
    t.destroyed = 0;
    t.team = team;
    t.empty = empty;
    if (team != TEAM_NOT_SET) {
	World.teams[team].NumTreasures++;
	World.teams[team].TreasuresLeft++;
    }
    STORE(treasure_t, World.treasures, World.NumTreasures, max_treasures, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_target(int cx, int cy, int team)
{
    target_t t;
    int ind = World.NumTargets;

    t.pos.cx = cx;
    t.pos.cy = cy;
    /*
     * If we have a block based map, the team is determined in
     * in Xpmap_find_map_object_teams().
     */
    t.team = team;
    t.dead_time = 0;
    t.damage = TARGET_DAMAGE;
    t.conn_mask = (unsigned)-1;
    t.update_mask = 0;
    t.last_change = frame_loops;
    t.group = -1;
    STORE(target_t, World.targets, World.NumTargets, max_targets, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_wormhole(int cx, int cy, wormType type)
{
    wormhole_t t;
    int ind = World.NumWormholes;

    t.pos.cx = cx;
    t.pos.cy = cy;
    t.countdown = 0;
    t.lastdest = -1;
    t.temporary = false;
    t.type = type;
    t.lastblock = SPACE;
    t.lastID = -1;
    STORE(wormhole_t, World.wormHoles, World.NumWormholes, max_wormholes, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

/*
 * if 0 <= ind < OLD_MAX_CHECKS, the checkpoint is directly inserted
 * into the check array and it is assumed it has been allocated earlier
 */
int Map_place_check(int cx, int cy, int ind)
{
    clpos t;

    if (ind >= 0 && ind < OLD_MAX_CHECKS) {
	World.check[ind].cx = cx;
	World.check[ind].cy = cy;
	Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
	return ind;
    }

    ind = World.NumChecks;
    t.cx = cx;
    t.cy = cy;
    STORE(clpos, World.check, World.NumChecks, max_checks, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_item_concentrator(int cx, int cy)
{
    item_concentrator_t t;
    int ind = World.NumItemConcentrators;

    t.pos.cx = cx;
    t.pos.cy = cy;
    STORE(item_concentrator_t, World.itemConcentrators,
	  World.NumItemConcentrators, max_itemconcs, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_asteroid_concentrator(int cx, int cy)
{
    asteroid_concentrator_t t;
    int ind = World.NumAsteroidConcs;

    t.pos.cx = cx;
    t.pos.cy = cy;
    STORE(asteroid_concentrator_t, World.asteroidConcs,
	  World.NumAsteroidConcs, max_asteroidconcs, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}

int Map_place_grav(int cx, int cy, DFLOAT force, int type)
{
    grav_t t;
    int ind = World.NumGravs;

    t.pos.cx = cx;
    t.pos.cy = cy;
    t.force = force;
    t.type = type;
    STORE(grav_t, World.grav, World.NumGravs, max_gravs, t);
    Map_set_itemid(CLICK_TO_BLOCK(cx), CLICK_TO_BLOCK(cy), ind);
    return ind;
}







void Free_map(void)
{
    if (World.block) {
	free(World.block);
	World.block = NULL;
    }
    if (World.itemID) {
	free(World.itemID);
	World.itemID = NULL;
    }
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
    if (World.check) {
	free(World.check);
	World.check = NULL;
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
    if (World.asteroidConcs) {
	free(World.asteroidConcs);
	World.asteroidConcs = NULL;
    }
}


void Alloc_map(void)
{
    int x;

    if (World.block || World.gravity)
	Free_map();

    World.block =
	(unsigned char **)malloc(sizeof(unsigned char *)*World.x
				 + World.x*sizeof(unsigned char)*World.y);
    World.itemID =
	(unsigned short **)malloc(sizeof(unsigned short *)*World.x
				 + World.x*sizeof(unsigned short)*World.y);
    World.gravity =
	(vector **)malloc(sizeof(vector *)*World.x
			  + World.x*sizeof(vector)*World.y);
    World.grav = NULL;
    World.base = NULL;
    World.fuel = NULL;
    World.cannon = NULL;
    World.check = NULL;
    World.wormHoles = NULL;
    World.itemConcentrators = NULL;
    World.asteroidConcs = NULL;
    if (World.block == NULL || World.itemID == NULL || World.gravity == NULL) {
	Free_map();
	error("Couldn't allocate memory for map (%d bytes)",
	      World.x * (World.y * (sizeof(unsigned char) + sizeof(vector))
			 + sizeof(vector*)
			 + sizeof(unsigned char*)));
	exit(-1);
    } else {
	unsigned char *map_line;
	unsigned char **map_pointer;
	unsigned short *item_line;
	unsigned short **item_pointer;
	vector *grav_line;
	vector **grav_pointer;

	map_pointer = World.block;
	map_line = (unsigned char*) ((unsigned char**)map_pointer + World.x);
	item_pointer = World.itemID;
	item_line = (unsigned short*) ((unsigned short**)item_pointer + World.x);
	grav_pointer = World.gravity;
	grav_line = (vector*) ((vector**)grav_pointer + World.x);

	for (x=0; x<World.x; x++) {
	    *map_pointer = map_line;
	    map_pointer += 1;
	    map_line += World.y;
	    *item_pointer = item_line;
	    item_pointer += 1;
	    item_line += World.y;
	    *grav_pointer = grav_line;
	    grav_pointer += 1;
	    grav_line += World.y;
	}
    }
}


static void Reset_map_object_counters(void)
{
    int i;

    World.NumCannons = 0;
    World.NumFuels = 0;
    World.NumGravs = 0;
    World.NumWormholes = 0;
    World.NumTreasures = 0;
    World.NumTargets = 0;
    World.NumBases = 0;
    World.NumItemConcentrators = 0;
    World.NumAsteroidConcs = 0;

    for (i = 0; i < MAX_TEAMS; i++) {
	World.teams[i].NumMembers = 0;
	World.teams[i].NumRobots = 0;
	World.teams[i].NumBases = 0;
	World.teams[i].NumTreasures = 0;
	World.teams[i].NumEmptyTreasures = 0;
	World.teams[i].TreasuresDestroyed = 0;
	World.teams[i].TreasuresLeft = 0;
	World.teams[i].score = 0;
	World.teams[i].prev_score = 0;
	World.teams[i].SwapperId = NO_ID;
    }
}


static void Reset_itemid_array(void)
{
    int x, y;
    for (x = 0; x < World.x; x++)
	for (y = 0; y < World.y; y++)
	    Map_set_itemid(x, y, -1);
}


static void Verify_wormhole_consistency(void)
{
    int i;
    int	worm_in = 0,
	worm_out = 0,
	worm_norm = 0;

    /* count wormhole types */
    for (i = 0; i < World.NumWormholes; i++) {
	int type = World.wormHoles[i].type;
	if (type == WORM_NORMAL)
	    worm_norm++;
	else if (type == WORM_IN)
	    worm_in++;
	else if (type == WORM_OUT)
	    worm_out++;
    }	
    
    /*
     * Verify that the wormholes are consistent, i.e. that if
     * we have no 'out' wormholes, make sure that we don't have
     * any 'in' wormholes, and (less critical) if we have no 'in'
     * wormholes, make sure that we don't have any 'out' wormholes.
     */
    if ((worm_norm) ? (worm_norm + worm_out < 2)
	: (worm_in) ? (worm_out < 1)
	: (worm_out > 0)) {
	
	int i;
	
	xpprintf("Inconsistent use of wormholes, removing them.\n");
	for (i = 0; i < World.NumWormholes; i++)
	    Wormhole_remove_from_map(i);
	World.NumWormholes = 0;
    }
    
    if (!wormTime) {
	for (i = 0; i < World.NumWormholes; i++) {
	    int j = (int)(rfrac() * World.NumWormholes);
	    while (World.wormHoles[j].type == WORM_IN)
		j = (int)(rfrac() * World.NumWormholes);
	    World.wormHoles[i].lastdest = j;
	}
    }
}


/*
 * This function can be called after the map options have been read.
 */
static bool Grok_map_size(void)
{
    bool bad = false;
    int w = mapWidth, h = mapHeight;

    if (!is_polygon_map) {
	w *= BLOCK_SZ;
	h *= BLOCK_SZ;
    }

    if (w < BLOCK_SZ) {
	warn("mapWidth too small, minimum is 1 block (%d pixels).\n",
	     BLOCK_SZ);
	bad = true;
    }
    if (w > MAX_MAP_SIZE * BLOCK_SZ) {
	warn("mapWidth too big, maximum is %d blocks (%d pixels).\n",
	    MAX_MAP_SIZE, MAX_MAP_SIZE * BLOCK_SZ);
	bad = true;
    }
    if (h < BLOCK_SZ) {
	warn("mapHeight too small, minimum is 1 block (%d pixels).\n",
	     BLOCK_SZ);
	bad = true;
    }
    if (h > MAX_MAP_SIZE * BLOCK_SZ) {
	warn("mapWidth too big, maximum is %d blocks (%d pixels).\n",
	     MAX_MAP_SIZE, MAX_MAP_SIZE * BLOCK_SZ);
	bad = true;
    }

    if (bad)
	return false;

    /* pixel sizes */
    World.width = w;
    World.height = h;
    if (!is_polygon_map && extraBorder) {
	World.width += 2 * BLOCK_SZ;
	World.height += 2 * BLOCK_SZ;
    }
    World.hypotenuse = (int) LENGTH(World.width, World.height);

    /* click sizes */
    World.cwidth = World.width * CLICK;
    World.cheight = World.height * CLICK;

    /* block sizes */
    World.x = (World.width - 1) / BLOCK_SZ + 1; /* !@# */
    World.y = (World.height - 1) / BLOCK_SZ + 1;
    World.diagonal = (int) LENGTH(World.x, World.y);

    return true;
}


bool Grok_map(void)
{
    if (!is_polygon_map) {
	if (!Grok_map_options())
	    return false;

	Xpmap_grok_map_data();
	Xpmap_allocate_checks();
	Reset_map_object_counters();
	Reset_itemid_array();
	Xpmap_tags_to_internal_data(true);
	Xpmap_find_map_object_teams();
    }

    Verify_wormhole_consistency();
    
    if (BIT(World.rules->mode, TIMING) && World.NumChecks == 0) {
	xpprintf("No checkpoints found while race mode (timing) was set.\n");
	xpprintf("Turning off race mode.\n");
	CLR_BIT(World.rules->mode, TIMING);
    }

    if (maxRobots == -1) {
	maxRobots = World.NumBases;
    }
    if (minRobots == -1) {
	minRobots = maxRobots;
    }
    if (BIT(World.rules->mode, TIMING)) {
	Find_base_order();
    }

    Realloc_map_objects();

    if (World.NumBases <= 0) {
	warn("WARNING: map has no bases!");
	return false;
    }

#ifndef	SILENT
    xpprintf("World....: %s\nBases....: %d\nMapsize..: %dx%d pixels\n"
	     "Team play: %s\n", World.name, World.NumBases, World.width,
	     World.height, BIT(World.rules->mode, TEAM_PLAY) ? "on" : "off");
#endif

    return true;
}

bool Grok_map_options(void)
{
    Reset_map_object_counters();

#if 0
    if (!Grok_map_size()) {
	if (mapData != NULL) {
	    free(mapData);
	    mapData = NULL;
	}
    }

    if (!mapData) {
	errno = 0;
	error("Generating random map");
	Generate_random_map();
	if (!mapData) {
	    return false;
	}
    }
#else
    if (!Grok_map_size())
	return false;
#endif

    strlcpy(World.name, mapName, sizeof(World.name));
    strlcpy(World.author, mapAuthor, sizeof(World.author));
    strlcpy(World.dataURL, dataURL, sizeof(World.dataURL));

    Alloc_map();

    Set_world_rules();
    Set_world_items();
    Set_world_asteroids();

    if (BIT(World.rules->mode, TEAM_PLAY|TIMING) == (TEAM_PLAY|TIMING)) {
	error("Cannot teamplay while in race mode -- ignoring teamplay");
	CLR_BIT(World.rules->mode, TEAM_PLAY);
    }

    return true;
}


/*
 * Use wildmap to generate a random map.
 */
/* kps - we need a poly random map generator */

static void Generate_random_map(void)
{
    edgeWrap = true;
    mapWidth = 150;
    mapHeight = 150;

    Wildmap(mapWidth, mapHeight, World.name, World.author, &mapData,
	    &mapWidth, &mapHeight);
    Grok_map_size();
}


/*
 * Return the team that is closest to this click position.
 */
unsigned short Find_closest_team(int cx, int cy)
{
    unsigned short team = TEAM_NOT_SET;
    int i;
    DFLOAT closest = FLT_MAX, l;

    for (i = 0; i < World.NumBases; i++) {
	if (World.base[i].team == TEAM_NOT_SET)
	    continue;

	l = Wrap_length((cx - World.base[i].pos.cx),
			(cy - World.base[i].pos.cy));

	if (l < closest) {
	    team = World.base[i].team;
	    closest = l;
	}
    }

    return team;
}


void Find_base_direction(void)
{
    /* kps - this might go wrong if we run in -polygonMode ? */
    if (!is_polygon_map)
	Xpmap_find_base_direction();
}

/*
 * Determine the order in which players are placed
 * on starting positions after race mode reset.
 */
/* kps - ng does not want this */
static void Find_base_order(void)
{
    int			i, j, k, n;
    DFLOAT		dist;
    int			cx, cy;

    if (!BIT(World.rules->mode, TIMING)) {
	World.baseorder = NULL;
	return;
    }
    if ((n = World.NumBases) <= 0) {
	error("Cannot support race mode in a map without bases");
	exit(-1);
    }

    if ((World.baseorder = (baseorder_t *)
	    malloc(n * sizeof(baseorder_t))) == NULL) {
	error("Out of memory - baseorder");
	exit(-1);
    }

    cx = World.check[0].cx;
    cy = World.check[0].cy;
    for (i = 0; i < n; i++) {
	dist = Wrap_length(World.base[i].pos.cx - cx,
			   World.base[i].pos.cy - cy) / CLICK;
	for (j = 0; j < i; j++) {
	    if (World.baseorder[j].dist > dist) {
		break;
	    }
	}
	for (k = i - 1; k >= j; k--) {
	    World.baseorder[k + 1] = World.baseorder[k];
	}
	World.baseorder[j].base_idx = i;
	World.baseorder[j].dist = dist;
    }
}


DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}

DFLOAT Wrap_cfindDir(int dcx, int dcy)
{
    dcx = WRAP_DCX(dcx);
    dcy = WRAP_DCY(dcy);
    return findDir(dcx, dcy);
}

DFLOAT Wrap_length(int dcx, int dcy)
{
    dcx = WRAP_DCX(dcx);
    dcy = WRAP_DCY(dcy);
    return LENGTH(dcx, dcy);
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
	gx = CLICK_TO_BLOCK(World.grav[g].pos.cx);
	gy = CLICK_TO_BLOCK(World.grav[g].pos.cy);
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

	gtype = World.grav[g].type;

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


void add_temp_wormholes(int xin, int yin, int xout, int yout)
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

    inhole.pos.cx = BLOCK_CENTER(xin);
    inhole.pos.cy = BLOCK_CENTER(yin);
    inhole.countdown = wormTime;
    inhole.lastdest = World.NumWormholes + 1;
    inhole.temporary = true;
    inhole.type = WORM_IN;
    inhole.lastblock = World.block[xin][yin];
    inhole.lastID = Map_get_itemid(xin, yin);
    World.wormHoles[World.NumWormholes] = inhole;
    World.block[xin][yin] = WORMHOLE;
    Map_set_itemid(xin, yin, World.NumWormholes);

    outhole.pos.cx = BLOCK_CENTER(xout);
    outhole.pos.cy = BLOCK_CENTER(yout);
    outhole.countdown = wormTime;
    outhole.temporary = true;
    outhole.type = WORM_OUT;
    outhole.lastblock = World.block[xout][yout];
    outhole.lastID = Map_get_itemid(xout, yout);
    World.wormHoles[World.NumWormholes + 1] = outhole;
    World.block[xout][yout] = WORMHOLE;
    Map_set_itemid(xout, yout, World.NumWormholes + 1);

    World.NumWormholes += 2;
}


void remove_temp_wormhole(int ind)
{
    wormhole_t hole;
    int bx, by;

    hole = World.wormHoles[ind];
    bx = CLICK_TO_BLOCK(hole.pos.cx);
    by = CLICK_TO_BLOCK(hole.pos.cy);
    World.block[bx][by] = hole.lastblock;
    Map_set_itemid(bx, by, hole.lastID);
    World.NumWormholes--;
    if (ind != World.NumWormholes) {
	World.wormHoles[ind] = World.wormHoles[World.NumWormholes];
    }
    World.wormHoles = (wormhole_t *)realloc(World.wormHoles,
					    World.NumWormholes
					    * sizeof(wormhole_t));
}


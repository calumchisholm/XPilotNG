/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 2000-2004 by
 *
 *      Uoti Urpala          <uau@users.sourceforge.net>
 *      Kristian S�derblom   <kps@users.sourceforge.net>
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpserver.h"

char map_version[] = VERSION;

#define GRAV_RANGE  10

/*
 * Globals.
 */
world_t World;
bool is_polygon_map = false;

/*static void Generate_random_map(void);*/
static void Find_base_direction(world_t *world);
static void Find_base_order(world_t *world);
static void Reset_map_object_counters(world_t *world);


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


static void Realloc_map_objects(world_t *world)
{
    SHRINK(cannon_t, world->cannons, world->NumCannons, world->MaxCannons);
    SHRINK(fuel_t, world->fuels, world->NumFuels, world->MaxFuels);
    SHRINK(grav_t, world->gravs, world->NumGravs, world->MaxGravs);
    SHRINK(wormhole_t, world->wormholes,
	   world->NumWormholes, world->MaxWormholes);
    SHRINK(treasure_t, world->treasures,
	   world->NumTreasures, world->MaxTreasures);
    SHRINK(target_t, world->targets, world->NumTargets, world->MaxTargets);
    SHRINK(base_t, world->bases, world->NumBases, world->MaxBases);
    SHRINK(item_concentrator_t, world->itemConcs,
	   world->NumItemConcs, world->MaxItemConcs);
    SHRINK(asteroid_concentrator_t, world->asteroidConcs,
	   world->NumAsteroidConcs, world->MaxAsteroidConcs);
    SHRINK(friction_area_t, world->frictionAreas,
	   world->NumFrictionAreas, world->MaxFrictionAreas);
}

int World_place_cannon(world_t *world, clpos_t pos, int dir, int team)
{
    cannon_t t, *cannon;
    int ind = world->NumCannons;

    t.world = world;
    t.pos = pos;
    t.dir = dir;
    t.team = team;
    t.dead_ticks = 0;
    t.conn_mask = (unsigned)-1;
    t.group = NO_GROUP;
    STORE(cannon_t, world->cannons, world->NumCannons, world->MaxCannons, t);
    cannon = Cannons(world, ind);
    Cannon_init(cannon);
    return ind;
}

int World_place_fuel(world_t *world, clpos_t pos, int team)
{
    fuel_t t;
    int ind = world->NumFuels;

    t.pos = pos;
    t.fuel = START_STATION_FUEL;
    t.conn_mask = (unsigned)-1;
    t.last_change = frame_loops;
    t.team = team;
    STORE(fuel_t, world->fuels, world->NumFuels, world->MaxFuels, t);
    return ind;
}

int World_place_base(world_t *world, clpos_t pos, int dir, int team)
{
    base_t t;
    int ind = world->NumBases;

    t.pos = pos;
    /*
     * The direction of the base should be so that it points
     * up with respect to the gravity in the region.  This
     * is fixed in Find_base_dir() when the gravity has
     * been computed.
     */
    t.dir = dir;
    if (BIT(world->rules->mode, TEAM_PLAY)) {
	if (team < 0 || team >= MAX_TEAMS)
	    team = 0;
	t.team = team;
	world->teams[team].NumBases++;
	if (world->teams[team].NumBases == 1)
	    world->NumTeamBases++;
    } else
	t.team = TEAM_NOT_SET;
    t.ind = world->NumBases;
    STORE(base_t, world->bases, world->NumBases, world->MaxBases, t);
    return ind;
}

int World_place_treasure(world_t *world, clpos_t pos, int team, bool empty,
			 int ball_style)
{
    treasure_t t;
    int ind = world->NumTreasures;

    t.pos = pos;
    t.have = false;
    t.destroyed = 0;
    t.team = team;
    t.empty = empty;
    t.ball_style = ball_style;
    if (team != TEAM_NOT_SET) {
	world->teams[team].NumTreasures++;
	world->teams[team].TreasuresLeft++;
    }
    STORE(treasure_t, world->treasures, world->NumTreasures,
	  world->MaxTreasures, t);
    return ind;
}

int World_place_target(world_t *world, clpos_t pos, int team)
{
    target_t t;
    int ind = world->NumTargets;

    t.pos = pos;
    /*
     * If we have a block based map, the team is determined in
     * in Xpmap_find_map_object_teams().
     */
    t.team = team;
    t.dead_ticks = 0;
    t.damage = TARGET_DAMAGE;
    t.conn_mask = (unsigned)-1;
    t.update_mask = 0;
    t.last_change = frame_loops;
    t.group = NO_GROUP;
    STORE(target_t, world->targets, world->NumTargets, world->MaxTargets, t);
    return ind;
}

int World_place_wormhole(world_t *world, clpos_t pos, wormType type)
{
    wormhole_t t;
    int ind = world->NumWormholes;

    t.pos = pos;
    t.countdown = 0;
    t.lastdest = NO_IND;
    t.temporary = false;
    t.type = type;
    t.lastblock = SPACE;
    t.lastID = NO_ID;
    t.group = NO_GROUP;
    STORE(wormhole_t, world->wormholes,
	  world->NumWormholes, world->MaxWormholes, t);
    return ind;
}


/*
 * Allocate checkpoints for an xp map.
 */
static void alloc_old_checks(world_t *world)
{
    int i;
    check_t t;
    clpos_t pos = { -1, -1 };

    t.pos = pos;

    for (i = 0; i < OLD_MAX_CHECKS; i++)
	STORE(check_t, world->checks, world->NumChecks, world->MaxChecks, t);

    SHRINK(check_t, world->checks, world->NumChecks, world->MaxChecks);
    world->NumChecks = 0;
}

int World_place_check(world_t *world, clpos_t pos, int ind)
{
    check_t t;

    if (!BIT(world->rules->mode, TIMING)) {
	warn("Checkpoint on map with no timing.");
	return NO_IND;
    }

    /* kps - need to do this for other map object types ? */
    if (!World_contains_clpos(world, pos)) {
	warn("Checkpoint outside world, ignoring.");
	return NO_IND;
    }

    /*
     * On xp maps we can have only 26 checkpoints.
     */
    if (ind >= 0 && ind < OLD_MAX_CHECKS) {
	check_t *check;

	if (world->NumChecks == 0)
	    alloc_old_checks(world);

	check = Checks(world, ind);

	if (World_contains_clpos(world, check->pos)) {
	    warn("Map contains too many '%c' checkpoints.", 'A' + ind);
	    return NO_IND;
	}

	check->pos = pos;
	world->NumChecks++;
	return ind;
    }

    ind = world->NumChecks;
    t.pos = pos;
    STORE(check_t, world->checks, world->NumChecks, world->MaxChecks, t);
    return ind;
}

int World_place_item_concentrator(world_t *world, clpos_t pos)
{
    item_concentrator_t t;
    int ind = world->NumItemConcs;

    t.pos = pos;
    STORE(item_concentrator_t, world->itemConcs,
	  world->NumItemConcs, world->MaxItemConcs, t);
    return ind;
}

int World_place_asteroid_concentrator(world_t *world, clpos_t pos)
{
    asteroid_concentrator_t t;
    int ind = world->NumAsteroidConcs;

    t.pos = pos;
    STORE(asteroid_concentrator_t, world->asteroidConcs,
	  world->NumAsteroidConcs, world->MaxAsteroidConcs, t);
    return ind;
}

int World_place_grav(world_t *world, clpos_t pos, double force, int type)
{
    grav_t t;
    int ind = world->NumGravs;

    t.pos = pos;
    t.force = force;
    t.type = type;
    STORE(grav_t, world->gravs, world->NumGravs, world->MaxGravs, t);
    return ind;
}

int World_place_friction_area(world_t *world, clpos_t pos, double fric)
{
    friction_area_t t;
    int ind = world->NumFrictionAreas;

    t.pos = pos;
    t.friction_setting = fric;
    /*t.friction = ... ; handled in timing setup */
    STORE(friction_area_t, world->frictionAreas,
	  world->NumFrictionAreas, world->MaxFrictionAreas, t);
    return ind;
}

void World_free(world_t *world)
{
    XFREE(world->block);
    XFREE(world->gravity);
    XFREE(world->gravs);
    XFREE(world->bases);
    XFREE(world->cannons);
    XFREE(world->checks);
    XFREE(world->fuels);
    XFREE(world->wormholes);
    XFREE(world->itemConcs);
    XFREE(world->asteroidConcs);
    XFREE(world->frictionAreas);
}

static void World_alloc(world_t *world)
{
    int x;

    if (world->block || world->gravity)
	World_free(world);

    world->block =
	(unsigned char **)malloc(sizeof(unsigned char *)*world->x
				 + world->x*sizeof(unsigned char)*world->y);
    world->gravity =
	(vector_t **)malloc(sizeof(vector_t *)*world->x
			  + world->x*sizeof(vector_t)*world->y);
    world->gravs = NULL;
    world->bases = NULL;
    world->fuels = NULL;
    world->cannons = NULL;
    world->checks = NULL;
    world->wormholes = NULL;
    world->itemConcs = NULL;
    world->asteroidConcs = NULL;
    if (world->block == NULL || world->gravity == NULL) {
	World_free(world);
	fatal("Couldn't allocate memory for map");
    } else {
	unsigned char *map_line;
	unsigned char **map_pointer;
	vector_t *grav_line;
	vector_t **grav_pointer;

	map_pointer = world->block;
	map_line = (unsigned char*) ((unsigned char**)map_pointer + world->x);

	grav_pointer = world->gravity;
	grav_line = (vector_t*) ((vector_t**)grav_pointer + world->x);

	for (x=0; x<world->x; x++) {
	    *map_pointer = map_line;
	    map_pointer += 1;
	    map_line += world->y;
	    *grav_pointer = grav_line;
	    grav_pointer += 1;
	    grav_line += world->y;
	}
    }
}


static void Reset_map_object_counters(world_t *world)
{
    int i;

    world->NumCannons = 0;
    world->NumFuels = 0;
    world->NumGravs = 0;
    world->NumWormholes = 0;
    world->NumTreasures = 0;
    world->NumTargets = 0;
    world->NumBases = 0;
    world->NumItemConcs = 0;
    world->NumAsteroidConcs = 0;

    for (i = 0; i < MAX_TEAMS; i++) {
	world->teams[i].NumMembers = 0;
	world->teams[i].NumRobots = 0;
	world->teams[i].NumBases = 0;
	world->teams[i].NumTreasures = 0;
	world->teams[i].NumEmptyTreasures = 0;
	world->teams[i].TreasuresDestroyed = 0;
	world->teams[i].TreasuresLeft = 0;
	world->teams[i].score = 0;
	world->teams[i].prev_score = 0;
	world->teams[i].SwapperId = NO_ID;
    }
}


static void Verify_wormhole_consistency(world_t *world)
{
    int i;
    int	worm_in = 0,
	worm_out = 0,
	worm_norm = 0;

    /* count wormhole types */
    for (i = 0; i < world->NumWormholes; i++) {
	int type = world->wormholes[i].type;
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

	xpprintf("Inconsistent use of wormholes, removing them.\n");
	for (i = 0; i < world->NumWormholes; i++)
	    World_remove_wormhole(world, Wormholes(world, i));
	world->NumWormholes = 0;
    }

    if (!options.wormholeStableTicks) {
	for (i = 0; i < world->NumWormholes; i++) {
	    int j = (int)(rfrac() * world->NumWormholes);

	    while (Wormholes(world, j)->type == WORM_IN)
		j = (int)(rfrac() * world->NumWormholes);
	    Wormholes(world, i)->lastdest = j;
	}
    }
}


/*
 * This function can be called after the map options have been read.
 */
static bool Grok_map_size(world_t *world)
{
    bool bad = false;
    int w = options.mapWidth, h = options.mapHeight;

    if (!is_polygon_map) {
	w *= BLOCK_SZ;
	h *= BLOCK_SZ;
    }

    if (w < MIN_MAP_SIZE) {
	warn("mapWidth too small, minimum is %d pixels (%d blocks).\n",
	     MIN_MAP_SIZE, MIN_MAP_SIZE / BLOCK_SZ + 1);
	bad = true;
    }
    if (w > MAX_MAP_SIZE) {
	warn("mapWidth too big, maximum is %d pixels (%d blocks).\n",
	     MAX_MAP_SIZE, MAX_MAP_SIZE / BLOCK_SZ);
	bad = true;
    }
    if (h < MIN_MAP_SIZE) {
	warn("mapHeight too small, minimum is %d pixels (%d blocks).\n",
	     MIN_MAP_SIZE, MIN_MAP_SIZE / BLOCK_SZ + 1);
	bad = true;
    }
    if (h > MAX_MAP_SIZE) {
	warn("mapWidth too big, maximum is %d pixels (%d blocks).\n",
	     MAX_MAP_SIZE, MAX_MAP_SIZE / BLOCK_SZ);
	bad = true;
    }

    if (bad)
	exit(1);

    /* pixel sizes */
    world->width = w;
    world->height = h;
    if (!is_polygon_map && options.extraBorder) {
	world->width += 2 * BLOCK_SZ;
	world->height += 2 * BLOCK_SZ;
    }
    world->hypotenuse = LENGTH(world->width, world->height);

    /* click sizes */
    world->cwidth = world->width * CLICK;
    world->cheight = world->height * CLICK;

    /* block sizes */
    world->x = (world->width - 1) / BLOCK_SZ + 1; /* !@# */
    world->y = (world->height - 1) / BLOCK_SZ + 1;
    world->diagonal = LENGTH(world->x, world->y);

    return true;
}


bool Grok_map(world_t *world)
{
    if (!is_polygon_map) {
	if (!Grok_map_options(world))
	    exit(1);

	Xpmap_grok_map_data(world, options.mapData);
	XFREE(options.mapData);
	Xpmap_tags_to_internal_data(world, true);
	Xpmap_find_map_object_teams(world);
    }

    Verify_wormhole_consistency(world);
    Wormhole_line_init();

    if (BIT(world->rules->mode, TIMING) && world->NumChecks == 0) {
	xpprintf("No checkpoints found while race mode (timing) was set.\n");
	xpprintf("Turning off race mode.\n");
	CLR_BIT(world->rules->mode, TIMING);
    }

    /* kps - what are these doing here ? */
    if (options.maxRobots == -1)
	options.maxRobots = world->NumBases;

    if (options.minRobots == -1)
	options.minRobots = options.maxRobots;

    if (BIT(world->rules->mode, TIMING))
	Find_base_order(world);

    Realloc_map_objects(world);

    if (world->NumBases <= 0) {
	warn("WARNING: map has no bases!");
	exit(1);
    }

    if (!options.silent)
	xpprintf("World....: %s\nBases....: %d\nMapsize..: %dx%d pixels\n"
		 "Team play: %s\n",
		 world->name, world->NumBases, world->width, world->height,
		 BIT(world->rules->mode, TEAM_PLAY) ? "on" : "off");

    if (!is_polygon_map)
	Xpmap_blocks_to_polygons(world);

    Compute_gravity(world);
    Find_base_direction(world);

    return true;
}

bool Grok_map_options(world_t *world)
{
    Reset_map_object_counters(world);

    if (!Grok_map_size(world))
	return false;

    strlcpy(world->name, options.mapName, sizeof(world->name));
    strlcpy(world->author, options.mapAuthor, sizeof(world->author));
    strlcpy(world->dataURL, options.dataURL, sizeof(world->dataURL));

    World_alloc(world);

    Set_world_rules(world);
    Set_world_items(world);
    Set_world_asteroids(world);

    if (BIT(world->rules->mode, TEAM_PLAY|TIMING) == (TEAM_PLAY|TIMING)) {
	warn("Cannot teamplay while in race mode -- ignoring teamplay");
	CLR_BIT(world->rules->mode, TEAM_PLAY);
    }

    return true;
}


/*
 * Return the team that is closest to this click position.
 */
int Find_closest_team(world_t *world, clpos_t pos)
{
    int team = TEAM_NOT_SET, i;
    double closest = FLT_MAX, l;

    for (i = 0; i < world->NumBases; i++) {
	base_t *base = Bases(world, i);
	if (base->team == TEAM_NOT_SET)
	    continue;

	l = Wrap_length(pos.cx - base->pos.cx, pos.cy - base->pos.cy);
	if (l < closest) {
	    team = base->team;
	    closest = l;
	}
    }

    return team;
}


static void Find_base_direction(world_t *world)
{
    /* kps - this might go wrong if we run in -options.polygonMode ? */
    if (!is_polygon_map)
	Xpmap_find_base_direction(world);
}

/*
 * Determine the order in which players are placed
 * on starting positions after race mode reset.
 */
static void Find_base_order(world_t *world)
{
    int			i, j, k, n;
    double		dist;
    clpos_t		chkpos;

    if (!BIT(world->rules->mode, TIMING)) {
	world->baseorder = NULL;
	return;
    }
    if ((n = world->NumBases) <= 0) {
	warn("Cannot support race mode in a map without bases");
	exit(-1);
    }

    if ((world->baseorder = malloc(n * sizeof(baseorder_t))) == NULL) {
	error("Out of memory - baseorder");
	exit(-1);
    }

    chkpos = Checks(world, 0)->pos;
    for (i = 0; i < n; i++) {
	clpos_t bpos = Bases(world, i)->pos;
	dist = Wrap_length(bpos.cx - chkpos.cx, bpos.cy - chkpos.cy) / CLICK;
	for (j = 0; j < i; j++) {
	    if (world->baseorder[j].dist > dist)
		break;
	}
	for (k = i - 1; k >= j; k--)
	    world->baseorder[k + 1] = world->baseorder[k];

	world->baseorder[j].base_idx = i;
	world->baseorder[j].dist = dist;
    }
}


double Wrap_findDir(double dx, double dy)
{
    dx = WRAP_DX(dx);
    dy = WRAP_DY(dy);
    return findDir(dx, dy);
}

double Wrap_cfindDir(int dcx, int dcy)
{
    dcx = WRAP_DCX(dcx);
    dcy = WRAP_DCY(dcy);
    return findDir((double)dcx, (double)dcy);
}

double Wrap_length(int dcx, int dcy)
{
    dcx = WRAP_DCX(dcx);
    dcy = WRAP_DCY(dcy);
    return LENGTH(dcx, dcy);
}


static void Compute_global_gravity(world_t *world)
{
    int			xi, yi, dx, dy;
    double		xforce, yforce, strength;
    double		theta;
    vector_t		*grav;


    if (options.gravityPointSource == false) {
	theta = (options.gravityAngle * PI) / 180.0;
	xforce = cos(theta) * options.gravity;
	yforce = sin(theta) * options.gravity;
	for (xi = 0; xi < world->x; xi++) {
	    grav = world->gravity[xi];

	    for (yi = 0; yi < world->y; yi++, grav++) {
		grav->x = xforce;
		grav->y = yforce;
	    }
	}
    } else {
	for (xi = 0; xi < world->x; xi++) {
	    grav = world->gravity[xi];
	    dx = (xi - options.gravityPoint.x) * BLOCK_SZ;
	    dx = WRAP_DX(dx);

	    for (yi = 0; yi < world->y; yi++, grav++) {
		dy = (yi - options.gravityPoint.y) * BLOCK_SZ;
		dy = WRAP_DX(dy);

		if (dx == 0 && dy == 0) {
		    grav->x = 0.0;
		    grav->y = 0.0;
		    continue;
		}
		strength = options.gravity / LENGTH(dx, dy);
		if (options.gravityClockwise) {
		    grav->x =  dy * strength;
		    grav->y = -dx * strength;
		}
		else if (options.gravityAnticlockwise) {
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


static void Compute_grav_tab(vector_t grav_tab[GRAV_RANGE+1][GRAV_RANGE+1])
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


static void Compute_local_gravity(world_t *world)
{
    int xi, yi, g, gx, gy, ax, ay, dx, dy, gtype;
    int first_xi, last_xi, first_yi, last_yi, mod_xi, mod_yi;
    int min_xi, max_xi, min_yi, max_yi;
    double force, fx, fy;
    vector_t *v, *grav, *tab, grav_tab[GRAV_RANGE+1][GRAV_RANGE+1];

    Compute_grav_tab(grav_tab);

    min_xi = 0;
    max_xi = world->x - 1;
    min_yi = 0;
    max_yi = world->y - 1;
    if (BIT(world->rules->mode, WRAP_PLAY)) {
	min_xi -= MIN(GRAV_RANGE, world->x);
	max_xi += MIN(GRAV_RANGE, world->x);
	min_yi -= MIN(GRAV_RANGE, world->y);
	max_yi += MIN(GRAV_RANGE, world->y);
    }
    for (g = 0; g < world->NumGravs; g++) {
	gx = CLICK_TO_BLOCK(world->gravs[g].pos.cx);
	gy = CLICK_TO_BLOCK(world->gravs[g].pos.cy);
	force = world->gravs[g].force;

	if ((first_xi = gx - GRAV_RANGE) < min_xi)
	    first_xi = min_xi;
	if ((last_xi = gx + GRAV_RANGE) > max_xi)
	    last_xi = max_xi;
	if ((first_yi = gy - GRAV_RANGE) < min_yi)
	    first_yi = min_yi;
	if ((last_yi = gy + GRAV_RANGE) > max_yi)
	    last_yi = max_yi;

	gtype = world->gravs[g].type;

	mod_xi = (first_xi < 0) ? (first_xi + world->x) : first_xi;
	dx = gx - first_xi;
	fx = force;
	for (xi = first_xi; xi <= last_xi; xi++, dx--) {
	    if (dx < 0) {
		fx = -force;
		ax = -dx;
	    } else
		ax = dx;

	    mod_yi = (first_yi < 0) ? (first_yi + world->y) : first_yi;
	    dy = gy - first_yi;
	    grav = &world->gravity[mod_xi][mod_yi];
	    tab = grav_tab[ax];
	    fy = force;
	    for (yi = first_yi; yi <= last_yi; yi++, dy--) {
		if (dx || dy) {
		    if (dy < 0) {
			fy = -force;
			ay = -dy;
		    } else
			ay = dy;

		    v = &tab[ay];
		    if (gtype == CWISE_GRAV || gtype == ACWISE_GRAV) {
			grav->x -= fy * v->y;
			grav->y += fx * v->x;
		    } else if (gtype == UP_GRAV || gtype == DOWN_GRAV)
			grav->y += force * v->x;
		    else if (gtype == RIGHT_GRAV || gtype == LEFT_GRAV)
			grav->x += force * v->y;
		    else {
			grav->x += fx * v->x;
			grav->y += fy * v->y;
		    }
		}
		else {
		    if (gtype == UP_GRAV || gtype == DOWN_GRAV)
			grav->y += force;
		    else if (gtype == LEFT_GRAV || gtype == RIGHT_GRAV)
			grav->x += force;
		}
		mod_yi++;
		grav++;
		if (mod_yi >= world->y) {
		    mod_yi = 0;
		    grav = world->gravity[mod_xi];
		}
	    }
	    if (++mod_xi >= world->x)
		mod_xi = 0;
	}
    }
    /*
     * We may want to free the world->gravity memory here
     * as it is not used anywhere else.
     * e.g.: free(world->gravity);
     *       world->gravity = NULL;
     *       world->NumGravs = 0;
     * Some of the more modern maps have quite a few gravity symbols.
     */
}


void Compute_gravity(world_t *world)
{
    Compute_global_gravity(world);
    Compute_local_gravity(world);
}

#define WORMHOLE_RADIUS ((BLOCK_CLICKS / 2) - 1)
shape_t		wormhole_wire;

void Wormhole_line_init(void)
{
    int i;
    static shapepos_t coords[MAX_SHIP_PTS];

    wormhole_wire.num_points = MAX_SHIP_PTS;
    for (i = 0; i < MAX_SHIP_PTS; i++) {
	wormhole_wire.pts[i] = coords + i;
	coords[i].clk.cx = cos(i * 2 * PI / MAX_SHIP_PTS) * WORMHOLE_RADIUS;
	coords[i].clk.cy = sin(i * 2 * PI / MAX_SHIP_PTS) * WORMHOLE_RADIUS;
    }

    return;
}

void add_temp_wormholes(world_t *world, int xin, int yin, int xout, int yout)
{
    wormhole_t inhole, outhole, *wwhtemp;

    if ((wwhtemp = realloc(world->wormholes,
			   (world->NumWormholes + 2) * sizeof(wormhole_t)))
	== NULL) {
	error("No memory for temporary wormholes.");
	return;
    }
    world->wormholes = wwhtemp;

    inhole.pos.cx = BLOCK_CENTER(xin);
    inhole.pos.cy = BLOCK_CENTER(yin);
    inhole.countdown = options.wormholeStableTicks;
    inhole.lastdest = world->NumWormholes + 1;
    inhole.temporary = true;
    inhole.type = WORM_IN;
    inhole.lastblock = world->block[xin][yin];
    /*inhole.lastID = Map_get_itemid(xin, yin);*/
    world->wormholes[world->NumWormholes] = inhole;
    world->block[xin][yin] = WORMHOLE;
    /*Map_set_itemid(xin, yin, world->NumWormholes);*/

    outhole.pos.cx = BLOCK_CENTER(xout);
    outhole.pos.cy = BLOCK_CENTER(yout);
    outhole.countdown = options.wormholeStableTicks;
    outhole.temporary = true;
    outhole.type = WORM_OUT;
    outhole.lastblock = world->block[xout][yout];
    /*outhole.lastID = Map_get_itemid(xout, yout);*/
    world->wormholes[world->NumWormholes + 1] = outhole;
    world->block[xout][yout] = WORMHOLE;
    /*Map_set_itemid(xout, yout, world->NumWormholes + 1);*/

    world->NumWormholes += 2;
}


void remove_temp_wormhole(world_t *world, int ind)
{
    World_remove_wormhole(world, Wormholes(world, ind));

    world->NumWormholes--;
    if (ind != world->NumWormholes)
	world->wormholes[ind] = world->wormholes[world->NumWormholes];

    world->wormholes = realloc(world->wormholes,
			      world->NumWormholes * sizeof(wormhole_t));
}

void World_add_temporary_wormholes(world_t *world, clpos_t pos1, clpos_t pos2)
{

#if 0

#if 0 /* kps - temporary wormholes disabled currently */
    if (counter
	&& options.wormTime
	&& BIT(1U << world->block[OBJ_X_IN_BLOCKS(pl)]
	       [OBJ_Y_IN_BLOCKS(pl)],
	       SPACE_BIT)
	&& BIT(1U << world->block[CLICK_TO_BLOCK(dest.cx)]
	       [CLICK_TO_BLOCK(dest.cy)],
	       SPACE_BIT))
	add_temp_wormholes(OBJ_X_IN_BLOCKS(pl),
			   OBJ_Y_IN_BLOCKS(pl),
			   CLICK_TO_BLOCK(dest.cx),
			   CLICK_TO_BLOCK(dest.cy));
#endif


    if (is_polygon_map) {
	;
    } else {
	blpos blk1, blk2;
	int type1, type2;

	blk1 = Clpos_to_blkpos(pos1);
	type1 = World_get_block(blk1);

	blk2 = Clpos_to_blkpos(pos2);
	type2 = World_get_block(blk2);

	if (!(type1 == SPACE && type2 == SPACE)) {
	    warn("World_add_temporary_wormholes: could not add tmp wormholes: "
		 "type1 = %d, type2 = %d", type1, type2);
	    return;
	}
    }
#endif
}

/* 
 * XPilot NG, a multiplayer space war game.
 *
 * Copyright (C) 2000-2004 by
 *
 *      Uoti Urpala          <uau@users.sourceforge.net>
 *      Kristian Söderblom   <kps@users.sourceforge.net>
 *
 * Copyright (C) 1991-2001 by
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	MAP_H
#define	MAP_H

#ifndef TYPES_H
# include "types.h"
#endif
#ifndef RULES_H
# include "rules.h"
#endif
#ifndef ITEM_H
# include "item.h"
#endif

#define SPACE			0
#define BASE			1
#define FILLED			2
#define REC_LU			3
#define REC_LD			4
#define REC_RU			5
#define REC_RD			6
#define FUEL			7
#define CANNON			8
#define CHECK			9
#define POS_GRAV		10
#define NEG_GRAV		11
#define CWISE_GRAV		12
#define ACWISE_GRAV		13
#define WORMHOLE		14
#define TREASURE		15
#define TARGET			16
#define ITEM_CONCENTRATOR	17
#define DECOR_FILLED		18
#define DECOR_LU		19
#define DECOR_LD		20
#define DECOR_RU		21
#define DECOR_RD		22
#define UP_GRAV			23
#define DOWN_GRAV		24
#define RIGHT_GRAV		25
#define LEFT_GRAV		26
#define FRICTION		27
#define ASTEROID_CONCENTRATOR	28
#define BASE_ATTRACTOR		127

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)

typedef struct world world_t;

typedef struct fuel {
    clpos_t	pos;
    double	fuel;
    unsigned	conn_mask;
    long	last_change;
    int		team;
} fuel_t;

typedef struct grav {
    clpos_t	pos;
    double	force;
    int		type;
} grav_t;

typedef struct base {
    clpos_t	pos;
    int		dir;
    int		ind;
    int		team;
    int		initial_items[NUM_ITEMS];
} base_t;

typedef struct cannon {
    world_t	*world;
    clpos_t	pos;
    int		dir;
    unsigned	conn_mask;
    long	last_change;
    int		item[NUM_ITEMS];
    int		tractor_target_id;
    bool	tractor_is_pressor;
    int		team;
    long	used;
    double	dead_ticks;
    double	damaged;
    double	tractor_count;
    double	emergency_shield_left;
    double	phasing_left;
    int		group;
    double	score;
    short	id;
    short	smartness;
    float	shot_speed;
    int		initial_items[NUM_ITEMS];
} cannon_t;

typedef struct check {
    clpos_t	pos;
} check_t;

typedef struct item {
    double	prob;		/* Probability [0..1] for item to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for this item to appear */
    int		min_per_pack;	/* minimum number of elements per item. */
    int		max_per_pack;	/* maximum number of elements per item. */
    int		initial;	/* initial number of elements per player. */
    int		cannon_initial;	/* initial number of elements per cannon. */
    int		limit;		/* max number of elements per player/cannon. */
} item_t;

typedef struct asteroid {
    double	prob;		/* Probability [0..1] for asteroid to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for asteroid to appear */
} asteroid_t;

typedef enum {
    WORM_NORMAL,
    WORM_IN,
    WORM_OUT,
    WORM_FIXED
} wormtype_t;

typedef struct wormhole {
    clpos_t	pos;
    int		lastdest;	/* last destination wormhole */
    double	countdown;	/* >0 warp to lastdest else random */
    wormtype_t	type;
    int		lastID;
    int		lastblock;	/* block it occluded */
    int		group;
} wormhole_t;

typedef struct treasure {
    clpos_t	pos;
    bool	have;		/* true if this treasure has ball in it */
    int		team;		/* team of this treasure */
    int 	destroyed;	/* how often this treasure destroyed */
    bool	empty;		/* true if this treasure never had a ball */
    int		ball_style;	/* polystyle to use for color */
} treasure_t;

typedef struct target {
    clpos_t	pos;
    int		team;
    double	dead_ticks;
    double	damage;
    unsigned	conn_mask;
    unsigned 	update_mask;
    long	last_change;
    int		group;
} target_t;

typedef struct team {
    int		NumMembers;		/* Number of current members */
    int		NumRobots;		/* Number of robot players */
    int		NumBases;		/* Number of bases owned */
    int		NumTreasures;		/* Number of treasures owned */
    int		NumEmptyTreasures;	/* Number of empty treasures owned */
    int		TreasuresDestroyed;	/* Number of destroyed treasures */
    int		TreasuresLeft;		/* Number of treasures left */
    int		SwapperId;		/* Player swapping to this full team */
} team_t;

typedef struct item_concentrator {
    clpos_t	pos;
} item_concentrator_t;

typedef struct asteroid_concentrator {
    clpos_t	pos;
} asteroid_concentrator_t;

typedef struct friction_area {
    clpos_t	pos;
    double	friction_setting;	/* Setting from map */
    double	friction;		/* Changes with gameSpeed */
    int		group;
} friction_area_t;

#define MAX_PLAYER_ECMS		8	/* Maximum simultaneous per player */
typedef struct {
    double	size;
    clpos_t	pos;
    int		id;
} ecm_t;

/*
 * Transporter info.
 */
typedef struct {
    clpos_t	pos;
    int		victim_id;
    int		id;
    double	count;
} transporter_t;


extern bool is_polygon_map;

struct world {
    int		x, y;		/* Size of world in blocks */
    double	diagonal;	/* Diagonal length in blocks */
    int		width, height;	/* Size of world in pixels (optimization) */
    int		cwidth, cheight;/* Size of world in clicks */
    double	hypotenuse;	/* Diagonal length in pixels (optimization) */
    rules_t	*rules;
    char	name[MAX_CHARS];
    char	author[MAX_CHARS];
    char	dataURL[MAX_CHARS];

    u_byte	**block;	/* type of item in each block */
    vector_t	**gravity;
    item_t	items[NUM_ITEMS];
    asteroid_t	asteroids;
    team_t	teams[MAX_TEAMS];

    int		NumTeamBases;	/* How many 'different' teams are allowed */

#if 0
    int		NumBases, MaxBases;
    base_t	*bases;
#else
    arraylist_t	*bases;
#endif

#if 0
    int		NumFuels, MaxFuels;
    fuel_t	*fuels;
#else
    arraylist_t	*fuels;
#endif

    int		NumGravs, MaxGravs;
    grav_t	*gravs;
    int		NumCannons, MaxCannons;
    cannon_t	*cannons;
    int		NumChecks, MaxChecks;
    check_t	*checks;
    int		NumWormholes, MaxWormholes;
    wormhole_t	*wormholes;
    int		NumTreasures, MaxTreasures;
    treasure_t	*treasures;
    int		NumTargets, MaxTargets;
    target_t	*targets;
    int		NumItemConcs, MaxItemConcs;
    item_concentrator_t		*itemConcs;
    int		NumAsteroidConcs, MaxAsteroidConcs;
    asteroid_concentrator_t	*asteroidConcs;
    int		NumFrictionAreas, MaxFrictionAreas;
    friction_area_t		*frictionAreas;

    int		NumEcms, MaxEcms;
    ecm_t	*ecms;

    int		NumTransporters, MaxTransporters;
    transporter_t	*transporters;

    bool	have_options;
};

extern world_t		World;

static inline void World_set_block(world_t *world, blkpos_t blk, int type)
{
    assert (! (blk.bx < 0 || blk.bx >= world->x
	       || blk.by < 0 || blk.by >= world->y));
    world->block[blk.bx][blk.by] = type;
}

static inline int World_get_block(world_t *world, blkpos_t blk)
{
    assert (! (blk.bx < 0 || blk.bx >= world->x
	       || blk.by < 0 || blk.by >= world->y));
    return world->block[blk.bx][blk.by];
}

static inline bool World_contains_clpos(world_t *world, clpos_t pos)
{
    if (pos.cx < 0 || pos.cx >= world->cwidth)
	return false;
    if (pos.cy < 0 || pos.cy >= world->cheight)
	return false;
    return true;
}

static inline clpos_t World_get_random_clpos(world_t *world)
{
    clpos_t pos;

    pos.cx = (int)(rfrac() * world->cwidth);
    pos.cy = (int)(rfrac() * world->cheight);

    return pos;
}

static inline int World_wrap_xclick(world_t *world, int cx)
{
    while (cx < 0)
	cx += world->cwidth;
    while (cx >= world->cwidth)
	cx -= world->cwidth;

    return cx;
}

static inline int World_wrap_yclick(world_t *world, int cy)
{
    while (cy < 0)
	cy += world->cheight;
    while (cy >= world->cheight)
	cy -= world->cheight;

    return cy;
}

static inline clpos_t World_wrap_clpos(world_t *world, clpos_t pos)
{
    pos.cx = World_wrap_xclick(world, pos.cx);
    pos.cy = World_wrap_yclick(world, pos.cy);

    return pos;
}


#define Num_bases(w)		Arraylist_get_num_elements((w)->bases)
#define Base_by_index(w, i)	((base_t *)Arraylist_get((w)->bases, (i)))
#define Num_fuels(w)		Arraylist_get_num_elements((w)->fuels)
#define Fuel_by_index(w, i)	((fuel_t *)Arraylist_get((w)->fuels, (i)))



static inline cannon_t *Cannon_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumCannons)
	return &world->cannons[ind];
    return NULL;
}

static inline check_t *Check_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumChecks)
	return &world->checks[ind];
    return NULL;
}

static inline grav_t *Grav_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumGravs)
	return &world->gravs[ind];
    return NULL;
}

static inline target_t *Target_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumTargets)
	return &world->targets[ind];
    return NULL;
}

static inline treasure_t *Treasure_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumTreasures)
	return &world->treasures[ind];
    return NULL;
}

static inline wormhole_t *Wormhole_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumWormholes)
	return &world->wormholes[ind];
    return NULL;
}

static inline asteroid_concentrator_t *AsteroidConc_by_index(world_t *world,
							     int ind)
{
    if (ind >= 0 && ind < world->NumAsteroidConcs)
	return &world->asteroidConcs[ind];
    return NULL;
}

static inline item_concentrator_t *ItemConc_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumItemConcs)
	return &world->itemConcs[ind];
    return NULL;
}

static inline friction_area_t *FrictionArea_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumFrictionAreas)
	return &world->frictionAreas[ind];
    return NULL;
}

static inline ecm_t *Ecm_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumEcms)
	return &world->ecms[ind];
    return NULL;
}

static inline transporter_t *Transporter_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < world->NumTransporters)
	return &world->transporters[ind];
    return NULL;
}

/*
 * Here the index is the team number.
 */
static inline team_t *Team_by_index(world_t *world, int ind)
{
    if (ind >= 0 && ind < MAX_TEAMS)
	return &world->teams[ind];
    return NULL;
}

#endif

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

#ifndef	MAP_H
#define	MAP_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#endif
#ifndef RULES_H
/* need rules_t */
#include "rules.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
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
#define ASTEROID_CONCENTRATOR		28
#define BASE_ATTRACTOR		127

#define SPACE_BIT		(1 << SPACE)
#define BASE_BIT		(1 << BASE)
#define FILLED_BIT		(1 << FILLED)
#define REC_LU_BIT		(1 << REC_LU)
#define REC_LD_BIT		(1 << REC_LD)
#define REC_RU_BIT		(1 << REC_RU)
#define REC_RD_BIT		(1 << REC_RD)
#define FUEL_BIT		(1 << FUEL)
#define CANNON_BIT		(1 << CANNON)
#define CHECK_BIT		(1 << CHECK)
#define POS_GRAV_BIT		(1 << POS_GRAV)
#define NEG_GRAV_BIT		(1 << NEG_GRAV)
#define CWISE_GRAV_BIT		(1 << CWISE_GRAV)
#define ACWISE_GRAV_BIT		(1 << ACWISE_GRAV)
#define WORMHOLE_BIT		(1 << WORMHOLE)
#define TREASURE_BIT		(1 << TREASURE)
#define TARGET_BIT		(1 << TARGET)
#define ITEM_CONCENTRATOR_BIT	(1 << ITEM_CONCENTRATOR)
#define DECOR_FILLED_BIT	(1 << DECOR_FILLED)
#define DECOR_LU_BIT		(1 << DECOR_LU)
#define DECOR_LD_BIT		(1 << DECOR_LD)
#define DECOR_RU_BIT		(1 << DECOR_RU)
#define DECOR_RD_BIT		(1 << DECOR_RD)
#define UP_GRAV_BIT             (1 << UP_GRAV)
#define DOWN_GRAV_BIT           (1 << DOWN_GRAV)
#define RIGHT_GRAV_BIT          (1 << RIGHT_GRAV)
#define LEFT_GRAV_BIT           (1 << LEFT_GRAV)
#define FRICTION_BIT		(1 << FRICTION)
#define ASTEROID_CONCENTRATOR_BIT	(1 << ASTEROID_CONCENTRATOR)

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)

typedef struct {
    clpos	pos;
    long	fuel;
    unsigned	conn_mask;
    long	last_change;
    int		team;
} fuel_t;

typedef struct {
    clpos	pos;
    DFLOAT	force;
    int		type;
} grav_t;

typedef struct base_t {
    clpos		pos;
    int			dir;
    int			ind;
    unsigned short	team;
} base_t;

/* kps - ng does not want this */
typedef struct {
    int		base_idx;	/* Index in World.base[] */
    DFLOAT	dist;		/* Distance to first checkpoint */
} baseorder_t;

typedef struct {
    clpos		pos;
    int			dir;
    unsigned		conn_mask;
    long		last_change;
    int			item[NUM_ITEMS];
    int			tractor_target;
    bool		tractor_is_pressor;
    unsigned short	team;
    long		used;
    DFLOAT		dead_time;
    DFLOAT		damaged;
    DFLOAT		tractor_count;
    DFLOAT		emergency_shield_left;
    DFLOAT		phasing_left;
    int			group;
} cannon_t;

typedef struct {
    DFLOAT	prob;		/* Probability [0..1] for item to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for this item to appear */
    DFLOAT	cannonprob;	/* Relative probability for item to appear */
    int		min_per_pack;	/* minimum number of elements per item. */
    int		max_per_pack;	/* maximum number of elements per item. */
    int		initial;	/* initial number of elements per player. */
    int		limit;		/* max number of elements per player/cannon. */
} item_t;

typedef struct {
    DFLOAT	prob;		/* Probability [0..1] for asteroid to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for asteroid to appear */
} asteroid_t;

typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;

typedef struct {
    clpos		pos;
    int			lastdest;	/* last destination wormhole */
    DFLOAT		countdown;	/* >0 warp to lastdest else random */
    bool		temporary;	/* wormhole was left by hyperjump */
    wormType		type;
    u_byte		lastblock;	/* block it occluded */
    unsigned short	lastID;
} wormhole_t;

typedef struct {
    clpos		pos;
    bool		have;	/* true if this treasure has ball in it */
    unsigned short	team;	/* team of this treasure */
    int 		destroyed;	/* how often this treasure destroyed */
    bool		empty;	/* true if this treasure never had a ball */
} treasure_t;

typedef struct {
    clpos		pos;
    unsigned short	team;
    DFLOAT		dead_time;
    int			damage;
    unsigned		conn_mask;
    unsigned 		update_mask;
    long		last_change;
    int			group;
} target_t;

typedef struct {
    int		NumMembers;		/* Number of current members */
    int		NumRobots;		/* Number of robot players */
    int		NumBases;		/* Number of bases owned */
    int		NumTreasures;		/* Number of treasures owned */
    int		NumEmptyTreasures;	/* Number of empty treasures owned */
    int		TreasuresDestroyed;	/* Number of destroyed treasures */
    int		TreasuresLeft;		/* Number of treasures left */
    int		SwapperId;		/* Player swapping to this full team */
    DFLOAT	score;
    DFLOAT	prev_score;
} team_t;

typedef struct {
    clpos	pos;
} item_concentrator_t, asteroid_concentrator_t;

extern bool is_polygon_map;

typedef struct {
    int			x, y;		/* Size of world in blocks */
    int			diagonal;	/* Diagonal length in blocks */
    int			width, height;	/* Size of world in pixels (optimization) */
    int			cwidth, cheight;/* Size of world in clicks */
    int			hypotenuse;	/* Diagonal length in pixels (optimization) */
    rules_t		*rules;
    char		name[MAX_CHARS];
    char		author[MAX_CHARS];
    char		dataURL[MAX_CHARS];

    u_byte		**block;        /* type of item in each block */

    vector		**gravity;

    item_t		items[NUM_ITEMS];

    asteroid_t		asteroids;

    team_t		teams[MAX_TEAMS];

    int			NumTeamBases;      /* How many 'different' teams are allowed */
    int			NumBases;
    base_t		*base;
    baseorder_t		*baseorder; /* kps - ng does not want this */
    int			NumFuels;
    fuel_t		*fuel;
    int			NumGravs;
    grav_t		*grav;
    int			NumCannons;
    cannon_t		*cannon;
    int			NumChecks;
    clpos		*check;
    int			NumWormholes;
    wormhole_t		*wormHoles;
    int			NumTreasures;
    treasure_t		*treasures;
    int			NumTargets;
    target_t		*targets;
    int			NumItemConcentrators;
    item_concentrator_t	*itemConcentrators;
    int			NumAsteroidConcs;
    asteroid_concentrator_t	*asteroidConcs;
} World_map;

/* kps change 100, 30 etc to something sane */
struct polystyle {
    char id[100];
    int color;
    int texture_id;
    int defedge_id;
    int flags;
};

struct edgestyle {
    char id[100];
    int width;
    int color;
    int style;
};

struct bmpstyle {
    char id[100];
    char filename[30];
    int flags;
};

typedef struct {
    int style;
    int group;
    int edges;
    int cx, cy;
    int num_points;
    int estyles_start;
    int num_echanges;
    int is_decor;
} poly_t;

struct move {
    clvec start;
    clvec delta;
    int hitmask;
    object *obj;
};

struct group {
    int type;
    int team;
    int item_id;
    unsigned int hitmask;
    bool (*hitfunc)(struct group *group, struct move *move);
};

extern struct polystyle pstyles[256];
extern struct edgestyle estyles[256];
extern struct bmpstyle  bstyles[256];
extern poly_t *pdata;
extern int *estyleptr;
extern int *edgeptr;
extern struct group groups[];
extern int num_pstyles, num_estyles, num_bstyles;

#endif

/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)

typedef struct {
    ipos	clk_pos;
    long	fuel;
    unsigned	conn_mask;
    long	last_change;
    int		team;
} fuel_t;

typedef struct {
    ipos	pos;
    DFLOAT	force;
} grav_t;

typedef struct {
    ipos	pos;
    int		dir;
    u_short	team;
} base_t;

typedef struct {
    ipos	blk_pos;
    ipos	clk_pos;
    int		dir;
    int		dead_time;
    unsigned	conn_mask;
    long	last_change;
    int		item[NUM_ITEMS];
    int		damaged;
    int		tractor_target;
    int		tractor_count;
    bool	tractor_is_pressor;
    u_short	team;
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

typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;

typedef struct {
    ipos	pos;
    int		lastdest,	/* last destination wormhole */
		countdown,	/* if >0 warp to lastdest else random */
		lastplayer;	/* last player to pass through */
    bool	temporary;	/* wormhole was left by hyperjump */
    wormType	type;
} wormhole_t;

typedef struct {
    ipos	pos;
    bool	have;		/* true if this treasure has ball in it */
    u_short	team;		/* team of this treasure */
    int 	destroyed;	/* number of times this treasure destroyed */
} treasure_t;

typedef struct {
    ipos	pos;
    u_short	team;
    int		dead_time;
    int		damage;
    unsigned	conn_mask;
    unsigned 	update_mask;
    long	last_change;
} target_t;

typedef struct {
    int		NumMembers;		/* Number of current members */
    int		NumBases;		/* Number of bases owned */
    int		NumTreasures;		/* Number of treasures owned */
    int		TreasuresDestroyed;	/* Number of destroyed treasures */
    int		TreasuresLeft;		/* Number of treasures left */
    int         SwapperId;              /* Player swapping to this full team */
} team_t;

typedef struct {
    ipos	pos;
} item_concentrator_t;

typedef struct {
    int		x, y;		/* Size of world in blocks */
    int		diagonal;	/* Diagonal length in blocks */
    int		width, height;	/* Size of world in pixels (optimization) */
    int         cwidth, cheight;/* Size of world in clicks */
    int		hypotenuse;	/* Diagonal length in pixels (optimization) */
    rules_t	*rules;
    char	name[MAX_CHARS];
    char	author[MAX_CHARS];
    char        dataURL[MAX_CHARS];

    vector	**gravity;

    item_t	items[NUM_ITEMS];

    team_t	teams[MAX_TEAMS];

    int		NumTeamBases;      /* How many 'different' teams are allowed */
    int		NumBases;
    base_t	*base;
    int		NumFuels;
    fuel_t	*fuel;
    int		NumGravs;
    grav_t	*grav;
    int		NumCannons;
    cannon_t	*cannon;
    int		NumChecks;
    ipos	*check;
    int		NumWormholes;
    wormhole_t	*wormHoles;
    int		NumTreasures;
    treasure_t	*treasures;
    int         NumTargets;
    target_t    *targets;
    int		NumItemConcentrators;
    item_concentrator_t	*itemConcentrators;
} World_map;

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
    int *edges;
    int x, y;
    int num_points;
    int hidden;
    int num_hidden;
} poly_t;

extern struct polystyle pstyles[256];
extern struct edgestyle estyles[256];
extern struct bmpstyle  bstyles[256];
extern poly_t *pdata;
extern int *hidptr;

extern int num_pstyles, num_estyles, num_bstyles;
#endif

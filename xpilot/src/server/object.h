/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
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

#ifndef	OBJECT_H
#define	OBJECT_H

#ifndef SERVERCONST_H
/* need MAX_TANKS */
#include "serverconst.h"
#endif
#ifndef KEYS_H
/* need NUM_KEYS */
#include "keys.h"
#endif
#ifndef BIT_H
/* need BITV_DECL */
#include "bit.h"
#endif
#ifndef DRAW_H
/* need shipshape_t */
#include "draw.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif
#ifndef CLICK_H
/* need CLICK */
#include "click.h"
#endif
#ifndef MAP_H
/* need treasure_t */
#include "map.h"
#endif

#ifdef _WINDOWS
#include "NT/winNet.h"
#endif



/*
 * Different types of objects, including player.
 * Robots and tanks are players but have an additional type_ext field.
 * Smart missile, heatseeker and torpedo can be merged into missile.
 */
#define OBJ_TYPEBIT(type)	(1U<<(type))

#define OBJ_PLAYER		0
#define OBJ_DEBRIS		1
#define OBJ_SPARK		2
#define OBJ_BALL		3
#define OBJ_SHOT		4
#define OBJ_SMART_SHOT		5
#define OBJ_MINE		6
#define OBJ_TORPEDO		7
#define OBJ_HEAT_SHOT		8
#define OBJ_PULSE		9
#define OBJ_ITEM		10
#define OBJ_WRECKAGE		11
#define OBJ_ASTEROID		12
#define OBJ_CANNON_SHOT		13

#define OBJ_PLAYER_BIT		OBJ_TYPEBIT(OBJ_PLAYER)
#define OBJ_DEBRIS_BIT		OBJ_TYPEBIT(OBJ_DEBRIS)
#define OBJ_SPARK_BIT		OBJ_TYPEBIT(OBJ_SPARK)
#define OBJ_BALL_BIT		OBJ_TYPEBIT(OBJ_BALL)
#define OBJ_SHOT_BIT		OBJ_TYPEBIT(OBJ_SHOT)
#define OBJ_SMART_SHOT_BIT	OBJ_TYPEBIT(OBJ_SMART_SHOT)
#define OBJ_MINE_BIT		OBJ_TYPEBIT(OBJ_MINE)
#define OBJ_TORPEDO_BIT		OBJ_TYPEBIT(OBJ_TORPEDO)
#define OBJ_HEAT_SHOT_BIT	OBJ_TYPEBIT(OBJ_HEAT_SHOT)
#define OBJ_PULSE_BIT		OBJ_TYPEBIT(OBJ_PULSE)
#define OBJ_ITEM_BIT		OBJ_TYPEBIT(OBJ_ITEM)
#define OBJ_WRECKAGE_BIT	OBJ_TYPEBIT(OBJ_WRECKAGE)
#define OBJ_ASTEROID_BIT	OBJ_TYPEBIT(OBJ_ASTEROID)
#define OBJ_CANNON_SHOT_BIT	OBJ_TYPEBIT(OBJ_CANNON_SHOT)

/*
 * Some object types are overloaded.
 * These bits are set in the player->type_ext field.
 */
#define OBJ_EXT_TANK		(1U<<1)
#define OBJ_EXT_ROBOT		(1U<<2)

/*
 * Different types of attributes a player can have.
 * These are the bits of the player->have and player->used fields.
 */
#define HAS_EMERGENCY_THRUST	(1U<<30)
#define HAS_AUTOPILOT		(1U<<29)
#define HAS_TRACTOR_BEAM	(1U<<28)
#define HAS_LASER		(1U<<27)
#define HAS_CLOAKING_DEVICE	(1U<<26)
#define HAS_SHIELD		(1U<<25)
#define HAS_REFUEL		(1U<<24)
#define HAS_REPAIR		(1U<<23)
#define HAS_COMPASS		(1U<<22)
#define HAS_AFTERBURNER		(1U<<21)
#define HAS_CONNECTOR		(1U<<20)
#define HAS_EMERGENCY_SHIELD	(1U<<19)
#define HAS_DEFLECTOR		(1U<<18)
#define HAS_PHASING_DEVICE	(1U<<17)
#define HAS_MIRROR		(1U<<16)
#define HAS_ARMOR		(1U<<15)
#define HAS_SHOT		(1U<<4)
#define HAS_BALL		(1U<<3)

/*
 * Possible object status bits.
 */
#define GRAVITY			(1U<<0)
#define WARPING			(1U<<1)
#define WARPED			(1U<<2)
#define CONFUSED		(1U<<3)
#define FROMCANNON		(1U<<4)		/* Object from cannon */
#define RECREATE		(1U<<5)		/* Recreate ball */
#define FROMBOUNCE		(1U<<6)		/* Spark from wall bounce */
#define OWNERIMMUNE		(1U<<7)		/* Owner is immune to object */
#define NOEXPLOSION		(1U<<8)		/* No recreate explosion */
#define COLLISIONSHOVE		(1U<<9)		/* Collision counts as shove */
#define RANDOM_ITEM		(1U<<10)	/* item shows up as random */

/*
 * Possible player status bits.
 * The bits that the client needs must fit into a byte,
 * so the first 8 bitvalues are reserved for that purpose,
 * those are defined in common/rules.h.
 */
#define KILLED			(1U<<8)
#define HOVERPAUSE		(1U<<9)		/* Hovering pause */
#define REPROGRAM		(1U<<10)	/* Player reprogramming */
#define FINISH			(1U<<11)	/* Finished a lap this frame */
#define RACE_OVER		(1U<<12)	/* After finished and score */


/*
 * Weapons modifiers.
 */
typedef struct {
    unsigned int	nuclear	:2;	/* N  modifier */
    unsigned int	warhead	:2;	/* CI modifier */
    unsigned int	velocity:2;	/* V# modifier */
    unsigned int	mini	:2;	/* X# modifier */
    unsigned int	spread	:2;	/* Z# modifier */
    unsigned int	power	:2;	/* B# modifier */
    unsigned int	laser	:2;	/* LS LB modifier */
    unsigned int	spare	:18;	/* padding for alignment */
} modifiers_t;

#define CLEAR_MODS(mods)	memset(&(mods), 0, sizeof(modifiers_t))

#define MODS_NUCLEAR_MAX	2	/* - N FN */
#define NUCLEAR			(1U<<0)
#define FULLNUCLEAR		(1U<<1)

#define MODS_WARHEAD_MAX	3	/* - C I CI */
#define CLUSTER			(1U<<0)
#define IMPLOSION		(1U<<1)

#define MODS_VELOCITY_MAX	3	/* - V1 V2 V3 */
#define MODS_MINI_MAX		3	/* - X2 X3 X4 */
#define MODS_SPREAD_MAX		3	/* - Z1 Z2 Z3 */
#define MODS_POWER_MAX		3	/* - B1 B2 B3 */

#define MODS_LASER_MAX		2	/* - LS LB */
#define STUN			(1U<<0)
#define BLIND			(1U<<1)

#define LOCK_NONE		0x00	/* No lock */
#define LOCK_PLAYER		0x01	/* Locked on player */
#define LOCK_VISIBLE		0x02	/* Lock information was on HUD */
					/* computed just before frame shown */
					/* and client input checked */
#define LOCKBANK_MAX		4	/* Maximum number of locks in bank */

/*
 * Hitmasks are 32 bits.
 */
#define ALL_BITS		0xffffffffU
#define BALL_BIT		(1U << 11)
#define NONBALL_BIT		(1U << 12)
#define NOTEAM_BIT		(1U << 10)
#define HITMASK(team) ((team) == TEAM_NOT_SET ? NOTEAM_BIT : 1U << (team))


/*
 * Node within a Cell list.
 */
typedef struct cell_node cell_node_t;
struct cell_node {
    cell_node_t		*next;
    cell_node_t		*prev;
};


#define OBJECT_BASE	\
    short		id;		/* For shots => id of player */	\
    uint16_t		team;		/* Team of player or cannon */	\
/* Object position pos must only be changed with the proper functions! */ \
    clpos_t		pos;		/* World coordinates */		\
    clpos_t		prevpos;	/* previous position */		\
    clpos_t		extmove;	/* For collision detection */	\
    float		wall_time;	/* bounce/crash time within frame */ \
    vector_t		vel;		/* speed in x,y */		\
    vector_t		acc;		/* acceleration in x,y */	\
    float		mass;		/* mass in unigrams */		\
    double		life;		/* No of ticks left to live */	\
/* Item pack count is kept in the 'count' field, float now, change !@# */ \
    float		count;		/* Misc timings */		\
    modifiers_t		mods;		/* Modifiers to this object */	\
    uint8_t		type;		/* one of OBJ_XXX */		\
    uint8_t		color;		/* Color of object */		\
    uint8_t		missile_dir;	/* missile direction */		\
    uint8_t		collmode;	/* collision checking mode */	\
    short		wormHoleHit;	\
    short		wormHoleDest;	\
    uint16_t		obj_status;	/* gravity, etc. */		\

/* up to here all object types are the same as all player types. */

#define OBJECT_EXTEND	\
    cell_node_t		cell;		/* node in cell linked list */	\
    short		pl_range;	/* distance for collision */	\
    short		pl_radius;	/* distance for hit */		\
    long		info;		/* Miscellaneous info */	\
    double		fusetime;	/* Frame when considered fused */ \

/* up to here all object types are the same. */


/*
 * Generic object
 */
typedef struct xp_object object_t;
struct xp_object {

    OBJECT_BASE

    OBJECT_EXTEND

#define OBJ_IND(ind)	(Obj[(ind)])
#define OBJ_PTR(ptr)	((object_t *)(ptr))
};

typedef struct player player_t;

/*
 * Mine object
 */
typedef struct xp_mineobject mineobject_t;
struct xp_mineobject {

    OBJECT_BASE

    OBJECT_EXTEND

    float		ecm_range;	/* Range from last ecm center */
    float		spread_left;	/* how much spread time left */
    short 		owner;		/* Who's object is this ? */

#define MINE_IND(ind)	((mineobject_t *)Obj[(ind)])
#define MINE_PTR(ptr)	((mineobject_t *)(ptr))
};


#define MISSILE_EXTEND		\
    float		max_speed;	/* speed limitation */		\
    float		turnspeed;	/* how fast to turn */

/* up to here all missiles types are the same. */

/*
 * Generic missile object
 */
typedef struct xp_missileobject missileobject_t;
struct xp_missileobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

#define MISSILE_IND(ind)	((missileobject_t *)Obj[(ind)])
#define MISSILE_PTR(ptr)	((missileobject_t *)(ptr))
};


/*
 * Smart missile is a generic missile with extras.
 */
typedef struct xp_smartobject smartobject_t;
struct xp_smartobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

    int			new_info;	/* smart re-lock id */
    float		ecm_range;	/* Range from last ecm center */

#define SMART_IND(ind)	((smartobject_t *)Obj[(ind)])
#define SMART_PTR(ptr)	((smartobject_t *)(ptr))
};


/*
 * Torpedo is a generic missile with extras
 */
typedef struct xp_torpobject torpobject_t;
struct xp_torpobject {

    OBJECT_BASE

    OBJECT_EXTEND

    MISSILE_EXTEND

    float		spread_left;	/* how much spread time left */

#define TORP_IND(ind)	((torpobject_t *)Obj[(ind)])
#define TORP_PTR(ptr)	((torpobject_t *)(ptr))
};


/*
 * The ball object.
 */
typedef struct xp_ballobject ballobject_t;
struct xp_ballobject {

    OBJECT_BASE

    OBJECT_EXTEND

    treasure_t		*treasure;	/* treasure for ball */
    short 		owner;		/* Who's object is this ? */
    short		style;		/* What polystyle to use */

#define BALL_IND(ind)	((ballobject_t *)Obj[(ind)])
#define BALL_PTR(obj)	((ballobject_t *)(obj))
};


/*
 * Object with a wireframe representation.
 */
typedef struct xp_wireobject wireobject_t;
struct xp_wireobject {

    OBJECT_BASE

    OBJECT_EXTEND

    float		turnspeed;	/* how fast to turn */

    uint8_t		size;		/* Size of object (wreckage) */
    uint8_t		rotation;	/* Rotation direction */

#define WIRE_IND(ind)	((wireobject_t *)Obj[(ind)])
#define WIRE_PTR(obj)	((wireobject_t *)(obj))
};


/*
 * Pulse object used for laser pulses.
 */
typedef struct xp_pulseobject pulseobject_t;
struct xp_pulseobject {

    OBJECT_BASE

    OBJECT_EXTEND

    float		len;		/* Length of the pulse */
    uint8_t		dir;		/* Direction of the pulse */
    bool		refl;		/* Pulse was reflected ? */

#define PULSE_IND(ind)	((pulseobject_t *)Obj[(ind)])
#define PULSE_PTR(obj)	((pulseobject_t *)(obj))
};


/*
 * Any object type should be part of this union.
 */
typedef union xp_anyobject anyobject_t;
union xp_anyobject {
    object_t		obj;
    ballobject_t	ball;
    mineobject_t	mine;
    missileobject_t	missile;
    smartobject_t	smart;
    torpobject_t	torp;
    wireobject_t	wireobj;
    pulseobject_t	pulse;
};


/*
 * Fuel structure, used by player
 */
typedef struct {
    double	sum;			/* Sum of fuel in all tanks */
    double	max;			/* How much fuel can you take? */
    int		current;		/* Number of currently used tank */
    int		num_tanks;		/* Number of tanks */
    double	tank[1 + MAX_TANKS];	/* main fixed tank + extra tanks. */
} pl_fuel_t;

typedef struct visibility {
    bool	canSee;
    long	lastChange;
} visibility_t;

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
    player_t	*victim;
    int		id;
    double	count;
} trans_t;

/*
 * Shove-information.
 *
 * This is for keeping a record of the last N times the player was shoved,
 * for assigning wall-smash-blame, where N=MAX_RECORDED_SHOVES.
 */
#define MAX_RECORDED_SHOVES 4

typedef struct {
    int		pusher_id;
    int		time;
} shove_t;

struct robot_data;
struct ranknode;

/* IMPORTANT
 *
 * This is the player structure, the first part MUST be similar to object_t,
 * this makes it possible to use the same basic operations on both of them
 * (mainly used in update.c).
 */
struct player {

    OBJECT_BASE

    /* up to here the player type should be the same as an object. */

    int		type_ext;		/* extended type info (tank, robot) */
    uint16_t	pl_status;		/* playing, etc. */

    double	turnspeed;		/* How fast player acc-turns */
    double	velocity;		/* Absolute speed */

    int		kills;			/* Number of kills this round */
    int		deaths;			/* Number of deaths this round */

    long	used;			/** Items you use **/
    long	have;			/** Items you have **/

    double	shield_time;		/* Shields if no options.playerShielding */
    pl_fuel_t	fuel;			/* ship tanks and the stored fuel */
    double	emptymass;		/* Mass of empty ship */
    double	float_dir;		/* Direction, in float var */
    int     	turnqueue;  	    	/*TURNQUEUE*//* use it? */
    double	wanted_float_dir;   	/*TURNQUEUE*//* Direction, in float var */
    double	float_dir_cos;		/* Cosine of float_dir */
    double	float_dir_sin;		/* Sine of float_dir */
    double	turnresistance;		/* How much is lost in % */
    double	turnvel;		/* Current velocity of turn (right) */
    double	oldturnvel;		/* Last velocity of turn (right) */
    double	turnacc;		/* Current acceleration of turn */
    double	score;			/* Current score of player */
    double	prev_score;		/* Last score that has been updated */
    int		prev_life;		/* Last life that has been updated */
    shipshape_t	*ship;			/* wire model of ship shape */
    double	power;			/* Force of thrust */
    double	power_s;		/* Saved power fiks */
    double	turnspeed_s;		/* Saved turnspeed */
    double	turnresistance_s;	/* Saved (see above) */
    double	sensor_range;		/* Range of sensors (radar) */
    int		shots;			/* Number of active shots by player */
    int		missile_rack;		/* Next missile rack to be active */

    int		num_pulses;		/* Number of laser pulses "flying". */

    double	emergency_thrust_left;	/* how much emergency thrust left */
    double	emergency_shield_left;	/* how much emergency shield left */
    double	phasing_left;		/* how much time left */

    double	pause_count;		/* ticks until unpause possible */
    double	recovery_count;		/* ticks to recovery */
    double	self_destruct_count;	/* if > 0, ticks before boom */

    int		item[NUM_ITEMS];	/* for each item type how many */
    int		lose_item;		/* which item to drop */
    int		lose_item_state;	/* lose item key state, 2=up,1=down */

    double	auto_power_s;		/* autopilot saves of current */
					/* power, turnspeed and */
    double	auto_turnspeed_s;	/* turnresistance settings. Restored */
    double	auto_turnresistance_s;	/* when autopilot turned off */
    modifiers_t	modbank[NUM_MODBANKS];	/* useful modifier settings */
    double	shot_time;		/* Time of last shot fired by player */
    double	laser_time;		/* Time of last laser pulse fired by player */
    bool	did_shoot;		/* Pressed fire during this frame */
    bool	tractor_is_pressor;	/* on if tractor is pressor */
    int		repair_target;		/* Repairing this target */
    int		fs;			/* Connected to fuel station fs */
    int		check;			/* Next check point to pass */
    int		prev_check;		/* Previous check point for score */
    int		time;			/* The time a player has used */
    int		round;			/* Number of rounds player have done */
    int		prev_round;		/* Previous rounds value for score */
    int		best_lap;		/* Players best lap time */
    int		last_lap;		/* Time on last pass */
    int		last_lap_time;		/* What was your last pass? */
    int		last_check_dir;		/* player dir at last checkpoint */
    long	last_wall_touch;	/* last time player touched a wall */

    base_t	*home_base;
    struct {
	int	    tagged;		/* Flag, what is tagged? */
	int	    pl_id;		/* Tagging player id */
	double	    distance;		/* Distance to object */
    } lock;
    int		lockbank[LOCKBANK_MAX]; /* Saved player locks */

    short	dir;			/* Direction of acceleration */
    char	mychar;			/* Special char for player */
    char	prev_mychar;		/* Special char for player */
    char	name[MAX_CHARS];	/* Nick-name of player */
    char	username[MAX_CHARS];	/* Real name of player */
    char	hostname[MAX_CHARS];	/* Hostname of client player uses */
    uint16_t	pseudo_team;		/* Which team for detaching tanks */
    int		alliance;		/* Member of which alliance? */
    int		prev_alliance;		/* prev. alliance for score */
    int		invite;			/* Invitation for alliance */
    ballobject_t	*ball;

    /*
     * Pointer to robot private data (dynamically allocated).
     * Only used in robot code.
     */
    struct robot_data	*robot_data_ptr;

    /*
     * A record of who's been pushing me (a circular buffer).
     */
    shove_t     shove_record[MAX_RECORDED_SHOVES];
    int	 	shove_next;

    visibility_t *visibility;

    double	forceVisible;
    double	damaged;
    double	stunned;
    int		updateVisibility;

    int		last_target_update;	/* index of last updated target */
    int		last_cannon_update;	/* index of last updated cannon */
    int		last_fuel_update;	/* index of last updated fuel */
    int		last_polystyle_update;	/* index of last updated polygon */

    int		ecmcount;		/* number of active ecms */

    connection_t *conn;			/* connection index, NULL if robot */
    unsigned	version;		/* XPilot version number of client */

    BITV_DECL(last_keyv, NUM_KEYS);	/* Keyboard state */
    BITV_DECL(prev_keyv, NUM_KEYS);	/* Keyboard state */

    long	frame_last_busy;	/* When player touched keyboard. */

    void	*audio;			/* audio private data */

    int		player_fps;		/* FPS that this player can do */
    int		maxturnsps;		/* turns per second limit */

    int		rectype;		/* normal, saved or spectator */
    struct ranknode	*rank;

    double	pauseTime;		/* seconds player has paused */
    double	idleTime;		/* seconds player has idled */

    int	 	flooding;

    bool	muted;			/* player started is muted? */
    bool	isowner;		/* player started this server? */
    bool	isoperator;		/* player has operator privileges? */

    int		privs;			/* Player privileges */

#define PRIV_NOAUTOKICK		1
#define PRIV_AUTOKICKLAST	2

    world_t	*world;			/* World player is in */

};

#endif

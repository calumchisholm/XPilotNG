/* $Id$
 *
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

#ifndef CONST_H
#define	CONST_H

#include <limits.h>
#ifndef _WINDOWS
#include <math.h>
#endif

#include "types.h"
/*
 * FLT_MAX and RAND_MAX is ANSI C standard, but some systems (BSD) use
 * MAXFLOAT and INT_MAX instead.
 */
#ifndef	FLT_MAX
#   if defined(__sgi) || defined(__FreeBSD__)
#       include <float.h>	/* FLT_MAX for SGI Personal Iris or FreeBSD */
#   else
#	if defined(__sun__)
#           include <values.h>	/* MAXFLOAT for suns */
#	endif
#	ifdef VMS
#	    include <float.h>
#	endif
#   endif
#   if !defined(FLT_MAX)
#	if defined(MAXFLOAT)
#	    define FLT_MAX	MAXFLOAT
#	else
#	    define FLT_MAX	1e30f	/* should suffice :-) */
#	endif
#   endif
#endif

/* Not everyone has PI (or M_PI defined). */
#ifndef	M_PI
#   define PI		3.14159265358979323846
#else
#   define PI		M_PI
#endif

/* Not everyone has LINE_MAX either, *sigh* */
#ifdef VMS
#   undef LINE_MAX
#endif
#ifndef LINE_MAX
#   define LINE_MAX 2048
#endif

#define RES		128

#define BLOCK_SZ	35

#define TABLE_SIZE	RES

extern DFLOAT		tbl_sin[];
extern DFLOAT		tbl_cos[];

#if 0
  /* The way it was: one table, and always range checking. */
# define tsin(x)	(tbl_sin[MOD2(x, TABLE_SIZE)])
# define tcos(x)	(tbl_sin[MOD2((x)+TABLE_SIZE/4, TABLE_SIZE)])
#else
# if 0
   /* Range checking: find out where the table size is exceeded. */
#  define CHK2(x, m)	((MOD2(x, m) != x) ? (printf("MOD %s:%d:%s\n", __FILE__, __LINE__, #x), MOD2(x, m)) : (x))
# else
   /* No range checking. */
#  define CHK2(x, m)	(x)
# endif
  /* New table lookup with optional range checking and no extra calculations. */
# define tsin(x)	(tbl_sin[CHK2(x, TABLE_SIZE)])
# define tcos(x)	(tbl_cos[CHK2(x, TABLE_SIZE)])
#endif

#define NELEM(a)	((int)(sizeof(a) / sizeof((a)[0])))

#undef ABS
#define ABS(x)			( (x)<0 ? -(x) : (x) )
#ifndef MAX
#   define MIN(x, y)		( (x)>(y) ? (y) : (x) )
#   define MAX(x, y)		( (x)>(y) ? (x) : (y) )
#endif
#define sqr(x)			( (x)*(x) )
#define DELTA(a, b)		(((a) >= (b)) ? ((a) - (b)) : ((b) - (a)))
#define LENGTH(x, y)		( hypot( (double) (x), (double) (y) ) )
#define VECTOR_LENGTH(v)	( hypot( (double) (v).x, (double) (v).y ) )
#define QUICK_LENGTH(x,y)	( ABS(x)+ABS(y) ) /*-BA Only approx, but v. quick */
#define LIMIT(val, lo, hi)	( val=(val)>(hi)?(hi):((val)<(lo)?(lo):(val)) )


/*
 * Two macros for edge wrap of differences in position.
 * If the absolute value of a difference is bigger than
 * half the map size then it is wrapped.
 */
#define WRAP_DX(dx)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((dx) < - (World.width >> 1) \
		? (dx) + World.width \
		: ((dx) > (World.width >> 1) \
		    ? (dx) - World.width \
		    : (dx))) \
	    : (dx))

#define WRAP_DY(dy)	\
	(BIT(World.rules->mode, WRAP_PLAY) \
	    ? ((dy) < - (World.height >> 1) \
		? (dy) + World.height \
		: ((dy) > (World.height >> 1) \
		    ? (dy) - World.height \
		    : (dy))) \
	    : (dy))

#ifndef MOD2
#  define MOD2(x, m)		( (x) & ((m) - 1) )
#endif	/* MOD2 */

#define MAX_OBSERVERS   8

/* Do NOT change this! */
#define MAX_TEAMS		10

#define PSEUDO_TEAM(i,j)\
	(Players[(i)]->pseudo_team == Players[(j)]->pseudo_team)

/*
 * Used where we wish to know if a player is simply on the same team.
 */
#define TEAM(i, j) \
	(BIT(World.rules->mode, TEAM_PLAY) \
	&& (Players[i]->team == Players[j]->team) \
	&& (Players[i]->team != TEAM_NOT_SET))

/*
 * Used where we wish to know if a player is on the same team
 * and has immunity to shots, thrust sparks, lasers, ecms, etc.
 */
#define TEAM_IMMUNE(i, j)	(teamImmunity && TEAM(i, j))

#define CANNON_DEAD_TIME	900

#define NUM_IDS			256
#define EXPIRED_MINE_ID		4096   /* assume no player has this id */
#define MAX_PSEUDO_PLAYERS      16

#define MAX_CHARS		80
#define MSG_LEN			256

#define FONT_LEN		256

#define NUM_MODBANKS		4

#define MAX_TOTAL_SHOTS		16384	/* must be <= 65536 */
#define MAX_TOTAL_PULSES	(5 * 64)
#define MAX_TOTAL_ECMS		64
#define MAX_TOTAL_TRANSPORTERS	(2 * 64)

#define SPEED_LIMIT		65.0
#define MAX_PLAYER_TURNSPEED	64.0
#define MIN_PLAYER_TURNSPEED	4.0
#define MAX_PLAYER_POWER	55.0
#define MIN_PLAYER_POWER	5.0
#define MAX_PLAYER_TURNRESISTANCE	1.0
#define MIN_PLAYER_TURNRESISTANCE	0.0

#define TIME_BITS		6
#define TIME_FACT 		(1 << TIME_BITS)
#define	RECOVERY_DELAY		(12*3*TIME_FACT)

#define FUEL_SCALE_BITS         8
#define FUEL_SCALE_FACT         (1<<FUEL_SCALE_BITS)
#define FUEL_MASS(f)            ((f)*0.005/FUEL_SCALE_FACT)
#define MAX_STATION_FUEL	(500<<FUEL_SCALE_BITS)
/* changed the default to max to avoid sending lots of fuel ACKs */
#define START_STATION_FUEL      MAX_STATION_FUEL
#define STATION_REGENERATION	(0.06*FUEL_SCALE_FACT/TIME_FACT)
#define MAX_PLAYER_FUEL		(2600<<FUEL_SCALE_BITS)
#define MIN_PLAYER_FUEL		(350<<FUEL_SCALE_BITS)
#define REFUEL_RATE		((5<<FUEL_SCALE_BITS)/TIME_FACT)
#define ENERGY_PACK_FUEL        ((500+(randomMT()&511))<<FUEL_SCALE_BITS)
#define FUEL_NOTIFY             (3*12)

#define TARGET_DEAD_TIME	(12 * 60)
#define TARGET_DAMAGE		(250<<FUEL_SCALE_BITS)
#define TARGET_FUEL_REPAIR_PER_FRAME (TARGET_DAMAGE / (12 * 10 * TIME_FACT))
#define TARGET_REPAIR_PER_FRAME	(TARGET_DAMAGE / (12 * 600 * TIME_FACT))
#define TARGET_UPDATE_DELAY	(TARGET_DAMAGE / (TARGET_REPAIR_PER_FRAME \
				    * BLOCK_SZ * TIME_FACT))

#define LG2_MAX_AFTERBURNER     4
#define ALT_SPARK_MASS_FACT     4.2
#define ALT_FUEL_FACT           3
#define MAX_AFTERBURNER        ((1<<LG2_MAX_AFTERBURNER)-1)
#define AFTER_BURN_SPARKS(s,n)  (((s)*(n))>>LG2_MAX_AFTERBURNER)
#define AFTER_BURN_POWER_FACTOR(n) \
 (1.0+(n)*((ALT_SPARK_MASS_FACT-1.0)/(MAX_AFTERBURNER+1.0)))
#define AFTER_BURN_POWER(p,n)   \
 ((p)*AFTER_BURN_POWER_FACTOR(n))
#define AFTER_BURN_FUEL(f,n)    \
 (((f)*((MAX_AFTERBURNER+1)+(n)*(ALT_FUEL_FACT-1)))/(MAX_AFTERBURNER+1.0))

#define THRUST_MASS             0.7

#define ARMOR_MASS		(ShipMass / 14)

#define MAX_TANKS               8
#define TANK_MASS               (ShipMass/10)
#define TANK_CAP(n)             (!(n)?MAX_PLAYER_FUEL:(MAX_PLAYER_FUEL/3))
#define TANK_FUEL(n)            ((TANK_CAP(n)*(5+(randomMT()&3)))/32)
#define TANK_REFILL_LIMIT       (MIN_PLAYER_FUEL/8)
#define TANK_THRUST_FACT        0.7
#define TANK_NOTHRUST_TIME      (HEAT_CLOSE_TIMEOUT/2+2)
#define TANK_THRUST_TIME        (TANK_NOTHRUST_TIME/2+1)

#define GRAVS_POWER		2.7

/*
 * Size (pixels) of radius for legal HIT!
 * Was 14 until 4.2. Increased due to `analytical collision detection'
 * which inspects a real circle and not just a square anymore.
 */
#define SHIP_SZ		        16
#define VISIBILITY_DISTANCE	1000.0
#define WARNING_DISTANCE	(VISIBILITY_DISTANCE*0.8)

#define ECM_DISTANCE		(VISIBILITY_DISTANCE*0.4)

#define TRANSPORTER_DISTANCE	(VISIBILITY_DISTANCE*0.2)

#define SHOT_MULT(o) \
	((BIT((o)->mods.nuclear, NUCLEAR) && BIT((o)->mods.warhead, CLUSTER)) \
	 ? nukeClusterDamage : 1.0f)

#define MINE_RADIUS		8
#define MINE_RANGE              (VISIBILITY_DISTANCE*0.1)
#define MINE_SENSE_BASE_RANGE   (MINE_RANGE*1.3)
#define MINE_SENSE_RANGE_FACTOR (MINE_RANGE*0.3)
#define MINE_MASS               30.0
#define MINE_LIFETIME           (5000 * TIME_FACT+(randomMT()&(255*TIME_FACT)))
#define MINE_SPEED_FACT         1.3

#define MISSILE_LIFETIME        (randomMT()%(64*12-1)+(128*12))
#define MISSILE_MASS            5.0
#define MISSILE_RANGE           4
#define SMART_SHOT_ACC		0.6
#define SMART_SHOT_DECFACT	3
#define SMART_SHOT_MIN_SPEED	(SMART_SHOT_ACC*8)
#define SMART_TURNSPEED         2.6
#define SMART_SHOT_MAX_SPEED	22.0
#define SMART_SHOT_LOOK_AH      4
#define TORPEDO_SPEED_TIME      (2*12)
#define TORPEDO_ACC 	(18.0*SMART_SHOT_MAX_SPEED/(12*TORPEDO_SPEED_TIME))
#define TORPEDO_RANGE           (MINE_RANGE*0.45)

#define NUKE_SPEED_TIME		(2*12)
#define NUKE_ACC 		(5*TORPEDO_ACC)
#define NUKE_RANGE		(MINE_RANGE*1.5)
#define NUKE_MASS_MULT		1
#define NUKE_MINE_EXPL_MULT	3
#define NUKE_SMART_EXPL_MULT	4

#define HEAT_RANGE              (VISIBILITY_DISTANCE/2)
#define HEAT_SPEED_FACT         1.7
#define HEAT_CLOSE_TIMEOUT      (2*12)
#define HEAT_CLOSE_RANGE        HEAT_RANGE
#define HEAT_CLOSE_ERROR        0
#define HEAT_MID_TIMEOUT        (4*12)
#define HEAT_MID_RANGE          (2*HEAT_RANGE)
#define HEAT_MID_ERROR          8
#define HEAT_WIDE_TIMEOUT       (8*12)
#define HEAT_WIDE_ERROR         16

#define CLUSTER_MASS_SHOTS(mass) ((mass) * 0.9 / ShotsMass)
#define CLUSTER_MASS_DRAIN(mass) (CLUSTER_MASS_SHOTS(mass)*ED_SHOT)

#define BALL_RADIUS		10

#define MISSILE_LEN		15
#define SMART_SHOT_LEN		12
#define HEAT_SHOT_LEN		15
#define TORPEDO_LEN		18

#define PULSE_SPEED		(90 * CLICK)
#define PULSE_LENGTH		(PULSE_SPEED - 5 * CLICK)
#define PULSE_MIN_LIFE		4.5
#define PULSE_LIFE(lasers)	(PULSE_MIN_LIFE + ((lasers) / 4))

#define TRACTOR_MAX_RANGE(items)  (200 + (items) * 50)
#define TRACTOR_MAX_FORCE(items)  (-40 + (items) * -20)
#define TRACTOR_PERCENT(dist, maxdist) \
	(1.0 - (0.5 * (dist) / (maxdist)))
#define TRACTOR_COST(percent) (-1.5 * FUEL_SCALE_FACT * (percent))
#define TRACTOR_FORCE(tr_pr, percent, maxforce) \
	((percent) * (maxforce) * ((tr_pr) ? -1 : 1))

#define WARN_TIME               2

#define BALL_STRING_LENGTH      (120 * CLICK)

#define TEAM_NOT_SET		0xffff
#define TEAM_NOT_SET_STR	"4095"

#define DEBRIS_MASS             4.5
#define DEBRIS_TYPES		(8 * 4 * 4)

#define ENERGY_RANGE_FACTOR	(2.5/FUEL_SCALE_FACT)

/* map dimension limitation: ((0x7FFF - 1280) / 35) */
#define MAX_MAP_SIZE		900

#define WORM_BRAKE_FACTOR	1
#define WORMCOUNT		64

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#ifdef __GNUC__
#define	INLINE	inline
#else
#define INLINE
#endif /* __GNUC__ */

#if defined(ultrix) || defined(AIX)
/* STDRUP_OBJ should be uncomented in Makefile also */
extern char* strdup(const char*);
#endif

#endif

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#ifndef _WINDOWS
# include <sys/types.h>  /* freebsd for in.h to work */
# include <sys/socket.h> /* freebsd for in.h to work */
# include <netinet/in.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
# include "../common/NT/winNet.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "saudio.h"
#include "item.h"
#include "error.h"
#include "walls.h"
#include "click.h"
#include "objpos.h"

#include "const.h"
#include "map.h"
#include "netserver.h"
#include "pack.h"
#include "srecord.h"

char walls_version[] = VERSION;



extern struct move_parameters mp;
DFLOAT wallBounceExplosionMult;
static char msg[MSG_LEN];

/* kps compatibility hacks - plz remove if you can */
void Walls_init_old(void);
void Move_object_old(object *obj);
void Move_player_old(int ind);
void Turn_player_old(int ind);

static void Walls_init_new(void);
static void Move_object_new(object *obj);
static void Move_player_new(int ind);
static void Turn_player_new(int ind);


/* polygon map related stuff */

/* start */

/* Maximum line length 32767-B_CLICKS, 30000 used in checks
 * There's a minimum map size to avoid "too much wrapping". A bit smaller
 * than that would cause rare errors for fast-moving things. I haven't
 * bothered to figure out what the limit is. 80k x 80k clicks should
 * be more than enough (probably...). */
#define B_SHIFT 11
#define B_CLICKS (1 << B_SHIFT)
#define B_MASK (B_CLICKS - 1)
#define CUTOFF (2 * BLOCK_CLICKS) /* Not sure about the optimum value */
#define MAX_MOVE 32000
#define SEPARATION_DIST 64
/* This must be increased if the ship corners are allowed to go farther
 * when turning! */
#define MAX_SHAPE_OFFSET (15 * CLICK)

#if ((-3) / 2 != -1) || ((-3) % 2 != -1)
#error "This code assumes that negative numbers round upwards."
#endif

struct collans {
    int line;
    int point;
    clvec moved;
};

struct tl2 {
    int base;
    int x;
    int y;
};

struct bline {
    clvec start;
    clvec delta;
    DFLOAT c;
    DFLOAT s;
    short group;
};

struct blockinfo {
    unsigned short distance;
    unsigned short *lines;
    unsigned short *points;
};

struct inside_block {
    short *y;
    short *lines;
    struct inside_block *next;
    short group;
    char base_value;
};

struct inside_block *inside_table;

struct test {
    double distance;
    int inside;
    struct tempy *y;
    struct templine *lines;
};

struct test *temparray;

shipobj ball_wire;

#define LINEY(X, Y, BASE, ARG)  (((Y)*(ARG)+(BASE))/(X))
#define SIDE(X, Y, LINE) (linet[(LINE)].delta.cy * (X) - linet[(LINE)].delta.cx * (Y))
#define SIGN(X) ((X) >= 0 ? 1 : -1)

struct bline *linet;
#define S_LINES 100 /* stupid hack */

/* kps - dynamic creation of groups asap! */
struct group groups[1000] = { /* !@# */
    {0, 0, 0, 0, NULL},
    {0, 0, 0, 0, NULL},
    {0, 0, 0, 0, NULL}};

struct blockinfo *blockline;
unsigned short *llist;
unsigned short *plist;
int num_lines = 0;
int num_polys = 0;
int num_groups = 0;
int mapx, mapy;

#if 1
#define can_hit(group, move) \
(((group)->hit_mask & (move)->hit_mask) ? false : \
 ((group)->hit_func == NULL ? true : (group)->hit_func(group, move)))
#else
bool can_hit(struct group *group, struct move *move)
{
    if (group->hit_mask & move->hit_mask)
	return false;
    if (group->hit_func == NULL)
	return true;
    return group->hit_func(group, move);
}
#endif


void Walls_init(void)
{
    /*
     * Always do the walls_init_new(), since we treat the
     * block map as a polygon map.
     */
    if (!is_polygon_map)
	Walls_init_old();
    Walls_init_new();
}

void Move_init(void)
{
    mp.click_width = PIXEL_TO_CLICK(World.width);
    mp.click_height = PIXEL_TO_CLICK(World.height);

    LIMIT(maxObjectWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxShieldedWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxUnshieldedWallBounceSpeed, 0, World.hypotenuse);

    /* kps - ng does not want the following 2 */
    LIMIT(maxShieldedWallBounceAngle, 0, 180);
    LIMIT(maxUnshieldedWallBounceAngle, 0, 180);

    LIMIT(playerWallBrakeFactor, 0, 1);
    LIMIT(objectWallBrakeFactor, 0, 1);
    LIMIT(objectWallBounceLifeFactor, 0, 1);
    LIMIT(wallBounceFuelDrainMult, 0, 1000);
    wallBounceExplosionMult = sqrt(wallBounceFuelDrainMult);

    /* kps - ng does not want the following 2 */
    mp.max_shielded_angle = (int)(maxShieldedWallBounceAngle * RES / 360);
    mp.max_unshielded_angle = (int)(maxUnshieldedWallBounceAngle * RES / 360);

    mp.obj_bounce_mask = 0;
    if (sparksWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SPARK);
    }
    if (debrisWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_DEBRIS);
    }
    if (shotsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SHOT|OBJ_CANNON_SHOT);
    }
    if (itemsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_ITEM);
    }
    if (missilesWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT);
    }
    if (minesWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_MINE);
    }
    if (ballsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_BALL);
    }
    if (asteroidsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_ASTEROID);
    }

    mp.obj_cannon_mask = (KILLING_SHOTS) | OBJ_MINE | OBJ_SHOT | OBJ_PULSE |
			OBJ_SMART_SHOT | OBJ_TORPEDO | OBJ_HEAT_SHOT |
			OBJ_ASTEROID;
    if (cannonsUseItems)
	mp.obj_cannon_mask |= OBJ_ITEM;
    mp.obj_target_mask = mp.obj_cannon_mask | OBJ_BALL | OBJ_SPARK;
    mp.obj_treasure_mask = mp.obj_bounce_mask | OBJ_BALL | OBJ_PULSE;
}

void Move_object(object *obj)
{
    if (is_polygon_map || !useOldCode)
	Move_object_new(obj);
    else
	Move_object_old(obj);
}

void Move_player(int ind)
{
    if (is_polygon_map || !useOldCode)
	Move_player_new(ind);
    else
	Move_player_old(ind);
}

void Turn_player(int ind)
{
    if (is_polygon_map || !useOldCode)
	Turn_player_new(ind);
    else
	Turn_player_old(ind);
}


void Cannon_dies(int ind, player *pl)
{
    cannon_t		*cannon = &World.cannon[ind];
    int			cx = cannon->pos.cx;
    int			cy = cannon->pos.cy;
    int			killer = -1;

    Cannon_remove_from_map(ind);
    Cannon_throw_items(ind);
    Cannon_init(ind);
    sound_play_sensors(cx, cy, CANNON_EXPLOSION_SOUND);
    Make_debris(
	/* pos.cx, pos.cy   */ cx, cy,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ NO_ID,
	/* owner team	  */ cannon->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 4.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 6,
	/* num debris     */ 20 + 20 * rfrac(),
	/* min,max dir    */ (int)(cannon->dir - (RES * 0.2)), (int)(cannon->dir + (RES * 0.2)),
	/* min,max speed  */ 20, 50,
	/* min,max life   */ 8 * TIME_FACT, 68 * TIME_FACT
	);
    Make_wreckage(
	/* pos.cx, pos.cy   */ cx, cy,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ NO_ID,
	/* owner team	  */ cannon->team,
	/* min,max mass   */ 3.5, 23,
	/* total mass     */ 28,
	/* status         */ GRAVITY,
	/* color          */ WHITE,
	/* max wreckage   */ 10,
	/* min,max dir    */ (int)(cannon->dir - (RES * 0.2)), (int)(cannon->dir + (RES * 0.2)),
	/* min,max speed  */ 10, 25,
	/* min,max life   */ 8 * TIME_FACT, 68 * TIME_FACT
	);

    if (pl) {
	killer = GetInd[pl->id];
	if (cannonPoints > 0) {
	    if (BIT(World.rules->mode, TEAM_PLAY)
		&& teamCannons) {
		TEAM_SCORE(cannon->team, -cannonPoints);
	    }
	    if (pl->score <= cannonMaxScore
		&& !(BIT(World.rules->mode, TEAM_PLAY)
		     && pl->team == cannon->team)) {
		SCORE(killer, cannonPoints, cannon->pos.cx,
					    cannon->pos.cy, "");
	    }
	}
    }
}


void Object_hits_target(int ind, object *obj, long player_cost)
{
    target_t		*targ = &World.targets[ind];
    int			j,
			killer;
    DFLOAT		sc, por,
			win_score = 0,
			lose_score = 0;
    int			win_team_members = 0,
			lose_team_members = 0,
			somebody_flag = 0,
			targets_remaining = 0,
			targets_total = 0;
    DFLOAT 		drainfactor;

    /* a normal shot or a direct mine hit work, cannons don't */
    /* KK: should shots/mines by cannons of opposing teams work? */
    /* also players suiciding on target will cause damage */
    if (!BIT(obj->type, KILLING_SHOTS|OBJ_MINE|OBJ_PULSE|OBJ_PLAYER)) {
	return;
    }
    if (obj->id <= 0) {
	return;
    }
    killer = GetInd[obj->id];
    if (targ->team == obj->team) {
	return;
    }

    switch(obj->type) {
    case OBJ_SHOT:
	if (shotHitFuelDrainUsesKineticEnergy) {
	    drainfactor = VECTOR_LENGTH(obj->vel);
	    drainfactor = (drainfactor * drainfactor * ABS(obj->mass))
			  / (ShotsSpeed * ShotsSpeed * ShotsMass);
	} else {
	    drainfactor = 1.0f;
	}
	targ->damage += (int)(ED_SHOT_HIT * drainfactor * SHOT_MULT(obj));
	break;
    case OBJ_PULSE:
	targ->damage += (int)(ED_LASER_HIT);
	break;
    case OBJ_SMART_SHOT:
    case OBJ_TORPEDO:
    case OBJ_HEAT_SHOT:
	if (!obj->mass) {
	    /* happens at end of round reset. */
	    return;
	}
	if (BIT(obj->mods.nuclear, NUCLEAR)) {
	    targ->damage = 0;
	}
	else {
	    targ->damage += (int)(ED_SMART_SHOT_HIT / (obj->mods.mini + 1));
	}
	break;
    case OBJ_MINE:
	if (!obj->mass) {
	    /* happens at end of round reset. */
	    return;
	}
	targ->damage -= TARGET_DAMAGE / (obj->mods.mini + 1);
	break;
    case OBJ_PLAYER:
	if (player_cost <= 0 || player_cost > TARGET_DAMAGE / 4)
	    player_cost = TARGET_DAMAGE / 4;
	targ->damage -= player_cost;
	break;

    default:
	/*???*/
	break;
    }

    targ->conn_mask = 0;
    targ->last_change = frame_loops;
    if (targ->damage > 0)
	return;

    Target_remove_from_map(ind);

    Make_debris(
	/* pos.cx, pos.cy   */ targ->pos.cx, targ->pos.cy,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ NO_ID,
	/* owner team	  */ targ->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 4.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 6,
	/* num debris     */ 75 + 75 * rfrac(),
	/* min,max dir    */ 0, RES-1,
	/* min,max speed  */ 20, 70,
	/* min,max life   */ 10 * TIME_FACT, 100 * TIME_FACT
	);

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for (j = 0; j < NumPlayers; j++) {
	    if (IS_TANK_IND(j)
		|| (BIT(Players[j]->status, PAUSE)
		    && Players[j]->count <= 0)
		|| (BIT(Players[j]->status, GAME_OVER)
		    && Players[j]->mychar == 'W'
		    && Players[j]->score == 0)) {
		continue;
	    }
	    if (Players[j]->team == targ->team) {
		lose_score += Players[j]->score;
		lose_team_members++;
		if (BIT(Players[j]->status, GAME_OVER) == 0) {
		    somebody_flag = 1;
		}
	    }
	    else if (Players[j]->team == Players[killer]->team) {
		win_score += Players[j]->score;
		win_team_members++;
	    }
	}
    }
    if (somebody_flag) {
	for (j = 0; j < World.NumTargets; j++) {
	    if (World.targets[j].team == targ->team) {
		targets_total++;
		if (World.targets[j].dead_time == 0) {
		    targets_remaining++;
		}
	    }
	}
    }
    if (!somebody_flag) {
	return;
    }

    sound_play_sensors(PIXEL_TO_CLICK(x), PIXEL_TO_CLICK(y),
		       DESTROY_TARGET_SOUND);

    if (targets_remaining > 0) {
	sc = Rate(Players[killer]->score, CANNON_SCORE)/4;
	sc = sc * (targets_total - targets_remaining) / (targets_total + 1);
	if (sc >= 0.01) {
	    SCORE(killer, sc, targ->pos.cx, targ->pos.cy, "Target: ");
	}
	/*
	 * If players can't collide with their own targets, we
	 * assume there are many used as shields.  Don't litter
	 * the game with the message below.
	 */
	if (targetTeamCollision && targets_total < 10) {
	    sprintf(msg, "%s blew up one of team %d's targets.",
		    Players[killer]->name, (int) targ->team);
	    Set_message(msg);
	}
	return;
    }

    sprintf(msg, "%s blew up team %d's %starget.",
	    Players[killer]->name,
	    (int) targ->team,
	    (targets_total > 1) ? "last " : "");
    Set_message(msg);

    if (targetKillTeam) {
	Rank_AddTargetKill(Players[killer]);
    }

    sc  = Rate(win_score, lose_score);
    por = (sc*lose_team_members)/win_team_members;

    for (j = 0; j < NumPlayers; j++) {
	if (IS_TANK_IND(j)
	    || (BIT(Players[j]->status, PAUSE)
		&& Players[j]->count <= 0)
	    || (BIT(Players[j]->status, GAME_OVER)
		&& Players[j]->mychar == 'W'
		&& Players[j]->score == 0)) {
	    continue;
	}
	if (Players[j]->team == targ->team) {
	    if (targetKillTeam
		&& targets_remaining == 0
		&& !BIT(Players[j]->status, KILLED|PAUSE|GAME_OVER))
		SET_BIT(Players[j]->status, KILLED);
	    SCORE(j, -sc, targ->pos.cx, targ->pos.cy, "Target: ");
	}
	else if (Players[j]->team == Players[killer]->team &&
		 (Players[j]->team != TEAM_NOT_SET || j == killer)) {
	    SCORE(j, por, targ->pos.cx, targ->pos.cy, "Target: ");
	}
    }
}



void Object_crash(object *obj, struct move *move, int crashtype, int item_id)
{
    player *pl = NULL;

    /* kps hack - is this ok ? */
    if (obj->id != NO_ID)
	pl = Players[GetInd[obj->id]];

    switch (crashtype) {

    case CrashWormHole:
    default:
	break;

    case CrashTreasure:
	/*
	 * Ball type has already been handled.
	 */
	if (obj->type == OBJ_BALL) {
	    break;
	}
	obj->life = 0;
	break;

    case CrashTarget:
	obj->life = 0;
	Object_hits_target(item_id, obj, -1);
	break;

    case CrashWall:
	obj->life = 0;
	/* add sparks ??? */
	break;

    case CrashUniverse:
	obj->life = 0;
	break;

    case CrashCannon:
	obj->life = 0;
	if (BIT(obj->type, OBJ_ITEM)) {
	    Cannon_add_item(item_id, obj->info, obj->count);
	} else {
	    if (!BIT(World.cannon[item_id].used, HAS_EMERGENCY_SHIELD)) {
		if (World.cannon[item_id].item[ITEM_ARMOR] > 0)
		    World.cannon[item_id].item[ITEM_ARMOR]--;
		else
		    Cannon_dies(item_id, pl);
	    }
	}
	break;

    case CrashUnknown:
	obj->life = 0;
	break;
    }
}


void Player_crash(player *pl, struct move *move, int crashtype,
		  int item_id, int pt)
{
    int			ind = GetInd[pl->id];
    const char		*howfmt = NULL;
    const char          *hudmsg = NULL;

    msg[0] = '\0';

    switch (crashtype) {

    default:
    case NotACrash:
	warn("Unrecognized crash %d", crashtype);
	break;

    case CrashWormHole:
	SET_BIT(pl->status, WARPING);
	pl->wormHoleHit = item_id;
	break;

    case CrashWall:
	howfmt = "%s crashed%s against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashWallSpeed:
	howfmt = "%s smashed%s against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashWallNoFuel:
	howfmt = "%s smacked%s against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashWallAngle:
	howfmt = "%s was trashed%s against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashTarget:
	howfmt = "%s smashed%s against a target";
	hudmsg = "[Target]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	Object_hits_target(item_id, (object *)pl, -1);
	break;

    case CrashTreasure:
	howfmt = "%s smashed%s against a treasure";
	hudmsg = "[Treasure]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashCannon:
	if (BIT(pl->used, HAS_SHIELD|HAS_EMERGENCY_SHIELD)
	    != (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
	    howfmt = "%s smashed%s against a cannon";
	    hudmsg = "[Cannon]";
	    sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_CANNON_SOUND);
	}
	if (!BIT(World.cannon[item_id].used, HAS_EMERGENCY_SHIELD)) {
	    Cannon_dies(item_id, pl);
	}
	break;

    case CrashUniverse:
	howfmt = "%s left the known universe%s";
	hudmsg = "[Universe]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashUnknown:
	howfmt = "%s slammed%s into a programming error";
	hudmsg = "[Bug]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;
    }

    if (howfmt && hudmsg) {
	player		*pushers[MAX_RECORDED_SHOVES];
	int		cnt[MAX_RECORDED_SHOVES];
	int		num_pushers = 0;
	int		total_pusher_count = 0;
	DFLOAT		total_pusher_score = 0;
	int		i, j;
	DFLOAT		sc;

	SET_BIT(pl->status, KILLED);
	move->delta.cx = 0;
	move->delta.cy = 0;
	sprintf(msg, howfmt, pl->name, (!pt) ? " head first" : "");

	/* get a list of who pushed me */
	for (i = 0; i < MAX_RECORDED_SHOVES; i++) {
	    shove_t *shove = &pl->shove_record[i];
	    if (shove->pusher_id == NO_ID) {
		continue;
	    }
	    if (shove->time < frame_loops - 20) {
		continue;
	    }
	    for (j = 0; j < num_pushers; j++) {
		if (shove->pusher_id == pushers[j]->id) {
		    cnt[j]++;
		    break;
		}
	    }
	    if (j == num_pushers) {
		pushers[num_pushers++] = Players[GetInd[shove->pusher_id]];
		cnt[j] = 1;
	    }
	    total_pusher_count++;
	    total_pusher_score += pushers[j]->score;
	}
	if (num_pushers == 0) {
	    sc = Rate(WALL_SCORE, pl->score);
	    SCORE(ind, -sc, pl->pos.cx, pl->pos.cy, hudmsg);
	    strcat(msg, ".");
	    Set_message(msg);
	}
	else {
	    int		msg_len = strlen(msg);
	    char	*msg_ptr = &msg[msg_len];
	    int		average_pusher_score = total_pusher_score
						/ total_pusher_count;

	    for (i = 0; i < num_pushers; i++) {
		player		*pusher = pushers[i];
		const char	*sep = (!i) ? " with help from "
					    : (i < num_pushers - 1) ? ", "
					    : " and ";
		int		sep_len = strlen(sep);
		int		name_len = strlen(pusher->name);

		if (msg_len + sep_len + name_len + 2 < sizeof msg) {
		    strcpy(msg_ptr, sep);
		    msg_len += sep_len;
		    msg_ptr += sep_len;
		    strcpy(msg_ptr, pusher->name);
		    msg_len += name_len;
		    msg_ptr += name_len;
		}
		sc = cnt[i] * Rate(pusher->score, pl->score)
				    * shoveKillScoreMult / total_pusher_count;
		SCORE(GetInd[pusher->id], sc,
		      pl->pos.cx, pl->pos.cy, pl->name);
		if (i >= num_pushers - 1) {
		    pusher->kills++;
		}

	    }
	    sc = Rate(average_pusher_score, pl->score)
		       * shoveKillScoreMult;
	    SCORE(ind, -sc, pl->pos.cx, pl->pos.cy, "[Shove]");

	    strcpy(msg_ptr, ".");
	    Set_message(msg);

	    /* Robots will declare war on anyone who shoves them. */
	    i = (int)(rfrac() * num_pushers);
	    Robot_war(ind, GetInd[pushers[i]->id]);
	}
    }

    if (BIT(pl->status, KILLED)
	&& pl->score < 0
	&& IS_ROBOT_PTR(pl)) {
	pl->home_base = 0;
	Pick_startpos(ind);
    }
}




static void *ralloc(void *ptr, size_t size)
{
    if (!(ptr = realloc(ptr, size))) {
	warn("Realloc failed.");
	exit(1);
    }
    return ptr;
}

static unsigned short *Shape_lines(const shipobj *shape, int dir)
{
    int p;
    static unsigned short foo[100];
    static const shipobj *lastshape;
    static int lastdir;
    const int os = num_lines;

    /* linet[i].group MUST BE INITIALIZED TO 0 */

    if (shape == lastshape && dir == lastdir)
	return foo;

    lastshape = shape;
    lastdir = dir;

    for (p = 0; p < shape->num_points; p++) {
	linet[p + os].start.cx = -shape->pts[p][dir].cx;
	linet[p + os].start.cy = -shape->pts[p][dir].cy;
    }
    for (p = 0; p < shape->num_points - 1; p++) {
	linet[p + os].delta.cx
	    = linet[p + os + 1].start.cx - linet[p + os].start.cx;
	linet[p + os].delta.cy
	    = linet[p + os + 1].start.cy - linet[p + os].start.cy;
    }
    linet[p + os].delta.cx = linet[os].start.cx - linet[p + os].start.cx;
    linet[p + os].delta.cy = linet[os].start.cy - linet[p + os].start.cy;
    for (p = 0; p < shape->num_points; p++)
	foo[p] = p + os;
    foo[p] = 65535;
    return foo;
}


static int Bounce_object(object *obj, struct move *move, int line, int point)
{
    DFLOAT fx, fy;
    DFLOAT c, s;
    int group, type, item_id;

    group = linet[line >= num_lines ? point : line].group;
    type = groups[group].type;
    item_id = groups[group].item_id;

    if (obj->collmode == 1) {
	fx = ABS(obj->vel.x) + ABS(obj->vel.y);
	/* If fx<1, there is practically no movement. Object
	   collision detection can ignore the bounce. */
	if (fx > 1) {
	    obj->wall_time = 1 -
		CLICK_TO_FLOAT(ABS(move->delta.cx) + ABS(move->delta.cy)) / fx;
	    obj->collmode = 2;
	}
    }

    if (type == TREASURE) {
	if (obj->type == OBJ_BALL)
	    Ball_hits_goal(BALL_PTR(obj), group);
	obj->life = 0;
	return 0;
    }

    /* kps hack */
    if (type == TARGET) {
	obj->life = 0;
	Object_hits_target(item_id, obj, -1);
	return 0;
    }
    /* kps hack */

    /* kps hack */
    if (type == CANNON) {
	Object_crash(obj, move, CrashCannon, item_id);
	return 0;
    }
    /* kps hack */

#if 1
    /* kps hack */
    if (type == WORMHOLE) {
	/* kps - ??? */
	Object_crash(obj, move, CrashWormHole, item_id);
	return 0;
    }
    /* kps hack */
#endif
    if (!BIT(mp.obj_bounce_mask, obj->type)) {
	obj->life = 0;
	return 0;
    }

    if (obj->type != OBJ_BALL) {
	obj->life = (long)(obj->life * objectWallBounceLifeFactor);
	if (obj->life <= 0)
	    return 0;
    }
    /*
     * Any bouncing sparks are no longer owner immune to give
     * "reactive" thrust.  This is exactly like ground effect
     * in the real world.  Very useful for stopping against walls.
     *
     * If the FROMBOUNCE bit is set the spark was caused by
     * the player bouncing of a wall and thus although the spark
     * should bounce, it is not reactive thrust otherwise wall
     * bouncing would cause acceleration of the player.
     */
    if (sqr(obj->vel.x) + sqr(obj->vel.y) > sqr(maxObjectWallBounceSpeed)){
	obj->life = 0;
	return 0;
    }
    if (!BIT(obj->status, FROMBOUNCE) && BIT(obj->type, OBJ_SPARK))
	CLR_BIT(obj->status, OWNERIMMUNE);


    if (line >= num_lines) {
	DFLOAT x, y, l2;
	x = linet[line].delta.cx;
	y = linet[line].delta.cy;
	l2 = (x*x + y*y);
	c = (x*x - y*y) / l2;
	s = 2*x*y / l2;
    }
    else {
	c = linet[line].c;
	s = linet[line].s;
    }
    fx = move->delta.cx * c + move->delta.cy * s;
    fy = move->delta.cx * s - move->delta.cy * c;
    move->delta.cx = fx * objectWallBrakeFactor;
    move->delta.cy = fy * objectWallBrakeFactor;
    fx = obj->vel.x * c + obj->vel.y * s;
    fy = obj->vel.x * s - obj->vel.y * c;
    obj->vel.x = fx * objectWallBrakeFactor;
    obj->vel.y = fy * objectWallBrakeFactor;
    if (obj->collmode == 2)
	obj->collmode = 3;
    return 1;
}



static void Bounce_player(player *pl, struct move *move, int line, int point)
{
    DFLOAT fx, fy;
    DFLOAT c, s;
    int group, type, item_id;

    if (line >= num_lines) {
	DFLOAT x, y, l2;
	x = linet[line].delta.cx;
	y = linet[line].delta.cy;
	l2 = (x*x + y*y);
	c = (x*x - y*y) / l2;
	s = 2*x*y / l2;
	group = linet[point].group;
    }
    else {
	group = linet[line].group;
	c = linet[line].c;
	s = linet[line].s;
    }
    type = groups[group].type;
    item_id = groups[group].item_id;
    if (type == TREASURE) {
	Player_crash(pl, move, CrashTreasure, NO_ID, 1);
	return;
    }
    /* kps hack - fix this */
    if (type == TARGET) {
	Player_crash(pl, move, CrashTarget, item_id, 1);
	return;
    }
    /* kps hack */

    /* kps hack */
    if (type == WORMHOLE) {
	Player_crash(pl, move, CrashWormHole, item_id, 1);
	return;
    }
    /* kps hack */

    /* kps hack */
    if (type == CANNON) {
	Player_crash(pl, move, CrashCannon, item_id, 1);
	return;
    }
    /* kps hack */

    pl->last_wall_touch = frame_loops;
    {
	DFLOAT	speed = VECTOR_LENGTH(pl->vel);
	int	v = (int) speed >> 2;
	int	m = (int) (pl->mass - pl->emptymass * 0.75f);
	DFLOAT	b = 1 - 0.5f * playerWallBrakeFactor;
	long	cost = (long) (b * m * v);
	DFLOAT	max_speed = BIT(pl->used, HAS_SHIELD)
		? maxShieldedWallBounceSpeed
		: maxUnshieldedWallBounceSpeed;

	if (BIT(pl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	    == (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
	    max_speed = 100;
	}
	if (speed > max_speed) {
	    Player_crash(pl, move, CrashWallSpeed, NO_ID, 1);
	    return;
	}

	/*
	 * Small explosion and fuel loss if survived a hit on a wall.
	 * This doesn't affect the player as the explosion is sparks
	 * which don't collide with player.
	 */
	cost *= 0.9; /* used to depend on bounce angle, .5 .. 1.0 */
	if (BIT(pl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	    != (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
	    Add_fuel(&pl->fuel, (long)(-((cost << FUEL_SCALE_BITS)
					 * wallBounceFuelDrainMult)));
	    Item_damage(GetInd[pl->id], wallBounceDestroyItemProb);
	}
	if (!pl->fuel.sum && wallBounceFuelDrainMult != 0) {
	    Player_crash(pl, move, CrashWallNoFuel, NO_ID, 1);
	    return;
	}
/* !@# I didn't implement wall direction calculation yet. */
	if (cost) {
#if 0
	    int intensity = (int)(cost * wallBounceExplosionMult);
	    Make_debris(
			/* pos.cx, pos.cy   */ pl->pos.cx, pl->pos.cy,
			/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
			/* owner id       */ pl->id,
			/* owner team	  */ pl->team,
			/* kind           */ OBJ_SPARK,
			/* mass           */ 3.5,
			/* status         */ GRAVITY | OWNERIMMUNE | FROMBOUNCE,
			/* color          */ RED,
			/* radius         */ 1,
			/* min,max debris */ intensity>>1, intensity,
			/* min,max dir    */ wall_dir - (RES/4), wall_dir + (RES/4),
			/* min,max speed  */ 20, 20 + (intensity>>2),
			/* min,max life   */ 10, 10 + (intensity>>1)
					     );
#endif
	    sound_play_sensors(pl->pos.cx, pl->pos.cy,
			       PLAYER_BOUNCED_SOUND);
#if 0
	    /* I'll leave this here until i implement targets */
	    if (ms[worst].target >= 0) {
		cost <<= FUEL_SCALE_BITS;
		cost = (long)(cost * (wallBounceFuelDrainMult / 4.0));
		Object_hits_target_old(&ms[worst], cost);
	    }
#endif
	}
    }
    fx = move->delta.cx * c + move->delta.cy * s;
    fy = move->delta.cx * s - move->delta.cy * c;
    move->delta.cx = fx * playerWallBrakeFactor;
    move->delta.cy = fy * playerWallBrakeFactor;
    fx = pl->vel.x * c + pl->vel.y * s;
    fy = pl->vel.x * s - pl->vel.y * c;
    pl->vel.x = fx * playerWallBrakeFactor;
    pl->vel.y = fy * playerWallBrakeFactor;
}

static int Away(struct move *move, int line)
{
    int i, dx, dy, lsx, lsy, res;
    unsigned short *lines;

    lsx = linet[line].start.cx - move->start.cx;
    lsy = linet[line].start.cy - move->start.cy;
    lsx = CENTER_XCLICK(lsx);
    lsy = CENTER_YCLICK(lsy);

    if (ABS(linet[line].delta.cx) >= ABS(linet[line].delta.cy)) {
	dx = 0;
	dy = -SIGN(linet[line].delta.cx);
    }
    else {
	dy = 0;
	dx = SIGN(linet[line].delta.cy);
    }

    if ((ABS(lsx) > SEPARATION_DIST || ABS(lsy) > SEPARATION_DIST)
	&& (ABS(lsx + linet[line].delta.cx) > SEPARATION_DIST
	    || ABS(lsy + linet[line].delta.cy) > SEPARATION_DIST)) {
	move->start.cx = WRAP_XCLICK(move->start.cx + dx);
	move->start.cy = WRAP_YCLICK(move->start.cy + dy);
	return -1;
    }

    lines = blockline[(move->start.cx >> B_SHIFT)
		     + mapx * (move->start.cy >> B_SHIFT)].lines;
    while ( (i = *lines++) != 65535) {
	if (i == line)
	    continue;
	if (linet[i].group
	    /*&& (groups[linet[i].group].hit_mask & move->hit_mask))*/
	    && (!can_hit(&groups[linet[i].group], move)))
	    continue;

	lsx = linet[i].start.cx - move->start.cx;
	lsy = linet[i].start.cy - move->start.cy;
	lsx = CENTER_XCLICK(lsx);
	lsy = CENTER_YCLICK(lsy);

	if ((ABS(lsx) > SEPARATION_DIST || ABS(lsy) > SEPARATION_DIST)
	    && (ABS(lsx + linet[i].delta.cx) > SEPARATION_DIST
		|| ABS(lsy + linet[i].delta.cy) > SEPARATION_DIST))
	    continue;

	if (lsx < dx && lsx + linet[i].delta.cx < dx)
	    continue;
	if (lsx > dx && lsx + linet[i].delta.cx > dx)
	    continue;
	if (lsy < dy && lsy + linet[i].delta.cy < dy)
	    continue;
	if (lsy > dy && lsy + linet[i].delta.cy > dy)
	    continue;

	if ((res = SIDE(lsx - dx, lsy - dy, i)) == 0
	    || res > 0 != SIDE(lsx, lsy, i) > 0) {
	    if (res) {
		if (lsx < 0 && lsx + linet[i].delta.cx < 0)
		    continue;
		if (lsx > 0 && lsx + linet[i].delta.cx > 0)
		    continue;
		if (lsy < 0 && lsy + linet[i].delta.cy < 0)
		    continue;
		if (lsy > 0 && lsy + linet[i].delta.cy > 0)
		    continue;
	    }
	    return i;
	}
    }

    move->start.cx = WRAP_XCLICK(move->start.cx + dx);
    move->start.cy = WRAP_YCLICK(move->start.cy + dy);
    return -1;
}

static int Shape_move1(int dx, int dy, struct move *move,
		       const shipobj *shape, int dir, int *line, int *point)
{
    int i, p, lsx, lsy, res;
    unsigned short *lines, *points;
    int block;

    block = (move->start.cx >> B_SHIFT) + mapx * (move->start.cy >> B_SHIFT);
    for (p = 0; p < shape->num_points; p++) {
	lines = blockline[block].lines;
	/* Can use the same block for all points because the block of the
	   center point contains lines for their start & end positions. */
	while ( (i = *lines++) != 65535) {
	    if (linet[i].group
		/*&& (groups[linet[i].group].hit_mask & move->hit_mask))*/
		&& (!can_hit(&groups[linet[i].group], move)))
		continue;
	    lsx = linet[i].start.cx - move->start.cx - shape->pts[p][dir].cx;
	    lsy = linet[i].start.cy - move->start.cy - shape->pts[p][dir].cy;
	    lsx = CENTER_XCLICK(lsx);
	    lsy = CENTER_YCLICK(lsy);

	    if (lsx < dx && lsx + linet[i].delta.cx < dx)
		continue;
	    if (lsx > dx && lsx + linet[i].delta.cx > dx)
		continue;
	    if (lsy < dy && lsy + linet[i].delta.cy < dy)
		continue;
	    if (lsy > dy && lsy + linet[i].delta.cy > dy)
		continue;

	    if ( (res = SIDE(lsx - dx, lsy - dy, i)) == 0
		 || res > 0 != SIDE(lsx, lsy, i) > 0) {
		if (res) {
		    if (lsx < 0 && lsx + linet[i].delta.cx < 0)
			continue;
		    if (lsx > 0 && lsx + linet[i].delta.cx > 0)
			continue;
		    if (lsy < 0 && lsy + linet[i].delta.cy < 0)
			continue;
		    if (lsy > 0 && lsy + linet[i].delta.cy > 0)
			continue;
		}
		*line = i;
		return 0;
	    }
	}
    }

    points = blockline[block].points;
    while ( (p = *points++) != 65535) {
	if (linet[p].group
	    /*&& (groups[linet[p].group].hit_mask & move->hit_mask))*/
	    && (!can_hit(&groups[linet[p].group], move)))
	    continue;
	lines = Shape_lines(shape, dir);
	while ( (i = *lines++) != 65535) {
	    lsx = linet[i].start.cx + (linet[p].start.cx - move->start.cx);
	    lsy = linet[i].start.cy + (linet[p].start.cy - move->start.cy);
	    lsx = CENTER_XCLICK(lsx);
	    lsy = CENTER_YCLICK(lsy);

	    if (lsx < dx && lsx + linet[i].delta.cx < dx)
		continue;
	    if (lsx > dx && lsx + linet[i].delta.cx > dx)
		continue;
	    if (lsy < dy && lsy + linet[i].delta.cy < dy)
		continue;
	    if (lsy > dy && lsy + linet[i].delta.cy > dy)
		continue;

	    if ( (res = SIDE(lsx - dx, lsy - dy, i)) == 0
		 || res > 0 != SIDE(lsx, lsy, i) > 0) {
		if (res) {
		    if (lsx < 0 && lsx + linet[i].delta.cx < 0)
			continue;
		    if (lsx > 0 && lsx + linet[i].delta.cx > 0)
			continue;
		    if (lsy < 0 && lsy + linet[i].delta.cy < 0)
			continue;
		    if (lsy > 0 && lsy + linet[i].delta.cy > 0)
			continue;
		}
		*line = i;
		*point = p;
		return 0;
	    }
	}
    }

    move->start.cx = WRAP_XCLICK(move->start.cx + dx);
    move->start.cy = WRAP_YCLICK(move->start.cy + dy);
    return 1;
}

static int Shape_away(struct move *move, const shipobj *shape,
		      int dir, int line, int *rline, int *rpoint)
{
    int dx, dy;

    if (ABS(linet[line].delta.cx) >= ABS(linet[line].delta.cy)) {
	dx = 0;
	dy = -SIGN(linet[line].delta.cx);
    }
    else {
	dy = 0;
	dx = SIGN(linet[line].delta.cy);
    }
    return Shape_move1(dx, dy, move, shape, dir, rline, rpoint);
}

static int Lines_check(int msx, int msy, int mdx, int mdy, int *mindone,
		       const unsigned short *lines, int chx, int chy,
		       int chxy, const struct move *move, int *minline,
		       int *height)
{
    int lsx, lsy, ldx, ldy, temp, bigger, start, end, i, x, sy, ey, prod;
    int mbase = mdy >> 1, hit = 0;

    while ( (i = *lines++) != 65535) {
	if (linet[i].group
	    /*&& (groups[linet[i].group].hit_mask & move->hit_mask))*/
	    && (!can_hit(&groups[linet[i].group], (struct move *)move)))
	    continue;
	lsx = linet[i].start.cx;
	lsy = linet[i].start.cy;
	ldx = linet[i].delta.cx;
	ldy = linet[i].delta.cy;

	if (chx) {
	    lsx = -lsx;
	    ldx = -ldx;
	}
	if (chy) {
	    lsy = -lsy;
	    ldy = -ldy;
	}
	if (chxy) {
	    temp = ldx;
	    ldx = ldy;
	    ldy = temp;
	    temp = lsx;
	    lsx = lsy;
	    lsy = temp;
	}
	lsx -= msx;
	lsy -= msy;
	if (chxy) {
	    lsx = CENTER_YCLICK(lsx);
	    lsy = CENTER_XCLICK(lsy);
	}
	else {
	    lsx = CENTER_XCLICK(lsx);
	    lsy = CENTER_YCLICK(lsy);
	}
	if (*height < lsy + (ldy < 0 ? ldy : 0))
	    continue;
	if (0 > lsy + (ldy < 0 ? 0 : ldy))
	    continue;

	lsx = CENTER_XCLICK(lsx);

	if (ldx < 0) {
	    lsx += ldx;
	    ldx = -ldx;
	    lsy += ldy;
	    ldy = -ldy;
	}

	start = MAX(0, lsx);
	end = MIN(*mindone + 1, lsx + ldx);
	if (start > end)
	    continue;

	sy = LINEY(mdx, mdy, mbase, start);
	prod = (start - lsx) * ldy - (sy - lsy) * ldx;

	if (!prod) {
	    if (!ldx && (lsy + (ldy < 0 ? ldy : 0) > sy ||
			 lsy + (ldy < 0 ? 0 : ldy) < sy))
		continue;
	    start--;
	}
	else {
	    bigger = prod > 0;
	    ey = LINEY(mdx, mdy, mbase, end);
	    if ( ABS(prod) >= ldx
		 && ABS( (prod = (end - lsx) * ldy - (ey - lsy) * ldx) )
		 >= ldx && prod > 0 == bigger)
		continue;
	    {
		int schs, sche;
		double diff = ((double)(-mbase)/mdx-(double)(lsx)*ldy/ldx+lsy);
		double diff2 = (double)mdy / mdx - (double)ldy / ldx;

		if (ABS(diff2) < 1. / (50000.*50000)) {
		    if (diff > 0 || diff < -1)
			continue;
		    else {
			schs = start + 1;
			sche = end;
		    }
		}
		/* Can this float->int conversion cause overflows?
		 * If so, calculate min/max before conversion. */
		else if (diff2 < 0) {
		    schs = MAX(start + 1, (int) ((diff + 1) / diff2 + .9));
		    sche = MIN(end, (int) (diff / diff2 + 1.1));
		}
		else {
		    schs = MAX(start + 1, (int) (diff / diff2 + .9));
		    sche = MIN(end, (int) ((diff + 1) / diff2 + 1.1));
		}

		for (x = schs; x <= sche; x++)
		    if ( (prod = (x - lsx) * ldy
			  - (LINEY(mdx, mdy, mbase, x) - lsy) * ldx)
			 >= 0 != bigger || prod == 0)
			goto found;
		continue;
	    found:
		start = x - 1;
	    }
	}

	if (start < *mindone
	    || (start == *mindone
		&& *minline != -1
		&& SIDE(move->delta.cx, move->delta.cy, i) < 0)) {
	    hit = 1;
	    *mindone = start;
	    *minline = i;
	    *height = LINEY(mdx, mdy, mbase, start);
	}
    }
    return hit;
}

/* Do not call this with no movement. */
/* May not be called with point already on top of line.
   Maybe I should change that to allow lines that could be crossed. */
static void Move_point(const struct move *move, struct collans *answer)
{
    int minline, mindone, minheight;
    int block;
    int msx = move->start.cx, msy = move->start.cy;
    int mdx = move->delta.cx, mdy = move->delta.cy;
    int mbase;
    int chxy = 0, chx = 0, chy = 0;
    int x, temp;
    unsigned short *lines;

    block = (move->start.cx >> B_SHIFT) + mapx * (move->start.cy >> B_SHIFT);
    x = blockline[block].distance;
    lines = blockline[block].lines;

    if (mdx < 0) {
	mdx = -mdx;
	msx = -msx;
	chx = 1;
    }
    if (mdy < 0) {
	mdy = -mdy;
	msy = -msy;
	chy = 1;
    }
    if (mdx < mdy) {
	temp = mdx;
	mdx = mdy;
	mdy = temp;
	temp = msx;
	msx = msy;
	msy = temp;
	chxy = 1;
    }

    /* 46341*46341 overflows signed 32-bit int */
    if (mdx > 45000) {
      mdy = (float)mdy * 45000 / mdx; /* might overflow without float */
      mdx = 45000;
    }

    mindone = mdx;
    minheight = mdy;
    mdx++;
    mdy++;
    mbase = mdy >> 1;

    if (mindone > x) {
	if (x < MAX_MOVE) {
	    /* !@# change this so that the point always moves away from
	       the current block */
	    temp = msx > 0 ? B_CLICKS - (msx & B_MASK) : (-msx) & B_MASK;
	    temp = MIN(temp,
		       (msy > 0 ? B_CLICKS - (msy & B_MASK) : -msy & B_MASK));
	    x += temp;
	    x = MIN(x, MAX_MOVE);
	}
	if (mindone > x) {
	    mindone = x;
	    minheight = LINEY(mdx, mdy, mbase, mindone);
	}
    }
    minline = -1;

    Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy, chxy,
		move, &minline, &minheight);

    answer->line = minline;
    if (chxy) {
	temp = mindone;
	mindone = minheight;
	minheight = temp;
    }
    if (chx)
	mindone = -mindone;
    if (chy)
	minheight = -minheight;
    answer->moved.cx = mindone;
    answer->moved.cy = minheight;

    return;
}

/* Do not call this with no movement. */
/* May not be called with point already on top of line.
   maybe I should change that to allow lines which could be
   crossed. */
/* This could be sped up by a lot in several ways if needed.
 * For example, there's no need to consider all the points
 * separately if the ship is not close to a wall.
 */
static void Shape_move(const struct move *move, const shipobj *shape,
		       int dir, struct collans *answer)
{
    int minline, mindone, minheight, minpoint;
    int p, block;
    int msx = move->start.cx, msy = move->start.cy;
    int mdx = move->delta.cx, mdy = move->delta.cy;
    int mbase;
    int chxy = 0, chx = 0, chy = 0;
    int x, temp;
    unsigned short *lines;
    unsigned short *points;

    if (mdx < 0) {
	mdx = -mdx;
	chx = 1;
    }
    if (mdy < 0) {
	mdy = -mdy;
	chy = 1;
    }
    if (mdx < mdy) {
	temp = mdx;
	mdx = mdy;
	mdy = temp;
	chxy = 1;
    }

    /* 46341*46341 overflows signed 32-bit int */
    if (mdx > 45000) {
      mdy = (float)mdy * 45000 / mdx; /* might overflow without float */
      mdx = 45000;
    }

    mindone = mdx;
    minheight = mdy;

    mdx++;
    mdy++;
    mbase = mdy >> 1;
    minline = -1;
    minpoint = -1;

    for (p = 0; p < shape->num_points; p++) {
	msx = WRAP_XCLICK(move->start.cx + shape->pts[p][dir].cx);
	msy = WRAP_YCLICK(move->start.cy + shape->pts[p][dir].cy);
	block = (msx >> B_SHIFT) + mapx * (msy >> B_SHIFT);
	if (chx)
	    msx = -msx;
	if (chy)
	    msy = -msy;
	if (chxy) {
	    temp = msx;
	    msx = msy;
	    msy = temp;
	}

	x = blockline[block].distance;
	lines = blockline[block].lines;

	if (mindone > x) {
	    if (x < MAX_MOVE) {
		temp = msx > 0 ? B_CLICKS - (msx & B_MASK) : -msx & B_MASK;
		temp = MIN(temp,
			   (msy > 0 ?
			    B_CLICKS - (msy & B_MASK)
			    : -msy & B_MASK));
		x += temp;
		x = MIN(x, MAX_MOVE);
	    }
	    if (mindone > x) {
		mindone = x;
		minheight = LINEY(mdx, mdy, mbase, mindone);
	    }
	}

	if (Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy,
			chxy, move, &minline, &minheight))
	    minpoint = p;
    }

    block = (move->start.cx >> B_SHIFT) + mapx * (move->start.cy >> B_SHIFT);
    points = blockline[block].points;
    lines = Shape_lines(shape, dir);
    x = -1;
    while ( ( p = *points++) != 65535) {
	if (linet[p].group
	    /*&& (groups[linet[p].group].hit_mask & move->hit_mask))*/
	    && (!can_hit(&groups[linet[p].group], (struct move *)move)))
	    continue;
	msx = move->start.cx - linet[p].start.cx;
	msy = move->start.cy - linet[p].start.cy;
	if (chx)
	    msx = -msx;
	if (chy)
	    msy = -msy;
	if (chxy) {
	    temp = msx;
	    msx = msy;
	    msy = temp;
	}
	if (Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy,
			chxy, move, &minline, &minheight))
	    minpoint = p;
    }

    answer->point = minpoint;
    answer->line = minline;
    answer->point = minpoint;
    if (chxy) {
	temp = mindone;
	mindone = minheight;
	minheight = temp;
    }
    if (chx)
	mindone = -mindone;
    if (chy)
	minheight = -minheight;
    answer->moved.cx = mindone;
    answer->moved.cy = minheight;

    return;
}


static int Shape_morph(const shipobj *shape1, int dir1, const shipobj *shape2,
		       int dir2, int hitmask, const object *obj, int x, int y)
{
    struct collans answer;
    int i, p, xo1, xo2, yo1, yo2, xn1, xn2, yn1, yn2, xp, yp, s, t;
    unsigned short *points;
    struct move mv;

    mv.hit_mask = hitmask;
    mv.obj = (object *)obj;
    for (i = 0; i < shape1->num_points; i++) {
	mv.start.cx = x + shape1->pts[i][dir1].cx;
	mv.start.cy = y + shape1->pts[i][dir1].cy;
	mv.delta.cx = x + shape2->pts[i][dir2].cx - mv.start.cx;
	mv.delta.cy = y + shape2->pts[i][dir2].cy - mv.start.cy;
	mv.start.cx = WRAP_XCLICK(mv.start.cx);
	mv.start.cy = WRAP_YCLICK(mv.start.cy);
	while (mv.delta.cx || mv.delta.cy) {
	    Move_point(&mv, &answer);
	    if (answer.line != -1)
		return linet[answer.line].group;
	    mv.start.cx = WRAP_XCLICK(mv.start.cx + answer.moved.cx);
	    mv.start.cy = WRAP_YCLICK(mv.start.cy + answer.moved.cy);
	    mv.delta.cx -= answer.moved.cx;
	    mv.delta.cy -= answer.moved.cy;
	}
    }

    /* Convex shapes would be much easier. */
    points = blockline[(x >> B_SHIFT) + mapx * (y >> B_SHIFT)].points;
    while ( (p = *points++) != 65535) {
	if (linet[p].group
	    /*&& (groups[linet[p].group].hit_mask & hitmask))*/
	    && (!can_hit(&groups[linet[p].group], &mv)))
	    continue;
	xp = CENTER_XCLICK(linet[p].start.cx - x);
	yp = CENTER_YCLICK(linet[p].start.cy - y);
	xo1 = shape1->pts[shape1->num_points - 1][dir1].cx - xp;
	yo1 = shape1->pts[shape1->num_points - 1][dir1].cy - yp;
	xn1 = shape2->pts[shape1->num_points - 1][dir2].cx - xp;
	yn1 = shape2->pts[shape1->num_points - 1][dir2].cy - yp;
	t = 0;
	for (i = 0; i < shape1->num_points; i++) {
	    xo2 = shape1->pts[i][dir1].cx - xp;
	    yo2 = shape1->pts[i][dir1].cy - yp;
	    xn2 = shape2->pts[i][dir2].cx - xp;
	    yn2 = shape2->pts[i][dir2].cy - yp;

#define TEMPFUNC(X1, Y1, X2, Y2)                                           \
	    if ((X1) < 0) {                                                \
		if ((X2) >= 0) {                                           \
		    if ((Y1) > 0 && (Y2) >= 0)                             \
			t++;                                               \
		    else if (((Y1) >= 0 || (Y2) >= 0) &&                   \
			     (s = (X1)*((Y1)-(Y2))-(Y1)*((X1)-(X2))) >= 0){\
			if (s == 0)                                        \
			    return linet[p].group;                         \
			else                                               \
			    t++;                                           \
		    }                                                      \
		}                                                          \
	    }                                                              \
	    else                                                           \
		if ((X2) <= 0) {                                           \
		    if ((X2) == 0) {                                       \
			if ((Y2)==0||((X1)==0 && (((Y1)<=0 && (Y2)>= 0) || \
						 ((Y1) >= 0 && (Y2)<=0)))) \
			    return linet[p].group;                         \
		    }                                                      \
		    else if ((Y1) > 0 && (Y2) >= 0)                        \
			t++;                                               \
		    else if (((Y1) >= 0 || (Y2) >= 0) &&                   \
			     (s = (X1)*((Y1)-(Y2))-(Y1)*((X1)-(X2))) <= 0){\
			if (s == 0)                                        \
			    return linet[p].group;                         \
			else                                               \
			    t++;                                           \
		    }                                                      \
		}

	    TEMPFUNC(xo1, yo1, xn1, yn1);
	    TEMPFUNC(xn1, yn1, xn2, yn2);
	    TEMPFUNC(xn2, yn2, xo2, yo2);
	    TEMPFUNC(xo2, yo2, xo1, yo1);
#undef TEMPFUNC

	    if (t & 1)
		return linet[p].group;
	    xo1 = xo2;
	    yo1 = yo2;
	    xn1 = xn2;
	    yn1 = yn2;
	}
    }
    return -1;
}


/* This function should get called only rarely, so it doesn't need to
   be too efficient. */
static int Clear_corner(struct move *move, object *obj, int l1, int l2)
{
    int x, y, xm, ym, s1, s2;
    int l1sx, l2sx, l1sy, l2sy, l1dx, l2dx, l1dy, l2dy;
    int side;

    l1sx = linet[l1].start.cx - move->start.cx;
    l1sy = linet[l1].start.cy - move->start.cy;
    l1sx = CENTER_XCLICK(l1sx);
    l1sy = CENTER_YCLICK(l1sy);
    l1dx = linet[l1].delta.cx;
    l1dy = linet[l2].delta.cy;
    l2sx = linet[l2].start.cx - move->start.cx;
    l2sy = linet[l2].start.cy - move->start.cy;
    l2sx = CENTER_XCLICK(l2sx);
    l2sy = CENTER_YCLICK(l2sy);
    l2dx = linet[l2].delta.cx;
    l2dy = linet[l2].delta.cy;

    for (;;) {
	if (SIDE(obj->vel.x, obj->vel.y, l1) < 0) {
	    if (!Bounce_object(obj, move, l1, 0))
		return 0;
	}
	if (SIDE(obj->vel.x, obj->vel.y, l2) < 0) {
	    if (!Bounce_object(obj, move, l2, 0))
		return 0;
	    continue;
	}
	break;
    }

    xm = SIGN(move->delta.cx);
    ym = SIGN(move->delta.cy);

    s1 = SIDE(move->start.cx - l1sx, move->start.cy - l1sy, l1) > 0;
    s2 = SIDE(move->start.cx - l2sx, move->start.cy - l2sy, l2) > 0;

#define TMPFUNC(X, Y) ((side = SIDE((X), (Y), l1)) == 0 || side > 0 != s1 || (side = SIDE((X), (Y), l2)) == 0 || side > 0 != s2)

    if (ABS(obj->vel.x) >= ABS(obj->vel.y)) {
	x = xm;
	y = 0;
	for (;;) {
	    if (TMPFUNC(move->start.cx + x, move->start.cy + y)) {
		y += ym;
		if (!TMPFUNC(move->start.cx + x, move->start.cy + y + ym))
		    break;
		else
		    x += xm;;
	    }
	    else {
		if (TMPFUNC(move->start.cx + x, move->start.cy + y + 1) &&
		    TMPFUNC(move->start.cx + x, move->start.cy + y - 1))
		    x += xm;
		else
		    break;
	    }
	}
	move->delta.cx -= x;
	move->delta.cy -= y;
	if ((obj->vel.x >= 0) ^ (move->delta.cx >= 0)) {
	    move->delta.cx = 0;
	    move->delta.cy = 0;
	}
    }
    else {
	x = 0;
	y = ym;
	for (;;) {
	    if (TMPFUNC(move->start.cx + x, move->start.cy + y)) {
		x += xm;
		if (!TMPFUNC(move->start.cx + x + xm, move->start.cy + y))
		    break;
		else
		    y += ym;
	    }
	    else {
		if (TMPFUNC(move->start.cx + x + 1, move->start.cy + y) &&
		    TMPFUNC(move->start.cx + x - 1, move->start.cy + y))
		    y += ym;
		else
		    break;
	    }
	}

#undef TMPFUNC

	move->delta.cx -= x;
	move->delta.cy -= y;
	if ((obj->vel.y >= 0) ^ (move->delta.cy >= 0)) {
	    move->delta.cx = 0;
	    move->delta.cy = 0;
	}
    }
    move->start.cx = WRAP_XCLICK(move->start.cx + x);
    move->start.cy = WRAP_YCLICK(move->start.cy + y);
    return 1;
}


static void store_short(unsigned char **ptr, int i)
{
    *(*ptr)++ = i >> 8;
    *(*ptr)++ = i & 0xff;
}


static void store_32bit(unsigned char **ptr, int i)
{
    store_short(ptr, i >> 16);
    store_short(ptr, i & 0xffff);
}


int Polys_to_client(unsigned char *ptr)
{
    int i, j, startx, starty, dx, dy;
    int *edges;
    unsigned char *start = ptr;

    *ptr++ = num_pstyles;
    *ptr++ = num_estyles;
    *ptr++ = num_bstyles;
    for (i = 0; i < num_pstyles; i++) {
	store_32bit(&ptr, pstyles[i].color);
	*ptr++ = pstyles[i].texture_id;
	*ptr++ = pstyles[i].defedge_id;
	*ptr++ = pstyles[i].flags;
    }
    for (i = 0; i < num_estyles; i++) {
	*ptr++ = estyles[i].width;
	store_32bit(&ptr, estyles[i].color);
	*ptr++ = estyles[i].style;
    }
    for (i = 0; i < num_bstyles; i++) {
	strcpy((char *)ptr, bstyles[i].filename);
	ptr += strlen((char *)ptr) + 1;
	*ptr++ = bstyles[i].flags;
    }
    store_short(&ptr, num_polys);
    for (i = 0; i < num_polys; i++) {
	*ptr++ = pdata[i].style;
	j = pdata[i].num_points;
	store_short(&ptr, pdata[i].num_echanges);
	edges = estyleptr + pdata[i].estyles_start;
	while (*edges != INT_MAX)
	    store_short(&ptr, *edges++);
	startx = pdata[i].cx;
	starty = pdata[i].cy;
	edges = pdata[i].edges;
	store_short(&ptr, j);
	store_short(&ptr, startx >> CLICK_SHIFT);
	store_short(&ptr, starty >> CLICK_SHIFT);
	dx = startx;
	dy = starty;
	for (; j > 0; j--) {
	    dx += *edges++;
	    dy += *edges++;
	    if (j != 1) {
		store_short(&ptr, (dx >> CLICK_SHIFT) - (startx>>CLICK_SHIFT));
		store_short(&ptr, (dy >> CLICK_SHIFT) - (starty>>CLICK_SHIFT));
	    }
	    startx = dx;
	    starty = dy;
	}
    }
    *ptr++ = World.NumBases;
    for (i = 0; i < World.NumBases; i++) {
	if (World.base[i].team == TEAM_NOT_SET)
	    *ptr++ = 0;
	else
	    *ptr++ = World.base[i].team;
	store_short(&ptr, World.base[i].pos.cx >> CLICK_SHIFT);
	store_short(&ptr, World.base[i].pos.cy >> CLICK_SHIFT);
	*ptr++ = World.base[i].dir;
    }
    store_short(&ptr, World.NumFuels);
    for (i = 0; i < World.NumFuels; i++) {
	store_short(&ptr, World.fuel[i].pos.cx >> CLICK_SHIFT);
	store_short(&ptr, World.fuel[i].pos.cy >> CLICK_SHIFT);
    }
    *ptr++ = World.NumChecks;
    for (i = 0; i < World.NumChecks; i++) {
	store_short(&ptr, World.check[i].cx >> CLICK_SHIFT);
	store_short(&ptr, World.check[i].cy >> CLICK_SHIFT);
    }
    return ptr - start;
}


struct tempy {
    short y;
    struct tempy *next;
};


struct templine {
    short x1, x2, y1, y2;
    struct templine *next;
};


int is_inside(int cx, int cy, int hit_mask, const object *obj)
{
    short *ptr;
    int inside, cx1, cx2, cy1, cy2, s;
    struct inside_block *gblock;
    struct move mv;

    mv.hit_mask = hit_mask;
    mv.obj = (object *)obj;
    gblock = &inside_table[(cx >> B_SHIFT) + mapx * (cy >> B_SHIFT)];
    if (gblock->group == -1)
	return -1;
    do {
	if (gblock->group
	    /*&& (groups[gblock->group].hit_mask & hit_mask)) {*/
	    && (!can_hit(&groups[gblock->group], &mv))) {
	    gblock = gblock->next;
	    continue;
	}
	inside = gblock->base_value;
	if (gblock->lines == NULL) {
	    if (inside)
		return gblock->group;
	    else {
		gblock = gblock->next;
		continue;
	    }
	}
	cx &= B_MASK;
	cy &= B_MASK;
	ptr = gblock->y;
	if (ptr)
	    while (cy > *ptr++)
		inside++;
	ptr = gblock->lines;
	while (*ptr != 32767) {
	    cx1 = *ptr++ - cx;
	    cy1 = *ptr++ - cy;
	    cx2 = *ptr++ - cx;
	    cy2 = *ptr++ - cy;
	    if (cy1 < 0) {
		if (cy2 >= 0) {
		    if (cx1 > 0 && cx2 >= 0)
			inside++;
		    else if ((cx1 >= 0 || cx2 >= 0) &&
			     (s = cy1 * (cx1 - cx2) - cx1 * (cy1 - cy2)) >= 0) {
			if (s == 0)
			    return gblock->group;
			else
			    inside++;
		    }
		}
	    }
	    else
		if (cy2 <= 0) {
		    if (cy2 == 0) {
			if (cx2 == 0 || (cy1 ==0 && ((cx1 <= 0 && cx2 >= 0) ||
						     (cx1 >= 0 && cx2 <= 0))))
			    return gblock->group;
		    }
		    else if (cx1 > 0 && cx2 >= 0)
			inside++;
		    else if ((cx1 >= 0 || cx2 >= 0) &&
			     (s = cy1 * (cx1 - cx2) - cx1 * (cy1 - cy2)) <= 0) {
			if (s == 0)
			    return gblock->group;
			else
			    inside++;
		    }
		}
	}
	if (inside & 1)
	    return gblock->group;
	gblock = gblock->next;
    } while (gblock);
    return -1;
}


int shape_is_inside(int cx, int cy, int hitmask, const object *obj,
		    const shipobj *shape, int dir)
{
    static clpos zeropos;
    static shipobj zeroshape;
    int i, group;

    if (zeroshape.pts[0] == NULL) {
	for (i = 0; i < MAX_SHIP_PTS; i++)
	    zeroshape.pts[i] = &zeropos;
    }

    if ( (group = is_inside(cx, cy, hitmask, obj)) != -1)
	return group;
    zeroshape.num_points = shape->num_points;
    return Shape_morph(&zeroshape, 0, shape, dir, hitmask, obj, cx, cy);
}


static void closest_line(int bx, int by, double dist, int inside)
{
    if (dist <= temparray[bx + mapx *by].distance) {
	if (dist == temparray[bx + mapx * by].distance)
	    /* Must be joined polygons(s) if the map is legal
	     * (the same line appears in both directions).
	     * Both sides of this line are inside. */
	    /* These lines could be removed from the table as a minor
	     * optimization. */
	     inside = 1;
	temparray[bx + mapx * by].distance = dist;
	temparray[bx + mapx * by].inside = inside;
    }
}


static void insert_y(int block, int y)
{
    struct tempy *ptr;
    struct tempy **prev;

    ptr = temparray[block].y;
    prev = &temparray[block].y;
    while (ptr && ptr->y < y) {
	prev = &ptr->next;
	ptr = ptr->next;
    }
    if (ptr && ptr->y == y) {
	*prev = ptr->next;
	free(ptr);
	return;
    }
    *prev = ralloc(NULL, sizeof(struct tempy));
    (*prev)->y = y;
    (*prev)->next = ptr;
}


static void store_inside_line(int bx, int by, int ox, int oy, int dx, int dy)
{
    int block;
    struct templine *s;

    block = bx + mapx * by;
    ox = CENTER_XCLICK(ox - bx * B_CLICKS);
    oy = CENTER_YCLICK(oy - by * B_CLICKS);
    if (oy >= 0 && oy < B_CLICKS && ox >= B_CLICKS)
	insert_y(block, oy);
    if (oy + dy >= 0 && oy + dy < B_CLICKS && ox + dx >= B_CLICKS)
	insert_y(block, oy + dy);
    s = ralloc(NULL, sizeof(struct templine));
    s->x1 = ox;
    s->x2 = ox + dx;
    s->y1 = oy;
    s->y2 = oy + dy;
    s->next = temparray[block].lines;
    temparray[block].lines = s;
}


static void finish_inside(int block, int group)
{
    int inside;
    struct inside_block *gblock;
    short *ptr;
    int x1, x2, y1, y2, s, j;
    struct tempy *yptr;
    struct templine *lptr;
    void *tofree;

    gblock = &inside_table[block];
    if (gblock->group != -1) {
	while (gblock->next) /* Maintain group order*/
	    gblock = gblock->next;
	gblock->next = ralloc(NULL, sizeof(struct inside_block));
	gblock = gblock->next;
    }
    gblock->group = group;
    gblock->next = NULL;
    j = 0;
    yptr = temparray[block].y;
    while (yptr) {
	j++;
	yptr = yptr->next;
    }
    if (j > 0) {
	ptr = ralloc(NULL, (j + 1) * sizeof(short));
	gblock->y = ptr;
	yptr = temparray[block].y;
	while (yptr) {
	    *ptr++ = yptr->y;
	    tofree = yptr;
	    yptr = yptr->next;
	    free(tofree);
	}
	*ptr = 32767;
    }
    else
	gblock->y = NULL;
    j = 0;
    lptr = temparray[block].lines;
    while (lptr) {
	j++;
	lptr = lptr->next;
    }
    if (j > 0) {
	ptr = ralloc(NULL, (j * 4 + 1) * sizeof(short));
	gblock->lines = ptr;
	lptr = temparray[block].lines;
	while (lptr) {
	    *ptr++ = lptr->x1;
	    *ptr++ = lptr->y1;
	    *ptr++ = lptr->x2;
	    *ptr++ = lptr->y2;
	    tofree = lptr;
	    lptr = lptr->next;
	    free(tofree);
	}
	*ptr = 32767;
    }
    else
	gblock->lines = NULL;
    inside = temparray[block].inside;
    if ( (ptr = gblock->lines) != NULL) {
	while (*ptr != 32767) {
	    x1 = *ptr++ * 2 - B_CLICKS * 2 + 1;
	    y1 = *ptr++ * 2 + 1;
	    x2 = *ptr++ * 2 - B_CLICKS * 2 + 1;
	    y2 = *ptr++ * 2 + 1;
	    if (y1 < 0) {
		if (y2 >= 0) {
		    if (x1 > 0 && x2 >= 0)
			inside++;
		    else if ((x1 >= 0 || x2 >= 0) &&
			     (s = y1 * (x1 - x2) - x1 * (y1 - y2)) > 0)
			inside++;
		}
	    }
	    else
		if (y2 <= 0) {
		    if (x1 > 0 && x2 >= 0)
			inside++;
		    else if ((x1 >= 0 || x2 >= 0) &&
			     (s = y1 * (x1 - x2) - x1 * (y1 - y2)) < 0)
			inside++;
		}
	}
    }
    gblock->base_value = inside & 1;
    temparray[block].y = NULL;
    temparray[block].lines = NULL;
    temparray[block].inside = 2;
    temparray[block].distance = 1e20;
}


static void allocate_inside(void)
{
    int i;

    inside_table = ralloc(NULL, mapx * mapy * sizeof(struct inside_block));
    temparray = ralloc(NULL, mapx * mapy * sizeof(struct test));
    for (i = 0; i < mapx * mapy; i++) {
	temparray[i].distance = 1e20;
	temparray[i].inside = 2;
	temparray[i].y = NULL;
	temparray[i].lines = NULL;
	inside_table[i].y = NULL;
	inside_table[i].lines = NULL;
	inside_table[i].base_value = 0;
	inside_table[i].group = -1;
	inside_table[i].next = NULL;
    }
}


/* Calculate distance of intersection from lower right corner of the
 * block counterclockwise along the edge. We don't return the true lengths
 * but values which compare the same with each other.
 * 'dir' is used to return whether the block is left through a horizontal
 * or a vertical side or a corner. */
static double edge_distance(int bx, int by, int ox, int oy, int dx, int dy,
			  int *dir)
{
    int last_width = (World.cwidth - 1) % B_CLICKS + 1;
    int last_height = (World.cheight - 1) % B_CLICKS + 1;
    double xdist, ydist, dist;
    ox = CENTER_XCLICK(ox - bx * B_CLICKS);
    oy = CENTER_YCLICK(oy - by * B_CLICKS);
    if (dx > 0)
	xdist = ((bx == mapx - 1) ? last_width : B_CLICKS) - .5 - ox;
    else if (dx < 0)
	xdist = ox + .5;
    else
	xdist = 1e20; /* Something big enough to be > ydist, dx */
    if (dy > 0)
	ydist = ((by == mapy - 1) ? last_height : B_CLICKS) - .5 - oy;
    else if (dy < 0)
	ydist = oy + .5;
    else
	ydist = 1e20;
    if (xdist > ABS(dx) && ydist > ABS(dy))
	return -1;	/* Doesn't cross box boundary */
    if (ABS(dy) * xdist == ABS(dx) * ydist)
	*dir = 3;
    else if (ABS(dy) * xdist < ABS(dx) * ydist)
	*dir = 1;
    else
	*dir = 2;
    if (*dir == 1)
	if (dx > 0)
	    dist = oy + dy * xdist / dx;
	else
	    dist = 5 * B_CLICKS - oy + dy * xdist / dx;
    else
	if (dy > 0)
	    dist = 3 * B_CLICKS - ox - dx * ydist / dy;
	else
	    dist = 6 * B_CLICKS + ox - dx * ydist / dy;
    return dist;
}


#define POSMOD(x, y) ((x) >= 0 ? (x) % (y) : ((x) + 1) % (y) + (y) - 1)
static void Inside_init(void)
{
    int dx, dy, bx, by, ox, oy, startx, starty;
    int i, j, num_points, minx = -1, miny = -1, poly, group;
    int bx2, by2, maxx = -1, maxy = -1, dir;
    double dist;
    int *edges;

    allocate_inside();
    for (group = 0; group <= num_groups; group++) {
	minx = -1;
	for (poly = 0; poly < num_polys; poly++) {
	    if (pdata[poly].is_decor || pdata[poly].group != group)
		continue;
	    num_points = pdata[poly].num_points;
	    dx = 0;
	    dy = 0;
	    startx = pdata[poly].cx;
	    starty = pdata[poly].cy;
	    /* Better wrapping for bx2/by2 could be selected for speed here,
	     * but this keeping track of min/max at all is probably
	     * unnoticeable in practice. */
	    bx2 = bx = startx >> B_SHIFT;
	    by2 = by = starty >> B_SHIFT;
	    if (minx == -1) {
		minx = maxx = bx2;
		miny = maxy = by2;
	    }
	    edges = pdata[poly].edges;
	    closest_line(bx, by, 1e10, 0); /* For polygons within one block */
	    for (j = 0; j < num_points; j++) {
		if (((startx >> B_SHIFT) != bx)
		    || ((starty >> B_SHIFT) != by)) {
		    printf("startx = %d, startx >> B_SHIFT = %d, bx = %d\n",
			   startx, startx >> B_SHIFT, bx);
		    printf("starty = %d, starty >> B_SHIFT = %d, by = %d\n",
			   starty, starty >> B_SHIFT, by);
		    printf("going into infinite loop...\n");
		    /* hmm ??? */
		    while (1);
		}
		ox = startx & B_MASK;
		oy = starty & B_MASK;
		dx = *edges++;
		dy = *edges++;
		while (1) {  /* All blocks containing a part of this line */
		    store_inside_line(bx, by, startx, starty, dx, dy);
		    dist = edge_distance(bx, by, WRAP_XCLICK(startx + dx),
				 WRAP_YCLICK(starty + dy), -dx, -dy, &dir);
		    if (dist != -1)
			closest_line(bx, by, dist, 1);
		    dist = edge_distance(bx, by, startx, starty, dx, dy, &dir);
		    if (dist == -1)
			break;
		    closest_line(bx, by, dist, 0);
		    if (dir == 1 || dir == 3)
			bx2 += (dx > 0) ? 1 : -1;
		    if (bx2 > maxx)
			maxx = bx2;
		    if (bx2 < minx)
			minx = bx2;
		    bx = POSMOD(bx2, mapx);
		    if (dir == 2 || dir == 3)
			by2 += (dy > 0) ? 1 : -1;
		    if (by2 > maxy)
			maxy = by2;
		    if (by2 < miny)
			miny = by2;
		    by = POSMOD(by2, mapy);
		}
		startx = WRAP_XCLICK(startx + dx);
		starty = WRAP_YCLICK(starty + dy);
	    }
	}
	bx = maxx - minx + 1;
	if (bx > 2 * mapx)
	    bx = 2 * mapx;
	by = maxy - miny + 1;
	if (by > mapy)
	    by = mapy;
	for (i = POSMOD(miny, mapy); by-- > 0; i++) {
	    if (i == mapy)
		i = 0;
	    bx2 = bx;
	    dir = 0;
	    for (j = POSMOD(minx, mapx); bx2-- > 0; j++) {
		if (j == mapx)
		    j = 0;
		if (temparray[j + mapx * i].inside < 2) {
		    dir = temparray[j + mapx * i].distance > B_CLICKS &&
			temparray[j + mapx * i].inside == 1;
		}
		else {
		    if (dir)
			temparray[i * mapx + j].inside = 1;
		}
		if (bx2 < mapx)
		    finish_inside(j + mapx * i, group);
	    }
	}
    }
    return;
}


/* Include NCLLIN - 1 closest lines or all closer than CUTOFF (whichever
 * is less) in the line table for this block.
 * Include all lines closer than DICLOSE, however many there are.
 * LINSIZE tells the amout of temporary memory to reserve for the algorithm.
 * If it is not large enough to hold the required lines, print error and
 * exit.
 */

#define DICLOSE (5 * CLICK)
#define LINSIZE 100
#define NCLLIN (10 + 1)
static void Distance_init(void)
{
    int cx,cy;
    int *lineno, *dis;
    int lsx, lsy, ldx, ldy, temp, dist, n, i, bx, by, j, k;
    int base, height2, by2, width, height;
    int distbound, size;
    unsigned short *lptr;

    /* max line delta 30000 */

    blockline = ralloc(NULL, mapx * mapy * sizeof(struct blockinfo));
    lineno = ralloc(NULL, mapx * mapy * LINSIZE * sizeof(int));
    dis = ralloc(NULL, mapx * mapy * LINSIZE * sizeof(int));
    size = 1; /* start with end marker */
    for (bx = 0; bx < mapx; bx++)
	for (by = 0; by < mapy; by++)
	    for (i = 0; i < LINSIZE; i++) {
		dis[(by * mapx + bx) * LINSIZE + i] = MAX_MOVE + B_CLICKS / 2;
		lineno[(by * mapx + bx) * LINSIZE +i] = 65535;
	    }
    for (i = 0; i < num_lines; i++) {
	bx = linet[i].start.cx;
	by = linet[i].start.cy;
	width = linet[i].delta.cx;
	height = linet[i].delta.cy;
	if (width < 0) {
	    bx += width;
	    width = -width;
	    bx = WRAP_XCLICK(bx);
	}
	if (height < 0) {
	    by += height;
	    height = -height;
	    by = WRAP_YCLICK(by);
	}
	width = (width + 2 * MAX_MOVE) / B_CLICKS + 5;
	if (width >= mapx)
	    width = mapx;
	height = (height + 2 * MAX_MOVE) / B_CLICKS + 5;
	if (height >= mapy)
	    height = mapy;
	bx = (bx - MAX_MOVE) / B_CLICKS - 2;
	by = (by - MAX_MOVE) / B_CLICKS - 2;
	while (bx < 0)
	    bx += mapx;
	while (by < 0)
	    by += mapy;
	height2 = height;
	by2 = by;
	for (; width-- > 0; bx = bx == mapx - 1 ? 0 : bx + 1)
	    for (by = by2, height = height2;
		 height -- > 0;
		 by = by == mapy - 1? 0 : by + 1) {
		cx = bx * B_CLICKS + B_CLICKS / 2;
		cy = by * B_CLICKS + B_CLICKS / 2;
		base = (by * mapx + bx) * LINSIZE;
		lsx = CENTER_XCLICK(linet[i].start.cx - cx);
		if (ABS(lsx) > 32767 + MAX_MOVE + B_CLICKS / 2)
		    continue;
		lsy = CENTER_YCLICK(linet[i].start.cy - cy);
		if (ABS(lsy) > 32767 + MAX_MOVE + B_CLICKS / 2)
		    continue;
		ldx = linet[i].delta.cx;
		ldy = linet[i].delta.cy;
		if (MAX(ABS(lsx), ABS(lsy)) > MAX(ABS(lsx + ldx),
						  ABS(lsy + ldy))) {
		    lsx += ldx;
		    ldx = -ldx;
		    lsy += ldy;
		    ldy = -ldy;
		}
		if (ABS(lsx) < ABS(lsy)) {
		    temp = lsx;
		    lsx = lsy;
		    lsy = temp;
		    temp = ldx;
		    ldx = ldy;
		    ldy = temp;
		}
		if (lsx < 0) {
		    lsx = -lsx;
		    ldx = -ldx;
		}
		if (ldx >= 0)
		    dist = lsx - 1;
		else {
		    if (lsy + ldy < 0) {
			lsy = -lsy;
			ldy = -ldy;
		    }
		    temp = lsy - lsx;
		    lsx += lsy;
		    lsy = temp;
		    temp = ldy - ldx;
		    ldx += ldy;
		    ldy = temp;
		    dist = lsx - ldx * lsy / ldy;
		    if (lsx + ldx < 0)
			dist = MIN(ABS(dist), ABS(lsy - ldy * lsx / ldx));
		    dist = dist / 2 - 3; /* 3? didn't bother to get the right value */
		}
		dist--;
		/* Room for one extra click of movement after main collision
		   detection. Used to get away from a line after a bounce. */
		if (dist < CUTOFF + B_CLICKS / 2) {
		    if (dist < B_CLICKS / 2 + DICLOSE)
			distbound = LINSIZE;
		    else
			distbound = NCLLIN;
		    for (j = 1; j < distbound; j++) {
			if (dis[base + j] <= dist)
			    continue;
			k = dis[base + j];
			n = j;
			for (j++; j < distbound; j++)
			    if (dis[base + j] > k) {
				k = dis[base + j];
				n = j;
			    }
			if (dis[base + 0] > dis[base + n])
			    dis[base + 0] = dis[base + n];
			if (lineno[base + n] == 65535) {
			    size++; /* more saved lines */
			    if (n == 1)
				size++; /* first this block, for 65535 */
			}
			dis[base + n] = dist;
			lineno[base + n] = i;
			goto stored;
		    }
		}
		if (dist < dis[base + 0])
		    dis[base + 0] = dist;
		if (dist < B_CLICKS / 2 + DICLOSE) {
		    printf("Not enough space in line table. "
			   "Fix allocation in walls.c\n");
		    exit(1);
		}
	    stored:
		; /* semicolon for ansi compatibility */
	    }
	}
    llist = ralloc(NULL, size * sizeof(short));
    lptr = llist;
    *lptr++ = 65535; /* All blocks with no lines stored point to this. */
    for (bx = 0; bx < mapx; bx++)
	for (by = 0; by < mapy; by++) {
	    base = (by * mapx + bx) * LINSIZE;
	    k = bx + mapx * by;
	    blockline[k].distance = dis[base + 0] - B_CLICKS / 2;
	    if (lineno[base + 1] == 65535)
		blockline[k].lines = llist;
	    else {
		blockline[k].lines = lptr;
		for (j = 1; j < LINSIZE && lineno[base + j] != 65535; j++)
		    *lptr++ = lineno[base + j];
		*lptr++ = 65535;
	    }
	}
    free(lineno);
    free(dis);
}


static void Corner_init(void)
{
    int bx, by, cx, cy, dist, i;
    unsigned short *ptr, *temp;
    int block, size = mapx * mapy;
    int height, height2, width, by2;

#define DISIZE 200
    temp = ralloc(NULL, mapx * mapy * DISIZE * sizeof(short)); /* !@# */
    for (i = 0; i < mapx * mapy; i++)
	temp[i * DISIZE] = 0;
    for (i = 0; i < num_lines; i++) {
	bx = linet[i].start.cx;
	by = linet[i].start.cy;
	width = height = (2 * MAX_MOVE) / B_CLICKS + 7;
	if (width >= mapx)
	    width = mapx;
	if (height >= mapy)
	    height = mapy;
	bx = (bx - MAX_MOVE) / B_CLICKS - 3;
	by = (by - MAX_MOVE) / B_CLICKS - 3;
	while (bx < 0)
	    bx += mapx;
	while (by < 0)
	    by += mapy;
	height2 = height;
	by2 = by;
	for (; width-- > 0; bx = bx == mapx - 1 ? 0 : bx + 1)
	    for (by = by2, height = height2;
		 height -- > 0;
		 by = by == mapy - 1? 0 : by + 1) {
		block = bx + mapx * by;
		dist = blockline[block].distance
		    + MAX_SHAPE_OFFSET + B_CLICKS / 2;
		cx = bx * B_CLICKS + B_CLICKS / 2;
		cy = by * B_CLICKS + B_CLICKS / 2;
		if (ABS(CENTER_XCLICK(linet[i].start.cx - cx)) > dist)
		    continue;
		if (ABS(CENTER_YCLICK(linet[i].start.cy - cy)) > dist)
		    continue;
		temp[++temp[DISIZE * block] + DISIZE * block] = i;
		size++;
	    }
    }
    plist = ralloc(NULL, size * sizeof(short));
    ptr = plist;
    for (block = 0; block < mapx * mapy; block++) {
	blockline[block].points = ptr;
	i = temp[block * DISIZE];
	if (i > DISIZE - 1) {
	    warn("Not enough corner space in walls.c, add more.");
	    exit(1);
	}
	while (i > 0) {
	    *ptr++ = temp[block * DISIZE + i];
	    i--;
	}
	*ptr++ = 65535;
    }
    free(temp);
#undef DISIZE
}


void Ball_line_init(void)
{
    int i;
    static clpos coords[24];

    if (!treatBallAsPoint) {
	ball_wire.num_points = 24;
	for (i = 0; i < 24; i++) {
	    ball_wire.pts[i] = coords + i;
	    coords[i].cx = cos(i * PI / 12) * BALL_RADIUS * CLICK;
	    coords[i].cy = sin(i * PI / 12) * BALL_RADIUS * CLICK;
	}
	/*xpprintf(__FILE__ ": treating ball as polygon.\n");*/
    } else {
	ball_wire.num_points = 1;
	ball_wire.pts[0] = coords;
	coords[0].cx = 0;
	coords[0].cy = 0;
	/*xpprintf(__FILE__ ": treating ball as point.\n");*/
    }

    return;
}


static void Poly_to_lines(void)
{
    int i, np, j, startx, starty, dx, dy, group, *styleptr, style;
    int *edges;

    num_lines = 0;
    for (i = 0; i < num_polys; i++) {
	if (pdata[i].is_decor)
	    continue;
	group = pdata[i].group;
	np = pdata[i].num_points;
	styleptr = estyleptr + pdata[i].estyles_start;
	style = pstyles[pdata[i].style].defedge_id;
	dx = 0;
	dy = 0;
	startx = pdata[i].cx;
	starty = pdata[i].cy;
	edges = pdata[i].edges;
	for (j = 0; j < np; j++) {
	    if (j == *styleptr) {
		styleptr++;
		style = *styleptr++;
	    }
	    if (style == 0) {
		dx += *edges++;
		dy += *edges++;
		continue;
	    }
	    if (!(num_lines % 2000))
		linet = ralloc(linet, (num_lines + 2000) * sizeof(struct bline));
	    linet[num_lines].group = group;
	    linet[num_lines].start.cx = TWRAP_XCLICK(startx + dx);
	    linet[num_lines].start.cy = TWRAP_YCLICK(starty + dy);
	    linet[num_lines].delta.cx = *edges;
	    dx += *edges++;
	    linet[num_lines++].delta.cy = *edges;
	    dy += *edges++;
	}
	if (dx || dy) {
	    warn("Polygon %d (%d points) doesn't start and end at the "
		 "same place", i + 1, np);
	    exit(1);
	}
    }
    linet = ralloc(linet, (num_lines + S_LINES) * sizeof(struct bline));
    return;
}


void Walls_init_new(void)
{
    DFLOAT x, y, l2;
    int i;

    mapx = (World.cwidth + B_MASK) >> B_SHIFT;
    mapy = (World.cheight + B_MASK) >> B_SHIFT;
    Poly_to_lines();
    Distance_init();
    Corner_init();
    Ball_line_init();
    Inside_init();
    groups[0].type = FILLED;

    for (i = 0; i < num_lines; i++) {
	x = linet[i].delta.cx;
	y = linet[i].delta.cy;
	l2 = (x*x + y*y);
	linet[i].c = (x*x - y*y) / l2;
	linet[i].s = 2*x*y / l2;
    }
}


/* end */

static char msg[MSG_LEN];



static void Move_ball_new(object *obj)
{
    /*object		*obj = Obj[ind];*/
    int line, point;
    struct move mv;
    struct collans ans;
    int owner;

    mv.delta.cx = FLOAT_TO_CLICK(obj->vel.x * timeStep2);
    mv.delta.cy = FLOAT_TO_CLICK(obj->vel.y * timeStep2);
    mv.obj = obj;
    obj->extmove.cx = mv.delta.cx;
    obj->extmove.cy = mv.delta.cy;

    if (obj->id != -1
	&& BIT(Players[GetInd[obj->id]]->used, HAS_PHASING_DEVICE)) {

	int cx = obj->pos.cx + mv.delta.cx;
	int cy = obj->pos.cy + mv.delta.cy;
	while (cx >= World.cwidth)
	    cx -= World.cwidth;
	while (cx < 0)
	    cx += World.cwidth;
	while (cy >= World.cheight)
	    cy -= World.cheight;
	while (cy < 0)
	    cy += World.cheight;
	Object_position_set_clicks(obj, cx, cy);
	Cell_add_object(obj);
	return;
    }
    owner = BALL_PTR(obj)->owner;
    if (owner == NO_ID || Players[GetInd[owner]]->team == TEAM_NOT_SET)
	mv.hit_mask = BALL_BIT | NOTEAM_BIT;
    else
	mv.hit_mask = BALL_BIT | 1 << Players[GetInd[owner]]->team;
    mv.start.cx = obj->pos.cx;
    mv.start.cy = obj->pos.cy;
    while (mv.delta.cx || mv.delta.cy) {
	Shape_move(&mv, &ball_wire, 0, &ans);
	mv.start.cx = WRAP_XCLICK(mv.start.cx + ans.moved.cx);
	mv.start.cy = WRAP_YCLICK(mv.start.cy + ans.moved.cy);
	mv.delta.cx -= ans.moved.cx;
	mv.delta.cy -= ans.moved.cy;
	if (ans.line != -1) {
	    if (!Shape_away(&mv, &ball_wire, 0, ans.line, &line, &point)) {
		if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0) {
		    if (!Bounce_object(obj, &mv, ans.line, ans.point))
			break;
		}
		else if (SIDE(obj->vel.x, obj->vel.y, line) < 0) {
		    if (!Bounce_object(obj, &mv, line, point))
			break;
		}
		else {
		    /* This case could be handled better,
		       I'll write the code for that if this
		       happens too often. */
		    mv.delta.cx = 0;
		    mv.delta.cy = 0;
		    obj->vel.x = 0;
		    obj->vel.y = 0;
		}
	    }
	    else if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0)
		if (!Bounce_object(obj, &mv, ans.line, ans.point))
		    break;
	}
    }
    Object_position_set_clicks(obj, mv.start.cx, mv.start.cy);
    Cell_add_object(obj);
    return;
}


/* kps- collision.c has a move_object call in ng */
/* used to have ind argument in ng */
static void Move_object_new(object *obj)
{
    int t;
    struct move mv;
    struct collans ans;
    int trycount = 5000;
    int team;            /* !@# should make TEAM_NOT_SET 0 */
    /*object *obj = Obj[ind];*/

    mv.obj = obj;
    Object_position_remember(obj);

    obj->collmode = 1;

#if 1
    if (obj->type == OBJ_BALL) {
	Move_ball_new(obj);
	return;
    }
#else
    if (obj->type == OBJ_BALL) {
	if (obj->owner != -1)
	    team =  Players[GetInd[obj->owner]].team;
	else
	    team = TEAM_NOT_SET;
	mv.hit_mask = BALL_BIT;
    }
    else
#endif
	{
	    mv.hit_mask = NONBALL_BIT;
	    team = obj->team;
	}
    if (team == TEAM_NOT_SET)
	mv.hit_mask |= NOTEAM_BIT;
    else
	mv.hit_mask |= 1 << team;

    mv.start.cx = obj->pos.cx;
    mv.start.cy = obj->pos.cy;
    mv.delta.cx = FLOAT_TO_CLICK(obj->vel.x * timeStep2);
    mv.delta.cy = FLOAT_TO_CLICK(obj->vel.y * timeStep2);
    obj->extmove.cx = mv.delta.cx;
    obj->extmove.cy = mv.delta.cy;
    while (mv.delta.cx || mv.delta.cy) {
	if (!trycount--) {
	    sprintf(msg, "COULDN'T MOVE OBJECT!!!! Type = %s, x = %d, y = %d. "
		    "Object was DELETED. [*DEBUG*]",
		    Object_typename(obj), mv.start.cx, mv.start.cy);
	    warn(msg);
	    Set_message(msg);
	    obj->life = 0;
	    return;
	}
	Move_point(&mv, &ans);
	mv.delta.cx -= ans.moved.cx;
	mv.delta.cy -= ans.moved.cy;
	mv.start.cx = WRAP_XCLICK(mv.start.cx + ans.moved.cx);
	mv.start.cy = WRAP_YCLICK(mv.start.cy + ans.moved.cy);
	if (ans.line != -1) {
	    if ( (t = Away(&mv, ans.line)) != -1) {
		if (!Clear_corner(&mv, obj, ans.line, t))
		    break;
	    }
	    else if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0) {
		if (!Bounce_object(obj, &mv, ans.line, 0))
		    break;
	    }
	}
    }
    Object_position_set_clicks(obj, mv.start.cx, mv.start.cy);
    Cell_add_object(obj);
    return;
}


static void Move_player_new(int ind)
{
    player *pl = Players[ind];
    clpos  pos;
    int    line, point;
    struct move mv;
    struct collans ans;


    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	if (!BIT(pl->status, KILLED|PAUSE)) {
	    pos.cx = pl->pos.cx + FLOAT_TO_CLICK(pl->vel.x * timeStep2);
	    pos.cy = pl->pos.cy + FLOAT_TO_CLICK(pl->vel.y * timeStep2);
	    pos.cx = WRAP_XCLICK(pos.cx);
	    pos.cy = WRAP_YCLICK(pos.cy);
	    if (pos.cx != pl->pos.cx || pos.cy != pl->pos.cy) {
		Player_position_remember(pl);
		Player_position_set_clicks(pl, pos.cx, pos.cy);
	    }
	}
	pl->velocity = VECTOR_LENGTH(pl->vel);
	return;
    }

    /* kps - changed from  vel *= friction */
    pl->vel.x *= (1.0f - friction);
    pl->vel.y *= (1.0f - friction);

    Player_position_remember(pl);

    pl->collmode = 1;

    mv.obj = (object *)pl;
    mv.delta.cx = FLOAT_TO_CLICK(pl->vel.x * timeStep2);
    mv.delta.cy = FLOAT_TO_CLICK(pl->vel.y * timeStep2);
#if 0
    pl->extmove.cx = mv.delta.cx;
    pl->extmove.cy = mv.delta.cy;
#endif

    if (BIT(pl->used, HAS_PHASING_DEVICE)) {
	int cx = pl->pos.cx + mv.delta.cx;
	int cy = pl->pos.cy + mv.delta.cy;
	while (cx >= World.cwidth)
	    cx -= World.cwidth;
	while (cx < 0)
	    cx += World.cwidth;
	while (cy >= World.cheight)
	    cy -= World.cheight;
	while (cy < 0)
	    cy += World.cheight;
	Player_position_set_clicks(pl, cx, cy);
    }
    else {
	if (pl->team != TEAM_NOT_SET)
	    mv.hit_mask = NONBALL_BIT | 1 << pl->team;
	else
	    mv.hit_mask = NONBALL_BIT | NOTEAM_BIT;
	mv.start.cx = pl->pos.cx;
	mv.start.cy = pl->pos.cy;
	while (mv.delta.cx || mv.delta.cy) {
	    Shape_move(&mv, pl->ship, pl->dir, &ans);
	    mv.start.cx = WRAP_XCLICK(mv.start.cx + ans.moved.cx);
	    mv.start.cy = WRAP_YCLICK(mv.start.cy + ans.moved.cy);
	    mv.delta.cx -= ans.moved.cx;
	    mv.delta.cy -= ans.moved.cy;
	    if (ans.line != -1) {
		if (!Shape_away(&mv, pl->ship, pl->dir,
				ans.line, &line, &point)) {
		    if (SIDE(pl->vel.x, pl->vel.y, ans.line) < 0)
			Bounce_player(pl, &mv, ans.line, ans.point);
		    else if (SIDE(pl->vel.x, pl->vel.y, line) < 0)
			Bounce_player(pl, &mv, line, point);
		    else {
			/* This case could be handled better,
			   I'll write the code for that if this
			   happens too often. */
			mv.delta.cx = 0;
			mv.delta.cy = 0;
			pl->vel.x = 0;
			pl->vel.y = 0;
		    }
		}
		else if (SIDE(pl->vel.x, pl->vel.y, ans.line) < 0)
		    Bounce_player(pl, &mv, ans.line, ans.point);
	    }
	}
	Player_position_set_clicks(pl, mv.start.cx, mv.start.cy);
    }
    pl->velocity = VECTOR_LENGTH(pl->vel);
    /* !@# Better than ignoring collisions after wall touch for players,
     * but might cause some erroneous hits */
    pl->extmove.cx = CENTER_XCLICK(pl->pos.cx - pl->prevpos.cx);
    pl->extmove.cy = CENTER_YCLICK(pl->pos.cy - pl->prevpos.cy);
    return;
}


static void Turn_player_new(int ind)
{
    player	*pl = Players[ind];
    int		new_dir = MOD2((int)(pl->float_dir + 0.5f), RES);
    int		next_dir, sign, hitmask;

    if (recOpt) {
	if (record)
	    *playback_data++ = new_dir;
	else if (playback)
	    new_dir = *playback_data++;
    }
    if (new_dir == pl->dir) {
	return;
    }
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	pl->dir = new_dir;
	return;
    }

    if (new_dir > pl->dir)
	sign = (new_dir - pl->dir <= RES + pl->dir - new_dir) ? 1 : -1;
    else
	sign = (pl->dir - new_dir <= RES + new_dir - pl->dir) ? -1 : 1;

    if (pl->team != TEAM_NOT_SET)
	hitmask = NONBALL_BIT | 1 << pl->team;
    else
	hitmask = NONBALL_BIT | NOTEAM_BIT;
#if 1
    while (pl->dir != new_dir) {
	next_dir = MOD2(pl->dir + sign, RES);
	if (Shape_morph(pl->ship, pl->dir, pl->ship, next_dir, hitmask,
			(object *)pl, pl->pos.cx, pl->pos.cy) != -1) {
	    pl->float_dir = pl->dir;
	    break;
	}
	pl->dir = next_dir;
    }
#else
    /* Mara's experimental "turnqueue" hack  */
    while (pl->dir != new_dir
	   && (Shape_turn1(pl->ship, &mv, pl->pos.cx, pl->pos.cy,
			   pl->dir, sign) /*|| (pl->float_dir = pl->dir, 0)*/))
	pl->dir = MOD2(pl->dir + sign, RES);
#endif

    return;
}

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#ifdef _WINDOWS
#include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "saudio.h"
#include "item.h"
#include "netserver.h"
#include "pack.h"
#include "error.h"
#include "walls.h"
#include "click.h"
#include "objpos.h"
#include "srecord.h"

/* start */

#include "math.h"

static struct move_parameters mp; /* moved here */

/* Maximum line length 32767.
   There's a minimum map size to avoid "too much wrapping". A bit smaller
   than that would cause rare errors for fast-moving things. I haven't
   bothered to figure out what the limit is. 80k x 80k clicks should
   be more than enough (probably...). */
#define B_SHIFT 11
#define B_CLICKS (1 << B_SHIFT)
#define B_MASK (B_CLICKS - 1)
#define CUTOFF (2 * BLOCK_CLICKS) /* Not sure about the optimum value */
#define MAX_MOVE 32000
#define SEPARATION_DIST 64
/* This must be increased if the ship corners are allowed to go farther
   when turning! */
#define MAX_SHAPE_OFFSET (15 * CLICK)
#define NOTEAM_BIT (1 << 10)
#define BALL_BIT (1 << 11)
#define NONBALL_BIT (1 << 12)

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

struct move {
    clvec start;
    clvec delta;
    int hit_mask;
};

struct bline {
    clvec start;
    clvec delta;
    DFLOAT c;
    DFLOAT s;
    short group;
};

struct group {
    int type;
    unsigned int hit_mask;
    int team;
};

struct blockinfo {
    unsigned short distance;
    unsigned short *lines;
    unsigned short *points;
};

wireobj ball_wire;

#define LINEY(X, Y, BASE, ARG)  (((Y)*(ARG)+(BASE))/(X))
#define SIDE(X, Y, LINE) (linet[(LINE)].delta.y * (X) - linet[(LINE)].delta.x * (Y))
#define SIGN(X) ((X) >= 0 ? 1 : -1)

struct bline *linet;
#define S_LINES 100 /* stupid hack */

struct group groups[100] = { /* !@# */
    {0, 0},
    {0, 0},
    {0, 0}};

struct blockinfo *blockline;

unsigned short *llist;

unsigned short *plist;

int linec = 0;

int polyc = 0;

int num_groups = 0;

int mapx, mapy;

static char msg[MSG_LEN];

static void *ralloc(void *ptr, size_t size)
{
    if (!(ptr = realloc(ptr, size))) {
	error("Realloc failed.");
	exit(1);
    }
    return ptr;
}

static DFLOAT wallBounceExplosionMult;

static unsigned short *Shape_lines(const wireobj *shape, int dir)
{
    int p;
    static unsigned short foo[100];
    static const wireobj *lastshape;
    static int lastdir;
    const int os = linec;

    /* linet[i].group MUST BE INITIALIZED TO 0 */

    if (shape == lastshape && dir == lastdir)
	return foo;

    lastshape = shape;
    lastdir = dir;

    for (p = 0; p < shape->num_points; p++) {
	linet[p + os].start.x = -shape->pts[p][dir].x;
	linet[p + os].start.y = -shape->pts[p][dir].y;
    }
    for (p = 0; p < shape->num_points - 1; p++) {
	linet[p + os].delta.x = linet[p + os + 1].start.x - linet[p + os].start.x;
	linet[p + os].delta.y = linet[p + os + 1].start.y - linet[p + os].start.y;
    }
    linet[p + os].delta.x = linet[os].start.x - linet[p + os].start.x;
    linet[p + os].delta.y = linet[os].start.y - linet[p + os].start.y;
    for (p = 0; p < shape->num_points; p++)
	foo[p] = p + os;
    foo[p] = 65535;
    return foo;
}


static int Bounce_object(object *obj, struct move *move, int line, int point)
{
    DFLOAT fx, fy;
    DFLOAT c, s;
    int group, type;

    group = linet[line >= linec ? point : line].group;
    type = groups[group].type;

    if (obj->collmode == 1) {
	fx = ABS(obj->vel.x) + ABS(obj->vel.y);
	/* If fx<1, there is practically no movement. Object
	   collision detection can ignore the bounce. */
	if (fx > 1) {
	    obj->wall_time = 1 -
		CLICK_TO_FLOAT(ABS(move->delta.x) + ABS(move->delta.y)) / fx;
	    obj->collmode = 2;
	}
    }

    if (type == TREASURE) {
	if (obj->type != OBJ_BALL) {
	    obj->life = 0;
	    return 0;
	}
	if (World.treasures[obj->treasure].team == groups[group].team) {
	    /*
	     * Ball has been replaced back in the hoop from whence
	     * it came. The player must be from the same team as the ball,
	     * otherwise Bounce_object() wouldn't have been called. It
	     * should be replaced into the hoop without exploding and
	     * the player gets some points.
	     */
	    player	*pl = NULL;
	    treasure_t	*tt = &World.treasures[obj->treasure];

	    if (obj->owner != -1)
		pl = Players[GetInd[obj->owner]];

	    obj->life = 0;
	    SET_BIT(obj->status, (NOEXPLOSION|RECREATE));

	    SCORE(GetInd[pl->id], 5,
		  tt->pos.x, tt->pos.y, "Treasure: ");
	    sprintf(msg, " < %s (team %d) has replaced the treasure >",
		    pl->name, pl->team);
	    Set_message(msg);
	    return 0;
	}
	if (obj->owner == -1) {
	    obj->life = 0;
	    return 0;
	}
	if (groups[group].team == Players[GetInd[obj->owner]]->team) {
	    /*
	     * Ball has been brought back to home treasure.
	     * The team should be punished.
	     */
	    sprintf(msg," < The ball was loose for %ld frames >",
		    LONG_MAX - obj->life);
	    Set_message(msg);
	    if (Punish_team(GetInd[obj->owner], obj->treasure,
			    obj->pos.cx, obj->pos.cy))
		CLR_BIT(obj->status, RECREATE);
	}
	obj->life = 0;
	return 0;
    }

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


    if (line >= linec) {
	DFLOAT x, y, l2;
	x = linet[line].delta.x;
	y = linet[line].delta.y;
	l2 = (x*x + y*y);
	c = (x*x - y*y) / l2;
	s = 2*x*y / l2;
    }
    else {
	c = linet[line].c;
	s = linet[line].s;
    }
    fx = move->delta.x * c + move->delta.y * s;
    fy = move->delta.x * s - move->delta.y * c;
    move->delta.x = fx * objectWallBrakeFactor;
    move->delta.y = fy * objectWallBrakeFactor;
    fx = obj->vel.x * c + obj->vel.y * s;
    fy = obj->vel.x * s - obj->vel.y * c;
    obj->vel.x = fx * objectWallBrakeFactor;
    obj->vel.y = fy * objectWallBrakeFactor;
    if (obj->collmode == 2)
	obj->collmode = 3;
    return 1;
}


static void Player_crash(player *pl, struct move *move, int crashtype)
{
    int			ind = GetInd[pl->id];
    const char		*howfmt = NULL;
    const char          *hudmsg = NULL;

    msg[0] = '\0';

    switch (crashtype) {

    default:
    case NotACrash:
	errno = 0;
	error("Unrecognized crash %d", crashtype);
	exit(1);

	/*  !@# target too */
#if 0
    case CrashWormHole:
	SET_BIT(pl->status, WARPING);
	pl->wormHoleHit = ms->wormhole;
	break;
#endif

    case CrashWall:
	howfmt = "%s crashed against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashWallSpeed:
	howfmt = "%s smashed against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashWallNoFuel:
	howfmt = "%s smacked against a wall";
	hudmsg = "[Wall]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

#if 0
    case CrashTarget:
	howfmt = "%s smashed against a target";
	hudmsg = "[Target]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	Object_hits_target(ms, -1);
	break;
#endif

    case CrashTreasure:
	howfmt = "%s smashed against a treasure";
	hudmsg = "[Treasure]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

#if 0
    case CrashCannon:
	if (BIT(pl->used, OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)
	    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
	    howfmt = "%s smashed against a cannon";
	    hudmsg = "[Cannon]";
	    sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_CANNON_SOUND);
	}
	Cannon_dies(ms);
	break;
#endif

    case CrashUniverse:
	howfmt = "%s left the known universe";
	hudmsg = "[Universe]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;

    case CrashUnknown:
	howfmt = "%s slammed into a programming error";
	hudmsg = "[Bug]";
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_HIT_WALL_SOUND);
	break;
    }

    if (howfmt && hudmsg) {
	player		*pushers[MAX_RECORDED_SHOVES];
	int		cnt[MAX_RECORDED_SHOVES];
	int		num_pushers = 0;
	int		total_pusher_count = 0;
	int		total_pusher_score = 0;
	int		i, j, sc;

	SET_BIT(pl->status, KILLED);
	move->delta.x = 0;
	move->delta.y = 0;
	sprintf(msg, howfmt, pl->name);

	/* get a list of who pushed me */
	for (i = 0; i < MAX_RECORDED_SHOVES; i++) {
	    shove_t *shove = &pl->shove_record[i];
	    if (shove->pusher_id == -1) {
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
	    SCORE(ind, -sc,
		  OBJ_X_IN_BLOCKS(pl),
		  OBJ_Y_IN_BLOCKS(pl),
		  hudmsg);
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
		sc = cnt[i] * (int)floor(Rate(pusher->score, pl->score)
				    * shoveKillScoreMult) / total_pusher_count;
		SCORE(GetInd[pusher->id], sc,
		      OBJ_X_IN_BLOCKS(pl),
		      OBJ_Y_IN_BLOCKS(pl),
		      pl->name);
	    }
	    sc = (int)floor(Rate(average_pusher_score, pl->score)
		       * shoveKillScoreMult);
	    SCORE(ind, -sc,
		  OBJ_X_IN_BLOCKS(pl),
		  OBJ_Y_IN_BLOCKS(pl),
		  "[Shove]");
	    strcpy(msg_ptr, ".");
	    Set_message(msg);
	}
    }
}


static void Bounce_player(player *pl, struct move *move, int line, int point)
{
    DFLOAT fx, fy;
    DFLOAT c, s;
    int group;

    if (line >= linec) {
	DFLOAT x, y, l2;
	x = linet[line].delta.x;
	y = linet[line].delta.y;
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
    if (groups[group].type == TREASURE) {
	Player_crash(pl, move, CrashTreasure);
	return;
    }

    pl->last_wall_touch = frame_loops;
    {
	DFLOAT	speed = VECTOR_LENGTH(pl->vel);
	int	v = (int) speed >> 2;
	int	m = (int) (pl->mass - pl->emptymass * 0.75f);
	DFLOAT	b = 1 - 0.5f * playerWallBrakeFactor;
	long	cost = (long) (b * m * v);
	DFLOAT	max_speed = BIT(pl->used, OBJ_SHIELD)
		? maxShieldedWallBounceSpeed
		: maxUnshieldedWallBounceSpeed;

	if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
	    == (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
	    max_speed = 100;
	}
	if (speed > max_speed) {
	    Player_crash(pl, move, CrashWallSpeed);
	    return;
	}

	/*
	 * Small explosion and fuel loss if survived a hit on a wall.
	 * This doesn't affect the player as the explosion is sparks
	 * which don't collide with player.
	 */
	cost *= 0.9; /* used to depend on bounce angle, .5 .. 1.0 */
	if (BIT(pl->used, (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD))
	    != (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
	    Add_fuel(&pl->fuel, (long)(-((cost << FUEL_SCALE_BITS)
					 * wallBounceFuelDrainMult)));
	    Item_damage(GetInd[pl->id], wallBounceDestroyItemProb);
	}
	if (!pl->fuel.sum && wallBounceFuelDrainMult != 0) {
	    Player_crash(pl, move, CrashWallNoFuel);
	    return;
	}
/* !@# I didn't implement wall direction calculation yet. */
	if (cost) {
#if 0
	    int intensity = (int)(cost * wallBounceExplosionMult);
	    Make_debris(
			/* pos.x, pos.y   */ pl->pos.cx, pl->pos.cy,
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
		Object_hits_target(&ms[worst], cost);
	    }
#endif
	}
    }
    fx = move->delta.x * c + move->delta.y * s;
    fy = move->delta.x * s - move->delta.y * c;
    move->delta.x = fx * playerWallBrakeFactor;
    move->delta.y = fy * playerWallBrakeFactor;
    fx = pl->vel.x * c + pl->vel.y * s;
    fy = pl->vel.x * s - pl->vel.y * c;
    pl->vel.x = fx * playerWallBrakeFactor;
    pl->vel.y = fy * playerWallBrakeFactor;
}

static int Away(struct move *move, int line)
{
    int i, dx, dy, lsx, lsy, res;
    unsigned short *lines;

    lsx = linet[line].start.x - move->start.x;
    lsy = linet[line].start.y - move->start.y;
    lsx = CENTER_XCLICK(lsx);
    lsy = CENTER_YCLICK(lsy);

    if (ABS(linet[line].delta.x) >= ABS(linet[line].delta.y)) {
	dx = 0;
	dy = -SIGN(linet[line].delta.x);
    }
    else {
	dy = 0;
	dx = SIGN(linet[line].delta.y);
    }

    if ((ABS(lsx) > SEPARATION_DIST || ABS(lsy) > SEPARATION_DIST) && (ABS(lsx + linet[line].delta.x) > SEPARATION_DIST || ABS(lsy + linet[line].delta.y) > SEPARATION_DIST)) {
	move->start.x = WRAP_XCLICK(move->start.x + dx);
	move->start.y = WRAP_YCLICK(move->start.y + dy);
	return -1;
    }

    lines = blockline[(move->start.x >> B_SHIFT) + mapx * (move->start.y >> B_SHIFT)].lines;
    while ( (i = *lines++) != 65535) {
	if (i == line)
	    continue;
	if (linet[i].group && (groups[linet[i].group].hit_mask & move->hit_mask))
	    continue;

	lsx = linet[i].start.x - move->start.x;
	lsy = linet[i].start.y - move->start.y;
	lsx = CENTER_XCLICK(lsx);
	lsy = CENTER_YCLICK(lsy);

	if ((ABS(lsx) > SEPARATION_DIST || ABS(lsy) > SEPARATION_DIST) && (ABS(lsx + linet[i].delta.x) > SEPARATION_DIST || ABS(lsy + linet[i].delta.y) > SEPARATION_DIST))
	    continue;

	if (lsx < dx && lsx + linet[i].delta.x < dx)
	    continue;
	if (lsx > dx && lsx + linet[i].delta.x > dx)
	    continue;
	if (lsy < dy && lsy + linet[i].delta.y < dy)
	    continue;
	if (lsy > dy && lsy + linet[i].delta.y > dy)
	    continue;

	if ((res = SIDE(lsx - dx, lsy - dy, i)) == 0 || res > 0 != SIDE(lsx, lsy, i) > 0) {
	    if (res) {
		if (lsx < 0 && lsx + linet[i].delta.x < 0)
		    continue;
		if (lsx > 0 && lsx + linet[i].delta.x > 0)
		    continue;
		if (lsy < 0 && lsy + linet[i].delta.y < 0)
		    continue;
		if (lsy > 0 && lsy + linet[i].delta.y > 0)
		    continue;
	    }
	    return i;
	}
    }

    move->start.x = WRAP_XCLICK(move->start.x + dx);
    move->start.y = WRAP_YCLICK(move->start.y + dy);
    return -1;
}

static int Shape_move1(int dx, int dy, struct move *move, const wireobj *shape, int dir, int *line, int *point)
{
    int i, p, lsx, lsy, res;
    unsigned short *lines, *points;
    int block;

    block = (move->start.x >> B_SHIFT) + mapx * (move->start.y >> B_SHIFT);
    for (p = 0; p < shape->num_points; p++) {
	lines = blockline[block].lines;
	/* Can use the same block for all points because the block of the
	   center point contains lines for their start & end positions. */
	while ( (i = *lines++) != 65535) {
	    if (linet[i].group && (groups[linet[i].group].hit_mask & move->hit_mask))
		continue;
	    lsx = linet[i].start.x - move->start.x - shape->pts[p][dir].x;
	    lsy = linet[i].start.y - move->start.y - shape->pts[p][dir].y;
	    lsx = CENTER_XCLICK(lsx);
	    lsy = CENTER_YCLICK(lsy);

	    if (lsx < dx && lsx + linet[i].delta.x < dx)
		continue;
	    if (lsx > dx && lsx + linet[i].delta.x > dx)
		continue;
	    if (lsy < dy && lsy + linet[i].delta.y < dy)
		continue;
	    if (lsy > dy && lsy + linet[i].delta.y > dy)
		continue;

	    if ( (res = SIDE(lsx - dx, lsy - dy, i)) == 0 || res > 0 != SIDE(lsx, lsy, i) > 0) {
		if (res) {
		    if (lsx < 0 && lsx + linet[i].delta.x < 0)
			continue;
		    if (lsx > 0 && lsx + linet[i].delta.x > 0)
			continue;
		    if (lsy < 0 && lsy + linet[i].delta.y < 0)
			continue;
		    if (lsy > 0 && lsy + linet[i].delta.y > 0)
			continue;
		}
		*line = i;
		return 0;
	    }
	}
    }

    points = blockline[block].points;
    while ( (p = *points++) != 65535) {
	if (linet[p].group && (groups[linet[p].group].hit_mask & move->hit_mask))
	    continue;
	lines = Shape_lines(shape, dir);
	while ( (i = *lines++) != 65535) {
	    lsx = linet[i].start.x + (linet[p].start.x - move->start.x);
	    lsy = linet[i].start.y + (linet[p].start.y - move->start.y);
	    lsx = CENTER_XCLICK(lsx);
	    lsy = CENTER_YCLICK(lsy);

	    if (lsx < dx && lsx + linet[i].delta.x < dx)
		continue;
	    if (lsx > dx && lsx + linet[i].delta.x > dx)
		continue;
	    if (lsy < dy && lsy + linet[i].delta.y < dy)
		continue;
	    if (lsy > dy && lsy + linet[i].delta.y > dy)
		continue;

	    if ( (res = SIDE(lsx - dx, lsy - dy, i)) == 0 || res > 0 != SIDE(lsx, lsy, i) > 0) {
		if (res) {
		    if (lsx < 0 && lsx + linet[i].delta.x < 0)
			continue;
		    if (lsx > 0 && lsx + linet[i].delta.x > 0)
			continue;
		    if (lsy < 0 && lsy + linet[i].delta.y < 0)
			continue;
		    if (lsy > 0 && lsy + linet[i].delta.y > 0)
			continue;
		}
		*line = i;
		*point = p;
		return 0;
	    }
	}
    }

    move->start.x = WRAP_XCLICK(move->start.x + dx);
    move->start.y = WRAP_YCLICK(move->start.y + dy);
    return 1;
}

static int Shape_away(struct move *move, const wireobj *shape, int dir, int line, int *rline, int *rpoint)
{
    int dx, dy;

    if (ABS(linet[line].delta.x) >= ABS(linet[line].delta.y)) {
	dx = 0;
	dy = -SIGN(linet[line].delta.x);
    }
    else {
	dy = 0;
	dx = SIGN(linet[line].delta.y);
    }
    return Shape_move1(dx, dy, move, shape, dir, rline, rpoint);
}

static int Lines_check(int msx, int msy, int mdx, int mdy, int *mindone, const unsigned short *lines, int chx, int chy, int chxy, const struct move *move, int *minline, int *height)
{
    int lsx, lsy, ldx, ldy, temp, bigger, start, end, i, x, sy, ey, prod;
    int mbase = mdy >> 1, hit = 0;

    while ( (i = *lines++) != 65535) {
	if (linet[i].group && (groups[linet[i].group].hit_mask & move->hit_mask))
	    continue;
	lsx = linet[i].start.x;
	lsy = linet[i].start.y;
	ldx = linet[i].delta.x;
	ldy = linet[i].delta.y;

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
	    if ( ABS(prod) >= ldx && ABS( (prod = (end - lsx) * ldy - (ey - lsy) * ldx) ) >= ldx && prod > 0 == bigger)
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
		    if ( (prod = (x - lsx) * ldy - (LINEY(mdx, mdy, mbase, x) - lsy) * ldx) >= 0 != bigger || prod == 0)
			goto found;
		continue;
	    found:
		start = x - 1;
	    }
	}

	if (start < *mindone || (start == *mindone && *minline != -1 && SIDE(move->delta.x, move->delta.y, i) < 0)) {
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
    int msx = move->start.x, msy = move->start.y;
    int mdx = move->delta.x, mdy = move->delta.y;
    int mbase;
    int chxy = 0, chx = 0, chy = 0;
    int x, temp;
    unsigned short *lines;

    block = (move->start.x >> B_SHIFT) + mapx * (move->start.y >> B_SHIFT);
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
	    temp = MIN(temp, (msy > 0 ? B_CLICKS - (msy & B_MASK) : -msy & B_MASK));
	    x += temp;
	    x = MIN(x, MAX_MOVE);
	}
	if (mindone > x) {
	    mindone = x;
	    minheight = LINEY(mdx, mdy, mbase, mindone);
	}
    }
    minline = -1;

    Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy, chxy, move, &minline, &minheight);

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
    answer->moved.x = mindone;
    answer->moved.y = minheight;

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
static void Shape_move(const struct move *move, const wireobj *shape, int dir, struct collans *answer)
{
    int minline, mindone, minheight, minpoint;
    int p, block;
    int msx = move->start.x, msy = move->start.y;
    int mdx = move->delta.x, mdy = move->delta.y;
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
	msx = WRAP_XCLICK(move->start.x + shape->pts[p][dir].x);
	msy = WRAP_YCLICK(move->start.y + shape->pts[p][dir].y);
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
		temp = MIN(temp, (msy > 0 ? B_CLICKS - (msy & B_MASK) : -msy & B_MASK));
		x += temp;
		x = MIN(x, MAX_MOVE);
	    }
	    if (mindone > x) {
		mindone = x;
		minheight = LINEY(mdx, mdy, mbase, mindone);
	    }
	}

	if (Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy, chxy, move, &minline, &minheight))
	    minpoint = p;
    }

    block = (move->start.x >> B_SHIFT) + mapx * (move->start.y >> B_SHIFT);
    points = blockline[block].points;
    lines = Shape_lines(shape, dir);
    x = -1;
    while ( ( p = *points++) != 65535) {
	if (linet[p].group && (groups[linet[p].group].hit_mask & move->hit_mask))
	    continue;
	msx = move->start.x - linet[p].start.x;
	msy = move->start.y - linet[p].start.y;
	if (chx)
	    msx = -msx;
	if (chy)
	    msy = -msy;
	if (chxy) {
	    temp = msx;
	    msx = msy;
	    msy = temp;
	}
	if (Lines_check(msx, msy, mdx, mdy, &mindone, lines, chx, chy, chxy, move, &minline, &minheight))
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
    answer->moved.x = mindone;
    answer->moved.y = minheight;

    return;
}

static int Shape_turn1(const wireobj *shape, int hitmask, int x, int y, int dir, int sign)
{
    struct move move;
    struct collans answer;
    int i, p, xo1, xo2, yo1, yo2, xn1, xn2, yn1, yn2, xp, yp, s, t;
    unsigned short *points;
    int newdir = MOD2(dir + sign, RES);

    move.hit_mask = hitmask;
    for (i = 0; i < shape->num_points; i++) {
	move.start.x = x + shape->pts[i][dir].x;
	move.start.y = y + shape->pts[i][dir].y;
	move.delta.x = x + shape->pts[i][newdir].x - move.start.x;
	move.delta.y = y + shape->pts[i][newdir].y - move.start.y;
	move.start.x = WRAP_XCLICK(move.start.x);
	move.start.y = WRAP_YCLICK(move.start.y);
	while (move.delta.x || move.delta.y) {
	    Move_point(&move, &answer);
	    if (answer.line != -1)
		return 0;
	    move.start.x = WRAP_XCLICK(move.start.x + answer.moved.x);
	    move.start.y = WRAP_YCLICK(move.start.y + answer.moved.y);
	    move.delta.x -= answer.moved.x;
	    move.delta.y -= answer.moved.y;
	}
    }

    /* Convex shapes would be much easier. */
    points = blockline[(x >> B_SHIFT) + mapx * (y >> B_SHIFT)].points;
    while ( (p = *points++) != 65535) {
	if (linet[p].group && (groups[linet[p].group].hit_mask & hitmask))
	    continue;
	xp = CENTER_XCLICK(linet[p].start.x - x);
	yp = CENTER_YCLICK(linet[p].start.y - y);
	xo1 = shape->pts[shape->num_points - 1][dir].x - xp;
	yo1 = shape->pts[shape->num_points - 1][dir].y - yp;
	xn1 = shape->pts[shape->num_points - 1][newdir].x - xp;
	yn1 = shape->pts[shape->num_points - 1][newdir].y - yp;
	t = 0;
	for (i = 0; i < shape->num_points; i++) {
	    xo2 = shape->pts[i][dir].x - xp;
	    yo2 = shape->pts[i][dir].y - yp;
	    xn2 = shape->pts[i][newdir].x - xp;
	    yn2 = shape->pts[i][newdir].y - yp;

#define TEMPFUNC(X1, Y1, X2, Y2)                                           \
	    if ((X1) < 0) {                                                \
		if ((X2) >= 0) {                                           \
		    if ((Y1) > 0 && (Y2) >= 0)                             \
			t++;                                               \
		    else if (((Y1) >= 0 || (Y2) >= 0) &&                   \
			     (s = (X1)*((Y1)-(Y2))-(Y1)*((X1)-(X2))) >= 0){\
			if (s == 0)                                        \
			    return 0;                                      \
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
			    return 0;                                      \
		    }                                                      \
		    else if ((Y1) > 0 && (Y2) >= 0)                        \
			t++;                                               \
		    else if (((Y1) >= 0 || (Y2) >= 0) &&                   \
			     (s = (X1)*((Y1)-(Y2))-(Y1)*((X1)-(X2))) <= 0){\
			if (s == 0)                                        \
			    return 0;                                      \
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
		return 0;
	    xo1 = xo2;
	    yo1 = yo2;
	    xn1 = xn2;
	    yn1 = yn2;
	}
    }
    return 1;
}


/* This function should get called only rarely, so it doesn't need to
   be too efficient. */
static int Clear_corner(struct move *move, object *obj, int l1, int l2)
{
    int x, y, xm, ym, s1, s2;
    int l1sx, l2sx, l1sy, l2sy, l1dx, l2dx, l1dy, l2dy;
    int side;

    l1sx = linet[l1].start.x - move->start.x;
    l1sy = linet[l1].start.y - move->start.y;
    l1sx = CENTER_XCLICK(l1sx);
    l1sy = CENTER_YCLICK(l1sy);
    l1dx = linet[l1].delta.x;
    l1dy = linet[l2].delta.y;
    l2sx = linet[l2].start.x - move->start.x;
    l2sy = linet[l2].start.y - move->start.y;
    l2sx = CENTER_XCLICK(l2sx);
    l2sy = CENTER_YCLICK(l2sy);
    l2dx = linet[l2].delta.x;
    l2dy = linet[l2].delta.y;

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

    xm = SIGN(move->delta.x);
    ym = SIGN(move->delta.y);

    s1 = SIDE(move->start.x - l1sx, move->start.y - l1sy, l1) > 0;
    s2 = SIDE(move->start.x - l2sx, move->start.y - l2sy, l2) > 0;

#define TMPFUNC(X, Y) ((side = SIDE((X), (Y), l1)) == 0 || side > 0 != s1 || (side = SIDE((X), (Y), l2)) == 0 || side > 0 != s2)

    if (ABS(obj->vel.x) >= ABS(obj->vel.y)) {
	x = xm;
	y = 0;
	for (;;) {
	    if (TMPFUNC(move->start.x + x, move->start.y + y)) {
		y += ym;
		if (!TMPFUNC(move->start.x + x, move->start.y + y + ym))
		    break;
		else
		    x += xm;;
	    }
	    else {
		if (TMPFUNC(move->start.x + x, move->start.y + y + 1) &&
		    TMPFUNC(move->start.x + x, move->start.y + y - 1))
		    x += xm;
		else
		    break;
	    }
	}
	move->delta.x -= x;
	move->delta.y -= y;
	if ((obj->vel.x >= 0) ^ (move->delta.x >= 0)) {
	    move->delta.x = 0;
	    move->delta.y = 0;
	}
    }
    else {
	x = 0;
	y = ym;
	for (;;) {
	    if (TMPFUNC(move->start.x + x, move->start.y + y)) {
		x += xm;
		if (!TMPFUNC(move->start.x + x + xm, move->start.y + y))
		    break;
		else
		    y += ym;
	    }
	    else {
		if (TMPFUNC(move->start.x + x + 1, move->start.y + y) &&
		    TMPFUNC(move->start.x + x - 1, move->start.y + y))
		    y += ym;
		else
		    break;
	    }
	}

#undef TMPFUNC

	move->delta.x -= x;
	move->delta.y -= y;
	if ((obj->vel.y >= 0) ^ (move->delta.y >= 0)) {
	    move->delta.x = 0;
	    move->delta.y = 0;
	}
    }
    move->start.x = WRAP_XCLICK(move->start.x + x);
    move->start.y = WRAP_YCLICK(move->start.y + y);
    return 1;
}

#ifdef _WINDOWS
#include "../common/NT/winNet.h"
#else
#include <netinet/in.h>
#endif

static void store_short(char **ptr, int i)
{
    *(*ptr)++ = i >> 8;
    *(*ptr)++ = i & 0xff;
}


static void store_32bit(char **ptr, int i)
{
    store_short(ptr, i >> 16);
    store_short(ptr, i & 0xffff);
}


int Polys_to_client(char *ptr)
{
    int i, j, startx, starty, dx, dy;
    int *edges;
    char *start = ptr;

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
	strcpy(ptr, bstyles[i].filename);
	ptr += strlen(ptr) + 1;
	*ptr++ = bstyles[i].flags;
    }
    store_short(&ptr, polyc);
    for (i = 0; i < polyc; i++) {
	*ptr++ = pdata[i].style;
	j = pdata[i].num_points;
	store_short(&ptr, pdata[i].num_echanges);
	edges = estyleptr + pdata[i].estyles_start;
	while (*edges != INT_MAX)
	    store_short(&ptr, *edges++);
	dx = 0;
	dy = 0;
	startx = pdata[i].x;
	starty = pdata[i].y;
	edges = pdata[i].edges;
	store_short(&ptr, j);
	store_short(&ptr, startx >> CLICK_SHIFT);
	store_short(&ptr, starty >> CLICK_SHIFT);
	startx = 0;
	starty = 0;
	for (; j > 0; j--) {
	    dx += *edges++;
	    dy += *edges++;
	    if (j != 1) {
		store_short(&ptr, (dx >> CLICK_SHIFT) - startx);
		store_short(&ptr, (dy >> CLICK_SHIFT) - starty);
	    }
	    startx = dx >> CLICK_SHIFT;
	    starty = dy >> CLICK_SHIFT;
	}
    }
    *ptr++ = World.NumBases;
    for (i = 0; i < World.NumBases; i++) {
	if (World.base[i].team == TEAM_NOT_SET)
	    *ptr++ = 0;
	else
	    *ptr++ = World.base[i].team;
	store_short(&ptr, World.base[i].pos.x >> CLICK_SHIFT);
	store_short(&ptr, World.base[i].pos.y >> CLICK_SHIFT);
	*ptr++ = World.base[i].dir;
    }
    store_short(&ptr, World.NumFuels);
    for (i = 0; i < World.NumFuels; i++) {
	store_short(&ptr, World.fuel[i].clk_pos.x >> CLICK_SHIFT);
	store_short(&ptr, World.fuel[i].clk_pos.y >> CLICK_SHIFT);
    }
    *ptr++ = World.NumChecks;
    for (i = 0; i < World.NumChecks; i++) {
	store_short(&ptr, World.check[i].x >> CLICK_SHIFT);
	store_short(&ptr, World.check[i].y >> CLICK_SHIFT);
    }
    return ptr - start;
}


#if 0
struct test {
    double distance;
    double direction;
    int fill;
};


struct test array[110][110];


#define STORE(T,P,N,M,V)						\
    if (N >= M && ((M <= 0)						\
	? (P = (T *) malloc((M = 1) * sizeof(*P)))			\
	: (P = (T *) realloc(P, (M += M) * sizeof(*P)))) == NULL) {	\
	error("No memory");						\
	exit(1);							\
    } else								\
	(P[N++] = V)


#define POSMOD(x, y) ((x) >= 0 ? (x) % (y) : (x) % (y) + (y))

static void inside_test(void)
{
    int dx, dy, bx, by, ox, oy, startx, starty, xsign, ysign, x1, x2, y1, y2;
    int i, j, last_height, last_width, num_points, minx, miny, poly;
    int bx2, by2, maxx, maxy;
    double xdist, ydist, dist;
    int *edges;

    for (i = 0; i < mapx; i++)
	for (j = 0; j < mapy; j++) {
	    array[i][j].distance = -1;
	    array[i][j].fill = 2;
	}
    last_width = (World.cwidth - 1) % B_CLICKS + 1;
    last_height = (World.cheight - 1) % B_CLICKS + 1;
    for (poly = 0; poly < polyc; poly++) {
	if (pdata[poly].is_decor)
	    continue;
/*	group = pdata[poly].group; !@# */
	num_points = pdata[poly].num_points;
	dx = 0;
	dy = 0;
	startx = pdata[poly].x;
	starty = pdata[poly].y;
	bx = startx >> B_SHIFT;
	by = starty >> B_SHIFT;
	bx2 = minx = maxx = bx;
	by2 = miny = maxy = by;
	edges = pdata[poly].edges;
	for (j = 0; j < num_points; j++) {
	    if (startx >> B_SHIFT != bx || starty >> B_SHIFT != by)
		while (1);
	    ox = startx & B_MASK;
	    oy = starty & B_MASK;
	    dx = *edges++;
	    dy = *edges++;
	    ysign = dy >= 0 ? 1 : -1;
	    xsign = dx >= 0 ? 1 : -1;
	    if (dy >= 0)
		y1 = 0;
	    else {
		y1 = B_CLICKS;
		if (by == mapy - 1)
		    y1 = last_height;
	    }
	    if (dx >= 0)
		x1 = 0;
	    else {
		x1 = B_CLICKS;
		if (bx == mapx - 1)
		    x1 = last_width;
	    }
	    if (ox >= array[bx][by].distance && (ox >array[bx][by].distance
		|| ABS(((double)dx) / dy) < array[bx][by].direction) && dy) {
		    array[bx][by].distance = ox;
		    array[bx][by].direction = ABS(((double)dx)/dy);
		    array[bx][by].fill = dy < 0;
	    }
	    x2 = x1 + xsign * B_CLICKS;
	    y2 = y1 + ysign * B_CLICKS;
	    if (bx == mapx - 1)
		x2 = x1 + xsign * last_width;
	    if (by == mapy - 1)
		y2 = y1 + ysign * last_height;
	    while (1) {
		if (array[bx][by].fill == 2)
		    array[bx][by].fill = 0;
		if (ABS(dx) >= ABS(dy)) {
		    xdist = (x2 - ox) * xsign;
		    if (xdist == -dx)
			xdist = 32768;
		    ydist = 32768;
		    if (dy != 0)
			ydist = ((double)(y2 - oy) * dx) / dy * xsign;
		    if (ydist == -dy)
			ydist = 32768;
		    if (xdist > dx * xsign && ydist > dx * xsign)
			break;
		}
		else {
		    ydist = (y2 - oy) * ysign;
		    if (ydist == -dy)
			ydist = 32768;
		    xdist = 32768;
		    if (dx != 0)
			xdist = ((double)(x2 - ox) * dy) / dx * ysign;
		    if (xdist == -dx)
			xdist = 32768;
		    if (xdist > dy * ysign && ydist > dy * ysign)
			break;
		}
		if (ydist <= xdist) {
		    if (ABS(dx) > ABS(dy))
			dist = ox + xsign * ydist - (xsign > 0 ? x1 : x2);
		    else
			dist = ox + ydist * ysign * dx / dy -
			    (xsign > 0 ? x1 : x2);
	    if (dist >= array[bx][by].distance && (dist >array[bx][by].distance
		|| ABS(((double)dx) / dy) < array[bx][by].direction)) {
		    array[bx][by].distance = dist;
		    array[bx][by].direction = ABS(((double)dx)/dy);
		    array[bx][by].fill = dy < 0;
	    }
		}
		/* If x/ydist is an integer, it should be exact
		 * on modern processors. */
		if (xdist <= ydist) {
		    bx2 += xsign;
		    if (bx2 > maxx)
			maxx = bx2;
		    if (bx2 < minx)
			minx = bx2;
		    bx = POSMOD(bx2, mapx);
		    x1 = x2;
		    x2 = x1 + xsign * B_CLICKS;
		    if (bx != mapx - 1) {
			x2 = x1 + xsign * B_CLICKS;
			dist -= B_CLICKS * xsign;
		    }
		    else {
			x2 = x1 + xsign * last_width;
			dist -= B_CLICKS * xsign;
		    }
		}
		if (ydist <= xdist) {
		    by2 += ysign;
		    if (by2 > maxy)
			maxy = by2;
		    if (by2 < miny)
			miny = by2;
		    by = POSMOD(by2, mapy);
		    y1 = y2;
		    y2 = y1 + ysign * B_CLICKS;
		    if (by == mapy - 1)
			y2 = y1 + ysign * last_height;
	    if (dist >= array[bx][by].distance && (dist >array[bx][by].distance
		|| ABS(((double)dx) / dy) < array[bx][by].direction)) {
		    array[bx][by].distance = dist;
		    array[bx][by].direction = ABS(((double)dx)/dy);
		    array[bx][by].fill = dy < 0;
	    }
		}
	    }
	    startx = WRAP_XCLICK(startx + dx);
	    starty = WRAP_YCLICK(starty + dy);
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
	    x1 = 0;
	    for (j = POSMOD(minx, mapx); bx2-- > 0; j++) {
		if (j == mapx)
		    j = 0;
		if (array[j][i].fill < 2) {
		    x1 = array[j][i].fill;
		}
		else {
		    if (x1) {
			fuel_t t;
			extern int max_fuels;
			t.clk_pos.x = j * B_CLICKS + B_CLICKS / 2;
			t.clk_pos.y = i * B_CLICKS + B_CLICKS / 2;
			t.fuel = START_STATION_FUEL;
			t.conn_mask = (unsigned)-1;
			t.last_change = frame_loops;
			t.team = 1;
			STORE(fuel_t, World.fuel, World.NumFuels, max_fuels,t);
		    }
		}
		array[j][i].fill = 2;
		array[j][i].distance = -1;
/*		array[j][i].direction = 0;*/
	    }
	}
    }
    return;
}
#endif


/* Include NCLLIN - 1 closest lines or all closer than CUTOFF (whichever
 * is less) in the line table for this block.
 * Include all lines closer than DICLOSE, however many there are.
 * LINSIZE tells the amout of temporary memory to reserve for the algorithm.
 * If it is not large enough to hold the required lines, print error and
 * exit.
 */

#define DICLOSE (5 * CLICK)
#define LINSIZE 50
#define NCLLIN (10 + 1)
static void Distance_init(void)
{
    int cx,cy;
    int *lineno, *dis;
    int lsx, lsy, ldx, ldy, temp, dist, n, i, bx, by, j, k;
    int base, height2, by2, width, height;
    int distbound, size;
    unsigned short *lptr;

    /* max line delta 32767 */

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
    for (i = 0; i < linec; i++) {
	bx = linet[i].start.x;
	by = linet[i].start.y;
	width = linet[i].delta.x;
	height = linet[i].delta.y;
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
	    for (by = by2, height = height2; height -- > 0; by = by == mapy - 1? 0 : by + 1) {
		cx = bx * B_CLICKS + B_CLICKS / 2;
		cy = by * B_CLICKS + B_CLICKS / 2;
		base = (by * mapx + bx) * LINSIZE;
		lsx = CENTER_XCLICK(linet[i].start.x - cx);
		if (ABS(lsx) > 32767 + MAX_MOVE + B_CLICKS / 2)
		    continue;
		lsy = CENTER_YCLICK(linet[i].start.y - cy);
		if (ABS(lsy) > 32767 + MAX_MOVE + B_CLICKS / 2)
		    continue;
		ldx = linet[i].delta.x;
		ldy = linet[i].delta.y;
		if (MAX(ABS(lsx), ABS(lsy)) > MAX(ABS(lsx + ldx), ABS(lsy + ldy))) {
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

#define DISIZE 100
    temp = ralloc(NULL, mapx * mapy * DISIZE * sizeof(short)); /* !@# */
    for (i = 0; i < mapx * mapy; i++)
	temp[i * DISIZE] = 0;
    for (i = 0; i < linec; i++) {
	bx = linet[i].start.x;
	by = linet[i].start.y;
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
	    for (by = by2, height = height2; height -- > 0; by = by == mapy - 1? 0 : by + 1) {
		block = bx + mapx * by;
		dist = blockline[block].distance + MAX_SHAPE_OFFSET + B_CLICKS / 2;
		cx = bx * B_CLICKS + B_CLICKS / 2;
		cy = by * B_CLICKS + B_CLICKS / 2;
		if (ABS(CENTER_XCLICK(linet[i].start.x - cx)) > dist)
		    continue;
		if (ABS(CENTER_YCLICK(linet[i].start.y - cy)) > dist)
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
	    errno = 0;
	    error("Not enough corner space in walls.c, add more.");
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


static void Ball_line_init(void)
{
    int i;
    static ipos coords[24];

    ball_wire.num_points = 24;

    for (i = 0; i < 24; i++) {
	ball_wire.pts[i] = coords + i;
	coords[i].x = cos(i * PI / 12) * BALL_RADIUS * CLICK;
	coords[i].y = sin(i * PI / 12) * BALL_RADIUS * CLICK;
    }

    return;
}


static void Poly_to_lines(void)
{
    int i, np, j, startx, starty, dx, dy, group, *styleptr, style;
    int *edges;

    linec = 0;
    for (i = 0; i < polyc; i++) {
	if (pdata[i].is_decor)
	    continue;
	group = pdata[i].group;
	np = pdata[i].num_points;
	styleptr = estyleptr + pdata[i].estyles_start;
	style = pstyles[pdata[i].style].defedge_id;
	dx = 0;
	dy = 0;
	startx = pdata[i].x;
	starty = pdata[i].y;
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
	    if (!(linec % 2000))
		linet = ralloc(linet, (linec + 2000) * sizeof(struct bline));
	    linet[linec].group = group;
	    linet[linec].start.x = TWRAP_XCLICK(startx + dx);
	    linet[linec].start.y = TWRAP_YCLICK(starty + dy);
	    linet[linec].delta.x = *edges;
	    dx += *edges++;
	    linet[linec++].delta.y = *edges;
	    dy += *edges++;
	}
	if (dx || dy) {
	    errno = 0;
	    error("Polygon %d (%d points) doesn't start and end at the same place", i + 1, np);
	    exit(1);
	}
    }
    linet = ralloc(linet, (linec + S_LINES) * sizeof(struct bline));
    return;
}


void Walls_init(void)
{
    DFLOAT x, y, l2;
    int i;

    mapx = (World.cwidth + B_MASK) >> B_SHIFT;
    mapy = (World.cheight + B_MASK) >> B_SHIFT;
    Poly_to_lines();
    Distance_init();
    Corner_init();
    Ball_line_init();
#if 0
    inside_test();
#endif
    groups[0].type = FILLED;

    for (i = 0; i < linec; i++) {
	x = linet[i].delta.x;
	y = linet[i].delta.y;
	l2 = (x*x + y*y);
	linet[i].c = (x*x - y*y) / l2;
	linet[i].s = 2*x*y / l2;
    }
}


/* end */
char walls_version[] = VERSION;

unsigned SPACE_BLOCKS = (
	SPACE_BIT | BASE_BIT | WORMHOLE_BIT |
	POS_GRAV_BIT | NEG_GRAV_BIT | CWISE_GRAV_BIT | ACWISE_GRAV_BIT |
	UP_GRAV_BIT | DOWN_GRAV_BIT | RIGHT_GRAV_BIT | LEFT_GRAV_BIT |
	DECOR_LU_BIT | DECOR_LD_BIT | DECOR_RU_BIT | DECOR_RD_BIT |
	DECOR_FILLED_BIT | CHECK_BIT | ITEM_CONCENTRATOR_BIT
    );

static char msg[MSG_LEN];


void Move_init(void)
{
    LIMIT(maxObjectWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxShieldedWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(maxUnshieldedWallBounceSpeed, 0, World.hypotenuse);
    LIMIT(playerWallBrakeFactor, 0, 1);
    LIMIT(objectWallBrakeFactor, 0, 1);
    LIMIT(objectWallBounceLifeFactor, 0, 1);
    LIMIT(wallBounceFuelDrainMult, 0, 1000);
    wallBounceExplosionMult = sqrt(wallBounceFuelDrainMult);

    mp.obj_bounce_mask = 0;
    if (sparksWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SPARK);
    }
    if (debrisWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_DEBRIS);
    }
    if (shotsWallBounce) {
	SET_BIT(mp.obj_bounce_mask, OBJ_SHOT);
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

    mp.obj_cannon_mask = (KILLING_SHOTS) | OBJ_MINE | OBJ_SHOT | OBJ_PULSE |
			OBJ_SMART_SHOT | OBJ_TORPEDO | OBJ_HEAT_SHOT;
    if (cannonsUseItems)
	mp.obj_cannon_mask |= OBJ_ITEM;
    mp.obj_target_mask = mp.obj_cannon_mask | OBJ_BALL | OBJ_SPARK;
    mp.obj_treasure_mask = mp.obj_bounce_mask | OBJ_BALL | OBJ_PULSE;
}


#if 0
static void Bounce_wall(move_state_t *ms, move_bounce_t bounce)
{
    /* This set ->dir too */
}

/*
 * Move a point through one block and detect
 * wall collisions or bounces within that block.
 * Complications arise when the point starts at
 * the edge of a block.  E.g., if a point is on the edge
 * of a block to which block does it belong to?
 *
 * The caller supplies a set of input parameters and expects
 * the following output:
 *  - the number of pixels moved within this block.  (ms->done)
 *  - the number of pixels that still remain to be traversed. (ms->todo)
 *  - whether a crash happened, in which case no pixels will have been
 *    traversed. (ms->crash)
 *  - some extra optional output parameters depending upon the type
 *    of the crash. (ms->cannon, ms->wormhole, ms->target, ms->treasure)
 *  - whether the point bounced, in which case no pixels will have been
 *    traversed, only a change in direction. (ms->bounce, ms->vel, ms->todo)
 */
static void Move_segment(move_state_t *ms)
{
    int			i;
    int			block_type;	/* type of block we're going through */
    int			inside;		/* inside the block or else on edge */
    int			need_adjust;	/* other param (x or y) needs recalc */
    unsigned		wall_bounce;	/* are we bouncing? what direction? */
    ipos		block;		/* block index */
    ipos		blk2;		/* new block index */
    ivec		sign;		/* sign (-1 or 1) of direction */
    clpos		delta;		/* delta position in clicks */
    clpos		enter;		/* enter block position in clicks */
    clpos		leave;		/* leave block position in clicks */
    clpos		offset;		/* offset within block in clicks */
    const move_info_t	*const mi = ms->mip;	/* alias */
    int			hole;		/* which wormhole */

    /*
     * Fill in default return values.
     */
    ms->crash = NotACrash;
    ms->bounce = NotABounce;
    ms->done.x = 0;
    ms->done.y = 0;

    enter = ms->pos;
    if (enter.x < 0 || enter.x >= mp.click_width
	|| enter.y < 0 || enter.y >= mp.click_height) {

	if (!mi->edge_wrap) {
	    ms->crash = CrashUniverse;
	    return;
	}
	if (enter.x < 0) {
	    enter.x += mp.click_width;
	    if (enter.x < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.x >= mp.click_width) {
	    enter.x -= mp.click_width;
	    if (enter.x >= mp.click_width) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	if (enter.y < 0) {
	    enter.y += mp.click_height;
	    if (enter.y < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.y >= mp.click_height) {
	    enter.y -= mp.click_height;
	    if (enter.y >= mp.click_height) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	ms->pos = enter;
    }

    sign.x = (ms->vel.x < 0) ? -1 : 1;
    sign.y = (ms->vel.y < 0) ? -1 : 1;
    block.x = enter.x / BLOCK_CLICKS;
    block.y = enter.y / BLOCK_CLICKS;
    if (walldist[block.x][block.y] > 2) {
	int maxcl = ((walldist[block.x][block.y] - 2) * BLOCK_CLICKS) >> 1;
	if (maxcl >= sign.x * ms->todo.x && maxcl >= sign.y * ms->todo.y) {
	    /* entire movement is possible. */
	    ms->done.x = ms->todo.x;
	    ms->done.y = ms->todo.y;
	}
	else if (sign.x * ms->todo.x > sign.y * ms->todo.y) {
	    /* horizontal movement. */
	    ms->done.x = sign.x * maxcl;
	    ms->done.y = ms->todo.y * maxcl / (sign.x * ms->todo.x);
	}
	else {
	    /* vertical movement. */
	    ms->done.x = ms->todo.x * maxcl / (sign.y * ms->todo.y);
	    ms->done.y = sign.y * maxcl;
	}
	ms->todo.x -= ms->done.x;
	ms->todo.y -= ms->done.y;
	return;
    }

    offset.x = enter.x - block.x * BLOCK_CLICKS;
    offset.y = enter.y - block.y * BLOCK_CLICKS;
    inside = 1;
    need_adjust = 0;
    if (sign.x == -1) {
	if (offset.x + ms->todo.x < 0) {
	    leave.x = enter.x - offset.x;
	    need_adjust = 1;
	}
	else {
	    leave.x = enter.x + ms->todo.x;
	}
    }
    else {
	if (offset.x + ms->todo.x > BLOCK_CLICKS) {
	    leave.x = enter.x + BLOCK_CLICKS - offset.x;
	    need_adjust = 1;
	}
	else {
	    leave.x = enter.x + ms->todo.x;
	}
	if (leave.x == mp.click_width && !mi->edge_wrap) {
	    leave.x--;
	    need_adjust = 1;
	}
    }
    if (sign.y == -1) {
	if (offset.y + ms->todo.y < 0) {
	    leave.y = enter.y - offset.y;
	    need_adjust = 1;
	}
	else {
	    leave.y = enter.y + ms->todo.y;
	}
    }
    else {
	if (offset.y + ms->todo.y > BLOCK_CLICKS) {
	    leave.y = enter.y + BLOCK_CLICKS - offset.y;
	    need_adjust = 1;
	}
	else {
	    leave.y = enter.y + ms->todo.y;
	}
	if (leave.y == mp.click_height && !mi->edge_wrap) {
	    leave.y--;
	    need_adjust = 1;
	}
    }
    if (need_adjust && ms->todo.y && ms->todo.x) {
	double wx = (double)(leave.x - enter.x) / ms->todo.x;
	double wy = (double)(leave.y - enter.y) / ms->todo.y;
	if (wx > wy) {
	    double x = ms->todo.x * wy;
	    leave.x = enter.x + DOUBLE_TO_INT(x);
	}
	else if (wx < wy) {
	    double y = ms->todo.y * wx;
	    leave.y = enter.y + DOUBLE_TO_INT(y);
	}
    }

    delta.x = leave.x - enter.x;
    delta.y = leave.y - enter.y;

    block_type = World.block[block.x][block.y];

    /*
     * We test for several different bouncing directions against the wall.
     * Sometimes there is more than one bounce possible if the point
     * starts at the corner of a block.
     * Therefore we maintain a bit mask for the bouncing possibilities
     * and later we will determine which bounce is appropriate.
     */
    wall_bounce = 0;

    if (!mi->phased) {

    switch (block_type) {

    default:
	break;

    case WORMHOLE:
	if (!mi->wormhole_warps) {
	    break;
	}
	hole = World.itemID[block.x][block.y];
	if (World.wormHoles[hole].type == WORM_OUT) {
	    break;
	}
	if (mi->pl) {
	    blk2.x = OBJ_X_IN_BLOCKS(mi->pl);
	    blk2.y = OBJ_Y_IN_BLOCKS(mi->pl);
	    if (BIT(mi->pl->status, WARPED)) {
		if (World.block[blk2.x][blk2.y] == WORMHOLE) {
		    int oldhole = World.itemID[blk2.x][blk2.y];
		    if (World.wormHoles[oldhole].type == WORM_NORMAL
			&& mi->pl->wormHoleDest == oldhole) {
			/*
			 * Don't warp again if we are still on the
			 * same wormhole we have just been warped to.
			 */
			break;
		    }
		}
		CLR_BIT(mi->pl->status, WARPED);
	    }
	    if (blk2.x == block.x && blk2.y == block.y) {
		ms->wormhole = hole;
		ms->crash = CrashWormHole;
		return;
	    }
	}
	else {
	    /*
	     * Warp object if this wormhole has ever warped a player.
	     * Warp the object to the same destination as the
	     * player has been warped to.
	     */
	    int last = World.wormHoles[hole].lastdest;
	    if (last >= 0
		&& (World.wormHoles[hole].countdown > 0 || !wormTime)
		&& last < World.NumWormholes
		&& World.wormHoles[last].type != WORM_IN
		&& last != hole
		&& (OBJ_X_IN_BLOCKS(mi->obj) != block.x
		 || OBJ_Y_IN_BLOCKS(mi->obj) != block.y) ) {
		ms->done.x += (World.wormHoles[last].pos.x
		    - World.wormHoles[hole].pos.x) * BLOCK_CLICKS;
		ms->done.y += (World.wormHoles[last].pos.y
		    - World.wormHoles[hole].pos.y) * BLOCK_CLICKS;
		break;
	    }
	}
	break;

    case CANNON:
	if (!mi->cannon_crashes) {
	    break;
	}
	if (BIT(mi->obj->status, FROMCANNON)
	    && !BIT(World.rules->mode, TEAM_PLAY)) {
	    break;
	}
	for (i = 0; ; i++) {
	    if (World.cannon[i].blk_pos.x == block.x
		&& World.cannon[i].blk_pos.y == block.y) {
		break;
	    }
	}
	ms->cannon = i;

	if (BIT(World.rules->mode, TEAM_PLAY)
	    && (teamImmunity
		|| BIT(mi->obj->status, FROMCANNON))
	    && mi->obj->team == World.cannon[i].team) {
	    break;
	}
	{
	    /*
	     * Calculate how far the point can travel in the cannon block
	     * before hitting the cannon.
	     * To reduce duplicate code we first transform all the
	     * different cannon types into one by matrix multiplications.
	     * Later we transform the result back to the real type.
	     */

	    ivec mx, my, dir;
	    clpos mirx, miry, start, end, todo, done, diff, a, b;
	    double d, w;

	    mirx.x = 0;
	    mirx.y = 0;
	    miry.x = 0;
	    miry.y = 0;
	    switch (World.cannon[i].dir) {
	    case DIR_UP:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = 1;
		break;
	    case DIR_DOWN:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = -1;
		miry.y = BLOCK_CLICKS;
		break;
	    case DIR_RIGHT:
		mx.x = 0; mx.y = 1;
		my.x = -1; my.y = 0;
		miry.x = BLOCK_CLICKS;
		break;
	    case DIR_LEFT:
		mx.x = 0; mx.y = -1;
		my.x = 1; my.y = 0;
		mirx.y = BLOCK_CLICKS;
		break;
	    }
	    start.x = mirx.x + mx.x * offset.x + miry.x + my.x * offset.y;
	    start.y = mirx.y + mx.y * offset.x + miry.y + my.y * offset.y;
	    diff.x  =          mx.x * delta.x           + my.x * delta.y;
	    diff.y  =          mx.y * delta.x           + my.y * delta.y;
	    dir.x   =          mx.x * sign.x            + my.x * sign.y;
	    dir.y   =          mx.y * sign.x            + my.y * sign.y;
	    todo.x  =          mx.x * ms->todo.x       + my.x * ms->todo.y;
	    todo.y  =          mx.y * ms->todo.x       + my.y * ms->todo.y;

	    end.x = start.x + diff.x;
	    end.y = start.y + diff.y;

	    if (start.x <= BLOCK_CLICKS/2) {
		if (3 * start.y <= 2 * start.x) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.x <= BLOCK_CLICKS/2) {
		    if (3 * end.y > 2 * end.x) {
			break;
		    }
		}
	    }
	    else {
		if (3 * start.y <= 2 * (BLOCK_CLICKS - start.x)) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.x > BLOCK_CLICKS/2) {
		    if (3 * end.y > 2 * (BLOCK_CLICKS - end.x)) {
			break;
		    }
		}
	    }

	    done = diff;

	    /* is direction x-major? */
	    if (dir.x * diff.x >= dir.y * diff.y) {
		/* x-major */
		w = (double) todo.y / todo.x;
		if (3 * todo.y != 2 * todo.x) {
		    d = (3 * start.y - 2 * start.x) / (2 - 3 * w);
		    a.x = DOUBLE_TO_INT(d);
		    a.y = (int)(a.x * w);
		    if (dir.x * a.x < dir.x * done.x && dir.x * a.x >= 0) {
			if (start.y + a.y <= BLOCK_CLICKS/3) {
			    done = a;
			    if (!(done.x | done.y)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
		if (-3 * todo.y != 2 * todo.x) {
		    d = (2 * BLOCK_CLICKS - 2 * start.x - 3 * start.y) /
			(2 + 3 * w);
		    b.x = DOUBLE_TO_INT(d);
		    b.y = (int)(b.x * w);
		    if (dir.x * b.x < dir.x * done.x && dir.x * b.x >= 0) {
			if (start.y + b.y <= BLOCK_CLICKS/3) {
			    done = b;
			    if (!(done.x | done.y)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
	    } else {
		/* y-major */
		w = (double) todo.x / todo.y;
		d = (2 * start.x - 3 * start.y) / (3 - 2 * w);
		a.y = DOUBLE_TO_INT(d);
		a.x = (int)(a.y * w);
		if (dir.y * a.y < dir.y * done.y && dir.y * a.y >= 0) {
		    if (start.y + a.y <= BLOCK_CLICKS/3) {
			done = a;
			if (!(done.x | done.y)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
		d = (2 * BLOCK_CLICKS - 2 * start.x - 3 * start.y) /
		    (3 + 2 * w);
		b.y = DOUBLE_TO_INT(d);
		b.x = (int)(b.y * w);
		if (dir.y * b.y < dir.y * done.y && dir.y * b.y >= 0) {
		    if (start.y + b.y <= BLOCK_CLICKS/3) {
			done = b;
			if (!(done.x | done.y)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
	    }

	    delta.x = mx.x * done.x + mx.y * done.y;
	    delta.y = my.x * done.x + my.y * done.y;
	}
	break;

    case TREASURE:
	/* removed */
	/*FALLTHROUGH*/

    case TARGET:
	if (block_type == TARGET) {
	    if (mi->target_crashes) {
		/*-BA This can be slow for large number of targets.
		 *     added itemID array for extra speed, (at cost of some memory.)
		 *
		 *for (i = 0; ; i++) {
		 *    if (World.targets[i].pos.x == block.x
		 *	&& World.targets[i].pos.y == block.y) {
		 *	break;
		 *     }
		 * }
		 *
		 * ms->target = i;
		 */
		ms->target = i = World.itemID[block.x][block.y];

		if (!targetTeamCollision) {
		    int team;
		    if (mi->pl) {
			team = mi->pl->team;
		    }
		    else if (BIT(mi->obj->type, OBJ_BALL)) {
			if (mi->obj->owner != -1) {
			    team = Players[GetInd[mi->obj->owner]]->team;
			} else {
			    team = TEAM_NOT_SET;
			}
		    }
		    else {
			team = mi->obj->team;
		    }
		    if (team == World.targets[i].team) {
			break;
		    }
		}
		if (!mi->pl) {
		    ms->crash = CrashTarget;
		    return;
		}
	    }
	}
	/*FALLTHROUGH*/

    case FUEL:
    case FILLED:
	if (inside) {
	    /* Could happen for targets reappearing and in case of bugs. */
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	}
	else if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	}
	else if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (!(ms->todo.x | ms->todo.y)) {
	    /* no bouncing possible and no movement.  OK. */
	    break;
	}
	if (!ms->todo.x && (offset.x == 0 || offset.x == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	if (!ms->todo.y && (offset.y == 0 || offset.y == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	/* what happened? we should never reach this */
	ms->crash = CrashWall;
	return;

    case REC_LD:
	/* test for bounces first. */
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	    if (offset.y == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	    if (offset.x == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x + offset.y < BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x + offset.y + delta.y >= BLOCK_CLICKS) {
	    /* movement is entirely within the space part of the block. */
	    break;
	}
	/*
	 * Find out where we bounce exactly
	 * and how far we can move before bouncing.
	 */
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (int)((BLOCK_CLICKS - offset.x - offset.y) / (1 + w));
	    delta.y = (int)(delta.x * w);
	    if (offset.x + delta.x + offset.y + delta.y < BLOCK_CLICKS) {
		delta.x++;
		delta.y = (int)(delta.x * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (int)((BLOCK_CLICKS - offset.x - offset.y) / (1 + w));
	    delta.x = (int)(delta.y * w);
	    if (offset.x + delta.x + offset.y + delta.y < BLOCK_CLICKS) {
		delta.y++;
		delta.x = (int)(delta.y * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	break;

    case REC_LU:
	if (offset.x == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceHorLo;
	    }
	    if (offset.y == 0 && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	    if (offset.x == BLOCK_CLICKS && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x < offset.y) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x >= offset.y + delta.y) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (int)((offset.y - offset.x) / (1 - w));
	    delta.y = (int)(delta.x * w);
	    if (offset.x + delta.x < offset.y + delta.y) {
		delta.x++;
		delta.y = (int)(delta.x * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (int)((offset.x - offset.y) / (1 - w));
	    delta.x = (int)(delta.y * w);
	    if (offset.x + delta.x < offset.y + delta.y) {
		delta.y--;
		delta.x = (int)(delta.y * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	break;

    case REC_RD:
	if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	    if (offset.y == BLOCK_CLICKS && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (offset.y == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceVerLo;
	    }
	    if (offset.x == 0 && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x > offset.y) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x <= offset.y + delta.y) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (int)((offset.y - offset.x) / (1 - w));
	    delta.y = (int)(delta.x * w);
	    if (offset.x + delta.x > offset.y + delta.y) {
		delta.x--;
		delta.y = (int)(delta.x * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (int)((offset.x - offset.y) / (1 - w));
	    delta.x = (int)(delta.y * w);
	    if (offset.x + delta.x > offset.y + delta.y) {
		delta.y++;
		delta.x = (int)(delta.y * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	break;

    case REC_RU:
	if (offset.x == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceHorHi;
	    }
	    if (offset.y == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (offset.y == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceVerHi;
	    }
	    if (offset.x == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.x + offset.y > BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.x + delta.x + offset.y + delta.y <= BLOCK_CLICKS) {
	    break;
	}
	if (sign.x * ms->todo.x >= sign.y * ms->todo.y) {
	    double w = (double) ms->todo.y / ms->todo.x;
	    delta.x = (int)((BLOCK_CLICKS - offset.x - offset.y) / (1 + w));
	    delta.y = (int)(delta.x * w);
	    if (offset.x + delta.x + offset.y + delta.y > BLOCK_CLICKS) {
		delta.x--;
		delta.y = (int)(delta.x * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.x) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.x / ms->todo.y;
	    delta.y = (int)((BLOCK_CLICKS - offset.x - offset.y) / (1 + w));
	    delta.x = (int)(delta.y * w);
	    if (offset.x + delta.x + offset.y + delta.y > BLOCK_CLICKS) {
		delta.y--;
		delta.x = (int)(delta.y * w);
	    }
	    leave.x = enter.x + delta.x;
	    leave.y = enter.y + delta.y;
	    if (!delta.y) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	break;
    }

    if (wall_bounce) {
	/*
	 * Bouncing.  As there may be more than one possible bounce
	 * test which bounce is not feasible because of adjacent walls.
	 * If there still is more than one possible then pick one randomly.
	 * Else if it turns out that none is feasible then we must have
	 * been trapped inbetween two blocks.  This happened in the early
	 * stages of this code.
	 */
	int count = 0;
	unsigned bit;
	unsigned save_wall_bounce = wall_bounce;
	unsigned block_mask = FILLED_BIT | FUEL_BIT;

	if (!mi->target_crashes) {
	    block_mask |= TARGET_BIT;
	}
	if (!mi->treasure_crashes) {
	    block_mask |= TREASURE_BIT;
	}
	for (bit = 1; bit <= wall_bounce; bit <<= 1) {
	    if (!(wall_bounce & bit)) {
		continue;
	    }

	    CLR_BIT(wall_bounce, bit);
	    switch (bit) {

	    case BounceHorLo:
		blk2.x = block.x - 1;
		if (blk2.x < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x += World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_RD_BIT)) {
		    continue;
		}
		break;

	    case BounceHorHi:
		blk2.x = block.x + 1;
		if (blk2.x >= World.x) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x -= World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_LU_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;

	    case BounceVerLo:
		blk2.x = block.x;
		blk2.y = block.y - 1;
		if (blk2.y < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y += World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_LU_BIT)) {
		    continue;
		}
		break;

	    case BounceVerHi:
		blk2.x = block.x;
		blk2.y = block.y + 1;
		if (blk2.y >= World.y) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y -= World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RD_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;
	    }

	    SET_BIT(wall_bounce, bit);
	    count++;
	}

	if (!count) {
	    wall_bounce = save_wall_bounce;
	    switch (wall_bounce) {
	    case BounceHorLo|BounceVerLo:
		wall_bounce = BounceLeftDown;
		break;
	    case BounceHorLo|BounceVerHi:
		wall_bounce = BounceLeftUp;
		break;
	    case BounceHorHi|BounceVerLo:
		wall_bounce = BounceRightDown;
		break;
	    case BounceHorHi|BounceVerHi:
		wall_bounce = BounceRightUp;
		break;
	    default:
		switch (block_type) {
		case REC_LD:
		    if ((offset.x == 0) ? (offset.y == BLOCK_CLICKS)
			: (offset.x == BLOCK_CLICKS && offset.y == 0)
			&& ms->vel.x + ms->vel.y >= 0) {
			wall_bounce = 0;
		    }
		    break;
		case REC_LU:
		    if ((offset.x == 0) ? (offset.y == 0)
			: (offset.x == BLOCK_CLICKS && offset.y == BLOCK_CLICKS)
			&& ms->vel.x >= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RD:
		    if ((offset.x == 0) ? (offset.y == 0)
			: (offset.x == BLOCK_CLICKS && offset.y == BLOCK_CLICKS)
			&& ms->vel.x <= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RU:
		    if ((offset.x == 0) ? (offset.y == BLOCK_CLICKS)
			: (offset.x == BLOCK_CLICKS && offset.y == 0)
			&& ms->vel.x + ms->vel.y <= 0) {
			wall_bounce = 0;
		    }
		    break;
		}
		if (wall_bounce) {
		    ms->crash = CrashWall;
		    return;
		}
	    }
	}
	else if (count > 1) {
	    /*
	     * More than one bounce possible.
	     * Pick one randomly.
	     */
	    count = (int)(rfrac() * count);
	    for (bit = 1; bit <= wall_bounce; bit <<= 1) {
		if (wall_bounce & bit) {
		    if (count == 0) {
			wall_bounce = bit;
			break;
		    } else {
			count--;
		    }
		}
	    }
	}
    }

    } /* phased */

    if (wall_bounce) {
	Bounce_wall(ms, (move_bounce_t) wall_bounce);
    }
    else {
	ms->done.x += delta.x;
	ms->done.y += delta.y;
	ms->todo.x -= delta.x;
	ms->todo.y -= delta.y;
    }
}


static void Cannon_dies(move_state_t *ms)
{
    cannon_t           *cannon = World.cannon + ms->cannon;
    int			x = CLICK_TO_PIXEL(cannon->clk_pos.x);
    int			y = CLICK_TO_PIXEL(cannon->clk_pos.y);
    int			sc;
    int			killer = -1;
    player		*pl = NULL;

    cannon->dead_time = CANNON_DEAD_TIME;
    cannon->conn_mask = 0;
    World.block[cannon->blk_pos.x][cannon->blk_pos.y] = SPACE;
    Cannon_throw_items(ms->cannon);
    Cannon_init(ms->cannon);
    sound_play_sensors(x, y, CANNON_EXPLOSION_SOUND);
    Make_debris(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ -1,
	/* owner team	  */ cannon->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 4.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 6,
	/* min,max debris */ 20, 40,
	/* min,max dir    */ (int)(cannon->dir - (RES * 0.2)), (int)(cannon->dir + (RES * 0.2)),
	/* min,max speed  */ 20, 50,
	/* min,max life   */ 8, 68
	);
    Make_wreckage(
	/* pos.x, pos.y   */ x, y,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ -1,
	/* owner team	  */ cannon->team,
	/* min,max mass   */ 3.5, 23,
	/* total mass     */ 28,
	/* status         */ GRAVITY,
	/* color          */ WHITE,
	/* max wreckage   */ 10,
	/* min,max dir    */ (int)(cannon->dir - (RES * 0.2)), (int)(cannon->dir + (RES * 0.2)),
	/* min,max speed  */ 10, 25,
	/* min,max life   */ 8, 68
	);

    if (!ms->mip->pl) {
	if (ms->mip->obj->id != -1) {
	    killer = GetInd[ms->mip->obj->id];
	    pl = Players[killer];
	}
    } else if (BIT(ms->mip->pl->used, OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)
	       == (OBJ_SHIELD|OBJ_EMERGENCY_SHIELD)) {
	pl = ms->mip->pl;
	killer = GetInd[pl->id];
    }
    if (pl) {
	sc = Rate(pl->score, CANNON_SCORE) / 4;
	if (BIT(World.rules->mode, TEAM_PLAY)
	    && pl->team == cannon->team) {
	    sc = -sc;
	}
	SCORE(killer, sc, cannon->blk_pos.x, cannon->blk_pos.y, "");
    }
}

static void Object_hits_target(move_state_t *ms, long player_cost)
{
    target_t		*targ = &World.targets[ms->target];
    object		*obj = ms->mip->obj;
    int			j, sc, por,
			x, y,
			killer;
    int			win_score = 0,
			win_team_members = 0,
			lose_score = 0,
			lose_team_members = 0,
			somebody_flag = 0,
			targets_remaining = 0,
			targets_total = 0;
    DFLOAT 		drainfactor;

    /* a normal shot or a direct mine hit work, cannons don't */
    /* BD: should shots/mines by cannons of opposing teams work? */
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
#ifdef DRAINFACTOR
/* BG: this is bad: one shot causes way too much damage. */
	drainfactor = VECTOR_LENGTH(obj->vel);
    	drainfactor = (drainfactor * drainfactor * ABS(obj->mass)) / (ShotsSpeed * ShotsSpeed * ShotsMass);
#else
	drainfactor = 1;
#endif
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

    targ->update_mask = (unsigned) -1;
    targ->damage = TARGET_DAMAGE;
    targ->dead_time = TARGET_DEAD_TIME;

    /*
     * Destroy target.
     * Turn it into a space to simplify other calculations.
     */
    x = targ->pos.x;
    y = targ->pos.y;
    World.block[x][y] = SPACE;

    Make_debris(
	/* pos.x, pos.y   */ (x+0.5f) * BLOCK_CLICKS, (y+0.5f) * BLOCK_CLICKS,
	/* vel.x, vel.y   */ 0.0, 0.0,
	/* owner id       */ -1,
	/* owner team	  */ targ->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 4.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 6,
	/* min,max debris */ 75, 150,
	/* min,max dir    */ 0, RES-1,
	/* min,max speed  */ 20, 70,
	/* min,max life   */ 10, 100
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

    sound_play_sensors(x, y, DESTROY_TARGET_SOUND);

    if (targets_remaining > 0) {
	sc = Rate(Players[killer]->score, CANNON_SCORE)/4;
	sc = sc * (targets_total - targets_remaining) / (targets_total + 1);
	if (sc > 0) {
	    SCORE(killer, sc,
		  targ->pos.x, targ->pos.y, "Target: ");
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
	Players[killer]->kills++;
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
	    SCORE(j, -sc, targ->pos.x, targ->pos.y,
		  "Target: ");
	}
	else if (Players[j]->team == Players[killer]->team &&
		 (Players[j]->team != TEAM_NOT_SET || j == killer)) {
	    SCORE(j, por, targ->pos.x, targ->pos.y,
		  "Target: ");
	}
    }
}


static void Object_crash(move_state_t *ms)
{
    object		*obj = ms->mip->obj;

    switch (ms->crash) {

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
	Object_hits_target(ms, -1);
	break;

    case CrashWall:
	obj->life = 0;
#if 0
/* GK: - Added sparks to wallcrashes for objects != OBJ_SPARK|OBJ_DEBRIS.
**       I'm not sure of the amount of sparks or the direction.
*/
	if (!BIT(obj->type, OBJ_SPARK | OBJ_DEBRIS)) {
	    Make_debris(ms->pos.x,
			ms->pos.y,
			0, 0,
			obj->owner,
			obj->team,
			OBJ_SPARK,
			(obj->mass * VECTOR_LENGTH(obj->vel)) / 3,
			GRAVITY,
			RED,
			1,
			5, 10,
			MOD2(ms->dir - RES/4, RES), MOD2(ms->dir + RES/4, RES),
			15, 25,
			5, 15);
	}
#endif
	break;

    case CrashUniverse:
	obj->life = 0;
	break;

    case CrashCannon:
	obj->life = 0;
	if (BIT(obj->type, OBJ_ITEM)) {
	    Cannon_add_item(ms->cannon, obj->info, obj->count);
	} else {
	    if (World.cannon[ms->cannon].item[ITEM_ARMOR] > 0)
		World.cannon[ms->cannon].item[ITEM_ARMOR]--;
	    else
		Cannon_dies(ms);
	}
	break;

    case CrashUnknown:
	obj->life = 0;
	break;
    }
}
#endif


static void Move_ball(int ind)
{
    object		*obj = Obj[ind];
    int line, point;
    struct move move;
    struct collans ans;

    obj->extpos.x = obj->pos.cx + FLOAT_TO_CLICK(obj->vel.x);
    while (obj->extpos.x >= World.cwidth)
	obj->extpos.x -= World.cwidth;
    while (obj->extpos.x <= 0)
	obj->extpos.x += World.cwidth;
    obj->extpos.y = obj->pos.cy + FLOAT_TO_CLICK(obj->vel.y);
    while (obj->extpos.y >= World.cheight)
	obj->extpos.y -= World.cheight;
    while (obj->extpos.y <0)
	obj->extpos.y += World.cheight;

    if (obj->id != -1 && BIT(Players[GetInd[obj->id]]->used, OBJ_PHASING_DEVICE)) {
	Object_position_set_clicks(obj, obj->extpos.x, obj->extpos.y);
	return;
    }
    if (obj->owner == -1 || Players[GetInd[obj->owner]]->team == TEAM_NOT_SET)
	move.hit_mask = BALL_BIT | NOTEAM_BIT;
    else
	move.hit_mask = BALL_BIT | 1 << Players[GetInd[obj->owner]]->team;
    move.start.x = obj->pos.cx;
    move.start.y = obj->pos.cy;
    move.delta.x = FLOAT_TO_CLICK(obj->vel.x);
    move.delta.y = FLOAT_TO_CLICK(obj->vel.y);
    while (move.delta.x || move.delta.y) {
	Shape_move(&move, &ball_wire, 0, &ans);
	move.start.x = WRAP_XCLICK(move.start.x + ans.moved.x);
	move.start.y = WRAP_YCLICK(move.start.y + ans.moved.y);
	move.delta.x -= ans.moved.x;
	move.delta.y -= ans.moved.y;
	if (ans.line != -1) {
	    if (!Shape_away(&move, &ball_wire, 0, ans.line, &line, &point)) {
		if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0) {
		    if (!Bounce_object(obj, &move, ans.line, ans.point))
			break;
		}
		else if (SIDE(obj->vel.x, obj->vel.y, line) < 0) {
		    if (!Bounce_object(obj, &move, line, point))
			break;
		}
		else {
		    /* This case could be handled better,
		       I'll write the code for that if this
		       happens too often. */
		    move.delta.x = 0;
		    move.delta.y = 0;
		    obj->vel.x = 0;
		    obj->vel.y = 0;
		}
	    }
	    else if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0)
		if (!Bounce_object(obj, &move, ans.line, ans.point))
		    break;
	}
    }
    Object_position_set_clicks(obj, move.start.x, move.start.y);
    return;
}


void Move_object(int ind)
{
    int t;
    struct move move;
    struct collans ans;
    int trycount = 5000;
    int team;            /* !@# should make TEAM_NOT_SET 0 */
    object *obj = Obj[ind];

    Object_position_remember(obj);

    obj->collmode = 1;

#if 1
    if (obj->type == OBJ_BALL) {
	Move_ball(ind);
	return;
    }
#else
    if (obj->type == OBJ_BALL) {
	if (obj->owner != -1)
	    team =  Players[GetInd[obj->owner]].team;
	else
	    team = TEAM_NOT_SET;
	move.hit_mask = BALL_BIT;
    }
    else
#endif
	{
	    move.hit_mask = NONBALL_BIT;
	    team = obj->team;
	}
    if (team == TEAM_NOT_SET)
	move.hit_mask |= NOTEAM_BIT;
    else
	move.hit_mask |= 1 << team;
    obj->extpos.x = WRAP_XCLICK(obj->pos.cx + FLOAT_TO_CLICK(obj->vel.x));
    obj->extpos.y = WRAP_YCLICK(obj->pos.cy + FLOAT_TO_CLICK(obj->vel.y));

    move.start.x = obj->pos.cx;
    move.start.y = obj->pos.cy;
    move.delta.x = FLOAT_TO_CLICK(obj->vel.x);
    move.delta.y = FLOAT_TO_CLICK(obj->vel.y);
    while (move.delta.x || move.delta.y) {
	if (!trycount--) {
	    sprintf(msg, "COULDN'T MOVE OBJECT!!!! Type = %d, x = %d, y = %d. Object was DELETED. [*DEBUG*]", obj->type, move.start.x, move.start.y);
	    Set_message(msg);
	    obj->life = 0;
	    return;
	}
	Move_point(&move, &ans);
	move.delta.x -= ans.moved.x;
	move.delta.y -= ans.moved.y;
	move.start.x = WRAP_XCLICK(move.start.x + ans.moved.x);
	move.start.y = WRAP_YCLICK(move.start.y + ans.moved.y);
	if (ans.line != -1) {
	    if ( (t = Away(&move, ans.line)) != -1) {
		if (!Clear_corner(&move, obj, ans.line, t))
		    break;
	    }
	    else if (SIDE(obj->vel.x, obj->vel.y, ans.line) < 0) {
		if (!Bounce_object(obj, &move, ans.line, 0))
		    break;
	    }
	}
    }
    Object_position_set_clicks(obj, move.start.x, move.start.y);
    return;
}


void Move_player(int ind)
{
    player *pl = Players[ind];
    clpos  pos;
    int    line, point;
    struct move move;
    struct collans ans;


    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	if (!BIT(pl->status, KILLED|PAUSE)) {
	    pos.x = pl->pos.cx + FLOAT_TO_CLICK(pl->vel.x);
	    pos.y = pl->pos.cy + FLOAT_TO_CLICK(pl->vel.y);
	    pos.x = WRAP_XCLICK(pos.x);
	    pos.y = WRAP_YCLICK(pos.y);
	    if (pos.x != pl->pos.cx || pos.y != pl->pos.cy) {
		Player_position_remember(pl);
		Player_position_set_clicks(pl, pos.x, pos.y);
	    }
	}
	pl->velocity = VECTOR_LENGTH(pl->vel);
	return;
    }

    pl->vel.x *= (1.0f - friction);
    pl->vel.y *= (1.0f - friction);

    Player_position_remember(pl);

    pl->collmode = 1;

    pl->extpos.x = pl->pos.cx + FLOAT_TO_CLICK(pl->vel.x);
    while (pl->extpos.x >= World.cwidth)
	pl->extpos.x -= World.cwidth;
    while (pl->extpos.x <= 0)
	pl->extpos.x += World.cwidth;
    pl->extpos.y = pl->pos.cy + FLOAT_TO_CLICK(pl->vel.y);
    while (pl->extpos.y >= World.cheight)
	pl->extpos.y -= World.cheight;
    while (pl->extpos.y <0)
	pl->extpos.y += World.cheight;

    if (BIT(pl->used, OBJ_PHASING_DEVICE))
	Player_position_set_clicks(pl, pl->extpos.x, pl->extpos.y);
    else {
	if (pl->team != TEAM_NOT_SET)
	    move.hit_mask = NONBALL_BIT | 1 << pl->team;
	else
	    move.hit_mask = NONBALL_BIT | NOTEAM_BIT;
	move.start.x = pl->pos.cx;
	move.start.y = pl->pos.cy;
	move.delta.x = FLOAT_TO_CLICK(pl->vel.x);
	move.delta.y = FLOAT_TO_CLICK(pl->vel.y);
	while (move.delta.x || move.delta.y) {
	    Shape_move(&move, pl->ship, pl->dir, &ans);
	    move.start.x = WRAP_XCLICK(move.start.x + ans.moved.x);
	    move.start.y = WRAP_YCLICK(move.start.y + ans.moved.y);
	    move.delta.x -= ans.moved.x;
	    move.delta.y -= ans.moved.y;
	    if (ans.line != -1) {
		if (!Shape_away(&move, pl->ship, pl->dir, ans.line, &line, &point)) {
		    if (SIDE(pl->vel.x, pl->vel.y, ans.line) < 0)
			Bounce_player(pl, &move, ans.line, ans.point);
		    else if (SIDE(pl->vel.x, pl->vel.y, line) < 0)
			Bounce_player(pl, &move, line, point);
		    else {
			/* This case could be handled better,
			   I'll write the code for that if this
			   happens too often. */
			move.delta.x = 0;
			move.delta.y = 0;
			pl->vel.x = 0;
			pl->vel.y = 0;
		    }
		}
		else if (SIDE(pl->vel.x, pl->vel.y, ans.line) < 0)
		    Bounce_player(pl, &move, ans.line, ans.point);
	    }
	}
	Player_position_set_clicks(pl, move.start.x, move.start.y);
    }
    pl->velocity = VECTOR_LENGTH(pl->vel);
    return;
}

void Turn_player(int ind)
{
    player	*pl = Players[ind];
    int		new_dir = MOD2((int)(pl->float_dir + 0.5f), RES);
    int		sign, hitmask;

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


    while (pl->dir != new_dir && (Shape_turn1(pl->ship, hitmask, pl->pos.cx, pl->pos.cy, pl->dir, sign) || (pl->float_dir = pl->dir, 0)))
	pl->dir = MOD2(pl->dir + sign, RES);
    return;
}

/*
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

#include "xpserver.h"

char shot_version[] = VERSION;

#define MISSILE_POWER_SPEED_FACT	0.25
#define MISSILE_POWER_TURNSPEED_FACT	0.75
#define MINI_TORPEDO_SPREAD_TIME	6
#define MINI_TORPEDO_SPREAD_SPEED	20
#define MINI_TORPEDO_SPREAD_ANGLE	90
#define MINI_MINE_SPREAD_TIME		18
#define MINI_MINE_SPREAD_SPEED		8
#define MINI_MISSILE_SPREAD_ANGLE	45

#define CONFUSED_UPDATE_GRANULARITY	10


/***********************
 * Functions for shots.
 */


void Place_mine(player *pl)
{
    vector	zero_vel = { 0.0, 0.0 };

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE) && !shieldedMining))
	return;

    if (minMineSpeed > 0) {
	Place_moving_mine(pl);
	return;
    }

    Place_general_mine(pl, pl->team, 0, pl->pos, zero_vel, pl->mods);
}


void Place_moving_mine(player *pl)
{
    vector	vel = pl->vel;

    if (pl->item[ITEM_MINE] <= 0
	|| (BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE) && !shieldedMining))
	return;

    if (minMineSpeed > 0) {
	if (pl->velocity < minMineSpeed) {
	    if (pl->velocity >= 1) {
		vel.x *= (minMineSpeed / pl->velocity);
		vel.y *= (minMineSpeed / pl->velocity);
	    } else {
		vel.x = minMineSpeed * tcos(pl->dir);
		vel.y = minMineSpeed * tsin(pl->dir);
	    }
	}
    }

    Place_general_mine(pl, pl->team, GRAVITY, pl->pos, vel, pl->mods);
}

void Place_general_mine(player *pl, int team, long status,
			clpos pos, vector vel, modifiers mods)
{
    char		msg[MSG_LEN];
    int			used;
    double		life, drain, mass;
    int			i, minis;
    vector		mv;

    if (NumObjs + mods.mini >= MAX_TOTAL_SHOTS)
	return;

    pos.cx = WRAP_XCLICK(pos.cx);
    pos.cy = WRAP_YCLICK(pos.cy);

    if (pl && BIT(pl->status, KILLED))
	life = rfrac() * 12;
    else if (BIT(status, FROMCANNON))
	life = CANNON_SHOT_LIFE;
    else
	life = (mineLife ? mineLife : (int)MINE_LIFETIME);

    if (!BIT(mods.warhead, CLUSTER))
	mods.velocity = 0;
    if (!mods.mini)
	mods.spread = 0;

    if (nukeMinSmarts <= 0)
	CLR_BIT(mods.nuclear, NUCLEAR);
    if (BIT(mods.nuclear, NUCLEAR)) {
	if (pl) {
	    used = (BIT(mods.nuclear, FULLNUCLEAR)
		    ? pl->item[ITEM_MINE]
		    : nukeMinMines);
	    if (pl->item[ITEM_MINE] < nukeMinMines) {
		sprintf(msg, "You need at least %d mines to %s %s!",
			nukeMinMines,
			(BIT(status, GRAVITY) ? "throw" : "drop"),
			Describe_shot (OBJ_MINE, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	} else
	    used = nukeMinMines;
	mass = MINE_MASS * used * NUKE_MASS_MULT;
    } else {
	mass = (BIT(status, FROMCANNON) ? MINE_MASS * 0.6 : MINE_MASS);
	used = 1;
    }

    if (pl) {
	drain = ED_MINE;
	if (BIT(mods.warhead, CLUSTER))
	    drain += CLUSTER_MASS_DRAIN(mass);
	if (pl->fuel.sum < -drain) {
	    sprintf(msg, "You need at least %.1f fuel units to %s %s!",
		    -drain, (BIT(status, GRAVITY) ? "throw" : "drop"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_player_message (pl, msg);
	    return;
	}
	if (baseMineRange) {
	    for (i = 0; i < NumPlayers; i++) {
		player *pl_i = Players(i);
		if (pl_i->home_base == NULL)
		    continue;
		if (pl_i->id != pl->id
		    && !Team_immune(pl_i->id, pl->id)
		    && !IS_TANK_PTR(pl_i)) {
		    int dx = pos.cx - pl_i->home_base->pos.cx;
		    int dy = pos.cy - pl_i->home_base->pos.cy;
		    if (Wrap_length(dx, dy) <= baseMineRange * BLOCK_CLICKS) {
			Set_player_message(pl, "No base mining!");
			return;
		    }
		}
	    }
	}
	Player_add_fuel(pl, drain);
	pl->item[ITEM_MINE] -= used;

	if (used > 1) {
	    sprintf(msg, "%s has %s %s!", pl->name,
		    (BIT(status, GRAVITY) ? "thrown" : "dropped"),
		    Describe_shot(OBJ_MINE, status, mods, 0));
	    Set_message(msg);
	    sound_play_all(NUKE_LAUNCH_SOUND);
	} else
	    sound_play_sensors(pl->pos,
			       BIT(status, GRAVITY)
			       ? DROP_MOVING_MINE_SOUND : DROP_MINE_SOUND);
    }

    minis = (mods.mini + 1);
    SET_BIT(status, OWNERIMMUNE);

    for (i = 0; i < minis; i++) {
	mineobject *mine;

	if ((mine = MINE_PTR(Object_allocate())) == NULL)
	    break;

	mine->type = OBJ_MINE;
	mine->color = BLUE;
	mine->fusetime = mineFuseTime;
	mine->status = status;
	mine->id = (pl ? pl->id : NO_ID);
	mine->team = team;
	mine->owner = mine->id;
	Object_position_init_clicks(OBJ_PTR(mine), pos.cx, pos.cy);
	if (minis > 1) {
	    int		space = RES/minis;
	    int		dir;
	    double	spread;

	    spread = (double)((unsigned)mods.spread + 1);
	    /*
	     * Dir gives (S is ship upwards);
	     *
	     *			      o		    o   o
	     *	X2: o S	o	X3:   S		X4:   S
	     *			    o   o	    o   o
	     */
	    dir = (i * space) + space/2 + (minis-2)*(RES/2) + (pl?pl->dir:0);
	    dir += (int)((rfrac() - 0.5f) * space * 0.5f);
	    dir = MOD2(dir, RES);
	    mv.x = MINI_MINE_SPREAD_SPEED * tcos(dir) / spread;
	    mv.y = MINI_MINE_SPREAD_SPEED * tsin(dir) / spread;
	    /*
	     * This causes the added initial velocity to reduce to
	     * zero over the MINI_MINE_SPREAD_TIME.
	     */
	    mine->spread_left = MINI_MINE_SPREAD_TIME;
	    mine->acc.x = -mv.x / MINI_MINE_SPREAD_TIME;
	    mine->acc.y = -mv.y / MINI_MINE_SPREAD_TIME;
	} else {
	    mv.x = mv.y = mine->acc.x = mine->acc.y = 0.0;
	    mine->spread_left = 0;
	}
	mine->vel = mv;
	mine->vel.x += vel.x * MINE_SPEED_FACT;
	mine->vel.y += vel.y * MINE_SPEED_FACT;
	mine->mass = mass / minis;
	mine->life = life / minis;
	mine->mods = mods;
	mine->pl_range = (int)(MINE_RANGE / minis);
	mine->pl_radius = MINE_RADIUS;
	Cell_add_object((object *) mine);
    }
}

/*
 * Up to and including 3.2.6 it was:
 *     Cause all of the given player's dropped/thrown mines to explode.
 * Since this caused a slowdown when many mines detonated it
 * is changed into:
 *     Cause the mine which is closest to a player and owned
 *     by that player to detonate.
 */
void Detonate_mines(player *pl)
{
    int			i;
    int			closest = -1;
    double		dist;
    double		min_dist = World.hypotenuse * CLICK + 1;

    if (BIT(pl->used, HAS_PHASING_DEVICE))
	return;

    for (i = 0; i < NumObjs; i++) {
	object *mine = Obj[i];

	if (! BIT(mine->type, OBJ_MINE))
	    continue;
	/*
	 * Mines which have been ECM reprogrammed should only be detonatable
	 * by the reprogrammer, not by the original mine placer:
	 */
	if (mine->id == pl->id) {
	    dist = Wrap_length(pl->pos.cx - mine->pos.cx,
			       pl->pos.cy - mine->pos.cy);
	    if (dist < min_dist) {
		min_dist = dist;
		closest = i;
	    }
	}
    }
    if (closest != -1)
	Obj[closest]->life = 0;

    return;
}

void Make_treasure_ball(treasure_t *t)
{
    ballobject *ball;
    int cx = t->pos.cx;
    int cy = t->pos.cy;

    if (!is_polygon_map)
	cy += (10 * PIXEL_CLICKS - BLOCK_CLICKS / 2);

    if (t->empty)
	return;
    if (t->have) {
	xpprintf("%s Failed Make_treasure_ball(treasure=%ld):\n",
		 showtime(), (long)t);
	xpprintf("\ttreasure: destroyed = %d, team = %d, have = %d\n",
		 t->destroyed, t->team, t->have);
	return;
    }

    if ((ball = BALL_PTR(Object_allocate())) == NULL)
	return;

    ball->life = 1e6;
    ball->mass = ballMass;
    ball->vel.x = 0;	  	/* make the ball stuck a little */
    ball->vel.y = 0;		/* longer to the ground */
    ball->acc.x = 0;
    ball->acc.y = 0;
    Object_position_init_clicks(OBJ_PTR(ball), cx, cy);
    ball->id = NO_ID;
    ball->owner = NO_ID;
    ball->team = t->team;
    ball->type = OBJ_BALL;
    ball->color = WHITE;
    ball->count = 0;
    ball->pl_range = BALL_RADIUS;
    ball->pl_radius = BALL_RADIUS;
    CLEAR_MODS(ball->mods);
    ball->status = RECREATE;
    ball->treasure = t;
    Cell_add_object(OBJ_PTR(ball));

    t->have = true;
}


/*
 * Describes shot of `type' which has `status' and `mods'.  If `hit' is
 * non-zero this description is part of a collision, otherwise its part
 * of a launch message.
 */
char *Describe_shot(int type, long status, modifiers mods, int hit)
{
    const char		*name, *howmany = "a ", *plural = "";
    static char		msg[MSG_LEN];

    switch (type) {
    case OBJ_MINE:
	if (BIT(status, GRAVITY))
	    name = "bomb";
	else
	    name = "mine";
	break;
    case OBJ_SMART_SHOT:
	name = "smart missile";
	break;
    case OBJ_TORPEDO:
	name = "torpedo";
	break;
    case OBJ_HEAT_SHOT:
	name = "heatseeker";
	break;
    case OBJ_CANNON_SHOT:
	if (BIT(mods.warhead, CLUSTER)) {
	    howmany = "";
	    name = "flak";
	} else
	    name = "shot";
	break;
    default:
	/*
	 * Cluster shots are actual debris from a cluster explosion
	 * so we describe it as "cluster debris".
	 */
	if (BIT(mods.warhead, CLUSTER)) {
	    howmany = "";
	    name = "debris";
	} else
	    name = "shot";
	break;
    }

    if (mods.mini && !hit) {
	howmany = "some ";
	plural = (type == OBJ_TORPEDO) ? "es" : "s";
    }

    sprintf (msg, "%s%s%s%s%s%s%s%s%s",
	     howmany,
	     ((mods.velocity || mods.spread || mods.power) ? "modified " : ""),
	     (mods.mini ? "mini " : ""),
	     (BIT(mods.nuclear, FULLNUCLEAR) ? "full " : ""),
	     (BIT(mods.nuclear, NUCLEAR) ? "nuclear " : ""),
	     (BIT(mods.warhead, IMPLOSION) ? "imploding " : ""),
	     (BIT(mods.warhead, CLUSTER) ? "cluster " : ""),
	     name,
	     plural);

    return msg;
}

void Fire_main_shot(player *pl, int type, int dir)
{
    clpos m_gun, pos;

    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    m_gun = Ship_get_m_gun_clpos(pl->ship, pl->dir);
    pos.cx = pl->pos.cx + m_gun.cx;
    pos.cy = pl->pos.cy + m_gun.cy;

    Fire_general_shot(pl, pl->team, 0, pos, type, dir, pl->mods, NO_ID);
}

void Fire_shot(player *pl, int type, int dir)
{
    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    Fire_general_shot(pl, pl->team, 0, pl->pos, type, dir, pl->mods, NO_ID);
}

void Fire_left_shot(player *pl, int type, int dir, int gun)
{
    clpos l_gun, pos;

    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    l_gun = Ship_get_l_gun_clpos(pl->ship, gun, pl->dir);
    pos.cx = pl->pos.cx + l_gun.cx;
    pos.cy = pl->pos.cy + l_gun.cy;

    Fire_general_shot(pl, pl->team, 0, pos, type, dir, pl->mods, NO_ID);
}

void Fire_right_shot(player *pl, int type, int dir, int gun)
{
    clpos r_gun, pos;

    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    r_gun = Ship_get_r_gun_clpos(pl->ship, gun, pl->dir);
    pos.cx = pl->pos.cx + r_gun.cx;
    pos.cy = pl->pos.cy + r_gun.cy;

    Fire_general_shot(pl, pl->team, 0, pos, type, dir, pl->mods, NO_ID);
}

void Fire_left_rshot(player *pl, int type, int dir, int gun)
{
    clpos l_rgun, pos;

    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    l_rgun = Ship_get_l_rgun_clpos(pl->ship, gun, pl->dir);
    pos.cx = pl->pos.cx + l_rgun.cx;
    pos.cy = pl->pos.cy + l_rgun.cy;

    Fire_general_shot(pl, pl->team, 0, pos, type, dir, pl->mods, NO_ID);
}

void Fire_right_rshot(player *pl, int type, int dir, int gun)
{
    clpos r_rgun, pos;

    if (pl->shots >= ShotsMax || BIT(pl->used, HAS_SHIELD|HAS_PHASING_DEVICE))
	return;

    r_rgun = Ship_get_r_rgun_clpos(pl->ship, gun, pl->dir);
    pos.cx = pl->pos.cx + r_rgun.cx;
    pos.cy = pl->pos.cy + r_rgun.cy;

    Fire_general_shot(pl, pl->team, 0, pos, type, dir, pl->mods, NO_ID);
}

void Fire_general_shot(player *pl, int team, bool cannon,
		       clpos pos, int type, int dir,
		       modifiers mods, int target_id)
{
    char		msg[MSG_LEN];
    int			used,
			fuse = 0,
			lock = 0,
			status = GRAVITY,
			i, ldir, minis,
			pl_range,
			pl_radius,
			rack_no = 0,
			racks_left = 0,
			r,
			on_this_rack = 0,
			side = 0,
			fired = 0;
    double		drain,
    			mass = ShotsMass,
			life = ShotsLife,
			speed = ShotsSpeed,
			turnspeed = 0,
			max_speed = SPEED_LIMIT,
			angle,
			spread;
    vector		mv;
    clpos		shotpos;
    object		*mini_objs[MODS_MINI_MAX + 1];

    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    if (!BIT(mods.warhead, CLUSTER))
	mods.velocity = 0;
    if (!mods.mini)
	mods.spread = 0;

    if (cannon) {
	mass = CANNON_SHOT_MASS;
	life = CANNON_SHOT_LIFE;
	SET_BIT(status, FROMCANNON);
    }


    switch (type) {
    default:
	return;

    case OBJ_SHOT:
	CLEAR_MODS(mods);	/* Shots can't be modified! */
	/* FALLTHROUGH */
    case OBJ_CANNON_SHOT:
	pl_range = pl_radius = 0;
	if (pl) {
	    if (pl->fuel.sum < -ED_SHOT)
		return;
	    Player_add_fuel(pl, ED_SHOT);
	    sound_play_sensors(pl->pos, FIRE_SHOT_SOUND);
	    Rank_FireShot(pl);
	}
	if (!ShotsGravity)
	    CLR_BIT(status, GRAVITY);
	break;

    case OBJ_SMART_SHOT:
    case OBJ_HEAT_SHOT:
	if (type == OBJ_HEAT_SHOT ? !allowHeatSeekers : !allowSmartMissiles) {
	    if (allowTorpedoes)
		type = OBJ_TORPEDO;
	    else
		return;
	}
	/* FALLTHROUGH */
    case OBJ_TORPEDO:
	/*
	 * Make sure there are enough object entries for the mini shots.
	 */
	if (NumObjs + mods.mini >= MAX_TOTAL_SHOTS)
	    return;

	if (pl && pl->item[ITEM_MISSILE] <= 0)
	    return;

	if (nukeMinSmarts <= 0)
	    CLR_BIT(mods.nuclear, NUCLEAR);
	if (BIT(mods.nuclear, NUCLEAR)) {
	    if (pl) {
		used = (BIT(mods.nuclear, FULLNUCLEAR)
			? pl->item[ITEM_MISSILE]
			: nukeMinSmarts);
		if (pl->item[ITEM_MISSILE] < nukeMinSmarts) {
		    sprintf(msg,
			    "You need at least %d missiles to fire %s!",
			    nukeMinSmarts,
			    Describe_shot (type, status, mods, 0));
		    Set_player_message (pl, msg);
		    return;
		}
	    } else
		used = nukeMinSmarts;
	    mass = MISSILE_MASS * used * NUKE_MASS_MULT;
	    pl_range = (type == OBJ_TORPEDO) ? (int)NUKE_RANGE : MISSILE_RANGE;
	} else {
	    mass = MISSILE_MASS;
	    used = 1;
	    pl_range = (type == OBJ_TORPEDO) ? (int)TORPEDO_RANGE : MISSILE_RANGE;
	}
	pl_range /= mods.mini + 1;
	pl_radius = MISSILE_LEN;

	drain = used * ED_SMART_SHOT;
	if (BIT(mods.warhead, CLUSTER)) {
	    if (pl)
		drain += CLUSTER_MASS_DRAIN(mass);
	}

	if (pl && BIT(pl->status, KILLED))
	    life = rfrac() * 12;
	else if (!cannon)
	    life = (missileLife ? missileLife : MISSILE_LIFETIME);

	switch (type) {
	case OBJ_HEAT_SHOT:
#ifndef HEAT_LOCK
	    lock = NO_ID;
#else  /* HEAT_LOCK */
	    if (pl == NULL)
		lock = target_id;
	    else {
		if (!BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY))) {
		    lock = NO_ID;
		} else
		    lock = pl->lock.pl_id;
	    }
#endif /* HEAT_LOCK */
	    if (pl)
		sound_play_sensors(pl->pos, FIRE_HEAT_SHOT_SOUND);
	    max_speed = SMART_SHOT_MAX_SPEED * HEAT_SPEED_FACT;
	    turnspeed = SMART_TURNSPEED * HEAT_SPEED_FACT;
	    speed *= HEAT_SPEED_FACT;
	    break;

	case OBJ_SMART_SHOT:
	    if (pl == NULL)
		lock = target_id;
	    else {
		if (!BIT(pl->lock.tagged, LOCK_PLAYER)
		|| ((pl->lock.distance > pl->sensor_range)
		    && BIT(World.rules->mode, LIMITED_VISIBILITY))
		|| !pl->visibility[GetInd(pl->lock.pl_id)].canSee)
		    return;
		lock = pl->lock.pl_id;
	    }
	    max_speed = SMART_SHOT_MAX_SPEED;
	    turnspeed = SMART_TURNSPEED;
	    break;

	case OBJ_TORPEDO:
	    lock = NO_ID;
	    fuse = 8;
	    break;

	default:
	    break;
	}

	if (pl) {
	    if (pl->fuel.sum < -drain) {
		sprintf(msg, "You need at least %.1f fuel units to fire %s!",
			-drain, Describe_shot(type, status, mods, 0));
		Set_player_message (pl, msg);
		return;
	    }
	    Player_add_fuel(pl, drain);
	    pl->item[ITEM_MISSILE] -= used;

	    if (used > 1) {
		sprintf(msg, "%s has launched %s!", pl->name,
			Describe_shot(type, status, mods, 0));
		Set_message(msg);
		sound_play_all(NUKE_LAUNCH_SOUND);
	    } else if (type == OBJ_SMART_SHOT)
		sound_play_sensors(pl->pos, FIRE_SMART_SHOT_SOUND);
	    else if (type == OBJ_TORPEDO)
		sound_play_sensors(pl->pos, FIRE_TORPEDO_SOUND);
	}
	break;
    }

    minis = (mods.mini + 1);
    speed *= (1 + (mods.power * MISSILE_POWER_SPEED_FACT));
    max_speed *= (1 + (mods.power * MISSILE_POWER_SPEED_FACT));
    turnspeed *= (1 + (mods.power * MISSILE_POWER_TURNSPEED_FACT));
    spread = (double)((unsigned)mods.spread + 1);
    /*
     * Calculate the maximum time it would take to cross one ships width,
     * don't fuse the shot/missile/torpedo for the owner only until that
     * time passes.  This is a hack to stop various odd missile and shot
     * mounting points killing the player when they're firing.
     */
    fuse += (int)((2.0 * (double)SHIP_SZ) / speed + 1.0);

    /*
     * 			Missile Racks and Spread
     * 			------------------------
     *
     * 		    A short story by H. J. Thompson
     *
     * Once upon a time, back in the "good old days" of XPilot, it was
     * relatively easy thing to remember the few keys needed to fly and shoot.
     * It was the day of Sopwith Camels biplanes, albeit triangular ones,
     * doing close to-the-death machine gun combat with other triangular
     * Red Barons, the hard vacuum of space whistling silently by as only
     * something that doesn't exist could do (this was later augmented by
     * artificial aural feedback devices on certain advanced hardware).
     *
     * Eventually the weapon designers came up with "smart" missiles, and
     * another key was added to the control board, causing one missile to
     * launch straight forwards from the front of the triangular ship.
     * Soon other types of missiles were added, including "heat" seekers,
     * and fast straight travelling "torpedoes" (hark, is that the sonorous
     * ping-ping-ping of sonar equipment I hear?).
     *
     * Then one day along came a certain fellow who thought, among other
     * things, that it would be neat to fire up to four missiles with one
     * key press, just so the enemy pilot would be scared witless by the
     * sudden appearance of four missiles hot on their tail.  To make things
     * fair these "mini" missiles would have the same total damage of a
     * normal missile, but would travel at the speed of a normal missile.
     *
     * However this fellow mused that simply launching all the missiles in
     * the same direction and from the same point would cause the missiles
     * to appear on top of each other.  Thus he added code to "spread" the
     * missiles out at various angular offsets from the ship.  Indeed the
     * angular offsets could be controlled using a spread modifier, and yet
     * more keys appeared on a now crowded control desk.
     *
     * Interestingly the future would see the same fellow adding a two seater
     * variant of the standard single seater ship, allowing one person
     * to concentrate on flying the ship, while another could flick through
     * out-of-date manuals searching for the right key combinations on
     * the now huge console which would launch four full nuclear slow-cluster
     * imploding mini super speed close spread torpedoes at the currently
     * targetted enemy, and then engage emergency thrust and shields before
     * the ominous looking tri-winged dagger ship recoiled at high velocity
     * into a rocky wall half way across the other side of the universe.
     *
     * Back to our story, and this same fellow was musing at the design of
     * multiple "mini" missiles, and noted that the angle of launch would
     * also require a different launch point on the ship (actually it was
     * the same position as if the front of the ship was rotated to point in
     * the direction of missile launch, mainly because it was easier to
     * write the launch/guidance computer software that way).
     *
     * Later, some artistically (or sadistically) minded person decided that
     * triangular ships just didn't look good (even though they were very
     * spatially dynamic, cheap and easy to build), and wouldn't it be just
     * fantastic if one could have a ship shaped like a banana!  Sensibly,
     * however, he restricted missiles and guns to the normal single frontal
     * launching point.
     *
     * A few weeks later, somebody else decided how visually pleasing it
     * would be if one could design where missiles could be fired from by
     * adding "missile rack" points on the ship.  Up to four racks were
     * available, and missiles would fire from exactly these points on the
     * ship.  Since one to four missiles could be fired in one go, the
     * combinations with various ship designs were numerous (16).
     *
     * What would happen if somebody fired four missiles in one go, from a
     * ship that only had three missile racks?  How about two missiles from
     * one with four racks?  Sadly the missile launch software hadn't been
     * designed to take this sort of thing into account, and incredibly the
     * original programmer wasn't notified until after First Customer Ship
     * [sic], the launch software only slightly modified by the ship
     * designer, who didn't know the first thing about launch acceleration
     * curves or electronic owner immunity fuse timers.
     *
     * Pilots found their missiles were being fired from random points and
     * in sometimes very odd directions, occasionally even destroying the
     * ship without trace, severely annoying the ship's owners and several
     * insurance underwriters.  Not soon after several ship designers were
     * mysteriously killed in a freak "accident" involving a stray nuclear
     * cluster bomb, and the remaining ship designers became very careful
     * to place missile racks and extra gun turrets well away from the
     * ship's superstructure.
     *
     * The original programmer who invented multiple "mini" spreading
     * missiles quickly decided to revisit his code before any "accidents"
     * came his way, and spent a good few hours making sure one couldn't
     * shoot oneself in the "foot", and that missiles where launched in some
     * reasonable and sensible directions based on the position of the
     * missile racks.
     *
     * 			How It Actually Works
     *			---------------------
     *
     * The first obstacle is getting the right number of missiles fired
     * from each combination of missile rack configurations;
     *
     *
     *		Minis	1	2	3	4
     * Racks
     *	1		1	2	3	4
     *
     *	2		1/-	1/1	2/1	2/2
     *			-/1		1/2
     *
     *	3		1/-/-	1/1/-	1/1/1	2/1/1
     *			-/1/-	-/1/1		1/2/1
     *			-/-/1	1/-/1		1/1/2
     *
     *	4		1/-/-/-	1/1/-/-	1/1/1/-	1/1/1/1
     *			-/1/-/-	-/1/1/-	-/1/1/1
     *			-/-/1/-	-/-/1/1	1/-/1/1
     *			-/-/-/1 1/-/-/1	1/1/-/1
     *
     * To read; For example with 2 Minis and 3 Racks, the first round will
     * fire 1/1/-, which is one missile from left and middle racks.  The
     * next time fired will be -/1/1; middle and right, next fire is
     * 1/-/1; left and right.  Next time goes to the beggining state.
     *
     * 			Comment Point 1
     *			---------------
     *
     * The *starting* rack number for each salvo cycles through the number
     * of missiles racks.  This is stored in the player variable
     * `pl->missile_rack', and is only incremented after each salvo (not
     * after each mini missile is fired).  This value is used to initialise
     * `rack_no', which stores the current rack that missiles are fired from.
     *
     * `on_this_rack' is computed to be the number of missiles that will be
     * fired from `rack_no', and `r' is used as a counter to this value.
     *
     * `racks_left' count how many unused missiles racks are left on the ship
     * in this mini missile salvo.
     *
     * 			Comment Point 2
     *			---------------
     *
     * When `r' reaches `on_this_rack' all the missiles have been fired for
     * this rack, and the next rack should be used.  `rack_no' is incremented
     * modulo the number of available racks, and `racks_left' is decremented.
     * At this point `on_this_rack' is recomputed for the next rack, and `r'
     * reset to zero.  Thus initially these two variables are both zero, and
     * `rack_no' is one less, such that these variables can be computed inside
     * the loop to make the code simpler.
     *
     * The computation of `on_this_rack' is as follows;  Given that there
     * are M missiles and R racks remaining;
     *
     *	on_this_rack = int(M / R);	(ie. round down to lowest int)
     *
     * Then;
     *
     *	(M - on_this_rack) / (R - 1) < (M / R).
     *
     * That is, the number of missiles fired on the next rack will be
     * more precise, and trivially can be seen that when R is 1, will
     * give an exact number of missiles to fire on the last rack.
     *
     * In the code `M' is (minis - i), and `R' is racks_left.
     *
     *			Comment Point 3
     *			---------------
     *
     * In order that multiple missiles fired from one rack do not conincide,
     * each missile has to be "spread" based on the number of missiles
     * fired from this rack point.
     *
     * This is computed similar to the wide shot code;
     *
     *	angle = (N - 1 - 2 * i) / (N - 1)
     *
     * Where N is the number of shots/missiles to be fired, and i is a counter
     * from 0 .. N-1.
     *
     * 		i	0	1	2	3
     * N
     * 1		0
     * 2		1	-1
     * 3		1	0	-1
     * 4		1	0.333	-0.333	-1
     *
     * In this code `N' is `on_this_rack'.
     *
     * Also the position of the missile rack from the center line of the
     * ship (stored in `side') has a linear effect on the angle, such that
     * a point farthest from the center line contributes the largest angle;
     *
     * angle += (side / SHIP_SZ)
     *
     * Since the eventual `angle' value used in the code should be a
     * percentage of the unmodified launch angle, it should be ranged between
     * -1.00 and +1.00, and thus the first angle is reduced by 33% and the
     * second by 66%.
     *
     * Contact: harveyt@sco.com
     */

    if (pl && type != OBJ_SHOT) {
	/*
	 * Initialise missile rack spread variables. (See Comment Point 1)
	 */
	on_this_rack = 0;
	racks_left = pl->ship->num_m_rack;
	rack_no = pl->missile_rack - 1;
	if (++pl->missile_rack >= pl->ship->num_m_rack)
	    pl->missile_rack = 0;
    }

    for (r = 0, i = 0; i < minis; i++, r++) {
	object *shot;

	if ((shot = Object_allocate()) == NULL)
	    break;

	shot->life 	= life / minis;
	shot->fusetime 	= frame_time + fuse;
	shot->mass	= mass / minis;
	shot->count 	= 0;
	shot->info 	= lock;
	shot->type	= type;
	shot->id	= (pl ? pl->id : NO_ID);
	shot->team	= team;
	shot->color	= (pl ? pl->color : WHITE);

	if (BIT(shot->type, OBJ_TORPEDO | OBJ_HEAT_SHOT | OBJ_SMART_SHOT)) {
	    MISSILE_PTR(shot)->turnspeed = turnspeed;
	    MISSILE_PTR(shot)->max_speed = max_speed;
	}

	shotpos = pos;
	if (pl && type != OBJ_SHOT) {
	    clpos m_rack;
	    if (r == on_this_rack) {
		/*
		 * We've fired all the mini missiles for the current rack,
		 * we now move onto the next one. (See Comment Point 2)
		 */
		on_this_rack = (minis - i) / racks_left--;
		if (on_this_rack < 1) on_this_rack = 1;
		if (++rack_no >= pl->ship->num_m_rack)
		    rack_no = 0;
		r = 0;
	    }
	    m_rack = Ship_get_m_rack_clpos(pl->ship, rack_no, pl->dir);
	    shotpos.cx += m_rack.cx;
	    shotpos.cy += m_rack.cy;
	    /*side = CLICK_TO_PIXEL(pl->ship->m_rack[rack_no][0].cy);*/
	    side = CLICK_TO_PIXEL(
		Ship_get_m_rack_clpos(pl->ship, rack_no, 0).cy);
	}
	shotpos.cx = WRAP_XCLICK(shotpos.cx);
	shotpos.cy = WRAP_YCLICK(shotpos.cy);
	Object_position_init_clicks(shot, shotpos.cx, shotpos.cy);

	if (type == OBJ_SHOT || !pl)
	    angle = 0.0;
	else {
	    /*
	     * Calculate the percentage unmodified launch angle for missiles.
	     * (See Comment Point 3).
	     */
	    if (on_this_rack <= 1)
		angle = 0.0;
	    else {
		angle = (double)(on_this_rack - 1 - 2 * r);
		angle /= (3.0 * (double)(on_this_rack - 1));
	    }
	    angle += (double)(2 * side) / (double)(3 * SHIP_SZ);
	}

	/*
	 * Torpedoes spread like mines, except the launch direction
	 * is preset over the range +/- MINI_TORPEDO_SPREAD_ANGLE.
	 * (This is not modified by the spread, the initial velocity is)
	 *
	 * Other missiles are just launched in a different direction
	 * which varies over the range +/- MINI_MISSILE_SPREAD_ANGLE,
	 * which the spread modifier varies.
	 */
	switch (type) {
	case OBJ_TORPEDO:
	    angle *= (MINI_TORPEDO_SPREAD_ANGLE / 360.0) * RES;
	    ldir = MOD2(dir + (int)angle, RES);
	    mv.x = MINI_TORPEDO_SPREAD_SPEED * tcos(ldir) / spread;
	    mv.y = MINI_TORPEDO_SPREAD_SPEED * tsin(ldir) / spread;
	    /*
	     * This causes the added initial velocity to reduce to
	     * zero over the MINI_TORPEDO_SPREAD_TIME.
	     * FIX: torpedoes should have the same speed
	     *      regardless of minification.
	     */
	    TORP_PTR(shot)->spread_left = MINI_TORPEDO_SPREAD_TIME;
	    shot->acc.x = -mv.x / MINI_TORPEDO_SPREAD_TIME;
	    shot->acc.y = -mv.y / MINI_TORPEDO_SPREAD_TIME;
	    ldir = dir;
	    break;

	default:
	    angle *= (MINI_MISSILE_SPREAD_ANGLE / 360.0) * RES / spread;
	    ldir = MOD2(dir + (int)angle, RES);
	    mv.x = mv.y = shot->acc.x = shot->acc.y = 0;
	    break;
	}

	/*
	 * If "NG controls" are activated, use float dir when shooting
	 * straight ahead.
	 */
	if (ngControls && pl && ldir == pl->dir) {
	    shot->vel.x = mv.x + pl->vel.x + pl->float_dir_cos * speed;
	    shot->vel.y = mv.y + pl->vel.y + pl->float_dir_sin * speed;
	} else {
	    shot->vel.x = mv.x + (pl ? pl->vel.x : 0.0) + tcos(ldir) * speed;
	    shot->vel.y = mv.y + (pl ? pl->vel.y : 0.0) + tsin(ldir) * speed;
	}
	shot->status	= status;
	shot->missile_dir	= ldir;
	shot->mods  	= mods;
	shot->pl_range  = pl_range;
	shot->pl_radius = pl_radius;
	Cell_add_object(shot);

	mini_objs[fired] = shot;
	fired++;
    }

    /*
     * Recoil must be done instantaneously otherwise ship moves back after
     * firing each mini missile.
     */
    if (pl) {
	double dx, dy;

	dx = dy = 0;
	for (i = 0; i < fired; i++) {
	    dx += (mini_objs[i]->vel.x - pl->vel.x) * mini_objs[i]->mass;
	    dy += (mini_objs[i]->vel.y - pl->vel.y) * mini_objs[i]->mass;
	}
	pl->vel.x -= dx / pl->mass;
	pl->vel.y -= dy / pl->mass;
    }
}


void Fire_normal_shots(player *pl)
{
    int			i, shot_angle;

    /* Average non-integer repeat rates, so that smaller gap occurs first.
     * 1e-3 "fudge factor" because "should be equal" cases return. */
    if (frame_time <= pl->shot_time + fireRepeatRate - timeStep + 1e-3)
 	return;
    pl->shot_time = MAX(frame_time, pl->shot_time + fireRepeatRate);

    shot_angle = MODS_SPREAD_MAX - pl->mods.spread;

    Fire_main_shot(pl, OBJ_SHOT, pl->dir);
    for (i = 0; i < pl->item[ITEM_WIDEANGLE]; i++) {
	if (pl->ship->num_l_gun > 0) {
	    Fire_left_shot(pl, OBJ_SHOT, MOD2(pl->dir + (1 + i) * shot_angle,
			   RES), i % pl->ship->num_l_gun);
	}
	else {
	    Fire_main_shot(pl, OBJ_SHOT, MOD2(pl->dir + (1 + i) * shot_angle,
			   RES));
	}
	if (pl->ship->num_r_gun > 0) {
	    Fire_right_shot(pl, OBJ_SHOT, MOD2(pl->dir - (1 + i) * shot_angle,
			    RES), i % pl->ship->num_r_gun);
	}
	else {
	    Fire_main_shot(pl, OBJ_SHOT, MOD2(pl->dir - (1 + i) * shot_angle,
			   RES));
	}
    }
    for (i = 0; i < pl->item[ITEM_REARSHOT]; i++) {
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) < 0) {
	    if (pl->ship->num_l_rgun > 0) {
		Fire_left_rshot(pl, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES), (i - (pl->item[ITEM_REARSHOT] + 1) / 2) % pl->ship->num_l_rgun);
	    }
	    else {
		Fire_shot(pl, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
	    }
	}
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) > 0) {
	    if (pl->ship->num_r_rgun > 0) {
		Fire_right_rshot(pl, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES), (pl->item[ITEM_REARSHOT] / 2 - i - 1) % pl->ship->num_r_rgun);
	    }
	    else {
		Fire_shot(pl, OBJ_SHOT, MOD2(pl->dir + RES/2
		    + ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
	    }
	}
	if ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) == 0)
	     Fire_shot(pl, OBJ_SHOT, MOD2(pl->dir + RES/2
		+ ((pl->item[ITEM_REARSHOT] - 1 - 2 * i) * shot_angle) / 2,
			RES));
    }
}


/* Removes shot from array */
void Delete_shot(int ind)
{
    object		*shot = Obj[ind];	/* Used when swapping places */
    ballobject		*ball;
    player		*pl;
    bool		addMine = false, addHeat = false, addBall = false;
    modifiers		mods;
    long		status;
    int			i;
    int			intensity, type, color;
    double		modv, speed_modv, life_modv, num_modv, mass;

    switch (shot->type) {

    case OBJ_SPARK:
    case OBJ_DEBRIS:
    case OBJ_WRECKAGE:
	break;

    case OBJ_ASTEROID:
	Break_asteroid(WIRE_PTR(shot));
	break;

    case OBJ_BALL:
	ball = BALL_PTR(shot);
	if (ball->id != NO_ID)
	    Detach_ball(Player_by_id(ball->id), ball);
	else {
	    /*
	     * Maybe some player is still busy trying to connect to this ball.
	     */
	    for (i = 0; i < NumPlayers; i++) {
		player *pl_i = Players(i);
		if (pl_i->ball == ball)
		    pl_i->ball = NULL;
	    }
	}
	if (ball->owner == NO_ID) {
	    /*
	     * If the ball has never been owned, the only way it could
	     * have been destroyed is by being knocked out of the goal.
	     * Therefore we force the ball to be recreated.
	     */
	    ball->treasure->have = false;
	    SET_BIT(ball->status, RECREATE);
	}
	if (BIT(ball->status, RECREATE)) {
	    addBall = true;
	    if (BIT(ball->status, NOEXPLOSION))
		break;
	    sound_play_sensors(ball->pos, EXPLODE_BALL_SOUND);

	    /* The ball could be inside a BallArea, check whether
	     * the sparks can exist here. Should we set a team? */
	    if (is_inside(ball->prevpos.cx, ball->prevpos.cy,
			  NONBALL_BIT | NOTEAM_BIT, OBJ_PTR(ball)) != NO_GROUP)
		break;

	    Make_debris(
		/* pos            */ ball->prevpos,
		/* vel            */ ball->vel,
		/* owner id       */ ball->id,
		/* owner team     */ ball->team,
		/* kind           */ OBJ_DEBRIS,
		/* mass           */ DEBRIS_MASS,
		/* status         */ GRAVITY,
		/* color          */ RED,
		/* radius         */ 8,
		/* num debris     */ (int)(10 + 10 * rfrac()),
		/* min,max dir    */ 0, RES-1,
		/* min,max speed  */ 10.0, 50.0,
		/* min,max life   */ 10.0, 54.0
		);

	}
	break;
	/* Shots related to a player. */

    case OBJ_MINE:
    case OBJ_HEAT_SHOT:
    case OBJ_TORPEDO:
    case OBJ_SMART_SHOT:
    case OBJ_CANNON_SHOT:
	if (shot->mass == 0)
	    break;

	status = GRAVITY;
	if (shot->type == OBJ_MINE)
	    status |= COLLISIONSHOVE;
	if (BIT(shot->status, FROMCANNON))
	    status |= FROMCANNON;

	if (BIT(shot->mods.nuclear, NUCLEAR))
	    sound_play_all(NUKE_EXPLOSION_SOUND);
	else if (BIT(shot->type, OBJ_MINE))
	    sound_play_sensors(shot->pos, MINE_EXPLOSION_SOUND);
	else
	    sound_play_sensors(shot->pos, OBJECT_EXPLOSION_SOUND);

	if (BIT(shot->mods.warhead, CLUSTER)) {
	    type = OBJ_SHOT;
	    if (shot->id != NO_ID)
		color = Player_by_id(shot->id)->color;
	    else
		color = WHITE;
	    mass = ShotsMass;
	    mass *= 3;
	    modv = 1 << shot->mods.velocity;
	    num_modv = 4;
	    if (BIT(shot->mods.nuclear, NUCLEAR)) {
		modv *= 4.0f;
		num_modv = 1;
	    }
	    life_modv = modv * 0.20f;
	    speed_modv = 1.0f / modv;
	    intensity = (int)CLUSTER_MASS_SHOTS(shot->mass);
	} else {
	    type = OBJ_DEBRIS;
	    color = RED;
	    mass = DEBRIS_MASS;
	    modv = 1;
	    num_modv = 1;
	    life_modv = modv;
	    speed_modv = modv;
	    if (shot->type == OBJ_MINE)
		intensity = 512;
	    else
		intensity = 32;
	    /*
	     * Writing it like this:
	     *   num_modv /= (shot->mods.mini + 1);
	     * triggers a bug in HP C A.09.19.
	     */
	    num_modv = num_modv / ((double)(unsigned)shot->mods.mini + 1.0f);
	}

	if (BIT(shot->mods.nuclear, NUCLEAR)) {
	    double nuke_factor;
	    if (shot->type == OBJ_MINE)
		nuke_factor = NUKE_MINE_EXPL_MULT * shot->mass / MINE_MASS;
	    else
		nuke_factor
		    = NUKE_SMART_EXPL_MULT * shot->mass / MISSILE_MASS;

	    nuke_factor
		= (nuke_factor * (shot->mods.mini + 1)) / SHOT_MULT(shot);
	    intensity = (int)(intensity * nuke_factor);
	}

	if (BIT(shot->mods.warhead, IMPLOSION))
	    /*intensity >>= 1;*/
	    mass = -mass;

	if (BIT(shot->type, OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_SMART_SHOT))
	    intensity /= (1 + shot->mods.power);

	Make_debris(
	    /* pos            */ shot->prevpos,
	    /* vel            */ shot->vel,
	    /* owner id       */ shot->id,
	    /* owner team     */ shot->team,
	    /* kind           */ type,
	    /* mass           */ mass,
	    /* status         */ status,
	    /* color          */ color,
	    /* radius         */ 6,
	    /* num debris     */ (int)((0.20f * intensity * num_modv) +
	                         (0.10f * intensity * num_modv * rfrac())),
	    /* min,max dir    */ 0, RES-1,
	    /* min,max speed  */ 20 * speed_modv,
				 (intensity >> 2) * speed_modv,
	    /* min,max life   */ (8 * life_modv), (intensity >> 1) * life_modv
	    );
	break;

    case OBJ_SHOT:
	if (shot->id == NO_ID
	    || BIT(shot->status, FROMCANNON)
	    || BIT(shot->mods.warhead, CLUSTER))
	    break;
	pl = Player_by_id(shot->id);
	if (--pl->shots <= 0)
	    pl->shots = 0;
	break;

    case OBJ_PULSE:
	if (shot->id == NO_ID
	    || BIT(shot->status, FROMCANNON))
	    break;
	pl = Player_by_id(shot->id);
	if (--pl->num_pulses <= 0)
	    pl->num_pulses = 0;
	break;

	/* Special items. */
    case OBJ_ITEM:

	switch (shot->info) {

	case ITEM_MISSILE:
	    /* If -timeStep < shot->life <= 0, then it died of old age. */
	    /* If it was picked up, then life was set to 0 and it is now
	     * -timeStep after the substract in update.c. */
	    if (-timeStep < shot->life && shot->life <= 0) {
		if (shot->color != WHITE) {
		    shot->color = WHITE;
		    shot->life  = WARN_TIME;
		    return;
		}
		if (rfrac() < rogueHeatProb)
		    addHeat = true;
	    }
	    break;

	case ITEM_MINE:
	    /* See comment for ITEM_MISSILE above */
	    if (-timeStep < shot->life && shot->life <= 0) {
		if (shot->color != WHITE) {
		    shot->color = WHITE;
		    shot->life  = WARN_TIME;
		    return;
		}
		if (rfrac() < rogueMineProb)
		    addMine = true;
	    }
	    break;

	default:
	    break;
	}

	World.items[shot->info].num--;

	break;

    default:
	xpprintf("%s Delete_shot(): Unknown shot type %d.\n",
		 showtime(), shot->type);
	break;
    }

    Cell_remove_object(shot);
    shot->life = 0;
    shot->type = 0;
    shot->mass = 0;

    Object_free_ind(ind);

    if (addMine || addHeat) {
	CLEAR_MODS(mods);
	if (BIT(World.rules->mode, ALLOW_CLUSTERS) && (rfrac() <= 0.333f))
	    SET_BIT(mods.warhead, CLUSTER);
	if (BIT(World.rules->mode, ALLOW_MODIFIERS) && (rfrac() <= 0.333f))
	    SET_BIT(mods.warhead, IMPLOSION);
	if (BIT(World.rules->mode, ALLOW_MODIFIERS))
	    mods.velocity = (int)(rfrac() * (MODS_VELOCITY_MAX + 1));
	if (BIT(World.rules->mode, ALLOW_MODIFIERS))
	    mods.power = (int)(rfrac() * (MODS_POWER_MAX + 1));
	if (addMine) {
	    long gravity_status = ((rfrac() < 0.5f) ? GRAVITY : 0);
	    vector zero_vel = { 0.0, 0.0 };

	    Place_general_mine(NULL, TEAM_NOT_SET, gravity_status,
			       shot->pos, zero_vel, mods);
	}
	else if (addHeat)
	    Fire_general_shot(NULL, TEAM_NOT_SET, 0, shot->pos,
			      OBJ_HEAT_SHOT, (int)(rfrac() * RES),
			      mods, NO_ID);
    }
    else if (addBall) {
	ball = BALL_PTR(shot);
	Make_treasure_ball(ball->treasure);
    }
}

void Fire_laser(player *pl)
{
    clpos	m_gun, pos;

    if (frame_time <= pl->laser_time + laserRepeatRate - timeStep + 1e-3)
 	return;
    pl->laser_time = MAX(frame_time, pl->laser_time + laserRepeatRate);

    if (pl->item[ITEM_LASER] > pl->num_pulses
	&& pl->velocity < pulseSpeed) {
	if (pl->fuel.sum < -ED_LASER)
	    CLR_BIT(pl->used, HAS_LASER);
	else {
	    m_gun = Ship_get_m_gun_clpos(pl->ship, pl->dir);
	    pos.cx = pl->pos.cx + m_gun.cx
		+ FLOAT_TO_CLICK(pl->vel.x * timeStep);
	    pos.cy = pl->pos.cy + m_gun.cy
		+ FLOAT_TO_CLICK(pl->vel.y * timeStep);
	    pos.cx = WRAP_XCLICK(pos.cx);
	    pos.cy = WRAP_YCLICK(pos.cy);
	    if (is_inside(pos.cx, pos.cy, NONBALL_BIT | NOTEAM_BIT, NULL)
		!= NO_GROUP)
		return;
	    Fire_general_laser(pl, pl->team, pos, pl->dir, pl->mods);
	}
    }
}

void Fire_general_laser(player *pl, int team, clpos pos,
			int dir, modifiers mods)
{
    int			life;
    pulseobject		*pulse;

    if (!INSIDE_MAP(pos.cx, pos.cy)) {
	warn("Fire_general_laser: not inside map.\n");
	return;
    }

    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    if ((pulse = PULSE_PTR(Object_allocate())) == NULL)
	return;

    if (pl) {
	Player_add_fuel(pl, ED_LASER);
	sound_play_sensors(pos, FIRE_LASER_SOUND);
	/* kps - hmm ??? */
	/*life = (int)PULSE_LIFE(pl->item[ITEM_LASER]);*/
	life = pulseLife;
	/*Rank_FireLaser(pl);*/
    } else
	life = (int)CANNON_PULSE_LIFE;

    pulse->id		= (pl ? pl->id : NO_ID);
    pulse->team 	= team;
    Object_position_init_clicks(OBJ_PTR(pulse), pos.cx, pos.cy);
    pulse->vel.x 	= pulseSpeed * tcos(dir);
    pulse->vel.y 	= pulseSpeed * tsin(dir);
    pulse->acc.x 	= 0;
    pulse->acc.y 	= 0;
    pulse->mass	 	= 0;
    pulse->life 	= life;
    pulse->status 	= (pl ? 0 : FROMCANNON);
    pulse->type 	= OBJ_PULSE;
    pulse->count 	= 0;
    pulse->mods 	= mods;
    pulse->color	= (pl ? pl->color : WHITE);

    pulse->info 	= 0;
    pulse->fusetime	= frame_time;
    pulse->pl_range 	= 0;
    pulse->pl_radius 	= 0;

    pulse->dir  	= dir;
    pulse->len  	= 0 /*pulseLength * CLICK*/;
    pulse->refl 	= false;

    Cell_add_object(OBJ_PTR(pulse));

    if (pl)
	pl->num_pulses++;
}


/*
 * The new ball movement code since XPilot version 3.4.0 as made
 * by Bretton Wade.  The code was submitted in context diff format
 * by Mark Boyns.  Here is a an excerpt from a post in
 * rec.games.computer.xpilot by Bretton Wade dated 27 Jun 1995:
 *
 * If I'm not mistaken (not having looked very closely at the code
 * because I wasn't sure what it was trying to do), the original move_ball
 * routine was trying to model a Hook's law spring, but squared the
 * deformation term, which would lead to exagerated behavior as the spring
 * stretched too far. Not really a divide by zero, but effectively
 * producing large numbers.
 *
 * When I coded up the spring myself, I found that I could recreate the
 * effect by using a VERY strong spring. This can be defeated, however, by
 * damping. Specifically, If you compute the critical damping factor, then
 * you could have the cable always be the correct length. This makes me
 * wonder how to decide when the cable snaps.
 *
 * I chose a relatively strong spring, and a small damping factor, to make
 * for a nice realistic bounce when you grab at the treasure. It also
 * gives a fairley close approximation to the "normal" feel of the
 * treasure.
 *
 * I modeled the cable as having zero mass, or at least insignificant mass
 * as compared to the ship and ball. This greatly simplifies the math, and
 * leads to the conclusion that there will be no change in velocity when
 * the cable breaks. You can check this by integrating the momentum along
 * the cable, and the ship or ball.
 *
 * If you assume that the cable snaps in the middle, then half of the
 * potential energy goes to each object attached. However, as you said, the
 * total momentum of the system cannot change. Because the weight of the
 * cable is small, the vast majority of the potential energy will become
 * heat. I've had two physicists verify this for me, and they both worked
 * really hard on the problem because they found it interesting.
 *
 * End of post.
 *
 * Changes since then:
 *
 * Comment from people was that the string snaps too soon.
 * Changed the value (max_spring_ratio) at which the string snaps
 * from 0.25 to 0.30.  Not sure if that helps enough, or too much.
 */
void Update_connector_force(ballobject *ball)
{
    player		*pl = Player_by_id(ball->id);
    vector		D;
    double		length, force, ratio, accell, damping;
    /* const double		k = 1500.0, b = 2.0; */
    /* const double		max_spring_ratio = 0.30; */

    /* no player connected ? */
    if (!pl)
	return;

    /* compute the normalized vector between the ball and the player */
    D.x = WRAP_DCX(pl->pos.cx - ball->pos.cx);
    D.y = WRAP_DCY(pl->pos.cy - ball->pos.cy);
    length = VECTOR_LENGTH(D);
    if (length > 0.0) {
	D.x /= length;
	D.y /= length;
    } else
	D.x = D.y = 0.0;

    /* compute the ratio for the spring action */
    ratio = 1 - length / (ballConnectorLength * CLICK);

    /* compute force by spring for this length */
    force = ballConnectorSpringConstant * ratio;

    /* If we have string-style connectors then it is allowed to be
     * shorted than its natural length. */
    if (connectorIsString && ratio > 0.0)
	return;

    /* if the tether is too long or too short, release it */
    if (ABS(ratio) > maxBallConnectorRatio) {
	Detach_ball(pl, ball);
	return;
    }

    damping = -ballConnectorDamping * ((pl->vel.x - ball->vel.x) * D.x +
				       (pl->vel.y - ball->vel.y) * D.y);

    /* compute accelleration for player, assume t = 1 */
    accell = (force + damping) / pl->mass;
    pl->vel.x += D.x * accell * timeStep;
    pl->vel.y += D.y * accell * timeStep;

    /* compute accelleration for ball, assume t = 1 */
    accell = (force + damping) / ball->mass;
    ball->vel.x += -D.x * accell * timeStep;
    ball->vel.y += -D.y * accell * timeStep;
}

void Update_torpedo(torpobject *torp)
{
    double		acc;

    if (BIT(torp->mods.nuclear, NUCLEAR))
	acc = (torp->count < NUKE_SPEED_TIME) ? NUKE_ACC : 0.0;
    else
	acc = (torp->count < TORPEDO_SPEED_TIME) ? TORPEDO_ACC : 0.0;
    torp->count += timeStep;
    acc *= (1 + (torp->mods.power * MISSILE_POWER_SPEED_FACT));
    if ((torp->spread_left -= timeStep) <= 0) {
	torp->acc.x = 0;
	torp->acc.y = 0;
	torp->spread_left = 0;
    }
    torp->vel.x += acc * tcos(torp->missile_dir);
    torp->vel.y += acc * tsin(torp->missile_dir);
}

void Update_missile(missileobject *shot)
{
    player		*pl;
    int			angle, theta;
    double		range = 0.0;
    double		acc = SMART_SHOT_ACC;
    double		x_dif = 0.0;
    double		y_dif = 0.0;
    double		shot_speed;

    if (shot->type == OBJ_HEAT_SHOT) {
	acc = SMART_SHOT_ACC * HEAT_SPEED_FACT;
	if (shot->info >= 0) {
	    clpos engine;
	    /* Get player and set min to distance */
	    pl = Player_by_id(shot->info);
	    engine = Ship_get_engine_clpos(pl->ship, pl->dir);
	    range = Wrap_length(pl->pos.cx + engine.cx - shot->pos.cx,
				pl->pos.cy + engine.cy - shot->pos.cy)
		/ CLICK;
	} else {
	    /* No player. Number of moves so that new target is searched */
	    pl = NULL;
	    shot->count = HEAT_WIDE_TIMEOUT + HEAT_WIDE_ERROR;
	}
	if (pl && BIT(pl->status, THRUSTING)) {
	    /*
	     * Target is thrusting,
	     * set number to moves to correct error value
	     */
	    if (range < HEAT_CLOSE_RANGE)
		shot->count = HEAT_CLOSE_ERROR;
	    else if (range < HEAT_MID_RANGE)
		shot->count = HEAT_MID_ERROR;
	    else
		shot->count = HEAT_WIDE_ERROR;
	} else {
	    shot->count += timeStep;
	    /* Look for new target */
	    if ((range < HEAT_CLOSE_RANGE
		 && shot->count > HEAT_CLOSE_TIMEOUT + HEAT_CLOSE_ERROR)
		|| (range < HEAT_MID_RANGE
		    && shot->count > HEAT_MID_TIMEOUT + HEAT_MID_ERROR)
		|| shot->count > HEAT_WIDE_TIMEOUT + HEAT_WIDE_ERROR) {
		double l;
		int i;

		range = HEAT_RANGE * (shot->count / HEAT_CLOSE_TIMEOUT);
		for (i = 0; i < NumPlayers; i++) {
		    player *pl_i = Players(i);
		    clpos engine;

		    if (!BIT(pl_i->status, THRUSTING))
			continue;

		    engine = Ship_get_engine_clpos(pl_i->ship, pl_i->dir);
		    l = Wrap_length(pl_i->pos.cx + engine.cx - shot->pos.cx,
				    pl_i->pos.cy + engine.cy - shot->pos.cy)
			/ CLICK;
		    /*
		     * After burners can be detected easier;
		     * so scale the length:
		     */
		    l *= MAX_AFTERBURNER + 1 - pl_i->item[ITEM_AFTERBURNER];
		    l /= MAX_AFTERBURNER + 1;
		    if (BIT(pl_i->have, HAS_AFTERBURNER))
			l *= 16 - pl_i->item[ITEM_AFTERBURNER];
		    if (l < range) {
			shot->info = pl_i->id;
			range = l;
			shot->count =
			    l < HEAT_CLOSE_RANGE ?
				HEAT_CLOSE_ERROR : l < HEAT_MID_RANGE ?
				    HEAT_MID_ERROR : HEAT_WIDE_ERROR;
			pl = pl_i;
		    }
		}
	    }
	}
	if (shot->info < 0)
	    return;
	/*
	 * Heat seekers cannot fly exactly, if target is far away or thrust
	 * isn't active.  So simulate the error:
	 */
	x_dif = rfrac() * 4 * shot->count;
	y_dif = rfrac() * 4 * shot->count;

    }
    else if (shot->type == OBJ_SMART_SHOT) {
	smartobject *smart = SMART_PTR(shot);

	/*
	 * kps - this can cause Arithmetic Exception (division by zero)
	 * since CONFUSED_UPDATE_GRANULARITY / gameSpeed is most often
	 * < 1 and when it is cast to int it will be 0, and then
	 * we get frameloops % 0, which is not good.
	 */
	/*if (BIT(smart->status, CONFUSED)
	  && (!(frame_loops % (int)(CONFUSED_UPDATE_GRANULARITY / gameSpeed)
	  || smart->count == CONFUSED_TIME))) {*/
	/* not going to fix now, I'll just remove the '/ gamespeed' part */

	if (BIT(smart->status, CONFUSED)
	    && (!(frame_loops % CONFUSED_UPDATE_GRANULARITY)
		|| smart->count == CONFUSED_TIME)) {

	    if (smart->count > 0) {
		smart->info = Players((int)(rfrac() * NumPlayers))->id;
		smart->count -= timeStep;
	    } else {
		smart->count = 0;
		CLR_BIT(smart->status, CONFUSED);

		/* range is percentage from center to periphery of ecm burst */
		range = (ECM_DISTANCE - smart->ecm_range) / ECM_DISTANCE;
		range *= 100.0;

		/*
		 * range%	lock%
		 * 100		100
		 *  50		75
		 *   0		50
		 */
		if ((int)(rfrac() * 100) <= ((int)(range/2)+50))
		    smart->info = smart->new_info;
	    }
	}
	pl = Player_by_id(shot->info);
    }
    else
	/*NOTREACHED*/
	return;

    /*
     * Use a little look ahead to fly more exact
     */
    acc *= (1 + (shot->mods.power * MISSILE_POWER_SPEED_FACT));
    if ((shot_speed = VECTOR_LENGTH(shot->vel)) < 1) shot_speed = 1;
    range = Wrap_length(pl->pos.cx - shot->pos.cx,
			pl->pos.cy - shot->pos.cy) / CLICK;
    x_dif += pl->vel.x * (range / shot_speed);
    y_dif += pl->vel.y * (range / shot_speed);
    theta = Wrap_cfindDir(pl->pos.cx + PIXEL_TO_CLICK(x_dif) - shot->pos.cx,
			  pl->pos.cy + PIXEL_TO_CLICK(y_dif) - shot->pos.cy);

    {
	double x, y, vx, vy;
	int i, xi, yi, j, freemax, k, foundw;
	static struct {
	    int dx, dy;
	} sur[8] = {
	    {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}
	};

#define BLOCK_PARTS 2
	vx = shot->vel.x;
	vy = shot->vel.y;
	x = shot_speed / (BLOCK_SZ*BLOCK_PARTS);
	vx /= x; vy /= x;
	x = CLICK_TO_PIXEL(shot->pos.cx); y = CLICK_TO_PIXEL(shot->pos.cy);
	foundw = 0;

	for (i = SMART_SHOT_LOOK_AH; i > 0 && foundw == 0; i--) {
	    xi = (int)((x += vx) / BLOCK_SZ);
	    yi = (int)((y += vy) / BLOCK_SZ);
	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (xi < 0) xi += World.x;
		else if (xi >= World.x) xi -= World.x;
		if (yi < 0) yi += World.y;
		else if (yi >= World.y) yi -= World.y;
	    }
	    if (xi < 0 || xi >= World.x || yi < 0 || yi >= World.y)
		break;

	    /* kps - fix */
	    if (!is_polygon_map) {
		switch(World.block[xi][yi]) {
		case TARGET:
		case TREASURE:
		case FUEL:
		case FILLED:
		case REC_LU:
		case REC_RU:
		case REC_LD:
		case REC_RD:
		case CANNON:
		    if (range
			> (SMART_SHOT_LOOK_AH-i)*(BLOCK_SZ/BLOCK_PARTS)) {
			if (shot_speed > SMART_SHOT_MIN_SPEED)
			    shot_speed -= acc * (SMART_SHOT_DECFACT+1);
		    }
		    foundw = 1;
		    break;
		default:
		    break;
		}
	    }
	}

	i = ((int)(shot->missile_dir * 8 / RES)&7) + 8;
	xi = OBJ_X_IN_BLOCKS(shot);
	yi = OBJ_Y_IN_BLOCKS(shot);

	for (j = 2, angle = -1, freemax = 0; j >= -2; --j) {
	    int si, xt, yt;

	    for (si=1, k=0; si >= -1; --si) {
		xt = xi + sur[(i+j+si)&7].dx;
		yt = yi + sur[(i+j+si)&7].dy;

		if (xt >= 0 && xt < World.x && yt >= 0 && yt < World.y)
		    if (!is_polygon_map) {
			switch (World.block[xt][yt]) {
			case TARGET:
			case TREASURE:
			case FUEL:
			case FILLED:
			case REC_LU:
			case REC_RU:
			case REC_LD:
			case REC_RD:
			case CANNON:
			    if (!si)
				k = -32;
			    break;
			default:
			    ++k;
			    break;
			}
		    }
	    }
	    if (k > freemax
		|| (k == freemax
		    && ((j == -1 && (rfrac() < 0.5f)) || j == 0 || j == 1))) {
		freemax = k > 2 ? 2 : k;
		angle = i + j;
	    }

	    if (k == 3 && !j) {
		angle = -1;
		break;
	    }
	}

	if (angle >= 0) {
	    i = angle&7;
	    theta = Wrap_findDir(
		(yi + sur[i].dy) * BLOCK_SZ - (CLICK_TO_PIXEL(shot->pos.cy)
					       + 2 * shot->vel.y),
		(xi + sur[i].dx) * BLOCK_SZ - (CLICK_TO_PIXEL(shot->pos.cx)
					       - 2 * shot->vel.x));
#ifdef SHOT_EXTRA_SLOWDOWN
	    if (!foundw && range > (SHOT_LOOK_AH-i) * BLOCK_SZ) {
		if (shot_speed
		    > (SMART_SHOT_MIN_SPEED + SMART_SHOT_MAX_SPEED)/2)
		    shot_speed -= SMART_SHOT_DECC+SMART_SHOT_ACC;
	    }
#endif
	}
    }
    angle = theta;

    if (angle < 0)
	angle += RES;
    angle %= RES;

    if (angle < shot->missile_dir)
	angle += RES;
    angle = angle - shot->missile_dir - RES/2;

    if (angle < 0)
	shot->missile_dir += (u_byte)(((-angle < shot->turnspeed)
					? -angle
					: shot->turnspeed));
    else
	shot->missile_dir -= (u_byte)(((angle < shot->turnspeed)
					? angle
					: shot->turnspeed));

    shot->missile_dir = MOD2(shot->missile_dir, RES); /* NOTE!!!! */

    if (shot_speed < shot->max_speed)
	shot_speed += acc;

    /*  shot->velocity = MIN(shot->velocity, shot->max_speed);  */

    shot->vel.x = tcos(shot->missile_dir) * shot_speed;
    shot->vel.y = tsin(shot->missile_dir) * shot_speed;
}

void Update_mine(mineobject *mine)
{
    if (BIT(mine->status, CONFUSED)) {
	if ((mine->count -= timeStep) <= 0) {
	    CLR_BIT(mine->status, CONFUSED);
	    mine->count = 0;
	}
    }

    /* if mineFuseTime == 0, owner immunity never expires */
    if (BIT(mine->status, OWNERIMMUNE) && mine->fusetime > 0) {
	if ((mine->fusetime -= timeStep) <= 0) {
	    CLR_BIT(mine->status, OWNERIMMUNE);
	    mine->fusetime = 0;
	}
    }

    if (mine->mods.mini) {
	if ((mine->spread_left -= timeStep) <= 0) {
	    mine->acc.x = 0;
	    mine->acc.y = 0;
	    mine->spread_left = 0;
	}
    }
}

/*
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

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
#include "score.h"
#include "bit.h"
#include "saudio.h"
#include "objpos.h"
#include "cannon.h"
#include "asteroid.h"
#include "click.h"
#include "commonproto.h"
#include "netserver.h"

char update_version[] = VERSION;


/* kps - gravity is block based, even on polygon maps */
#define update_object_speed(o_)						\
    if (BIT((o_)->status, GRAVITY)) {					\
	(o_)->vel.x += ((o_)->acc.x					\
	  + World.gravity[(o_)->pos.bx][(o_)->pos.by].x) * timeStep2;	\
	(o_)->vel.y += ((o_)->acc.y					\
	  + World.gravity[(o_)->pos.bx][(o_)->pos.by].y) * timeStep2;	\
    } else {								\
	(o_)->vel.x += (o_)->acc.x * timeStep2;			\
	(o_)->vel.y += (o_)->acc.y * timeStep2;			\
    }

int	round_delay = 0;	/* delay until start of next round */
int	round_delay_send = 0;	/* number of frames to send round_delay */
int	roundtime = -1;		/* time left this round */
static int time_to_update = TIME_FACT;	/* time before less frequent updates */
static bool do_update_this_frame = false; /* less frequent update this frame */

static char msg[MSG_LEN];


static void Transport_to_home(int ind)
{
    /*
     * Transport a corpse from the place where it died back to its homebase,
     * or if in race mode, back to the last passed check point.
     *
     * During the first part of the distance we give it a positive constant
     * acceleration G, during the second part we make this a negative one -G.
     * This results in a visually pleasing take off and landing.
     */
    player		*pl = Players[ind];
    int			cx, cy;
    DFLOAT		dx, dy, t, m;
    const int		T = RECOVERY_DELAY;

    if (BIT(World.rules->mode, TIMING) && pl->round) {
	int check;

	if (pl->check)
		check = pl->check - 1;
	else
		check = World.NumChecks - 1;
	cx = World.check[check].cx;
	cy = World.check[check].cy;
    } else {
	cx = World.base[pl->home_base].pos.cx;
	cy = World.base[pl->home_base].pos.cy;
    }
    dx = WRAP_DCX(cx - pl->pos.cx);
    dy = WRAP_DCY(cy - pl->pos.cy);
    t = pl->count + 0.5f;
    if (2 * t <= T) {
	m = 2 / t;
    } else {
	t = T - t;
	m = (4 * t) / (T * T - 2 * t * t);
    }
    /* kps - this can be optimized by multiplying by timeStep here
     * and not multiplying by timeStep2 in some places of Move_player
     * functions, but it can be confusing.
     */
    m *= TIME_FACT;
    pl->vel.x = dx * m / CLICK;
    pl->vel.y = dy * m / CLICK;
}

/*
 * Turn phasing on or off.
 */
void Phasing(int ind, int on)
{
    player	*pl = Players[ind];

    if (on) {
	if (pl->phasing_left <= 0) {
	    pl->phasing_left = PHASING_TIME;
	    pl->item[ITEM_PHASING]--;
	}
	SET_BIT(pl->used, HAS_PHASING_DEVICE);
	CLR_BIT(pl->used, HAS_REFUEL);
	CLR_BIT(pl->used, HAS_REPAIR);
	if (BIT(pl->used, HAS_CONNECTOR))
	    pl->ball = NULL;
	CLR_BIT(pl->used, HAS_TRACTOR_BEAM);
	CLR_BIT(pl->status, GRAVITY);
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PHASING_ON_SOUND);
    } else {
	CLR_BIT(pl->used, HAS_PHASING_DEVICE);
	if (pl->phasing_left <= 0) {
	    if (pl->item[ITEM_PHASING] <= 0)
		CLR_BIT(pl->have, HAS_PHASING_DEVICE);
	}
	SET_BIT(pl->status, GRAVITY);
	sound_play_sensors(pl->pos.cx, pl->pos.cy, PHASING_OFF_SOUND);
    }
}

/*
 * Turn cloak on or off.
 */
void Cloak(int ind, int on)
{
    player	*pl = Players[ind];

    if (on) {
	if (!BIT(pl->used, HAS_CLOAKING_DEVICE) && pl->item[ITEM_CLOAK] > 0) {
	    if (!cloakedShield) {
		if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
		    Emergency_shield(ind, false);
		if (BIT(pl->used, HAS_DEFLECTOR))
		    Deflector(ind, false);
		CLR_BIT(pl->used, HAS_SHIELD);
		CLR_BIT(pl->have, HAS_SHIELD);
	    }
	    sound_play_player(pl, CLOAK_SOUND);
	    pl->updateVisibility = 1;
	    SET_BIT(pl->used, HAS_CLOAKING_DEVICE);
	}
    } else {
	if (BIT(pl->used, HAS_CLOAKING_DEVICE)) {
	    sound_play_player(pl, CLOAK_SOUND);
	    pl->updateVisibility = 1;
	    CLR_BIT(pl->used, HAS_CLOAKING_DEVICE);
	}
	if (!pl->item[ITEM_CLOAK])
	    CLR_BIT(pl->have, HAS_CLOAKING_DEVICE);
	if (!cloakedShield) {
	    if (BIT(pl->have, HAS_EMERGENCY_SHIELD)) {
		SET_BIT(pl->have, HAS_SHIELD);
		Emergency_shield(ind, true);
	    }
	    if (BIT(DEF_HAVE, HAS_SHIELD) && !BIT(pl->have, HAS_SHIELD))
		SET_BIT(pl->have, HAS_SHIELD);
	    if (BITV_ISSET(pl->last_keyv, KEY_SHIELD))
		SET_BIT(pl->used, HAS_SHIELD);
	}
    }
}

/*
 * Turn deflector on or off.
 */
void Deflector(int ind, int on)
{
    player	*pl = Players[ind];

    if (on) {
	if (!BIT(pl->used, HAS_DEFLECTOR) && pl->item[ITEM_DEFLECTOR] > 0) {
	    if (!cloakedShield || !BIT(pl->used, HAS_CLOAKING_DEVICE)) {
		SET_BIT(pl->used, HAS_DEFLECTOR);
		sound_play_player(pl, DEFLECTOR_SOUND);
	    }
	}
    } else {
	if (BIT(pl->used, HAS_DEFLECTOR)) {
	    CLR_BIT(pl->used, HAS_DEFLECTOR);
	    sound_play_player(pl, DEFLECTOR_SOUND);
	}
	if (!pl->item[ITEM_DEFLECTOR])
	    CLR_BIT(pl->have, HAS_DEFLECTOR);
    }
}

/*
 * Turn emergency thrust on or off.
 */
void Emergency_thrust (int ind, int on)
{
    player	*pl = Players[ind];

    if (on) {
	if (pl->emergency_thrust_left <= 0) {
	    pl->emergency_thrust_left = EMERGENCY_THRUST_TIME;
	    pl->item[ITEM_EMERGENCY_THRUST]--;
	}
	if (!BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	    SET_BIT(pl->used, HAS_EMERGENCY_THRUST);
	    sound_play_sensors(pl->pos.cx, pl->pos.cy, EMERGENCY_THRUST_ON_SOUND);
	}
    } else {
	if (BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	    CLR_BIT(pl->used, HAS_EMERGENCY_THRUST);
	    sound_play_sensors(pl->pos.cx, pl->pos.cy, EMERGENCY_THRUST_OFF_SOUND);
	}
	if (pl->emergency_thrust_left <= 0) {
	    if (pl->item[ITEM_EMERGENCY_THRUST] <= 0)
		CLR_BIT(pl->have, HAS_EMERGENCY_THRUST);
	}
    }
}

/*
 * Turn emergency shield on or off.
 */
void Emergency_shield (int ind, int on)
{
    player	*pl = Players[ind];

    if (on) {
	if (BIT(pl->have, HAS_EMERGENCY_SHIELD)) {
	    if (pl->emergency_shield_left <= 0) {
		pl->emergency_shield_left += EMERGENCY_SHIELD_TIME;
		pl->item[ITEM_EMERGENCY_SHIELD]--;
	    }
	    if (cloakedShield || !BIT(pl->used, HAS_CLOAKING_DEVICE)) {
		SET_BIT(pl->have, HAS_SHIELD);
		if (!BIT(pl->used, HAS_EMERGENCY_SHIELD)) {
		    SET_BIT(pl->used, HAS_EMERGENCY_SHIELD);
		    sound_play_sensors(pl->pos.cx, pl->pos.cy,
				       EMERGENCY_SHIELD_ON_SOUND);
		}
	    }
	}
    } else {
	if (pl->emergency_shield_left <= 0) {
	    if (pl->item[ITEM_EMERGENCY_SHIELD] <= 0)
		CLR_BIT(pl->have, HAS_EMERGENCY_SHIELD);
	}
	if (!BIT(DEF_HAVE, HAS_SHIELD)) {
	    CLR_BIT(pl->have, HAS_SHIELD);
	    CLR_BIT(pl->used, HAS_SHIELD);
	}
	if (BIT(pl->used, HAS_EMERGENCY_SHIELD)) {
	    CLR_BIT(pl->used, HAS_EMERGENCY_SHIELD);
	    sound_play_sensors(pl->pos.cx, pl->pos.cy,
			       EMERGENCY_SHIELD_OFF_SOUND);
	}
    }
}

/*
 * Turn autopilot on or off.  This always clears the thrusting bit.  During
 * automatic pilot mode any changes to the current power, turnacc, turnspeed
 * and turnresistance settings will be temporary.
 */
void Autopilot (int ind, int on)
{
    player	*pl = Players[ind];

    CLR_BIT(pl->status, THRUSTING);
    if (on) {
	pl->auto_power_s = pl->power;
	pl->auto_turnspeed_s = pl->turnspeed;
	pl->auto_turnresistance_s = pl->turnresistance;
	SET_BIT(pl->used, HAS_AUTOPILOT);
	pl->power = (MIN_PLAYER_POWER+MAX_PLAYER_POWER)/2.0;
	pl->turnspeed = (MIN_PLAYER_TURNSPEED+MAX_PLAYER_TURNSPEED)/2.0;
	pl->turnresistance = 0.2;
	sound_play_sensors(pl->pos.cx, pl->pos.cy, AUTOPILOT_ON_SOUND);
    } else {
	pl->power = pl->auto_power_s;
	pl->turnacc = 0.0;
	pl->turnspeed = pl->auto_turnspeed_s;
	pl->turnresistance = pl->auto_turnresistance_s;
	CLR_BIT(pl->used, HAS_AUTOPILOT);
	sound_play_sensors(pl->pos.cx, pl->pos.cy, AUTOPILOT_OFF_SOUND);
    }
}

/*
 * Automatic pilot will try to hold the ship steady, turn to face away
 * from direction of travel, if so then turn on thrust which will
 * cause the ship to come to a rest within a short period of time.
 * This code is fairly self contained.
 */
static void do_Autopilot (player *pl)
{
    int		vad;	/* Velocity Away Delta */
    int		dir;
    int		afterburners;
    int		ix, iy;
    DFLOAT	gx, gy;
    DFLOAT	acc, vel;
    DFLOAT	delta;
    DFLOAT	turnspeed, power;
    const DFLOAT	emergency_thrust_settings_delta = 150.0 / FPS;
    const DFLOAT	auto_pilot_settings_delta = 15.0 / FPS;
    const DFLOAT	auto_pilot_turn_factor = 2.5;
    const DFLOAT	auto_pilot_dead_velocity = 0.5;

    /*
     * If the last movement touched a wall then we shouldn't
     * mess with the position (speed too?) settings.
     */
    if (pl->last_wall_touch + 1 >= frame_loops) {
	return;
    }

    /*
     * Having more autopilot items or using emergency thrust causes a much
     * quicker deceleration to occur than during normal flight.  Having
     * no autopilot items will cause minimum delta to occur, this is because
     * the autopilot code is used by the pause code.
     */
    delta = auto_pilot_settings_delta;
    if (pl->item[ITEM_AUTOPILOT])
	delta *= pl->item[ITEM_AUTOPILOT];

    if (BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	afterburners = MAX_AFTERBURNER;
	if (delta < emergency_thrust_settings_delta)
	    delta = emergency_thrust_settings_delta;
    } else {
	afterburners = pl->item[ITEM_AFTERBURNER];
    }

    ix = OBJ_X_IN_BLOCKS(pl);
    iy = OBJ_Y_IN_BLOCKS(pl);
    gx = World.gravity[ix][iy].x;
    gy = World.gravity[ix][iy].y;

    /*
     * Due to rounding errors if the velocity is very small we were probably
     * on target to stop last time round, so we actually absolutely stop.
     * This enables the ship to orient away from gravity and set up the
     * thrust to counteract it.
     */
    if ((vel = VECTOR_LENGTH(pl->vel)) < auto_pilot_dead_velocity)
	pl->vel.x = pl->vel.y = vel = 0.0;

    /*
     * Calculate power needed to change instantaneously to stopped.  We
     * must include gravity here for next time round the update loop.
     */
    acc = LENGTH(gx, gy) + vel;
    power = acc * pl->mass;
    if (afterburners)
	power /= AFTER_BURN_POWER_FACTOR(afterburners);

    /*
     * Calculate direction change needed to reduce velocity to zero.
     */
    if (vel == 0.0) {
	if (gx == 0 && gy == 0)
	    vad = pl->dir;
	else
	    vad = (int)findDir(-gx, -gy);
    } else {
	vad = (int)findDir(-pl->vel.x, -pl->vel.y);
    }
    vad = MOD2(vad - pl->dir, RES);
    if (vad > RES/2) {
	vad = RES - vad;
	dir = -1;
    } else {
	dir = 1;
    }

    /*
     * Calculate turnspeed needed to change direction instantaneously by
     * above direction change.
     */
    turnspeed = ((DFLOAT)vad) / pl->turnresistance - pl->turnvel;
    if (turnspeed < 0) {
	turnspeed = -turnspeed;
	dir = -dir;
    }

    /*
     * Change the turnspeed setting towards the perfect value, and limit
     * to the maximum only (limiting to the minimum causes oscillation).
     */
    if (turnspeed < pl->turnspeed) {
	pl->turnspeed -= delta;
	if (turnspeed > pl->turnspeed)
	    pl->turnspeed = turnspeed;
    } else if (turnspeed > pl->turnspeed) {
	pl->turnspeed += delta;
	if (turnspeed < pl->turnspeed)
	    pl->turnspeed = turnspeed;
    }
    if (pl->turnspeed > MAX_PLAYER_TURNSPEED)
	pl->turnspeed = MAX_PLAYER_TURNSPEED;

    /*
     * Decide if its wise to turn this time.
     */
    if (pl->turnspeed > (turnspeed*auto_pilot_turn_factor)) {
	pl->turnacc = 0.0;
	pl->turnvel = 0.0;
    } else {
	pl->turnacc = dir * pl->turnspeed;
    }

    /*
     * Change the power setting towards the perfect value, and limit
     * to the maximum only (limiting to the minimum causes oscillation).
     */
    if (power < pl->power) {
	pl->power -= delta;
	if (power > pl->power)
	    pl->power = power;
    } else if (power > pl->power) {
	pl->power += delta;
	if (power < pl->power)
	    pl->power = power;
    }
    if (pl->power > MAX_PLAYER_POWER)
	pl->power = MAX_PLAYER_POWER;

    /*
     * Don't thrust if the direction will not be absolutely correct and hasn't
     * been very close last time.  The latter clause was added such that
     * when a fine direction adjustment is needed, but the turnspeed is too
     * high at the moment, it gets the ship slowing down even though it
     * will impart some sideways velocity.
     */
    if (pl->turnspeed != turnspeed && vad > RES/32) {
	CLR_BIT(pl->status, THRUSTING);
	return;
    }

    /*
     * Only thrust if the power setting is correct or less than correct,
     * we don't want to over thrust.
     */
    if (pl->power > power) {
	CLR_BIT(pl->status, THRUSTING);
    } else {
	SET_BIT(pl->status, THRUSTING);
    }
}


static void Fuel_update(void)
{
    int i;
    DFLOAT fuel = (NumPlayers * STATION_REGENERATION);
    int frames_per_update = MAX_STATION_FUEL / (fuel * BLOCK_SZ);

    if (NumPlayers == 0)
	return;

    for (i = 0; i < World.NumFuels; i++) {
	if (World.fuel[i].fuel == MAX_STATION_FUEL) {
	    continue;
	}
	if ((World.fuel[i].fuel += fuel) >= MAX_STATION_FUEL) {
	    World.fuel[i].fuel = MAX_STATION_FUEL;
	}
	else if (World.fuel[i].last_change + frames_per_update
		 > frame_loops) {
	    /*
	     * We don't send fuelstation info to the clients every frame
	     * if it wouldn't change their display.
	     */
	    continue;
	}
	World.fuel[i].conn_mask = 0;
	World.fuel[i].last_change = frame_loops;
    }
}

static void Misc_object_update(void)
{
    int i;
    object *obj;

    for (i = 0; i < NumObjs; i++) {
	obj = Obj[i];

	if (BIT(obj->type, OBJ_MINE))
	    Move_mine(i);

	else if (BIT(obj->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT|OBJ_TORPEDO))
	    Move_smart_shot(i);

	else if (BIT(obj->type, OBJ_BALL) && obj->id != NO_ID)
	    Connector_force(i);

	else if (BIT(obj->type, OBJ_WRECKAGE)) {
	    wireobject *wireobj = WIRE_PTR(obj);
	    wireobj->rotation =
		(wireobj->rotation + (int) (wireobj->turnspeed * RES)) % RES;
	}

	else if (BIT(obj->type, OBJ_PULSE)) {
	    pulseobject *pulse = PULSE_PTR(obj);
	    pulse->len += pulseSpeed * timeStep2;
	    if (pulse->len > pulseLength)
		pulse->len = pulseLength;
	}

	update_object_speed(obj);

	if (!BIT(obj->type, OBJ_ASTEROID))
	    Move_object(obj);
    }
}

static void Cannon_update(int do_update_this_frame)
{
    int i;
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = World.cannon + i;
	if (cannon->dead_time > 0) {
	    if ((cannon->dead_time -= timeStep) <= 0)
		Cannon_restore_on_map(i);
	    continue;
	} else {
	    /* don't check too often, because this gets quite expensive
	       on maps with many cannons with defensive items */
	    if (do_update_this_frame
		&& cannonsUseItems
		&& cannonsDefend
		&& rfrac() < 0.65) {
		Cannon_check_defense(i);
	    }
	    if (do_update_this_frame
		&& !BIT(cannon->used, HAS_EMERGENCY_SHIELD)
		&& !BIT(cannon->used, HAS_PHASING_DEVICE)
		&& (cannon->damaged <= 0)
		&& (cannon->tractor_count <= 0)
		&& rfrac() * 16 < 1) {
		Cannon_check_fire(i);
	    }
	    else if (do_update_this_frame
		     && cannonsUseItems
		     && itemProbMult > 0
		     && cannonItemProbMult > 0) {
		int item = (int)(rfrac() * NUM_ITEMS);
		/* this gives the cannon an item about once every minute */
		if (World.items[item].cannonprob > 0
		    && cannonItemProbMult > 0
		    && (int)(rfrac() * (60 * 12))
			< (cannonItemProbMult * World.items[item].cannonprob)) {
		    Cannon_add_item(i, item, (item == ITEM_FUEL ?
					ENERGY_PACK_FUEL >> FUEL_SCALE_BITS
					: 1));
		}
	    }
	}
	if ((cannon->damaged -= timeStep) <= 0) {
	    cannon->damaged = 0;
	}
	if (cannon->tractor_count > 0) {
	    int ind = GetInd[cannon->tractor_target];
	    if (Wrap_length(Players[ind]->pos.cx - cannon->pos.cx,
			    Players[ind]->pos.cy - cannon->pos.cy)
		< TRACTOR_MAX_RANGE(cannon->item[ITEM_TRACTOR_BEAM]) * CLICK
		&& BIT(Players[ind]->status, PLAYING|GAME_OVER|KILLED|PAUSE)
		   == PLAYING) {
		General_tractor_beam(-1, cannon->pos.cx, cannon->pos.cy,
				     cannon->item[ITEM_TRACTOR_BEAM], ind,
				     cannon->tractor_is_pressor);
		if ((cannon->tractor_count -= timeStep) <= 0)
		    cannon->tractor_count = 0;
	    } else {
		cannon->tractor_count = 0;
	    }
	}
	if (cannon->emergency_shield_left > 0) {
	    if ((cannon->emergency_shield_left -= timeStep) <= 0) {
		CLR_BIT(cannon->used, HAS_EMERGENCY_SHIELD);
		sound_play_sensors(cannon->pos.cx, cannon->pos.cy,
				   EMERGENCY_SHIELD_OFF_SOUND);
	    }
	}
	if (cannon->phasing_left > 0) {
	    if ((cannon->phasing_left -= timeStep) <= 0) {
		CLR_BIT(cannon->used, HAS_PHASING_DEVICE);
	        sound_play_sensors(cannon->pos.cx, cannon->pos.cy,
				   PHASING_OFF_SOUND);
	    }
	}
    }
}

static void Target_update(void)
{
    int i, j;

    for (i = 0; i < World.NumTargets; i++) {
	if (World.targets[i].dead_time > 0) {
	    if ((World.targets[i].dead_time -= timeStep) <= 0) {
		Target_restore_on_map(i);

		if (targetSync) {
		    unsigned short team = World.targets[i].team;

		    for (j = 0; j < World.NumTargets; j++) {
			if (World.targets[j].team == team)
			    Target_restore_on_map(j);
		    }
		}
	    }
	    continue;
	}
	else if (World.targets[i].damage == TARGET_DAMAGE) {
	    continue;
	}

	World.targets[i].damage += TARGET_REPAIR_PER_FRAME;
	if (World.targets[i].damage >= TARGET_DAMAGE) {
	    World.targets[i].damage = TARGET_DAMAGE;
	}
	else if (World.targets[i].last_change + TARGET_UPDATE_DELAY
		 < frame_loops) {
	    /*
	     * We don't send target info to the clients every frame
	     * if the latest repair wouldn't change their display.
	     */
	    continue;
	}
	World.targets[i].conn_mask = 0;
	World.targets[i].last_change = frame_loops;
    }
}


static void Player_turns(void)
{
    int i;
    player *pl;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];

	if (BIT(pl->status, PLAYING|GAME_OVER|PAUSE) != PLAYING)
	    continue;

	/* Only do autopilot code if switched on and player is not
	 * damaged (ie. can see). */
	if (BIT(pl->used, HAS_AUTOPILOT)
	    || (BIT(pl->status, HOVERPAUSE) && !pl->damaged))
	    do_Autopilot(pl);

	pl->turnvel += pl->turnacc;

	/*
	 * turnresistance is zero: client requests linear turning behaviour
	 * when playing with pointer control.
	 */
	if (pl->turnresistance)
	    pl->turnvel *= pl->turnresistance;

	pl->float_dir += pl->turnvel;

	while (pl->float_dir < 0)
	    pl->float_dir += RES;
	while (pl->float_dir >= RES)
	    pl->float_dir -= RES;

	if (!pl->turnresistance)
	    pl->turnvel = 0;

	Turn_player(i);
    }
}

static void Use_items(int i)
{
    player *pl = Players[i];

    if (pl->shield_time > 0) {
	if ((pl->shield_time -= timeStep) <= 0) {
	    pl->shield_time = 0;
	    if (!BIT(pl->used, HAS_EMERGENCY_SHIELD))
		CLR_BIT(pl->used, HAS_SHIELD);
	}
	if (BIT(pl->used, HAS_SHIELD) == 0) {
	    if (!BIT(pl->used, HAS_EMERGENCY_SHIELD))
		CLR_BIT(pl->have, HAS_SHIELD);
	    pl->shield_time = 0;
	}
    }

    if (BIT(pl->used, HAS_PHASING_DEVICE)) {
	if ((pl->phasing_left -= timeStep2) <= 0) {
	    if (pl->item[ITEM_PHASING])
		Phasing(i, 1);
	    else
		Phasing(i, 0);
	}
    }

    if (BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	if (pl->fuel.sum > 0
	    && BIT(pl->status, THRUSTING)
	    && (pl->emergency_thrust_left -= timeStep2) <= 0) {
	    if (pl->item[ITEM_EMERGENCY_THRUST])
		Emergency_thrust(i, true);
	    else
		Emergency_thrust(i, false);
	}
    }

    if (BIT(pl->used, HAS_EMERGENCY_SHIELD)) {
	if (pl->fuel.sum > 0
	    && BIT(pl->used, HAS_SHIELD)
	    && ((pl->emergency_shield_left -= timeStep) <= 0)) {
	    if (pl->item[ITEM_EMERGENCY_SHIELD])
		Emergency_shield(i, true);
	    else
		Emergency_shield(i, false);
	}
    }

    if (do_update_this_frame && BIT(pl->used, HAS_DEFLECTOR))
	Do_deflector(i);	/* !@# no real need for do_update_this_frame */

    /*
     * Compute energy drainage
     */
    if (do_update_this_frame) {
	if (BIT(pl->used, HAS_SHIELD))
	    Add_fuel(&(pl->fuel), (long)ED_SHIELD);

	if (BIT(pl->used, HAS_PHASING_DEVICE))
	    Add_fuel(&(pl->fuel), (long)ED_PHASING_DEVICE);

	if (BIT(pl->used, HAS_CLOAKING_DEVICE))
	    Add_fuel(&(pl->fuel), (long)ED_CLOAKING_DEVICE);

	if (BIT(pl->used, HAS_DEFLECTOR))
	    Add_fuel(&(pl->fuel), (long)ED_DEFLECTOR);
    }
}


/********** **********
 * Updating objects and the like.
 */
void Update_objects(void)
{
    int i, j;
    player *pl;

    /*
     * Since the amount per frame of some things could get too small to
     * be represented accurately as an integer, FPSMultiplier makes these
     * things happen less often (in terms of frames) rather than smaller
     * amount each time.
     *
     * Can also be used to do some updates less frequently.
     */
    do_update_this_frame = false;
    if ((time_to_update -= timeStep) <= 0) {
	do_update_this_frame = true;
	time_to_update += TIME_FACT;
    }

#if 0
    xpprintf(__FILE__ ": frame loops / update : %ld / %d\n",
	     frame_loops, do_update_this_frame);
#endif

    /*
     * Update robots.
     */
    Robot_update();

    if (fastAim)
	Player_turns();

    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];

	if (pl->stunned > 0) {
	    pl->stunned -= timeStep;
	    if (pl->stunned <= 0)
		pl->stunned = 0;
	    CLR_BIT(pl->used, HAS_SHIELD|HAS_LASER|HAS_SHOT);
	    pl->did_shoot = false;
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (BIT(pl->used, HAS_SHOT) || pl->did_shoot)
	    Fire_normal_shots(i);
	if (BIT(pl->used, HAS_LASER)) {
	    if (pl->item[ITEM_LASER] <= 0 || BIT(pl->used, HAS_PHASING_DEVICE))
		CLR_BIT(pl->used, HAS_LASER);
	    else
		Fire_laser(i);
	}
	pl->did_shoot = false;
    }

    /*
     * Special items.
     */
    if (do_update_this_frame) {
	for (i = 0; i < NUM_ITEMS; i++)
	    if (World.items[i].num < World.items[i].max
		&& World.items[i].chance > 0
		&& (rfrac() * World.items[i].chance) < 1.0f)
		Place_item(i, -1);
    }

    Fuel_update();
    Misc_object_update();
    Asteroid_update();

    /*
     * Update ECM blasts
     */
    for (i = 0; i < NumEcms; i++) {
	if ((Ecms[i]->size *= ecmSizeFactor) < 1.0) {
	    if (Ecms[i]->id != NO_ID)
		Players[GetInd[Ecms[i]->id]]->ecmcount--;
	    free(Ecms[i]);
	    --NumEcms;
	    Ecms[i] = Ecms[NumEcms];
	    i--;
	}
    }

    /*
     * Update transporters
     */
    for (i = 0; i < NumTransporters; i++) {
	if ((Transporters[i]->count -= timeStep) <= 0) {
	    free(Transporters[i]);
	    --NumTransporters;
	    Transporters[i] = Transporters[NumTransporters];
	    i--;
	}
    }

    Cannon_update(do_update_this_frame);
    Target_update();

    if (!fastAim)
	Player_turns();

    /* * * * * *
     *
     * Player loop. Computes miscellaneous updates.
     *
     */
    for (i = 0; i < NumPlayers; i++) {
	pl = Players[i];

	if ((pl->damaged -= timeStep) <= 0)
	    pl->damaged = 0;

	/* kps - fix these */
	if (pl->flooding > FPS + 1) {
	    char msg[MSG_LEN];
	    sprintf(msg, "%s was kicked out because of flooding.", pl->name);
	    Destroy_connection(pl->conn, "flooding");
	    i--;
	    continue;
	} else if ( pl->flooding >= 0 )
	    pl->flooding--;

#define IDLETHRESHOLD (FPS * 60)

	if (IS_HUMAN_PTR(pl)) {
	    pl->rank->score = pl->score;
	    if ( pl->mychar == ' ' ) {
		if ( pl->idleCount++ == IDLETHRESHOLD ) {
		    if ( NumPlayers - 1 > NumPseudoPlayers + NumRobots ) {
			/* Kill player, he/she will be paused when returned
			   to base, unless he/she wakes up. */
			Kill_player(i);
			Rank_IgnoreLastDeath(pl);
		    } else
			pl->idleCount = 0;
		}
	    }
	}

	if (pl->count >= 0) {
	    pl->count -= timeStep;
	    if (pl->count > 0) {
		if (!BIT(pl->status, PLAYING)) {
		    Transport_to_home(i);
		    Move_player(i);
		    continue;
		}
	    } else {
		pl->count = -1;
		if (!BIT(pl->status, PLAYING)) {
		    if (pl->idleCount >= IDLETHRESHOLD) { /* idle */
			if (!game_lock && Team_zero_pausing_available()) {
			    sprintf(msg,
				    "%s was pause-swapped because of idling.",
				    Players[i]->name);
			    sprintf(team_0,"team 0");
			    Handle_player_command(Players[i],team_0);
			} else {
			    Pause_player(i, 1);
			    sprintf(msg, "%s was paused for idling.",
				    Players[i]->name);
			}
			Set_message(msg);
			continue;
		    }

		    SET_BIT(pl->status, PLAYING);
		    Go_home(i);
		}
		if (BIT(pl->status, SELF_DESTRUCT)) {
		    if (selfDestructScoreMult != 0) {
			DFLOAT sc = Rate(0, pl->score) * selfDestructScoreMult;
			SCORE(GetInd[pl->id], -sc, pl->pos.cx, pl->pos.cy,
			      "Self-Destruct");
		    }
		    SET_BIT(pl->status, KILLED);
		    sprintf(msg, "%s has committed suicide.", pl->name);
		    Set_message(msg);
		    Throw_items(i);
		    Kill_player(i);
		    updateScores = true;
		}
	    }
	}

	if (BIT(pl->status, PLAYING|GAME_OVER|PAUSE) != PLAYING)
	    continue;

	if (round_delay > 0)
	    continue;

	Use_items(i);

#define UPDATE_RATE 100

	for (j = 0; j < NumPlayers; j++) {
	    if (pl->forceVisible > 0)
		Players[j]->visibility[i].canSee = 1;

	    if (i == j || !BIT(Players[j]->used, HAS_CLOAKING_DEVICE))
		pl->visibility[j].canSee = 1;
	    else if (pl->updateVisibility
		     || Players[j]->updateVisibility
		     || (int)(rfrac() * UPDATE_RATE)
		     < ABS(frame_loops - pl->visibility[j].lastChange)) {

		pl->visibility[j].lastChange = frame_loops;
		pl->visibility[j].canSee
		    = (rfrac() * (pl->item[ITEM_SENSOR] + 1))
			> (rfrac() * (Players[j]->item[ITEM_CLOAK] + 1));
	    }
	}

	if (BIT(pl->used, HAS_REFUEL)) {
	    if ((Wrap_length(pl->pos.cx - World.fuel[pl->fs].pos.cx,
			     pl->pos.cy - World.fuel[pl->fs].pos.cy)
		 > 90.0 * CLICK)
		|| (pl->fuel.sum >= pl->fuel.max)
#if 0
		/* kps - ng wants this World.block part removed, currently
		 * it crashes a poly server if enabled since the World.block
		 * array is not initialized.
		 */
		|| (World.block[World.fuel[pl->fs].blk_pos.x]
			       [World.fuel[pl->fs].blk_pos.y] != FUEL)
#endif
		|| BIT(pl->used, HAS_PHASING_DEVICE)
		|| (BIT(World.rules->mode, TEAM_PLAY)
		    && teamFuel
		    && World.fuel[pl->fs].team != pl->team)) {
		CLR_BIT(pl->used, HAS_REFUEL);
	    } else {
		int i = pl->fuel.num_tanks;
		int ct = pl->fuel.current;

		do {
		    if (World.fuel[pl->fs].fuel > REFUEL_RATE) {
			World.fuel[pl->fs].fuel -= REFUEL_RATE;
			World.fuel[pl->fs].conn_mask = 0;
			World.fuel[pl->fs].last_change = frame_loops;
			Add_fuel(&(pl->fuel), REFUEL_RATE);
		    } else {
			Add_fuel(&(pl->fuel), World.fuel[pl->fs].fuel);
			World.fuel[pl->fs].fuel = 0;
			World.fuel[pl->fs].conn_mask = 0;
			World.fuel[pl->fs].last_change = frame_loops;
			CLR_BIT(pl->used, HAS_REFUEL);
			break;
		    }
		    if (pl->fuel.current == pl->fuel.num_tanks)
			pl->fuel.current = 0;
		    else
			pl->fuel.current += 1;
		} while (i--);
		pl->fuel.current = ct;
	    }
	}

	/* target repair */
	if (BIT(pl->used, HAS_REPAIR)) {
	    target_t *targ = &World.targets[pl->repair_target];
	    int cx = targ->pos.cx;
	    int cy = targ->pos.cy;
	    if (Wrap_length(pl->pos.cx - cx, pl->pos.cy - cy) > 90.0 * CLICK
		|| targ->damage >= TARGET_DAMAGE
		|| targ->dead_time > 0
		|| BIT(pl->used, HAS_PHASING_DEVICE)) {
		CLR_BIT(pl->used, HAS_REPAIR);
	    } else {
		int i = pl->fuel.num_tanks;
		int ct = pl->fuel.current;

		do {
		    if (pl->fuel.tank[pl->fuel.current] > REFUEL_RATE) {
			targ->damage += TARGET_FUEL_REPAIR_PER_FRAME;
			targ->conn_mask = 0;
			targ->last_change = frame_loops;
			Add_fuel(&(pl->fuel), -REFUEL_RATE);
			if (targ->damage > TARGET_DAMAGE) {
			    targ->damage = TARGET_DAMAGE;
			    break;
			}
		    } else {
			CLR_BIT(pl->used, HAS_REPAIR);
		    }
		    if (pl->fuel.current == pl->fuel.num_tanks)
			pl->fuel.current = 0;
		    else
			pl->fuel.current += 1;
		} while (i--);
		pl->fuel.current = ct;
	    }
	}

	if (pl->fuel.sum <= 0) {
	    CLR_BIT(pl->used, HAS_SHIELD|HAS_CLOAKING_DEVICE|HAS_DEFLECTOR);
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (pl->fuel.sum > (pl->fuel.max - REFUEL_RATE))
	    CLR_BIT(pl->used, HAS_REFUEL);

	/*
	 * Update acceleration vector etc.
	 */
	if (BIT(pl->status, THRUSTING)) {
	    DFLOAT power = pl->power;
	    DFLOAT f = pl->power * 0.0008;	/* 1/(FUEL_SCALE*MIN_POWER) */
	    int a = (BIT(pl->used, HAS_EMERGENCY_THRUST)
		     ? MAX_AFTERBURNER
		     : pl->item[ITEM_AFTERBURNER]);
	    DFLOAT inert = pl->mass;

	    if (a) {
		power = AFTER_BURN_POWER(power, a);
		f = AFTER_BURN_FUEL(f, a);
	    }
	    pl->acc.x = power * tcos(pl->dir) / inert;
	    pl->acc.y = power * tsin(pl->dir) / inert;
	    /* Decrement fuel */
	    if (do_update_this_frame)
		Add_fuel(&(pl->fuel), (long)(-f * FUEL_SCALE_FACT));
	} else {
	    pl->acc.x = pl->acc.y = 0.0;
	}

	pl->mass = pl->emptymass
		   + FUEL_MASS(pl->fuel.sum)
		   + pl->item[ITEM_ARMOR] * ARMOR_MASS;

	/* kps - ng wants if 0 here */
	/* Turn back on after implemented */
	/*#if 0*/

	if (BIT(pl->status, WARPING)) {
	    position w;
	    int wx, wy, proximity,
		nearestFront, nearestRear,
		proxFront, proxRear;

	    if (pl->wormHoleHit >= World.NumWormholes) {
		/* could happen if the player hit a temporary wormhole
		   that was removed while the player was warping */
		CLR_BIT(pl->status, WARPING);
		break;
	    }

	    if (pl->wormHoleHit != -1) {

	    if (World.wormHoles[pl->wormHoleHit].countdown > 0) {
		j = World.wormHoles[pl->wormHoleHit].lastdest;
	    } else if (rfrac() < 0.10f) {
		do
		    j = (int)(rfrac() * World.NumWormholes);
		while (World.wormHoles[j].type == WORM_IN
		       || pl->wormHoleHit == j
		       || World.wormHoles[j].temporary);
	    } else {
		nearestFront = nearestRear = -1;
		proxFront = proxRear = 10000000;

		for (j = 0; j < World.NumWormholes; j++) {
		    if (j == pl->wormHoleHit
			|| World.wormHoles[j].type == WORM_IN
			|| World.wormHoles[j].temporary)
			continue;

		    wx = (World.wormHoles[j].pos.cx -
			  World.wormHoles[pl->wormHoleHit].pos.cx) / CLICK;
		    wy = (World.wormHoles[j].pos.cy -
			  World.wormHoles[pl->wormHoleHit].pos.cy) / CLICK;
		    wx = WRAP_DX(wx);
		    wy = WRAP_DY(wy);

		    proximity = (int)(pl->vel.y * wx + pl->vel.x * wy);
		    proximity = ABS(proximity);

		    if (pl->vel.x * wx + pl->vel.y * wy < 0) {
			if (proximity < proxRear) {
			    nearestRear = j;
			    proxRear = proximity;
			}
		    } else if (proximity < proxFront) {
			nearestFront = j;
			proxFront = proximity;
		    }
		}

#define RANDOM_REAR_WORM
#ifndef RANDOM_REAR_WORM
		j = nearestFront < 0 ? nearestRear : nearestFront;
#else /* RANDOM_REAR_WORM */
		if (nearestFront >= 0) {
		    j = nearestFront;
		} else {
		    do
			j = (int)(rfrac() * World.NumWormholes);
		    while (World.wormHoles[j].type == WORM_IN
			   || j == pl->wormHoleHit);
		}
#endif /* RANDOM_REAR_WORM */
	    }

	    sound_play_sensors(pl->pos.cx, pl->pos.cy, WORM_HOLE_SOUND);

	    w.x = CLICK_TO_PIXEL(World.wormHoles[j].pos.cx);
	    w.y = CLICK_TO_PIXEL(World.wormHoles[j].pos.cy);

	    } else { /* wormHoleHit == -1 */
		int counter;
		for (counter = 20; counter > 0; counter--) {
		    w.x = (int)(rfrac() * World.width);
		    w.y = (int)(rfrac() * World.height);
		    if (BIT(1U << World.block[(int)(w.x/BLOCK_SZ)]
					     [(int)(w.y/BLOCK_SZ)],
			    SPACE_BLOCKS)) {
			break;
		    }
		}
		if (!counter) {
		    w.x = OBJ_X_IN_BLOCKS(pl);
		    w.y = OBJ_Y_IN_BLOCKS(pl);
		}
		if (counter
		    && wormTime
		    && BIT(1U << World.block[OBJ_X_IN_BLOCKS(pl)]
					    [OBJ_Y_IN_BLOCKS(pl)],
			   SPACE_BIT)
		    && BIT(1U << World.block[(int)(w.x/BLOCK_SZ)]
					    [(int)(w.y/BLOCK_SZ)],
			   SPACE_BIT)) {
		    add_temp_wormholes(OBJ_X_IN_BLOCKS(pl),
				       OBJ_Y_IN_BLOCKS(pl),
				       (int)(w.x/BLOCK_SZ),
				       (int)(w.y/BLOCK_SZ));
		}
		j = -2;
		sound_play_sensors(pl->pos.cx, pl->pos.cy, HYPERJUMP_SOUND);
	    }

	    /*
	     * Don't connect to balls while warping.
	     */
	    if (BIT(pl->used, HAS_CONNECTOR))
		pl->ball = NULL;

	    if (BIT(pl->have, HAS_BALL)) {
		/*
		 * Take every ball associated with player through worm hole.
		 * NB. the connector can cross a wall boundary this is
		 * allowed, so long as the ball itself doesn't collide.
		 */
		int k;
		for (k = 0; k < NumObjs; k++) {
		    object *b = Obj[k];
		    if (BIT(b->type, OBJ_BALL) && b->id == pl->id) {
			clpos ballpos;
			ballpos.cx = b->pos.cx
			    + (PIXEL_TO_CLICK(w.x) - pl->pos.cx);
			ballpos.cy = b->pos.cy
			    + (PIXEL_TO_CLICK(w.y) - pl->pos.cy);
			ballpos.cx = WRAP_XCLICK(ballpos.cx);
			ballpos.cy = WRAP_YCLICK(ballpos.cy);
			if (!INSIDE_MAP(ballpos.cx, ballpos.cy)) {
			    b->life = 0;
			    continue;
			}
			Object_position_set_clicks(b, ballpos.cx, ballpos.cy);
			Object_position_remember(b);
			b->vel.x *= WORM_BRAKE_FACTOR;
			b->vel.y *= WORM_BRAKE_FACTOR;
			Cell_add_object(b);
		    }
		}
	    }

	    pl->wormHoleDest = j;
	    Player_position_init_clicks(pl,
					PIXEL_TO_CLICK(w.x),
					PIXEL_TO_CLICK(w.y));
	    pl->vel.x *= WORM_BRAKE_FACTOR;
	    pl->vel.y *= WORM_BRAKE_FACTOR;
	    pl->forceVisible += 15 * TIME_FACT;

	    if ((j != pl->wormHoleHit) && (pl->wormHoleHit != -1)) {
		World.wormHoles[pl->wormHoleHit].lastdest = j;
		if (!World.wormHoles[j].temporary) {
		    World.wormHoles[pl->wormHoleHit].countdown = (wormTime ?
			wormTime : WORMCOUNT);
		}
	    }

	    CLR_BIT(pl->status, WARPING);
	    SET_BIT(pl->status, WARPED);

	    sound_play_sensors(pl->pos.cx, pl->pos.cy, WORM_HOLE_SOUND);
	}
	/* kps - ng wants an endif here */
	/*#endif*/ /* not-yet-supported warping stuff */

	update_object_speed(pl);	    /* New position */
	Move_player(i);

	if ((!BIT(pl->used, HAS_CLOAKING_DEVICE) || cloakedExhaust)
	    && !BIT(pl->used, HAS_PHASING_DEVICE)) {
	    if (BIT(pl->status, THRUSTING))
  		Thrust(i);
	}

	Compute_sensor_range(pl);

	pl->used &= pl->have;
    }

    /* kps - ng does not want this for loop */
    for (i = World.NumWormholes - 1; i >= 0; i--) {
	if (World.wormHoles[i].countdown > 0) {
	    World.wormHoles[i].countdown--;
	}
	if (World.wormHoles[i].temporary
	    && World.wormHoles[i].countdown <= 0) {
	    remove_temp_wormhole(i);
	}
    }


    for (i = 0; i < NumPlayers; i++) {
	player *pl = Players[i];

	pl->updateVisibility = 0;

	if (pl->forceVisible > 0) {
	    if ((pl->forceVisible -= timeStep) <= 0)
		pl->forceVisible = 0;

	    if (!pl->forceVisible)
		pl->updateVisibility = 1;
	}

	if (BIT(pl->used, HAS_TRACTOR_BEAM))
	    Tractor_beam(i);

	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    pl->lock.distance =
		Wrap_length(pl->pos.cx
			    - Players[GetInd[pl->lock.pl_id]]->pos.cx,
			    pl->pos.cy
			    - Players[GetInd[pl->lock.pl_id]]->pos.cy) / CLICK;
	}
    }

    /*
     * Checking for collision, updating score etc. (see collision.c)
     */
    Check_collision();


    /*
     * Update tanks, Kill players that ought to be killed.
     */
    for (i = NumPlayers - 1; i >= 0; i--) {
	player *pl = Players[i];

	if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) == PLAYING)
	    Update_tanks(&(pl->fuel));
	if (BIT(pl->status, KILLED)) {
	    Throw_items(i);

	    Detonate_items(i);

	    Kill_player(i);

	    if (IS_HUMAN_PTR(pl)) {
		if (frame_loops - pl->frame_last_busy > 60 * FPS) {
		    if ((NumPlayers - NumRobots - NumPseudoPlayers) > 1) {
			if (!game_lock && Team_zero_pausing_available()) {
			    sprintf(msg, "%s was pause-swapped because of "
				    "idling.", Players[i]->name);
			    sprintf(team_0, "team 0");
			    Handle_player_command(Players[i],team_0);
			} else {
			    Pause_player(i, 1);
			    sprintf(msg, "%s was paused for idling.",
				    Players[i]->name);
	    		}
			Set_message(msg);
		    }
		}
	    }
	}

	if (maxPauseTime > 0
	    && IS_HUMAN_PTR(pl)
	    && BIT(pl->status, PAUSE)
	    && (!(teamZeroPausing && pl->team == 0))
	    && frame_loops - pl->frame_last_busy > maxPauseTime) {
	    sprintf(msg,
		    "%s was auto-kicked for pausing too long [*Server notice*]",
		    pl->name);
	    Set_message(msg);
	    Destroy_connection(Players[i]->conn,
			       "auto-kicked: paused too long");
	}
    }

    /*
     * Kill shots that ought to be dead.
     */
    for (i = NumObjs - 1; i >= 0; i--)
	if ((Obj[i]->life -= timeStep) <= 0)
	    Delete_shot(i);

    /*
     * Compute general game status, do we have a winner?
     * (not called after Game_Over() )
     */
    if (gameDuration >= 0.0 || maxRoundTime > 0) {
	Compute_game_status();
    }

    /*
     * Now update labels if need be.
     */
    if (updateScores && frame_loops % UPDATE_SCORE_DELAY == 0)
	Update_score_table();
}

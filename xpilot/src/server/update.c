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

char update_version[] = VERSION;

int	round_delay = 0;	/* delay until start of next round */
int	round_delay_send = 0;	/* number of frames to send round_delay */
int	roundtime = -1;		/* time left this round */
static double time_to_update = 1;	/* time before less frequent updates */
static bool do_update_this_frame = false; /* less frequent update this frame */

static char msg[MSG_LEN];

static inline void update_object_speed(object *obj)
{
    world_t *world = &World;

    if (BIT(obj->status, GRAVITY)) {
	vector gravity = World_gravity(world, obj->pos);

	obj->vel.x += (obj->acc.x + gravity.x) * timeStep;
	obj->vel.y += (obj->acc.y + gravity.y) * timeStep;
    } else {
	obj->vel.x += obj->acc.x * timeStep;
	obj->vel.y += obj->acc.y * timeStep;
    }
}

static void Transport_to_home(player *pl)
{
    /*
     * Transport a corpse from the place where it died back to its homebase,
     * or if in race mode, back to the last passed check point.
     *
     * During the first part of the distance we give it a positive constant
     * acceleration G, during the second part we make this a negative one -G.
     * This results in a visually pleasing take off and landing.
     */
    clpos		startpos;
    double		dx, dy, t, m;
    const int		T = RECOVERY_DELAY;
    world_t *world = &World;

    if (pl->home_base == NULL) {
	pl->vel.x = 0;
	pl->vel.y = 0;
	return;
    }

    if (BIT(world->rules->mode, TIMING) && pl->round) {
	int check;

	if (pl->check)
	    check = pl->check - 1;
	else
	    check = world->NumChecks - 1;
	startpos = Checks(world, check)->pos;
    } else
	startpos = pl->home_base->pos;

    dx = WRAP_DCX(startpos.cx - pl->pos.cx);
    dy = WRAP_DCY(startpos.cy - pl->pos.cy);
    t = pl->count + 0.5;
    if (2 * t <= T)
	m = 2 / t;
    else {
	t = T - t;
	m = (4 * t) / (T * T - 2 * t * t);
    }
    pl->vel.x = dx * m / CLICK;
    pl->vel.y = dy * m / CLICK;
}

/*
 * Turn phasing on or off.
 */
void Phasing(player *pl, bool on)
{
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
	sound_play_sensors(pl->pos, PHASING_ON_SOUND);
    } else {
	hitmask_t hitmask = NONBALL_BIT | HITMASK(pl->team); /* kps - ok ? */
	int group;

	CLR_BIT(pl->used, HAS_PHASING_DEVICE);
	if (pl->phasing_left <= 0) {
	    if (pl->item[ITEM_PHASING] <= 0)
		CLR_BIT(pl->have, HAS_PHASING_DEVICE);
	}
	SET_BIT(pl->status, GRAVITY);
	sound_play_sensors(pl->pos, PHASING_OFF_SOUND);
	/* kps - ok to have this check here ? */
	if ((group = shape_is_inside(pl->pos.cx, pl->pos.cy, hitmask,
				     OBJ_PTR(pl), (shape_t *)pl->ship,
				     pl->dir)) != NO_GROUP)
	    /* kps - check for crashes against targets etc ??? */
	    Player_crash(pl, CrashWall, NO_IND, 0);
    }
}

/*
 * Turn cloak on or off.
 */
void Cloak(player *pl, bool on)
{
    if (on) {
	if (!BIT(pl->used, HAS_CLOAKING_DEVICE) && pl->item[ITEM_CLOAK] > 0) {
	    if (!cloakedShield) {
		if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
		    Emergency_shield(pl, false);
		if (BIT(pl->used, HAS_DEFLECTOR))
		    Deflector(pl, false);
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
		Emergency_shield(pl, true);
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
void Deflector(player *pl, bool on)
{
    if (on) {
	if (!BIT(pl->used, HAS_DEFLECTOR) && pl->item[ITEM_DEFLECTOR] > 0) {
	    /* only allow deflector when cloaked shielding or not cloaked */
	    if (cloakedShield || !BIT(pl->used, HAS_CLOAKING_DEVICE)) {
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
void Emergency_thrust(player *pl, bool on)
{
    if (on) {
	if (pl->emergency_thrust_left <= 0) {
	    pl->emergency_thrust_left = EMERGENCY_THRUST_TIME;
	    pl->item[ITEM_EMERGENCY_THRUST]--;
	}
	if (!BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	    SET_BIT(pl->used, HAS_EMERGENCY_THRUST);
	    sound_play_sensors(pl->pos, EMERGENCY_THRUST_ON_SOUND);
	}
    } else {
	if (BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	    CLR_BIT(pl->used, HAS_EMERGENCY_THRUST);
	    sound_play_sensors(pl->pos, EMERGENCY_THRUST_OFF_SOUND);
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
void Emergency_shield (player *pl, bool on)
{
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
		    sound_play_sensors(pl->pos, EMERGENCY_SHIELD_ON_SOUND);
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
	    sound_play_sensors(pl->pos, EMERGENCY_SHIELD_OFF_SOUND);
	}
    }
}

/*
 * Turn autopilot on or off.  This always clears the thrusting bit.  During
 * automatic pilot mode any changes to the current power, turnacc, turnspeed
 * and turnresistance settings will be temporary.
 */
void Autopilot(player *pl, bool on)
{
    CLR_BIT(pl->status, THRUSTING);
    if (on) {
	pl->auto_power_s = pl->power;
	pl->auto_turnspeed_s = pl->turnspeed;
	pl->auto_turnresistance_s = pl->turnresistance;
	SET_BIT(pl->used, HAS_AUTOPILOT);
	pl->power = (MIN_PLAYER_POWER+MAX_PLAYER_POWER)/2.0;
	pl->turnspeed = (MIN_PLAYER_TURNSPEED+MAX_PLAYER_TURNSPEED)/2.0;
	pl->turnresistance = 0.2;
	sound_play_sensors(pl->pos, AUTOPILOT_ON_SOUND);
    } else {
	pl->power = pl->auto_power_s;
	pl->turnacc = 0.0;
	pl->turnspeed = pl->auto_turnspeed_s;
	pl->turnresistance = pl->auto_turnresistance_s;
	CLR_BIT(pl->used, HAS_AUTOPILOT);
	sound_play_sensors(pl->pos, AUTOPILOT_OFF_SOUND);
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
    vector	gravity;
    double	acc, vel;
    double	delta;
    double	turnspeed, power;
    const double	emergency_thrust_settings_delta = 150.0 / FPS;
    const double	auto_pilot_settings_delta = 15.0 / FPS;
    const double	auto_pilot_turn_factor = 2.5;
    const double	auto_pilot_dead_velocity = 0.5;
    world_t *world = &World;

    /*
     * If the last movement touched a wall then we shouldn't
     * mess with the position (speed too?) settings.
     */
    if (pl->last_wall_touch + 1 >= frame_loops)
	return;

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
    } else
	afterburners = pl->item[ITEM_AFTERBURNER];

    gravity = World_gravity(world, pl->pos);

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
    acc = LENGTH(gravity.x, gravity.y) + vel;
    power = acc * pl->mass;
    if (afterburners)
	power /= AFTER_BURN_POWER_FACTOR(afterburners);

    /*
     * Calculate direction change needed to reduce velocity to zero.
     */
    if (vel == 0.0) {
	if (gravity.x == 0 && gravity.y == 0)
	    vad = pl->dir;
	else
	    vad = findDir(-gravity.x, -gravity.y);
    } else
	vad = findDir(-pl->vel.x, -pl->vel.y);

    vad = MOD2(vad - pl->dir, RES);
    if (vad > RES/2) {
	vad = RES - vad;
	dir = -1;
    } else
	dir = 1;

    /*
     * Calculate turnspeed needed to change direction instantaneously by
     * above direction change.
     */
    turnspeed = ((double)vad) / pl->turnresistance - pl->turnvel;
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
    } else
	pl->turnacc = dir * pl->turnspeed;

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
    if (pl->power > power)
	CLR_BIT(pl->status, THRUSTING);
    else
	SET_BIT(pl->status, THRUSTING);
}


static void Fuel_update(void)
{
    int i;
    double fuel;
    int frames_per_update;
    world_t *world = &World;

    if (NumPlayers == 0)
	return;

    fuel = (NumPlayers * STATION_REGENERATION * timeStep);
    frames_per_update = MAX_STATION_FUEL / (fuel * BLOCK_SZ);

    for (i = 0; i < world->NumFuels; i++) {
	fuel_t *fs = Fuels(world, i);

	if (fs->fuel == MAX_STATION_FUEL)
	    continue;
	if ((fs->fuel += fuel) >= MAX_STATION_FUEL)
	    fs->fuel = MAX_STATION_FUEL;
	else if (fs->last_change + frames_per_update > frame_loops)
	    /*
	     * We don't send fuelstation info to the clients every frame
	     * if it wouldn't change their display.
	     */
	    continue;

	fs->conn_mask = 0;
	fs->last_change = frame_loops;
    }
}

static void Misc_object_update(void)
{
    int i;
    object *obj;

    for (i = 0; i < NumObjs; i++) {
	obj = Obj[i];

	if (BIT(obj->type, OBJ_MINE))
	    Update_mine(MINE_PTR(obj));

	else if (BIT(obj->type, OBJ_TORPEDO))
	    Update_torpedo(TORP_PTR(obj));

	else if (BIT(obj->type, OBJ_SMART_SHOT|OBJ_HEAT_SHOT))
	    Update_missile(MISSILE_PTR(obj));

	else if (BIT(obj->type, OBJ_BALL))
	    Update_connector_force(BALL_PTR(obj));

	else if (BIT(obj->type, OBJ_WRECKAGE)) {
	    wireobject *wireobj = WIRE_PTR(obj);
	    wireobj->rotation =
		(wireobj->rotation
		 + (int) (wireobj->turnspeed * timeStep * RES)) % RES;
	}

	else if (BIT(obj->type, OBJ_PULSE)) {
	    pulseobject *pulse = PULSE_PTR(obj);
	    pulse->len += pulseSpeed * timeStep;
	    if (pulse->len > pulseLength)
		pulse->len = pulseLength;
	}

	update_object_speed(obj);

	if (!BIT(obj->type, OBJ_ASTEROID))
	    Move_object(obj);
    }
}

static void Target_update(void)
{
    int i, j;
    world_t *world = &World;

    for (i = 0; i < world->NumTargets; i++) {
	target_t *targ = Targets(world, i);

	if (targ->dead_time > 0) {
	    if ((targ->dead_time -= timeStep) <= 0) {
		Target_restore_on_map(targ);

		if (targetSync) {
		    for (j = 0; j < world->NumTargets; j++) {
			target_t *t = Targets(world, j);
			if (t->team == targ->team)
			    Target_restore_on_map(t);
		    }
		}
	    }
	    continue;
	}
	else if (targ->damage == TARGET_DAMAGE)
	    continue;

	targ->damage += TARGET_REPAIR_PER_FRAME * timeStep;
	if (targ->damage >= TARGET_DAMAGE)
	    targ->damage = TARGET_DAMAGE;
	else if (targ->last_change + TARGET_UPDATE_DELAY < frame_loops)
	    /*
	     * We don't send target info to the clients every frame
	     * if the latest repair wouldn't change their display.
	     */
	    continue;

	targ->conn_mask = 0;
	targ->last_change = frame_loops;
    }
}

static void Ecm_update(void)
{
    int i;

    for (i = 0; i < NumEcms; i++) {
	if ((Ecms[i]->size *= ecmSizeFactor) < 1.0) {
	    if (Ecms[i]->id != NO_ID)
		Player_by_id(Ecms[i]->id)->ecmcount--;
	    free(Ecms[i]);
	    --NumEcms;
	    Ecms[i] = Ecms[NumEcms];
	    i--;
	}
    }
}

static void Transporter_update(void)
{
    int i;

    for (i = 0; i < NumTransporters; i++) {
	if ((Transporters[i]->count -= timeStep) <= 0) {
	    free(Transporters[i]);
	    --NumTransporters;
	    Transporters[i] = Transporters[NumTransporters];
	    i--;
	}
    }
}

static void Players_turn(void)
{
    int i;
    player *pl;
    double new_float_dir;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);

	if (!Player_is_active(pl))
	    continue;

	/* Only do autopilot code if switched on and player is not
	 * damaged (ie. can see). */
	if (BIT(pl->used, HAS_AUTOPILOT)
	    || (BIT(pl->status, HOVERPAUSE) && !pl->damaged))
	    do_Autopilot(pl);

	pl->turnvel += pl->turnacc * timeStep;

	/*
	 * turnresistance is zero: client requests linear turning behaviour
	 * when playing with pointer control.
	 */
	if (pl->turnresistance)
	    pl->turnvel *= pl->turnresistance;

	new_float_dir = pl->float_dir;
	new_float_dir += RES * pl->turnvel / 128.0;

	while (new_float_dir < 0)
	    new_float_dir += RES;
	while (new_float_dir >= RES)
	    new_float_dir -= RES;

	Player_set_float_dir(pl, new_float_dir);

	if (!pl->turnresistance)
	    pl->turnvel = 0;

	Turn_player(pl);
    }
}

static void Use_items(player *pl)
{
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
	if ((pl->phasing_left -= timeStep) <= 0) {
	    if (pl->item[ITEM_PHASING])
		Phasing(pl, true);
	    else
		Phasing(pl, false);
	}
    }

    if (BIT(pl->used, HAS_EMERGENCY_THRUST)) {
	if (pl->fuel.sum > 0
	    && BIT(pl->status, THRUSTING)
	    && (pl->emergency_thrust_left -= timeStep) <= 0) {
	    if (pl->item[ITEM_EMERGENCY_THRUST])
		Emergency_thrust(pl, true);
	    else
		Emergency_thrust(pl, false);
	}
    }

    if (BIT(pl->used, HAS_EMERGENCY_SHIELD)) {
	if (pl->fuel.sum > 0
	    && BIT(pl->used, HAS_SHIELD)
	    && ((pl->emergency_shield_left -= timeStep) <= 0)) {
	    if (pl->item[ITEM_EMERGENCY_SHIELD])
		Emergency_shield(pl, true);
	    else
		Emergency_shield(pl, false);
	}
    }

    if (do_update_this_frame && BIT(pl->used, HAS_DEFLECTOR))
	Do_deflector(pl);	/* !@# no real need for do_update_this_frame */

    /*
     * Compute energy drainage
     */
    if (do_update_this_frame) {
	if (BIT(pl->used, HAS_SHIELD))
	    Player_add_fuel(pl, ED_SHIELD);

	if (BIT(pl->used, HAS_PHASING_DEVICE))
	    Player_add_fuel(pl, ED_PHASING_DEVICE);

	if (BIT(pl->used, HAS_CLOAKING_DEVICE))
	    Player_add_fuel(pl, ED_CLOAKING_DEVICE);

	if (BIT(pl->used, HAS_DEFLECTOR))
	    Player_add_fuel(pl, ED_DEFLECTOR);
    }
}

/*
 * Player is refueling.
 */
static void Do_refuel(player *pl)
{
    world_t *world = &World;
    fuel_t *fs = Fuels(world, pl->fs);

    if ((Wrap_length(pl->pos.cx - fs->pos.cx,
		     pl->pos.cy - fs->pos.cy)
	 > 90.0 * CLICK)
	|| (pl->fuel.sum >= pl->fuel.max)
	|| BIT(pl->used, HAS_PHASING_DEVICE)
	|| (BIT(world->rules->mode, TEAM_PLAY)
	    && teamFuel
	    && fs->team != pl->team)) {
	CLR_BIT(pl->used, HAS_REFUEL);
    } else {
	int n = pl->fuel.num_tanks;
	int ct = pl->fuel.current;

	do {
	    if (fs->fuel > REFUEL_RATE * timeStep) {
		fs->fuel -= REFUEL_RATE * timeStep;
		fs->conn_mask = 0;
		fs->last_change = frame_loops;
		Player_add_fuel(pl, REFUEL_RATE * timeStep);
	    } else {
		Player_add_fuel(pl, fs->fuel);
		fs->fuel = 0;
		fs->conn_mask = 0;
		fs->last_change = frame_loops;
		CLR_BIT(pl->used, HAS_REFUEL);
		break;
	    }
	    if (pl->fuel.current == pl->fuel.num_tanks)
		pl->fuel.current = 0;
	    else
		pl->fuel.current += 1;
	} while (n--);
	pl->fuel.current = ct;
    }
}

/*
 * Player is repairing a target.
 */
static void Do_repair(player *pl)
{
    world_t *world = &World;
    target_t *targ = Targets(world, pl->repair_target);

    if ((Wrap_length(pl->pos.cx - targ->pos.cx,
		     pl->pos.cy - targ->pos.cy) > 90.0 * CLICK)
	|| targ->damage >= TARGET_DAMAGE
	|| targ->dead_time > 0
	|| BIT(pl->used, HAS_PHASING_DEVICE))
	CLR_BIT(pl->used, HAS_REPAIR);
    else {
	int n = pl->fuel.num_tanks;
	int ct = pl->fuel.current;

	do {
	    if (pl->fuel.tank[pl->fuel.current]
		> REFUEL_RATE * timeStep) {
		targ->damage += TARGET_FUEL_REPAIR_PER_FRAME;
		targ->conn_mask = 0;
		targ->last_change = frame_loops;
		Player_add_fuel(pl, -REFUEL_RATE * timeStep);
		if (targ->damage > TARGET_DAMAGE) {
		    targ->damage = TARGET_DAMAGE;
		    break;
		}
	    } else
		CLR_BIT(pl->used, HAS_REPAIR);

	    if (pl->fuel.current == pl->fuel.num_tanks)
		pl->fuel.current = 0;
	    else
		pl->fuel.current += 1;
	} while (n--);
	pl->fuel.current = ct;
    }
}

/*
 * Player is warping.
 */
static void Do_warping(player *pl)
{
    clpos dest;
    int wh_dest, wcx, wcy, nearestFront, nearestRear;
    double proximity, proxFront, proxRear;
    world_t *world = &World;

    if (pl->wormHoleHit != -1) {
	wormhole_t *wh_hit = Wormholes(world, pl->wormHoleHit);

	if (wh_hit->countdown > 0)
	    wh_dest = wh_hit->lastdest;
	else if (rfrac() < 0.1) {
	    do
		wh_dest = (int)(rfrac() * world->NumWormholes);
	    while (world->wormholes[wh_dest].type == WORM_IN
		   || pl->wormHoleHit == wh_dest
		   || world->wormholes[wh_dest].temporary);
	} else {
	    nearestFront = nearestRear = -1;
	    proxFront = proxRear = 1e20;

	    for (wh_dest = 0; wh_dest < world->NumWormholes; wh_dest++) {
		wormhole_t *wh = Wormholes(world, wh_dest);

		if (wh_dest == pl->wormHoleHit
		    || wh->type == WORM_IN
		    || wh->temporary)
		    continue;

		wcx = WRAP_DCX(wh->pos.cx - wh_hit->pos.cx);
		wcy = WRAP_DCY(wh->pos.cy - wh_hit->pos.cy);

		proximity = (pl->vel.y * wcx + pl->vel.x * wcy);
		proximity = ABS(proximity);

		if (pl->vel.x * wcx + pl->vel.y * wcy < 0) {
		    if (proximity < proxRear) {
			nearestRear = wh_dest;
			proxRear = proximity;
		    }
		} else if (proximity < proxFront) {
		    nearestFront = wh_dest;
		    proxFront = proximity;
		}
	    }

#define RANDOM_REAR_WORM 1

	    if (! RANDOM_REAR_WORM)
		wh_dest = nearestFront < 0 ? nearestRear : nearestFront;
	    else {
		if (nearestFront >= 0)
		    wh_dest = nearestFront;
		else {
		    do
			wh_dest = (int)(rfrac() * world->NumWormholes);
		    while (world->wormholes[wh_dest].type == WORM_IN
			   || wh_dest == pl->wormHoleHit);
		}
	    }
	}

	sound_play_sensors(pl->pos, WORM_HOLE_SOUND);
	dest = world->wormholes[wh_dest].pos;

    } else { /* wormHoleHit == -1 */
	int counter;
	hitmask_t hitmask = NONBALL_BIT | HITMASK(pl->team); /* kps - ok ? */

	/* try to find empty space to hyperjump to */
	for (counter = 20; counter > 0; counter--) {
	    dest.cx = (int)(rfrac() * world->cwidth);
	    dest.cy = (int)(rfrac() * world->cheight);
	    if (shape_is_inside(dest.cx, dest.cy, hitmask,
				(object *)pl, (shape_t *)pl->ship,
				pl->dir)
		== NO_GROUP)
		break;
	}

	/* can't find an empty space, hyperjump failed */
	if (!counter)
	    dest = pl->pos;

#if 0 /* kps - temporary wormholes disabled currently */
	if (counter
	    && wormTime
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
	/* hack */
	wh_dest = -2;
	sound_play_sensors(pl->pos, HYPERJUMP_SOUND);
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
		hitmask_t hitmask = BALL_BIT|HITMASK(pl->team);

		ballpos.cx = b->pos.cx + dest.cx - pl->pos.cx;
		ballpos.cy = b->pos.cy + dest.cy - pl->pos.cy;
		ballpos.cx = WRAP_XCLICK(ballpos.cx);
		ballpos.cy = WRAP_YCLICK(ballpos.cy);
		if (!World_contains_clpos(world, ballpos)) {
		    b->life = 0.0;
		    continue;
		}
		if (shape_is_inside(ballpos.cx, ballpos.cy, hitmask,
				    (object *)b, &ball_wire, 0) != NO_GROUP) {
		    b->life = 0.0;
		    continue;
		}
		Object_position_set_clpos(b, ballpos);
		Object_position_remember(b);
		b->vel.x *= WORM_BRAKE_FACTOR;
		b->vel.y *= WORM_BRAKE_FACTOR;
		Cell_add_object(b);
	    }
	}
    }

    pl->wormHoleDest = wh_dest;
    Player_position_init_clpos(pl, dest);
    pl->vel.x *= WORM_BRAKE_FACTOR;
    pl->vel.y *= WORM_BRAKE_FACTOR;
    pl->forceVisible += 15;

    if ((wh_dest != pl->wormHoleHit) && (pl->wormHoleHit != -1)) {
	world->wormholes[pl->wormHoleHit].lastdest = wh_dest;
	if (!world->wormholes[wh_dest].temporary)
	    world->wormholes[pl->wormHoleHit].countdown
		= (wormTime ? wormTime : WORMCOUNT);
    }

    CLR_BIT(pl->status, WARPING);
    SET_BIT(pl->status, WARPED);

    sound_play_sensors(pl->pos, WORM_HOLE_SOUND);
}


/* * * * * *
 *
 * Player loop. Computes miscellaneous updates.
 *
 */
static void Update_players(void)
{
    int i, j;
    player *pl;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);

	if (BIT(pl->status, PAUSE)) {
	    if (pauseTax > 0.0 && (frame_loops % FPS) == 0) {
		pl->score -= pauseTax;
		updateScores = true;
	    }
	}

	if ((pl->damaged -= timeStep) <= 0)
	    pl->damaged = 0;

	/* kps - fix these */
	if (pl->flooding > FPS + 1) {
	    sprintf(msg, "%s was kicked out because of flooding.", pl->name);
	    Destroy_connection(pl->conn, "flooding");
	    i--;
	    continue;
	} else if ( pl->flooding >= 0 )
	    pl->flooding--;

#define IDLETHRESHOLD (FPS * 60)

	if (Player_is_human(pl)) {
	    pl->rank->score = pl->score;
	    if ( pl->mychar == ' ' ) {
		if ( pl->idleCount++ == IDLETHRESHOLD ) {
		    if ( NumPlayers - 1 > NumPseudoPlayers + NumRobots ) {
			/* Kill player, he/she will be paused when returned
			   to base, unless he/she wakes up. */
			Kill_player(pl, false);
		    } else
			pl->idleCount = 0;
		}
	    }
	}

	if (pl->count >= 0) {
	    pl->count -= timeStep;
	    if (pl->count > 0) {
		if (!BIT(pl->status, PLAYING)) {
		    Transport_to_home(pl);
		    Move_player(pl);
		    continue;
		}
	    } else {
		pl->count = -1;
		if (!BIT(pl->status, PLAYING)) {
		    /*
		     * kps - another idle check comes later in this file.
		     * Are both needed ?
		     */
		    if (pl->idleCount >= IDLETHRESHOLD) { /* idle */
			Pause_player(pl, true);
			sprintf(msg, "%s was paused for idling.", pl->name);
			Set_message(msg);
			continue;
		    }

		    SET_BIT(pl->status, PLAYING);
		    Go_home(pl);
		}
		if (BIT(pl->status, SELF_DESTRUCT)) {
		    if (selfDestructScoreMult != 0) {
			double sc = Rate(0.0, pl->score)
			    * selfDestructScoreMult;
			Score(pl, -sc, pl->pos, "Self-Destruct");
		    }
		    SET_BIT(pl->status, KILLED);
		    sprintf(msg, "%s has committed suicide.", pl->name);
		    Set_message(msg);
		    Throw_items(pl);
		    Kill_player(pl, true);
		    updateScores = true;
		}
	    }
	}

	if (!Player_is_active(pl))
	    continue;

	if (round_delay > 0)
	    continue;

	Use_items(pl);

#define UPDATE_RATE 100

	for (j = 0; j < NumPlayers; j++) {
	    player *pl_j = Players(j);

	    if (pl->forceVisible > 0)
		pl_j->visibility[i].canSee = 1;

	    if (i == j || !BIT(pl_j->used, HAS_CLOAKING_DEVICE))
		pl->visibility[j].canSee = 1;
	    else if (pl->updateVisibility
		     || pl_j->updateVisibility
		     || (int)(rfrac() * UPDATE_RATE)
		     < ABS(frame_loops - pl->visibility[j].lastChange)) {

		pl->visibility[j].lastChange = frame_loops;
		pl->visibility[j].canSee
		    = (rfrac() * (pl->item[ITEM_SENSOR] + 1))
		    > (rfrac() * (pl_j->item[ITEM_CLOAK] + 1));
	    }
	}

	if (BIT(pl->used, HAS_REFUEL))
	    Do_refuel(pl);

	if (BIT(pl->used, HAS_REPAIR))
	    Do_repair(pl);

	if (pl->fuel.sum <= 0) {
	    CLR_BIT(pl->used, HAS_SHIELD|HAS_CLOAKING_DEVICE|HAS_DEFLECTOR);
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (pl->fuel.sum > (pl->fuel.max - REFUEL_RATE * timeStep))
	    CLR_BIT(pl->used, HAS_REFUEL);

	/*
	 * Update acceleration vector etc.
	 */
	if (BIT(pl->status, THRUSTING)) {
	    double power = pl->power;
	    double f = pl->power * 0.0008;	/* 1/(FUEL_SCALE*MIN_POWER) */
	    int a = (BIT(pl->used, HAS_EMERGENCY_THRUST)
		     ? MAX_AFTERBURNER
		     : pl->item[ITEM_AFTERBURNER]);
	    double inert = pl->mass;

	    if (a) {
		power = AFTER_BURN_POWER(power, a);
		f = AFTER_BURN_FUEL(f, a);
	    }

	    if (!ngControls) {
		pl->acc.x = power * tcos(pl->dir) / inert;
		pl->acc.y = power * tsin(pl->dir) / inert;
	    } else {
		pl->acc.x = power * pl->float_dir_cos / inert;
		pl->acc.y = power * pl->float_dir_sin / inert;
	    }

	    /* Decrement fuel */
	    if (do_update_this_frame)
		Player_add_fuel(pl, -f);
	} else
	    pl->acc.x = pl->acc.y = 0.0;

	Player_set_mass(pl);

	/* Wormholes and warping */
	if (BIT(pl->status, WARPING))
	    Do_warping(pl);

	update_object_speed(OBJ_PTR(pl));
	Move_player(pl);

	if ((!BIT(pl->used, HAS_CLOAKING_DEVICE) || cloakedExhaust)
	    && !BIT(pl->used, HAS_PHASING_DEVICE)) {
	    if (BIT(pl->status, THRUSTING))
  		Thrust(pl);
	}

	Compute_sensor_range(pl);

	pl->used &= pl->have;
    }
}

/********** **********
 * Updating objects and the like.
 */
void Update_objects(void)
{
    int i;
    player *pl;
    world_t *world = &World;

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
	time_to_update += 1;
    }

    Robot_update();

    /*
     * Fast aim:
     * When calculating a frame, turn the ship before firing.
     * This means you can change aim one frame faster.
     */
    Players_turn();

    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);

	if (pl->stunned > 0) {
	    pl->stunned -= timeStep;
	    if (pl->stunned <= 0)
		pl->stunned = 0;
	    CLR_BIT(pl->used, HAS_SHIELD|HAS_LASER|HAS_SHOT);
	    pl->did_shoot = false;
	    CLR_BIT(pl->status, THRUSTING);
	}
	if (BIT(pl->used, HAS_SHOT) || pl->did_shoot)
	    Fire_normal_shots(pl);
	if (BIT(pl->used, HAS_LASER)) {
	    if (pl->item[ITEM_LASER] <= 0 || BIT(pl->used, HAS_PHASING_DEVICE))
		CLR_BIT(pl->used, HAS_LASER);
	    else
		Fire_laser(pl);
	}
	pl->did_shoot = false;
    }

    /*
     * Special items.
     */
    if (do_update_this_frame) {
	for (i = 0; i < NUM_ITEMS; i++)
	    if (world->items[i].num < world->items[i].max
		&& world->items[i].chance > 0
		&& (rfrac() * world->items[i].chance) < 1.0f)
		Place_item(NULL, i);
    }

    Fuel_update();
    Misc_object_update();
    Asteroid_update();
    Ecm_update();
    Transporter_update();
    Cannon_update(do_update_this_frame);
    Target_update();
    Update_players();

    for (i = world->NumWormholes - 1; i >= 0; i--) {
	wormhole_t *wh = Wormholes(world, i);

	if ((wh->countdown -= timeStep) <= 0)
	    wh->countdown = 0;

	if (wh->temporary && wh->countdown <= 0)
	    remove_temp_wormhole(world, i);
    }


    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);

	pl->updateVisibility = 0;

	if (pl->forceVisible > 0) {
	    if ((pl->forceVisible -= timeStep) <= 0)
		pl->forceVisible = 0;

	    if (!pl->forceVisible)
		pl->updateVisibility = 1;
	}

	if (BIT(pl->used, HAS_TRACTOR_BEAM))
	    Tractor_beam(pl);

	if (BIT(pl->lock.tagged, LOCK_PLAYER)) {
	    player *lpl = Player_by_id(pl->lock.pl_id);

	    pl->lock.distance =
		Wrap_length(pl->pos.cx - lpl->pos.cx,
			    pl->pos.cy - lpl->pos.cy) / CLICK;
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
	pl = Players(i);

	if (Player_is_playing(pl))
	    Update_tanks(&(pl->fuel));
	if (BIT(pl->status, KILLED)) {
	    Throw_items(pl);

	    Detonate_items(pl);

	    Kill_player(pl, true);

	    if (Player_is_human(pl)) {
		if (frame_loops - pl->frame_last_busy > 60 * FPS) {
		    if ((NumPlayers - NumRobots - NumPseudoPlayers) > 1) {
			Pause_player(pl, true);
			sprintf(msg, "%s was paused for idling.", pl->name);
			Set_message(msg);
		    }
		}
	    }
	}

	if (maxPauseTime > 0
	    && Player_is_human(pl)
	    && BIT(pl->status, PAUSE)
	    && frame_loops - pl->frame_last_busy > maxPauseTime) {
	    sprintf(msg, "%s was auto-kicked for pausing too long "
		    "[*Server notice*]", pl->name);
	    Set_message(msg);
	    Destroy_connection(pl->conn, "auto-kicked: paused too long");
	}
    }

    /*
     * Kill shots that ought to be dead.
     */
    for (i = NumObjs - 1; i >= 0; i--)
	if ((Obj[i]->life -= timeStep) <= 0)
	    Delete_shot(i);

     /*
      * In tag games, check if anyone is tagged. otherwise, tag someone.
      */
    if (tagGame) {
	int oldtag = tagItPlayerId;

	Check_tag();
	if (tagItPlayerId != oldtag && tagItPlayerId != NO_ID) {
	    sprintf(msg, " < %s is 'it' now. >",
		    Player_by_id(tagItPlayerId)->name);
	    Set_message(msg);
	}
    }

    /*
     * Compute general game status, do we have a winner?
     * (not called after Game_Over() )
     */
    if (gameDuration >= 0.0 || maxRoundTime > 0)
	Compute_game_status();

    /*
     * Now update labels if need be.
     */
    if (updateScores && frame_loops % UPDATE_SCORE_DELAY == 0)
	Update_score_table();
}


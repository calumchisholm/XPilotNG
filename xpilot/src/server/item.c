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

char item_version[] = VERSION;


static void Item_update_flags(player *pl)
{
    if (pl->item[ITEM_CLOAK] <= 0
	&& BIT(pl->have, HAS_CLOAKING_DEVICE)) {
	CLR_BIT(pl->have, HAS_CLOAKING_DEVICE);
	pl->updateVisibility = 1;
    }
    if (pl->item[ITEM_MIRROR] <= 0)
	CLR_BIT(pl->have, HAS_MIRROR);
    if (pl->item[ITEM_DEFLECTOR] <= 0)
	CLR_BIT(pl->have, HAS_DEFLECTOR);
    if (pl->item[ITEM_AFTERBURNER] <= 0)
	CLR_BIT(pl->have, HAS_AFTERBURNER);
    if (pl->item[ITEM_PHASING] <= 0
	&& !BIT(pl->used, HAS_PHASING_DEVICE)
	&& pl->phasing_left <= 0)
	CLR_BIT(pl->have, HAS_PHASING_DEVICE);
    if (pl->item[ITEM_EMERGENCY_THRUST] <= 0
	&& !BIT(pl->used, HAS_EMERGENCY_THRUST)
	&& pl->emergency_thrust_left <= 0)
	CLR_BIT(pl->have, HAS_EMERGENCY_THRUST);
    if (pl->item[ITEM_EMERGENCY_SHIELD] <= 0
	&& !BIT(pl->used, HAS_EMERGENCY_SHIELD)
	&& pl->emergency_shield_left <= 0) {
	if (BIT(pl->have, HAS_EMERGENCY_SHIELD)) {
	    CLR_BIT(pl->have, HAS_EMERGENCY_SHIELD);
	    if (!BIT(DEF_HAVE, HAS_SHIELD) && pl->shield_time <= 0) {
		CLR_BIT(pl->have, HAS_SHIELD);
		CLR_BIT(pl->used, HAS_SHIELD);
	    }
	}
    }
    if (pl->item[ITEM_TRACTOR_BEAM] <= 0)
	CLR_BIT(pl->have, HAS_TRACTOR_BEAM);
    if (pl->item[ITEM_AUTOPILOT] <= 0) {
	if (BIT(pl->used, HAS_AUTOPILOT))
	    Autopilot(pl, false);
	CLR_BIT(pl->have, HAS_AUTOPILOT);
    }
    if (pl->item[ITEM_ARMOR] <= 0)
	CLR_BIT(pl->have, HAS_ARMOR);
}

/*
 * Player loses some items after some event (collision, bounce).
 * The `prob' parameter gives the chance that items are lost
 * and, if they are lost, what percentage.
 */
void Item_damage(player *pl, double prob)
{
    if (prob < 1.0f) {
	int		i;
	double		loss;

	loss = prob;
	LIMIT(loss, 0.0f, 1.0f);

	for (i = 0; i < NUM_ITEMS; i++) {
	    if (!BIT(1U << i, ITEM_BIT_FUEL|ITEM_BIT_TANK)) {
		if (pl->item[i]) {
		    double f = rfrac();
		    if (f < loss)
			pl->item[i] = (int)(pl->item[i] * loss + 0.5f);
		}
	    }
	}

	Item_update_flags(pl);
    }
}

int Choose_random_item(void)
{
    int		i;
    double	item_prob_sum = 0;
    world_t *world = &World;

    for (i = 0; i < NUM_ITEMS; i++)
	item_prob_sum += world->items[i].prob;

    if (item_prob_sum > 0.0) {
	double sum = item_prob_sum * rfrac();

	for (i = 0; i < NUM_ITEMS; i++) {
	    sum -= world->items[i].prob;
	    if (sum <= 0)
		break;
	}
	if (i >= NUM_ITEMS)
	    i = ITEM_FUEL;
    }

    return i;
}

void Place_item(player *pl, int item)
{
    int			num_lose, num_per_pack, place_count, dist;
    long		grav, rand_item;
    clpos		pos;
    vector		vel;
    item_concentrator_t	*con;
    world_t *world = &World;

    if (NumObjs >= MAX_TOTAL_SHOTS) {
	if (pl && !BIT(pl->status, KILLED))
	    pl->item[item]--;
	return;
    }

    if (pl) {
	if (BIT(pl->status, KILLED)) {
	    num_lose = pl->item[item] - world->items[item].initial;
	    if (num_lose <= 0)
		return;
	    pl->item[item] -= num_lose;
	    num_per_pack = (int)(num_lose * dropItemOnKillProb);
	    if (num_per_pack < world->items[item].min_per_pack)
		return;
	}
	else {
	    num_lose = pl->item[item];
	    if (num_lose <= 0)
		return;
	    if (world->items[item].min_per_pack
		== world->items[item].max_per_pack)
		num_per_pack = world->items[item].max_per_pack;
	    else
		num_per_pack = world->items[item].min_per_pack
		    + (int)(rfrac() * (1 + world->items[item].max_per_pack
				       - world->items[item].min_per_pack));
	    if (num_per_pack > num_lose)
		num_per_pack = num_lose;
	    else
		num_lose = num_per_pack;
	    pl->item[item] -= num_lose;
	}
    } else {
	if (world->items[item].min_per_pack == world->items[item].max_per_pack)
	    num_per_pack = world->items[item].max_per_pack;
	else
	    num_per_pack = world->items[item].min_per_pack
		+ (int)(rfrac() * (1 + world->items[item].max_per_pack
				   - world->items[item].min_per_pack));
    }

    if (pl) {
	grav = GRAVITY;
	rand_item = 0;
	pos = pl->prevpos;
	if (!BIT(pl->status, KILLED)) {
	    /*
	     * Player is dropping an item on purpose.
	     * Give the item some offset so that the
	     * player won't immediately pick it up again.
	     */
	    if (pl->vel.x >= 0)
		pos.cx -= (BLOCK_CLICKS + (int)(rfrac() * 8 * CLICK));
	    else
		pos.cx += (BLOCK_CLICKS + (int)(rfrac() * 8 * CLICK));
	    if (pl->vel.y >= 0)
		pos.cy -= (BLOCK_CLICKS + (int)(rfrac() * 8 * CLICK));
	    else
		pos.cy += (BLOCK_CLICKS + (int)(rfrac() * 8 * CLICK));
	}
	pos.cx = WRAP_XCLICK(pos.cx);
	pos.cy = WRAP_YCLICK(pos.cy);
	if (!INSIDE_MAP(pos.cx, pos.cy))
	    return;
	/*if (!BIT(1U << world->block[bx][by], SPACE_BLOCKS))*/
	if (is_inside(pos.cx, pos.cy, NOTEAM_BIT | NONBALL_BIT, NULL)
	    != NO_GROUP)
	    return;

    } else {
	if (rfrac() < movingItemProb)
	    grav = GRAVITY;
	else
	    grav = 0;

	if (rfrac() < randomItemProb)
	    rand_item = RANDOM_ITEM;
	else
	    rand_item = 0;

	if (world->NumItemConcs > 0 && rfrac() < itemConcentratorProb)
	    con = ItemConcs(world, (int)(rfrac() * world->NumItemConcs));
	else
	    con = NULL;
	/*
	 * kps - write a generic function that can be used here and
	 * with asteroids.
	 */
	/*
	 * This will take very long (or forever) with maps
	 * that hardly have any (or none) spaces.
	 * So bail out after a few retries.
	 */
	for (place_count = 0; ; place_count++) {
	    if (place_count >= 8)
		return;

	    if (con) {
		int dir = (int)(rfrac() * RES);

		dist = (int)(rfrac() * ((itemConcentratorRadius
					 * BLOCK_CLICKS) + 1));
		pos.cx = con->pos.cx + dist * tcos(dir);
		pos.cy = con->pos.cy + dist * tsin(dir);
		pos.cx = WRAP_XCLICK(pos.cx);
		pos.cy = WRAP_YCLICK(pos.cy);
		if (!INSIDE_MAP(pos.cx, pos.cy))
		    continue;
	    } else {
		pos.cx = (int)(rfrac() * world->cwidth);
		pos.cy = (int)(rfrac() * world->cheight);
	    }

	    /*if (BIT(1U << world->block[bx][by], SPACE_BLOCKS|CANNON_BIT))*/
	    if (is_inside(pos.cx, pos.cy, NOTEAM_BIT | NONBALL_BIT, NULL)
		== NO_GROUP)
		break;
	}
    }
    vel.x = vel.y = 0;
    if (grav) {
	if (pl) {
	    vel.x += pl->vel.x;
	    vel.y += pl->vel.y;
	    if (!BIT(pl->status, KILLED)) {
		double vl = LENGTH(vel.x, vel.y);
		int dvx = (int)(rfrac() * 8);
		int dvy = (int)(rfrac() * 8);
		const float drop_speed_factor = 0.75f;

		vel.x *= drop_speed_factor;
		vel.y *= drop_speed_factor;
		if (vl < 1.0f) {
		    vel.x -= (pl->vel.x >= 0) ? dvx : -dvx;
		    vel.y -= (pl->vel.y >= 0) ? dvy : -dvy;
		} else {
		    vel.x -= dvx * (vel.x / vl);
		    vel.y -= dvy * (vel.y / vl);
		}
	    } else {
		double v = rfrac() * 6;
		int dir = (int)(rfrac() * RES);

		vel.x += tcos(dir) * v;
		vel.y += tsin(dir) * v;
	    }
	} else {
	    vector gravity = World_gravity(world, pos);

	    vel.x -= Gravity * gravity.x;
	    vel.y -= Gravity * gravity.y;
	    vel.x += (int)(rfrac() * 8) - 3;
	    vel.y += (int)(rfrac() * 8) - 3;
	}
    }

    Make_item(pos, vel, item, num_per_pack, grav | rand_item);
}

void Make_item(clpos pos, vector vel,
	       int item, int num_per_pack,
	       long status)
{
    object *obj;
    world_t *world = &World;

    if (!INSIDE_MAP(pos.cx, pos.cy))
	return;

    if (world->items[item].num >= world->items[item].max)
	return;

    if ((obj = Object_allocate()) == NULL)
	return;

    obj->type = OBJ_ITEM;
    obj->info = item;
    obj->color = RED;
    obj->status = status;
    obj->id = NO_ID;
    obj->team = TEAM_NOT_SET;
    Object_position_init_clpos(obj, pos);
    obj->vel = vel;
    obj->acc.x =
    obj->acc.y = 0.0;
    obj->mass = 10.0;
    obj->life = 1500 + rfrac() * 512;
    obj->count = num_per_pack;
    obj->pl_range = ITEM_SIZE/2;
    obj->pl_radius = ITEM_SIZE/2;

    world->items[item].num++;
    Cell_add_object(obj);
}

void Throw_items(player *pl)
{
    int			num_items_to_throw, remain, item;
    world_t *world = &World;

    if (!dropItemOnKillProb || !pl)
	return;

    for (item = 0; item < NUM_ITEMS; item++) {
	if (!BIT(1U << item, ITEM_BIT_FUEL | ITEM_BIT_TANK)) {
	    do {
		num_items_to_throw
		    = pl->item[item] - world->items[item].initial;
		if (num_items_to_throw <= 0)
		    break;
		Place_item(pl, item);
		remain = pl->item[item] - world->items[item].initial;
	    } while (remain > 0 && remain < num_items_to_throw);
	}
    }

    Item_update_flags(pl);
}

/*
 * Cause some remaining mines or missiles to be launched in
 * a random direction with a small life time (ie. magazine has
 * gone off).
 */
void Detonate_items(player *pl)
{
    player		*owner_pl;
    int			i;
    modifiers		mods;
    world_t *world = &World;

    if (!BIT(pl->status, KILLED))
	return;

    /* ZE: Detonated items on tanks should belong to the tank's owner. */
    if (Player_is_tank(pl))
	owner_pl = Player_by_id(pl->lock.pl_id);
    else
	owner_pl = pl;

    /*
     * These are always immune to detonation.
     */
    if ((pl->item[ITEM_MINE] -= world->items[ITEM_MINE].initial) < 0)
	pl->item[ITEM_MINE] = 0;
    if ((pl->item[ITEM_MISSILE] -= world->items[ITEM_MISSILE].initial) < 0)
	pl->item[ITEM_MISSILE] = 0;

    /*
     * Drop shields in order to launch mines and missiles.
     */
    CLR_BIT(pl->used, HAS_SHIELD);

    /*
     * Mines are always affected by gravity and are sent in random directions
     * slowly out from the ship (velocity relative).
     */
    for (i = 0; i < pl->item[ITEM_MINE]; i++) {
	if (rfrac() < detonateItemOnKillProb) {
	    int dir = (int)(rfrac() * RES);
	    double speed = rfrac() * 4.0f;
	    vector vel;

	    mods = pl->mods;
	    if (BIT(mods.nuclear, NUCLEAR)
		&& pl->item[ITEM_MINE] < nukeMinMines)
		CLR_BIT(mods.nuclear, NUCLEAR);

	    vel.x = pl->vel.x + speed * tcos(dir);
	    vel.y = pl->vel.y + speed * tsin(dir);
	    Place_general_mine(owner_pl, pl->team, GRAVITY,
			       pl->pos, vel, mods);
	}
    }
    for (i = 0; i < pl->item[ITEM_MISSILE]; i++) {
	if (rfrac() < detonateItemOnKillProb) {
	    int	type;

	    if (pl->shots >= ShotsMax)
		break;

	    /*
	     * Missiles are random type at random players, which could
	     * mean a misfire.
	     */
	    SET_BIT(pl->lock.tagged, LOCK_PLAYER);
	    pl->lock.pl_id = Players((int)(rfrac() * NumPlayers))->id;

	    switch ((int)(rfrac() * 3)) {
	    case 0:	type = OBJ_TORPEDO;	break;
	    case 1:	type = OBJ_HEAT_SHOT;	break;
	    default:	type = OBJ_SMART_SHOT;	break;
	    }

	    mods = pl->mods;
	    if (BIT(mods.nuclear, NUCLEAR)
		&& pl->item[ITEM_MISSILE] < nukeMinSmarts)
		CLR_BIT(mods.nuclear, NUCLEAR);

	    Fire_general_shot(owner_pl, 0, pl->team, pl->pos,
			      type, (int)(rfrac() * RES), mods, NO_ID);
	}
    }
}

void Tractor_beam(player *pl)
{
    double	maxdist, percent, cost;
    player	*locked_pl = Player_by_id(pl->lock.pl_id);

    maxdist = TRACTOR_MAX_RANGE(pl->item[ITEM_TRACTOR_BEAM]);
    if (BIT(pl->lock.tagged, LOCK_PLAYER|LOCK_VISIBLE)
	!= (LOCK_PLAYER|LOCK_VISIBLE)
	|| !Player_is_playing(locked_pl)
	|| pl->lock.distance >= maxdist
	|| BIT(pl->used, HAS_PHASING_DEVICE)
	|| BIT(locked_pl->used, HAS_PHASING_DEVICE)) {
	CLR_BIT(pl->used, HAS_TRACTOR_BEAM);
	return;
    }
    percent = TRACTOR_PERCENT(pl->lock.distance, maxdist);
    cost = TRACTOR_COST(percent);
    if (pl->fuel.sum < -cost) {
	CLR_BIT(pl->used, HAS_TRACTOR_BEAM);
	return;
    }
    General_tractor_beam(pl, pl->pos, pl->item[ITEM_TRACTOR_BEAM],
			 locked_pl, pl->tractor_is_pressor);
}

void General_tractor_beam(player *pl, clpos pos,
			  int items, player *victim, bool pressor)
{
    double	maxdist = TRACTOR_MAX_RANGE(items),
		maxforce = TRACTOR_MAX_FORCE(items),
		percent, force, dist, cost;
    int		theta;

    dist = Wrap_length(pos.cx - victim->pos.cx,
		       pos.cy - victim->pos.cy) / CLICK;
    if (dist > maxdist)
	return;
    percent = TRACTOR_PERCENT(dist, maxdist);
    cost = TRACTOR_COST(percent);
    force = TRACTOR_FORCE(pressor, percent, maxforce);

    sound_play_sensors(pos, pressor ? PRESSOR_BEAM_SOUND : TRACTOR_BEAM_SOUND);

    if (pl)
	Player_add_fuel(pl, cost);

    theta = Wrap_cfindDir(pos.cx - victim->pos.cx, pos.cy - victim->pos.cy);

    if (pl) {
	pl->vel.x += tcos(theta) * (force / pl->mass);
	pl->vel.y += tsin(theta) * (force / pl->mass);
	Record_shove(pl, victim, frame_loops);
	Record_shove(victim, pl, frame_loops);
    }
    victim->vel.x -= tcos(theta) * (force / victim->mass);
    victim->vel.y -= tsin(theta) * (force / victim->mass);
}


void Do_deflector(player *pl)
{
    double	range = (pl->item[ITEM_DEFLECTOR] * 0.5 + 1) * BLOCK_CLICKS;
    double	maxforce = pl->item[ITEM_DEFLECTOR] * 0.2;
    object	*obj, **obj_list;
    int		i, obj_count;
    double	dx, dy, dist;

    if (pl->fuel.sum < -ED_DEFLECTOR) {
	if (BIT(pl->used, HAS_DEFLECTOR))
	    Deflector(pl, false);
	return;
    }
    Player_add_fuel(pl, ED_DEFLECTOR);

    Cell_get_objects(OBJ_X_IN_BLOCKS(pl), OBJ_Y_IN_BLOCKS(pl),
		     (int)(range / BLOCK_CLICKS + 1), 200,
		     &obj_list, &obj_count);

    for (i = 0; i < obj_count; i++) {
	obj = obj_list[i];

	if (obj->life <= 0 || obj->mass == 0)
	    continue;

	if (obj->id == pl->id) {
	    if (BIT(obj->status, OWNERIMMUNE)
		|| frame_time < obj->fusetime
		|| selfImmunity)
		continue;
	} else {
	    if (Team_immune(obj->id, pl->id))
		continue;
	}

	/* don't push balls out of treasure boxes */
	if (BIT(obj->type, OBJ_BALL)
	    && !BIT(obj->status, GRAVITY))
	    continue;

	dx = WRAP_DCX(obj->pos.cx - pl->pos.cx);
	dy = WRAP_DCY(obj->pos.cy - pl->pos.cy);

	/* kps - 4.3.1X had some nice code here, consider using it ? */
	dist = (double)(LENGTH(dx, dy) - PIXEL_TO_CLICK(SHIP_SZ));
	if (dist < range
	    && dist > 0) {
	    int dir = findDir(dx, dy);
	    int idir = MOD2((int)(dir - findDir(obj->vel.x, obj->vel.y)), RES);

	    if (idir > RES * 0.25
		&& idir < RES * 0.75) {
		double force = ((double)(range - dist) / range)
				* ((double)(range - dist) / range)
				* maxforce
				* ((RES * 0.25) - ABS(idir - RES * 0.5))
				/ (RES * 0.25);
		double dv = force / ABS(obj->mass);

		obj->vel.x += tcos(dir) * dv;
		obj->vel.y += tsin(dir) * dv;
	    }
	}
    }
}

void Do_transporter(player *pl)
{
    player	*victim = NULL;
    int		i;
    double	dist, closest = TRANSPORTER_DISTANCE * CLICK;

    /* if not available, fail silently */
    if (!pl->item[ITEM_TRANSPORTER]
	|| pl->fuel.sum < -ED_TRANSPORTER
	|| BIT(pl->used, HAS_PHASING_DEVICE))
	return;

    /* find victim */
    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);
	if (pl_i == pl
	    || !Player_is_active(pl_i)
	    || Team_immune(pl->id, pl_i->id)
	    || Player_is_tank(pl_i)
	    || BIT(pl_i->used, HAS_PHASING_DEVICE))
	    continue;
	dist = Wrap_length(pl->pos.cx - pl_i->pos.cx,
			   pl->pos.cy - pl_i->pos.cy);
	if (dist < closest) {
	    closest = dist;
	    victim = pl_i;
	}
    }

    /* no victims in range */
    if (!victim) {
	sound_play_sensors(pl->pos, TRANSPORTER_FAIL_SOUND);
	Player_add_fuel(pl, ED_TRANSPORTER);
	pl->item[ITEM_TRANSPORTER]--;
	return;
    }

    /* victim found */
    Do_general_transporter(pl, pl->pos, victim, NULL, NULL);
}

void Do_general_transporter(player *pl, clpos pos, player *victim,
			    int *itemp, double *amountp)
{
    char		msg[MSG_LEN];
    const char		*what = NULL;
    int			i;
    int			item = ITEM_FUEL;
    double		amount;
    world_t *world = &World;

    /* choose item type to steal */
    for (i = 0; i < 50; i++) {
	item = (int)(rfrac() * NUM_ITEMS);
	if (victim->item[item]
	    || (item == ITEM_TANK && victim->fuel.num_tanks)
	    || (item == ITEM_FUEL && victim->fuel.sum))
	    break;
    }

    if (i == 50) {
	/* you can't pluck from a bald chicken.. */
	sound_play_sensors(pos, TRANSPORTER_FAIL_SOUND);
	if (!pl) {
	    *amountp = 0.0;
	    *itemp = -1;
	}
	return;
    } else {
	sound_play_sensors(pos, TRANSPORTER_SUCCESS_SOUND);
	if (NumTransporters < MAX_TOTAL_TRANSPORTERS) {
	    Transporters[NumTransporters] = (trans_t *)malloc(sizeof(trans_t));
	    if (Transporters[NumTransporters] != NULL) {
		Transporters[NumTransporters]->pos = pos;
		Transporters[NumTransporters]->victim = victim;
		Transporters[NumTransporters]->id = (pl ? pl->id : NO_ID);
		Transporters[NumTransporters]->count = 5;
		NumTransporters++;
	    }
	}
    }

    /* remove loot from victim */
    amount = 1.0;
    if (!(item == ITEM_MISSILE
	  || item == ITEM_FUEL
	  || item == ITEM_TANK))
	victim->item[item]--;

    /* describe loot and update victim */
    msg[0] = '\0';
    switch (item) {
    case ITEM_AFTERBURNER:
	what = "an afterburner";
	if (victim->item[item] == 0)
	    CLR_BIT(victim->have, HAS_AFTERBURNER);
	break;
    case ITEM_MISSILE:
	amount = (double)MIN(victim->item[item], 3);
	if (amount == 1.0)
	    sprintf(msg, "%s stole a missile from %s.",
		    (pl ? pl->name : "A cannon"), victim->name);
	else
	    sprintf(msg, "%s stole %d missiles from %s",
		    (pl ? pl->name : "A cannon"), (int)amount, victim->name);
        break;
    case ITEM_CLOAK:
	what = "a cloaking device";
	victim->updateVisibility = 1;
	if (!victim->item[item])
	    Cloak(victim, false);
        break;
    case ITEM_WIDEANGLE:
	what = "a wideangle";
        break;
    case ITEM_REARSHOT:
	what = "a rearshot";
        break;
    case ITEM_MINE:
	what = "a mine";
        break;
    case ITEM_SENSOR:
	what = "a sensor";
	victim->updateVisibility = 1;
        break;
    case ITEM_ECM:
	what = "an ECM";
        break;
    case ITEM_ARMOR:
	what = "an armor";
	if (!victim->item[item])
	    CLR_BIT(victim->have, HAS_ARMOR);
	break;
    case ITEM_TRANSPORTER:
	what = "a transporter";
        break;
    case ITEM_MIRROR:
	what = "a mirror";
	if (!victim->item[item])
	    CLR_BIT(victim->have, HAS_MIRROR);
	break;
    case ITEM_DEFLECTOR:
	what = "a deflector";
	if (!victim->item[item])
	    Deflector(victim, false);
        break;
    case ITEM_HYPERJUMP:
	what = "a hyperjump";
        break;
    case ITEM_PHASING:
	what = "a phasing device";
	if (!victim->item[item]) {
	    if (BIT(victim->used, HAS_PHASING_DEVICE))
		Phasing(victim, false);
	    CLR_BIT(victim->have, HAS_PHASING_DEVICE);
	}
        break;
    case ITEM_LASER:
	what = "a laser";
        break;
    case ITEM_EMERGENCY_THRUST:
	what = "an emergency thrust";
	if (!victim->item[item]) {
	    if (BIT(victim->used, HAS_EMERGENCY_THRUST))
		Emergency_thrust(victim, false);
	    CLR_BIT(victim->have, HAS_EMERGENCY_THRUST);
	}
        break;
    case ITEM_EMERGENCY_SHIELD:
	what = "an emergency shield";
	if (!victim->item[item]) {
	    if (BIT(victim->used, HAS_EMERGENCY_SHIELD))
		Emergency_shield(victim, false);
	    CLR_BIT(victim->have, HAS_EMERGENCY_SHIELD);
	    if (!BIT(DEF_HAVE, HAS_SHIELD)) {
		CLR_BIT(victim->have, HAS_SHIELD);
		CLR_BIT(victim->used, HAS_SHIELD);
	    }
	}
        break;
    case ITEM_TRACTOR_BEAM:
	what = "a tractor beam";
	if (!victim->item[item])
	    CLR_BIT(victim->have, HAS_TRACTOR_BEAM);
        break;
    case ITEM_AUTOPILOT:
	what = "an autopilot";
	if (!victim->item[item]) {
	    if (BIT(victim->used, HAS_AUTOPILOT))
		Autopilot(victim, false);
	    CLR_BIT(victim->have, HAS_AUTOPILOT);
	}
        break;
    case ITEM_TANK:
	/* for tanks, amount is the amount of fuel in the stolen tank */
	what = "a tank";
	i = (int)(rfrac() * victim->fuel.num_tanks) + 1;
	amount = victim->fuel.tank[i];
	Player_remove_tank(victim, i);
        break;
    case ITEM_FUEL:
	{
	    /* choose percantage between 10 and 50. */
	    double percent = 10.0 + 40.0 * rfrac();
	    amount = victim->fuel.sum * percent / 100.0;
	    sprintf(msg, "%s stole %.1f units (%.1f%%) of fuel from %s.",
		    (pl ? pl->name : "A cannon"),
		    amount, percent, victim->name);
	}
	Player_add_fuel(victim, -amount);
        break;
    default:
	warn("Do_general_transporter: unknown item type.");
	break;
    }

    /* inform the world about the robbery */
    if (!msg[0])
	sprintf(msg, "%s stole %s from %s.", (pl ? pl->name : "A cannon"),
		what, victim->name);
    Set_message(msg);

    /* cannons take care of themselves */
    if (!pl) {
	*itemp = item;
	*amountp = amount;
	return;
    }

    /* don't forget the penalty for robbery */
    pl->item[ITEM_TRANSPORTER]--;
    Player_add_fuel(pl, ED_TRANSPORTER);

    /* update thief */
    if (!(item == ITEM_FUEL
	  || item == ITEM_TANK))
	pl->item[item] += amount;
    switch(item) {
    case ITEM_AFTERBURNER:
	SET_BIT(pl->have, HAS_AFTERBURNER);
	LIMIT(pl->item[item], 0, MAX_AFTERBURNER);
	break;
    case ITEM_CLOAK:
	SET_BIT(pl->have, HAS_CLOAKING_DEVICE);
	pl->updateVisibility = 1;
	break;
    case ITEM_SENSOR:
	pl->updateVisibility = 1;
	break;
    case ITEM_MIRROR:
	SET_BIT(pl->have, HAS_MIRROR);
	break;
    case ITEM_ARMOR:
	SET_BIT(pl->have, HAS_ARMOR);
	break;
    case ITEM_DEFLECTOR:
	SET_BIT(pl->have, HAS_DEFLECTOR);
	break;
    case ITEM_PHASING:
	SET_BIT(pl->have, HAS_PHASING_DEVICE);
	break;
    case ITEM_EMERGENCY_THRUST:
	SET_BIT(pl->have, HAS_EMERGENCY_THRUST);
	break;
    case ITEM_EMERGENCY_SHIELD:
	SET_BIT(pl->have, HAS_EMERGENCY_SHIELD);
	break;
    case ITEM_TRACTOR_BEAM:
	SET_BIT(pl->have, HAS_TRACTOR_BEAM);
	break;
    case ITEM_AUTOPILOT:
	SET_BIT(pl->have, HAS_AUTOPILOT);
	break;
    case ITEM_TANK:
	/* for tanks, amount is the amount of fuel in the stolen tank */
	if (pl->fuel.num_tanks < MAX_TANKS)
	    Player_add_tank(pl, amount);
	break;
    case ITEM_FUEL:
	Player_add_fuel(pl, amount);
	break;
    default:
	break;
    }

    LIMIT(pl->item[item], 0, world->items[item].limit);
}

/*
 * Returns true if warp status was achieved.
 */
bool Do_hyperjump(player *pl)
{
    if (pl->item[ITEM_HYPERJUMP] <= 0)
	return false;
    if (pl->fuel.sum < -ED_HYPERJUMP)
	return false;
    pl->item[ITEM_HYPERJUMP]--;
    Player_add_fuel(pl, ED_HYPERJUMP);
    SET_BIT(pl->status, WARPING);
    pl->wormHoleHit = -1;
    return true;
}

void do_lose_item(player *pl)
{
    int		item;

    if (!pl)
	return;
    item = pl->lose_item;
    if (item < 0 || item >= NUM_ITEMS) {
	error("BUG: do_lose_item %d", item);
	return;
    }
    if (BIT(1U << pl->lose_item, ITEM_BIT_FUEL | ITEM_BIT_TANK))
	return;

    if (pl->item[item] <= 0)
	return;

    if (loseItemDestroys == false && !BIT(pl->used, HAS_PHASING_DEVICE))
	Place_item(pl, item);
    else
	pl->item[item]--;

    Item_update_flags(pl);
}


void Fire_general_ecm(player *pl, int team, clpos pos)
{
    object		*shot;
    mineobject		*closest_mine = NULL;
    smartobject		*smart;
    mineobject		*mine;
    world_t *world = &World;
    double		closest_mine_range = world->hypotenuse;
    int			i, j;
    double		range, perim, damage;
    player		*p;
    ecm_t		*ecm;

    if (NumEcms >= MAX_TOTAL_ECMS)
	return;

    Ecms[NumEcms] = (ecm_t *)malloc(sizeof(ecm_t));
    if (Ecms[NumEcms] == NULL)
	return;

    ecm = Ecms[NumEcms];
    ecm->pos = pos;
    ecm->id = (pl ? pl->id : NO_ID);
    ecm->size = ECM_DISTANCE;
    NumEcms++;
    if (pl) {
	pl->ecmcount++;
	pl->item[ITEM_ECM]--;
	Player_add_fuel(pl, ED_ECM);
	sound_play_sensors(ecm->pos, ECM_SOUND);
    }

    for (i = 0; i < NumObjs; i++) {
	shot = Obj[i];

	if (! BIT(shot->type, OBJ_SMART_SHOT|OBJ_MINE))
	    continue;
	if ((range = (Wrap_length(pos.cx - shot->pos.cx,
				  pos.cy - shot->pos.cy) / CLICK))
	    > ECM_DISTANCE)
	    continue;

	/*
	 * Ignore mines owned by yourself which you are immune to,
	 * or missiles owned by you which are after somebody else.
	 *
	 * Ignore any object not owned by you which are owned by
	 * team members if team immunity is on.
	 */
	if (shot->id != NO_ID) {
	    player *owner_pl = Player_by_id(shot->id);
	    if (pl == owner_pl) {
		if (shot->type == OBJ_MINE) {
		    if (BIT(shot->status, OWNERIMMUNE))
			continue;
		}
		if (shot->type == OBJ_SMART_SHOT) {
		    if (shot->info != owner_pl->id)
			continue;
		}
	    } else if ((pl && Team_immune(pl->id, owner_pl->id))
		       || (BIT(world->rules->mode, TEAM_PLAY)
			   && team == shot->team))
		continue;
	}

	switch (shot->type) {
	case OBJ_SMART_SHOT:
	    /*
	     * See Move_smart_shot() for re-lock probabilities after confusion
	     * ends.
	     */
	    smart = SMART_PTR(shot);
	    SET_BIT(smart->status, CONFUSED);
	    smart->ecm_range = range;
	    smart->count = CONFUSED_TIME;
	    if (pl
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& (pl->lock.distance <= pl->sensor_range
		    || !BIT(world->rules->mode, LIMITED_VISIBILITY))
		&& pl->visibility[GetInd(pl->lock.pl_id)].canSee)
		smart->new_info = pl->lock.pl_id;
	    else
		smart->new_info = Players((int)(rfrac() * NumPlayers))->id;
	    /* Can't redirect missiles to team mates. */
	    /* So let the missile keep on following this unlucky player. */
	    /*-BA Why not redirect missiles to team mates?
	     *-BA It's not ideal, but better them than me...
	     *if (TEAM_IMMUNE(ind, GetInd(smart->new_info))) {
	     *	smart->new_info = ind;
	     * }
	     */
	    break;

	case OBJ_MINE:
	    mine = MINE_PTR(shot);
	    mine->ecm_range = range;

	    /*
	     * perim is distance from the mine to its detonation perimeter
	     *
	     * range is the proportion from the mine detonation perimeter
	     * to the maximum ecm range.
	     * low values of range mean the mine is close
	     *
	     * remember the closest unconfused mine -- it gets reprogrammed
	     */
	    perim = MINE_RANGE / (mine->mods.mini+1);
	    range = (range - perim) / (ECM_DISTANCE - perim);

	    /*
	     * range%		explode%	confuse time (seconds)
	     * 100		5		2
	     *  50		10		6
	     *	 0 (closest)	15		10
	     */
	    if (range <= 0
		|| (int)(rfrac() * 100.0f) < ((int)(10*(1-range)) + 5)) {
		mine->life = 0;
		break;
	    }
	    mine->count = ((8 * (1 - range)) + 2) * 12;
	    if (!BIT(mine->status, CONFUSED)
		&& (closest_mine == NULL || range < closest_mine_range)) {
		closest_mine = mine;
		closest_mine_range = range;
	    }
	    SET_BIT(mine->status, CONFUSED);
	    if (mine->count <= 0)
		CLR_BIT(mine->status, CONFUSED);
	    break;
	default:
	    break;
	}
    }

    /*
     * range%		reprogram%
     * 100		50
     *  50		75
     *	 0 (closest)	100
     */
    if (ecmsReprogramMines && closest_mine != NULL) {
	range = closest_mine_range;
	if (range <= 0 || (int)(rfrac() * 100.0f) < (100 - (int)(50*range)))
	    closest_mine->id = (pl ? pl->id : NO_ID);
	    closest_mine->team = team;
    }

    /* in non-team mode cannons are immune to cannon ECMs */
    if (BIT(world->rules->mode, TEAM_PLAY) || pl) {
	for (i = 0; i < world->NumCannons; i++) {
	    cannon_t *c = Cannons(world, i);
	    if (BIT(world->rules->mode, TEAM_PLAY)
		&& c->team == team)
		continue;
	    range = Wrap_length(pos.cx - c->pos.cx,
				pos.cy - c->pos.cy) / CLICK;
	    if (range > ECM_DISTANCE)
		continue;
	    damage = (ECM_DISTANCE - range) / ECM_DISTANCE;
	    if (c->item[ITEM_LASER])
		c->item[ITEM_LASER]
		    -= (int)(damage * c->item[ITEM_LASER] + 0.5);
	    c->damaged += 24 * range * pow(0.75, (double)c->item[ITEM_SENSOR]);
	}
    }

    for (i = 0; i < NumPlayers; i++) {
	p = Players(i);

	if (p == pl)
	    continue;

	/*
	 * Team members are always immune from ECM effects from other
	 * team members.  Its too nasty otherwise.
	 */
	if (BIT(world->rules->mode, TEAM_PLAY) && p->team == team)
	    continue;

	if (pl && Players_are_allies(pl, p))
	    continue;

	if (BIT(p->used, HAS_PHASING_DEVICE))
	    continue;

	if (Player_is_active(p)) {
	    range = Wrap_length(pos.cx - p->pos.cx,
				pos.cy - p->pos.cy) / CLICK;
	    if (range > ECM_DISTANCE)
		continue;

	    /* range is how close the player is to the center of ecm */
	    range = ((ECM_DISTANCE - range) / ECM_DISTANCE);

	    /*
	     * range%	damage (sec)	laser destroy%	reprogram%	drop%
	     * 100	4		75		100		25
	     * 50	2		50		75		15
	     * 0	0		25		50		5
	     */

	    /*
	     * should this be FPS dependant: damage = 4.0f * FPS * range; ?
	     * No, i think.
	     */
	    damage = 24.0f * range;

	    if (p->item[ITEM_CLOAK] <= 1)
		p->forceVisible += damage;
	    else
		p->forceVisible
		    += damage * pow(0.75, (double)(p->item[ITEM_CLOAK]-1));

	    /* ECM may cause balls to detach. */
	    if (BIT(p->have, HAS_BALL)) {
		for (j = 0; j < NumObjs; j++) {
		    shot = Obj[j];
		    if (BIT(shot->type, OBJ_BALL)) {
			ballobject *ball = BALL_PTR(shot);
			if (ball->owner == p->id) {
			    if ((int)(rfrac() * 100.0f)
				< ((int)(20*range)+5))
				Detach_ball(p, ball);
			}
		    }
		}
	    }

	    /* ECM damages sensitive equipment like lasers */
	    if (p->item[ITEM_LASER] > 0)
		p->item[ITEM_LASER]
		    -= (int)(range * p->item[ITEM_LASER] + 0.5);

	    if (!Player_is_robot(p) || !ecmsReprogramRobots || !pl) {
		/* player is blinded by light flashes. */
		long duration
		    = (long)(damage * pow(0.75, (double)p->item[ITEM_SENSOR]));
		p->damaged += duration;
		if (pl)
		    Record_shove(p, pl, frame_loops + duration);
	    } else {
		if (BIT(pl->lock.tagged, LOCK_PLAYER)
		    && (pl->lock.distance < pl->sensor_range
			|| !BIT(world->rules->mode, LIMITED_VISIBILITY))
		    && pl->visibility[GetInd(pl->lock.pl_id)].canSee
		    && pl->lock.pl_id != p->id
		    /*&& !TEAM_IMMUNE(ind, GetInd(pl->lock.pl_id))*/) {

		    /*
		     * Player programs robot to seek target.
		     */
		    Robot_program(p, pl->lock.pl_id);
		    for (j = 0; j < NumPlayers; j++) {
			player *pl_j = Players(j);
			if (pl_j->conn != NULL) {
			    Send_seek(pl_j->conn, pl->id,
				      p->id, pl->lock.pl_id);
			}
		    }
		}
	    }
	}
    }
}

void Fire_ecm(player *pl)
{
    if (pl->item[ITEM_ECM] == 0
	|| pl->fuel.sum <= -ED_ECM
	|| pl->ecmcount >= MAX_PLAYER_ECMS
	|| BIT(pl->used, HAS_PHASING_DEVICE))
	return;

    Fire_general_ecm(pl, pl->team, pl->pos);
}



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

#include "xpserver.h"

char ship_version[] = VERSION;


/******************************
 * Functions for ship movement.
 */

void Thrust(player_t *pl)
{
    static int min_dir, max_dir;
    static double max_speed;
    const double max_life = 3 + pl->power * 0.35;
    clpos_t engine = Ship_get_engine_clpos(pl->ship, pl->dir);
    clpos_t pos;
    int afterburners;
    double tot_sparks = (pl->power * 0.15 + 2.5) * timeStep;
    double alt_sparks;

    min_dir = (int)(pl->dir + RES/2 - (RES*0.2 + 1) * options.thrustWidth);
    max_dir = (int)(pl->dir + RES/2 + (RES*0.2 + 1) * options.thrustWidth);
    max_speed = (1 + (pl->power * 0.14)) * options.sparkSpeed;

    pos.cx = pl->pos.cx + engine.cx;
    pos.cy = pl->pos.cy + engine.cy;

    sound_play_sensors(pl->pos, THRUST_SOUND);

    afterburners = (BIT(pl->used, HAS_EMERGENCY_THRUST)
		    ? MAX_AFTERBURNER
		    : pl->item[ITEM_AFTERBURNER]);
    alt_sparks = tot_sparks * afterburners * (1. / (MAX_AFTERBURNER + 1));

    /* floor(tot_sparks + rfrac()) randomly rounds up or down to an integer,
     * so that the expectation value of the result is tot_sparks */
    Make_debris(pl->world,
		pos,
		pl->vel,
		pl->id,
		pl->team,
		OBJ_SPARK,
		options.thrustMass,
		GRAVITY | OWNERIMMUNE,
		RED,
		8,
		(int)((tot_sparks-alt_sparks) + rfrac()),
		min_dir, max_dir,
		1.0, max_speed,
		3.0, max_life);

    Make_debris(pl->world,
		pos,
		pl->vel,
		pl->id,
		pl->team,
		OBJ_SPARK,
		options.thrustMass * ALT_SPARK_MASS_FACT,
		GRAVITY | OWNERIMMUNE,
		BLUE,
		8,
		(int)(alt_sparks + rfrac()),
		min_dir, max_dir,
		1.0, max_speed,
		3.0, max_life);
}

void Record_shove(player_t *pl, player_t *pusher, long shove_time)
{
    shove_t		*shove = &pl->shove_record[pl->shove_next];

    if (++pl->shove_next == MAX_RECORDED_SHOVES)
	pl->shove_next = 0;

    shove->pusher_id = pusher->id;
    shove->time = shove_time;
}

/* Calculates the effect of a collision between two objects */
/* This calculates a completely inelastic collision. Ie. the
 * objects remain stuck together (same velocity and direction.
 * Use this function if one of the objects will die in the
 * collision. */
void Delta_mv(object_t *ship, object_t *obj)
{
    double	vx, vy, m;

    m = ship->mass + ABS(obj->mass);
    vx = (ship->vel.x * ship->mass + obj->vel.x * obj->mass) / m;
    vy = (ship->vel.y * ship->mass + obj->vel.y * obj->mass) / m;
    if (ship->type == OBJ_PLAYER
	&& obj->id != NO_ID
	&& BIT(obj->obj_status, COLLISIONSHOVE)) {
	player_t *pl = (player_t *)ship;
	player_t *pusher = Player_by_id(obj->id);
	if (pusher != pl)
	    Record_shove(pl, pusher, frame_loops);
    }
    ship->vel.x = vx;
    ship->vel.y = vy;
    obj->vel.x = vx;
    obj->vel.y = vy;
}

/* Calculates the effect of a collision between two objects */
/* And now for a completely elastic collision. Ie. the objects
 * will bounce off of eachother. Use this function if both
 * objects stay alive after the collision. */
void Delta_mv_elastic(object_t *obj1, object_t *obj2)
{
    double	m1 = (double)obj1->mass,
		m2 = (double)obj2->mass,
		ms = m1 + m2;
    double	v1x = obj1->vel.x,
		v1y = obj1->vel.y,
		v2x = obj2->vel.x,
		v2y = obj2->vel.y;

    obj1->vel.x = (m1 - m2) / ms * v1x
		  + 2 * m2 / ms * v2x;
    obj1->vel.y = (m1 - m2) / ms * v1y
		  + 2 * m2 / ms * v2y;
    obj2->vel.x = 2 * m1 / ms * v1x
		  + (m2 - m1) / ms * v2x;
    obj2->vel.y = 2 * m1 / ms * v1y
		  + (m2 - m1) / ms * v2y;
    if (obj1->type == OBJ_PLAYER
	&& obj2->id != NO_ID
	&& BIT(obj2->obj_status, COLLISIONSHOVE)) {
	player_t *pl = (player_t *)obj1;
	player_t *pusher = Player_by_id(obj2->id);
	if (pusher != pl)
	    Record_shove(pl, pusher, frame_loops);
    }
}


void Obj_repel(object_t *obj1, object_t *obj2, int repel_dist)
{
    double xd, yd, force, dm, dvx1, dvy1, dvx2, dvy2, a;
    int obj_theta;

    xd = WRAP_DCX(obj2->pos.cx - obj1->pos.cx);
    yd = WRAP_DCY(obj2->pos.cy - obj1->pos.cy);
    force = CLICK_TO_PIXEL((int)(repel_dist - LENGTH(xd, yd)));

    if (force <= 0)
	return;

    force = MIN(force, 10);

    a = findDir(xd, yd);
    obj_theta = MOD2((int) (a + 0.5), RES);

    dm = obj1->mass / obj2->mass;
    dvx2 = tcos(obj_theta) * force * dm;
    dvy2 = tsin(obj_theta) * force * dm;

    dvx1 = -(tcos(obj_theta) * force / dm);
    dvy1 = -(tsin(obj_theta) * force / dm);

    if (obj1->type == OBJ_PLAYER && obj2->id != NO_ID) {
	player_t *pl = (player_t *)obj1;
	player_t *pusher = Player_by_id(obj2->id);
	if (pusher != pl)
	    Record_shove(pl, pusher, frame_loops);
    }

    if (obj2->type == OBJ_PLAYER && obj1->id != NO_ID) {
	player_t *pl = (player_t *)obj2;
	player_t *pusher = Player_by_id(obj1->id);
	if (pusher != pl)
	    Record_shove(pl, pusher, frame_loops);
    }

    obj1->vel.x += dvx1;
    obj1->vel.y += dvy1;

    obj2->vel.x += dvx2;
    obj2->vel.y += dvy2;
}


/*
 * Add fuel to fighter's tanks.
 * Maybe use more than one of tank to store the fuel.
 */
void Add_fuel(pl_fuel_t *ft, double fuel)
{
    if (ft->sum + fuel > ft->max)
	fuel = ft->max - ft->sum;
    else if (ft->sum + fuel < 0.0)
	fuel = -ft->sum;
    ft->sum += fuel;
    ft->tank[ft->current] += fuel;
}


/*
 * Move fuel from add-on tanks to main tank,
 * handle over and underflow of tanks.
 */
void Update_tanks(pl_fuel_t *ft)
{
    if (ft->num_tanks) {
	int  t, check;
	double low_level;
	double fuel;
	double *f;
	double frame_refuel = REFUEL_RATE * timeStep;

	/* Set low_level to minimum fuel in each tank */
	low_level = ft->sum / (ft->num_tanks + 1) - 1;
	if (low_level < 0.0)
	    low_level = 0.0;
	if (TANK_REFILL_LIMIT < low_level)
	    low_level = TANK_REFILL_LIMIT;

	t = ft->num_tanks;
	check = MAX_TANKS << 2;
	fuel = 0;
	f = ft->tank + t;

	while (t >= 0 && check--) {
	    double m = TANK_CAP(t);

	    /* Add the previous over/underflow and do a new cut */
	    *f += fuel;
	    if (*f > m) {
		fuel = *f - m;
		*f = m;
	    } else if (*f < 0) {
		fuel = *f;
		*f = 0;
	    } else
		fuel = 0;

	    /* If there is no over/underflow, let the fuel run to main-tank */
	    if (!fuel) {
		if (t
		    && t != ft->current
		    && *f >= low_level + frame_refuel
		    && *(f-1) <= TANK_CAP(t-1) - frame_refuel) {

		    *f -= frame_refuel;
		    fuel = frame_refuel;
		} else if (t && *f < low_level) {
		    *f += frame_refuel;
		    fuel = frame_refuel;
		}
	    }
	    if (fuel && t == 0) {
	       t = ft->num_tanks;
	       f = ft->tank + t;
	    } else {
		t--;
		f--;
	    }
	}
	if (!check) {
	    error("fuel problem");
	    fuel = ft->sum;
	    ft->sum =
	    ft->max = 0;
	    t = 0;
	    while (t <= ft->num_tanks) {
		if (fuel) {
		    if (fuel>TANK_CAP(t)) {
			ft->tank[t] = TANK_CAP(t);
			fuel -= TANK_CAP(t);
		    } else {
			ft->tank[t] = fuel;
			fuel = 0;
		    }
		    ft->sum += ft->tank[t];
		} else
		    ft->tank[t] = 0;
		ft->max += TANK_CAP(t);
		t++;
	    }
	}
    } else
	ft->tank[0] = ft->sum;
}


/*
 * Use current tank as dummy target for heat seeking missles.
 */
void Tank_handle_detach(player_t *pl)
{
    world_t *world = pl->world;
    player_t *tank;
    int i, ct;

    if (BIT(pl->used, HAS_PHASING_DEVICE))
	return;

    /* Return, if no more players or no tanks */
    if (pl->fuel.num_tanks == 0
	|| NumPseudoPlayers == MAX_PSEUDO_PLAYERS
	|| peek_ID() == 0)
	return;

    /* If current tank is main, use another one */
    if ((ct = pl->fuel.current) == 0)
	ct = pl->fuel.num_tanks;

    Update_tanks(&(pl->fuel));

    /* Fork the current player */
    tank = Player_by_index(NumPlayers);

    /*
     * MWAAH: this was ... naieve at least:
     * *tank = *pl;
     * Player structures contain pointers to dynamic memory...
     */

    Init_player(world, NumPlayers,
		options.allowShipShapes
		? Parse_shape_str(options.tankShipShape) : NULL,
		PL_TYPE_TANK);

    /* Released tanks don't have tanks... */
    while (tank->fuel.num_tanks > 0)
	Player_remove_tank(tank, tank->fuel.num_tanks);

    Player_position_init_clpos(tank, pl->pos);
    tank->vel = pl->vel;
    tank->acc = pl->acc;
    tank->dir = pl->dir;
    tank->turnspeed = pl->turnspeed;
    tank->velocity = pl->velocity;
    Player_set_float_dir(tank, pl->float_dir);
    tank->turnresistance = pl->turnresistance;
    tank->turnvel = pl->turnvel;
    tank->oldturnvel = pl->oldturnvel;
    tank->turnacc = pl->turnacc;
    tank->power = pl->power;

    strlcpy(tank->name, pl->name, MAX_CHARS);
    strlcat(tank->name, "'s tank", MAX_CHARS);
    strlcpy(tank->username, options.tankUserName, MAX_CHARS);
    strlcpy(tank->hostname, options.tankHostName, MAX_CHARS);
    tank->home_base = pl->home_base;
    tank->team = pl->team;
    tank->pseudo_team = pl->pseudo_team;
    tank->alliance = ALLIANCE_NOT_SET;
    tank->invite = NO_ID;
    tank->score = pl->score - options.tankScoreDecrement;

    /* Fuel is the one from chosen tank */
    tank->fuel.sum =
    tank->fuel.tank[0] = pl->fuel.tank[ct];
    tank->fuel.max = TANK_CAP(ct);
    tank->fuel.current = 0;
    tank->fuel.num_tanks = 0;

    /* Mass is only tank + fuel */
    tank->emptymass = options.shipMass;
    tank->mass = tank->emptymass + FUEL_MASS(tank->fuel.sum);
    tank->power *= TANK_THRUST_FACT;

    /* Reset visibility. */
    tank->updateVisibility = true;
    for (i = 0; i <= NumPlayers; i++) {
	tank->visibility[i].lastChange = 0;
	Player_by_index(i)->visibility[NumPlayers].lastChange = 0;
    }

    /* Remember whose tank this is */
    tank->lock.pl_id = pl->id;

    request_ID();
    NumPlayers++;
    NumPseudoPlayers++;
    updateScores = true;

    /* Possibly join alliance. */
    if (pl->alliance != ALLIANCE_NOT_SET)
	Player_join_alliance(tank, pl);

    sound_play_sensors(pl->pos, TANK_DETACH_SOUND);

    /* The tank uses shield and thrust */
    tank->obj_status = GRAVITY;
    Player_thrust(tank, true);
    tank->have = DEF_HAVE;
    tank->used = (DEF_USED & ~USED_KILL & pl->have) | HAS_SHIELD;

    if (!options.allowShields) {
	tank->shield_time = 30 * 12;
	tank->have |= HAS_SHIELD;
    }

    /* Maybe heat-seekers to retarget? */
    for (i = 0; i < NumObjs; i++) {
	object_t *obj = Obj[i];

	if (obj->type == OBJ_HEAT_SHOT) {
	    heatobject_t *heat = HEAT_PTR(obj);

	    if (heat->heat_lock_id > 0
		&& Player_by_id(heat->heat_lock_id) == pl)
		/* kps - is this right ? */
		heat->heat_lock_id = NumPlayers - 1;
	}
    }

    /* Remove tank, fuel and mass from myself */
    Player_remove_tank(pl, ct);

    for (i = 0; i < NumPlayers - 1; i++) {
	player_t *pl_i = Player_by_index(i);

	if (pl_i->conn != NULL) {
	    Send_player(pl_i->conn, tank->id);
	    Send_score(pl_i->conn, tank->id,
		       tank->score, (int)tank->life,
		       tank->mychar, tank->alliance);
	}
    }

    for (i = 0; i < NumSpectators - 1; i++) {
	player_t *pl_i = Player_by_index(i + spectatorStart);

	Send_player(pl_i->conn, tank->id);
	Send_score(pl_i->conn, tank->id, tank->score,
		   (int)tank->life, tank->mychar, tank->alliance);
    }
}


void Make_wreckage(world_t *world,
		   clpos_t  pos,
		   vector_t vel,
		   int      owner_id,
		   int      owner_team,
		   double   min_mass,     double max_mass,
		   double   total_mass,
		   int      status,
		   int      max_wreckage,
		   int      min_dir,      int    max_dir,
		   double   min_speed,    double max_speed,
		   double   min_life,     double max_life)
{
    wireobject_t *wreckage;
    int i, size;
    double life, mass, sum_mass = 0.0;
    modifiers_t mods;

    if (!options.useWreckage)
	return;

    pos = World_wrap_clpos(world, pos);
    if (!World_contains_clpos(world, pos))
	return;

    if (max_life < min_life)
	max_life = min_life;

    if (max_speed < min_speed)
	max_speed = min_speed;

    if (max_wreckage > MAX_TOTAL_SHOTS - NumObjs)
	max_wreckage = MAX_TOTAL_SHOTS - NumObjs;

    CLEAR_MODS(mods);

    for (i = 0; i < max_wreckage && sum_mass < total_mass; i++) {

	double		speed;
	int		dir, radius;

	/* Calculate mass */
	mass = min_mass + rfrac() * (max_mass - min_mass);
	if ( sum_mass + mass > total_mass )
	    mass = total_mass - sum_mass;

	if (mass < min_mass)
	    /* not enough mass available. */
	    break;

	/* Allocate object */
	if ((wreckage = WIRE_PTR(Object_allocate())) == NULL)
	    break;

	wreckage->color = WHITE;
	wreckage->id = owner_id;
	wreckage->team = owner_team;
	wreckage->type = OBJ_WRECKAGE;

	/* Position */
	Object_position_init_clpos(world, OBJ_PTR(wreckage), pos);

	/* Direction */
	dir = MOD2(min_dir + (int)(rfrac() * MOD2(max_dir - min_dir, RES)),
		   RES);

	/* Velocity and acceleration */
	speed = min_speed + rfrac() * (max_speed - min_speed);
	wreckage->vel.x = vel.x + tcos(dir) * speed;
	wreckage->vel.y = vel.y + tsin(dir) * speed;
	wreckage->acc.x = 0;
	wreckage->acc.y = 0;

	/* Mass */
	wreckage->mass = mass;
	sum_mass += mass;

	/* Lifespan  */
	life = min_life + rfrac() * (max_life - min_life);

	wreckage->life = life;
	wreckage->fusetime = 0;

	/* Wreckage type, rotation, and size */
	wreckage->wire_turnspeed = 0.02 + rfrac() * 0.35;
	wreckage->wire_rotation = (int)(rfrac() * RES);
	size = (int) ( 256.0 * 1.5 * mass / total_mass );
	if (size > 255)
	    size = 255;
	wreckage->wire_size = size;
	wreckage->wire_type = (uint8_t)(rfrac() * 256);

	radius = wreckage->wire_size * 16 / 256;
	if (radius < 8)
	    radius = 8;

	wreckage->pl_range = radius;
	wreckage->pl_radius = radius;
	wreckage->obj_status = status;
	wreckage->mods = mods;
	Cell_add_object(world, OBJ_PTR(wreckage));
    }
}


void Explode_fighter(player_t *pl)
{
    int min_debris;
    double debris_range;

    sound_play_sensors(pl->pos, PLAYER_EXPLOSION_SOUND);

    min_debris = (int)(1 + (pl->fuel.sum / 8.0));
    debris_range = pl->mass;
    /* reduce debris since we also create wreckage objects */
    min_debris >>= 1; /* Removed *2.0 from range */

    Make_debris(pl->world,
		pl->pos,
		pl->vel,
		pl->id,
		pl->team,
		OBJ_DEBRIS,
		3.5,
		GRAVITY,
		RED,
		8,
		(int)(min_debris + debris_range * rfrac()),
		0, RES-1,
		20.0, 20.0 + pl->mass * 0.5,
		5.0, 5.0 + pl->mass * 1.5);

    Make_wreckage(pl->world,
		  pl->pos,
		  pl->vel,
		  pl->id,
		  pl->team,
		  MAX(pl->mass/8.0, 0.33), pl->mass,
		  2.0 * pl->mass,
		  GRAVITY,
		  10,
		  0, RES-1,
		  10.0, 10.0 + pl->mass * 0.5,
		  5.0, 5.0 + pl->mass * 1.5);
}

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "xpconfig.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "saudio.h"
#include "error.h"
#include "objpos.h"
#include "netserver.h"
#include "click.h"
#include "commonproto.h"
#include "proto.h"

char ship_version[] = VERSION;



/******************************
 * Functions for ship movement.
 */

void Thrust(player *pl)
{
    const int		min_dir = (int)(pl->dir + RES/2 - RES*0.2 - 1);
    const int		max_dir = (int)(pl->dir + RES/2 + RES*0.2 + 1);
    const DFLOAT	max_speed = 1 + (pl->power * 0.14);
    const DFLOAT	max_life = 3 + pl->power * 0.35;
    clpos		engine = Ship_get_engine_clpos(pl->ship, pl->dir);
    int			cx = pl->pos.cx + engine.cx;
    int			cy = pl->pos.cy + engine.cy;
    int			afterburners;
    DFLOAT		tot_sparks = (pl->power * 0.15 + 2.5) * timeStep;
    DFLOAT		alt_sparks;

    sound_play_sensors(pl->pos.cx, pl->pos.cy, THRUST_SOUND);

    afterburners = (BIT(pl->used, HAS_EMERGENCY_THRUST)
		    ? MAX_AFTERBURNER
		    : pl->item[ITEM_AFTERBURNER]);
    alt_sparks = tot_sparks * afterburners * (1. / (MAX_AFTERBURNER + 1));

    /* floor(tot_sparks + rfrac()) randomly rounds up or down to an integer,
     * so that the expectation value of the result is tot_sparks */
    Make_debris(
	/* pos.x, pos.y   */ cx, cy,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ RED,
	/* radius         */ 8,
	/* num debris     */ (tot_sparks-alt_sparks) + rfrac(),
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1.0, max_speed,
	/* min,max life   */ 3, max_life
	);

    Make_debris(
	/* pos.x, pos.y   */ cx, cy,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_SPARK,
	/* mass           */ THRUST_MASS * ALT_SPARK_MASS_FACT,
	/* status         */ GRAVITY | OWNERIMMUNE,
	/* color          */ BLUE,
	/* radius         */ 8,
	/* num debris     */ alt_sparks + rfrac(),
	/* min,max dir    */ min_dir, max_dir,
	/* min,max speed  */ 1.0, max_speed,
	/* min,max life   */ 3, max_life
	);
}

void Record_shove(player *pl, player *pusher, long time)
{
    shove_t		*shove = &pl->shove_record[pl->shove_next];

    if (++pl->shove_next == MAX_RECORDED_SHOVES) {
	pl->shove_next = 0;
    }
    shove->pusher_id = pusher->id;
    shove->time = time;
}

/* Calculates the effect of a collision between two objects */
/* This calculates a completely inelastic collision. Ie. the
 * objects remain stuck together (same velocity and direction.
 * Use this function if one of the objects will die in the
 * collision. */
void Delta_mv(object *ship, object *obj)
{
    DFLOAT	vx, vy, m;

    m = ship->mass + ABS(obj->mass);
    vx = (ship->vel.x * ship->mass + obj->vel.x * obj->mass) / m;
    vy = (ship->vel.y * ship->mass + obj->vel.y * obj->mass) / m;
    if (ship->type == OBJ_PLAYER
	&& obj->id != NO_ID
	&& BIT(obj->status, COLLISIONSHOVE)) {
	player *pl = (player *)ship;
	player *pusher = Player_by_id(obj->id);
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
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
void Delta_mv_elastic(object *obj1, object *obj2)
{
    DFLOAT	m1 = (DFLOAT)obj1->mass,
		m2 = (DFLOAT)obj2->mass,
		ms = m1 + m2;
    DFLOAT	v1x = obj1->vel.x,
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
	&& BIT(obj2->status, COLLISIONSHOVE)) {
	player *pl = (player *)obj1;
	player *pusher = Player_by_id(obj2->id);
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
    }
}


void Obj_repel(object *obj1, object *obj2, int repel_dist)
{
    DFLOAT		xd, yd,
			force, dm,
			dvx1, dvy1,
			dvx2, dvy2;
    int			obj_theta;

    xd = WRAP_DCX(obj2->pos.cx - obj1->pos.cx);
    yd = WRAP_DCY(obj2->pos.cy - obj1->pos.cy);
    force = CLICK_TO_PIXEL((int)(repel_dist - LENGTH(xd, yd)));

    if (force <= 0)
	return;

    force = MIN(force, 10);

    obj_theta = (int)findDir(xd, yd);

    dm = obj1->mass / obj2->mass;
    dvx2 = tcos(obj_theta) * force * dm;
    dvy2 = tsin(obj_theta) * force * dm;

    dvx1 = -(tcos(obj_theta) * force / dm);
    dvy1 = -(tsin(obj_theta) * force / dm);

    if (obj1->type == OBJ_PLAYER && obj2->id != NO_ID) {
	player *pl = (player *)obj1;
	player *pusher = Player_by_id(obj2->id);
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
    }

    if (obj2->type == OBJ_PLAYER && obj1->id != NO_ID) {
	player *pl = (player *)obj2;
	player *pusher = Player_by_id(obj1->id);
	if (pusher != pl) {
	    Record_shove(pl, pusher, frame_loops);
	}
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
void Add_fuel(pl_fuel_t *ft, long fuel)
{
    if (ft->sum + fuel > ft->max)
	fuel = ft->max - ft->sum;
    else if (ft->sum + fuel < 0)
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
	long low_level;
	long fuel;
	long *f;
	DFLOAT frame_refuel = REFUEL_RATE * timeStep;

	/* Set low_level to minimum fuel in each tank */
	low_level = ft->sum / (ft->num_tanks + 1) - 1;
	if (low_level < 0)
	    low_level = 0;
	if (TANK_REFILL_LIMIT < low_level)
	    low_level = TANK_REFILL_LIMIT;

	t = ft->num_tanks;
	check = MAX_TANKS<<2;
	fuel = 0;
	f = ft->tank + t;

	while (t>=0 && check--) {
	    long m = TANK_CAP(t);

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
void Tank_handle_detach(player *pl)
{
    player		*dummy;
    int			i, ct;

    if (BIT(pl->used, HAS_PHASING_DEVICE))
	return;

    /* Return, if no more players or no tanks */
    if (pl->fuel.num_tanks == 0
	|| NumPseudoPlayers == MAX_PSEUDO_PLAYERS
	|| peek_ID() == 0) {
	return;
    }

    /* If current tank is main, use another one */
    if ((ct = pl->fuel.current) == 0)
	ct = pl->fuel.num_tanks;

    Update_tanks(&(pl->fuel));
    /* Fork the current player */
    dummy               = Players(NumPlayers);
    /*
     * MWAAH: this was ... naieve at least:
     * *dummy              = *pl;
     * Player structures contain pointers to dynamic memory...
     */

    Init_player(NumPlayers, (allowShipShapes)
			    ? Parse_shape_str(tankShipShape)
			    : NULL);
    /* Released tanks don't have tanks... */
    while (dummy->fuel.num_tanks > 0) {
	Player_remove_tank(dummy, dummy->fuel.num_tanks);
    }
    SET_BIT(dummy->type_ext, OBJ_EXT_TANK);
    Player_position_init_clicks(dummy, pl->pos.cx, pl->pos.cy);
    dummy->vel		= pl->vel;
    dummy->acc		= pl->acc;
    dummy->dir		= pl->dir;
    dummy->turnspeed	= pl->turnspeed;
    dummy->velocity	= pl->velocity;
    dummy->float_dir	= pl->float_dir;
    dummy->turnresistance = pl->turnresistance;
    dummy->turnvel	= pl->turnvel;
    dummy->oldturnvel	= pl->oldturnvel;
    dummy->turnacc	= pl->turnacc;
    dummy->power	= pl->power;

    strlcpy(dummy->name, pl->name, MAX_CHARS);
    strlcat(dummy->name, "'s tank", MAX_CHARS);
    strlcpy(dummy->realname, tankRealName, MAX_CHARS);
    strlcpy(dummy->hostname, tankHostName, MAX_CHARS);
    dummy->home_base	= pl->home_base;
    dummy->team		= pl->team;
    dummy->pseudo_team	= pl->pseudo_team;
    dummy->alliance	= ALLIANCE_NOT_SET;
    dummy->invite	= NO_ID;
    dummy->mychar       = 'T';
    dummy->score	= pl->score - tankScoreDecrement;
    updateScores	= true;

    /* Fuel is the one from chosen tank */
    dummy->fuel.sum     =
    dummy->fuel.tank[0] = pl->fuel.tank[ct];
    dummy->fuel.max     = TANK_CAP(ct);
    dummy->fuel.current = 0;
    dummy->fuel.num_tanks = 0;

    /* Mass is only tank + fuel */
    dummy->mass = (dummy->emptymass = ShipMass) + FUEL_MASS(dummy->fuel.sum);
    dummy->power *= TANK_THRUST_FACT;

    /* Reset visibility. */
    dummy->updateVisibility = 1;
    for (i = 0; i <= NumPlayers; i++) {
	dummy->visibility[i].lastChange = 0;
	Players(i)->visibility[NumPlayers].lastChange = 0;
    }

    /* Remember whose tank this is */
    dummy->lock.pl_id = pl->id;

    request_ID();
    NumPlayers++;
    NumPseudoPlayers++;
    updateScores = true;

    /* Possibly join alliance. */
    if (pl->alliance != ALLIANCE_NOT_SET) {
	Player_join_alliance(dummy, pl);
    }

    sound_play_sensors(pl->pos.cx, pl->pos.cy, TANK_DETACH_SOUND);

    /* The tank uses shield and thrust */
    dummy->status = (DEF_BITS & ~KILL_BITS) | PLAYING | GRAVITY | THRUSTING;
    dummy->have = DEF_HAVE;
    dummy->used = (DEF_USED & ~USED_KILL & pl->have) | HAS_SHIELD;
    if (playerShielding == 0) {
	dummy->shield_time = 30 * 12;
	dummy->have |= HAS_SHIELD;
    }

    /* Maybe heat-seekers to retarget? */
    for (i = 0; i < NumObjs; i++) {
	if (Obj[i]->type == OBJ_HEAT_SHOT
	    && Obj[i]->info > 0
	    && Player_by_id(Obj[i]->info) == pl) {
	    Obj[i]->info = NumPlayers - 1;
	}
    }

    /* Remove tank, fuel and mass from myself */
    Player_remove_tank(pl, ct);

    for (i = 0; i < NumPlayers - 1; i++) {
	player *pl_i = Players(i);

	if (pl_i->conn != NOT_CONNECTED) {
	    Send_player(pl_i->conn, dummy->id);
	    Send_score(pl_i->conn, dummy->id,
		       dummy->score, dummy->life,
		       dummy->mychar, dummy->alliance);
	}
    }

    for (i = 0; i < NumObservers - 1; i++) {
	Send_player(Players(i + observerStart)->conn, dummy->id);
	Send_score(Players(i + observerStart)->conn, dummy->id, dummy->score,
		   dummy->life, dummy->mychar, dummy->alliance);
    }
}


void Make_wreckage(
    /* pos.x, pos.y     */ int    cx,           int    cy,
    /* vel.x, vel.y     */ DFLOAT velx,         DFLOAT vely,
    /* owner id         */ int    id,
    /* owner team	*/ unsigned short team,
    /* min,max mass     */ DFLOAT min_mass,     DFLOAT max_mass,
    /* total mass       */ DFLOAT total_mass,
    /* status           */ long   status,
    /* color            */ int    color,
    /* max wreckage     */ int    max_wreckage,
    /* min,max dir      */ int    min_dir,      int    max_dir,
    /* min,max speed    */ DFLOAT min_speed,    DFLOAT max_speed,
    /* min,max life     */ DFLOAT min_life,     DFLOAT max_life
)
{
    wireobject		*wreckage;
    int			i, size;
    DFLOAT		life;
    modifiers		mods;
    DFLOAT		mass, sum_mass = 0.0;

    if (!useWreckage) {
	return;
    }

    cx = WRAP_XCLICK(cx);
    cy = WRAP_YCLICK(cy);
    if (!INSIDE_MAP(cx, cy))
	return;

    if (max_life < min_life)
	max_life = min_life;

    if (max_speed < min_speed)
	max_speed = min_speed;

    if (max_wreckage > MAX_TOTAL_SHOTS - NumObjs) {
	max_wreckage = MAX_TOTAL_SHOTS - NumObjs;
    }

    CLEAR_MODS(mods);

    for (i = 0; i < max_wreckage && sum_mass < total_mass; i++) {

	DFLOAT		speed;
	int		dir, radius;

	/* Calculate mass */
	mass = min_mass + rfrac() * (max_mass - min_mass);
	if ( sum_mass + mass > total_mass ) {
	    mass = total_mass - sum_mass;
	}
	if (mass < min_mass) {
	    /* not enough mass available. */
	    break;
	}

	/* Allocate object */
	if ((wreckage = WIRE_PTR(Object_allocate())) == NULL) {
	    break;
	}

	wreckage->color = color;
	wreckage->id = id;
	wreckage->team = team;
	wreckage->type = OBJ_WRECKAGE;

	/* Position */
	Object_position_init_clicks(OBJ_PTR(wreckage), cx, cy);

	/* Direction */
	dir = MOD2(min_dir + (int)(rfrac() * MOD2(max_dir - min_dir, RES)), RES);

	/* Velocity and acceleration */
	speed = min_speed + rfrac() * (max_speed - min_speed);
	wreckage->vel.x = velx + tcos(dir) * speed;
	wreckage->vel.y = vely + tsin(dir) * speed;
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
	wreckage->turnspeed = 0.02 + rfrac() * 0.35;
	wreckage->rotation = (int)(rfrac() * RES);
	size = (int) ( 256.0 * 1.5 * mass / total_mass );
	if (size > 255)
	    size = 255;
	wreckage->size = size;
	wreckage->info = (int)(rfrac() * 256);

	radius = wreckage->size * 16 / 256;
	if (radius < 8) radius = 8;

	wreckage->pl_range = radius;
	wreckage->pl_radius = radius;
	wreckage->status = status;
	wreckage->mods = mods;
	Cell_add_object((object *) wreckage);
    }
}

/* Explode a fighter */
void Explode_fighter(player *pl)
{
    int min_debris;
    DFLOAT debris_range;

    sound_play_sensors(pl->pos.cx, pl->pos.cy, PLAYER_EXPLOSION_SOUND);

    min_debris = (int)(1 + (pl->fuel.sum / (8.0 * FUEL_SCALE_FACT)));
    debris_range = pl->mass;
    /* reduce debris since we also create wreckage objects */
    min_debris >>= 1; /* Removed *2.0 from range */

    Make_debris(
	/* pos.x, pos.y   */ pl->pos.cx, pl->pos.cy,
	/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
	/* owner id       */ pl->id,
	/* owner team	  */ pl->team,
	/* kind           */ OBJ_DEBRIS,
	/* mass           */ 3.5,
	/* status         */ GRAVITY,
	/* color          */ RED,
	/* radius         */ 8,
	/* num debris     */ min_debris + debris_range * rfrac(),
	/* min,max dir    */ 0, RES-1,
	/* min,max speed  */ 20.0, 20 + (((int)(pl->mass))>>1),
	/* min,max life   */ 5, 5 + (pl->mass * 1.5)
	);

    if ( !BIT(pl->status, KILLED) )
	return;
    Make_wreckage(
	/* pos.x, pos.y     */ pl->pos.cx, pl->pos.cy,
	/* vel.x, vel.y     */ pl->vel.x, pl->vel.y,
	/* owner id         */ pl->id,
	/* owner team	    */ pl->team,
	/* min,max mass     */ MAX(pl->mass/8.0, 0.33), pl->mass,
	/* total mass       */ 2.0 * pl->mass,
	/* status           */ GRAVITY,
	/* color            */ WHITE,
	/* max wreckage     */ 10,
	/* min,max dir      */ 0, RES-1,
	/* min,max speed    */ 10.0, 10 + (((int)(pl->mass))>>1),
	/* min,max life     */ 5,5 + (pl->mass * 1.5)
	);

}

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

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "list.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "score.h"
#include "saudio.h"
#include "error.h"
#include "portability.h"
#include "objpos.h"
#include "asteroid.h"


char laser_version[] = VERSION;


/*
 * Do what needs to be done when a laser pulse
 * actually hits a player.
 */
void Laser_pulse_hits_player(int ind, pulseobject *pulse)
{
    player		*pl;
    player		*vicpl = Players[ind];
    int			killer;
    DFLOAT		sc;
    char		msg[MSG_LEN];

    if (pulse->id != NO_ID) {
	killer = GetInd[pulse->id];
	pl = Players[killer];
    } else {
	killer = -1;
	pl = NULL;
    }

    vicpl->forceVisible += TIME_FACT;
    if (BIT(vicpl->have, HAS_MIRROR)
	&& (rfrac() * (2 * vicpl->item[ITEM_MIRROR])) >= 1) {
	/*pulse->pos.cx = cx - tcos(pulse->dir) * 0.5 * PULSE_SAMPLE_DISTANCE;
	  pulse->pos.cy = cy - tsin(pulse->dir) * 0.5 * PULSE_SAMPLE_DISTANCE;*/
	/* is this ok ? */
	pulse->dir = (int)Wrap_cfindDir(vicpl->pos.cx - pulse->pos.cx,
					vicpl->pos.cy - pulse->pos.cy)
		     * 2 - RES / 2 - pulse->dir;
	pulse->dir = MOD2(pulse->dir, RES);
	
	pulse->vel.x = pulseSpeed * tcos(pulse->dir);
	pulse->vel.y = pulseSpeed * tsin(pulse->dir);

	pulse->life += vicpl->item[ITEM_MIRROR];
	/*pulse->len = PULSE_LENGTH;*/
	pulse->refl = true;
	/**refl = true;*/
	return;
    }

    sound_play_sensors(vicpl->pos.cx, vicpl->pos.cy, PLAYER_EAT_LASER_SOUND);
    if (BIT(vicpl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	== (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	return;
    if (pulse->type != OBJ_PULSE) {
	/* kps -remove */
	warn("Player_collides_with_laser_pulse: "
	     "(pulse->type != OBJ_PULSE)\n");
	return;
    }
#if 0
    if (!BIT(obj->type, KILLING_SHOTS))
	return;
#endif
    if (BIT(pulse->mods.laser, STUN)
	|| (laserIsStunGun == true
	    && allowLaserModifiers == false)) {
	if (BIT(vicpl->used, HAS_SHIELD|HAS_LASER|HAS_SHOT)
	    || BIT(vicpl->status, THRUSTING)) {
	    if (pl) {
		sprintf(msg,
			"%s got paralysed by %s's stun laser.",
			vicpl->name, pl->name);
		if (vicpl->id == pl->id)
		    strcat(msg, " How strange!");
	    } else {
		sprintf(msg,
			"%s got paralysed by a stun laser.",
			vicpl->name);
	    }
	    Set_message(msg);
	    CLR_BIT(vicpl->used,
		    HAS_SHIELD|HAS_LASER|OBJ_SHOT);
	    CLR_BIT(vicpl->status, THRUSTING);
	    vicpl->stunned += 5 * TIME_FACT;
	}
    } else if (BIT(pulse->mods.laser, BLIND)) {
	vicpl->damaged += (12 + 6) * TIME_FACT;
	vicpl->forceVisible += (12 + 6) * TIME_FACT;
	if (pl)
	    Record_shove(vicpl, pl, frame_loops + 12 + 6);
    } else {
	Add_fuel(&(vicpl->fuel), (long)ED_LASER_HIT);
	if (!BIT(vicpl->used, HAS_SHIELD)
	    && !BIT(vicpl->have, HAS_ARMOR)) {
	    SET_BIT(vicpl->status, KILLED);
	    if (pl) {
		sprintf(msg,
			"%s got roasted alive by %s's laser.",
			vicpl->name, pl->name);
		if (vicpl->id == pl->id) {
		    sc = Rate(0, pl->score)
			* laserKillScoreMult
			* selfKillScoreMult;
		    SCORE(ind, -sc, vicpl->pos.cx, vicpl->pos.cy, vicpl->name);
		    strcat(msg, " How strange!");
		} else {
		    sc = Rate(pl->score, vicpl->score)
			* laserKillScoreMult;
		    Score_players(killer, sc, vicpl->name, ind, -sc, pl->name);
		}
	    } else {
		sc = Rate(CANNON_SCORE, vicpl->score) / 4;
		SCORE(ind, -sc, vicpl->pos.cx, vicpl->pos.cy, "Cannon");
		if (BIT(World.rules->mode, TEAM_PLAY)
		    && vicpl->team != pulse->team)
		    TEAM_SCORE(pulse->team, sc);
		sprintf(msg,
			"%s got roasted alive by cannonfire.",
			vicpl->name);
	    }
	    sound_play_sensors(vicpl->pos.cx, vicpl->pos.cy,
			       PLAYER_ROASTED_SOUND);
	    Set_message(msg);
	    if (pl && pl->id != vicpl->id) {
		Rank_AddLaserKill(pl);
		Robot_war(ind, killer);
	    }
	}
	if (!BIT(vicpl->used, HAS_SHIELD)
	    && BIT(vicpl->have, HAS_ARMOR)) {
	    Player_hit_armor(ind);
	}
    }
}


#if 0 /* laserhack */
/*
 * For all existing laser pulse check
 * if they collide with ships or asteroids.
 */
void Laser_pulse_collision(void)
{
    int				ind, i;
    int				p;
    int				max, hits;
    bool			refl;
    vicbuf_t			vicbuf;
    int				cx, cy, cx1, cx2, cy1, cy2;
    int				dcx, dcy, midcx, midcy;
    player			*pl;
    pulse_t			*pulse;
    object			*obj = NULL, *ast = NULL;
    list_t			obj_list = NULL;
    list_iter_t			iter;

    /* object allocation removed */

    /* init vicbuf */
    vicbuf.num_vic = 0;
    vicbuf.max_vic = 0;
    vicbuf.vic_ptr = NULL;

    for (p = NumPulses - 1; p >= 0; --p) {
	pulse = Pulses[p];

	/* check for end of pulse life removed (pulse->life and pulse->len) */
	/* removed determination of pl and ind */

	/* pulse moves every frame */
	dcx = tcos(pulse->dir) * PULSE_SPEED * timeStep2;
	dcy = tsin(pulse->dir) * PULSE_SPEED * timeStep2;
	pulse->pos.cx += dcx;
	pulse->pos.cy += dcy;
	pulse->pos.cx = WRAP_XCLICK(pulse->pos.cx);
	pulse->pos.cy = WRAP_YCLICK(pulse->pos.cy);

	if (BIT(World.rules->mode, WRAP_PLAY)) {
	    cx1 = pulse->pos.cx;
	    cy1 = pulse->pos.cy;
	    cx2 = cx1 + tcos(pulse->dir) * pulse->len;
	    cy2 = cy1 + tsin(pulse->dir) * pulse->len;
	} else {
	    cx1 = pulse->pos.cx;
	    cy1 = pulse->pos.cy;
	    if (!INSIDE_MAP(cx1, cy1)) {
		pulse->len = 0;
		continue;
	    }
	    cx2 = cx1 + tcos(pulse->dir) * pulse->len;
	    if (cx2 < 0) {
		pulse->len = pulse->len * (0 - cx1) / (cx2 - cx1);
		cx2 = 0;
	    }
	    if (cx2 >= World.cwidth) {
		pulse->len = pulse->len * (World.cwidth - 1 - cx1)
		    / (cx2 - cx1);
		cx2 = World.cwidth - 1;
	    }
	    cy2 = cy1 + tsin(pulse->dir) * pulse->len;
	    if (cy2 < 0) {
		pulse->len = pulse->len * (0 - cy1) / (cy2 - cy1);
		cx2 = cx1 + tcos(pulse->dir) * pulse->len;
		cy2 = 0;
	    }
	    if (cy2 >= World.cheight) {
		pulse->len = pulse->len * (World.cheight - 1 - cy1)
		    / (cy2 - cy1);
		cx2 = cx1 + tcos(pulse->dir) * pulse->len;
		cy2 = World.cheight - 1;
	    }
	    if (pulse->len <= 0) {
		pulse->len = 0;
		continue;
	    }
	}

	/* calculate delta x and y for pulse start and end position. */
	dcx = cx2 - cx1;
	dcy = cy2 - cy1;
	dcx = WRAP_DCX(dcx);
	dcy = WRAP_DCY(dcy);

	/* max is the highest absolute delta length of either x or y. */
	max = MAX(ABS(dcx), ABS(dcy));
	if (max == 0) {
	    continue;
	}

	/* calculate the midpoint of the new laser pulse position. */
	midcx = cx1 + (dcx / 2);
	midcy = cy1 + (dcy / 2);
	midcx = WRAP_XCLICK(midcx);
	midcy = WRAP_YCLICK(midcy);

	if (round_delay == 0) {
	    /* assemble a shortlist of players which might get hit. */
	    Laser_pulse_find_victims(&vicbuf, pulse, midcx, midcy);
	}

	obj_list = Laser_pulse_get_object_list(obj_list, pulse,	midcx, midcy);

	obj->type = OBJ_PULSE;
	obj->life = 1;
	obj->id = pulse->id;
	obj->team = pulse->team;
	obj->count = 0;
	obj->status = 0;
	if (pulse->id == NO_ID) {
	    obj->status = FROMCANNON;
	}
	Object_position_init_clicks(obj, cx1, cy1);

	refl = false;

	hits = 0;
	for (i = 0; i <= max; i += PULSE_SAMPLE_DISTANCE) {
	    cx = cx1 + (i * dcx) / max;
	    cy = cy1 + (i * dcy) / max;
	    obj->vel.x = CLICK_TO_FLOAT(cx - obj->pos.cx);
	    obj->vel.y = CLICK_TO_FLOAT(cy - obj->pos.cy);
	    /* changed from = x - obj->pos.x to make lasers disappear
	       less frequently when wrapping. There's still a small
	       chance of it happening though. */
	    Move_object(obj);
	    if (obj->life == 0) {
		break;
	    }
	    if (BIT(World.rules->mode, WRAP_PLAY)) {
		if (cx < 0) {
		    cx += World.cwidth;
		    cx1 += World.cwidth;
		}
		else if (cx >= World.cwidth) {
		    cx -= World.cwidth;
		    cx1 -= World.cwidth;
		}
		if (cy < 0) {
		    cy += World.cheight;
		    cy1 += World.cheight;
		}
		else if (cy >= World.cheight) {
		    cy -= World.cheight;
		    cy1 -= World.cheight;
		}
	    }

	    /* check for collision with objects. */
	    if (obj_list != NULL) {
		for (iter = List_begin(obj_list);
		     iter != List_end(obj_list);
		     LI_FORWARD(iter))
		{
		    int adcx, adcy, radius
;
		    ast = LI_DATA(iter);
		    adcx = cx - ast->pos.cx;
		    adcy = cy - ast->pos.cy;
		    adcx = WRAP_DCX(adcx);
		    adcy = WRAP_DCY(adcy);
		    radius = ast->pl_radius * CLICK;
		    if (ABS(adcx) > radius || ABS(adcy) > radius)
			continue;
		    if (sqr(adcx) + sqr(adcy) <= sqr(radius)) {
			obj->life = 0;
			ast->life += ASTEROID_FUEL_HIT(ED_LASER_HIT,
						       WIRE_PTR(ast)->size);
			if (ast->life < 0)
			    ast->life = 0;
			if (ast->life == 0
			    && ind != -1
			    && asteroidPoints > 0
			    && Players[ind]->score <= asteroidMaxScore) {
			    SCORE(ind, asteroidPoints,
				  ast->pos.cx, ast->pos.cy, "");
			}
			break;
		    }
		}
	    }

	    if (obj->life == 0) {
		/* pulse hit asteroid */
		continue;
	    }

	    if (round_delay > 0) {
		/* at round delay no hits are possible */
		continue;
	    }

	    hits = Laser_pulse_check_player_hits(
			    pulse, obj,
			    cx, cy,
			    &vicbuf,
			    &refl);

	    if (hits > 0) {
		break;
	    }
	}

	if (i < max && refl == false) {
	    pulse->len = (pulse->len * i) / max;
	}
    }
    if (vicbuf.max_vic > 0 && vicbuf.vic_ptr != NULL) {
	free(vicbuf.vic_ptr);
    }

    obj->type = OBJ_DEBRIS;
    obj->life = 0;
    Cell_add_object(obj);

    if (obj_list != NULL) {
	List_delete(obj_list);
    }
}
#endif


#if 0
/* kps - take stuff from poly version and move into the above */
/* commented everything that is handled ok in Laser_pulse_collision() */
static void LaserCollision(void)
{
    int				ind, j, p, sc,
				objnum = -1;
    int 			dx, dy;
    player			*pl, *vic;
    pulse_t			*pulse;
    object			*obj = NULL;
    char			msg[MSG_LEN];

    for (p = 0; p < NumPulses; p++) {
	/* removed determine ind and pl */
	/* removed checking end of pulse life */
	/* removed setting pulse->len to 0 for pulses < PULSE_LENGTH */
	/* removed allocation of obj */
	/* removed pulse moves every frame part */

	/* ng did not have calculation of pulse start and end points */
	/* ng did not have Laser_pulse_find_victims */
	/* ng did not have Laser_pulse_get_object_list */

	/* removed setting of obj->type etc. and object position */

	obj->vel.x = CLICK_TO_FLOAT(dx);
	obj->vel.y = CLICK_TO_FLOAT(dy);
	Move_object(objnum);
	if (obj->life == 0) {
	    pulse->len = PULSE_SPEED * obj->wall_time;
	    if (pulse->len > CLICK_TO_PIXEL(PULSE_LENGTH))
		/* -1 is a hack to make the pulse die in the next frame. */
		pulse->len = CLICK_TO_PIXEL(PULSE_LENGTH) - 1;
	}
	if (rdelay <= 0) {
	    for (j = 0; j < NumPlayers; j++) {
		vic = Players[j];
		/* removed check for playing */
		/* Team_immune handles this */
		/*if (BIT(World.rules->mode, TEAM_PLAY)
		    && teamImmunity
		    && vic->team == pulse->team
		    && vic->id != pulse->id) {
		    continue;
		    }*/
		/* checked */
		/*if (vic->id == pulse->id && !pulse->refl) {
		  continue;
		  }*/
		if (Wrap_length(vic->pos.cx - pulse->pos.x, vic->pos.cy - pulse->pos.y)
		    > pulse->len + PIXEL_TO_CLICK(SHIP_SZ)) {
		    continue;
		}
		if (obj->life) {
		    if (!in_range_acd(vic->prevpos.x - obj->prevpos.x,
				      vic->prevpos.y - obj->prevpos.y,
				      vic->extmove.x - obj->extmove.x,
				      vic->extmove.y - obj->extmove.y,
				      SHIP_SZ * CLICK))
			continue;
		}
		else
		    if (!in_range_partial(vic->prevpos.x - obj->prevpos.x,
					  vic->prevpos.y - obj->prevpos.y,
					  vic->extmove.x - obj->extmove.x,
					  vic->extmove.y - obj->extmove.y,
					  SHIP_SZ * CLICK, obj->wall_time))
			continue;
		vic->forceVisible += TIME_FACT;
		pulse->len = PULSE_LENGTH - 1;
		/* removed broken mirrors */
		/* removed laser pulse hits player stuff */
	    }
	}
    }
    if (objnum >= 0 && obj != NULL) {
	obj->type = OBJ_DEBRIS;
	obj->life = 0;
    }
}
#endif

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
 * Type to hold info about a player
 * which might be hit by a laser pulse.
 */
typedef struct victim {
    int			ind;		/* player index */
    clpos		pos;		/* current player position */
    DFLOAT		prev_dist;	/* distance at previous sample */
} victim_t;


/*
 * Type to hold info about all players
 * which may be hit by a laser pulse.
 */
typedef struct vicbuf {
    int			num_vic;	/* number of victims. */
    int			max_vic;	/* max number */
    victim_t		*vic_ptr;	/* pointer to buffer for victims */
} vicbuf_t;


/*
 * Destroy one laser pulse.
 */
static void Laser_pulse_destroy_one(int pulse_index)
{
    int			ind;
    player		*pl;
    pulse_t		*pulse_ptr;

    pulse_ptr = Pulses[pulse_index];
    if (pulse_ptr->id != NO_ID) {
	ind = GetInd[pulse_ptr->id];
	pl = Players[ind];
	pl->num_pulses--;
    }

    free(pulse_ptr);

    if (--NumPulses > pulse_index) {
	Pulses[pulse_index] = Pulses[NumPulses];
    }
}


/*
 * Destroy all laser pulses.
 */
static void Laser_pulse_destroy_all(void)
{
    int			p;

    for (p = NumPulses - 1; p >= 0; --p) {
	Laser_pulse_destroy_one(p);
    }
}


/*
 * Loop over all players and put the
 * ones which are close the pulse midpoint
 * in a vicbuf structure.
 */
static void Laser_pulse_find_victims(
	vicbuf_t *vicbuf, 
	pulse_t *pulse,
	int midcx,
	int midcy)
{
    int		i;
    player	*vic;
    int		dist;

    vicbuf->num_vic = 0;
    for (i = 0; i < NumPlayers; i++) {
	vic = Players[i];
	if (BIT(vic->status, PLAYING|GAME_OVER|KILLED|PAUSE)
	    != PLAYING) {
	    continue;
	}
	if (BIT(vic->used, HAS_PHASING_DEVICE)) {
	    continue;
	}
	if (vic->id == pulse->id
	    && selfImmunity) {
	    continue;
	}
	if (selfImmunity &&
	    IS_TANK_PTR(vic) &&
	    vic->lock.pl_id == pulse->id) {
	    continue;
	}
	if (Team_immune(vic->id, pulse->id)) {
	    continue;
	}
	/* special case for cannon pulses */
	if (pulse->id == NO_ID &&
	    teamImmunity &&
	    BIT(World.rules->mode, TEAM_PLAY) &&
	    pulse->team == vic->team) {
	    continue;
	}
	if (vic->id == pulse->id && !pulse->refl) {
	    continue;
	}
	dist = Wrap_length(vic->pos.cx - midcx,
			   vic->pos.cy - midcy);
	if (dist > pulse->len / 2 + PIXEL_TO_CLICK(SHIP_SZ)) {
	    continue;
	}
	if (vicbuf->max_vic == 0) {
	    size_t victim_bufsize = NumPlayers * sizeof(victim_t);
	    vicbuf->vic_ptr = (victim_t *) malloc(victim_bufsize);
	    if (vicbuf->vic_ptr == NULL) {
		break;
	    }
	    vicbuf->max_vic = NumPlayers;
	}
	vicbuf->vic_ptr[vicbuf->num_vic].ind = i;
	vicbuf->vic_ptr[vicbuf->num_vic].pos.cx = vic->pos.cx;
	vicbuf->vic_ptr[vicbuf->num_vic].pos.cy = vic->pos.cy;
	vicbuf->vic_ptr[vicbuf->num_vic].prev_dist = 1e10;
	vicbuf->num_vic++;
    }
}


/*
 * Do what needs to be done when a laser pulse
 * actually hits a player.
 * If the pulse was reflected by a mirror
 * then set "refl" to true.
 */
static void Laser_pulse_hits_player(
	pulse_t *pulse,
	object *obj,
	int cx,
	int cy,
	victim_t *victim,
	bool *refl)
{
    player		*pl;
    player		*vicpl;
    int			ind;
    DFLOAT		sc;
    char		msg[MSG_LEN];

    if (pulse->id != NO_ID) {
	ind = GetInd[pulse->id];
	pl = Players[ind];
    } else {
	ind = -1;
	pl = NULL;
    }

    vicpl = Players[victim->ind];
    vicpl->forceVisible += TIME_FACT;
    if (BIT(vicpl->have, HAS_MIRROR)
	&& (rfrac() * (2 * vicpl->item[ITEM_MIRROR])) >= 1) {
	pulse->pos.cx = cx - tcos(pulse->dir) * 0.5 * PULSE_SAMPLE_DISTANCE;
	pulse->pos.cy = cy - tsin(pulse->dir) * 0.5 * PULSE_SAMPLE_DISTANCE;
	pulse->dir = (int)Wrap_cfindDir(vicpl->pos.cx - pulse->pos.cx,
					vicpl->pos.cy - pulse->pos.cy)
		     * 2 - RES / 2 - pulse->dir;
	pulse->dir = MOD2(pulse->dir, RES);
	pulse->life += vicpl->item[ITEM_MIRROR];
	pulse->len = PULSE_LENGTH;
	pulse->refl = true;
	*refl = true;
	return;
    }

    sound_play_sensors(vicpl->pos.cx, vicpl->pos.cy,
		       PLAYER_EAT_LASER_SOUND);
    if (BIT(vicpl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	== (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
	return;
    if (!BIT(obj->type, KILLING_SHOTS))
	return;
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
		    SCORE(victim->ind, -sc, vicpl->pos.cx, vicpl->pos.cy,
			  vicpl->name);
		    strcat(msg, " How strange!");
		} else {
		    sc = Rate(pl->score,
					 vicpl->score)
			 * laserKillScoreMult;
		    Score_players(ind, sc, vicpl->name,
				  victim->ind, -sc,
				  pl->name);
		}
	    } else {
		sc = Rate(CANNON_SCORE, vicpl->score) / 4;
		SCORE(victim->ind, -sc, vicpl->pos.cx, vicpl->pos.cy,
		      "Cannon");
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
		Robot_war(victim->ind, ind);
	    }
	}
	if (!BIT(vicpl->used, HAS_SHIELD)
	    && BIT(vicpl->have, HAS_ARMOR)) {
	    Player_hit_armor(victim->ind);
	}
    }
}


/*
 * Check a given pulse position against a list of players.
 * Do what needs to be done when on any pulse hits player event.
 * Return the number of hits.
 * When the pulse was reflected then "refl" will have been set to true.
 */
static int Laser_pulse_check_player_hits(
		    pulse_t *pulse,
		    object *obj,
		    int cx,
		    int cy,
		    vicbuf_t *vicbuf,
		    bool *refl)
{
    int			j;
    int			hits = 0;
    int			ind;
    DFLOAT		dist;
    player		*pl;
    victim_t		*victim;

    if (pulse->id != NO_ID) {
	ind = GetInd[pulse->id];
	pl = Players[ind];
    } else {
	ind = -1;
	pl = NULL;
    }

    for (j = vicbuf->num_vic - 1; j >= 0; --j) {
	victim = &(vicbuf->vic_ptr[j]);
	dist = Wrap_length(cx - victim->pos.cx,
			   cy - victim->pos.cy);
	if (dist <= PIXEL_TO_CLICK(SHIP_SZ)) {
	    Laser_pulse_hits_player(
			pulse,
			obj,
			cx, cy,
			victim,
			refl);
	    hits++;
	    /* stop at the first hit. */
	    break;
	}
	else if (dist >= victim->prev_dist) {
	    /* remove victim by copying the last victim over it */
	    vicbuf->vic_ptr[j] = vicbuf->vic_ptr[--vicbuf->num_vic];
	} else {
	    /* remember shortest distance from pulse to player */
	    vicbuf->vic_ptr[j].prev_dist = dist;
	}
    }

    return hits;
}


static list_t Laser_pulse_get_object_list(
	list_t input_obj_list,
	pulse_t *pulse,
	int midcx,
	int midcy)
{
    list_t		output_obj_list;
    list_t		ast_list;
    int			dcx, dcy, range;
    list_iter_t		iter;
    object		*ast;

    if (input_obj_list != NULL) {
	List_clear(input_obj_list);
    }
    output_obj_list = input_obj_list;

    ast_list = Asteroid_get_list();
    if (ast_list != NULL) {
	if (output_obj_list == NULL) {
	    output_obj_list = List_new();
	}
	if (output_obj_list != NULL) {
	    /* fill list with interesting objects
	     * which are close to our pulse. */
	    for (iter = List_begin(ast_list);
		 iter != List_end(ast_list);
		 LI_FORWARD(iter))
	    {
		ast = (object *) LI_DATA(iter);
		dcx = midcx - ast->pos.cx;
		dcy = midcy - ast->pos.cy;
		dcx = WRAP_DCX(dcx);
		dcy = WRAP_DCY(dcy);
		range = ast->pl_radius * CLICK + pulse->len / 2;
		if (ABS(dcx) > range || ABS(dcy) > range)
		    continue;
		if (sqr(dcx) + sqr(dcy) < sqr(range)) {
		    List_push_back(output_obj_list, ast);
		}
	    }
	}
    }

    return output_obj_list;
}


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

    /*
     * Allocate one object with which we will
     * do pulse wall bounce checking.
     */
    if ((obj = Object_allocate()) == NULL) {
	/* overload.  we can't do bounce checking. */
	Laser_pulse_destroy_all();
	return;
    }

    /* init vicbuf */
    vicbuf.num_vic = 0;
    vicbuf.max_vic = 0;
    vicbuf.vic_ptr = NULL;

    for (p = NumPulses - 1; p >= 0; --p) {
	pulse = Pulses[p];

	/* check for end of pulse life */
	/* kps - why check pulse->len here ??? */
	if ((pulse->life -= timeStep) < 0 || pulse->len < PULSE_LENGTH) {
	    Laser_pulse_destroy_one(p);
	    continue;
	}

	if (pulse->id != NO_ID) {
	    ind = GetInd[pulse->id];
	    pl = Players[ind];
	} else {
	    ind = -1;
	    pl = NULL;
	}

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
		if (BIT(vic->status, PLAYING|GAME_OVER|KILLED|PAUSE)
		    != PLAYING) {
		    continue;
		}
		if (BIT(vic->used, OBJ_PHASING_DEVICE)) {
		    continue;
		}
		if (BIT(World.rules->mode, TEAM_PLAY)
		    && teamImmunity
		    && vic->team == pulse->team
		    && vic->id != pulse->id) {
		    continue;
		}
		if (vic->id == pulse->id && !pulse->refl) {
		    continue;
		}
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
#if 0
		/* Mirrors don't work at the moment. */
		if (BIT(vic->have, OBJ_MIRROR)
		    && (rfrac() * (2 * vic->item[ITEM_MIRROR])) >= 1) {
		    /* STUFF REMOVED */
		    pulse->life += vic->item[ITEM_MIRROR];
		    pulse->len = PULSE_LENGTH;
		    pulse->refl = true;
		    continue;
		}
#endif
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

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

    vicpl->forceVisible += 1;
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
	pulse->len = 0 /*PULSE_LENGTH*/;
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
    if (!BIT(pulse->type, KILLING_SHOTS))
	return;
#endif
    /* kps - do we need some hack so that the laser pulse is
     * not removed in the same frame that its life ends ?? */
    pulse->life = 0;
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
	    vicpl->stunned += 5;
	}
    } else if (BIT(pulse->mods.laser, BLIND)) {
	vicpl->damaged += (12 + 6);
	vicpl->forceVisible += (12 + 6);
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

/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 2003 Kristian Söderblom <kps@users.sourceforge.net>
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

char laser_version[] = VERSION;

/*
 * Do what needs to be done when a laser pulse
 * actually hits a player.
 */
void Laser_pulse_hits_player(player_t *pl, pulseobject_t *pulse)
{
    player_t *kp;
    double sc;
    world_t *world = pl->world;

    kp = Player_by_id(pulse->id);

    pl->forceVisible += 1;
    if (BIT(pl->have, HAS_MIRROR)
	&& (rfrac() * (2 * pl->item[ITEM_MIRROR])) >= 1) {
	pulse->dir = (int)(Wrap_cfindDir(pl->pos.cx - pulse->pos.cx,
					 pl->pos.cy - pulse->pos.cy)
			   * 2 - RES / 2 - pulse->dir);
	pulse->dir = MOD2(pulse->dir, RES);

	pulse->vel.x = options.pulseSpeed * tcos(pulse->dir);
	pulse->vel.y = options.pulseSpeed * tsin(pulse->dir);

	pulse->life += pl->item[ITEM_MIRROR];
	pulse->len = 0 /*PULSE_LENGTH*/;
	pulse->refl = true;
	return;
    }

    sound_play_sensors(pl->pos, PLAYER_EAT_LASER_SOUND);
    if (Player_used_emergency_shield(pl))
	return;
    assert(pulse->type == OBJ_PULSE);

    /* kps - do we need some hack so that the laser pulse is
     * not removed in the same frame that its life ends ?? */
    pulse->life = 0;
    if (BIT(pulse->mods.laser, STUN)
	|| (options.laserIsStunGun == true
	    && options.allowLaserModifiers == false)) {
	if (BIT(pl->used, HAS_SHIELD|HAS_LASER|HAS_SHOT)
	    || BIT(pl->status, THRUSTING)) {
	    if (kp)
		Set_message_f("%s got paralysed by %s's stun laser.%s",
			      pl->name, kp->name,
			      pl->id == kp->id ? " How strange!" : "");
	    else
		Set_message_f("%s got paralysed by a stun laser.", pl->name);

	    CLR_BIT(pl->used,
		    HAS_SHIELD|HAS_LASER|OBJ_SHOT);
	    CLR_BIT(pl->status, THRUSTING);
	    pl->stunned += 5;
	}
    } else if (BIT(pulse->mods.laser, BLIND)) {
	pl->damaged += (12 + 6);
	pl->forceVisible += (12 + 6);
	if (kp)
	    Record_shove(pl, kp, frame_loops + 12 + 6);
    } else {
	Player_add_fuel(pl, ED_LASER_HIT);
	if (!BIT(pl->used, HAS_SHIELD)
	    && !BIT(pl->have, HAS_ARMOR)) {
	    SET_BIT(pl->status, KILLED);
	    if (kp) {
		Set_message_f("%s got roasted alive by %s's laser.%s",
			      pl->name, kp->name,
			      pl->id == kp->id ? " How strange!" : "");
		if (pl->id == kp->id) {
		    sc = Rate(0.0, kp->score)
			* options.laserKillScoreMult
			* options.selfKillScoreMult;
		    Score(kp, -sc, kp->pos, kp->name);
		} else {
		    sc = Rate(kp->score, pl->score)
			* options.laserKillScoreMult;
		    Score_players(kp, sc, pl->name, pl, -sc, kp->name, true);
		}
	    } else {
		sc = Rate(CANNON_SCORE, pl->score) / 4;
		Score(pl, -sc, pl->pos, "Cannon");
		if (BIT(world->rules->mode, TEAM_PLAY)
		    && pl->team != pulse->team)
		    Team_score(world, pulse->team, sc);
		Set_message_f("%s got roasted alive by cannonfire.", pl->name);
	    }
	    sound_play_sensors(pl->pos, PLAYER_ROASTED_SOUND);
	    if (kp && kp->id != pl->id) {
		Rank_add_laser_kill(kp);
		Robot_war(pl, kp);
	    }
	}
	if (!BIT(pl->used, HAS_SHIELD)
	    && BIT(pl->have, HAS_ARMOR))
	    Player_hit_armor(pl);
    }
}

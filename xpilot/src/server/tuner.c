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

char tuner_version[] = VERSION;



void tuner_plock(void)
{
    pLockServer = (plock_server(pLockServer) == 1) ? true : false;
}

void tuner_shipmass(void)
{
    int i;

    for (i = 0; i < NumPlayers; i++)
	Players(i)->emptymass = ShipMass;
}

void tuner_ballmass(void)
{
    int i;

    for (i = 0; i < NumObjs; i++) {
	if (BIT(Obj[i]->type, OBJ_BALL))
	    Obj[i]->mass = ballMass;
    }
}

void tuner_maxrobots(void)
{
    if (maxRobots < 0)
	maxRobots = World.NumBases;

    if (maxRobots < minRobots)
	minRobots = maxRobots;

    while (maxRobots < NumRobots)
	Robot_delete(NULL, true);
}

void tuner_minrobots(void)
{
    if (minRobots < 0)
	minRobots = maxRobots;

    if (maxRobots < minRobots)
	maxRobots = minRobots;
}

void tuner_playershielding(void)
{
    int i;

    Set_world_rules();

    if (playerShielding) {
	SET_BIT(DEF_HAVE, HAS_SHIELD);

	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);
	    if (!IS_TANK_PTR(pl_i)) {
		if (!BIT(pl_i->used, HAS_SHOT))
		    SET_BIT(pl_i->used, HAS_SHIELD);

		SET_BIT(pl_i->have, HAS_SHIELD);
		pl_i->shield_time = 0;
	    }
	}
    }
    else {
	CLR_BIT(DEF_HAVE, HAS_SHIELD);

	for (i = 0; i < NumPlayers; i++)
	    /* approx 2 seconds to get to safety */
	    Players(i)->shield_time = SHIELD_TIME;
    }
}

void tuner_playerstartsshielded(void)
{
    if (playerShielding)
	/* Doesn't make sense to turn off when shields are on. */
	playerStartsShielded = true;
}

void tuner_worldlives(void)
{
    if (worldLives < 0)
	worldLives = 0;

    Set_world_rules();

    if (BIT(World.rules->mode, LIMITED_LIVES)) {
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;
    }
}

void tuner_cannonsmartness(void)
{
    LIMIT(cannonSmartness, 0, 3);
}

void tuner_teamcannons(void)
{
    int i;
    int team;

    if (teamCannons) {
	for (i = 0; i < World.NumCannons; i++) {
	    cannon_t *cannon = Cannons(i);
	    team = Find_closest_team(&World, cannon->pos);
	    if (team == TEAM_NOT_SET)
		warn("Couldn't find a matching team for the cannon.");
	    cannon->team = team;
	}
    }
    else {
	for (i = 0; i < World.NumCannons; i++)
	    Cannons(i)->team = TEAM_NOT_SET;
    }
}

void tuner_cannonsuseitems(void)
{
    int i, j;
    cannon_t *c;

    Move_init();

    for (i = 0; i < World.NumCannons; i++) {
	c = Cannons(i);
	for (j = 0; j < NUM_ITEMS; j++) {
	    c->item[j] = 0;

	    if (cannonsUseItems)
		Cannon_add_item(c, j,
				(int)(rfrac() * (World.items[j].initial + 1)));
	}
    }
}

void tuner_wormtime(void)
{
    int i;

    if (wormTime < 0)
	wormTime = 0;

    if (wormTime) {
	for (i = 0; i < World.NumWormholes; i++)
	    Wormholes(i)->countdown = wormTime;
    }
    else {
	for (i = 0; i < World.NumWormholes; i++) {
	    if (Wormholes(i)->temporary)
		remove_temp_wormhole(&World, i);
	    else
		Wormholes(i)->countdown = WORMCOUNT;
	}
    }
}

void tuner_modifiers(void)
{
    int i;

    Set_world_rules();

    for (i = 0; i < NumPlayers; i++)
	filter_mods(&Players(i)->mods);
}

void tuner_gameduration(void)
{
    if (gameDuration <= 0.0)
	gameOverTime = time((time_t *) NULL);
    else
	gameOverTime = (time_t) (gameDuration * 60) + time((time_t *) NULL);
}

void tuner_racelaps(void)
{
    if (BIT(World.rules->mode, TIMING)) {
	Reset_all_players();
	if (gameDuration == -1)
	    gameDuration = 0;
    }
}

void tuner_allowalliances(void)
{
    if (BIT(World.rules->mode, TEAM_PLAY))
	CLR_BIT(World.rules->mode, ALLIANCES);

    if (!BIT(World.rules->mode, ALLIANCES) && NumAlliances > 0)
	Dissolve_all_alliances();
}

void tuner_announcealliances(void)
{
    updateScores = true;
}

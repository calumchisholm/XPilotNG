/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 2000-2004 by
 *
 *      Uoti Urpala          <uau@users.sourceforge.net>
 *      Kristian Söderblom   <kps@users.sourceforge.net>
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

char play_version[] = VERSION;

int tagItPlayerId = NO_ID;	/* player who is 'it' */

void Transfer_tag(player_t *oldtag_pl, player_t *newtag_pl)
{
    char	msg[MSG_LEN];

    if (tagItPlayerId != oldtag_pl->id
 	|| oldtag_pl->id == newtag_pl->id)
 	return;

    tagItPlayerId = newtag_pl->id;
    sprintf(msg, " < %s killed %s and gets to be 'it' now. >",
	    newtag_pl->name, oldtag_pl->name);
    Set_message(msg);
}

#if 0
double Handle_tag(double sc, player_t *victim_pl, player_t *killer_pl)
{
    if (tagItPlayerId == killer_pl->id)
	return tagItKillMult * sc;
    else if (tagItPlayerId == victim_pl->id) {
	Transfer_tag(victim_pl, killer_pl);
	return tagKillItMult * sc;
    }
    return sc;
}
#endif

static inline bool Player_can_be_tagged(player_t *pl)
{
    if (Player_is_tank(pl))
	return false;
    if (BIT(pl->status, PAUSE))
	return false;
    return true;
}

/*
 * Called from update during tag game to check that a non-paused
 * player is tagged.
 */
void Check_tag(void)
{
    int num = 0, i, candidate;
    player_t *tag_pl = Player_by_id(tagItPlayerId);

    if (tag_pl && Player_can_be_tagged(tag_pl))
	return;

    /* Find number of players that might get the tag */
    for (i = 0; i < NumPlayers; i++) {
	player_t *pl = Player_by_index(i);
	if (Player_can_be_tagged(pl))
	    num++;
    }

    if (num == 0) {
	tagItPlayerId = NO_ID;
	return;
    }

    /* select first candidate for tag */
    candidate = (int)(rfrac() * num);
    for (i = candidate; i < NumPlayers; i++) {
	player_t *pl = Player_by_index(i);
	if (Player_can_be_tagged(pl)) {
	    tagItPlayerId = pl->id;
	    break;
	}
    }

    if (tagItPlayerId == NO_ID) {
	for (i = 0; i < candidate; i++) {
	    player_t *pl = Player_by_index(i);
	    if (Player_can_be_tagged(pl)) {
		tagItPlayerId = pl->id;
		break;
	    }
	}
    }

    /* someone should be tagged by now */
    assert(tagItPlayerId != NO_ID);
}




/****************************
 * Functions for explosions.
 */

/* Create debris particles */
void Make_debris(world_t  *world,
		 clpos_t  pos,
		 vector_t vel,
		 int      owner_id,
		 int      owner_team,
		 int      type,
		 double   mass,
		 long     status,
		 int      color,
		 int      radius,
		 int      num_debris,
		 int      min_dir,   int     max_dir,
		 double   min_speed, double  max_speed,
		 double   min_life,  double  max_life)
{
    object_t *debris;
    int i;
    double life;
    modifiers_t mods;
    
    pos = World_wrap_clpos(world, pos);
    if (!World_contains_clpos(world, pos))
	return;

    if (max_life < min_life)
	max_life = min_life;

    if (max_speed < min_speed)
	max_speed = min_speed;

    CLEAR_MODS(mods);

    if (type == OBJ_SHOT) {
	SET_BIT(mods.warhead, CLUSTER);
	if (!options.shotsGravity)
	    CLR_BIT(status, GRAVITY);
    }

    if (num_debris > MAX_TOTAL_SHOTS - NumObjs)
	num_debris = MAX_TOTAL_SHOTS - NumObjs;

    for (i = 0; i < num_debris; i++) {
	double		speed, dx, dy, diroff;
	int		dir, dirplus;

	if ((debris = Object_allocate()) == NULL)
	    break;

	debris->color = color;
	debris->id = owner_id;
	debris->team = owner_team;
	Object_position_init_clpos(world, debris, pos);
	dir = MOD2(min_dir + (int)(rfrac() * (max_dir - min_dir)), RES);
	dirplus = MOD2(dir + 1, RES);
	diroff = rfrac();
	dx = tcos(dir) + (tcos(dirplus) - tcos(dir)) * diroff;
	dy = tsin(dir) + (tsin(dirplus) - tsin(dir)) * diroff;
	speed = min_speed + rfrac() * (max_speed - min_speed);
	debris->vel.x = vel.x + dx * speed;
	debris->vel.y = vel.y + dy * speed;
	debris->acc.x = 0;
	debris->acc.y = 0;
	if (options.shotHitFuelDrainUsesKineticEnergy
	    && type == OBJ_SHOT) {
	    /* compensate so that m*v^2 is constant */
	    double sp_shotsp = speed / options.shotSpeed;

	    debris->mass = mass / (sp_shotsp * sp_shotsp);
	} else
	    debris->mass = mass;
	debris->type = type;
	life = min_life + rfrac() * (max_life - min_life);
	debris->life = life;
	debris->fusetime = 0;
	debris->pl_range = radius;
	debris->pl_radius = radius;
	debris->status = status;
	debris->mods = mods;
	Cell_add_object(world, debris);
    }
}

/* kps - this is really ugly */
extern bool in_move_player;

bool Friction_area_hitfunc(group_t *groupptr, move_t *move)
{
    UNUSED_PARAM(groupptr); UNUSED_PARAM(move);

    if (in_move_player)
	return true;
    return false;
}

/*
 * Handling of group properties
 */
void Team_immunity_init(world_t *world)
{
    int group;

    for (group = 0; group < num_groups; group++) {
	group_t *gp = groupptr_by_id(group);

	if (gp->type == CANNON) {
	    cannon_t *cannon = Cannon_by_index(world, gp->mapobj_ind);

	    assert(cannon->group == group);
	    Cannon_set_hitmask(group, cannon);
	}
    }

#if 0
    /* change hitmask of all cannons */
    P_grouphack(CANNON, Cannon_set_hitmask);
#endif
}

/* kps - called at server startup to initialize hit masks */
void Hitmasks_init(world_t *world)
{
    Target_init(world);
    Team_immunity_init(world);
}

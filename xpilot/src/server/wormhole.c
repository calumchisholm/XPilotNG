/* 
 * XPilot NG, a multiplayer space war game.
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

void Object_hits_wormhole(object_t *obj, int ind)
{
    /*world_t *world = &World;
      wormhole_t *wormhole = Wormhole_by_index(world, ind);*/

    SET_BIT(obj->obj_status, WARPING);
    obj->wormHoleHit = ind;

#if 0
    if (obj->type == OBJ_PLAYER) {
	player_t *pl = (player_t *)obj;

	warn("Player %s hits wormhole %d.", pl->name, ind);
    }
#endif
}


/*
 * Warp balls connected to warped player.
 */
static void Warp_balls(player_t *pl, clpos_t dest)
{
    world_t *world = pl->world;

    /*
     * Don't connect to balls while warping.
     */
    if (Player_uses_connector(pl))
	pl->ball = NULL;

    if (BIT(pl->have, HAS_BALL)) {
	/*
	 * Warp every ball associated with player.
	 * NB. the connector can cross a wall boundary this is
	 * allowed, so long as the ball itself doesn't collide.
	 */
	int k;

	for (k = 0; k < NumObjs; k++) {
	    object_t *b = Obj[k];

	    if (b->type == OBJ_BALL && b->id == pl->id) {
		clpos_t ballpos;
		hitmask_t hitmask = BALL_BIT|HITMASK(pl->team);

		ballpos.cx = b->pos.cx + dest.cx - pl->pos.cx;
		ballpos.cy = b->pos.cy + dest.cy - pl->pos.cy;
		ballpos = World_wrap_clpos(world, ballpos);
		if (!World_contains_clpos(world, ballpos)
		    || (shape_is_inside(ballpos.cx, ballpos.cy, hitmask,
					(object_t *)b, &ball_wire, 0)
			!= NO_GROUP)) {
		    b->life = 0.0;
		    continue;
		}
		Object_position_set_clpos(world, b, ballpos);
		Object_position_remember(b);
		b->vel.x *= WORM_BRAKE_FACTOR;
		b->vel.y *= WORM_BRAKE_FACTOR;
		Cell_add_object(world, b);
	    }
	}
    }
}


static int Find_wormhole_dest(world_t *world, wormhole_t *wh_hit, player_t *pl)
{
    int wh_dest, wcx, wcy, nearestFront, nearestRear;
    double proximity, proxFront, proxRear;

    if (wh_hit->type == WORM_FIXED)
	return pl->wormHoleHit;

    if (wh_hit->countdown > 0)
	return wh_hit->lastdest;

    if (rfrac() < 0.1) {
	do
	    wh_dest = (int)(rfrac() * world->NumWormholes);
	while (world->wormholes[wh_dest].type == WORM_IN
	       || pl->wormHoleHit == wh_dest);
	return wh_dest;
    }

    nearestFront = nearestRear = -1;
    proxFront = proxRear = 1e20;

    for (wh_dest = 0; wh_dest < world->NumWormholes; wh_dest++) {
	wormhole_t *wh = Wormhole_by_index(world, wh_dest);

	if (wh_dest == pl->wormHoleHit
	    || wh->type == WORM_IN)
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

    return wh_dest;
}


/*
 * Move player trough wormhole.
 */
void Traverse_wormhole(player_t *pl)
{
    clpos_t dest;
    int wh_dest;
    world_t *world = pl->world;
    wormhole_t *wh_hit = Wormhole_by_index(world, pl->wormHoleHit);

#if 0
    warn("player %s is in Traverse_wormhole", pl->name);
#endif

    wh_dest = Find_wormhole_dest(world, wh_hit, pl);

    sound_play_sensors(pl->pos, WORM_HOLE_SOUND);
    dest = world->wormholes[wh_dest].pos;

    Warp_balls(pl, dest);

    pl->wormHoleDest = wh_dest;
    Player_position_init_clpos(pl, dest);
    pl->vel.x *= WORM_BRAKE_FACTOR;
    pl->vel.y *= WORM_BRAKE_FACTOR;
    pl->forceVisible += 15;

    assert(pl->wormHoleHit != -1);

    if (wh_dest != pl->wormHoleHit) {
	wh_hit->lastdest = wh_dest;
	wh_hit->countdown = options.wormholeStableTicks;
    }

    CLR_BIT(pl->obj_status, WARPING);
    SET_BIT(pl->obj_status, WARPED);

    sound_play_sensors(pl->pos, WORM_HOLE_SOUND);
}

/*
 * Player has used hyperjump item.
 */
void Hyperjump(player_t *pl)
{
    clpos_t dest;
    world_t *world = pl->world;
    int counter;
    hitmask_t hitmask = NONBALL_BIT | HITMASK(pl->team); /* kps - ok ? */

    /* Try to find empty space to hyperjump to. */
    for (counter = 20; counter > 0; counter--) {
	dest = World_get_random_clpos(world);
	if (shape_is_inside(dest.cx, dest.cy, hitmask,
			    (object_t *)pl, (shape_t *)pl->ship,
			    pl->dir)
	    == NO_GROUP)
	    break;
    }

    /* We can't find an empty space, hyperjump failed. */
    if (!counter) {
	/* need to do something else here ? */
	Set_player_message(pl, "Could not hyperjump. [*Server notice*]");
	CLR_BIT(pl->obj_status, WARPING);
	sound_play_sensors(pl->pos, HYPERJUMP_SOUND);
	return;
    }

    sound_play_sensors(pl->pos, HYPERJUMP_SOUND);

    Warp_balls(pl, dest);

    Player_position_init_clpos(pl, dest);
    pl->vel.x *= WORM_BRAKE_FACTOR;
    pl->vel.y *= WORM_BRAKE_FACTOR;
    pl->forceVisible += 15;

    CLR_BIT(pl->obj_status, WARPING);
}



/*
 * Wormhole specific functions
 */
hitmask_t Wormhole_hitmask(wormhole_t *wormhole)
{
    if (wormhole->type == WORM_OUT)
	return ALL_BITS;
    return 0;
}

bool Wormhole_hitfunc(group_t *gp, move_t *move)
{
    object_t *obj = move->obj;
    world_t *world = &World;
    wormhole_t *wormhole = Wormhole_by_index(world, gp->mapobj_ind);

    if (wormhole->type == WORM_OUT)
	return false;

    if (obj == NULL)
	return true;

    if (BIT(obj->obj_status, WARPED|WARPING))
	return false;

    return true;

#if 0
    warn("Wormhole_hitfunc: wormhole %p hit by a %s",
	 wormhole, Object_typename(move->obj));
#endif
    if (obj->type == OBJ_PLAYER) {
	player_t *pl = (player_t *)obj;
	if (pl->wormHoleHit == gp->mapobj_ind)
	    return false;
    }

    return true;

#if 0

    if (obj->type == OBJ_PLAYER) {
	player_t *pl = (player_t *)obj;

	if (BIT(pl->obj_status, WARPING))
	    return false;
#if 0
	/*
	 * Don't warp again if we are still on the
	 * same wormhole we have just been warped to.
	 */
	if (pl->warped > 0
	    && wormhole->type == WORM_NORMAL
	    && wormhole == Wormhole_by_index(pl->wormHoleDest)
	    /* kps - wormHoleDest is now pointer */
	    /*&& pl->wormHoleDest == ind */)
	    return false;
#endif
    } else {
	    /*int last = wormhole->lastdest;*/

	return false;
    }

    return true;
#endif
}

void World_remove_wormhole(world_t *world, wormhole_t *wormhole)
{
    blkpos_t blk = Clpos_to_blkpos(wormhole->pos);

    World_set_block(world, blk, wormhole->lastblock);
}

void Verify_wormhole_consistency(world_t *world)
{
    int i;
    int	worm_in = 0,
	worm_out = 0,
	worm_norm = 0;

    /* count wormhole types */
    for (i = 0; i < world->NumWormholes; i++) {
	int type = world->wormholes[i].type;
	if (type == WORM_NORMAL)
	    worm_norm++;
	else if (type == WORM_IN)
	    worm_in++;
	else if (type == WORM_OUT)
	    worm_out++;
    }

    /*
     * Verify that the wormholes are consistent, i.e. that if
     * we have no 'out' wormholes, make sure that we don't have
     * any 'in' wormholes, and (less critical) if we have no 'in'
     * wormholes, make sure that we don't have any 'out' wormholes.
     */
    if ((worm_norm) ? (worm_norm + worm_out < 2)
	: (worm_in) ? (worm_out < 1)
	: (worm_out > 0)) {

	xpprintf("Inconsistent use of wormholes, removing them.\n");
	for (i = 0; i < world->NumWormholes; i++)
	    World_remove_wormhole(world, Wormhole_by_index(world, i));
	world->NumWormholes = 0;
    }

    if (!options.wormholeStableTicks) {
	for (i = 0; i < world->NumWormholes; i++) {
	    int j = (int)(rfrac() * world->NumWormholes);

	    while (Wormhole_by_index(world, j)->type == WORM_IN)
		j = (int)(rfrac() * world->NumWormholes);
	    Wormhole_by_index(world, i)->lastdest = j;
	}
    }
}

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

char collision_version[] = VERSION;

/* new acd functions */
/* doubles because the multiplies might overflow ints */
static inline bool in_range_acd(double dx, double dy, double dvx, double dvy,
				double r)
{
    double tmin, fminx, fminy;
    double top, bot;

    dx = CENTER_XCLICK(dx);
    dy = CENTER_YCLICK(dy);

    if (dx * dx + dy * dy < r * r)
	return true;
    top = -(dvx * dx + dvy * dy);
    bot = dvx * dvx + dvy * dvy;
    if (top < 0 || bot < CLICK * CLICK || top > bot)
	return false;
    tmin = top / bot;
    fminx = dx + dvx * tmin;
    fminy = dy + dvy * tmin;
    if (fminx * fminx + fminy * fminy < r * r)
	return true;
    else
	return false;
}

static inline bool in_range_simple(int px, int py, int qx, int qy, double r)
{
    int dx = px - qx, dy = py - qy;

    dx = CENTER_XCLICK(dx);
    dy = CENTER_YCLICK(dy);

    if ((double)dx * dx + (double)dy * dy < r * r)
	return true;
    else
	return false;
}

static inline bool in_range_partial(double dx, double dy,
				    double dvx, double dvy,
				    double r, double wall_time)
{
    double tmin, fminx, fminy;
    double top, bot;

    dx = CENTER_XCLICK(dx);
    dy = CENTER_YCLICK(dy);

    top = -(dvx * dx + dvy * dy);
    bot = dvx * dvx + dvy * dvy;
    if (top <= 0)
	return false;
    if (bot < 5 * CLICK * CLICK || top >= bot)
	tmin = wall_time;
    else {
	tmin = top / bot;
	tmin = MIN(tmin, wall_time);
    }
    fminx = dx + dvx * tmin;
    fminy = dy + dvy * tmin;
    if (fminx * fminx + fminy * fminy < r * r)
	return true;
    else
	return false;
}

/* Collmodes:
   0 - Object has not been moved in walls.c after it was created.
       Check only whether end-of-frame position is on top of a
       player.
   1 - Object was moved in walls.c. It did not hit walls and
       therefore moved at constant speed from obj->prevpos to
       obj->pos. Check whether it was within range during movement
       using analytical collision detection.
   2 - Object was moving from obj->prevpos by obj->extmove but it
       hit a wall and was destroyed after completing obj->wall_time
       of the distance. Check whether it was within range similarly
       to case 1 but only consider hits at the beginning of the
       frame before obj->wall_time.
   3 - Object bounced off a wall at least once without getting
       destroyed. Checking all the linear parts of movement
       separately is not implemented yet so we don't detect any
       possible collisions. Note that it would already be possible
       to check the first linear part until obj->wall_time similarly
       to case 2. This is not done because we lack the information
       needed to calculate the effect of non-fatal hits. The
       direction and speed of the object at the moment of impact
       were likely completely different from the end-of-frame values
       we have now.

       Different collision modes for players have not been implemented
       yet. It's supposed that they move in a straight line from
       prevpos to pos. This can lead to some erroneous hits.
*/
static bool in_range(object_t *obj1, object_t *obj2, double range)
{
    bool hit;

    switch (obj2->collmode) {
    case 0:
	hit = in_range_simple(obj1->pos.cx, obj1->pos.cy,
			      obj2->pos.cx, obj2->pos.cy,
			      range);
	break;
    case 1:
	hit = in_range_acd((double)(obj1->prevpos.cx - obj2->prevpos.cx),
			   (double)(obj1->prevpos.cy - obj2->prevpos.cy),
			   (double)(obj1->extmove.cx - obj2->extmove.cx),
			   (double)(obj1->extmove.cy - obj2->extmove.cy),
			   range);
	break;
    case 2:
	hit = in_range_partial((double)(obj1->prevpos.cx - obj2->prevpos.cx),
			       (double)(obj1->prevpos.cy - obj2->prevpos.cy),
			       (double)(obj1->extmove.cx - obj2->extmove.cx),
			       (double)(obj1->extmove.cy - obj2->extmove.cy),
			       range, obj2->wall_time);
	break;
    case 3:
    default:
#if 0
	warn("Unimplemented collision mode %d", obj2->collmode);
#endif
	return false;
    }
    return hit;
}

static void PlayerCollision(world_t *world);
static void PlayerObjectCollision(player_t *pl);
static void AsteroidCollision(world_t *world);
static void BallCollision(world_t *world);
static void MineCollision(world_t *world);
static void Player_collides_with_ball(player_t *pl, object_t *obj);
static void Player_collides_with_item(player_t *pl, object_t *obj);
static void Player_collides_with_mine(player_t *pl, object_t *obj);
static void Player_collides_with_debris(player_t *pl, object_t *obj);
static void Player_collides_with_asteroid(player_t *pl, wireobject_t *obj);
static void Player_collides_with_killing_shot(player_t *pl, object_t *obj);


void Check_collision(world_t *world)
{
    BallCollision(world);
    MineCollision(world);
    PlayerCollision(world);
    AsteroidCollision(world);
}


static void PlayerCollision(world_t *world)
{
    int i, j;
    double sc, sc2;
    player_t *pl;

    /* Player - player, checkpoint, treasure, object and wall */
    for (i = 0; i < NumPlayers; i++) {
	pl = Player_by_index(i);
	if (!Player_is_playing(pl))
	    continue;

	if (!World_contains_clpos(world, pl->pos)) {
	    SET_BIT(pl->status, KILLED);
	    Set_message_f("%s left the known universe.", pl->name);
	    sc = Rate(WALL_SCORE, pl->score);
	    Score(pl, -sc, pl->pos, pl->name);
	    continue;
	}

	if (BIT(pl->used, HAS_PHASING_DEVICE))
	    continue;

	/* Player - player */
	if (BIT(world->rules->mode, CRASH_WITH_PLAYER | BOUNCE_WITH_PLAYER)) {
	    for (j = i + 1; j < NumPlayers; j++) {
		player_t *pl_j = Player_by_index(j);
		double range;

		if (!Player_is_playing(pl_j))
		    continue;
		if (BIT(pl_j->used, HAS_PHASING_DEVICE))
		    continue;

		range = (2*SHIP_SZ-6) * CLICK;
		if (!in_range(OBJ_PTR(pl), OBJ_PTR(pl_j), range))
		    continue;

		/*
		 * Here we can add code to do more accurate player against
		 * player collision detection.
		 * A new algorithm could be based on the following idea:
		 *
		 * - If we can draw an uninterupted line between two players:
		 *   - Then test for both ships:
		 *     - For the three points which make up a ship:
		 *       - If we can draw a line between its previous
		 *         position and its current position which does not
		 *         cross the first line.
		 * Then the ships have not collided even though they may be
		 * very close to one another.
		 * The choosing of the first line may not be easy however.
		 */

		if (Team_immune(pl->id, pl_j->id) || PSEUDO_TEAM(pl, pl_j))
		    continue;

		sound_play_sensors(pl->pos, PLAYER_HIT_PLAYER_SOUND);
		if (BIT(world->rules->mode, BOUNCE_WITH_PLAYER)) {
		    if (!Player_used_emergency_shield(pl)) {
			Player_add_fuel(pl, ED_PL_CRASH);
			Item_damage(pl, options.destroyItemInCollisionProb);
		    }
		    if (!Player_used_emergency_shield(pl_j)) {
			Player_add_fuel(pl_j, ED_PL_CRASH);
			Item_damage(pl_j, options.destroyItemInCollisionProb);
		    }
		    pl->forceVisible = 20;
		    pl_j->forceVisible = 20;
		    Obj_repel((object_t *)pl, (object_t *)pl_j,
			      PIXEL_TO_CLICK(2*SHIP_SZ));
		}
		if (!BIT(world->rules->mode, CRASH_WITH_PLAYER))
		    continue;

		if (pl->fuel.sum <= 0.0
		    || (!BIT(pl->used, HAS_SHIELD)
			&& !BIT(pl->have, HAS_ARMOR)))
		    SET_BIT(pl->status, KILLED);

		if (pl_j->fuel.sum <= 0.0
		    || (!BIT(pl_j->used, HAS_SHIELD)
			&& !BIT(pl_j->have, HAS_ARMOR)))
		    SET_BIT(pl_j->status, KILLED);

		if (!BIT(pl->used, HAS_SHIELD)
		    && BIT(pl->have, HAS_ARMOR))
		    Player_hit_armor(pl);

		if (!BIT(pl_j->used, HAS_SHIELD)
		    && BIT(pl_j->have, HAS_ARMOR))
		    Player_hit_armor(pl_j);

		if (BIT(pl_j->status, KILLED)) {
		    if (BIT(pl->status, KILLED)) {
			Set_message_f("%s and %s crashed.",
				      pl->name, pl_j->name);
			if (!Player_is_tank(pl) && !Player_is_tank(pl_j)) {
			    sc = Rate(pl_j->score, pl->score)
				* options.crashScoreMult;
			    sc2 = Rate(pl->score, pl_j->score)
				* options.crashScoreMult;
			    Score_players(pl, -sc, pl_j->name,
					  pl_j, -sc2, pl->name, false);
			}
			else if (Player_is_tank(pl)) {
			    player_t *i_tank_owner_pl
				= Player_by_id(pl->lock.pl_id);

			    sc = Rate(i_tank_owner_pl->score, pl_j->score)
				* options.tankKillScoreMult;
			    Score_players(i_tank_owner_pl, sc, pl_j->name,
					  pl_j, -sc, pl->name, true);
			}
			else if (Player_is_tank(pl_j)) {
			    player_t *j_tank_owner_pl

				= Player_by_id(pl_j->lock.pl_id);
			    sc = Rate(j_tank_owner_pl->score, pl->score)
				* options.tankKillScoreMult;
			    Score_players(j_tank_owner_pl, sc, pl->name,
					  pl, -sc, pl_j->name, true);
			} /* don't bother scoring two tanks */
		    } else {
			int i_tank_owner = i;
			player_t *i_tank_owner_pl;

			if (Player_is_tank(pl)) {
			    i_tank_owner = GetInd(pl->lock.pl_id);
			    if (i_tank_owner == j)
				i_tank_owner = i;
			}
			i_tank_owner_pl = Player_by_index(i_tank_owner);
			Set_message_f("%s ran over %s.", pl->name, pl_j->name);
			sound_play_sensors(pl_j->pos,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			if (Player_is_tank(pl)) {
			    Rank_add_tank_kill(i_tank_owner_pl);
			    sc = Rate(i_tank_owner_pl->score, pl_j->score)
				* options.tankKillScoreMult;
			} else {
			    Rank_add_runover_kill(pl);
			    sc = Rate(pl->score, pl_j->score)
				* options.runoverKillScoreMult;
			}
			Score_players(i_tank_owner_pl, sc, pl_j->name,
				      pl_j, -sc, pl->name, true);
		    }

		} else {
		    if (BIT(pl->status, KILLED)) {
			int j_tank_owner = j;
			player_t *j_tank_owner_pl;

			if (Player_is_tank(pl_j)) {
			    j_tank_owner = GetInd(pl_j->lock.pl_id);
			    if (j_tank_owner == i)
				j_tank_owner = j;
			}
			j_tank_owner_pl = Player_by_index(j_tank_owner);
			Set_message_f("%s ran over %s.", pl_j->name, pl->name);
			sound_play_sensors(pl->pos,
					   PLAYER_RAN_OVER_PLAYER_SOUND);
			if (Player_is_tank(pl_j)) {
			    Rank_add_tank_kill(j_tank_owner_pl);
			    sc = Rate(j_tank_owner_pl->score, pl->score)
				* options.tankKillScoreMult;
			} else {
			    Rank_add_runover_kill(pl_j);
			    sc = Rate(pl_j->score, pl->score)
				* options.runoverKillScoreMult;
			}

			Score_players(j_tank_owner_pl, sc, pl->name,
				      pl, -sc, pl_j->name, true);
		    }
		}

		if (BIT(pl_j->status, KILLED)) {
		    if (Player_is_robot(pl_j)
			&& Robot_war_on_player(pl_j) == pl->id)
			Robot_reset_war(pl_j);
		}

		if (BIT(pl->status, KILLED)) {
		    if (Player_is_robot(pl)
			&& Robot_war_on_player(pl) == pl_j->id)
			Robot_reset_war(pl);
		}

		/* cannot crash with more than one player at the same time? */
		/* if 3 players meet at the same point at the same time? */
		/* break; */

	    }
	}

	/* Player picking up ball/treasure */
	if (!BIT(pl->used, HAS_CONNECTOR)
	    || BIT(pl->used, HAS_PHASING_DEVICE))
	    pl->ball = NULL;
	else if (pl->ball != NULL) {
	    ballobject_t *ball = pl->ball;

	    if (ball->life <= 0.0 || ball->id != NO_ID)
		pl->ball = NULL;
	    else {
		double distance = Wrap_length(pl->pos.cx - ball->pos.cx,
					      pl->pos.cy - ball->pos.cy);
		int group;

		if (distance >= options.ballConnectorLength * CLICK) {
		    ball->id = pl->id;
		    /* this is only the team of the owner of the ball,
		     * not the team the ball belongs to. the latter is
		     * found through the ball's treasure */
		    ball->team = pl->team;
		    if (ball->treasure->have)
			ball->life = 1e6;  /* for frame counter */
		    ball->owner = pl->id;
		    SET_BIT(ball->status, GRAVITY);
		    ball->treasure->have = false;
		    SET_BIT(pl->have, HAS_BALL);
		    pl->ball = NULL;
		    sound_play_sensors(pl->pos, CONNECT_BALL_SOUND);
		    {
			/* The ball might already be inside the team's ball
			 * target. */
			group = shape_is_inside(ball->pos.cx,
						ball->pos.cy,
						BALL_BIT | HITMASK(pl->team),
						(object_t *)ball,
						&ball_wire, 0);
			if (group != NO_GROUP) {
			    Ball_hits_goal(ball, groupptr_by_id(group));
			    ball->life = 0.0;
			}
		    }
		}
	    }
	} else {
	    /*
	     * We want a separate list of balls to avoid searching
	     * the object list for balls.
	     */
	    double dist, mindist = options.ballConnectorLength * CLICK;

	    for (j = 0; j < NumObjs; j++) {
		object_t *obj = Obj[j];

		if (obj->type == OBJ_BALL
		    && obj->id == NO_ID) {
		    dist = Wrap_length(pl->pos.cx - obj->pos.cx,
				       pl->pos.cy - obj->pos.cy);
		    if (dist < mindist) {
			ballobject_t *ball = BALL_PTR(obj);

			/*
			 * The treasure's team cannot connect before
			 * somebody else has owned the ball.
			 * This was done to stop team members
			 * taking and hiding with the ball... this was
			 * considered bad gamesmanship.
			 */
			if (BIT(world->rules->mode, TEAM_PLAY)
			    && ball->treasure->have
			    && pl->team == ball->treasure->team)
			    continue;
			pl->ball = ball;
			mindist = dist;
		    }
		}
	    }
	}

	PlayerObjectCollision(pl);
	PlayerCheckpointCollision(pl);
    }
}

int IsOffensiveItem(enum Item i)
{
    if (BIT(1 << i,
	    ITEM_BIT_WIDEANGLE |
	    ITEM_BIT_REARSHOT |
	    ITEM_BIT_MINE |
	    ITEM_BIT_MISSILE |
	    ITEM_BIT_LASER))
	return true;
    return false;
}

int IsDefensiveItem(enum Item i)
{
    if (BIT(1 << i,
	    ITEM_BIT_CLOAK |
	    ITEM_BIT_ECM |
	    ITEM_BIT_TRANSPORTER |
	    ITEM_BIT_TRACTOR_BEAM |
	    ITEM_BIT_EMERGENCY_SHIELD |
	    ITEM_BIT_MIRROR |
	    ITEM_BIT_DEFLECTOR |
	    ITEM_BIT_HYPERJUMP |
	    ITEM_BIT_PHASING |
	    ITEM_BIT_TANK |
	    ITEM_BIT_ARMOR))
	return true;
    return false;
}

int CountOffensiveItems(player_t *pl)
{
    return (pl->item[ITEM_WIDEANGLE] + pl->item[ITEM_REARSHOT] +
	    pl->item[ITEM_MINE] + pl->item[ITEM_MISSILE] +
	    pl->item[ITEM_LASER]);
}

int CountDefensiveItems(player_t *pl)
{
    int count;

    count = pl->item[ITEM_CLOAK] + pl->item[ITEM_ECM] + pl->item[ITEM_ARMOR] +
	    pl->item[ITEM_TRANSPORTER] + pl->item[ITEM_TRACTOR_BEAM] +
	    pl->item[ITEM_EMERGENCY_SHIELD] + pl->fuel.num_tanks +
	    pl->item[ITEM_DEFLECTOR] + pl->item[ITEM_HYPERJUMP] +
	    pl->item[ITEM_PHASING] + pl->item[ITEM_MIRROR];
    if (pl->emergency_shield_left > 0)
 	count++;
    if (pl->phasing_left > 0)
	count++;
    return count;
}

static inline double collision_cost(double mass, double speed)
{
    /*
     * kps - this was ABS(2 * mass * speed), because fuel related
     * values used to be multiplied by 256 in older code.
     */
    return ABS(mass * speed / 128.0);
}

static void PlayerObjectCollision(player_t *pl)
{
    int j, obj_count;
    double range, radius;
    object_t *obj, **obj_list;
    world_t *world = pl->world;

    /*
     * Collision between a player and an object.
     */
    if (!Player_is_playing(pl))
	return;

    if (ObjCount >= options.cellGetObjectsThreshold)
	Cell_get_objects(world, pl->pos, 4, 500, &obj_list, &obj_count);
    else {
	obj_list = Obj;
	obj_count = ObjCount;
    }
   
    for (j = 0; j < obj_count; j++) {
	bool hit;

	obj = obj_list[j];

	range = (SHIP_SZ + obj->pl_range) * CLICK;
	if (!in_range(OBJ_PTR(pl), obj, range))
	    continue;

	if (obj->id != NO_ID) {
	    if (obj->id == pl->id) {
		if ((obj->type == OBJ_SPARK
		     || obj->type == OBJ_MINE)
		    && BIT(obj->status, OWNERIMMUNE))
		    continue;
		else if (options.selfImmunity)
		    continue;
	    } else if (options.selfImmunity &&
		       Player_is_tank(pl) &&
		       (pl->lock.pl_id == obj->id))
		continue;
	    else if (Team_immune(obj->id, pl->id))
		continue;
	    else if (BIT(Player_by_id(obj->id)->status, PAUSE))
		continue;
	} else if (BIT(world->rules->mode, TEAM_PLAY)
		   && options.teamImmunity
		   && obj->team == pl->team
		   /* allow players to destroy their team's unowned balls */
		   && obj->type != OBJ_BALL)
	    continue;

	if (obj->type == OBJ_ITEM) {
	    if (BIT(pl->used, HAS_SHIELD) && !options.shieldedItemPickup) {
		SET_BIT(obj->status, GRAVITY);
		Delta_mv((object_t *)pl, obj);
		continue;
	    }
	}
	else if (obj->type == OBJ_HEAT_SHOT
		 || obj->type == OBJ_SMART_SHOT
		 || obj->type == OBJ_TORPEDO
		 || obj->type == OBJ_SHOT
		 || obj->type == OBJ_CANNON_SHOT) {
	    if (pl->id == obj->id && frame_time < obj->fusetime)
		continue;
	}
	else if (obj->type == OBJ_MINE) {
	    if (BIT(obj->status, CONFUSED))
		continue;
	}
	else if (obj->type == OBJ_BALL
		 && obj->id != NO_ID) {
	    if (BIT(Player_by_id(obj->id)->used, HAS_PHASING_DEVICE))
		continue;
	}
	else if (obj->type == OBJ_PULSE) {
	    pulseobject_t *pulse = PULSE_PTR(obj);

	    if (pl->id == pulse->id && !pulse->refl)
		continue;
	}
	/*
	 * Objects actually only hit the player if they are really close.
	 */
	radius = (SHIP_SZ + obj->pl_radius) * CLICK;

	/*
	 * kps - why was radius used in 4.3.1X and range in 4.5.4 ?
	 */
	if (radius >= range)
	    hit = true;
	else
	    hit = in_range(OBJ_PTR(pl), obj, radius);

#if 0
	if (obj->collmode != 1) {
	    char MSG[80];
	    sprintf(MSG, "Collision type=%d, hit=%d, cm=%d, time=%f, "
		    "frame=%ld [*DEBUG*]", obj->type, hit, obj->collmode,
		    obj->wall_time, frame_loops);
	    Set_message(MSG);
	}
#endif

	/*
	 * Object collision.
	 */
	switch (obj->type) {
	case OBJ_BALL:
	    if (!hit)
		continue;
	    Player_collides_with_ball(pl, obj);
	    if (BIT(pl->status, KILLED))
		return;
	    continue;

	case OBJ_ITEM:
	    Player_collides_with_item(pl, obj);
	    /* if life is non-zero then no collision occurred */
	    if (obj->life != 0)
		continue;
	    break;

	case OBJ_MINE:
	    Player_collides_with_mine(pl, obj);
	    break;

	case OBJ_WRECKAGE:
	case OBJ_DEBRIS:
	    Player_collides_with_debris(pl, obj);
	    if (BIT(pl->status, KILLED))
		return;
	    break;

	case OBJ_ASTEROID:
	    if (hit) {
		Player_collides_with_asteroid(pl, WIRE_PTR(obj));
		Delta_mv_elastic((object_t *)pl, (object_t *)obj);
	    }
	    if (BIT(pl->status, KILLED))
		return;
	    continue;

	case OBJ_CANNON_SHOT:
	    /* don't explode cannon flak if it hits directly*/
	    CLR_BIT(obj->mods.warhead, CLUSTER);
	    break;

	case OBJ_PULSE:
	    Laser_pulse_hits_player(pl, PULSE_PTR(obj));
	    if (BIT(pl->status, KILLED))
		return;
	    continue;

	default:
	    break;
	}

	obj->life = 0;

	if (BIT(OBJ_TYPEBIT(obj->type), KILLING_SHOTS)) {
	    Player_collides_with_killing_shot(pl, obj);
	    if (BIT(pl->status, KILLED))
		return;
	}

	if (hit)
	    Delta_mv((object_t *)pl, (object_t *)obj);
    }
}


static void Player_collides_with_ball(player_t *pl, object_t *obj)
{
    double sc;
    ballobject_t *ball = BALL_PTR(obj);

    /*
     * The ball is special, usually players bounce off of it with
     * shields up, or die with shields down.  The treasure may
     * be destroyed.
     */
    Delta_mv((object_t *)pl, obj);
    if (!Player_used_emergency_shield(pl)) {
	Player_add_fuel(pl, ED_BALL_HIT);
	if (options.treasureCollisionDestroys)
	    ball->life = 0;
    }
    if (pl->fuel.sum > 0) {
	if (!options.treasureCollisionMayKill || BIT(pl->used, HAS_SHIELD))
	    return;
	if (!BIT(pl->used, HAS_SHIELD) && BIT(pl->have, HAS_ARMOR)) {
	    Player_hit_armor(pl);
	    return;
	}
    }
    if (ball->owner == NO_ID) {
	Set_message_f("%s was killed by a ball.", pl->name);
	sc = Rate(0.0, pl->score)
		* options.ballKillScoreMult
		* options.unownedKillScoreMult;
	Score(pl, -sc, pl->pos, "Ball");
    } else {
	player_t *kp = Player_by_id(ball->owner);

	Set_message_f("%s was killed by a ball owned by %s.%s",
		      pl->name, kp->name,
		      kp->id == pl->id ? "  How strange!" : "");

	if (kp->id == pl->id) {
	    sc = Rate(0.0, pl->score)
		   * options.ballKillScoreMult
		   * options.selfKillScoreMult;
	    Score(pl, -sc, pl->pos, kp->name);
	} else {
	    Rank_add_ball_kill(kp);
	    sc = Rate(kp->score, pl->score) * options.ballKillScoreMult;
	    Score_players(kp, sc, pl->name, pl, -sc, kp->name, true);
	    Robot_war(pl, kp);
	}
    }
    SET_BIT(pl->status, KILLED);
}


static void Player_collides_with_item(player_t *pl, object_t *obj)
{
    int old_have;
    enum Item item_index;
    world_t *world = pl->world;

    if (IsOffensiveItem((enum Item) obj->info)) {
	int off_items = CountOffensiveItems(pl);

	if (off_items >= options.maxOffensiveItems) {
	    /* Set_player_message(pl, "No space left for offensive items."); */
	    Delta_mv((object_t *)pl, obj);
	    return;
	}
	else if (obj->count > 1
		 && off_items + obj->count > options.maxOffensiveItems)
	    obj->count = options.maxOffensiveItems - off_items;
    }
    else if (IsDefensiveItem((enum Item) obj->info)) {
	int def_items = CountDefensiveItems(pl);

	if (def_items >= options.maxDefensiveItems) {
	    /* Set_player_message(pl,
	       "No space for left for defensive items."); */
	    Delta_mv((object_t *)pl, obj);
	    return;
	}
	else if (obj->count > 1
		 && def_items + obj->count > options.maxDefensiveItems)
	    obj->count = options.maxDefensiveItems - def_items;
    }

    item_index = (enum Item) obj->info;

    switch (item_index) {
    case ITEM_WIDEANGLE:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, WIDEANGLE_SHOT_PICKUP_SOUND);
	break;
    case ITEM_ECM:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, ECM_PICKUP_SOUND);
	break;
    case ITEM_ARMOR:
	pl->item[item_index]++;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_ARMOR);
	sound_play_sensors(pl->pos, ARMOR_PICKUP_SOUND);
	break;
    case ITEM_TRANSPORTER:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, TRANSPORTER_PICKUP_SOUND);
	break;
    case ITEM_MIRROR:
	pl->item[ITEM_MIRROR] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_MIRROR);
	sound_play_sensors(pl->pos, MIRROR_PICKUP_SOUND);
	break;
    case ITEM_DEFLECTOR:
	pl->item[ITEM_DEFLECTOR] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_DEFLECTOR);
	sound_play_sensors(pl->pos, DEFLECTOR_PICKUP_SOUND);
	break;
    case ITEM_HYPERJUMP:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, HYPERJUMP_PICKUP_SOUND);
	break;
    case ITEM_PHASING:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_PHASING_DEVICE);
	sound_play_sensors(pl->pos, PHASING_DEVICE_PICKUP_SOUND);
	break;
    case ITEM_SENSOR:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	pl->updateVisibility = 1;
	sound_play_sensors(pl->pos, SENSOR_PACK_PICKUP_SOUND);
	break;
    case ITEM_AFTERBURNER:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_AFTERBURNER);
	sound_play_sensors(pl->pos, AFTERBURNER_PICKUP_SOUND);
	break;
    case ITEM_REARSHOT:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, BACK_SHOT_PICKUP_SOUND);
	break;
    case ITEM_MISSILE:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, ROCKET_PACK_PICKUP_SOUND);
	break;
    case ITEM_CLOAK:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_CLOAKING_DEVICE);
	pl->updateVisibility = 1;
	sound_play_sensors(pl->pos, CLOAKING_DEVICE_PICKUP_SOUND);
	break;
    case ITEM_FUEL:
	Player_add_fuel(pl, ENERGY_PACK_FUEL);
	sound_play_sensors(pl->pos, ENERGY_PACK_PICKUP_SOUND);
	break;
    case ITEM_MINE:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, MINE_PACK_PICKUP_SOUND);
	break;
    case ITEM_LASER:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	sound_play_sensors(pl->pos, LASER_PICKUP_SOUND);
	break;
    case ITEM_EMERGENCY_THRUST:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_EMERGENCY_THRUST);
	sound_play_sensors(pl->pos, EMERGENCY_THRUST_PICKUP_SOUND);
	break;
    case ITEM_EMERGENCY_SHIELD:
	old_have = pl->have;
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_EMERGENCY_SHIELD);
	sound_play_sensors(pl->pos, EMERGENCY_SHIELD_PICKUP_SOUND);
	/*
	 * New feature since 3.2.7:
	 * If we're playing in a map where shields are not allowed
	 * and a player picks up her first emergency shield item
	 * then we'll immediately turn on emergency shield.
	 */
	if (!BIT(old_have, HAS_SHIELD | HAS_EMERGENCY_SHIELD)
	    && pl->item[ITEM_EMERGENCY_SHIELD] == 1)
	    Emergency_shield(pl, true);
	break;
    case ITEM_TRACTOR_BEAM:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_TRACTOR_BEAM);
	sound_play_sensors(pl->pos, TRACTOR_BEAM_PICKUP_SOUND);
	break;
    case ITEM_AUTOPILOT:
	pl->item[item_index] += (int)obj->count;
	LIMIT(pl->item[item_index], 0, world->items[item_index].limit);
	if (pl->item[item_index] > 0)
	    SET_BIT(pl->have, HAS_AUTOPILOT);
	sound_play_sensors(pl->pos, AUTOPILOT_PICKUP_SOUND);
	break;

    case ITEM_TANK:
	if (pl->fuel.num_tanks < world->items[ITEM_TANK].limit)
	    Player_add_tank(pl, TANK_FUEL(pl->fuel.num_tanks + 1));
	else
	    Player_add_fuel(pl, TANK_FUEL(MAX_TANKS));
	sound_play_sensors(pl->pos, TANK_PICKUP_SOUND);
	break;
    case NUM_ITEMS:
	/* impossible */
	break;
    default:
	warn("Player_collides_with_item: unknown item.");
	break;
    }

    obj->life = 0.0;
}


static void Player_collides_with_mine(player_t *pl, object_t *obj)
{
    player_t *kp = NULL;
    double sc;
    mineobject_t *mine = MINE_PTR(obj);

    sound_play_sensors(pl->pos, PLAYER_HIT_MINE_SOUND);
    if (mine->id == NO_ID && mine->owner == NO_ID)
	Set_message_f("%s hit %s.",
		      pl->name,
		      Describe_shot(mine->type, mine->status, mine->mods, 1));
    else if (mine->owner == mine->id) {
	kp = Player_by_id(mine->owner);
	Set_message_f("%s hit %s %s by %s.", pl->name,
		      Describe_shot(mine->type, mine->status, mine->mods,1),
		      BIT(mine->status, GRAVITY) ? "thrown " : "dropped ",
		      kp->name);
    }
    else if (mine->owner == NO_ID) {
	const char *reprogrammer_name = "some jerk";

	if (mine->id != NO_ID) {
	    kp = Player_by_id(mine->id);
	    reprogrammer_name = kp->name;
	}
	Set_message_f("%s hit %s reprogrammed by %s.",
		      pl->name,
		      Describe_shot(mine->type, mine->status, mine->mods, 1),
		      reprogrammer_name);
    }
    else {
	const char *reprogrammer_name = "some jerk";

	if (mine->id != NO_ID) {
	    kp = Player_by_id(mine->id);
	    reprogrammer_name = kp->name;
	}
	Set_message_f("%s hit %s %s by %s and reprogrammed by %s.",
		      pl->name,
		      Describe_shot(mine->type, mine->status, mine->mods,1),
		      BIT(mine->status, GRAVITY) ? "thrown " : "dropped ",
		      Player_by_id(mine->owner)->name,
		      reprogrammer_name);
    }
    if (kp) {
	/*
	 * Question with this is if we want to give the same points for
	 * a high-scored-player hitting a low-scored-player's mine as
	 * for a low-scored-player hitting a high-scored-player's mine.
	 * Maybe not.
	 */
	sc = Rate(kp->score, pl->score) * options.mineScoreMult;
	Score_players(kp, sc, pl->name, pl, -sc, kp->name, false);
    }
}


static void Player_collides_with_debris(player_t *pl, object_t *obj)
{
    player_t *kp = NULL;
    double cost, sc;
    char msg[MSG_LEN];

    cost = collision_cost(obj->mass, VECTOR_LENGTH(obj->vel));

    if (!Player_used_emergency_shield(pl))
	Player_add_fuel(pl, -cost);
    if (pl->fuel.sum == 0.0
	|| (obj->type == OBJ_WRECKAGE
	    && options.wreckageCollisionMayKill
	    && !BIT(pl->used, HAS_SHIELD)
	    && !BIT(pl->have, HAS_ARMOR))) {
	SET_BIT(pl->status, KILLED);
	sprintf(msg, "%s succumbed to an explosion.", pl->name);
	if (obj->id != NO_ID) {
	    kp = Player_by_id(obj->id);
	    sprintf(msg + strlen(msg) - 1, " from %s.", kp->name);
	    if (obj->id == pl->id)
		sprintf(msg + strlen(msg), "  How strange!");
	}
	Set_message(msg);
	if (!kp || kp->id == pl->id) {
	    sc = Rate(0.0, pl->score)
		* options.explosionKillScoreMult * options.selfKillScoreMult;
	    Score(pl, -sc, pl->pos, (kp == NULL) ? "[Explosion]" : pl->name);
	} else {
	    Rank_add_explosion_kill(kp);
	    sc = Rate(kp->score, pl->score) * options.explosionKillScoreMult;
	    Score_players(kp, sc, pl->name, pl, -sc, kp->name, true);
	}
	obj->life = 0.0;
	return;
    }
    if (obj->type == OBJ_WRECKAGE
	&& options.wreckageCollisionMayKill
	&& !BIT(pl->used, HAS_SHIELD)
	&& BIT(pl->have, HAS_ARMOR))
	Player_hit_armor(pl);
}


static void Player_collides_with_asteroid(player_t *pl, wireobject_t *ast)
{
    double v = VECTOR_LENGTH(ast->vel);
    double cost = collision_cost(ast->mass, v);
    double sc;

    ast->life += ASTEROID_FUEL_HIT(ED_PL_CRASH, ast->size);
    if (ast->life < 0.0)
	ast->life = 0.0;
    if (ast->life == 0.0) {
	sc = Rate(pl->score, ASTEROID_SCORE) * options.unownedKillScoreMult;
	Score(pl, sc, ast->pos, "");
    }

    if (!Player_used_emergency_shield(pl))
	Player_add_fuel(pl, -cost);

    if (options.asteroidCollisionMayKill
	&& (pl->fuel.sum == 0.0
	    || (!BIT(pl->used, HAS_SHIELD)
		&& !BIT(pl->have, HAS_ARMOR)))) {
	SET_BIT(pl->status, KILLED);
	if (pl->velocity > v)
	    /* player moves faster than asteroid */
	    Set_message_f("%s smashed into an asteroid.", pl->name);
	else
	    Set_message_f("%s was hit by an asteroid.", pl->name);
	sc = Rate(0.0, pl->score) * options.unownedKillScoreMult;
	Score(pl, -sc, pl->pos, "[Asteroid]");
	if (Player_is_tank(pl)) {
	    player_t *owner_pl = Player_by_id(pl->lock.pl_id);

	    sc = Rate(owner_pl->score, ASTEROID_SCORE)
		* options.unownedKillScoreMult;
	    Score(owner_pl, sc, ast->pos, "");
	}
	return;
    }
    if (options.asteroidCollisionMayKill
	&& !BIT(pl->used, HAS_SHIELD)
	&& BIT(pl->have, HAS_ARMOR))
	Player_hit_armor(pl);
}


static void Player_collides_with_killing_shot(player_t *pl, object_t *obj)
{
    player_t *kp = NULL;
    double sc, drainfactor, drain;

    /*
     * Player got hit by a potentially deadly object.
     *
     * When a player has shields up, and not enough fuel
     * to `absorb' the shot then shields are lowered.
     * This is not very logical, rather in this case
     * the shot should be considered to be deadly too.
     *
     * Sound effects are missing when shot is deadly.
     */

    if (BIT(pl->used, HAS_SHIELD)
	|| BIT(pl->have, HAS_ARMOR)
	|| (obj->type == OBJ_TORPEDO
	    && BIT(obj->mods.nuclear, NUCLEAR)
	    && (rfrac() >= 0.25))) {
	switch (obj->type) {
	case OBJ_TORPEDO:
	    sound_play_sensors(pl->pos, PLAYER_EAT_TORPEDO_SHOT_SOUND);
	    break;
	case OBJ_HEAT_SHOT:
	    sound_play_sensors(pl->pos, PLAYER_EAT_HEAT_SHOT_SOUND);
	    break;
	case OBJ_SMART_SHOT:
	    sound_play_sensors(pl->pos, PLAYER_EAT_SMART_SHOT_SOUND);
	    break;
	default:
	    break;
	}

	switch(obj->type) {
	case OBJ_TORPEDO:
	case OBJ_HEAT_SHOT:
	case OBJ_SMART_SHOT:
	    if (obj->id == NO_ID)
		Set_message_f("%s ate %s.", pl->name,
			      Describe_shot(obj->type, obj->status,
					    obj->mods, 1));
	    else {
		kp = Player_by_id(obj->id);
		Set_message_f("%s ate %s from %s.", pl->name,
			      Describe_shot(obj->type, obj->status,
					    obj->mods, 1),
			      kp->name);
	    }
	    drain = (ED_SMART_SHOT_HIT /
		((obj->mods.mini + 1) * (obj->mods.power + 1)));
	    if (!Player_used_emergency_shield(pl))
		Player_add_fuel(pl, drain);
	    pl->forceVisible += 2;
	    break;

	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    sound_play_sensors(pl->pos, PLAYER_EAT_SHOT_SOUND);
	    if (!Player_used_emergency_shield(pl)) {
		if (options.shotHitFuelDrainUsesKineticEnergy) {
		    double rel_velocity = LENGTH(pl->vel.x - obj->vel.x,
						 pl->vel.y - obj->vel.y);
		    drainfactor
			= ((rel_velocity * rel_velocity * ABS(obj->mass))
			   / (options.shotSpeed * options.shotSpeed
			      * options.shotMass));
		} else
		    drainfactor = 1.0;
		drain = ED_SHOT_HIT * drainfactor * SHOT_MULT(obj);
		Player_add_fuel(pl, drain);
	    }
	    pl->forceVisible += SHOT_MULT(obj);
	    break;

	default:
	    warn("Player hit by unknown object type %d.", obj->type);
	    break;
	}

	if (pl->fuel.sum <= 0)
	    CLR_BIT(pl->used, HAS_SHIELD);
	if (!BIT(pl->used, HAS_SHIELD) && BIT(pl->have, HAS_ARMOR))
	    Player_hit_armor(pl);

    } else {
	double factor;

	switch (obj->type) {
	case OBJ_TORPEDO:
	case OBJ_SMART_SHOT:
	case OBJ_HEAT_SHOT:
	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    if (BIT(obj->status, FROMCANNON)) {
		sound_play_sensors(pl->pos, PLAYER_HIT_CANNONFIRE_SOUND);
		Set_message_f("%s was hit by cannonfire.", pl->name);
		sc = Rate(CANNON_SCORE, pl->score)/4;
	    } else if (obj->id == NO_ID) {
		Set_message_f("%s was killed by %s.", pl->name,
			      Describe_shot(obj->type, obj->status,
					    obj->mods, 1));
		sc = Rate(0.0, pl->score) * options.unownedKillScoreMult;
	    } else {
		kp = Player_by_id(obj->id);
		Set_message_f("%s was killed by %s from %s.%s", pl->name,
			      Describe_shot(obj->type, obj->status,
					    obj->mods, 1),
			      kp->name,
			      kp->id == pl->id ? "  How strange!" : "");
		if (kp->id == pl->id) {
		    sound_play_sensors(pl->pos, PLAYER_SHOT_THEMSELF_SOUND);
		    sc = Rate(0.0, pl->score) * options.selfKillScoreMult;
		} else {
		    Rank_add_shot_kill(kp);
		    sc = Rate(kp->score, pl->score);
		}
	    }

	    switch (obj->type) {
	    case OBJ_SHOT:
		if (BIT(obj->mods.warhead, CLUSTER))
		    factor = options.clusterKillScoreMult;
		else
		    factor = options.shotKillScoreMult;
		break;
	    case OBJ_TORPEDO:
		factor = options.torpedoKillScoreMult;
		break;
	    case OBJ_SMART_SHOT:
		factor = options.smartKillScoreMult;
		break;
	    case OBJ_HEAT_SHOT:
		factor = options.heatKillScoreMult;
		break;
	    default:
		factor = options.shotKillScoreMult;
		break;
	    }

	    sc *= factor;
	    if (BIT(obj->status, FROMCANNON))
		Score(pl, -sc, pl->pos, "Cannon");
	    else if (obj->id == NO_ID || kp->id == pl->id)
		Score(pl, -sc, pl->pos, (obj->id == NO_ID ? "" : pl->name));
	    else {
		Score_players(kp, sc, pl->name, pl, -sc, kp->name, true);
		Robot_war(pl, kp);
	    }
	    SET_BIT(pl->status, KILLED);
	    return;

	default:
	    break;
	}
    }
}

static void AsteroidCollision(world_t *world)
{
    int j, radius, obj_count;
    object_t *ast;
    object_t *obj = NULL, **obj_list;
    list_t list;
    list_iter_t iter;
    double damage = 0.0;
    bool sound = false;

    list = Asteroid_get_list();
    if (!list)
	return;

    for (iter = List_begin(list); iter != List_end(list); LI_FORWARD(iter)) {
	ast = (object_t *)LI_DATA(iter);

	assert(ast->type == OBJ_ASTEROID);

	if (ast->life <= 0.0)
	    continue;

	assert(World_contains_clpos(world, ast->pos));

	if (ObjCount >= options.cellGetObjectsThreshold)
	    Cell_get_objects(world, ast->pos, ast->pl_radius / BLOCK_SZ + 1,
			     300, &obj_list, &obj_count);
	else {
	    obj_list = Obj;
	    obj_count = ObjCount;
	}

	for (j = 0; j < obj_count; j++) {
	    obj = obj_list[j];
	    assert(obj != NULL);

	    /* asteroids don't hit these objects */
	    if ((obj->type == OBJ_ITEM
		 || obj->type == OBJ_DEBRIS
		 || obj->type == OBJ_SPARK
		 || obj->type == OBJ_WRECKAGE)
		&& obj->id == NO_ID
		&& !BIT(obj->status, FROMCANNON))
		continue;
	    /* don't collide while still overlapping  after breaking */
	    if (obj->type == OBJ_ASTEROID && frame_time < ast->fusetime)
		continue;
	    /* don't collide with self */
	    if (obj == ast)
		continue;
	    /* don't collide with phased balls */
	    if (obj->type == OBJ_BALL
		&& obj->id != NO_ID
		&& BIT(Player_by_id(obj->id)->used, HAS_PHASING_DEVICE))
		continue;

	    radius = (ast->pl_radius + obj->pl_radius) * CLICK;
	    if (!in_range(OBJ_PTR(ast), obj, (double)radius))
		continue;

	    switch (obj->type) {
	    case OBJ_BALL:
		Obj_repel(ast, obj, radius);
		if (options.treasureCollisionDestroys)
		    obj->life = 0.0;
		damage = ED_BALL_HIT;
		sound = true;
		break;
	    case OBJ_ASTEROID:
		obj->life -= ASTEROID_FUEL_HIT(
		    collision_cost(ast->mass, VECTOR_LENGTH(ast->vel)),
		    WIRE_PTR(obj)->size);
		damage = -collision_cost(obj->mass, VECTOR_LENGTH(obj->vel));
		Delta_mv_elastic(ast, obj);
		/* avoid doing collision twice */
		obj->fusetime = frame_time + timeStep;
		sound = true;
		break;
	    case OBJ_SPARK:
		obj->life = 0.0;
		Delta_mv(ast, obj);
		damage = 0.0;
		break;
	    case OBJ_DEBRIS:
	    case OBJ_WRECKAGE:
		obj->life = 0.0;
		damage = -collision_cost(obj->mass, VECTOR_LENGTH(obj->vel));
		Delta_mv(ast, obj);
		break;
	    case OBJ_MINE:
		if (!BIT(obj->status, CONFUSED))
		    obj->life = 0.0;
		break;
	    case OBJ_SHOT:
	    case OBJ_CANNON_SHOT:
		obj->life = 0.0;
		Delta_mv(ast, obj);
		damage = ED_SHOT_HIT;
		sound = true;
		break;
	    case OBJ_SMART_SHOT:
	    case OBJ_TORPEDO:
	    case OBJ_HEAT_SHOT:
		obj->life = 0.0;
		Delta_mv(ast, obj);
		damage = ED_SMART_SHOT_HIT
			 / ((obj->mods.mini + 1) * (obj->mods.power + 1));
		sound = true;
		break;
	    case OBJ_PULSE:
		obj->life = 0;
		damage = ED_LASER_HIT;
		sound = true;
		break;
	    default:
		Delta_mv(ast, obj);
		damage = 0.0;
		break;
	    }

	    if (ast->life > 0.0) {
		/* kps - this is some strange sort of hack - fix it*/
		/*if (ast->life <= ast->fuselife) {*/
		ast->life += ASTEROID_FUEL_HIT(damage, WIRE_PTR(ast)->size);
		/*}*/
		if (sound)
		    sound_play_sensors(ast->pos, ASTEROID_HIT_SOUND);
		if (ast->life < 0.0)
		    ast->life = 0.0;
		if (ast->life == 0.0) {
		    if ((obj->id != NO_ID
			 || (obj->type == OBJ_BALL
			     && BALL_PTR(obj)->owner != NO_ID))) {
			int owner_id = ((obj->type == OBJ_BALL)
					? BALL_PTR(obj)->owner
					: obj->id);
			player_t *pl = Player_by_id(owner_id);
			double sc = Rate(pl->score, ASTEROID_SCORE)
			    * options.unownedKillScoreMult;

			Score(pl, sc, ast->pos, "");
		    }

		    /* break; */
		}
	    }
	}
    }
}


/* do ball - object and ball - checkpoint collisions */
static void BallCollision(world_t *world)
{
    int i, j, obj_count;
    int	ignored_object_types;
    object_t **obj_list;
    object_t *obj;
    ballobject_t *ball;

    /*
     * These object types ignored;
     * some are handled by other code,
     * some don't interact.
     */
    ignored_object_types = OBJ_PLAYER | OBJ_ASTEROID | OBJ_MINE | OBJ_ITEM;
    if (!options.ballSparkCollisions)
	ignored_object_types |= OBJ_SPARK;

    for (i = 0; i < NumObjs; i++) {
	ball = BALL_IND(i);

	/* ignore if: */
	if (ball->type != OBJ_BALL ||	/* not a ball */
	    ball->life <= 0.0 ||	/* dying ball */
	    (ball->id != NO_ID
	     && BIT(Player_by_id(ball->id)->used, HAS_PHASING_DEVICE)) ||
					/* phased ball */
	    ball->treasure->have)	/* safe in a treasure */
	    continue;

	/* Ball - checkpoint */
	if (BIT(world->rules->mode, TIMING)
	    && options.ballrace
	    && ball->owner != NO_ID) {
	    player_t *owner = Player_by_id(ball->owner);

	    if (!options.ballrace_connect || ball->id == owner->id) {
		clpos_t cpos = Check_by_index(world, owner->check)->pos;

		if (Wrap_length(ball->pos.cx - cpos.cx,
				ball->pos.cy - cpos.cy)
		    < options.checkpointRadius * BLOCK_CLICKS)
		    Player_pass_checkpoint(owner);
	    }
	}

	/* Ball - object */
	if (!options.ballCollisions)
	    continue;

	if (ObjCount >= options.cellGetObjectsThreshold)
	    Cell_get_objects(world, ball->pos, 4, 300, &obj_list, &obj_count);
	else {
	    obj_list = Obj;
	    obj_count = ObjCount;
	}
	    
	for (j = 0; j < obj_count; j++) {
	    int radius;

	    obj = obj_list[j];

	    if (BIT(OBJ_TYPEBIT(obj->type), ignored_object_types))
		continue;

	    /* have we already done this ball pair? */
	    if (obj->type == OBJ_BALL && obj <= OBJ_PTR(ball))
		continue;

	    radius = (ball->pl_radius + obj->pl_radius) * CLICK;
	    if (!in_range(OBJ_PTR(ball), obj, (double)radius))
		continue;

	    /* bang! */

	    switch (obj->type) {
	    case OBJ_BALL:
		/* Balls bounce off other balls that aren't safe in
		 * the treasure: */
		{
		    ballobject_t *b2 = BALL_PTR(obj);
		    if (b2->treasure->have)
			break;

		    if (b2->id != NO_ID
			&& BIT(Player_by_id(b2->id)->used, HAS_PHASING_DEVICE))
			break;
		}

		/* if the collision was too violent, destroy ball and object */
		if ((sqr(ball->vel.x - obj->vel.x) +
		     sqr(ball->vel.y - obj->vel.y)) >
		    sqr(options.maxObjectWallBounceSpeed)) {
		    ball->life = 0.0;
		    obj->life  = 0.0;
		} else
		    /* they bounce */
		    Obj_repel(OBJ_PTR(ball), obj, radius);
		break;

	    /* balls absorb and destroy all other objects: */
	    case OBJ_SPARK:
	    case OBJ_TORPEDO:
	    case OBJ_SMART_SHOT:
	    case OBJ_HEAT_SHOT:
	    case OBJ_SHOT:
	    case OBJ_CANNON_SHOT:
	    case OBJ_DEBRIS:
	    case OBJ_WRECKAGE:
		Delta_mv(OBJ_PTR(ball), obj);
		obj->life = 0.0;
		break;
	    default:
		break;
	    }
	}
    }
}


/* do mine - object collisions */
static void MineCollision(world_t *world)
{
    int i, j, obj_count;
    object_t **obj_list;
    object_t *obj;
    mineobject_t *mine;

    if (!options.mineShotDetonateDistance)
	return;

    for (i = 0; i < NumObjs; i++) {
	mine = MINE_IND(i);

	/* ignore if: */
	if (mine->type != OBJ_MINE ||	/* not a mine */
	    mine->life <= 0.0)		/* dying mine */
	    continue;

	if (ObjCount >= options.cellGetObjectsThreshold)
	    Cell_get_objects(world, mine->pos, 4, 300, &obj_list, &obj_count);
	else {
	    obj_list = Obj;
	    obj_count = ObjCount;
	}

	for (j = 0; j < obj_count; j++) {
	    double radius;

	    obj = obj_list[j];

	    /*
	     * These object types ignored;
	     * some are handled by other code,
	     * some don't interact.
	     */
	    if (!(obj->type == OBJ_SHOT
		  || obj->type == OBJ_TORPEDO
		  || obj->type == OBJ_SMART_SHOT
		  || obj->type == OBJ_HEAT_SHOT
		  || obj->type == OBJ_CANNON_SHOT))
		continue;

	    radius = (options.mineShotDetonateDistance + obj->pl_radius)
		* CLICK;
	    if (!in_range(OBJ_PTR(mine), obj, radius))
		continue;

	    /* bang! */
	    obj->life = 0.0;
	    mine->life = 0.0;
	    break;
	}
    }
}

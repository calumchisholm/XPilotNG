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

char play_version[] = VERSION;

int tagItPlayerId = NO_ID;	/* player who is 'it' */
 
void Transfer_tag(player *oldtag_pl, player *newtag_pl)
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
double Handle_tag(double sc, player *victim_pl, player *killer_pl)
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

static inline bool Player_can_be_tagged(player *pl)
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
    player *tag_pl = Player_by_id(tagItPlayerId);

    if (tag_pl && Player_can_be_tagged(tag_pl))
	return;

    /* Find number of players that might get the tag */
    for (i = 0; i < NumPlayers; i++) {
	player *pl = Players(i);
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
	player *pl = Players(i);
	if (Player_can_be_tagged(pl)) {
	    tagItPlayerId = pl->id;
	    break;
	}
    }

    if (tagItPlayerId == NO_ID) {
	for (i = 0; i < candidate; i++) {
	    player *pl = Players(i);
	    if (Player_can_be_tagged(pl)) {
		tagItPlayerId = pl->id;
		break;
	    }
	}
    }

    /* someone should be tagged by now */
    assert(tagItPlayerId != NO_ID);
}

static int Punish_team(player *pl, treasure_t *td, clpos pos)
{
    static char		msg[MSG_LEN];
    int			i;
    double		win_score = 0.0,lose_score = 0.0;
    int			win_team_members = 0, lose_team_members = 0;
    int			somebody_flag = 0;
    double		sc, por;
    world_t *world = &World;

    Check_team_members (td->team);
    if (td->team == pl->team)
	return 0;

    if (BIT(world->rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);

	    if (Player_is_tank(pl_i)
		|| (BIT(pl_i->status, PAUSE) && pl_i->count <= 0)
		|| Player_is_waiting(pl_i))
		continue;
	    if (pl_i->team == td->team) {
		lose_score += pl_i->score;
		lose_team_members++;
		if (BIT(pl_i->status, GAME_OVER) == 0)
		    somebody_flag = 1;
	    }
	    else if (pl_i->team == pl->team) {
		win_score += pl_i->score;
		win_team_members++;
	    }
	}
    }

    sound_play_all(DESTROY_BALL_SOUND);
    sprintf(msg, " < %s's (%d) team has destroyed team %d treasure >",
	    pl->name, pl->team, td->team);
    Set_message(msg);

    if (!somebody_flag) {
	Score(pl, Rate(pl->score, CANNON_SCORE)/2, pos, "Treasure:");
	return 0;
    }

    td->destroyed++;
    world->teams[td->team].TreasuresLeft--;
    world->teams[pl->team].TreasuresDestroyed++;

    sc  = 3 * Rate(win_score, lose_score);
    por = (sc * lose_team_members) / (2 * win_team_members + 1);

    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);

	if (Player_is_tank(pl_i)
	    || (BIT(pl_i->status, PAUSE)
		&& pl_i->count <= 0)
	    || Player_is_waiting(pl_i))
	    continue;
	if (pl_i->team == td->team) {
	    Score(pl_i, -sc, pos, "Treasure: ");
	    Rank_LostBall(pl_i);
	    if (treasureKillTeam)
		SET_BIT(pl_i->status, KILLED);
	}
	else if (pl_i->team == pl->team &&
		 (pl_i->team != TEAM_NOT_SET || pl_i->id == pl->id)) {
	    if (lose_team_members > 0) {
		if (pl_i->id == pl->id)
		    Rank_CashedBall(pl_i);
		Rank_WonBall(pl_i);
	    }
	    Score(pl_i, (pl_i->id == pl->id ? 3*por : 2*por),
		  pos, "Treasure: ");
	}
    }

    if (treasureKillTeam)
	Rank_AddKill(pl);

    updateScores = true;

    return 1;
}


/****************************
 * Functions for explosions.
 */

/* Create debris particles */
void Make_debris(
    /* pos            */ clpos  pos,
    /* vel            */ vector vel,
    /* owner id       */ int    id,
    /* owner team     */ int    team,
    /* type           */ int    type,
    /* mass           */ double  mass,
    /* status         */ long   status,
    /* color          */ int    color,
    /* radius         */ int    radius,
    /* num debris     */ int    num_debris,
    /* min,max dir    */ int    min_dir,    int    max_dir,
    /* min,max speed  */ double min_speed,  double max_speed,
    /* min,max life   */ double min_life,   double max_life
)
{
    object		*debris;
    int			i;
    double		life;
    modifiers		mods;
    world_t *world = &World;

    pos.cx = WRAP_XCLICK(pos.cx);
    pos.cy = WRAP_YCLICK(pos.cy);
    if (!World_contains_clpos(world, pos))
	return;

    if (max_life < min_life)
	max_life = min_life;

    if (max_speed < min_speed)
	max_speed = min_speed;

    CLEAR_MODS(mods);

    if (type == OBJ_SHOT) {
	SET_BIT(mods.warhead, CLUSTER);
	if (!ShotsGravity)
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
	debris->id = id;
	debris->team = team;
	Object_position_init_clpos(debris, pos);
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
	if (shotHitFuelDrainUsesKineticEnergy
	    && type == OBJ_SHOT) {
	    /* compensate so that m*v^2 is constant */
	    double sp_shotsp = speed / ShotsSpeed;
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
	Cell_add_object(debris);
    }
}


/*
 * Ball has been replaced back in the hoop from whence
 * it came. The player must be from the same team as the ball,
 * otherwise Bounce_object() wouldn't have been called. It
 * should be replaced into the hoop without exploding and
 * the player gets some points.
 */
void Ball_is_replaced(ballobject *ball)
{
    char msg[MSG_LEN];
    player *pl = Player_by_id(ball->owner);

    ball->life = 0;
    SET_BIT(ball->status, (NOEXPLOSION|RECREATE));

    Score(pl, 5.0, ball->pos, "Treasure: ");
    sprintf(msg, " < %s (team %d) has replaced the treasure >",
	    pl->name, pl->team);
    Set_message(msg);
    Rank_SavedBall(pl);
}


/*
 * Ball has been brought back to home treasure.
 * The team should be punished.
 */
void Ball_is_destroyed(ballobject *ball)
{
    char msg[MSG_LEN];
    player *pl = Player_by_id(ball->owner);
    double frames = (1e6 - ball->life) / timeStep + .5;
    double seconds = frames / framesPerSecond;

    if (timeStep != 1.0) {
	double normalized = frames * timeStep;

	sprintf(msg," < The ball was loose for %d frames "
		"/ %.2f frames at gamespeed=FPS (best %d) / %.2fs >",
		(int)frames, normalized, Rank_GetBestBall(pl), seconds);
	Rank_BallRun(pl, (int)(normalized + 0.5));
    } else {
	sprintf(msg," < The ball was loose for %d frames / %.2fs >",
		(int)frames, seconds);
	Rank_BallRun(pl, frames);
    }
    Set_message(msg);
}



void Ball_hits_goal(ballobject *ball, group_t *gp)
{
    player *owner;
    treasure_t *td;
    char msg[MSG_LEN];
    world_t *world = &World;

    /*
     * Player already quit ?
     */
    if (ball->owner == NO_ID) {
	SET_BIT(ball->status, (NOEXPLOSION|RECREATE));
	return;
    }
    /*
     * If it's not team play, nothing interesting happens.
     */
    if (!BIT(world->rules->mode, TEAM_PLAY))
	return;

    td = ball->treasure;
    if (td->team == gp->team) {
	Ball_is_replaced(ball);
	return;
    }
    owner = Player_by_id(ball->owner);
    if (gp->team == owner->team) {
	treasure_t *tt = Treasures(world, gp->mapobj_ind);

	Ball_is_destroyed(ball);

	if (captureTheFlag && !tt->have && !tt->empty) {
	    sprintf(msg, " < The treasure must be safe before you "
		    "can cash an opponent's! >");
	    Set_message(msg);
	} else if (Punish_team(owner, td, ball->pos))
	    CLR_BIT(ball->status, RECREATE);
	return;
    }
}

/*
 * Here follows some hit functions, used in the walls code to determine
 * if some object can hit some polygon. The arguments for a hit function
 * are: the pointer to the polygon group that the polygon belongs to and
 * the pointer to the move. The hit function checks if the move can hit
 * the polygon group.
 * NOTE: hit functions should not have any side effects (i.e. change
 * anything) unless you know what you are doing.
 */

/*
 * This function is called when something would hit a balltarget.
 * The function determines if it hits or not.
 */
bool Balltarget_hitfunc(group_t *gp, move_t *move)
{
    ballobject *ball = NULL;
    world_t *world = &World;

    /* this can happen if is_inside is called for a balltarget with
       a NULL obj */
    if (move->obj == NULL)
	return true;

    /* can this happen ? */
    if (move->obj->type != OBJ_BALL) {
	printf("Balltarget_hitfunc: hit by a %s.\n",
	       Object_typename(move->obj));
	return true;
    }

    ball = BALL_PTR(move->obj);

    if (ball->owner == NO_ID)
	return true;

    if (BIT(world->rules->mode, TEAM_PLAY)) {
	/*
	 * The only case a ball does not hit a balltarget is when
	 * the ball and the target are of the same team, but the
	 * owner is not.
	 */
	if (ball->treasure->team == gp->team
	    && Player_by_id(ball->owner)->team != gp->team)
	    return false;
	return true;
    }

    /* not teamplay */

    /* kps - fix this */

    /* allow grabbing of ball */
    return false;
}


/*
 * Cannon specific functions
 */
hitmask_t Cannon_hitmask(cannon_t *cannon)
{
    world_t *world = &World;

    if (cannon->dead_time > 0)
	return ALL_BITS;
    if (BIT(world->rules->mode, TEAM_PLAY) && teamImmunity)
	return HITMASK(cannon->team);
    return 0;
}

static void Cannon_set_hitmask(int group, cannon_t *cannon)
{
    assert(group == cannon->group);

    P_set_hitmask(cannon->group, Cannon_hitmask(cannon));
}


void World_restore_cannon(world_t *world, cannon_t *cannon)
{
    blpos blk = Clpos_to_blpos(cannon->pos);

    World_set_block(world, blk, CANNON);

    cannon->conn_mask = 0;
    cannon->last_change = frame_loops;
    cannon->dead_time = 0;

    P_set_hitmask(cannon->group, Cannon_hitmask(cannon));
}

void World_remove_cannon(world_t *world, cannon_t *cannon)
{
    blpos blk = Clpos_to_blpos(cannon->pos);

    cannon->dead_time = cannonDeadTime;
    cannon->conn_mask = 0;

    World_set_block(world, blk, SPACE);

    P_set_hitmask(cannon->group, Cannon_hitmask(cannon));
}


extern struct move_parameters mp;
/*
 * This function is called when something would hit a cannon.
 *
 * Ideas stolen from Move_segment in walls_old.c
 */
bool Cannon_hitfunc(group_t *gp, move_t *move)
{
    object *obj = move->obj;
    world_t *world = &World;
    cannon_t *cannon = Cannons(world, gp->mapobj_ind);
    unsigned long cannon_mask;

    /* this should never happen if hitmasks are ok */
    assert (! (cannon->dead_time > 0));

    /* if cannon is phased nothing will hit it */
    if (BIT(cannon->used, HAS_PHASING_DEVICE))
	return false;

    if (obj == NULL)
	return true;

    cannon_mask = mp.obj_cannon_mask | OBJ_PLAYER;
    if (!BIT(cannon_mask, obj->type))
	return false;

    /*
     * kps - if no team play, both cannons have team == TEAM_NOT_SET,
     * this code should work, no matter if team play is true or not.
     */
     if (BIT(obj->status, FROMCANNON)
         && obj->team == cannon->team) {
         return false;
     }

     return true;
}


/*
 * Target specific functions
 */
hitmask_t Target_hitmask(target_t *targ)
{
    /* target is destroyed - nothing hits */
    if (targ->dead_time > 0)
	return ALL_BITS;

    /* everything hits targets that don't belong to a team */
    if (targ->team == TEAM_NOT_SET)
	return 0;

    /* if targetTeamCollision is true, everything hits a target */
    if (targetTeamCollision)
	return 0;

    /* note that targets are always team immune */
    return HITMASK(targ->team);
}

static void Target_set_hitmask(int group, target_t *targ)
{
    assert(targ->group == group);
    P_set_hitmask(targ->group, Target_hitmask(targ));
}

void Target_init(void)
{
    int group;
    world_t *world = &World;

    for (group = 0; group < num_groups; group++) {
	group_t *gp = groupptr_by_id(group);

	if (gp->type == TARGET)
	    Target_set_hitmask(group, Targets(world, gp->mapobj_ind));
    }

#if 0
    P_grouphack(TARGET, Target_set_hitmask);
#endif
}

void World_restore_target(world_t *world, target_t *targ)
{
    blpos blk = Clpos_to_blpos(targ->pos);

    World_set_block(world, blk, TARGET);

    targ->conn_mask = 0;
    targ->update_mask = (unsigned)-1;
    targ->last_change = frame_loops;
    targ->dead_time = 0;
    targ->damage = TARGET_DAMAGE;

    P_set_hitmask(targ->group, Target_hitmask(targ));
}

void World_remove_target(world_t *world, target_t *targ)
{
    blpos blk = Clpos_to_blpos(targ->pos);

    targ->update_mask = (unsigned) -1;
    /* is this necessary? (done also in Target_restore_on_map() ) */
    targ->damage = TARGET_DAMAGE;
    targ->dead_time = targetDeadTime;

    /*
     * Destroy target.
     * Turn it into a space to simplify other calculations.
     */
    World_set_block(world, blk, SPACE);

    /*P_set_hitmask(targ->group, ALL_BITS);*/
    P_set_hitmask(targ->group, Target_hitmask(targ));
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
    object *obj = move->obj;
    world_t *world = &World;
    wormhole_t *wormhole = Wormholes(world, gp->mapobj_ind);

    return false;

#if 0
    warn("Wormhole_hitfunc: wormhole %p hit by a %s",
	 wormhole, Object_typename(move->obj));
#endif
    if (obj->type == OBJ_PLAYER) {
	player *pl = (player *)obj;
	if (pl->wormHoleHit == gp->mapobj_ind)
	    return false;
    }

    return true;

#if 0
    /* this should never happen, because of the hitmask */
    if (wormhole->type == WORM_OUT) {
	warn("BUG: Wormhole_hitfunc: wormhole->type == WORM_OUT\n");
	return false;
    }

    if (obj == NULL)
	return true;

    if (obj->type == OBJ_PLAYER) {
	player *pl = (player *)obj;

	if (BIT(pl->status, WARPING))
	    return false;
#if 0
	/*
	 * Don't warp again if we are still on the
	 * same wormhole we have just been warped to.
	 */
	if (pl->warped > 0
	    && wormhole->type == WORM_NORMAL
	    && wormhole == Wormholes(pl->wormHoleDest)
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

void Wormhole_remove_from_map(wormhole_t *wormhole)
{
    world_t *world = &World;
    blpos blk = Clpos_to_blpos(wormhole->pos);

    World_set_block(world, blk, wormhole->lastblock);
}

extern void Describe_group(int group);
/*
 * Handling of group properties
 */
void Team_immunity_init(void)
{
    int group;
    world_t *world = &World;

    for (group = 0; group < num_groups; group++) {
	group_t *gp = groupptr_by_id(group);

	if (gp->type == CANNON) {
	    cannon_t *cannon = Cannons(world, gp->mapobj_ind);
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
void Hitmasks_init(void)
{
    Target_init();
    Team_immunity_init();
}

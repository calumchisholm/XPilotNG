/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
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

void Make_treasure_ball(world_t *world, treasure_t *t)
{
    ballobject_t *ball;
    clpos_t pos = t->pos;

    if (!is_polygon_map)
	pos.cy += (10 * PIXEL_CLICKS - BLOCK_CLICKS / 2);

    if (t->empty)
	return;
    if (t->have) {
	xpprintf("%s Failed Make_treasure_ball(treasure=%ld):\n",
		 showtime(), (long)t);
	xpprintf("\ttreasure: destroyed = %d, team = %d, have = %d\n",
		 t->destroyed, t->team, t->have);
	return;
    }

    if ((ball = BALL_PTR(Object_allocate())) == NULL)
	return;

    ball->life = 1e6;
    ball->mass = options.ballMass;
    ball->vel.x = 0;	  	/* make the ball stuck a little */
    ball->vel.y = 0;		/* longer to the ground */
    ball->acc.x = 0;
    ball->acc.y = 0;
    Object_position_init_clpos(world, OBJ_PTR(ball), pos);
    ball->id = NO_ID;
    ball->ball_owner = NO_ID;
    ball->team = t->team;
    ball->type = OBJ_BALL;
    ball->color = WHITE;
    ball->ball_count = 0;
    ball->pl_range = BALL_RADIUS;
    ball->pl_radius = BALL_RADIUS;
    CLEAR_MODS(ball->mods);
    ball->obj_status = RECREATE;
    ball->ball_treasure = t;
    ball->ball_style = t->ball_style;
    Cell_add_object(world, OBJ_PTR(ball));

    t->have = true;
}

void Treasure_init(world_t *world)
{
    int i;

    for (i = 0; i < world->NumTreasures; i++)
	Make_treasure_ball(world, Treasure_by_index(world, i));
}

/*
 * Ball has been replaced back in the hoop from whence
 * it came. The player must be from the same team as the ball,
 * otherwise Bounce_object() wouldn't have been called. It
 * should be replaced into the hoop without exploding and
 * the player gets some points.
 */
void Ball_is_replaced(ballobject_t *ball)
{
    player_t *pl = Player_by_id(ball->ball_owner);

    ball->life = 0;
    SET_BIT(ball->obj_status, (NOEXPLOSION|RECREATE));

    Score(pl, 5.0, ball->pos, "Treasure: ");
    Set_message_f(" < %s (team %d) has replaced the treasure >",
		  pl->name, pl->team);
    Rank_saved_ball(pl);
}


/*
 * Ball has been brought back to home treasure.
 * The team should be punished.
 */
void Ball_is_destroyed(ballobject_t *ball)
{
    player_t *pl = Player_by_id(ball->ball_owner);
    double ticks = 1e6 - ball->life;
    int frames = (int)(ticks / timeStep + .5);
    double seconds = ticks / options.gameSpeed;

    Set_message_f(" < The ball was loose for %d frames / "
		  "%.2f ticks (best %.2f) / %.2fs >",
		  frames, ticks, Rank_get_best_ballrun(pl), seconds);
    Rank_ballrun(pl, ticks);
}

static int Punish_team(player_t *pl, treasure_t *td, clpos_t pos)
{
    int i;
    double win_score = 0.0,lose_score = 0.0;
    int win_team_members = 0, lose_team_members = 0;
    double sc, por;
    world_t *world = pl->world;
    bool somebody = false;

    Check_team_members (world, td->team);
    if (td->team == pl->team)
	return 0;

    if (BIT(world->rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    player_t *pl_i = Player_by_index(i);

	    if (Player_is_tank(pl_i)
		|| (Player_is_paused(pl_i) && pl_i->pause_count <= 0)
		|| Player_is_waiting(pl_i))
		continue;
	    if (pl_i->team == td->team) {
		lose_score += pl_i->score;
		lose_team_members++;
		if (!Player_is_dead(pl_i))
		    somebody = true;
	    }
	    else if (pl_i->team == pl->team) {
		win_score += pl_i->score;
		win_team_members++;
	    }
	}
    }

    sound_play_all(DESTROY_BALL_SOUND);
    Set_message_f(" < %s's (%d) team has destroyed team %d treasure >",
		  pl->name, pl->team, td->team);

    if (!somebody) {
	Score(pl, Rate(pl->score, CANNON_SCORE)/2, pos, "Treasure:");
	return 0;
    }

    td->destroyed++;
    world->teams[td->team].TreasuresLeft--;
    world->teams[pl->team].TreasuresDestroyed++;

    sc  = 3 * Rate(win_score, lose_score);
    por = (sc * lose_team_members) / (2 * win_team_members + 1);

    for (i = 0; i < NumPlayers; i++) {
	player_t *pl_i = Player_by_index(i);

	if (Player_is_tank(pl_i)
	    || (Player_is_paused(pl_i) && pl_i->pause_count <= 0)
	    || Player_is_waiting(pl_i))
	    continue;

	if (pl_i->team == td->team) {
	    Score(pl_i, -sc, pos, "Treasure: ");
	    Rank_lost_ball(pl_i);
	    if (options.treasureKillTeam)
		Player_set_state(pl_i, PL_STATE_KILLED);
	}
	else if (pl_i->team == pl->team &&
		 (pl_i->team != TEAM_NOT_SET || pl_i->id == pl->id)) {
	    if (lose_team_members > 0) {
		if (pl_i->id == pl->id)
		    Rank_cashed_ball(pl_i);
		Rank_won_ball(pl_i);
	    }
	    Score(pl_i, (pl_i->id == pl->id ? 3*por : 2*por),
		  pos, "Treasure: ");
	}
    }

    if (options.treasureKillTeam)
	Rank_add_treasure_kill(pl);

    updateScores = true;

    return 1;
}

void Ball_hits_goal(ballobject_t *ball, group_t *gp)
{
    player_t *owner;
    treasure_t *td;
    world_t *world = &World;
    int i;

    /*
     * Player already quit ?
     */
    if (ball->ball_owner == NO_ID) {
	SET_BIT(ball->obj_status, (NOEXPLOSION|RECREATE));
	return;
    }
    /*
     * If it's not team play, nothing interesting happens.
     */
    if (!BIT(world->rules->mode, TEAM_PLAY))
	return;

    td = ball->ball_treasure;
    owner = Player_by_id(ball->ball_owner);
    if (td->team == gp->team && td->team == owner->team) {
	Ball_is_replaced(ball);
	return;
    }
    if (gp->team == owner->team) {
	treasure_t *tt = Treasure_by_index(world, gp->mapobj_ind);

	Ball_is_destroyed(ball);

	if (options.captureTheFlag && !tt->have && !tt->empty)
	    Set_message(" < The treasure must be safe before you "
			"can cash an opponent's! >");
	else if (Punish_team(owner, td, ball->pos))
	    CLR_BIT(ball->obj_status, RECREATE);
	return;
    }

    /* {KS} mods for team 'options.specialBallTeam' treasures start here*/
    /* Ball and box have to be in 'specialBallTeam'. If such a ball     */
    /* is scored by any player, it counts as if he had scored one ball */
    /* against every single opponent team */
    /* This means that (nonlinear scoring) the first team is punished */
    /* slightly more then the last team in the loop - fix for that?*/

    /* gp->team: team of treasure box     td->team: team of ball */
    if (gp->team == options.specialBallTeam  && 
	td->team == options.specialBallTeam ) {
      int opponent_teams = 0; 
      Ball_is_destroyed(ball);
      
      for (i = 0; i < MAX_TEAMS; i++) {

	if(world->teams[i].NumMembers == 0 || i == owner->team)
	  continue;
	opponent_teams++;
	td->team=i; /* give ball to team that has to be punished*/
	if (Punish_team(owner, td, ball->pos)) 
	  {
	    CLR_BIT(ball->obj_status, RECREATE);
	    /*undo treasure counts from Punish_team so we don't 
	      have to touch that function and possibly break it*/
	    world->teams[owner->team].TreasuresDestroyed--;
	    world->teams[td->team].TreasuresLeft++;
	  }
      }
      td->team=options.specialBallTeam;
      world->teams[options.specialBallTeam].TreasuresLeft--;
      world->teams[owner->team].TreasuresDestroyed++;
      world->teams[options.specialBallTeam].TreasuresDestroyed++;

      if(!opponent_teams){
	SET_BIT(ball->obj_status, RECREATE);
	world->teams[options.specialBallTeam].TreasuresLeft++;
	if (Punish_team(owner, td, ball->pos));
      }
      return;
    }

   /* {KS} mods for special treasures stop here*/
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
    ballobject_t *ball = NULL;
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

    if (ball->ball_owner == NO_ID)
	return true;

    if (BIT(world->rules->mode, TEAM_PLAY)) {
	/*
	 * The only case a ball does not hit a balltarget is when
	 * the ball and the target are of the same team, but the
	 * owner is not.
	 */
	if (ball->ball_treasure->team != options.specialBallTeam 
	    && ball->ball_treasure->team == gp->team
	    && Player_by_id(ball->ball_owner)->team != gp->team)
	    return false;
	return true;
    }

    /* not teamplay */

    /* kps - fix this */

    /* allow grabbing of ball */
    return false;
}


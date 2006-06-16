/* 
 * XPilot NG, a multiplayer space war game.
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
void Find_Adversaries(player_t *pl, arraylist_t *list);
void Generic_Scoring(arraylist_t *winners, arraylist_t *losers, double multiplier, const char *culprit);

void Score(player_t * pl, double points, clpos_t pos, const char *msg)
{
    if (!options.temporaryScoring)
    	Rank_add_score(pl, points);
    else Add_Score(pl,points);
    if (pl->conn != NULL)
	Send_score_object(pl->conn, points, pos, msg);
    updateScores = true;
}

double Rate(double winner, double loser)
{
    double t;

    if (options.constantScoring)
	return RATE_SIZE / 2;
    t = ((RATE_SIZE / 2) * RATE_RANGE) / (ABS(loser - winner) +
					  RATE_RANGE);
    if (loser > winner)
	t = RATE_SIZE - t;
    return t;
}

/*
 * Cause 'winner' to get 'winner_score' points added with message
 * 'winner_msg', and similarly with the 'loser' and equivalent
 * variables.
 *
 * In general the winner_score should be positive, and the loser_score
 * negative, but this need not be true.
 *
 * If the winner and loser players are on the same team, the scores are
 * made negative, since you shouldn't gain points by killing team members,
 * or being killed by a team member (it is both players faults).
 *
 * KK 28-4-98: Same for killing your own tank.
 * KK 7-11-1: And for killing a member of your alliance
 */
void Score_players(player_t * winner_pl, double winner_score,
		   char *winner_msg, player_t * loser_pl,
		   double loser_score, char *loser_msg, bool transfer_tag)
{
    if (Players_are_teammates(winner_pl, loser_pl)
	|| Players_are_allies(winner_pl, loser_pl)
	|| (Player_is_tank(loser_pl)
	    && loser_pl->lock.pl_id == winner_pl->id)) {
	if (winner_score > 0)
	    winner_score = -winner_score;
	if (loser_score > 0)
	    loser_score = -loser_score;
    }

    if (options.tagGame
	&& winner_score > 0.0 && loser_score < 0.0 && transfer_tag) {
	if (tagItPlayerId == winner_pl->id) {
	    winner_score *= options.tagItKillScoreMult;
	    loser_score *= options.tagItKillScoreMult;
	} else if (tagItPlayerId == loser_pl->id) {
	    winner_score *= options.tagKillItScoreMult;
	    loser_score *= options.tagKillItScoreMult;
	    Transfer_tag(loser_pl, winner_pl);
	}
    }

    Score(winner_pl, winner_score, loser_pl->pos, winner_msg);
    Score(loser_pl, loser_score, loser_pl->pos, loser_msg);
}

double Get_Real_Score(player_t * pl)
{
    return pl->score;
}

double Get_Score(player_t * pl)
{
    if (options.temporaryScoring) {
    	double score = 0.0;
    	int i;
    	for ( i=0 ; i<12 ; ++i) {
	    score += pl->scorelist[i];
	}
	return score;
    } else 
    	return pl->score;
}

void Set_Real_Score(player_t * pl, double score)
{
    pl->score = score;
}

void Set_Score(player_t * pl, double score)
{
    if (options.temporaryScoring) {
    	int i;
    	for ( i=0 ; i<12 ; ++i) {
	    pl->scorelist[i] = score;
	}
	pl->score = 0.0;
    } else {
    	pl->score = score;
    }
}

void Add_Score(player_t * pl, double score)
{
    pl->score += score;
}

void Increment_Scorelist(player_t *pl)
{
    Rank_add_score(pl, pl->score);
    Set_player_message_f(pl,"You scored %f points this round.",pl->score);
    pl->scorelist[pl->scorelist_i] = pl->score;
    pl->score = 0.0;
    pl->scorelist_i = (pl->scorelist_i +1)%12;
}

void Find_Adversaries(player_t *pl, arraylist_t *list)
{
    int i;
    for (i = 0; i < NumPlayers; i++) {
      player_t *pl_i = Player_by_index(i);

      if ( Players_are_teammates(pl, pl_i)
	  || (Player_is_paused(pl_i) && pl_i->pause_count <= 0)
	  || Player_is_tank(pl_i)
	  || Player_is_waiting(pl_i)
	  || Players_are_allies(pl, pl_i) )
	  continue;
      if (pl_i->id != pl->id) /*might not be needed due to team check */
	  Arraylist_add(list,&pl_i);
    }
}

void Generic_Scoring(arraylist_t *winners, arraylist_t *losers, double multiplier, const char *culprit)
{
    int num_winners, num_losers;
    double winner_score = 0.0, loser_score = 0.0;
    int i;
    double sc;
    player_t *killer, *victim;
    
    num_winners = Arraylist_get_num_elements(winners);
    num_losers = Arraylist_get_num_elements(losers);
    
    if ((!num_winners) && (!num_losers)) {
      warn("Generic_Scoring called with neither winners nor losers!");
      return;
    }
    
    if (options.zeroSumScoring) {
      if (!num_winners) {
	  Find_Adversaries(*(player_t **)Arraylist_get(losers,0),winners);
	  num_winners = Arraylist_get_num_elements(winners);
	  if (!num_winners) return;
      }
      if (!num_losers) {
	  Find_Adversaries(*(player_t **)Arraylist_get(winners,0),losers);
	  num_losers = Arraylist_get_num_elements(losers);
	  if (!num_losers) return;
      }
    }

    for ( i=0 ; i < num_winners ; ++i)
      winner_score += Get_Score(*(player_t **)Arraylist_get(winners,i));
    for ( i=0 ; i < num_losers ; ++i)
      loser_score += Get_Score(*(player_t **)Arraylist_get(losers,i));

    sc = Rate(winner_score,loser_score)*multiplier;
    
    if ( (num_winners==1) && (num_losers==1) && !culprit) {
      killer = *(player_t **)Arraylist_get(winners,0);
      victim = *(player_t **)Arraylist_get(losers,0);
      Score_players(killer, sc, victim->name, victim, -sc, killer->name, true);
    } else {
      for ( i=0 ; i < num_winners ; ++i ) {
	  killer = *(player_t **)Arraylist_get(winners,i);
	  Score(killer, sc/(double)num_winners, killer->pos
		  , (culprit == NULL) ? "" : culprit);
      }
      for ( i=0 ; i < num_losers ; ++i ) {
	  victim = *(player_t **)Arraylist_get(losers,i);
	  Score(victim, -sc/(double)num_losers, victim->pos
		  , (culprit == NULL) ? "" : culprit);
      }
    }
}

void Handle_Scoring(scoretype_t st, player_t * killer, player_t * victim,
		    void *extra, const char *somemsg)
{
    double factor = 0.0;
    int i_tank_owner = 0, j = 0;
    static player_t *true_killer;
    static arraylist_t *winners = NULL, *losers = NULL;
    
    if (!winners)
      winners = Arraylist_alloc(sizeof(player_t *));
    if (!losers)
      losers = Arraylist_alloc(sizeof(player_t *));

    if (!(killer || victim)) {
	warn("Attempted to score with neither victim nor killer");
	return;
    }

    switch (st) {
    case SCORE_CANNON_KILL:
    	if (BIT(world->rules->mode, TEAM_PLAY)
    	    && killer->team == ((cannon_t *)extra)->team) {
    	    Arraylist_add(losers,&killer);
    	} else {
    	    Arraylist_add(winners,&killer);
    	}
    	Generic_Scoring(winners,losers,options.cannonKillScoreMult,NULL);
	break;
    case SCORE_WALL_DEATH:
    	Arraylist_add(losers,&victim);
	if (somemsg) {
    	    Generic_Scoring(winners,losers,options.wallDeathScoreMult,somemsg);
	} else {
    	    Generic_Scoring(winners,losers,options.wallDeathScoreMult,victim->name);
	}
	break;
    case SCORE_COLLISION:
    	if (Player_is_tank(killer) && Player_is_tank(victim)) {
    	    break;
    	} else if (Player_is_tank(killer)) {
    	    true_killer = Player_by_id(killer->lock.pl_id);
    	    Arraylist_add(winners,&true_killer);
    	    Arraylist_add(losers,&victim);
    	} else if (Player_is_tank(victim)) {
    	    true_killer = Player_by_id(victim->lock.pl_id);
    	    Arraylist_add(winners,&true_killer);
    	    Arraylist_add(losers,&killer);
    	} else {
    	    Arraylist_add(winners,&killer);
    	    Arraylist_add(losers,&victim);
    	}
    	
	if (!Player_is_tank(killer) && !Player_is_tank(victim)) {
    	    Generic_Scoring(winners,losers,options.crashScoreMult,NULL);
    	    Generic_Scoring(losers,winners,options.crashScoreMult,NULL);
    	} else {
    	    Generic_Scoring(winners,losers,options.tankKillScoreMult,NULL);
    	}
	break;
    case SCORE_ROADKILL:
    	Arraylist_add(losers,&victim);
	if (Player_is_tank(killer)) {
	    i_tank_owner = GetInd(killer->lock.pl_id);
	    if (i_tank_owner == GetInd(victim->id))
		i_tank_owner = GetInd(killer->id);
	    true_killer = Player_by_index(i_tank_owner);
	    Rank_add_tank_kill(true_killer);
    	    Arraylist_add(winners,&true_killer);
    	    Generic_Scoring(winners,losers,options.tankKillScoreMult,NULL);
	} else {
	    Rank_add_runover_kill(killer);
    	    Arraylist_add(winners,&killer);
    	    Generic_Scoring(winners,losers,options.tankKillScoreMult,NULL);
	}
	break;
    case SCORE_BALL_KILL:
    	Arraylist_add(losers,&victim);
	if (!killer) {
    	    Generic_Scoring(winners,losers,options.ballKillScoreMult
    	    	    	    * options.unownedKillScoreMult,"Ball");
	} else {
	    if (killer == victim) {
    	    	Generic_Scoring(winners,losers,options.ballKillScoreMult
    	    	    	    	* options.selfKillScoreMult,NULL);
	    } else {
    	    	Arraylist_add(winners,&killer);
    	    	Generic_Scoring(winners,losers,options.ballKillScoreMult,NULL);
	    }
	}
	break;
    case SCORE_HIT_MINE:
    	Arraylist_add(winners,&killer);
    	Arraylist_add(losers,&victim);
    	Generic_Scoring(winners,losers,options.mineScoreMult,NULL);
	break;
    case SCORE_EXPLOSION:
    	Arraylist_add(losers,&victim);
	if (!killer || killer->id == victim->id) {
    	    Generic_Scoring(winners,losers,options.explosionKillScoreMult
    	    	    	    * options.selfKillScoreMult,
    	    	    	    (killer == NULL) ? "[Explosion]" : victim->name);
	} else {
    	    Arraylist_add(winners,&killer);
    	    Generic_Scoring(winners,losers,options.explosionKillScoreMult,NULL);
	}
	break;
    case SCORE_ASTEROID_KILL:
    	Arraylist_add(winners,&killer);
    	Generic_Scoring(winners,losers,options.asteroidKillScoreMult,NULL);
	break;
    case SCORE_ASTEROID_DEATH:
    	Arraylist_add(losers,&victim);
    	Generic_Scoring(winners,losers,options.asteroidDeathScoreMult,"[Asteroid]");
	break;
    case SCORE_SHOT_DEATH:
	if (BIT(((object_t *) extra)->obj_status, FROMCANNON)) {
    	    cannon_t *cannon;
    	    cannon = Cannon_by_id(((object_t *)extra)->id);

	    if (options.survivalScore != 0.0) {
	    	double mult;
    	    	factor = Get_Score(victim) * 0.02 / 10;
		mult = victim->survival_time * victim->survival_time * options.survivalScore / 2.0;
	    	Handle_Scoring(SCORE_BONUS,victim,NULL,&mult,"[Survival Time]");
    	    } else {
    	    	factor = options.cannonKillScoreMult;
    	    }
	} else if (((object_t *) extra)->id == NO_ID) {
    	    factor = options.unownedKillScoreMult;
	} else {
	    if (killer->id == victim->id) {
    	    	factor = options.selfKillScoreMult;
	    } else {
		Rank_add_shot_kill(killer);
    	    	factor = 1.0;
	    }
	}

	switch (((object_t *) extra)->type) {
	case OBJ_SHOT:
	    if (Mods_get(((object_t *) extra)->mods, ModsCluster))
    	    	factor *= options.clusterKillScoreMult;
	    else
    	    	factor *= options.shotKillScoreMult;
	    break;
	case OBJ_TORPEDO:
    	    factor *= options.torpedoKillScoreMult;
	    break;
	case OBJ_SMART_SHOT:
    	    factor *= options.smartKillScoreMult;
	    break;
	case OBJ_HEAT_SHOT:
    	    factor *= options.heatKillScoreMult;
	    break;
	default:
    	    factor *= options.shotKillScoreMult;
	    break;
	}
    
    	Arraylist_add(losers,&victim);
    	
	if (BIT(((object_t *) extra)->obj_status, FROMCANNON)) {
    	    Generic_Scoring(winners,losers,factor,"Cannon");
	} else if ((((object_t *) extra)->id == NO_ID)
		   || (killer && (killer->id == victim->id))) {
    	    Generic_Scoring(winners,losers,factor,
    	    	((object_t *)extra)->id == NO_ID ? NULL : victim->name);
	} else {
    	    Arraylist_add(winners,&killer);
    	    Generic_Scoring(winners,losers,factor,NULL);
	}

	break;
    case SCORE_LASER:
    	Arraylist_add(losers,&victim);
	if (killer) {
    	    Arraylist_add(winners,&killer);
    	    factor = options.laserKillScoreMult;
	    if (victim->id == killer->id) {
    	    	factor *= options.selfKillScoreMult;
    	    } else {
		Rank_add_laser_kill(killer);
	    }
    	    Generic_Scoring(winners,losers,factor,NULL);
	} else if (((cannon_t *) extra) != NULL) {
    	    Generic_Scoring(winners,losers,options.cannonKillScoreMult,"Cannon");
	} else {
    	    Generic_Scoring(winners,losers,options.unownedKillScoreMult,NULL);
	    Set_message_f("%s got roasted alive.", victim->name);
	}
	break;
    case SCORE_TARGET:
	{
    	    int targets_remaining = 0, targets_total = 0;
	    target_t *targ = (target_t *) extra;
    	    bool somebody = false;

	    if (BIT(world->rules->mode, TEAM_PLAY)) {
		for (j = 0; j < NumPlayers; j++) {
		    player_t *pl = Player_by_index(j);

		    if (Player_is_tank(pl)
			|| (Player_is_paused(pl) && pl->pause_count <= 0)
			|| Player_is_waiting(pl))
			continue;

		    if (pl->team == targ->team) {
    	    	    	Arraylist_add(losers,&pl);
			if (!Player_is_dead(pl))
			    somebody = true;
    	    	    } else if (pl->team == killer->team) {
    	    	    	Arraylist_add(winners,&pl);
		    }
		}
	    }
	    
	    if (somebody) {
		for (j = 0; j < Num_targets(); j++) {
		    target_t *t = Target_by_index(j);

		    if (t->team == targ->team) {
			targets_total++;
			if (t->dead_ticks <= 0)
			    targets_remaining++;
		    }
		}
    	    } else {
		break;
    	    }

	    sound_play_sensors(targ->pos, DESTROY_TARGET_SOUND);

	    if (targets_remaining > 0) {
    	    	factor = options.targetScoreMult / 4 * 
    	    	    	(targets_total - targets_remaining) / (targets_total + 1);
    	    	Generic_Scoring(winners,losers,factor,"Target: ");
		break;
	    }

	    if (options.targetKillTeam)
		Rank_add_target_kill(killer);

    	    Generic_Scoring(winners,losers,options.targetScoreMult
    	    	    *Arraylist_get_num_elements(losers),"Target: ");

	    for (j = 0; j < NumPlayers; j++) {
		player_t *pl = Player_by_index(j);

		if (Player_is_tank(pl)
		    || (Player_is_paused(pl) && pl->pause_count <= 0)
		    || Player_is_waiting(pl))
		    continue;

		if (pl->team == targ->team) {
		    if (options.targetKillTeam
			&& targets_remaining == 0 && Player_is_alive(pl))
			Player_set_state(pl, PL_STATE_KILLED);
    	    	}
	    }
	    break;
	}
    case SCORE_TREASURE:
	{
    	    int i;
	    treasure_t *treasure = (treasure_t *) extra;
	    bool somebody = false;

	    if (BIT(world->rules->mode, TEAM_PLAY)) {
		for (i = 0; i < NumPlayers; i++) {
    	    	    player_t *pl = Player_by_index(i);

    	    	    if (Player_is_tank(pl)
    	    	    	|| (Player_is_paused(pl) && pl->pause_count <= 0)
    	    	    	|| Player_is_waiting(pl))
			continue;
    	    	    if (pl->team == treasure->team) {
    	    	    	Arraylist_add(losers,&pl);
			somebody = true;
    	    	    } else if (pl->team == killer->team) {
    	    	    	Arraylist_add(winners,&pl);
		    }
		}
	    }

	    if (!somebody) {
    	    	Generic_Scoring(winners,losers,0.0,"Treasure: ");
		break;
	    }

    	    Generic_Scoring(winners,losers,options.treasureScoreMult
    	    	*Arraylist_get_num_elements(losers),"Treasure: ");

	    treasure->destroyed++;
	    world->teams[treasure->team].TreasuresLeft--;
	    world->teams[killer->team].TreasuresDestroyed++;

	    for (i = 0; i < NumPlayers; i++) {
		player_t *pl_i = Player_by_index(i);

		if (Player_is_tank(pl_i)
		    || (Player_is_paused(pl_i) && pl_i->pause_count <= 0)
		    || Player_is_waiting(pl_i))
		    continue;

		if (pl_i->team == treasure->team) {
		    Rank_lost_ball(pl_i);
		    if (options.treasureKillTeam)
			Player_set_state(pl_i, PL_STATE_KILLED);
    	    	} else if (pl_i->team == killer->team &&
			   (pl_i->team != TEAM_NOT_SET
			    || pl_i->id == killer->id)) {
    	    	    if (pl_i->id == killer->id)
    	    	    	Rank_cashed_ball(pl_i);
    	    	    Rank_won_ball(pl_i);
		}
	    }

	    if (options.treasureKillTeam)
		Rank_add_treasure_kill(killer);

	    break;
	}
    case SCORE_TREASURE_REPLACE:
    	Arraylist_add(winners,&killer);
    	Generic_Scoring(winners,losers,0.5,"Treasure: ");
    	break;
    case SCORE_SELF_DESTRUCT:
    	Arraylist_add(losers,&killer);
    	Generic_Scoring(winners,losers,options.selfDestructScoreMult,"Self-Destruct");
	break;
    case SCORE_SHOVE_KILL:
    	Arraylist_add(winners,&killer);
    	Generic_Scoring(winners,losers,options.shoveKillScoreMult
    	    	    	* (*((double *)extra)), victim->name);
	break;
    case SCORE_SHOVE_DEATH:
    	Arraylist_add(losers,&victim);
    	Generic_Scoring(winners,losers,options.shoveKillScoreMult
    	    	    	* (*((double *)extra)), "[Shove]");
	break;
    case SCORE_BONUS:
    	Arraylist_add(winners,&killer);
    	Generic_Scoring(winners,losers,*((double *)extra), somemsg);
    	break;
    default:
	error("unknown scoring type!");
	break;
    }
    
    Arraylist_clear(winners);
    Arraylist_clear(losers);
}

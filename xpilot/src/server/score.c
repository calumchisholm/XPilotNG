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


char score_version[] = VERSION;


void Score(player *pl, double points, clpos pos, const char *msg)
{
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	if (!teamShareScore)
	    Rank_AddScore(pl, points);
	TEAM_SCORE(pl->team, points);
    } else {
	if (pl->alliance != ALLIANCE_NOT_SET && teamShareScore)
	    Alliance_score(pl->alliance, points);
	else
	    Rank_AddScore(pl, points);
    }

    if (pl->conn != NULL)
	Send_score_object(pl->conn, points, pos, msg);

    updateScores = true;
}

void TEAM_SCORE(int team, double points)
{
    if (team == TEAM_NOT_SET)	/* could happen if teamCannons is off */
	return;

    World.teams[team].score += points;
    if (teamShareScore) {
	int i;
	double share = World.teams[team].score / World.teams[team].NumMembers;
	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);
	    if (pl_i->team == team)
		Rank_SetScore(pl_i, share);
	}
    }

    updateScores = true;
}

void Alliance_score(int id, double points)
{
    int		i;
    int		member_count = Get_alliance_member_count(id);
    double	share = points / member_count;

    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);
	if (pl_i->alliance == id)
	    Rank_AddScore(pl_i, share);
    }
}

double Rate(double winner, double loser)
{
    double t;

    if (constantScoring)
	return RATE_SIZE / 2;
    t = ((RATE_SIZE / 2) * RATE_RANGE) / (ABS(loser - winner) + RATE_RANGE);
    if (loser > winner)
	t = RATE_SIZE - t;
    return (t);
}

/*
 * Cause `winner' to get `winner_score' points added with message
 * `winner_msg', and similarly with the `loser' and equivalent
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
void Score_players(player *winner_pl, double winner_score, char *winner_msg,
		   player *loser_pl, double loser_score, char *loser_msg)
{
    if (TEAM(winner_pl, loser_pl)
	|| ALLIANCE(winner_pl, loser_pl)
	|| (IS_TANK_PTR(loser_pl)
	    && loser_pl->lock.pl_id == winner_pl->id)) {
	if (winner_score > 0)
	    winner_score = -winner_score;
	if (loser_score > 0)
	    loser_score = -loser_score;
    }

    if (tagGame && winner_score > 0.0) {
	if (tag == winner_pl->id) {
	    winner_score *= 2;
	    loser_score *= 2;
	} else if (tag == loser_pl->id) {
	    winner_score *= 10;
	    loser_score *= 10;
	    Transfer_tag(loser_pl, winner_pl);
	}
    }

    Score(winner_pl, winner_score, loser_pl->pos, winner_msg);
    Score(loser_pl, loser_score, loser_pl->pos, loser_msg);
}

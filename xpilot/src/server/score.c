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

void Score(player_t *pl, double points, clpos_t pos, const char *msg)
{
    Rank_add_score(pl, points);
    if (pl->conn != NULL)
	Send_score_object(pl->conn, points, pos, msg);
    updateScores = true;
}

double Rate(double winner, double loser)
{
    double t;

    if (options.constantScoring)
	return RATE_SIZE / 2;
    t = ((RATE_SIZE / 2) * RATE_RANGE) / (ABS(loser - winner) + RATE_RANGE);
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
void Score_players(player_t *winner_pl, double winner_score, char *winner_msg,
		   player_t *loser_pl, double loser_score, char *loser_msg,
		   bool transfer_tag)
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
	&& winner_score > 0.0 && loser_score < 0.0
	&& transfer_tag) {
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

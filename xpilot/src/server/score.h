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

#ifndef SCORE_H
#define SCORE_H

#define CANNON_SCORE	    	-1436.0
#define WALL_SCORE	    	2000.0

#define RATE_SIZE	    	20
#define RATE_RANGE	    	1024

/* score.c */

void Score(player_t *pl, double points, clpos_t pos, const char *msg);
void TEAM_SCORE(int team, double points);
void Alliance_score(int id, double points);
double Rate(double winner, double loser);

void Score_players(player_t *winner_pl, double winner_score, char *winner_msg,
		   player_t *loser_pl, double loser_score, char *loser_msg,
		   bool transfer_tag);

#endif

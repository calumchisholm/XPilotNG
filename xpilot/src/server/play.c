/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#ifdef _WINDOWS
#include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "saudio.h"
#include "score.h"
#include "objpos.h"
#include "click.h"
#include "object.h"

char play_version[] = VERSION;

extern int Rate(int winner, int loser);

static int Punish_team(int ind, int t_destroyed, int posx, int posy)
{
    static char		msg[MSG_LEN];
    treasure_t		*td = &World.treasures[t_destroyed];
    player		*pl = Players[ind];
    int			i;
    int			win_score = 0,lose_score = 0;
    int			win_team_members = 0, lose_team_members = 0;
    int			somebody_flag = 0;
    int			sc, por;

    Check_team_members (td->team);
    if (td->team == pl->team)
	return 0;

    if (BIT(World.rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    if (IS_TANK_IND(i)
		|| (BIT(Players[i]->status, PAUSE)
		    && Players[i]->count <= 0)
		|| (BIT(Players[i]->status, GAME_OVER)
		    && Players[i]->mychar == 'W')) {
		continue;
	    }
	    if (Players[i]->team == td->team) {
		lose_score += Players[i]->score;
		lose_team_members++;
		if (BIT(Players[i]->status, GAME_OVER) == 0) {
		    somebody_flag = 1;
		}
	    }
	    else if (Players[i]->team == pl->team) {
		win_score += Players[i]->score;
		win_team_members++;
	    }
	}
    }

    sound_play_all(DESTROY_BALL_SOUND);
    sprintf(msg, " < %s's (%d) team has destroyed team %d treasure >",
	    pl->name, pl->team, td->team);
    Set_message(msg);

    if (!somebody_flag) {
	SCORE(ind, Rate(pl->score, CANNON_SCORE)/2,
	      posx, posy, "Treasure:");
	return 0;
    }

    td->destroyed++;
    World.teams[td->team].TreasuresLeft--;
    World.teams[pl->team].TreasuresDestroyed++;


    sc  = 3 * Rate(win_score, lose_score);
    por = (sc * lose_team_members) / (2 * win_team_members + 1);

    for (i = 0; i < NumPlayers; i++) {
	if (IS_TANK_IND(i)
	    || (BIT(Players[i]->status, PAUSE)
		&& Players[i]->count <= 0)
	    || (BIT(Players[i]->status, GAME_OVER)
		&& Players[i]->mychar == 'W'
		&& Players[i]->score == 0)) {
	    continue;
	}
	if (Players[i]->team == td->team) {
	    SCORE(i, -sc, posx, posy,
		  "Treasure: ");
	    Rank_lost_ball(Players[i]);
	    if (treasureKillTeam)
		SET_BIT(Players[i]->status, KILLED);
	}
	else if (Players[i]->team == pl->team &&
		 (Players[i]->team != TEAM_NOT_SET || i == ind)) {
	    if (i == ind && lose_team_members > 0)
		Rank_cashed_ball(Players[i]);
	    Rank_won_ball(Players[i]);
	    SCORE(i, (i == ind ? 3*por : 2*por), posx, posy,
		  "Treasure: ");
	}
    }

    if (treasureKillTeam)
	Rank_kill(Players[ind]);

    updateScores = true;

    return 1;
}


void Ball_hits_goal(object *ball, int group)
{
    char msg[MSG_LEN];
    if (ball->owner == -1) {	/* Probably the player quit */
	SET_BIT(ball->status, (NOEXPLOSION|RECREATE));
	return;
    }
    if (World.treasures[ball->treasure].team == groups[group].team) {
	/*
	 * Ball has been replaced back in the hoop from whence
	 * it came. The player must be from the same team as the ball,
	 * otherwise Bounce_object() wouldn't have been called. It
	 * should be replaced into the hoop without exploding and
	 * the player gets some points.
	 */
	treasure_t	*tt = &World.treasures[ball->treasure];
	player	*pl = Players[GetInd[ball->owner]];

	SET_BIT(ball->status, (NOEXPLOSION|RECREATE));

	SCORE(GetInd[pl->id], 5,
	      tt->pos.x, tt->pos.y, "Treasure: ");
	sprintf(msg, " < %s (team %d) has replaced the treasure >",
		pl->name, pl->team);
	Set_message(msg);
	Rank_saved_ball(pl);
	return;
    }
    /*
     * Ball has been brought back to home treasure.
     * The team should be punished.
     */
    sprintf(msg," < The ball was loose for %ld frames >",
	    LONG_MAX - ball->life);
    Set_message(msg);
    if (Punish_team(GetInd[ball->owner], ball->treasure,
		    ball->pos.cx, ball->pos.cy))
	CLR_BIT(ball->status, RECREATE);
}


/****************************
 * Functions for explosions.
 */

/* Create debris particles */
void Make_debris(
    /* pos.x, pos.y   */ int  x, int y,
    /* vel.x, vel.y   */ DFLOAT  velx,       DFLOAT vely,
    /* owner id       */ int    id,
    /* owner team     */ u_short team,
    /* type           */ int    type,
    /* mass           */ DFLOAT  mass,
    /* status         */ long   status,
    /* color          */ int    color,
    /* radius         */ int    radius,
    /* min,max debris */ int    min_debris, int    max_debris,
    /* min,max dir    */ int    min_dir,    int    max_dir,
    /* min,max speed  */ DFLOAT  min_speed,  DFLOAT  max_speed,
    /* min,max life   */ int    min_life,   int    max_life
)
{
    object		*debris;
    int			i, num_debris, life;
    modifiers		mods;

    x = WRAP_XCLICK(x);
    y = WRAP_YCLICK(y);

    if (max_life < min_life)
	max_life = min_life;
    if (ShotsLife >= FPS) {
	if (min_life > ShotsLife) {
	    min_life = ShotsLife;
	    max_life = ShotsLife;
	} else if (max_life > ShotsLife) {
	    max_life = ShotsLife;
	}
    }
    if (min_speed * max_life > World.hypotenuse)
	min_speed = World.hypotenuse / max_life;
    if (max_speed * min_life > World.hypotenuse)
	max_speed = World.hypotenuse / min_life;
    if (max_speed < min_speed)
	max_speed = min_speed;

    CLEAR_MODS(mods);

    if (type == OBJ_SHOT) {
	SET_BIT(mods.warhead, CLUSTER);
	if (!ShotsGravity) {
	    CLR_BIT(status, GRAVITY);
	}
    }

    num_debris = min_debris + (int)(rfrac() * (max_debris - min_debris));
    if (num_debris > MAX_TOTAL_SHOTS - NumObjs) {
	num_debris = MAX_TOTAL_SHOTS - NumObjs;
    }
    for (i = 0; i < num_debris; i++, NumObjs++) {
	DFLOAT		speed, dx, dy, diroff;
	int		dir, dirplus;

	debris = Obj[NumObjs];
	debris->color = color;
	debris->id = id;
	debris->team = team;
	debris->owner = -1;
	Object_position_init_clicks(debris, x, y);
	dir = MOD2(min_dir + (int)(rfrac() * (max_dir - min_dir)), RES);
	dirplus = MOD2(dir + 1, RES);
	diroff = rfrac();
	dx = tcos(dir) + (tcos(dirplus) - tcos(dir)) * diroff;
	dy = tsin(dir) + (tsin(dirplus) - tsin(dir)) * diroff;
	speed = min_speed + rfrac() * (max_speed - min_speed);
	debris->vel.x = velx + dx * speed;
	debris->vel.y = vely + dy * speed;
	debris->acc.x = 0;
	debris->acc.y = 0;
	debris->dir = dir;
	debris->mass = mass;
	debris->type = type;
	life = (int)(min_life + rfrac() * (max_life - min_life) + 1);
	if (life * speed > World.hypotenuse) {
	    life = (long)(World.hypotenuse / speed);
	}
	debris->life = life;
	debris->fuseframe = 0;
	debris->spread_left = 0;
	debris->pl_range = radius;
	debris->pl_radius = radius;
	debris->status = status;
	debris->mods = mods;
    }
}

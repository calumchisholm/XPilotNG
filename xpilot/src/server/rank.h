/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) TODO
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

#ifndef RANK_H
#define RANK_H

#include "xpcommon.h"

#ifndef OBJECT_H
/* need player */
#include "object.h"
#endif

typedef struct rankhead {
	char magic[4];
	uint32_t version;
	uint32_t entries;
} rankhead_t;

#define RANK_MAGIC		"rNk7"
#define RANK_VER_MK(maj,min)	((((maj)&0xffff)<<16) | ((min)&0xffff))
#define RANK_VER_MAJ(ver)	(((ver) >> 16)&0xffff)
#define RANK_VER_MIN(ver)	(((ver) >> 16)&0xffff)
#define RANK_VER_CUR_MAJ	2
#define RANK_VER_CUR_MIN	0
#define RANK_VER_CURRENT	RANK_VER_MK(RANK_VER_CUR_MAJ,RANK_VER_CUR_MIN)


typedef struct rankentry {
    char nick[MAX_CHARS];
    char real[MAX_CHARS];
    char host[MAX_CHARS];
    char logout[MAX_CHARS];
    uint32_t disk_score;
    uint32_t shots;
    uint32_t timestamp;

    uint16_t kills;
    uint16_t deaths;
    uint16_t rounds;
    uint16_t ballsSaved;

    uint16_t ballsLost;
    uint16_t ballsWon;
    uint16_t ballsCashed;
    uint16_t bestball;
} rankentry_t;

typedef struct rankinfo {
    rankentry_t entry;
    double score;
    player_t *pl;
} rankinfo_t;


static inline void Rank_set_logout_message(player_t *pl, const char *msg)
{
    if (pl->rank)
	strlcpy(pl->rank->entry.logout, msg, sizeof(pl->rank->entry.logout));
}

void Rank_get_stats(player_t *pl, char *buf);
rankinfo_t *Rank_get_by_name(char *name);
void Rank_nuke_score(rankinfo_t *rank);
void Rank_init_saved_scores(void);
void Rank_get_saved_score(player_t *pl);
void Rank_save_score(player_t *pl);
void Rank_write_score_file(void);
void Rank_write_webpage(void);
void Rank_show_ranks(void);

/* these 2 don't really touch the rank stuff */
static inline void Rank_clear_kills(player_t *pl)
{
    pl->kills = 0;
}

static inline void Rank_clear_deaths(player_t *pl)
{
    pl->deaths = 0;
}

static inline void Rank_add_score(player_t *pl, double points)
{
    pl->score += points;
    if (pl->rank)
	pl->rank->score += points;
}

static inline void Rank_set_score(player_t *pl, double points)
{
    pl->score = points;
    if (pl->rank)
	pl->rank->score = points;
}

static inline void Rank_fire_shot(player_t *pl)
{
    pl->shots++;
    if (pl->rank)
	pl->rank->entry.shots++;
}

static inline void Rank_add_kill(player_t *pl)
{
    pl->kills++;
    if (pl->rank)
	pl->rank->entry.kills++;
}

static inline void Rank_add_death(player_t *pl)
{
    pl->deaths++;
    if (pl->rank)
	pl->rank->entry.deaths++;
}

static inline void Rank_add_round(player_t *pl)
{
    if (pl->rank)
	pl->rank->entry.rounds++;
}

static inline void Rank_cashed_ball(player_t *pl)
{
    if (pl->rank)
	pl->rank->entry.ballsCashed++;
}

static inline void Rank_saved_ball(player_t *pl)
{
    if (pl->rank)
	pl->rank->entry.ballsSaved++;
}

static inline void Rank_won_ball(player_t *pl)
{
    if (pl->rank)
	pl->rank->entry.ballsWon++;
}

static inline void Rank_lost_ball(player_t *pl)
{
    if (pl->rank)
	pl->rank->entry.ballsLost++;
}

static inline void Rank_ballrun(player_t *pl, int tim)
{
    if (pl->rank && tim < pl->rank->entry.bestball)
	pl->rank->entry.bestball = tim;
}

static inline double Rank_get_best_ballrun(player_t *pl)
{
    return (double)(pl->rank ? pl->rank->entry.bestball : 65535);
}

static inline void Rank_add_ball_kill(player_t *pl)      { Rank_add_kill(pl); }
static inline void Rank_add_explosion_kill(player_t *pl) { Rank_add_kill(pl); }
static inline void Rank_add_laser_kill(player_t *pl)     { Rank_add_kill(pl); }
static inline void Rank_add_runover_kill(player_t *pl)   { Rank_add_kill(pl); }
static inline void Rank_add_shot_kill(player_t *pl)      { Rank_add_kill(pl); }
static inline void Rank_add_shove_kill(player_t *pl)     { Rank_add_kill(pl); }
static inline void Rank_add_tank_kill(player_t *pl)      { Rank_add_kill(pl); }
static inline void Rank_add_target_kill(player_t *pl)    { Rank_add_kill(pl); }
static inline void Rank_add_treasure_kill(player_t *pl)  { Rank_add_kill(pl); }


#endif /* RANK_H */

#ifndef RANK_H
#define RANK_H

#include "xpcommon.h"

struct oldScoreNode {
    char nick[MAX_CHARS];
    char real[MAX_CHARS];
    char host[MAX_CHARS];
    char logout[MAX_CHARS];
    int  timestamp;
    short score;
    unsigned short kills;
    unsigned short deaths;
    unsigned short rounds;
    unsigned long  firedShots;

    unsigned short ballsSaved;
    unsigned short ballsLost;
    unsigned short ballsWon;
    unsigned short ballsCashed;
    unsigned short bestball;

    char  futureextensions[6];
    struct player *pl;
};


typedef struct RankHead {
	char magic[4];
	uint32_t version;
	uint32_t entries;
} RankHead;
#define RANK_MAGIC		"rNk7"
#define RANK_VER_MK(maj,min)	((((maj)&0xffff)<<16) | ((min)&0xffff))
#define RANK_VER_MAJ(ver)	(((ver) >> 16)&0xffff)
#define RANK_VER_MIN(ver)	(((ver) >> 16)&0xffff)
#define RANK_VER_CUR_MAJ	2
#define RANK_VER_CUR_MIN	0
#define RANK_VER_CURRENT	RANK_VER_MK(RANK_VER_CUR_MAJ,RANK_VER_CUR_MIN)


typedef struct RankEntry {
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
} RankEntry;

typedef struct RankInfo {
    struct RankEntry entry;
    DFLOAT score;
    struct player *pl;
} RankInfo;


#define Rank_SetLogoutMessage(rank,msg) \
	do { strcpy((rank)->entry.logout, (msg)); } while(0)

#if 0
void Rank_get_saved_score(struct player *pl);
void Rank_write_score_file(void);
void Rank_write_webpage(void);
void Rank_save_score(const struct player * pl);
void Rank_get_stats(struct player *pl, char *buf);
void Rank_show_ranks(void);
bool Rank_IsLegalNameUserHost(const char string[]);
RankInfo *Rank_get_by_name(char *name);
void Rank_nuke_score(RankInfo *rank);
#endif
void Rank_init_saved_scores(void);
void Rank_get_saved_score(player *pl);
void Rank_save_data(void);
void Rank_web_scores(void);
void Rank_save_score(const player *pl);
void Rank_show_standings(void);
void Rank_kill(player *pl);
void Rank_lost_ball(player *pl);
void Rank_cashed_ball(player *pl);
void Rank_won_ball(player *pl);
void Rank_saved_ball(player *pl);
void Rank_death(player *pl);
void Rank_add_score(player *pl, DFLOAT points);
void Rank_set_score(player *pl, DFLOAT points);
void Rank_fire_shot(player *pl);


/*
 * Converted from C++,
 * moved here from object.h and renamed to have Rank_ prefix
 */
#define Rank_StartBallRun(pl) \
	{ (pl)->grabbedBallFrame = main_loops; }
#define Rank_AbortBallRun(pl) \
	{ (pl)->grabbedBallFrame = -1; }
#define Rank_ClearKills(pl)	{ (pl)->kills = 0; }
#define Rank_ClearDeaths(pl)	{ (pl)->deaths = 0; }
/*#define Rank_IgnoreLastDeath(pl) \
  { if ((pl)->rank) (pl)->rank->entry.deaths--;}*/

#define Rank_AddRound(pl) \
	{ if ((pl)->rank) (pl)->rank->entry.rounds++; }

#define Rank_FireShot(pl) \
	{ (pl)->shots++; if ((pl)->rank) (pl)->rank->entry.shots++; }

#define Rank_SavedBall(pl) \
	{ if ((pl)->rank) (pl)->rank->entry.ballsSaved++; }
#define Rank_LostBall(pl) \
	{ if ((pl)->rank) (pl)->rank->entry.ballsLost++; }
#define Rank_CashedBall(pl) \
	{ if ((pl)->rank) (pl)->rank->entry.ballsCashed++;}
#define Rank_WonBall(pl) \
	{ if ((pl)->rank) (pl)->rank->entry.ballsWon++; }
#define Rank_BallRun(pl,tim)	{ \
	 if ((pl)->rank && (tim) < (pl)->rank->entry.bestball) \
		 (pl)->rank->entry.bestball = (tim); }

#define Rank_AddDeath(pl) { \
	(pl)->deaths++; \
	if ((pl)->rank) (pl)->rank->entry.deaths++; \
}

#define Rank_AddKill(pl) { \
	(pl)->kills++; \
	if ((pl)->rank) (pl)->rank->entry.kills++; \
}

#define Rank_AddBallKill(pl) { Rank_AddKill(pl); }
#define Rank_AddTargetKill(pl) { Rank_AddKill(pl); }
#define Rank_AddLaserKill(pl) { Rank_AddKill(pl); }

#define Rank_AddScore(pl,add) { \
	(pl)->score += add; \
	if ((pl)->rank) (pl)->rank->score += add; \
}

#define Rank_SetScore(pl,newScore) { \
	(pl)->score = newScore; \
	if ((pl)->rank) (pl)->rank->score = newScore; \
}

#define Rank_GetBestBall(pl) \
	((pl)->rank ? (pl)->rank->entry.bestball : 65535)


#endif /* RANK_H */

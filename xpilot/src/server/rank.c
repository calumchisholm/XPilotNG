/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
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

char rank_version[] = VERSION;

/* MAX_SCORES = how many players we remember */
#define MAX_SCORES 200

#define XPILOTSCOREFILE		"XPILOTSCOREFILE"
#define XPILOTNOJSRANKINGPAGE	"XPILOTNOJSRANKINGPAGE"
#define RANKING_SERVER		"Ranking server"

#define PAGEHEAD \
/* Head of page */ \
"<h1>XPilot @ " RANKING_SERVER "</h1>" \
"<a href=\"previous_ranks.html\">Previous rankings</a> " \
"<a href=\"rank_explanation.html\">How does the ranking work?</a><hr>\n"


/* Score data */
static const char *xpilotscorefile = NULL;
static RankHead rank_head;
static RankInfo scores[MAX_SCORES];

static int rankedplayer[MAX_SCORES];
static double rankedscore[MAX_SCORES];
static double sc_table[MAX_SCORES];
static double kr_table[MAX_SCORES];
static double kd_table[MAX_SCORES];
static double hf_table[MAX_SCORES];

static inline void swap2(int *i1, int *i2, double *d1, double *d2)
{
    int i;
    double d;

    i = *i1;
    d = *d1;
    *i1 = *i2;
    *d1 = *d2;
    *i2 = i;
    *d2 = d;
}

static char *rank_showtime(void)
{
    time_t now;
    struct tm *tmp;
    static char month_names[13][4] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	"Bug"
    };
    static char buf[80];

    time(&now);
    tmp = localtime(&now);
    sprintf(buf, "%02d\xA0%s\xA0%02d:%02d:%02d",
	    tmp->tm_mday, month_names[tmp->tm_mon],
	    tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
    return buf;
}

/* Here's where we calculate the ranks. Figure it out yourselves! */
static void SortRankings(void)
{
    double lowSC = 0.0, highSC = 0.0;
    double lowKD = 0.0, highKD = 0.0;
    double lowKR = 0.0, highKR = 0.0;
    double lowHF = 0.0, highHF = 0.0;
    bool foundFirst = false;
    int k;

    /* Ok, there are two loops: the first one calculates the scores and
       records lowest and highest scores. The second loop combines the
       scores into a rank. I cannot do it in one loop since I need to
       know low- and highmarks for each score before I can calculate the
       rank. */
    for (k = 0; k < MAX_SCORES; k++) {
	RankInfo *score = &scores[k];
	double attenuation, kills, sc, kd, kr, hf;
	if (score->entry.nick[0] == '\0')
	    continue;

	/* The attenuation affects players with less than 300 rounds. */
	attenuation = (score->entry.rounds < 300) ?
	    ((double) score->entry.rounds / 300.0) : 1.0;

	kills = score->entry.kills;
	sc = (double) score->score * attenuation;
	kd = ((score->entry.deaths != 0) ?
	      (kills / (double) score->entry.deaths) :
	      (kills)) * attenuation;
	kr = ((score->entry.rounds != 0) ?
	      (kills / (double) score->entry.rounds) :
	      (kills)) * attenuation;
	hf = ((score->entry.ballsLost != 0) ?
	      ((double) score->entry.ballsCashed /
	       (double) score->entry.ballsLost) :
	      (double) score->entry.ballsCashed) * attenuation;

	sc_table[k] = sc;
	kd_table[k] = kd;
	kr_table[k] = kr;
	hf_table[k] = hf;

	if (!foundFirst) {
	    lowSC = highSC = sc;
	    lowKD = highKD = kd;
	    lowKR = highKR = kr;
	    lowHF = highHF = hf;
	    foundFirst = true;
	} else {
	    if (sc > highSC)
		highSC = sc;
	    else if (sc < lowSC)
		lowSC = sc;

	    if (kd > highKD)
		highKD = kd;
	    else if (kd < lowKD)
		lowKD = kd;

	    if (kr > highKR)
		highKR = kr;
	    else if (kr < lowKR)
		lowKR = kr;

	    if (hf > highHF)
		highHF = hf;
	    else if (hf < lowHF)
		lowHF = hf;
	}
    }

    /* Normalize */
    highSC -= lowSC;
    highKD -= lowKD;
    highKR -= lowKR;
    highHF -= lowHF;

    {
	const double factorSC = (highSC != 0.0) ? (100.0 / highSC) : 0.0;
	const double factorKD = (highKD != 0.0) ? (100.0 / highKD) : 0.0;
	const double factorKR = (highKR != 0.0) ? (100.0 / highKR) : 0.0;
	const double factorHF = (highHF != 0.0) ? (100.0 / highHF) : 0.0;
	int ranked_players = 0;
	int i;

	for (i = 0; i < MAX_SCORES; i++) {
	    RankInfo *score = &scores[i];
	    double sc, kd, kr, hf, rsc, rkd, rkr, rhf, rank;
	    rankedplayer[i] = i;
	    if (score->entry.nick[0] == '\0') {
		rankedscore[i] = -1;
		continue;
	    }
	    ranked_players++;

	    sc = sc_table[i];
	    kd = kd_table[i];
	    kr = kr_table[i];
	    hf = hf_table[i];

	    rsc = (sc - lowSC) * factorSC;
	    rkd = (kd - lowKD) * factorKD;
	    rkr = (kr - lowKR) * factorKR;
	    rhf = (hf - lowHF) * factorHF;

	    rank = 0.20 * rsc + 0.30 * rkd + 0.30 * rkr + 0.20 * rhf;
	    rankedscore[i] = rank;
	}
	rank_head.entries = ranked_players;

	/* And finally we sort the ranks, using some lame N^2 sort.. wheee! */
	for (i = 0; i < ranked_players; i++) {
	    int j;
	    for (j = i + 1; j < ranked_players; j++) {
		if (rankedscore[i] < rankedscore[j])
		    swap2(&rankedplayer[i], &rankedplayer[j],
			  &rankedscore[i], &rankedscore[j]);
	    }
	}
    }
}

#define TABLEHEAD \
"<table><tr><td></td>" /* First column is the position */ \
"<td align=left><h1><u><b>Player</b></u></h1></td>" \
"<td align=right><h1><u><b>Score</b></u></h1></td>" \
"<td align=right><h1><u><b>Kills</b></u></h1></td>" \
"<td align=right><h1><u><b>Deaths</b></u></h1></td>" \
"<td align=right><h1><u><b>Rounds</b></u></h1></td>" \
"<td align=right><h1><u><b>Shots</b></u></h1></td>" \
"<td align=center><h1><u><b>Balls</b></u></h1></td>" \
"<td align=right><h1><u><b>Ratio</b></u></h1></td>" \
"<td align=right><h1><u><b>User</b></u></h1></td>" \
"<td align=left><h1><u><b>Host</b></u></h1></td>" \
"<td align=center><h1><u><b>Logout</b></u></h1></td>" \
"</tr>\n"

/* Sort the ranks and save them to the webpage. */
void Rank_write_webpage(void)
{
    static const char headernojs[] =
	"<html><head><title>XPilot @ " RANKING_SERVER "</title>\n"
	"</head><body>\n" PAGEHEAD TABLEHEAD;

    static const char footer[] = "</table>"
	"<i>Explanation for ballstats</i>:<br>"
	"The numbers are c/s/w/l/b, where<br>"
	"c = The number of enemy balls you have cashed.<br>"
	"s = The number of your own balls you have returned.<br>"
	"w = The number of enemy balls your team has cashed.<br>"
	"l = The number of your own balls you have lost.<br>"
	"b = The fastest ballrun you have made.<br>"
	"<hr>%s<BR>\n\n"	/* <-- Insert time here. */
	"</body></html>";

    SortRankings();

    if (getenv(XPILOTNOJSRANKINGPAGE) != NULL) {
	FILE *const file = fopen(getenv(XPILOTNOJSRANKINGPAGE), "w");
	if (file != NULL) {
	    int i;
	    fprintf(file, "%s", headernojs);
	    for (i = 0; i < MAX_SCORES; i++) {
		const int j = rankedplayer[i];
		const RankInfo *score = &scores[j];
		if (score->entry.nick[0] != '\0') {
		    fprintf(file,
			    "<tr><td align=left><tt>%d</tt>"
			    "<td align=left><b>%s</b>"
			    "<td align=right>%.1f"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=center>%u/%u/%u/%u/%u"
			    "<td align=right>%.1f"
			    "<td align=right>%s"
			    "<td align=left>%s"
			    "<td align=center>%s\n"
			    "</tr>\n",
			    i + 1,
			    score->entry.nick,
			    score->score,
			    score->entry.kills,
			    score->entry.deaths,
			    score->entry.rounds,
			    score->entry.shots,
			    score->entry.ballsCashed,
			    score->entry.ballsSaved, score->entry.ballsWon,
			    score->entry.ballsLost, score->entry.bestball,
			    rankedscore[i], score->entry.real,
			    score->entry.host, score->entry.logout);
		}
	    }
	    fprintf(file, footer, rank_showtime());
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }
}

/* Return a line with the ranking status of the specified player. */
void Rank_get_stats(player * pl, char *buf)
{
    RankInfo *score = pl->rank;

    sprintf(buf, "%-15s  %4d/%4d, R: %3d, S: %5d, %2d/%2d/%2d/%2d (%d)",
	    pl->name, score->entry.kills, score->entry.deaths,
	    score->entry.rounds, score->entry.shots,
	    score->entry.ballsCashed, score->entry.ballsSaved,
	    score->entry.ballsWon, score->entry.ballsLost,
	    score->entry.bestball);
}


/* Send a line with the ranks of the current players to the game. */
void Rank_show_ranks(void)
{
    char buf[1000] = "";
    int i;

    for (i = 0; i < MAX_SCORES; i++) {
	RankInfo *score = &scores[rankedplayer[i]];
	if (score->pl != NULL) {
	    char msg[MSG_LEN];
	    sprintf(msg, "%s [%d], ", score->entry.nick, i + 1);
	    strcat(buf, msg);
	}
    }

    Set_message(buf);
    return;
}


static void
Init_scorenode(RankInfo * node,
	       const char nick[], const char real[], const char host[])
{
    strlcpy(node->entry.nick, nick, MAX_CHARS);
    strlcpy(node->entry.real, real, MAX_CHARS);
    strlcpy(node->entry.host, host, MAX_CHARS);
    strcpy(node->entry.logout, "");
    node->score = 0;
    node->entry.kills = 0;
    node->entry.deaths = 0;
    node->entry.rounds = 0;
    node->entry.shots = 0;
    node->entry.ballsSaved = 0;
    node->entry.ballsLost = 0;
    node->entry.ballsCashed = 0;
    node->entry.ballsWon = 0;
    node->entry.bestball = 65535;
    node->pl = NULL;
}


RankInfo *Rank_get_by_name(char *name)
{
    RankInfo *score;
    int i;

    for (i = 0; i < MAX_SCORES; i++) {
	score = &scores[i];
	if (strcmp(name, score->entry.nick) == 0)
	    return score;
    }

    return NULL;
}


void Rank_nuke_score(RankInfo * node)
{
    Init_scorenode(node, "", "", "");
    node->entry.timestamp = 0;
}


/* Read scores from disk, and zero-initialize the ones that are not used.
   Call this on startup. */
void Rank_init_saved_scores(void)
{
    int i = 0;

    xpilotscorefile = getenv(XPILOTSCOREFILE);

    if (getenv("XPILOTRANKINGPAGE") != NULL)
	warn("Support for Javascript ranking pages has been removed.");

    if (xpilotscorefile != NULL) {
	FILE *file = fopen(xpilotscorefile, "r");
	if (file != NULL) {
	    int actual;

	    actual = fread(&rank_head, sizeof(RankHead), 1, file);
	    if (actual != 1) {
		error("Error when reading score file!\n");
		goto init_tail;
	    }
	    if (memcmp(rank_head.magic, RANK_MAGIC, 4) != 0) {
		warn("Rank file format is too old.\n");
		goto init_tail;
	    }
	    rank_head.version = ntohl(rank_head.version);
	    rank_head.entries = ntohl(rank_head.entries);
	    switch (RANK_VER_MAJ(rank_head.version)) {
	    case 2:
		/* Current version. */
		xpprintf("%s Rank file with %d entries opened successfully.\n",
			 showtime(), rank_head.entries);
		break;
	    default:
		error("Unknown version of score file!\n");
		goto init_tail;
	    }

	    for (i = 0; i < (int)rank_head.entries; i++) {
		RankInfo *node = &scores[i];
		actual = fread(&node->entry, sizeof(node->entry), 1, file);
		if (actual != 1) {
		    error("Error when reading score file!\n");
		    break;
		}
		node->entry.shots = ntohl(node->entry.shots);
		node->entry.timestamp = ntohl(node->entry.timestamp);
		node->entry.kills = ntohs(node->entry.kills);
		node->entry.deaths = ntohs(node->entry.deaths);
		node->entry.rounds = ntohs(node->entry.rounds);
		node->entry.ballsSaved = ntohs(node->entry.ballsSaved);
		node->entry.ballsLost = ntohs(node->entry.ballsLost);
		node->entry.ballsWon = ntohs(node->entry.ballsWon);
		node->entry.ballsCashed = ntohs(node->entry.ballsCashed);
		node->entry.bestball = ntohs(node->entry.bestball);
		node->score =
		    (double) ((int32_t) ntohl(node->entry.disk_score))
		    / 65536.0;
		/* Fix buggy first implementation... */
		if (node->score >= 65000)
		    node->score -= 65536;
		node->pl = NULL;
	    }
	    fclose(file);
	}
    }

  init_tail:
    while (i < MAX_SCORES) {
	Init_scorenode(&scores[i], "", "", "");
	scores[i].entry.timestamp = 0;
	i++;
    }
}

/* A player has logged in. Find his info or create new info by kicking
   the player who hasn't played for the longest time. */
void Rank_get_saved_score(player * pl)
{
    RankInfo *score;
    int oldest = 0;
    int i;
    updateScores = true;

    for (i = 0; i < MAX_SCORES; i++) {
	score = &scores[i];
	if (strcmp(pl->name, score->entry.nick) == 0) {
	    if (score->pl == NULL) {
		/* Ok, found it. */
		score->pl = pl;
		strcpy(score->entry.logout, "playing");
		pl->score = score->score;
		pl->rank = score;
		return;
	    } else {
		/* That scorenode is already in use by another player! */
		pl->score = 0;
		pl->rank = NULL;
		return;

	    }
	} else if (score->entry.timestamp < scores[oldest].entry.timestamp)
	    oldest = i;
    }

    /* Didn't find it, use the least-recently-used node. */
    score = &scores[oldest];

    Init_scorenode(score, pl->name, pl->realname, pl->hostname);
    strcpy(score->entry.logout, "playing");
    score->pl = pl;
    score->entry.timestamp = time(0);
    pl->score = 0;
    pl->rank = score;
}

/* A player has quit, save his info and mark him as not playing. */
void Rank_save_score(const player * pl)
{
    RankInfo *score = pl->rank;
    score->score = pl->score;
    strlcpy(score->entry.logout, rank_showtime(), MAX_CHARS);
    score->pl = NULL;
    score->entry.timestamp = time(NULL);
}

/* Save the scores to disk (not the webpage). */
void Rank_write_score_file(void)
{
    FILE *file = NULL;
    char tmp_file[4096];
    RankHead head;
    int actual;
    int i;

    if (!xpilotscorefile)
	return;
    actual = snprintf(tmp_file, sizeof(tmp_file), "%s-new", xpilotscorefile);
    if (actual < (int)strlen(xpilotscorefile)
	|| actual > (int)sizeof(tmp_file))
	/* Use a shorter path-name and be happy... */
	return;

    file = fopen(tmp_file, "w");
    if (file == NULL)
	return;

    memcpy(head.magic, RANK_MAGIC, 4);
    head.version = htonl(RANK_VER_CURRENT);
    head.entries = htonl(rank_head.entries);
    actual = fwrite(&head, sizeof(head), 1, file);
    if (actual != 1) {
	error("Error when writing score file head!\n");
	fclose(file);
	remove(tmp_file);
	return;
    }
    for (i = 0; i < (int)rank_head.entries; i++) {
	RankEntry entry;
	int idx = rankedplayer[i];

	memcpy(&entry, &scores[idx].entry, sizeof(entry));
	entry.disk_score = htonl(((uint32_t) (int32_t)
				  (scores[idx].score * 65536)));
	entry.shots = htonl(entry.shots);
	entry.timestamp = htonl(entry.timestamp);
	entry.kills = htons(entry.kills);
	entry.deaths = htons(entry.deaths);
	entry.rounds = htons(entry.rounds);
	entry.ballsSaved = htons(entry.ballsSaved);
	entry.ballsLost = htons(entry.ballsLost);
	entry.ballsWon = htons(entry.ballsWon);
	entry.ballsCashed = htons(entry.ballsCashed);
	entry.bestball = htons(entry.bestball);
	actual = fwrite(&entry, sizeof(entry), 1, file);
	if (actual != 1) {
	    error("Error when writing score file!\n");
	    break;
	}
    }
    fclose(file);

    /* Overwrite old score file. */
    rename(tmp_file, xpilotscorefile);
    remove(tmp_file);
}

/* This function checks wether the strings contains certain characters
   that might be hazardous to include on a webpage (ie they screw it up). */
bool Rank_IsLegalNameUserHost(const char string[])
{
    const int length = strlen(string);
    int i;

    for (i = 0; i < length; i++)
	switch (string[i]) {
	case '<':
	case '>':
	case '\t':
	    return false;
	default:
	    break;
	}
    return true;
}

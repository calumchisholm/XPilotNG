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

#include "xpserver.h"

char rank_version[] = VERSION;

/* MAX_SCORES = how many players we remember */
#define MAX_SCORES 250

#define XPILOTSCOREFILE		"XPILOTSCOREFILE"
#define XPILOTNOJSRANKINGPAGE	"XPILOTNOJSRANKINGPAGE"
#define RANKING_SERVER		"Ranking server"

#define PAGEHEAD \
/* Head of page */ \
"<h1>XPilot @ " RANKING_SERVER "</h1>" \
"<a href=\"previous_ranks.html\">Previous rankings</a> " \
"<a href=\"rank_explanation.html\">How does the ranking work?</a><hr>\n"

static bool Rank_parse_scorefile(FILE *file);

/* Score data */
static const char *xpilotscorefile = NULL;
static rankinfo_t scores[MAX_SCORES];

typedef struct rank {
    int ind;
    double ratio;
} rank_t;

static rank_t rank_base[MAX_SCORES];

static double sc_table[MAX_SCORES];
static double kr_table[MAX_SCORES];
static double kd_table[MAX_SCORES];
static double hf_table[MAX_SCORES];

static bool playerstag = false;
static int num_players = 0;
static int rank_entries = 0;

static int rank_cmp(const void *p1, const void *p2)
{
    rank_t *r1, *r2;

    r1 = (rank_t *)p1;
    r2 = (rank_t *)p2;

    /*
     * Function qsort(3) normally sorts array elements into ascending order.
     * We want a descending order (greatest ratio first), that's why we
     * tell qsort a greater ratio is less than a lesser ratio.
     */
    if (r1->ratio < r2->ratio)
	return 1;
    if (r1->ratio > r2->ratio)
	return -1;
    return 0;
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
	rankinfo_t *score = &scores[k];
	double attenuation, kills, sc, kd, kr, hf;

	if (score->name[0] == '\0')
	    continue;

	/* The attenuation affects players with less than 300 rounds. */
	attenuation = (score->rounds < 300) ?
	    ((double) score->rounds / 300.0) : 1.0;

	kills = score->kills;
	sc = (double) score->score * attenuation;
	kd = ((score->deaths != 0) ?
	      (kills / (double) score->deaths) :
	      (kills)) * attenuation;
	kr = ((score->rounds != 0) ?
	      (kills / (double) score->rounds) :
	      (kills)) * attenuation;
	hf = ((score->ballsLost != 0) ?
	      ((double) score->ballsCashed /
	       (double) score->ballsLost) :
	      (double) score->ballsCashed) * attenuation;

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
	    rankinfo_t *score = &scores[i];
	    double sc, kd, kr, hf, rsc, rkd, rkr, rhf;

	    rank_base[i].ind = i;
	    if (score->name[0] == '\0') {
		rank_base[i].ratio = -1;
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

	    rank_base[i].ratio
		= 0.20 * rsc + 0.30 * rkd + 0.30 * rkr + 0.20 * rhf;
	}
	rank_entries = ranked_players;

	/* And finally we sort the ranks, wheee! */
	qsort(rank_base, ranked_players, sizeof(rank_t), rank_cmp);
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
		const int j = rank_base[i].ind;
		const rankinfo_t *score = &scores[j];

		if (score->name[0] != '\0') {
		    fprintf(file,
			    "<tr><td align=left><tt>%d</tt>"
			    "<td align=left><b>%s</b>"
			    "<td align=right>%.1f"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=center>%u/%u/%u/%u/%.2f"
			    "<td align=right>%.1f"
			    "<td align=right>%s"
			    "<td align=left>%s"
			    "<td align=center>%s\n"
			    "</tr>\n",
			    i + 1,
			    score->name,
			    score->score,
			    score->kills,
			    score->deaths,
			    score->rounds,
			    score->shots,
			    score->ballsCashed,
			    score->ballsSaved,
			    score->ballsWon,
			    score->ballsLost,
			    score->bestball,
			    rank_base[i].ratio,
			    score->user,
			    score->host,
			    score->logout);
		}
	    }
	    fprintf(file, footer, rank_showtime());
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }
}

/* Return a line with the ranking status of the specified player. */
void Rank_get_stats(player_t * pl, char *buf)
{
    rankinfo_t *score = pl->rank;

    sprintf(buf, "%-15s  %4d/%4d, R: %3d, S: %5d, %d/%d/%d/%d/%.2f",
	    pl->name,
	    score->kills,
	    score->deaths,
	    score->rounds,
	    score->shots,
	    score->ballsCashed,
	    score->ballsSaved,
	    score->ballsWon,
	    score->ballsLost,
	    score->bestball);
}


/* Send a line with the ranks of the current players to the game. */
void Rank_show_ranks(void)
{
    char msg[MSG_LEN];
    char tmpbuf[MSG_LEN];
    int i, num = 0, numranks = 0;

    snprintf(msg, sizeof(msg), "Ranks: ");

    for (i = 0; i < MAX_SCORES; i++) {
	rankinfo_t *score = &scores[rank_base[i].ind];

	if (score->name[0] != '\0')
	    numranks++;

	if (score->pl != NULL) {
	    if (num > 0)
		strlcat(msg, ", ", sizeof(msg));
	    snprintf(tmpbuf, sizeof(tmpbuf), "%s [%d]", score->name, i + 1);
	    strlcat(msg, tmpbuf, sizeof(msg));
	    num++;
	}
    }

    strlcat(msg, ".", sizeof(msg));
    Set_message(msg);

    /* let's not show top rankings if playing teamcup */
    if (options.teamcup)
	return;

    /* show a few best ranks */
    snprintf(msg, sizeof(msg), " < Top %d ranks: ",
	     numranks < 3 ? numranks : 3);
    num = 0;
    for (i = 0; i < MAX_SCORES; i++) {
	rankinfo_t *score = &scores[rank_base[i].ind];

	if (score->name[0] == '\0')
	    continue;

	if (num > 0)
	    strlcat(msg, ", ", sizeof(msg));
	snprintf(tmpbuf, sizeof(tmpbuf), "%d. %s (%.1f)",
		 num + 1, score->name, rank_base[i].ratio);
	strlcat(msg, tmpbuf, sizeof(msg));
	num++;
	if (num > 2)
	    break;
    }

    strlcat(msg, " >", sizeof(msg));
    Set_message(msg);

    return;
}


static void Init_scorenode(rankinfo_t *node,
			   char *name, char *user, char *host)
{
    memset(node, 0, sizeof(rankinfo_t));
    strlcpy(node->name, name, sizeof(node->name));
    strlcpy(node->user, user, sizeof(node->user));
    strlcpy(node->host, host, sizeof(node->host));
}


rankinfo_t *Rank_get_by_name(char *name)
{
    rankinfo_t *score;
    int i;

    for (i = 0; i < MAX_SCORES; i++) {
	score = &scores[i];
	if (!strcasecmp(name, score->name))
	    return score;
    }

    return NULL;
}


void Rank_nuke_score(rankinfo_t * node)
{
    Init_scorenode(node, "", "", "");
    node->timestamp = 0;
}


/* Read scores from disk, and zero-initialize the ones that are not used.
   Call this on startup. */
void Rank_init_saved_scores(void)
{
    int i;
    FILE *file;

    for (i = 0; i < MAX_SCORES; i++) {
	rankinfo_t *rank = &scores[i];

	memset(rank, 0, sizeof(rankinfo_t));
    }

    xpilotscorefile = getenv(XPILOTSCOREFILE);
    if (!xpilotscorefile)
	return;

    file = fopen(xpilotscorefile, "r");
    if (!file)
	return;

    Rank_parse_scorefile(file);

    fclose(file);

    xpprintf("%s Rank file with %d entries opened successfully.\n",
	     showtime(), num_players);
}

/*
 * A player has logged in. Find his info or create new info by kicking
 * the player who hasn't played for the longest time.
 */
void Rank_get_saved_score(player_t * pl)
{
    rankinfo_t *score;
    int oldest = 0, i;

    updateScores = true;

    for (i = 0; i < MAX_SCORES; i++) {
	score = &scores[i];
	if (!strcasecmp(pl->name, score->name)) {
	    if (score->pl == NULL) {
		/* Ok, found it. */
		score->pl = pl;
		pl->score = score->score;
		pl->rank = score;
		Rank_set_logout_message(pl, "playing");
		return;
	    } else {
		/* That scorenode is already in use by another player! */
		pl->score = 0;
		pl->rank = NULL;
		return;

	    }
	} else if (score->timestamp < scores[oldest].timestamp)
	    oldest = i;
    }

    /* Didn't find it, use the least-recently-used node. */
    score = &scores[oldest];

    Init_scorenode(score, pl->name, pl->username, pl->hostname);
    score->pl = pl;
    score->timestamp = time(NULL);
    pl->score = 0;
    pl->rank = score;
    Rank_set_logout_message(pl, "playing");
}

/* A player has quit, save his info and mark him as not playing. */
void Rank_save_score(player_t * pl)
{
    rankinfo_t *rank = pl->rank;

    rank->score = pl->score;
    Rank_set_logout_message(pl, rank_showtime());
    rank->pl = NULL;
    rank->timestamp = time(NULL);
}

/* Save the scores to disk (not the webpage). */
void Rank_write_score_file(void)
{
    FILE *file = NULL;
    char tmp_file[PATH_MAX];
    int i;

    if (!xpilotscorefile)
	return;

    snprintf(tmp_file, sizeof(tmp_file), "%s-new", xpilotscorefile);

    file = fopen(tmp_file, "w");
    if (file == NULL) {
	error("Open temporary file \"%s\"", tmp_file);
	goto failed;
    }

    if (fprintf(file,
		"<?xml version=\"1.0\"?>\n"
		"<XPilotNGRank version=\"0.0\">\n"
		"<Players>\n") < 0)
	goto writefailed;
    

    for (i = 0; i < rank_entries; i++) {
	int idx = rank_base[i].ind;
	rankinfo_t *rank = &scores[idx];

	if (rank->name[0] == '\0')
	    continue;

	if (fprintf(file,
		    "<Player "
		    "name=\"%s\" user=\"%s\" host=\"%s\" ",
		    rank->name, rank->user, rank->host) < 0)
	    goto writefailed;

	if (rank->score != 0.0
	    && fprintf(file, "score=\"%.2f\" ", rank->score) < 0)
	    goto writefailed;

	if (rank->kills > 0
	    && fprintf(file, "kills=\"%d\" ", rank->kills) < 0)
	    goto writefailed;

	if (rank->deaths > 0
	    && fprintf(file, "deaths=\"%d\" ", rank->deaths) < 0)
	    goto writefailed;

	if (rank->rounds > 0
	    && fprintf(file, "rounds=\"%d\" ", rank->rounds) < 0)
	    goto writefailed;

	if (rank->shots > 0
	    && fprintf(file, "shots=\"%d\" ", rank->shots) < 0)
	    goto writefailed;

	if (rank->ballsCashed > 0
	    && fprintf(file, "ballscashed=\"%d\" ", rank->ballsCashed) < 0)
	    goto writefailed;

	if (rank->ballsSaved > 0
	    && fprintf(file, "ballssaved=\"%d\" ", rank->ballsSaved) < 0)
	    goto writefailed;

	if (rank->ballsWon > 0
	    && fprintf(file, "ballswon=\"%d\" ", rank->ballsWon) < 0)
	    goto writefailed;

	if (rank->ballsLost > 0
	    && fprintf(file, "ballslost=\"%d\" ", rank->ballsLost) < 0)
	    goto writefailed;

	if (rank->bestball > 0
	    && fprintf(file, "bestball=\"%.2f\" ", rank->bestball) < 0)
	    goto writefailed;

	if (fprintf(file, "timestamp=\"%u\" ", (unsigned)rank->timestamp) < 0)
	    goto writefailed;
	
	if (fprintf(file, "/>\n") < 0)
	    goto writefailed;
    }
    
    if (fprintf(file,
		"</Players>\n"
		"</XPilotNGRank>\n") < 0)
	goto writefailed;
    

    if (fclose(file) != 0) {
	error("Close temporary file \"%s\"", tmp_file);
	goto failed;
    }
    file = NULL;

    /* Overwrite old score file. */
    if (rename(tmp_file, xpilotscorefile) < 0) {
	error("Rename \"%s\" to \"%s\"", tmp_file, xpilotscorefile);	
	goto failed;
    }

    remove(tmp_file);

    /*xpprintf("%s Rank file with %d entries written successfully.\n",
      showtime(), rank_entries);*/

    return;

 writefailed:
    error("Write temporary file \"%s\"", tmp_file);

 failed:

    if (file) {
	fclose(file);
	remove(tmp_file);
    }
    warn("Couldn't save ranking data to file \"%s\".", xpilotscorefile);

    return;
}


static void tagstart(void *data, const char *el, const char **attr)
{
    static bool xptag = false;

    UNUSED_PARAM(data);

    if (!strcasecmp(el, "XPilotNGRank")) {
	double version = -1;

	while (*attr) {
	    if (!strcasecmp(*attr, "version"))
		version = atof(*(attr + 1));
	    attr += 2;
	}
	if (version == 0.0)
	    warn("Score file version is 0.0.");
	else if (version > 0.0) {
	    warn("Score file has newer version than this server recognizes.");
	    warn("The score file might use unsupported features.");
	}
	xptag = true;
	return;
    }

    if (!xptag) {
	fatal("This doesn't look like a score file "
	      " (XPilotNGRank must be first tag).");
	return; /* not reached */
    }

    if (!strcasecmp(el, "Player")) {
	rankinfo_t *node;

	if (!playerstag)
	    fatal("Player tag in rank file without Players.");

	node = &scores[num_players++];
	memset(node, 0, sizeof(rankinfo_t));

	while (*attr) {
	    if (!strcasecmp(*attr, "name"))
		strlcpy(node->name, *(attr + 1), sizeof(node->name));
	    if (!strcasecmp(*attr, "user"))
		strlcpy(node->user, *(attr + 1), sizeof(node->user));
	    if (!strcasecmp(*attr, "host"))
		strlcpy(node->host, *(attr + 1), sizeof(node->host));
	    if (!strcasecmp(*attr, "score"))
		node->score = atof(*(attr + 1));
	    if (!strcasecmp(*attr, "kills"))
		node->kills = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "deaths"))
		node->deaths = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "rounds"))
		node->rounds = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "shots"))
		node->shots = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballssaved"))
		node->ballsSaved = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballslost"))
		node->ballsLost = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballswon"))
		node->ballsWon = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballscashed"))
		node->ballsCashed = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "bestball"))
		node->bestball = atof(*(attr + 1));
	    if (!strcasecmp(*attr, "timestamp"))
		node->timestamp = atoi(*(attr + 1));

	    attr += 2;
	}

	return;
    }

    if (!strcasecmp(el, "Players")) {
	playerstag = true;
	return;
    }

    warn("Unknown tag in score file: \"%s\"", el);
    return;
}


static void tagend(void *data, const char *el)
{
    UNUSED_PARAM(data);

    if (!strcasecmp(el, "Players"))
	playerstag = false;

    return;
}




static bool Rank_parse_scorefile(FILE *file)
{
    char buff[8192];
    int len, fd;
    XML_Parser p = XML_ParserCreate(NULL);

    fd = fileno(file);
    if (fd == -1)
	return false;

    if (!p) {
	warn("Creating Expat instance for ranking file parsing failed.\n");
	return false;
    }
    XML_SetElementHandler(p, tagstart, tagend);
    do {
	len = read(fd, buff, 8192);
	if (len < 0) {
	    error("Error reading rank scorefile!");
	    return false;
	}
	if (!XML_Parse(p, buff, len, !len)) {
	    warn("Parse error reading rank scorefile at line %d:\n%s\n",
		  XML_GetCurrentLineNumber(p),
		  XML_ErrorString(XML_GetErrorCode(p)));
	    return false;
	}
    } while (len);
    return true;
}

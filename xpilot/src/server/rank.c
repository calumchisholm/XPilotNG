/* 
 * XPilot NG, a multiplayer space war game.
 *
 * Copyright (C) 1999-2004 by
 *
 *      Marcus Sundberg      <mackan@stacken.kth.se>
 *      Kristian Söderblom   <kps@users.sourceforge.net>
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

/* MAX_SCORES = how many players we remember */
#define MAX_SCORES 300

static bool Rank_parse_rankfile(FILE *file);

/* Score data */
static ranknode_t ranknodes[MAX_SCORES];

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
    const rank_t *r1, *r2;

    r1 = (const rank_t *)p1;
    r2 = (const rank_t *)p2;

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

static char *rank_showtime(const time_t t)
{
    struct tm *tmp;
    static char month_names[13][4] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	"Bug"
    };
    static char buf[80];
    time_t t2 = t;

    tmp = localtime(&t2);
    snprintf(buf, sizeof(buf), "%02d\xA0%s\xA0%02d:%02d:%02d",
	     tmp->tm_mday, month_names[tmp->tm_mon],
	     tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
    return buf;
}

/*
 * Encode 'str' for XML or HTML.
 */
static char *encode(const char *str)
{
    static char result[MAX_CHARS];
    char c;

    result[0] = '\0';
    while ((c = *str++) != '\0') {
	if (c == '<')
	    strlcat(result, "&lt;", sizeof(result));
	else if (c == '>')
	    strlcat(result, "&gt;", sizeof(result));
	else if (c == '&')
	    strlcat(result, "&amp;", sizeof(result));
	else if (c == '\'')
	    strlcat(result, "&apos;", sizeof(result));
	else if (c == '"')
	    strlcat(result, "&quot;", sizeof(result));
	else {
	    char tmp[2];

	    sprintf(tmp, "%c", c);
	    strlcat(result, tmp, sizeof(result));
	}
    }

    return result;
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
	ranknode_t *rank = &ranknodes[k];
	double attenuation, kills, sc, kd, kr, hf;

	if (strlen(rank->name) == 0)
	    continue;

	/* The attenuation affects players with less than 300 rounds. */
	attenuation = (rank->rounds < 300) ?
	    ((double) rank->rounds / 300.0) : 1.0;

	kills = rank->kills;
	sc = (double) rank->score * attenuation;
	kd = ((rank->deaths != 0) ?
	      (kills / (double) rank->deaths) :
	      (kills)) * attenuation;
	kr = ((rank->rounds != 0) ?
	      (kills / (double) rank->rounds) :
	      (kills)) * attenuation;
	hf = ((rank->ballsLost != 0) ?
	      ((double) rank->ballsCashed /
	       (double) rank->ballsLost) :
	      (double) rank->ballsCashed) * attenuation;

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
	int i;

	rank_entries = 0;
	for (i = 0; i < MAX_SCORES; i++) {
	    ranknode_t *rank = &ranknodes[i];
	    double sc, kd, kr, hf, rsc, rkd, rkr, rhf;

	    rank_base[i].ind = i;
	    if (strlen(rank->name) == 0) {
		rank_base[i].ratio = -1;
		continue;
	    }
	    rank_entries++;

	    sc = sc_table[i];
	    kd = kd_table[i];
	    kr = kr_table[i];
	    hf = hf_table[i];

	    rsc = (sc - lowSC) * factorSC;
	    rkd = (kd - lowKD) * factorKD;
	    rkr = (kr - lowKR) * factorKR;
	    rhf = (hf - lowHF) * factorHF;

	    rank_base[i].ratio = 0.20 * rsc + 0.30 * rkd + 0.30 * rkr + 0.20 * rhf;

	    /* KHS: maximum survived time serves as factor */
	    if(options.survivalScore != 0.0){
		rank_base[i].ratio=rank->max_survival_time;
	    }
	} 
	  

	/* And finally we sort the ranks, wheee! */
	qsort(rank_base, MAX_SCORES, sizeof(rank_t), rank_cmp);
    }
}

static const char *Rank_get_logout_message(ranknode_t *rank)
{
    static char msg[MSG_LEN];
    player_t *pl;

    assert(strlen(rank->name) > 0);
    pl = Get_player_by_name(rank->name, NULL, NULL);
    if (pl) {
	if (Player_is_paused(pl))
	    snprintf(msg, sizeof(msg), "paused");
	else
	    snprintf(msg, sizeof(msg), "playing");
    }
    else
	snprintf(msg, sizeof(msg), "%s", rank_showtime(rank->timestamp));

    return msg;
}

#define TABLEHEAD \
"<table><tr><td></td>" /* First column is the position */ \
"<td align=left><h1><u><b>Player</b></u></h1></td>" \
"<td align=right><h1><u><b>Score</b></u></h1></td>" \
"<td align=right><h1><u><b>Kills</b></u></h1></td>" \
"<td align=right><h1><u><b>Deaths</b></u></h1></td>" \
"<td align=right><h1><u><b>Rounds</b></u></h1></td>" \
"<td align=right><h1><u><b>Shots</b></u></h1></td>" \
"<td align=center><h1><u><b>Deadliest</b></u></h1></td>" \
"<td align=center><h1><u><b>Balls</b></u></h1></td>" \
"<td align=right><h1><u><b>Ratio</b></u></h1></td>" \
"<td align=right><h1><u><b>User</b></u></h1></td>" \
"<td align=left><h1><u><b>Host</b></u></h1></td>" \
"<td align=center><h1><u><b>Logout</b></u></h1></td>" \
"</tr>\n"

/* Sort the ranks and save them to the webpage. */
void Rank_write_webpage(void)
{
    static const char header[] =
	"<html><head><title>" PACKAGE_NAME " @ %s</title>\n"
	"</head><body>\n"
	"<h1>" PACKAGE_NAME " @ %s</h1>" /* <-- server name at %s and %s */
	"<a href=\"previous_ranks.html\">Previous rankings</a> "
	"<a href=\"rank_explanation.html\">How does the ranking work?</a>"
	"<hr>\n" TABLEHEAD;

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

    char *filename;
    FILE *file;
    int i;

    SortRankings();

    filename = options.rankWebpageFileName;
    if (!filename)
	return;

    file = fopen(filename, "w");
    if (!file) {
	error("Couldn't open ranking file \"%s\" for writing", filename);
	return;
    }

    fprintf(file, header, Server.host, Server.host);

    for (i = 0; i < MAX_SCORES; i++) {
	ranknode_t *rank = &ranknodes[rank_base[i].ind];

	if (strlen(rank->name) == 0)
	    continue;

	fprintf(file,
		"<tr><td align=left><tt>%d</tt>"
		"<td align=left><b>%s</b>",
		i + 1,
		encode(rank->name));

	fprintf(file,
		"<td align=right>%.1f"
		"<td align=right>%u"
		"<td align=right>%u"
		"<td align=right>%u"
		"<td align=right>%u"
		"<td align=right>%u"
		"<td align=center>%u/%u/%u/%u/%.2f"
		"<td align=right>%.2f"
		"<td align=right>%s",
		rank->score,
		rank->kills, rank->deaths,
		rank->rounds, rank->shots,
		rank->deadliest,
		rank->ballsCashed, rank->ballsSaved,
		rank->ballsWon, rank->ballsLost,
		rank->bestball,
		rank_base[i].ratio,
		encode(rank->user));

	fprintf(file,
		"<td align=left>%s"
		"<td align=center>%s\n"
		"</tr>\n",
		encode(rank->host),
		Rank_get_logout_message(rank));
    }
    fprintf(file, footer, rank_showtime(time(NULL)));
    fclose(file);
}


bool Rank_get_stats(const char *name, char *buf, size_t size)
{
    ranknode_t *r = Rank_get_by_name(name);

    if (r == NULL)
	return false;

    snprintf(buf, size,
	     "%-15s  SC: %7.1f  K/D: %5d/%5d  R: %4d  SH: %6d  Dl: %d "
	     "B: %d/%d/%d/%d/%.2f TM: %.2f",
	     r->name, r->score, r->kills, r->deaths, r->rounds, r->shots, r->deadliest,
	     r->ballsCashed, r->ballsSaved, r->ballsWon, r->ballsLost,
	     r->bestball,r->max_survival_time);

    return true;
}


/* Send a line with the ranks of the current players to the game. */
void Rank_show_ranks(void)
{
    char msg[MSG_LEN];
    char tmpbuf[MSG_LEN];
    int i, num = 0, numranks = 0;

    snprintf(msg, sizeof(msg), "Ranks: ");

    for (i = 0; i < MAX_SCORES; i++) {
	ranknode_t *rank = &ranknodes[rank_base[i].ind];

	if (strlen(rank->name) > 0)
	    numranks++;

	if (rank->pl != NULL) {
	    if (num > 0)
		strlcat(msg, ", ", sizeof(msg));
	    snprintf(tmpbuf, sizeof(tmpbuf), "%s [%d]", rank->name, i + 1);
	    strlcat(msg, tmpbuf, sizeof(msg));
	    num++;
	}
    }

    strlcat(msg, ".", sizeof(msg));
    Set_message(msg);

    /* show a few best ranks */
    snprintf(msg, sizeof(msg), " < Top %d ranks: ",
	     numranks < 3 ? numranks : 3);
    num = 0;
    for (i = 0; i < MAX_SCORES; i++) {
	ranknode_t *rank = &ranknodes[rank_base[i].ind];

	if (strlen(rank->name) == 0)
	    continue;

	if (num > 0)
	    strlcat(msg, ", ", sizeof(msg));
	snprintf(tmpbuf, sizeof(tmpbuf), 
		 (options.survivalScore == 0.0) ? 
		 "%d. %s (%.2f)" : "%d. %s (%.1f)",
		 num + 1, rank->name, rank_base[i].ratio);
	strlcat(msg, tmpbuf, sizeof(msg));
	num++;
	if (num > 2)
	    break;
    }

    strlcat(msg, " >", sizeof(msg));
    Set_message(msg);

    return;
}

static void Init_ranknode(ranknode_t *rank,
			  const char *name, const char *user, const char *host)
{
    memset(rank, 0, sizeof(ranknode_t));
    strlcpy(rank->name, name, sizeof(rank->name));
    strlcpy(rank->user, user, sizeof(rank->user));
    strlcpy(rank->host, host, sizeof(rank->host));
}


ranknode_t *Rank_get_by_name(const char *name)
{
    int i;
    player_t *pl;

    assert(name != NULL);

    for (i = 0; i < MAX_SCORES; i++) {
	ranknode_t *rank = &ranknodes[i];

	if (!strcasecmp(name, rank->name))
	    return rank;
    }

    /*
     * If name doesn't equal any nick in the ranking list (ignoring case),
     * let's see if it could be an abbreviation of the nick of some player
     * who is currently playing.
     */
    pl = Get_player_by_name(name, NULL, NULL);
    if (pl && pl->rank)
	return pl->rank;

    return NULL;
}

/* Read scores from disk, and zero-initialize the ones that are not used.
   Call this on startup. */
void Rank_init_saved_scores(void)
{
    int i;
    FILE *file;

    for (i = 0; i < MAX_SCORES; i++) {
	ranknode_t *rank = &ranknodes[i];

	memset(rank, 0, sizeof(ranknode_t));
    }

    if (getenv("XPILOTSCOREFILE")) {
	warn("Environment variable XPILOTSCOREFILE is obsolete.");
	warn("Use server option rankFileName instead.");
    }

    if (getenv("XPILOTRANKINGPAGE")) {
	warn("Environment variable XPILOTRANKINGPAGE is obsolete.");
	warn("Use server option rankWebpageFileName instead.");
    }

    if (getenv("XPILOTNOJSRANKINGPAGE")) {
	warn("Environment variable XPILOTNOJSRANKINGPAGE is obsolete.");
	warn("Use server option rankWebpageFileName instead.");
    }

    if (!options.rankFileName)
	return;

    file = fopen(options.rankFileName, "r");
    if (!file) {
	if (errno != ENOENT)
	    error("Couldn't open rank file \"%s\"", options.rankFileName);
	return;
    }

    Rank_parse_rankfile(file);

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
    ranknode_t *rank, *unused = NULL;
    int i;

    updateScores = true;

    for (i = 0; i < MAX_SCORES; i++) {
	rank = &ranknodes[i];
	if (!strcasecmp(pl->name, rank->name)) {
	    if (rank->pl == NULL) {
		/* Ok, found it. */
		rank->pl = pl;
		Player_set_score(pl,rank->score);
		pl->rank = rank;
	    } else {
		/* That ranknode is already in use by another player! */
		Player_set_score(pl,0);
		pl->rank = NULL;
	    }
	    return;
	}
    }

    /* find unused rank node */
    for (i = 0; i < MAX_SCORES; i++) {
	rank = &ranknodes[i];

	if (strlen(rank->name) == 0) {
	    unused = rank;
	    /*warn("found unused node %d", i);*/
	    break;
	}
    }

    /*
     * If all entries are in use, use the least-recently-used node
     * of the bottom half of the list.
     */
    if (!unused) {
	for (i = MAX_SCORES / 2; i < MAX_SCORES; i++) {
	    rank = &ranknodes[rank_base[i].ind];

	    /*warn("i is %d, index is %d, timestamp is %u",
	      i, rank_base[i].ind, rank->timestamp); */

	    if (!unused || rank->timestamp < unused->timestamp)
		unused = rank;
	}
    }

    rank = unused;
    /*warn("timestamp of lru node = %u", rank->timestamp);*/

    Init_ranknode(rank, pl->name, pl->username, pl->hostname);
    rank->pl = pl;
    rank->timestamp = time(NULL);
    Player_set_score(pl,0);
    pl->rank = rank;
}

/* A player has quit, save his info and mark him as not playing. */
void Rank_save_score(player_t * pl)
{
    ranknode_t *rank = pl->rank;

    rank->score =  Get_Score(pl);
    rank->pl = NULL;
    rank->timestamp = time(NULL);
}

/* Save the scores to disk (not the webpage). */
void Rank_write_rankfile(void)
{
    FILE *file = NULL;
    char tmp_file[PATH_MAX];
    int i;

    if (!options.rankFileName)
	return;

    snprintf(tmp_file, sizeof(tmp_file), "%s-new", options.rankFileName);

    file = fopen(tmp_file, "w");
    if (file == NULL) {
	error("Open temporary file \"%s\"", tmp_file);
	goto failed;
    }

    if (fprintf(file,
		"<?xml version=\"1.0\"?>\n"
		"<XPilotNGRank version=\"1.0\">\n"
		"<Players>\n") < 0)
	goto writefailed;
    

    for (i = 0; i < rank_entries; i++) {
	ranknode_t *rank = &ranknodes[rank_base[i].ind];

	if (strlen(rank->name) == 0)
	    continue;

	if (fprintf(file, "<Player "
		    "name=\"%s\" ", encode(rank->name)) < 0)
	    goto writefailed;

	if (fprintf(file,
		    "user=\"%s\" ", encode(rank->user)) < 0)
	    goto writefailed;

	if (fprintf(file,
		    "host=\"%s\" ", encode(rank->host)) < 0)
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

	if (rank->deadliest > 0
	    && fprintf(file, "deadliest=\"%d\" ", rank->deadliest) < 0)
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

        if (rank->max_survival_time > 0
            && fprintf(file, "max_survival_time=\"%.2f\" ", 
            rank->max_survival_time) < 0)
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

    /* Overwrite old rank file. */
    if (rename(tmp_file, options.rankFileName) < 0) {
	error("Rename \"%s\" to \"%s\"", tmp_file, options.rankFileName);
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
    warn("Couldn't save ranking data to file \"%s\".", options.rankFileName);

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
	if (version > 1.0) {
	    warn("Rank file has newer version than this server recognizes.");
	    warn("The file might use unsupported features.");
	}
	xptag = true;
	return;
    }

    if (!xptag) {
	fatal("This doesn't look like a rank file "
	      " (XPilotNGRank must be first tag).");
	return; /* not reached */
    }

    if (!strcasecmp(el, "Player")) {
	ranknode_t *rank;

	if (!playerstag)
	    fatal("Player tag in rank file without Players.");

	rank = &ranknodes[num_players++];
	memset(rank, 0, sizeof(ranknode_t));

	while (*attr) {
	    if (!strcasecmp(*attr, "name"))
		strlcpy(rank->name, *(attr + 1), sizeof(rank->name));
	    if (!strcasecmp(*attr, "user"))
		strlcpy(rank->user, *(attr + 1), sizeof(rank->user));
	    if (!strcasecmp(*attr, "host"))
		strlcpy(rank->host, *(attr + 1), sizeof(rank->host));
	    if (!strcasecmp(*attr, "score"))
		rank->score = atof(*(attr + 1));
	    if (!strcasecmp(*attr, "kills"))
		rank->kills = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "deaths"))
		rank->deaths = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "rounds"))
		rank->rounds = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "shots"))
		rank->shots = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "deadliest"))
		rank->deadliest = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballssaved"))
		rank->ballsSaved = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballslost"))
		rank->ballsLost = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballswon"))
		rank->ballsWon = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "ballscashed"))
		rank->ballsCashed = atoi(*(attr + 1));
	    if (!strcasecmp(*attr, "bestball"))
		rank->bestball = atof(*(attr + 1));
	    if (!strcasecmp(*attr, "max_survival_time"))
	        rank->max_survival_time = atof(*(attr + 1));
	    if (!strcasecmp(*attr, "timestamp"))
		rank->timestamp = atoi(*(attr + 1));

	    attr += 2;
	}

	return;
    }

    if (!strcasecmp(el, "Players")) {
	playerstag = true;
	return;
    }

    warn("Unknown tag in rank file: \"%s\"", el);
    return;
}

static void tagend(void *data, const char *el)
{
    UNUSED_PARAM(data);

    if (!strcasecmp(el, "Players"))
	playerstag = false;

    return;
}

static bool Rank_parse_rankfile(FILE *file)
{
    char buf[8192];
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
	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
	    error("Error reading rankfile!");
	    return false;
	}
	if (!XML_Parse(p, buf, len, !len)) {
	    warn("Parse error reading rankfile at line %d:\n%s\n",
		  XML_GetCurrentLineNumber(p),
		  XML_ErrorString(XML_GetErrorCode(p)));
	    return false;
	}
    } while (len);
    return true;
}

#ifdef	_WINDOWS
#include "NT/winServer.h"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#else
#include <unistd.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define SERVER
#include "version.h"
#include "config.h"
#include "types.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "netserver.h"
#include "error.h"
#include "object.h"
#include "checknames.h"

/* MAX_SCORES = how many players we remember */
#define MAX_SCORES 400

static const char XPILOTSCOREFILE[] = "XPILOTSCOREFILE";
static const char XPILOTRANKINGPAGE[] = "XPILOTRANKINGPAGE";
static const char XPILOTNOJSRANKINGPAGE[] = "XPILOTNOJSRANKINGPAGE";

/* Score data */
static ScoreNode scores[MAX_SCORES];
static ScoreNode dummyScoreNode;

static int    rankedplayer[MAX_SCORES];
static double rankedscore[MAX_SCORES];
static double sc_table[MAX_SCORES];
static double kr_table[MAX_SCORES];
static double kd_table[MAX_SCORES];
static double hf_table[MAX_SCORES];
static int first = 0;

static void swap2(int *i1, int *i2, double *d1, double *d2)
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
    time_t		now;
    struct tm		*tmp;
    static char		month_names[13][4] = {
			    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
			    "Bug"
			};
    static char		buf[80];

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
    double lowSC, highSC;
    double lowKD, highKD;
    double lowKR, highKR;
    double lowHF, highHF;
    bool   foundFirst = false;
    int i;

    /* Ok, there are two loops: the first one calculates the scores and
       records lowest and highest scores. The second loop combines the
       scores into a rank. I cannot do it in one loop since I need to
       know low- and highmarks for each score before I can calculate the
       rank. */
    for (i = 0; i < MAX_SCORES; i++) {
	ScoreNode *score = &scores[i];
	double attenuation, kills, sc, kd, kr, hf;
	if (score->nick[0] == '\0') continue;

	/* The attenuation affects players with less than 300 rounds. */
	attenuation = (score->rounds < 300) ?
	    ((double)score->rounds / 300.0) : 1.0;

	kills = score->kills;
	sc = (double)score->score * attenuation;
	kd = ( (score->deaths != 0) ?
			    (kills / (double)score->deaths) :
			    (kills) ) * attenuation;
	kr = ( (score->rounds != 0) ?
			    (kills / (double)score->rounds) :
			    (kills) ) * attenuation;
	hf = ( (score->ballsLost != 0) ?
			    ( (double)score->ballsCashed /
			      (double)score->ballsLost ) :
			    (double)score->ballsCashed ) * attenuation;

	sc_table[i] = sc;
	kd_table[i] = kd;
	kr_table[i] = kr;
	hf_table[i] = hf;

	if ( !foundFirst ) {
	    lowSC = highSC = sc;
	    lowKD = highKD = kd;
	    lowKR = highKR = kr;
	    lowHF = highHF = hf;
	    foundFirst = true;
	} else {
	    if ( sc > highSC )
		highSC = sc;
	    else if ( sc < lowSC )
		lowSC = sc;

	    if ( kd > highKD )
		highKD = kd;
	    else if ( kd < lowKD )
		lowKD = kd;

	    if ( kr > highKR )
		highKR = kr;
	    else if ( kr < lowKR )
		lowKR = kr;

	    if ( hf > highHF )
		highHF = hf;
	    else if ( hf < lowHF )
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

	for (i = 0; i < MAX_SCORES; i++) {
		ScoreNode *score = &scores[i];
		double sc, kd, kr, hf, rsc, rkd, rkr, rhf, rank;
		rankedplayer[i] = i;
		if (score->nick[0] == '\0') {
			rankedscore[i] = -1;
			continue;
		}

		sc = sc_table[i];
		kd = kd_table[i];
		kr = kr_table[i];
		hf = hf_table[i];

		rsc = (sc - lowSC) * factorSC;
		rkd = (kd - lowKD) * factorKD;
		rkr = (kr - lowKR) * factorKR;
		rhf = (hf - lowHF) * factorHF;

		rank = 0.20*rsc + 0.30*rkd + 0.30*rkr + 0.20*rhf;
		rankedscore[i] = rank;
	}

	/* And finally we sort the ranks, using some lame N^2 sort.. wheee! */
	for (i = 0; i < MAX_SCORES; i++ ) {
	    int j;
	    for (j = i+1; j < MAX_SCORES; j++) {
		if ( rankedscore[i] < rankedscore[j] )
		    swap2(&rankedplayer[i], &rankedplayer[j],
			  &rankedscore[i], &rankedscore[j]);
	    }
	}
    }
}

/* Sort the ranks and save them to the webpage. */
void Rank_web_scores(void)
{
    static const char HEADER[] =
	"<html><head><title>XPilot ranking</title>\n"

	/* In order to save space/bandwidth, the table is saved as one */
	/* giant javascript file, instead of writing all the <TR>, <TD>, etc */
	"<SCRIPT language=\"Javascript\">\n<!-- Hide script\n"
	"function g(nick, score, kills, deaths, rounds, shots, ballsCashed, "
	"           ballsSaved, ballsWon, ballsLost, ratio, user, log) {\n"
	"document.write('<tr><td align=left><tt>', i, '</tt></td>');\n"
	"document.write('<td align=left><b>', nick, '</b></td>');\n"
	"document.write('<td align=right>', score, '</td>');\n"
	"document.write('<td align=right>', kills, '</td>');\n"
	"document.write('<td align=right>', deaths, '</td>');\n"
	"document.write('<td align=right>', rounds, '</td>');\n"
	"document.write('<td align=right>', shots, '</td>');\n"
	"document.write('<td align=center>', ballsCashed);\n"
	"document.write('/', ballsSaved, '/', ballsWon, '/', ballsLost);\n"
	"document.write('</td>');\n"
	"document.write('<td align=right>', ratio, '</td>');\n"
	"document.write('<td align=center>', user, '</td>');\n"
	"document.write('<td align=center>', log, '</td>');\n"
	"document.write('</tr>\\n');\n"
	"i = i + 1\n"
	"}\n// Hide script --></SCRIPT>\n"

	"</head><body>\n"

	/* Head of page */
	"<h1>XPilot ranking</h1>"

/*	"<a href=\"previous_ranks.html\">Previous rankings</a> "
*/	"<a href=\"rank_explanation.html\">How does the ranking "
	"work?</a><hr>\n"

	"<noscript>"
        "<blink><h1>YOU MUST HAVE JAVASCRIPT FOR THIS PAGE</h1></blink>"
	"Please go <A href=\"index_nojs.html\">here</A> for the non-js page"
	"</noscript>\n"

	"<table><tr><td></td>" /* First column is the position 1..400 */
	"<td align=left><h1><u><b>Player</b></u></h1></td>"
	"<td align=right><h1><u><b>Score</b></u></h1></td>"
	"<td align=right><h1><u><b>Kills</b></u></h1></td>"
	"<td align=right><h1><u><b>Deaths</b></u></h1></td>"
	"<td align=right><h1><u><b>Rounds</b></u></h1></td>"
	"<td align=right><h1><u><b>Shots</b></u></h1></td>"
	"<td align=center><h1><u><b>Balls</b></u></h1></td>"
	"<td align=right><h1><u><b>Ratio</b></u></h1></td>"
	"<td align=center><h1><u><b>User @ Host</b></u></h1></td>"
	"<td align=center><h1><u><b>Logout</b></u></h1></td>"
	"</tr>\n"

	"<SCRIPT language=\"Javascript\">\n"
	"var i = 1\n"
	;

    static const char HEADERNOJS[] =
	"<html><head><title>Xpilot ranking</title>\n"
	"</head><body>\n"

	/* Head of page */
	"<h1>XPilot ranking</h1>"

/*	"<a href=\"previous_ranks.html\">Previous rankings</a> "
*/	"<a href=\"rank_explanation.html\">How does the ranking "
	"work?</a><hr>\n"

	"<table><tr><td></td>" /* First column is the position 1..400 */
	"<td align=left><h1><u><b>Player</b></u></h1></td>"
	"<td align=right><h1><u><b>Score</b></u></h1></td>"
	"<td align=right><h1><u><b>Kills</b></u></h1></td>"
	"<td align=right><h1><u><b>Deaths</b></u></h1></td>"
	"<td align=right><h1><u><b>Rounds</b></u></h1></td>"
	"<td align=right><h1><u><b>Shots</b></u></h1></td>"
	"<td align=center><h1><u><b>Balls</b></u></h1></td>"
	"<td align=right><h1><u><b>Ratio</b></u></h1></td>"
	"<td align=center><h1><u><b>User @ Host</b></u></h1></td>"
	"<td align=center><h1><u><b>Logout</b></u></h1></td>"
	"</tr>\n"
	;

    static const char FOOTER[] =
	"</table>"
	"<i>Explanation for ballstats</i>:<br>"
	"The numbers are c/s/w/l, where<br>"
	"c = The number of enemy balls you have cashed.<br>"
	"s = The number of your own balls you have returned.<br>"
	"w = The number of enemy balls your team has cashed.<br>"
	"l = The number of your own balls you have lost.<br>"
	"<hr>%s<BR>\n\n" /* <-- Insert time here. */

	"</body></html>"
	;

    SortRankings();

    if (getenv(XPILOTRANKINGPAGE) != NULL) {
	FILE * const file = fopen(getenv(XPILOTRANKINGPAGE), "w");
	if (file != NULL /* && fseek(file, 2000, SEEK_SET ) == 0*/) {
	    int i;
	    fprintf(file, "%s", HEADER);
	    for (i = 0; i < MAX_SCORES; i++) {
		const int j = rankedplayer[i];
		const ScoreNode *score = &scores[j];
		if ( score->nick[0] != '\0' ) {
		    fprintf(file, "g(\"%s\", %d, %u, %u, %u, %lu, "
			    "%u, %u, %u, %u, %.1f, \"%s@%s\", '%s');\n",
			    score->nick,
			    score->score,
			    score->kills,
			    score->deaths,
			    score->rounds,
			    score->firedShots,
			    score->ballsCashed, score->ballsSaved,
			    score->ballsWon, score->ballsLost,
			    rankedscore[i],
			    score->real, score->host,
			    score->logout);
		}
	    }
	    fprintf(file, "</script>");
	    fprintf(file, FOOTER, rank_showtime());
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }
    if (getenv(XPILOTNOJSRANKINGPAGE) != NULL) {
	FILE * const file = fopen(getenv(XPILOTNOJSRANKINGPAGE), "w");
	if (file != NULL && fseek(file, 2000, SEEK_SET) == 0) {
	    int i;
	    fprintf(file, "%s", HEADERNOJS);
	    for (i = 0; i < MAX_SCORES; i++) {
		const int j = rankedplayer[i];
		const ScoreNode *score = &scores[j];
		if ( score->nick[0] != '\0' ) {
		    fprintf(file,
			    "<tr><td align=left><tt>%d</tt>"
			    "<td align=left><b>%s</b>"
			    "<td align=right>%d"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%lu"
			    "<td align=center>%u/%u/%u/%u"
			    "<td align=right>%.1f"
			    "<td align=center>%s@%s"
			    "<td align=center>%s\n"
			    "</tr>\n",
			    i+1,
			    score->nick,
			    score->score,
			    score->kills,
			    score->deaths,
			    score->rounds,
			    score->firedShots,
			    score->ballsCashed, score->ballsSaved,
			    score->ballsWon, score->ballsLost,
			    rankedscore[i],
			    score->real, score->host,
			    score->logout);
		}
	    }
	    fprintf(file, FOOTER, rank_showtime());
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }
}

/* Send a line with the ranks of the current players to the game. */
void Rank_show_standings(void)
{
    char buf[1000] = "";
    int i;

    for (i = 0; i < MAX_SCORES; i++) {
	ScoreNode *score = &scores[rankedplayer[i]];
	if ( score->pl != 0 ) {
	    char msg[MSG_LEN];
	    sprintf(msg, "%s [%d], ", score->nick, i+1);
	    strcat(buf, msg);
	}
    }

    Set_message(buf);
    return;
}

static void Init_scorenode(ScoreNode *node,
	       const char  nick[],
	       const char  real[],
	       const char  host[])
{
    strcpy(node->nick, nick);
    strcpy(node->real, real);
    strcpy(node->host, host);
    strcpy(node->logout, "");
    node->score = 0;
    node->kills = 0;
    node->deaths = 0;
    node->rounds = 0;
    node->firedShots = 0;
    node->ballsSaved = 0;
    node->ballsLost = 0;
    node->ballsCashed = 0;
    node->ballsWon = 0;
    node->pl = 0;
}

/* Read scores from disk, and zero-initialize the ones that are not used.
   Call this on startup. */
void Rank_init_saved_scores(void)
{
    int i = 0;

    if ( getenv(XPILOTSCOREFILE) != NULL ) {
	FILE *file = fopen(getenv(XPILOTSCOREFILE), "r");
	if ( file != NULL ) {

	    const int actual = fread(scores, sizeof(ScoreNode),
				     MAX_SCORES, file);
	    if ( actual != MAX_SCORES )
		error("Error when reading score file!\n");

	    i += actual;

	    fclose(file);
	}
    }

    while (i < MAX_SCORES) {
	Init_scorenode(&scores[i], "", "", "");
	scores[i].timestamp = 0;
	i++;
    }
}

/* A player has logged in. Find his info or create new info by kicking
 * the player who hasn't played for the longest time. */
void Rank_get_saved_score(player *pl)
{
    ScoreNode *score;
    int oldest = 0;
    int i;
    updateScores = true;

    if (pl->name[strlen(pl->name) - 1] == PROT_EXT)
	return;

    for (i = 0; i < MAX_SCORES; i++) {
	score = &scores[i];
	if ( strcasecmp(pl->name, score->nick) == 0 )
	    if ( score->pl == 0 ) {
		/* Ok, found it. */
		score->pl = pl;
		strcpy(score->logout, "playing");
		pl->score = score->score;
		pl->scorenode = score;
		return;
	    } else {
		/* That scorenode is already in use by another player! */
		pl->score = 0;
		pl->scorenode = &dummyScoreNode;
		return;
	    }
	else if ( score->timestamp < scores[oldest].timestamp )
	    oldest = i;
    }

    /* Didn't find it, use the least-recently-used node. */
    score = &scores[oldest];

    Init_scorenode(score, pl->name, pl->realname, pl->hostname);
    strcpy(score->logout, "playing");
    score->pl = pl;
    score->timestamp = time(0);
    pl->score = 0;
    pl->scorenode = score;
}

/* A player has quit, save his info and mark him as not playing. */
void Rank_save_score(const player *pl)
{
    ScoreNode *score = pl->scorenode;

    if (!score)
	return;

    score->score = pl->score;
    strcpy(score->logout, rank_showtime());
    score->pl = 0;
    score->timestamp = time(0);
}

/* Save the scores to disk (not the webpage). */
void Rank_save_data(void)
{
    FILE * file = NULL;

    if ( getenv(XPILOTSCOREFILE) != NULL &&
	 (file = fopen(getenv(XPILOTSCOREFILE), "w")) != NULL ) {

	const int actual = fwrite(scores, sizeof(ScoreNode),
				  MAX_SCORES, file);
	if ( actual != MAX_SCORES )
	    error("Error when writing score file!\n");

	fclose(file);
    }
}


#define CNODE if (!pl->scorenode) return
void Rank_kill(player *pl)
{
    pl->kills++;
    CNODE;
    pl->scorenode->kills++;
}

void Rank_lost_ball(player *pl)
{
    CNODE;
    pl->scorenode->ballsLost++;
}

void Rank_cashed_ball(player *pl)
{
    CNODE;
    pl->scorenode->ballsCashed++;
}

void Rank_won_ball(player *pl)
{
    CNODE;
    pl->scorenode->ballsWon++;
}

void Rank_saved_ball(player *pl)
{
    CNODE;
    pl->scorenode->ballsSaved++;
}

void Rank_death(player *pl)
{
    pl->deaths++;
    CNODE;
    pl->scorenode->deaths++;
}

void Rank_add_score(player *pl, int points)
{
    pl->score += points;
    CNODE;
    pl->scorenode->score += points;
}

void Rank_fire_shot(player *pl)
{
    CNODE;
    pl->scorenode->firedShots++;
}

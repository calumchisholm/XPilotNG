#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

#include "clientrank.h"		/*message scan hack by Mara */
#include "error.h"

/* message scan hack by Mara*/
#define MAX_SCORES 500
/* end message scan hack by Mara*/


/* message scan hack by Mara*/
/* message scan hack by Mara*/
/* message scan hack by Mara*/
static const char CLIENTSCOREFILE[] = "CLIENTSCOREFILE";
static const char CLIENTRANKINGPAGE[] = "CLIENTRANKINGPAGE";
static const char CLIENTNOJSRANKINGPAGE[] = "CLIENTNOJSRANKINGPAGE";
char *clientRankFile = NULL;	/* */
char *clientRankHTMLFile = NULL;	/* */
char *clientRankHTMLNOJSFile = NULL;	/* */

/*
Defining one/both of CLIENTRANKINGPAGE/CLIENTNOJSRANKINGPAGE while leaving
CLIENTSCOREFILE undefined grants a rank of last/(current maybe aswell) xpilot session
*/
/* Score data */
static ScoreNode scores[MAX_SCORES];
static int recent[10];
static int oldest_cache = 0;
/*static ScoreNode dummyScoreNode;*/

static int timesort[MAX_SCORES];	/*mara client hack */
static double kd[MAX_SCORES];
static int kdsort[MAX_SCORES];



static const int meterHeight = 10;

int Client_Scoring = 0;



#if 0
static void swap(ScoreNode * sn1, ScoreNode * sn2)
{				/*mara client hack */
    ScoreNode *dummy;

    dummy = sn1;
    sn1 = sn2;
    sn2 = dummy;
}				/*mara client hack */
#endif
static void swapd(double *d1, double *d2)
{
    double d;

    d = *d1;
    *d1 = *d2;
    *d2 = d;
}
static void swapi(int *i1, int *i2)
{
    int i;

    i = *i1;
    *i1 = *i2;
    *i2 = i;
}


void Time_Sort(void)
{
    int i;
    for (i = 0; i < MAX_SCORES; i++) {
	int j;
	for (j = i + 1; j < MAX_SCORES; j++) {
	    if (scores[timesort[i]].timestamp <
		scores[timesort[j]].timestamp)
		swapi(&timesort[i], &timesort[j]);
	}
    }
    for (i = 0; i < 10; i++) {
	recent[i] = timesort[i];
    }
    oldest_cache = 5;
}

/* This function checks wether the strings contains certain characters
   that might be hazardous to include on a webpage (ie they screw it up). */
void LegalizeName(char string[])
{
    const int length = strlen(string);
    int i;

    for (i = 0; i < length; i++)
	switch (string[i]) {
	case '<':
	case '>':
	case '\t':
	    string[i] = '?';
	default:
	    break;
	}
}

/* Sort the ranks and save them to the webpage. */
void Rank_score(void)
{
    static const char HEADER[] =
	"<html><head><title>Xpilot Clientrank - Evolved by Mara</title>\n"
	/* In order to save space/bandwidth, the table is saved as one */
	/* giant javascript file, instead of writing all the <TR>, <TD>, etc */
	"<SCRIPT language=\"Javascript\">\n<!-- Hide script\n"
	"function g(nick, kills, deaths, ratio) {\n"
	"document.write('<tr><td align=left><tt>', i, '</tt></td>');\n"
	"document.write('<td align=left><b>', nick, '</b></td>');\n"
	"document.write('<td align=right>', kills, '</td>');\n"
	"document.write('<td align=right>', deaths, '</td>');\n"
	"document.write('</td>');\n"
	"document.write('<td align=right>', ratio, '</td>');\n"
	"document.write('</tr>\\n');\n"
	"i = i + 1\n" "}\n// Hide script --></SCRIPT>\n" "</head><body>\n"
	/* Head of page */
	"<h1>XPilot Clientrank - Evolved by Mara</h1>"
	"<noscript>"
	"<blink><h1>YOU MUST HAVE JAVASCRIPT FOR THIS PAGE</h1></blink>"
	"Please go <A href=\"index_nojs.html\">here</A> for the non-js page"
	"</noscript>\n"
	"<table><tr><td></td>"
	"<td align=left><h1><u><b>Player</b></u></h1></td>"
	"<td align=right><h1><u><b>Kills</b></u></h1></td>"
	"<td align=right><h1><u><b>Deaths</b></u></h1></td>"
	"<td align=right><h1><u><b>Ratio</b></u></h1></td>"
	"</tr>\n" "<SCRIPT language=\"Javascript\">\n" "var i = 1\n";

    static const char HEADERNOJS[] =
	"<html><head><title>XPilot Clientrank - Evolved by Mara</title>\n"
	"</head><body>\n"
	/* Head of page */
	"<h1>XPilot Clientrank</h1>"
	"<table><tr><td></td>"
	"<td align=left><h1><u><b>Player</b></u></h1></td>"
	"<td align=right><h1><u><b>Kills</b></u></h1></td>"
	"<td align=right><h1><u><b>Deaths</b></u></h1></td>"
	"<td align=right><h1><u><b>Ratio</b></u></h1></td>" "</tr>\n";

    static const char FOOTER[] =
	"</table>"
	"<i>Explanation for rank</i>:<br>"
	"The numbers are k/d/r, where<br>"
	"k = The number of times he has shot me<br>"
	"d = The number of time I have shot him<br>"
	"r = the quota between k and d<br>" "</body></html>";


    int i;
    for (i = 0; i < MAX_SCORES; i++) {
	kdsort[i] = i;
	kd[i] =
	    (scores[i].deaths !=
	     0.0) ? ((double) (scores[i].kills) /
		     (double) (scores[i].deaths)) : 0.0;
    }

    for (i = 0; i < MAX_SCORES; i++) {
	int j;
	for (j = i + 1; j < MAX_SCORES; j++) {
	    if (kd[i] < kd[j]) {
		swapi(&kdsort[i], &kdsort[j]);
		swapd(&kd[i], &kd[j]);
	    }
	}
    }
    for (i = 0; i < MAX_SCORES; i++) {
    }

    /*if (getenv(CLIENTRANKINGPAGE) != NULL) { */
    if (clientRankHTMLFile != NULL) {
	/*FILE * const file = fopen(getenv(CLIENTRANKINGPAGE), "w"); */
	FILE *const file = fopen(clientRankHTMLFile, "w");
	if (file != NULL && fseek(file, 2000, SEEK_SET) == 0) {
	    int i;
	    fprintf(file, "%s", HEADER);
	    for (i = 0; i < MAX_SCORES; i++) {
		if (scores[kdsort[i]].nick[0] != '\0') {
		    LegalizeName(scores[kdsort[i]].nick);
		    fprintf(file, "g(\"%s\", %u, %u, %.3f);\n",
			    scores[kdsort[i]].nick,
			    scores[kdsort[i]].kills,
			    scores[kdsort[i]].deaths, kd[i]);
		}
	    }
	    fprintf(file, "</script>");
	    fprintf(file, FOOTER);
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }

    /* if (getenv(CLIENTNOJSRANKINGPAGE) != NULL) {
       FILE * const file = fopen(getenv(CLIENTNOJSRANKINGPAGE), "w"); */
    if (clientRankHTMLNOJSFile != NULL) {
	FILE *const file = fopen(clientRankHTMLNOJSFile, "w");
	if (file != NULL && fseek(file, 2000, SEEK_SET) == 0) {
	    int i;
	    fprintf(file, "%s", HEADERNOJS);
	    for (i = 0; i < MAX_SCORES; i++) {
		if (scores[kdsort[i]].nick[0] != '\0') {
		    LegalizeName(scores[kdsort[i]].nick);
		    fprintf(file,
			    "<tr><td align=left><tt>%d</tt>"
			    "<td align=left><b>%s</b>"
			    "<td align=right>%u"
			    "<td align=right>%u"
			    "<td align=right>%.3f"
			    "</tr>\n",
			    i + 1,
			    scores[kdsort[i]].nick,
			    scores[kdsort[i]].kills,
			    scores[kdsort[i]].deaths, kd[i]);
		}
	    }
	    fprintf(file, FOOTER);
	    fclose(file);
	} else
	    error("Could not open the rank file.");
    }

}

static void Init_scorenode(ScoreNode * node, const char nick[])
{
    strcpy(node->nick, nick);
    node->kills = 0;
    node->deaths = 0;
}

/* Read scores from disk, and zero-initialize the ones that are not used.
   Call this on startup. */
void Init_saved_scores(void)
{
    int i = 0;

    /*if ( getenv(CLIENTSCOREFILE) != NULL ) { */
    if (clientRankFile != NULL) {
	/* FILE *file = fopen(getenv(CLIENTSCOREFILE), "r"); */
	FILE *file = fopen(clientRankFile, "r");
	if (file != NULL) {

	    const int actual = fread(scores, sizeof(ScoreNode),
				     MAX_SCORES, file);
	    if (actual != MAX_SCORES)
		error("Error when reading score file!\n");

	    i += actual;

	    fclose(file);
	}
	Client_Scoring = 1;
    }

    while (i < MAX_SCORES) {
	Init_scorenode(&scores[i], "");
	scores[i].timestamp = 0;
	timesort[i] = i;
	i++;
    }
    if (Client_Scoring == 1) {
	Time_Sort();
    }
}

int Get_saved_score(char *nick)
{
    int oldest = 0;
    int i;

    for (i = 0; i < MAX_SCORES; i++) {
	if (strcmp(nick, scores[i].nick) == 0)
	    return i;
	if (scores[i].timestamp < scores[oldest].timestamp)
	    oldest = i;
    }

    Init_scorenode(&scores[oldest], nick);
    scores[oldest].timestamp = time(0);
    return oldest;
}


int Find_player(char *nick)
{
    int i;
    for (i = 0; i < 10; i++) {
	/*if (scores[recent[i]].timestamp > 0) { */
	if (strcmp(nick, scores[recent[i]].nick) == 0)
	    return i;
    }
    i = Get_saved_score(nick);
    recent[oldest_cache] = i;
    i = oldest_cache;
    oldest_cache = (oldest_cache + 1) / 10;
    return i;
}


void Add_rank_Kill(char *nick)
{
    int i;
    i = Find_player(nick);
    scores[recent[i]].kills = scores[recent[i]].kills + 1;
    scores[recent[i]].timestamp = time(0);
}

void Add_rank_Death(char *nick)
{
    int i;
    i = Find_player(nick);
    scores[recent[i]].deaths = scores[recent[i]].deaths + 1;
    scores[recent[i]].timestamp = time(0);
}

int Get_kills(char *nick)
{
    int i;
    i = Find_player(nick);
    return scores[recent[i]].kills;
}

int Get_deaths(char *nick)
{
    int i;
    i = Find_player(nick);
    return scores[recent[i]].deaths;
}


/* Save the scores to disk (not the webpage). */
void Print_saved_scores(void)
{
    FILE *file = NULL;
    Rank_score();

    /* if ( getenv(CLIENTSCOREFILE) != NULL &&
       (file = fopen(getenv(CLIENTSCOREFILE), "w")) != NULL ) { */
    if (clientRankFile != NULL &&
	(file = fopen(clientRankFile, "w")) != NULL) {

	const int actual = fwrite(scores, sizeof(ScoreNode),
				  MAX_SCORES, file);
	if (actual != MAX_SCORES)
	    error("Error when writing score file!\n");

	fclose(file);
    }
}

/* end message scan hack by Mara*/
/* end message scan hack by Mara*/
/* end message scan hack by Mara*/





/*called from guimap for basewarning */
/*void Paint_baseInfoOnHudRadar(int xi, int yi)*/
/*{
   float x,y,x2,y2,x3,y3;
	float hrscale = hrScale;
	float hrw = (float)hrscale * (float)256;
	float hrh = (float)hrscale * (float)RadarHeight;
	int sz = hrSize;
	float xf = (float) hrw / (float) Setup->width;
   float yf = (float) hrh / (float) Setup->height;
   
   x = X(xi) + SHIP_SZ/2;
   y = Y(yi) - SHIP_SZ*4/3;
   x2 = (ext_view_width / 2) - (sz/2);
   y2 = (ext_view_height / 2) -(sz/2);
   x3 = (x-x2)*xf + x2;
   y3 = (y-y2)*yf + y2;

   if (hrColor1 >= 1)
	   Arc_add(hrColor1, (int)x, (int)y, sz, sz, 0,
			   64 * 360);
   if (hrColor1 >= 1)
	   Arc_add(hrColor1, (int)x2,(int)y2, sz, sz, 0,
			   64 * 360);
   if (hrColor2 >= 1)
	   Arc_add(hrColor2, (int)x3, (int)y3, sz, sz, 0,
			   64 * 360);
		      		   }*//*doesn't work (yet) since client only knows visible bases */

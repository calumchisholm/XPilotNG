#ifndef CLIENTRANK_H
#define CLIENTRANK_H 1

#include <string.h>

extern char clientname[16];	/*assigned in xpilot.c :( */
extern char *clientRankFile;	/* */
extern char *clientRankHTMLFile;	/* */
extern char *clientRankHTMLNOJSFile;	/* */

typedef struct ScoreNode {
    char nick[16];
    int timestamp;
    unsigned short kills;
    unsigned short deaths;
} ScoreNode;

void Init_saved_scores(void);

void Print_saved_scores(void);
void Paint_baseInfoOnMap(int x, int y);

void Add_rank_Kill(char *nick);
void Add_rank_Death(char *nick);
int Get_kills(char *nick);
int Get_deaths(char *nick);

#endif				/* PAINTHUD_H */

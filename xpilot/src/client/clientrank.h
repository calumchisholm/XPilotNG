#ifndef CLIENTRANK_H
#define CLIENTRANK_H 1

extern char clientname[16];	/*assigned in xpilot.c :( */
extern char clientRankFile[PATH_MAX];
extern char clientRankHTMLFile[PATH_MAX];
extern char clientRankHTMLNOJSFile[PATH_MAX];

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

#endif

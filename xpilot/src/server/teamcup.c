/* 
 * XPilot NG, a multiplayer space war game.
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

char teamcup_version[] = VERSION;

static char *teamcup_score_file_name = NULL;
static FILE *teamcup_score_file = NULL;
static int teamcup_match_inited = 0;
static int teamcup_status_fd = -1;
static int teamcup_have_fork = 0;
static int teamcup_child_pid = 0;

#define STATUSBUFS 8
#define STATBUFSIZE 512

struct teamcup_player_info {
    int matchno;
    int id;
    uint8_t team;
    uint8_t namelen;
    uint8_t name[MAX_CHARS];
    int16_t score;
    uint16_t last_kills;
    uint16_t last_deaths;
    uint16_t total_kills;
    uint16_t total_deaths;
};


struct teamcup_match_info {
    uint32_t inet_addr;
    char addr_str[16];
    int matchno;
    int finished;
    int inited;
    int lastround;
    int lastwinner;
    int fd;
    char t2name[MAX_CHARS];
    char t4name[MAX_CHARS];
    uint8_t t2players;
    uint8_t t4players;
    int16_t t2score;
    int16_t t4score;
    struct teamcup_player_info *team2;
    struct teamcup_player_info *team4;
};


#define INIT_MATCH_MAGIC 0xada0
#define END_MATCH_MAGIC 0x6daf
#define ROUND_END_MAGIC 0x0f41
#define INIT_PLAYER_MAGIC 0xfeed
#define PLAYER_STATUS_MAGIC 0xa537

struct teamcup_match_init {
    uint16_t magic;
    uint16_t matchno;
    uint8_t t2players;
    uint8_t t4players;
    uint8_t pad[2];
};


struct teamcup_match_end {
    uint16_t magic;
    uint16_t matchno;
};


struct teamcup_player_init {
    uint16_t magic;
    uint16_t id;
    uint8_t team;
    uint8_t namelen;
    uint8_t pad[2];
    /* Name here */
};


struct teamcup_round_end {
    uint16_t magic;
    uint8_t roundno;
    uint8_t winner;
    uint8_t by;
#define BY_DRAW 0
#define BY_BALL 1
#define BY_ALIVE 2
    uint8_t pad[3];
};


struct teamcup_player_status {
    uint16_t magic;
    uint16_t id;
    int16_t score;
    uint8_t kills;
    uint8_t deaths;
};



static void
teamcup_status_thread(int readfd)
{
    char buf[STATUSBUFS][STATBUFSIZE], *bufptr[STATUSBUFS];
    int len[STATUSBUFS];
    int sockfd;
    int wrbuf, rdbuf, maxfd, i;
    /*int sockret;*/
    sock_t sock;

    for (i = 0; i < 64; i++)
	signal(i, _exit);

    if (sock_open_tcp_connected_non_blocking(&sock, options.teamcupStatServer,
					     options.teamcupStatPort) != SOCK_IS_OK) {
	xpprintf("\n!!! Unable to connect to master server\n\n");
	_exit(1);
    }
    sockfd = sock.fd;

    wrbuf = 0;
    rdbuf = 0;
    for (i = 0; i < STATUSBUFS; i++) {
	bufptr[i] = NULL;
	len[i] = 0;
    }

    /* Read first block of data */
    do {
	len[rdbuf] = read(readfd, buf[rdbuf], STATBUFSIZE);
    } while (len[rdbuf] <= 0);
    bufptr[rdbuf] = buf[rdbuf];
    rdbuf++;

    if (readfd > sockfd) maxfd = readfd + 1;
    else maxfd = sockfd + 1;

    while (1) {
	fd_set rfds, wfds;
	int dowr = 0;

	FD_ZERO(&rfds);
	FD_SET(readfd, &rfds);
	if (bufptr[wrbuf]) {
	    FD_ZERO(&wfds);
	    FD_SET(sockfd, &wfds);
	    dowr = 1;
	}
	do {
	    FD_ZERO(&rfds);
	    FD_SET(readfd, &rfds);
	    if (bufptr[wrbuf]) {
		FD_ZERO(&wfds);
		FD_SET(sockfd, &wfds);
		dowr = 1;
	    }
	} while (select(maxfd, &rfds, (dowr ? &wfds : NULL), NULL, NULL) <= 0);
	if (dowr && FD_ISSET(sockfd, &wfds)) {
	    int ret;

	    ret = write(sockfd, bufptr[wrbuf], (size_t)len[wrbuf]);
	    if (ret < 0) ret = 0;
	    len[wrbuf] -= ret;
	    bufptr[wrbuf] += ret;
	    if (len[wrbuf] <= 0) {
		bufptr[wrbuf] = NULL;
		wrbuf++;
		if (wrbuf >= STATUSBUFS) wrbuf = 0;
	    }
	}
	if (FD_ISSET(readfd, &rfds)) {
	    int ret;

	    if (bufptr[rdbuf] != NULL) {
		char dummybuf[STATBUFSIZE];

		read(readfd, dummybuf, STATBUFSIZE);
		continue;
	    }
	    ret = read(readfd, buf[rdbuf], STATBUFSIZE);
	    if (ret <= 0)
		continue;
	    len[rdbuf] = ret;
	    bufptr[rdbuf] = buf[rdbuf];
	    rdbuf++;
	    if (rdbuf >= STATUSBUFS) rdbuf = 0;
	}
    }
}


void teamcup_open_score_file(void)
{
    if (!options.teamcup)
	return;

    if (teamcup_score_file != NULL) {
	error("teamcup_score_file != NULL");
	End_game();
    }
    if (teamcup_score_file_name != NULL) {
	error("teamcup_score_file_name != NULL");
	End_game();
    }

    /* kps - gcc complains about tempnam */
    teamcup_score_file_name = tempnam(NULL, "teamcup-");
    if (teamcup_score_file_name == NULL) {
	error("tempnam() failed, could not create score file name");
	End_game();
    }
    teamcup_score_file = fopen(teamcup_score_file_name, "w");
    if (teamcup_score_file == NULL) {
	error("fopen() failed, could not create score file");
	End_game();
    }
    xpprintf("score file is \"%s\".\n", teamcup_score_file_name);

    teamcup_log("1) Fill the names of the teams below.\n"
		"2) Fill the team number of total winner only if "
		"this was the second match.\n"
		"3) Send this file to <FILLINTHISADDY> with subject "
		"CUP-RESULT\n"
		"4) Copy this file in a safe place.  "
		"Do not delete it after sending.\n"
		"\nTeam 2 name: \n"
		"Team 4 name: \n"
		"Match: %d\n"
		"Total winner (team number): \n"
		"\nDO NOT CHANGE ANYTHING AFTER THIS LINE\n\n",
		options.teamcupMatchNumber
	);

    if (options.teamcupMatchNumber && !teamcup_have_fork) {
	int mypipes[2];

#ifndef _WINDOWS
	if (pipe(mypipes) != 0) {
#else
	{
#endif
	    xpprintf("Unable to create pipes!\n");
	    End_game();
	}

	teamcup_status_fd = mypipes[1];
#ifndef _WINDOWS
	teamcup_child_pid = fork();
#else
	teamcup_child_pid = -1;
#endif
	switch (teamcup_child_pid) {
	case -1:
	    teamcup_child_pid = 0;
	    xpprintf("Unable to fork!\n");
	    End_game();
	    break;
	case 0:
	    teamcup_status_thread(mypipes[0]);
	    break;
	default:
	    break;
	}
	teamcup_have_fork = 1;
    }
    teamcup_match_inited = 0;
}

void teamcup_close_score_file(void)
{
    char msg[MSG_LEN];

    if (!options.teamcup || teamcup_score_file == NULL)
	return;

    fclose(teamcup_score_file);
    teamcup_score_file = NULL;

    sprintf(msg,"score file \"%s\" closed", teamcup_score_file_name);
    Set_message(msg);
    xpprintf("%s\n", msg);

    free(teamcup_score_file_name);
    teamcup_score_file_name = NULL;
}

void teamcup_game_over(void)
{
    if (teamcup_status_fd != -1) {
	struct teamcup_match_end mend;

	mend.magic = htons(END_MATCH_MAGIC);
	mend.matchno = htons(options.teamcupMatchNumber);
	write(teamcup_status_fd, &mend, sizeof(mend));
    }
    teamcup_close_score_file();
}

void teamcup_log(const char *fmt, ...)
{
    if (options.teamcup && teamcup_score_file) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(teamcup_score_file, fmt, ap);
  	va_end(ap);
    }
}

void teamcup_round_start(void)
{
    if (!options.teamcup)
	return;

    teamcup_log("\nRound %d\n", roundsPlayed + 1);
}

void teamcup_round_end(int winning_team)
{
    int i;
    int j;
    int *list;
    double team_score[MAX_TEAMS];
    int team_players[MAX_TEAMS];
    double best_score = (double)(-100000); /* hack */
    int best;
    player_t *pl;
    double double_max;

    if (!options.teamcup)
	return;

    double_max=(double)(100000); /*not really*/

    list = XMALLOC(int, NumPlayers);
    if (list == NULL) {
	error("Can't allocate memory for list");
	End_game();
    }

    for (i = 0; i < NumPlayers; i++)
	list[i] = i;

    for (i = 0; i < MAX_TEAMS; i++)
	team_score[i] = double_max;

    for (i = 0; i < MAX_TEAMS; i++)
	team_players[i] = 0;

    for (i = 0; i < NumPlayers; i++) {
	best = NumPlayers;
	for (j = 0; j < NumPlayers; j++) {
	    if (list[j] == NumPlayers)
		continue;

	    pl = Player_by_index(j);
	    if (best == NumPlayers || pl->score > best_score) {
		best_score = pl->score;
		best = j;
	    }
	}

	list[best] = NumPlayers;
	pl = Player_by_index(best);
	teamcup_log("%d\t%d\t%2d/%d\t%s\n", pl->team, (int)(pl->score),
		    pl->kills, pl->deaths, pl->name);

	if (team_score[pl->team] == double_max)
	    team_score[pl->team] = 0.0;
	team_score[pl->team] += pl->score;
	team_players[pl->team]++;
    }

    for (i = 0; i < MAX_TEAMS; i++) {
	if (team_score[i] != double_max)
	    teamcup_log("Team %d\t%d\n", i, (int)(team_score[i]));
    }
    if (teamcup_score_file != NULL)
	fflush(teamcup_score_file);

    if (teamcup_status_fd != -1) {
	struct teamcup_round_end rend;

	if (!teamcup_match_inited) {
	    struct teamcup_match_init minit;

	    minit.magic = htons(INIT_MATCH_MAGIC);
	    minit.matchno = htons(options.teamcupMatchNumber);
	    minit.t2players = team_players[2];
	    minit.t4players = team_players[4];
	    write(teamcup_status_fd, &minit, sizeof(minit));

	    for (i = 0; i < NumPlayers; i++) {
		struct teamcup_player_init pinit;

		pl = Player_by_index(i);
		pinit.magic = htons(INIT_PLAYER_MAGIC);
		pinit.id = htons(i);
		pinit.team = pl->team;
		pinit.namelen = strlen(pl->name);
		write(teamcup_status_fd, &pinit, sizeof(pinit));
		write(teamcup_status_fd, &pl->name, pinit.namelen);
	    }

	    teamcup_match_inited = 1;
	}

	for (i = 0; i < NumPlayers; i++) {
	    struct teamcup_player_status pstat;

	    pl = Player_by_index(i);
	    pstat.magic = htons(PLAYER_STATUS_MAGIC);
	    pstat.id = htons(i);
	    /*status server won't know about float scores I think*/
	    pstat.score = htons((short)(pl->score));
	    pstat.kills = pl->kills;
	    pstat.deaths = pl->deaths;
	    write(teamcup_status_fd, &pstat, sizeof(pstat));
	}

	rend.magic = htons(ROUND_END_MAGIC);
	rend.roundno = roundsPlayed + 1;
	rend.winner = winning_team;
	write(teamcup_status_fd, &rend, sizeof(rend));
    }

    free(list);
}

void teamcup_kill_child(void)
{
#ifndef _WINDOWS
    if (teamcup_child_pid != 0)
	kill(teamcup_child_pid, SIGINT);
#endif
}

#if 0
int Get_scorefileHeader(char *buf, int offset, int maxlen, int *size_ptr)
{
    static int		motd_size;
    static char		*motd_buf;
    static long		motd_loops;
    static time_t	motd_mtime;

    if (size_ptr) {
	*size_ptr = 0;
    }
    if (offset < 0 || maxlen < 0) {
	return -1;
    }

    if (!motd_loops
	|| (motd_loops + MAX_MOTD_LOOPS < main_loops
	    && offset == 0)) {

	int			fd, size;
	struct stat		st;

	motd_loops = main_loops;

	if ((fd = open(options.motdFileName, O_RDONLY)) == -1) {
	    motd_size = 0;
	    return -1;
	}
	if (fstat(fd, &st) == -1 || st.st_size == 0) {
	    motd_size = 0;
	    close(fd);
	    return -1;
	}
	size = st.st_size;
	if (size > MAX_MOTD_SIZE)
	    size = MAX_MOTD_SIZE;
	if (size != motd_size) {
	    motd_mtime = 0;
	    motd_size = size;
	    if (motd_size == 0) {
		close(fd);
		return 0;
	    }
	    if (motd_buf)
		free(motd_buf);

	    if ((motd_buf = (char *) malloc(size)) == NULL) {
		close(fd);
		return -1;
	    }
	}
	if (motd_mtime != st.st_mtime) {
	    motd_mtime = st.st_mtime;
	    if ((size = read(fd, motd_buf, motd_size)) <= 0) {
		free(motd_buf);
		motd_buf = 0;
		close(fd);
		motd_size = 0;
		return -1;
	    }
	    motd_size = size;
	}
	close(fd);
    }

    motd_loops = main_loops;

    if (size_ptr)
	*size_ptr = motd_size;

    if (offset + maxlen > motd_size)
	maxlen = motd_size - offset;

    if (maxlen <= 0)
	return 0;

    memcpy(buf, motd_buf + offset, maxlen);
    return maxlen;
}
#endif

/* 
 * XPilot NG, a multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpserver.h"

#define META_VERSION	VERSION "ng"

struct MetaServer {
    char name[64];
    char addr[16];
};
struct MetaServer meta_servers[2] = {
    {
	META_HOST,
	META_IP
    },
    {
	META_HOST_TWO,
	META_IP_TWO
    },
};

void Meta_send(char *mesg, size_t len)
{
    int i;

    if (!options.reportToMetaServer)
	return;

    for (i = 0; i < NELEM(meta_servers); i++) {
	if (sock_send_dest(&contactSocket, meta_servers[i].addr,
			   META_PORT, mesg, (int)len) != (int)len) {
	    sock_get_error(&contactSocket);
	    sock_send_dest(&contactSocket, meta_servers[i].addr,
			   META_PORT, mesg, (int)len);
	}
    }
}

int Meta_from(char *addr, int port)
{
    int i;

    for (i = 0; i < NELEM(meta_servers); i++) {
	if (!strcmp(addr, meta_servers[i].addr))
	    return (port == META_PORT);
    }
    return 0;
}

void Meta_gone(void)
{
    char msg[MSG_LEN];

    if (options.reportToMetaServer) {
	snprintf(msg, sizeof(msg), "server %s\nremove", Server.host);
	Meta_send(msg, strlen(msg) + 1);
    }
}

void Meta_init(void)
{
    int i;
    char *addr;

    if (!options.reportToMetaServer)
	return;

    xpprintf("%s Locating Internet Meta server... ", showtime());
    fflush(stdout);

    for (i = 0; i < NELEM(meta_servers); i++) {
	addr = sock_get_addr_by_name(meta_servers[i].name);
	if (addr)
	    strlcpy(meta_servers[i].addr, addr, sizeof(meta_servers[i].addr));
	if (addr)
	    xpprintf("found %d", i + 1);
	else
	    xpprintf("%d not found", i + 1);
	if (i + 1 == NELEM(meta_servers))
	    xpprintf("\n");
	else
	    xpprintf("... ");
	fflush(stdout);
    }
}

#if 0
static void asciidump(void *p, size_t size)
{
    int i;
    unsigned char *up = p;
    char c;

    for (i = 0; i < size; i++) {
       if (!(i % 64))
           printf("\n%08x ", i);
       c = *(up + i);
       if (isprint(c))
           printf("%c", c);
       else
           printf(".");
    }
    printf("\n\n");
}
#endif

static char meta_update_string[MAX_STR_LEN];

void Meta_update_max_size_tuner(void)
{
    LIMIT(options.metaUpdateMaxSize, 1, (int) sizeof(meta_update_string));
}

void Meta_update(bool change)
{
#define GIVE_META_SERVER_A_HINT	180

    char *string = meta_update_string, freebases[120];
    int i, num_active_players, active_per_team[MAX_TEAMS];
    size_t len, max_size;
    time_t currentTime;
    const char *game_mode;
    static time_t lastMetaSendTime = 0;
    static int queue_length = 0;
    bool first;

    if (!options.reportToMetaServer)
	return;

    currentTime = time(NULL);
    if (!change) {
	if (currentTime - lastMetaSendTime < GIVE_META_SERVER_A_HINT) {
	    if (NumQueuedPlayers == queue_length ||
		currentTime - lastMetaSendTime < 5)
		return;
	}
    }

    Meta_update_max_size_tuner();
    max_size = options.metaUpdateMaxSize;

    lastMetaSendTime = currentTime;
    queue_length = NumQueuedPlayers;

    /* Find out the number of active players. */
    num_active_players = 0;
    memset(active_per_team, 0, sizeof active_per_team);

    for (i = 0; i < NumPlayers; i++) {
	player_t *pl = Player_by_index(i);

	if (!Player_is_human(pl)
	    || Player_is_paused(pl))
	    continue;

	num_active_players++;
	if (BIT(world->rules->mode, TEAM_PLAY))
	    active_per_team[pl->team]++;
    }

    game_mode = Describe_game_status();

    /* calculate number of available homebases per team. */
    freebases[0] = '\0';
    if (BIT(world->rules->mode, TEAM_PLAY)) {
	bool firstteam = true;

	for (i = 0; i < MAX_TEAMS; i++) {
	    team_t *team = Team_by_index(i);

	    if (i == options.robotTeam && options.reserveRobotTeam)
		continue;

	    if (team->NumBases > 0) {
		char str[32];

		snprintf(str, sizeof(str), "%s%d=%d",
			 (firstteam ? "" : ","), i,
			 team->NumBases - active_per_team[i]);
		firstteam = false;
		strlcat(freebases, str, sizeof(freebases));
	    }
	}
    }
    else
	snprintf(freebases, sizeof(freebases), "=%d",
		 Num_bases() - num_active_players - login_in_progress);

    snprintf(string, max_size,
	     "add server %s\n"
	     "add users %d\n"
	     "add version %s\n"
	     "add map %s\n"
	     "add sizeMap %3dx%3d\n"
	     "add author %s\n"
	     "add bases %d\n"
	     "add fps %d\n"
	     "add port %d\n"
	     "add mode %s\n"
	     "add teams %d\n"
	     "add free %s\n"
	     "add timing %d\n"
	     "add stime %ld\n"
	     "add queue %d\n"
	     "add sound %s\n",
	     Server.host, num_active_players,
	     META_VERSION, world->name, world->x, world->y, world->author,
	     Num_bases(), FPS, options.contactPort,
	     game_mode, world->NumTeamBases, freebases,
	     BIT(world->rules->mode, TIMING) ? 1:0,
	     (long)(time(NULL) - serverStartTime),
	     queue_length, options.sound ? "yes" : "no");


    /*
     * 'len' must always hold the exact number of
     * non-zero bytes which are in string[].
     */
    len = strlen(string);
    first = true;

    for (i = 0; i < NumPlayers; i++) {
	player_t *pl = Player_by_index(i);
	char str[4 * MAX_CHARS];
	char tstr[32];

	if (!Player_is_human(pl)
	    || Player_is_paused(pl))
	    continue;

	snprintf(str, sizeof(str),
		 "%s%s=%s@%s",
		 first ? "add players " : ",",
		 pl->name,
		 pl->username,
		 pl->hostname);

	if (BIT(world->rules->mode, TEAM_PLAY)) {
	    snprintf(tstr, sizeof(tstr), "{%d}", pl->team);
	    strlcat(str, tstr, sizeof(str));
	}

	if (len + strlen(str) + 1 > max_size)
	    break;

	strlcat(string, str, max_size);
	len += strlen(str);
	first = false;
    }

#if 0
    /* kps - don't bother to send status, it probably isn't useful */
    if (len + MSG_LEN < max_size) {
	char status[MAX_STR_LEN];

	strlcpy(&string[len], "\nadd status ", max_size - len);
	len += strlen(&string[len]);

	Server_info(status, sizeof(status));

	strlcpy(&string[len], status, max_size - len);
	len += strlen(&string[len]);
    }
#else
    {
	char status[MAX_STR_LEN];

	strlcpy(status,
		"\nadd status Use server text interface to query status.",
		sizeof(status));
	if (len + strlen(status) + 1 <= max_size) {
	    strlcat(string, status, max_size);
	    len += strlen(status);
	}
    }
#endif

#if 0
    warn("Meta update string len is %d (limit is %d)",
	 len, options.metaUpdateMaxSize);

    asciidump(string, len);
#endif

    Meta_send(string, len + 1);
}

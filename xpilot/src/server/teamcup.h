/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#ifndef	TEAMCUP_H
#define	TEAMCUP_H

#define TEAMCUP_MAX_MATCHES  256
#define TEAMCUP_STATUS_PORTNO  3749
#define TEAMCUP_STATUS_PORTSTR "3749"


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

#endif

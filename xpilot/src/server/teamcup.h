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

#ifndef	TEAMCUP_H
#define	TEAMCUP_H

#define TEAMCUP_MAX_MATCHES  256
#define TEAMCUP_STATUS_PORTNO  3749
#define TEAMCUP_STATUS_PORTSTR "3749"

void teamcup_log(const char *fmt, ...);
void teamcup_round_start(void);
void teamcup_round_end(int winning_team);
void teamcup_open_score_file(void);
void teamcup_close_score_file(void);
void teamcup_kill_child(void);
void teamcup_game_over(void);

#endif

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

#ifndef	NETSERVER_H
#define	NETSERVER_H

#ifndef PLAYER_H
/* need player_t */
#include "player.h"
#endif

int Setup_net_server(void);
void Conn_change_nick(connection_t *connp, const char *nick);
void Destroy_connection(connection_t *connp, const char *reason);
int Check_connection(char *real, char *nick, char *dpy, char *addr);
int Setup_connection(char *real, char *nick, char *dpy, int team,
		     char *addr, char *host, unsigned version);
int Input(void);
int Send_reply(connection_t *connp, int replyto, int result);
int Send_self(connection_t *connp, player_t *pl,
	      int lock_id,
	      int lock_dist,
	      int lock_dir,
	      int autopilotlight,
	      int status,
	      char *mods);
int Send_leave(connection_t *connp, int id);
int Send_player(connection_t *connp, int id);
int Send_team(connection_t *connp, int id, int team);
int Send_score(connection_t *connp, int id, double score,
	       int life, int mychar, int alliance);
int Send_score_object(connection_t *connp, double score, clpos_t pos, const char *string);
int Send_timing(connection_t *connp, int id, int check, int round);
int Send_base(connection_t *connp, int id, int num);
int Send_fuel(connection_t *connp, int num, double fuel);
int Send_cannon(connection_t *connp, int num, int dead_ticks);
int Send_destruct(connection_t *connp, int count);
int Send_shutdown(connection_t *connp, int count, int delay);
int Send_thrusttime(connection_t *connp, int count, int max);
int Send_shieldtime(connection_t *connp, int count, int max);
int Send_phasingtime(connection_t *connp, int count, int max);
int Send_debris(connection_t *connp, int type, unsigned char *p, unsigned n);
int Send_wreckage(connection_t *connp, clpos_t pos, int wrtype, int size, int rot);
int Send_asteroid(connection_t *connp, clpos_t pos, int type, int size, int rot);
int Send_fastshot(connection_t *connp, int type, unsigned char *p, unsigned n);
int Send_missile(connection_t *connp, clpos_t pos, int len, int dir);
int Send_ball(connection_t *connp, clpos_t pos, int id, int style);
int Send_mine(connection_t *connp, clpos_t pos, int teammine, int id);
int Send_target(connection_t *connp, int num, int dead_ticks, double damage);
int Send_wormhole(connection_t *connp, clpos_t pos);
int Send_polystyle(connection_t *connp, int polyind, int newstyle);
int Send_audio(connection_t *connp, int type, int vol);
int Send_item(connection_t *connp, clpos_t pos, int type);
int Send_paused(connection_t *connp, clpos_t pos, int count);
int Send_appearing(connection_t *connp, clpos_t pos, int id, int count);
int Send_ecm(connection_t *connp, clpos_t pos, int size);
int Send_ship(connection_t *connp, clpos_t pos, int id, int dir, int shield, int cloak, int eshield, int phased, int deflector);
int Send_refuel(connection_t *connp, clpos_t pos1, clpos_t pos2);
int Send_connector(connection_t *connp, clpos_t pos1, clpos_t pos2, int tractor);
int Send_laser(connection_t *connp, int color, clpos_t pos, int len, int dir);
int Send_radar(connection_t *connp, int x, int y, int size);
int Send_fastradar(connection_t *connp, unsigned char *buf, unsigned n);
int Send_damaged(connection_t *connp, int damaged);
int Send_message(connection_t *connp, const char *msg);
int Send_loseitem(connection_t *connp, int lose_item_index);
int Send_start_of_frame(connection_t *connp);
int Send_end_of_frame(connection_t *connp);
int Send_reliable(connection_t *connp);
int Send_time_left(connection_t *connp, long sec);
int Send_eyes(connection_t *connp, int id);
int Send_trans(connection_t *connp, clpos_t pos1, clpos_t pos2);
void Get_display_parameters(connection_t *connp, int *width, int *height,
			    int *debris_colors, int *spark_rand);
int Get_player_id(connection_t *connp);
const char *Player_get_addr(player_t *pl);
const char *Player_get_dpy(player_t *pl);
int Send_shape(connection_t *connp, int shape);
int Check_max_clients_per_IP(char *host_addr);
#define FEATURE(connp, feature)	((connp)->features & (feature))
#define F_POLY			(1 << 0)
#define F_FLOATSCORE		(1 << 1)
#define F_EXPLICITSELF		(1 << 2)
#define F_ASTEROID		(1 << 3)
#define F_TEMPWORM		(1 << 4)
#define F_FASTRADAR		(1 << 5)
#define F_SEPARATEPHASING	(1 << 6)
#define F_TEAMRADAR		(1 << 7)
#define F_SHOW_APPEARING	(1 << 8)
#define F_SENDTEAM		F_SHOW_APPEARING
#define F_CUMULATIVETURN	(1 << 9)
#define F_BALLSTYLE		(1 << 10)
#define F_POLYSTYLE		(1 << 11)

#endif

/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <sys/param.h>
#else
#include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "const.h"
#include "global.h"
#include "proto.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"
#include "error.h"
#include "srecord.h"
#include "click.h"
#include "commonproto.h"

char frame_version[] = VERSION;

#ifndef	lint
static char sourceid[] =
    "@(#)$Id$";
#endif


/*
 * Structure for calculating if a pixel is visible by a player.
 * The following always holds:
 *	(world.x >= realWorld.x && world.y >= realWorld.y)
 */
typedef struct {
    position	world;			/* Lower left hand corner is this */
					/* world coordinate */
    position	realWorld;		/* If the player is on the edge of
					   the screen, these are the world
					   coordinates before adjustment... */
} pixel_visibility_t;

typedef struct {
    unsigned char	x, y;
} debris_t;

typedef struct {
    short		x, y, size;
} radar_t;


long			frame_loops = 1;
static long		last_frame_shuffle;
static unsigned short	object_shuffle[MAX_TOTAL_SHOTS];
static unsigned short	player_shuffle[64 + MAX_PSEUDO_PLAYERS];
static radar_t		*radar_ptr;
static int		num_radar, max_radar;

static pixel_visibility_t pv;
static int		view_width,
			view_height,
			debris_x_areas,
			debris_y_areas,
			debris_areas,
			debris_colors,
			spark_rand;
static debris_t		*debris_ptr[DEBRIS_TYPES];
static unsigned		debris_num[DEBRIS_TYPES],
			debris_max[DEBRIS_TYPES];
static debris_t		*fastshot_ptr[DEBRIS_TYPES * 2];
static unsigned		fastshot_num[DEBRIS_TYPES * 2],
			fastshot_max[DEBRIS_TYPES * 2];

/*
 * Macro to make room in a given dynamic array for new elements.
 * P is the pointer to the array memory.
 * N is the current number of elements in the array.
 * M is the current size of the array.
 * T is the type of the elements.
 * E is the number of new elements to store in the array.
 * The goal is to keep the number of malloc/realloc calls low
 * while not wasting too much memory because of over-allocation.
 */
#define EXPAND(P,N,M,T,E)						\
    if ((N) + (E) > (M)) {						\
	if ((M) <= 0) {							\
	    M = (E) + 2;						\
	    P = (T *) malloc((M) * sizeof(T));				\
	    N = 0;							\
	} else {							\
	    M = ((M) << 1) + (E);					\
	    P = (T *) realloc(P, (M) * sizeof(T));			\
	}								\
	if (P == NULL) {						\
	    error("No memory");						\
	    N = M = 0;							\
	    return;	/* ! */						\
	}								\
    }

#define inview(x_, y_) 								\
    (   (   ((x_) > pv.world.x && (x_) < pv.world.x + view_width)		\
	 || ((x_) > pv.realWorld.x && (x_) < pv.realWorld.x + view_width))	\
     && (   ((y_) > pv.world.y && (y_) < pv.world.y + view_height)		\
	 || ((y_) > pv.realWorld.y && (y_) < pv.realWorld.y + view_height)))

#define DEBRIS_STORE(xd,yd,color,offset) \
    int			i;						  \
    if (xd < 0) {							  \
	xd += World.width;						  \
    }									  \
    if (yd < 0) {							  \
	yd += World.height;						  \
    }									  \
    if ((unsigned) xd >= (unsigned)view_width || (unsigned) yd >= (unsigned)view_height) {	  \
	/*								  \
	 * There's some rounding error or so somewhere.			  \
	 * Should be possible to resolve it.				  \
	 */								  \
	return;								  \
    }									  \
									  \
    i = offset + color * debris_areas					  \
	+ (((yd >> 8) % debris_y_areas) * debris_x_areas)		  \
	+ ((xd >> 8) % debris_x_areas);					  \
									  \
    if (num_ >= 255) {							  \
	return;								  \
    }									  \
    if (num_ >= max_) {							  \
	if (num_ == 0) {							  \
	    ptr_ = (debris_t *) malloc((max_ = 16) * sizeof(*ptr_));	  \
	} else {							  \
	    ptr_ = (debris_t *) realloc(ptr_, (max_ += max_) * sizeof(*ptr_)); \
	}								  \
	if (ptr_ == 0) {							  \
	    error("No memory for debris");				  \
	    num_ = 0;							  \
	    return;							  \
	}								  \
    }									  \
    ptr_[num_].x = (unsigned char) xd;					  \
    ptr_[num_].y = (unsigned char) yd;					  \
    num_++;

static void fastshot_store(int xf, int yf, int color, int offset)
{
#define ptr_		(fastshot_ptr[i])
#define num_		(fastshot_num[i])
#define max_		(fastshot_max[i])
    DEBRIS_STORE(xf, yf, color, offset);
#undef ptr_
#undef num_
#undef max_
}

static void debris_store(int xf, int yf, int color)
{
#define ptr_		(debris_ptr[i])
#define num_		(debris_num[i])
#define max_		(debris_max[i])
    DEBRIS_STORE(xf, yf, color, 0);
#undef ptr_
#undef num_
#undef max_
}

static void fastshot_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES * 2; i++) {
	if (fastshot_num[i] != 0) {
	    Send_fastshot(conn, i,
			  (unsigned char *) fastshot_ptr[i],
			  fastshot_num[i]);
	    fastshot_num[i] = 0;
	}
    }
}

static void debris_end(int conn)
{
    int			i;

    for (i = 0; i < DEBRIS_TYPES; i++) {
	if (debris_num[i] != 0) {
	    Send_debris(conn, i,
			(unsigned char *) debris_ptr[i],
			debris_num[i]);
	    debris_num[i] = 0;
	}
    }
}

static void Frame_radar_buffer_reset(void)
{
    num_radar = 0;
}

static void Frame_radar_buffer_add(int x, int y, int s)
{
    radar_t		*p;

    EXPAND(radar_ptr, num_radar, max_radar, radar_t, 1);
    p = &radar_ptr[num_radar++];
    p->x = x;
    p->y = y;
    p->size = s;
}

static void Frame_radar_buffer_send(int conn)
{
    int		i;
    int		dest;
    int		tmp;
    radar_t	*p;
    const int	radar_width = 256;
    int		radar_height = (radar_width * World.height) / World.width;
    int		radar_x;
    int		radar_y;
    int		send_x;
    int		send_y;
    unsigned short radar_shuffle[MAX_TOTAL_SHOTS];

    for (i = 0; i < num_radar; i++) {
	radar_shuffle[i] = i;
    }

    if (conn < World.NumBases) {
	/* permute. */
	for (i = 0; i < num_radar; i++) {
	    dest = (int)(rfrac() * (num_radar - i)) + i;
	    tmp = radar_shuffle[i];
	    radar_shuffle[i] = radar_shuffle[dest];
	    radar_shuffle[dest] = tmp;
	}
    }

    for (i = 0; i < num_radar; i++) {
	p = &radar_ptr[radar_shuffle[i]];
	radar_x = (radar_width * p->x) / World.width;
	radar_y = (radar_height * p->y) / World.height;
	send_x = (World.width * radar_x) / radar_width;
	send_y = (World.height * radar_y) / radar_height;
	Send_radar(conn, send_x, send_y, p->size);
    }
}

static void Frame_radar_buffer_free(void)
{
    free(radar_ptr);
    radar_ptr = NULL;
    num_radar = 0;
    max_radar = 0;
}


/*
 * Fast conversion of `num' into `str' starting at position `i', returns
 * index of character after converted number.
 */
static int num2str(int num, char *str, int i)
{
    int	digits, t;

    if (num < 0) {
	str[i++] = '-';
	num = -num;
    }
    if (num < 10) {
	str[i++] = '0' + num;
	return i;
    }
    for (t = num, digits = 0; t; t /= 10, digits++)
	;
    for (t = i+digits-1; t >= 0; t--) {
	str[t] = num % 10;
	num /= 10;
    }
    return i + digits;
}

static int Frame_status(int conn, int ind)
{
    static char		mods[MAX_CHARS];
    player		*pl = Players[ind];
    int			n,
			lock_ind,
			lock_id = -1,
			lock_dist = 0,
			lock_dir = 0,
			i,
			showautopilot;

    /*
     * Don't make lock visible during this frame if;
     * 0) we are not player locked or compass is not on.
     * 1) we have limited visibility and the player is out of range.
     * 2) the player is invisible and he's not in our team.
     * 3) he's not actively playing.
     * 4) we have blind mode and he's not on the visible screen.
     * 5) his distance is zero.
     */

    CLR_BIT(pl->lock.tagged, LOCK_VISIBLE);
    if (BIT(pl->lock.tagged, LOCK_PLAYER) && BIT(pl->used, OBJ_COMPASS)) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee || TEAM(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar ||
		inview(Players[lock_ind]->pos.px, Players[lock_ind]->pos.py))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = (int)Wrap_findDir((int)(Players[lock_ind]->pos.px - pl->pos.px),
				    (int)(Players[lock_ind]->pos.py - pl->pos.py));
	    lock_dist = (int)pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (frame_loops % 8) < 4);
    else if (BIT(pl->used, OBJ_AUTOPILOT))
	showautopilot = (frame_loops % 8) < 4;
    else
	showautopilot = 0;

    /*
     * Don't forget to modify Receive_modifier_bank() in netserver.c
     */
    i = 0;
    if (BIT(pl->mods.nuclear, FULLNUCLEAR))
	mods[i++] = 'F';
    if (BIT(pl->mods.nuclear, NUCLEAR))
	mods[i++] = 'N';
    if (BIT(pl->mods.warhead, CLUSTER))
	mods[i++] = 'C';
    if (BIT(pl->mods.warhead, IMPLOSION))
	mods[i++] = 'I';
    if (pl->mods.velocity) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'V';
	i = num2str (pl->mods.velocity, mods, i);
    }
    if (pl->mods.mini) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'X';
	i = num2str (pl->mods.mini + 1, mods, i);
    }
    if (pl->mods.spread) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'Z';
	i = num2str (pl->mods.spread, mods, i);
    }
    if (pl->mods.power) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'B';
	i = num2str (pl->mods.power, mods, i);
    }
    if (pl->mods.laser) {
	if (i) mods[i++] = ' ';
	mods[i++] = 'L';
	mods[i++] = (BIT(pl->mods.laser, STUN) ? 'S' : 'B');
    }
    mods[i] = '\0';
    n = Send_self(conn,
		  pl,
		  lock_id,
		  lock_dist,
		  lock_dir,
		  showautopilot,
		  Players[GetInd[Get_player_id(conn)]]->status,
		  mods);
    if (n <= 0) {
	return 0;
    }

    if (BIT(pl->used, OBJ_EMERGENCY_THRUST))
	Send_thrusttime(conn,
			pl->emergency_thrust_left,
			pl->emergency_thrust_max);
    if (BIT(pl->used, OBJ_EMERGENCY_SHIELD))
	Send_shieldtime(conn,
			pl->emergency_shield_left,
			pl->emergency_shield_max);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(conn, pl->count);
    }
    if (BIT(pl->used, OBJ_PHASING_DEVICE))
	Send_phasingtime(conn,
			 pl->phasing_left,
			 pl->phasing_max);
    if (ShutdownServer != -1) {
	Send_shutdown(conn, ShutdownServer, ShutdownDelay);
    }

    if (rdelaySend > 0) {
	    Send_rounddelay(conn, rdelay, roundDelay * FPS);
	    /*xpprintf("rdelay=%d (%d)\n", rdelay, rdelaySend);*/
    }

    return 1;
}

static void Frame_map(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i, conn_bit = (1 << conn);

    for (i = 0; i < World.NumFuels; i++) {
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
	    if ((CENTER_XCLICK(World.fuel[i].clk_pos.x - pl->pos.cx) <
		 (view_width << CLICK_SHIFT) + BLOCK_CLICKS) &&
		(CENTER_YCLICK(World.fuel[i].clk_pos.y - pl->pos.cy) <
		 (view_height << CLICK_SHIFT) + BLOCK_CLICKS))
		Send_fuel(conn, i, (int) World.fuel[i].fuel);
	}
    }

#if 0
    for (i = 0; i < World.NumCannons; i++) {
	if (block_inview(&bv,
			 World.cannon[i].blk_pos.x,
			 World.cannon[i].blk_pos.y)) {
	    if (BIT(World.cannon[i].conn_mask, conn_bit) == 0) {
		Send_cannon(conn, i, World.cannon[i].dead_time);
	    }
	}
    }

    for (i = 0; i < World.NumTargets; i++) {
	target_t *targ = &World.targets[i];

	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& block_inview(&bv, targ->pos.x, targ->pos.y))) {
	    Send_target(conn, i, targ->dead_time, targ->damage);
	}
    }
#endif
}

static void Frame_shuffle(void)
{
    int			i;
    unsigned short	tmp, dest;

    if (last_frame_shuffle != frame_loops) {
	last_frame_shuffle = frame_loops;

	for (i = 0; i < NumObjs; i++) {
	    object_shuffle[i] = i;
	}
	/* permute. */
	for (i = 0; i < NumObjs; i++) {
	    dest = (int)(rfrac() * (NumObjs - i)) + i;
	    tmp = object_shuffle[i];
	    object_shuffle[i] = object_shuffle[dest];
	    object_shuffle[dest] = tmp;
	}

	for (i = 0; i < NumPlayers; i++) {
	    player_shuffle[i] = i;
	}
	/* permute. */
	for (i = 0; i < NumPlayers; i++) {
	    dest = (int)(rfrac() * (NumPlayers - i)) + i;
	    tmp = player_shuffle[i];
	    player_shuffle[i] = player_shuffle[dest];
	    player_shuffle[dest] = tmp;
	}

    }
}

static void Frame_shots(int conn, int ind)
{
    player			*pl = Players[ind];
    int				i, color, x, y, fuzz = 0, teamshot, len;
    object			*shot;

    for (i = 0; i < NumObjs; i++) {
	shot = Obj[object_shuffle[i]];
	x = shot->pos.px;
	y = shot->pos.py;
	if (!inview(x, y)) {
	    continue;
	}
	if ((color = shot->color) == BLACK) {
	    xpprintf("black %d,%d\n", shot->type, shot->id);
	    color = WHITE;
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	    if ((fuzz >>= 7) < 0x40) {
		if (conn < World.NumBases)  /* if not pl-> */
		    fuzz = randomMT();
		else
		    fuzz = 0;
	    }
	    if ((fuzz & 0x7F) >= spark_rand) {
		/*
		 * produce a sparkling effect by not displaying
		 * particles every frame.
		 */
		break;
	    }
	    /*
	     * The number of colors which the client
	     * uses for displaying debris is bigger than 2
	     * then the color used denotes the temperature
	     * of the debris particles.
	     * Higher color number means hotter debris.
	     */
	    if (debris_colors >= 3) {
		if (debris_colors > 4) {
		    if (color == BLUE) {
			color = (shot->life >> 1);
		    } else {
			color = (shot->life >> 2);
		    }
		} else {
		    if (color == BLUE) {
			color = (shot->life >> 2);
		    } else {
			color = (shot->life >> 3);
		    }
		}
		if (color >= debris_colors) {
		    color = debris_colors - 1;
		}
	    }

	    debris_store((int)(shot->pos.px - pv.world.x),
			 (int)(shot->pos.py - pv.world.y),
			 color);
	    break;

	case OBJ_WRECKAGE:
	    if (spark_rand != 0 || wreckageCollisionMayKill) {
		Send_wreckage(conn, x, y,
			      (u_byte)shot->info, shot->size, shot->rotation);
	    }
	    break;

	case OBJ_SHOT:
	    if (BIT(World.rules->mode, TEAM_PLAY)
		&& teamImmunity
		&& shot->team == pl->team
		&& shot->id != pl->id) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (frame_loops & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else {
		teamshot = 0;
	    }

	    fastshot_store((int)(shot->pos.px - pv.world.x),
			   (int)(shot->pos.py - pv.world.y),
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, x, y, len, shot->dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, x, y, shot->id);
	    break;
	case OBJ_MINE:
	    {
		int id = 0;
		int laid_by_team = 0;
		int confused = 0;
		/* calculate whether ownership of mine can be determined */
		if (identifyMines
		    && (Wrap_length(pl->pos.cx - shot->pos.cx,
				    pl->pos.cy - shot->pos.cy) / CLICK
			< (SHIP_SZ + MINE_SENSE_BASE_RANGE
			   + pl->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))) {
		    id = shot->id;
		    if (id==-1)
			id = EXPIRED_MINE_ID;
		    if (BIT(shot->status, CONFUSED))
			confused = 1;
		}
		laid_by_team = ((BIT(World.rules->mode, TEAM_PLAY)
				 && teamImmunity
				 && shot->team == pl->team
				 && shot->id != pl->id)
				|| (BIT(shot->status, OWNERIMMUNE)
				    && shot->owner == pl->id));
		if (confused) {
		    id = 0;
		    laid_by_team = (rfrac() < 0.5f);
		}
		Send_mine(conn, x, y, laid_by_team, id);
	    }
	    break;
	case OBJ_ITEM:
	    Send_item(conn, x, y, shot->info);
	    break;
	default:
	    error("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(int conn, int ind)
{
    player			*pl = Players[ind],
				*pl_i;
    pulse_t			*pulse;
    int				i, j, k, color, dir;
    int 			x, y;

    for (j = 0; j < NumPulses; j++) {
	pulse = Pulses[j];
	if (pulse->len <= 0) {
	    continue;
	}
	x = WRAP_XCLICK(pulse->pos.x);
	y = WRAP_YCLICK(pulse->pos.y);
	if (inview(CLICK_TO_PIXEL(x), CLICK_TO_PIXEL(y))) {
	    dir = pulse->dir;
	} else {
	    x += WRAP_XCLICK(tcos(pulse->dir) * pulse->len);
	    y += WRAP_YCLICK(tsin(pulse->dir) * pulse->len);
	    if (inview(CLICK_TO_PIXEL(x), CLICK_TO_PIXEL(y))) {
		dir = MOD2(pulse->dir + RES/2, RES);
	    }
	    else {
		continue;
	    }
	}
	if (BIT(World.rules->mode, TEAM_PLAY)
	    && teamImmunity
	    && pulse->team == pl->team
	    && pulse->id != pl->id) {
	    color = BLUE;
	} else {
	    color = RED;
	}
	Send_laser(conn, color, CLICK_TO_PIXEL(x), CLICK_TO_PIXEL(y), CLICK_TO_PIXEL(pulse->len), dir);
    }
    for (i = 0; i < NumEcms; i++) {
	ecm_t *ecm = Ecms[i];
	Send_ecm(conn, (int)ecm->pos.x, (int)ecm->pos.y, ecm->size);
    }
    for (i = 0; i < NumTransporters; i++) {
	trans_t *trans = Transporters[i];
	player 	*victim = Players[GetInd[trans->target]],
		*pl = (trans->id == -1 ? NULL : Players[GetInd[trans->id]]);
	int 	x = (pl ? pl->pos.px : trans->pos.x),
		y = (pl ? pl->pos.py : trans->pos.y);
	Send_trans(conn, victim->pos.px, victim->pos.py, x, y);
    }
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = World.cannon + i;
	if (cannon->tractor_count > 0) {
	    player *t = Players[GetInd[cannon->tractor_target]];
	    if (inview(t->pos.px, t->pos.py)) {
		int j;
		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   t->pos.px + CLICK_TO_PIXEL(t->ship->pts[j][t->dir].x),
				   t->pos.py + CLICK_TO_PIXEL(t->ship->pts[j][t->dir].y),
				   CLICK_TO_PIXEL(cannon->clk_pos.x),
				   CLICK_TO_PIXEL(cannon->clk_pos.y), 1);
		}
	    }
	}
    }

    for (k = 0; k < NumPlayers; k++) {
	i = player_shuffle[k];
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	if (!inview(pl_i->pos.px, pl_i->pos.py)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(conn,
			pl_i->pos.px,
			pl_i->pos.py,
			pl_i->count);
	    continue;
	}

	/* Don't transmit information if fighter is invisible */
	if (pl->visibility[i].canSee
	    || i == ind
	    || TEAM(i, ind)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(conn,
		      pl_i->pos.px,
		      pl_i->pos.py,
		      pl_i->id,
		      pl_i->dir,
		      BIT(pl_i->used, OBJ_SHIELD) != 0,
		      (BIT(pl_i->used, OBJ_CLOAKING_DEVICE) != 0
		       || BIT(pl_i->used, OBJ_PHASING_DEVICE) != 0),
		      BIT(pl_i->used, OBJ_EMERGENCY_SHIELD) != 0,
			  BIT(pl_i->used, OBJ_PHASING_DEVICE) != 0,
			  BIT(pl_i->used, OBJ_DEFLECTOR) != 0
	    );
	}
	if (BIT(pl_i->used, OBJ_REFUEL)) {
	    if (inview(CLICK_TO_PIXEL(World.fuel[pl_i->fs].clk_pos.x),
		       CLICK_TO_PIXEL(World.fuel[pl_i->fs].clk_pos.y))) {
		Send_refuel(conn,
			    CLICK_TO_PIXEL(World.fuel[pl_i->fs].clk_pos.x),
			    CLICK_TO_PIXEL(World.fuel[pl_i->fs].clk_pos.y),
			    pl_i->pos.px,
			    pl_i->pos.py);
	    }
	}
	if (BIT(pl_i->used, OBJ_REPAIR)) {
	    DFLOAT x = (DFLOAT)(World.targets[pl_i->repair_target].pos.x + 0.5) * BLOCK_SZ;
	    DFLOAT y = (DFLOAT)(World.targets[pl_i->repair_target].pos.y + 0.5) * BLOCK_SZ;
	    if (inview(x, y)) {
		/* same packet as refuel */
		Send_refuel(conn, pl_i->pos.px, pl_i->pos.py, (int) x, (int) y);
	    }
	}
	if (BIT(pl_i->used, OBJ_TRACTOR_BEAM)) {
	    player *t = Players[GetInd[pl_i->lock.pl_id]];
	    if (inview(t->pos.px, t->pos.py)) {
		int j;

		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   (int)(t->pos.px + CLICK_TO_PIXEL(t->ship->pts[j][t->dir].x)),
				   (int)(t->pos.py + CLICK_TO_PIXEL(t->ship->pts[j][t->dir].y)),
				   pl_i->pos.px,
				   pl_i->pos.py, 1);
		}
	    }
	}

	if (pl_i->ball != NULL
	    && inview(pl_i->ball->pos.px, pl_i->ball->pos.py)) {
	    Send_connector(conn,
			   pl_i->ball->pos.px,
			   pl_i->ball->pos.py,
			   pl_i->pos.px,
			   pl_i->pos.py, 0);
	}
    }
}

static void Frame_radar(int conn, int ind)
{
    int			i, mask, shownuke, s;
    player		*pl = Players[ind];
    object		*shot;
    int 		x, y;

#ifndef NO_SMART_MIS_RADAR
    if (nukesOnRadar) {
	mask = OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_MINE;
    } else {
	mask = (missilesOnRadar ?
		(OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT) : 0);
	mask |= (minesOnRadar) ? OBJ_MINE : 0;
    }
    if (treasuresOnRadar)
	mask |= OBJ_BALL;

    if (mask) {
	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[object_shuffle[i]];
	    if (! BIT(shot->type, mask))
		continue;

	    shownuke = (nukesOnRadar && (shot)->mods.nuclear);
	    if (shownuke && (frame_loops & 2)) {
		s = 3;
	    } else {
		s = 0;
	    }

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (frame_loops % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		s = 2;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (frame_loops & 1)
		    continue;
	    }

	    x = shot->pos.cx;
	    y = shot->pos.cy;
	    if (Wrap_length(pl->pos.cx - x,
			     pl->pos.cy - y) <= pl->sensor_range * CLICK) {
		Frame_radar_buffer_add(CLICK_TO_PIXEL(x), CLICK_TO_PIXEL(y), s);
	    }
	}
    }
#endif

    if (playersOnRadar || BIT(World.rules->mode, TEAM_PLAY)) {
	for (i = 0; i < NumPlayers; i++) {
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necessarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams if;
	     *			no playersOnRadar or if not visible
	     */
	    if (Players[i]->conn == conn
		|| BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
		|| (!TEAM(i, ind)
		    && (!playersOnRadar || !pl->visibility[i].canSee))) {
		continue;
	    }
	    x = Players[i]->pos.cx;
	    y = Players[i]->pos.cy;
	    if (BIT(World.rules->mode, LIMITED_VISIBILITY)
		&& Wrap_length(pl->pos.cx - x,
				pl->pos.cy - y) > pl->sensor_range * CLICK) {
		continue;
	    }
	    if (BIT(pl->used, OBJ_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd[pl->lock.pl_id] == i
		&& frame_loops % 5 >= 3) {
		continue;
	    }
	    Frame_radar_buffer_add(CLICK_TO_PIXEL(x), CLICK_TO_PIXEL(y), TEAM(i, ind) ? 3+0x80:3);
	}
    }
}

static void Frame_parameters(int conn, int ind)
{
    player		*pl = Players[ind];

    Get_display_parameters(conn, &view_width, &view_height,
			   &debris_colors, &spark_rand);
    debris_x_areas = (view_width + 255) >> 8;
    debris_y_areas = (view_height + 255) >> 8;
    debris_areas = debris_x_areas * debris_y_areas;

    pv.world.x = pl->pos.px - view_width / 2;	/* Scroll */
    pv.world.y = pl->pos.py - view_height / 2;
    pv.realWorld = pv.world;
    if (BIT (World.rules->mode, WRAP_PLAY)) {
	if (pv.world.x < 0 && pv.world.x + view_width < World.width) {
	    pv.world.x += World.width;
	}
	else if (pv.world.x > 0 && pv.world.x + view_width >= World.width) {
	    pv.realWorld.x -= World.width;
	}
	if (pv.world.y < 0 && pv.world.y + view_height < World.height) {
	    pv.world.y += World.height;
	}
	else if (pv.world.y > 0 && pv.world.y + view_height >= World.height) {
	    pv.realWorld.y -= World.height;
	}
    }
}

void Frame_update(void)
{
    int			i,
			conn,
			ind;
    player		*pl;
    time_t		newTimeLeft = 0;
    static time_t	oldTimeLeft;
    static bool		game_over_called = false;

    if (++frame_loops >= LONG_MAX)	/* Used for misc. timing purposes */
	frame_loops = 0;

    Frame_shuffle();

    if (gameDuration > 0.0
	&& game_over_called == false
	&& oldTimeLeft != (newTimeLeft = gameOverTime - time(NULL))) {
	/*
	 * Do this once a second.
	 */
	if (newTimeLeft <= 0) {
	    Game_Over();
	    ShutdownServer = 30 * FPS;	/* Shutdown in 30 seconds */
	    game_over_called = true;
	}
    }

    for (i = 0; i < observerStart + MAX_OBSERVERS - 1; i++) {
	if (i >= observerStart + NumObservers ||
	    i >= NumPlayers && i < observerStart)
	    continue;
	pl = Players[i];
	conn = pl->conn;
	if (conn == NOT_CONNECTED) {
	    continue;
	}
	playback = (pl->rectype == 1);
	if (BIT(pl->status, PAUSE|GAME_OVER)
	    && !allowViewing && pl->rectype != 2
	    && !pl->isowner) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     * Owner always gets full framerate even if paused.
	     * With allowViewing on, everyone gets full framerate.
	     */
	    if (BIT(pl->status, PAUSE)) {
		if (frame_loops & 0x03) {
		    continue;
		}
	    } else {
		if (frame_loops & 0x01) {
		    continue;
		}
	    }
	}

	/*
	* Reduce frame rate to player's own rate.
	*/
	if (pl->player_count > 0) {
	    pl->player_round++;
	    if (pl->player_round >= pl->player_count) {
		pl->player_round = 0;
		continue;
	    }
	}

	if (Send_start_of_frame(conn) == -1) {
	    continue;
	}
	if (newTimeLeft != oldTimeLeft) {
	    Send_time_left(conn, newTimeLeft);
	}
	/*
	 * If status is GAME_OVER or PAUSE'd, the user may look through the
	 * other players 'eyes'. lockOtherTeam determines whether you can
	 * watch opponents while your own team is still alive (potentially
	 * giving information to your team).
	 *
	 * This is done by using two indexes, one
	 * determining which data should be used (ind, set below) and
	 * one determining which connection to send it to (conn).
	 */
	if (BIT(pl->lock.tagged, LOCK_PLAYER) && (BIT(pl->status, (GAME_OVER|PLAYING)) == (GAME_OVER|PLAYING) || BIT(pl->status, PAUSE))) {
	    ind = GetInd[pl->lock.pl_id];
	} else {
	    ind = i;
	}
	if (pl->rectype == 2) {
	    if (BIT(pl->lock.tagged, LOCK_PLAYER))
		ind = GetInd[pl->lock.pl_id];
	    else
		ind = 0;
	}
	if (Players[ind]->damaged > 0) {
	    Send_damaged(conn, Players[ind]->damaged);
	} else {
	    Frame_parameters(conn, ind);
	    if (Frame_status(conn, ind) <= 0) {
		continue;
	    }
	    Frame_map(conn, ind);
	    Frame_shots(conn, ind);
	    Frame_ships(conn, ind);
	    Frame_radar_buffer_reset();
	    Frame_radar(conn, ind);
	    Frame_radar_buffer_send(conn);
	    if (pl->lose_item_state != 0) {
		Send_loseitem(pl->lose_item, conn);
		if (pl->lose_item_state == 1)
		    pl->lose_item_state = -5;
		if (pl->lose_item_state < 0)
		    pl->lose_item_state++;
	    }
	    debris_end(conn);
	    fastshot_end(conn);
	}
	sound_play_queued(Players[ind]);
	Send_end_of_frame(conn);
    }
    playback = rplayback;
    oldTimeLeft = newTimeLeft;

    Frame_radar_buffer_free();
}

void Set_message(const char *message)
{
    player		*pl;
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	strncpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    if (!rplayback || playback)
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players[i];
	    if (pl->conn != NOT_CONNECTED) {
		Send_message(pl->conn, msg);
	    }
	}
    for (i = 0; i < NumObservers; i++) {
	pl = Players[i + observerStart];
	Send_message(pl->conn, msg);
    }
}

void Set_player_message(player *pl, const char *message)
{
    int			i;
    const char		*msg;
    char		tmp[MSG_LEN];

    if (rplayback && !playback && pl->rectype != 2)
	return;
    if ((i = strlen(message)) >= MSG_LEN) {
#ifndef SILENT
	errno = 0;
	error("Max message len exceed (%d,%s)", i, message);
#endif
	strncpy(tmp, message, MSG_LEN - 1);
	tmp[MSG_LEN - 1] = '\0';
	msg = tmp;
    } else {
	msg = message;
    }
    if (pl->conn != NOT_CONNECTED)
	Send_message(pl->conn, msg);
}

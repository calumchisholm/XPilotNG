/*
 *
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <sys/param.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "bit.h"
#include "netserver.h"
#include "saudio.h"
#include "error.h"
#include "click.h"
#include "commonproto.h"
#include "srecord.h"


char frame_version[] = VERSION;


#define MAX_SHUFFLE_INDEX	65535
#define MAX_VISIBLE_OBJECTS	maxVisibleObject


typedef unsigned short shuffle_t;

/*
 * Structure for calculating if a click position is visible by a player.
 * Used for map state info updating.
 * The following always holds:
 *	(world.cx >= realWorld.cx && world.cy >= realWorld.cy)
 */
typedef struct {
    clpos	world;			/* Lower left hand corner is this */
					/* world coordinate */
    clpos	realWorld;		/* If the player is on the edge of
					   the screen, these are the world
					   coordinates before adjustment... */
} click_visibility_t;

/*
 * Structure with player position info measured in blocks instead of pixels.
 * Used for map state info updating.
 */
/* kps - ng does not want this */
typedef struct {
    ipos		world;
    ipos		realWorld;
} block_visibility_t;

typedef struct {
    unsigned char	x, y;
} debris_t;

typedef struct {
    short		x, y, size;
} radar_t;


extern time_t		gameOverTime;
long			frame_loops = 1;
double			frame_time = 0;
static long		last_frame_shuffle;
static shuffle_t	*object_shuffle_ptr;
static int		num_object_shuffle;
static int		max_object_shuffle;
static shuffle_t	*player_shuffle_ptr;
static int		num_player_shuffle;
static int		max_player_shuffle;
static radar_t		*radar_ptr;
static int		num_radar, max_radar;

static click_visibility_t cv;
static int		view_width,
			view_height,
			view_cwidth,
			view_cheight,
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


/*
 * Note - I've changed the block_inview calls to click_inview calls,
 * which means that the center of a block has to be visible to be
 * inview.
 */
static int click_inview(click_visibility_t *v, int cx, int cy)
{
    return ((cx > v->world.cx && cx < v->world.cx + view_cwidth)
	    || (cx > v->realWorld.cx && cx < v->realWorld.cx + view_cwidth))
	&& ((cy > v->world.cy && cy < v->world.cy + view_cheight)
	    || (cy > v->realWorld.cy && cy < v->realWorld.cy + view_cheight));
}

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
	if (num_ == 0) {						  \
	    ptr_ = (debris_t *) malloc((max_ = 16) * sizeof(*ptr_));	  \
	} else {							  \
	    ptr_ = (debris_t *) realloc(ptr_, (max_ += max_) * sizeof(*ptr_)); \
	}								  \
	if (ptr_ == 0) {						  \
	    error("No memory for debris");				  \
	    num_ = 0;							  \
	    return;							  \
	}								  \
    }									  \
    ptr_[num_].x = (unsigned char) xd;					  \
    ptr_[num_].y = (unsigned char) yd;					  \
    num_++;

static void fastshot_store(int cx, int cy, int color, int offset)
{
    int xf = CLICK_TO_PIXEL(cx),
	yf = CLICK_TO_PIXEL(cy);
#define ptr_		(fastshot_ptr[i])
#define num_		(fastshot_num[i])
#define max_		(fastshot_max[i])
    DEBRIS_STORE(xf, yf, color, offset);
#undef ptr_
#undef num_
#undef max_
}

static void debris_store(int cx, int cy, int color)
{
    int xf = CLICK_TO_PIXEL(cx),
	yf = CLICK_TO_PIXEL(cy);
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

static void Frame_radar_buffer_add(int cx, int cy, int s)
{
    radar_t		*p;

    EXPAND(radar_ptr, num_radar, max_radar, radar_t, 1);
    p = &radar_ptr[num_radar++];
    p->x = CLICK_TO_PIXEL(cx);
    p->y = CLICK_TO_PIXEL(cy);
    p->size = s;
}

static void Frame_radar_buffer_send(int conn)
{
    int			i;
    int			dest;
    int			tmp;
    int			ver;
    radar_t		*p;
    const int		radar_width = 256;
    int			radar_height;
    int			radar_x;
    int			radar_y;
    int			send_x;
    int			send_y;
    shuffle_t		*radar_shuffle;
    size_t		shuffle_bufsize;

    radar_height = (radar_width * World.height) / World.width;

    if (num_radar > MIN(256, MAX_SHUFFLE_INDEX)) {
	num_radar = MIN(256, MAX_SHUFFLE_INDEX);
    }
    shuffle_bufsize = (num_radar * sizeof(shuffle_t));
    radar_shuffle = (shuffle_t *) malloc(shuffle_bufsize);
    if (radar_shuffle == (shuffle_t *) NULL) {
	return;
    }
    for (i = 0; i < num_radar; i++) {
	radar_shuffle[i] = i;
    }

    if (conn < World.NumBases) {
	/* permute. */
	for (i = 0; i < num_radar; i++) {
#if 1
	    dest = (int)(rfrac() * num_radar);
#else /* ng wants this */
	    dest = (int)(rfrac() * (num_radar - i)) + i;
#endif
	    tmp = radar_shuffle[i];
	    radar_shuffle[i] = radar_shuffle[dest];
	    radar_shuffle[dest] = tmp;
	}
    }

    ver = Get_conn_version(conn);
    if (ver <= 0x4400 || (ver >= 0x4F09 && ver < 0x4F11)) {
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    send_x = (World.width * radar_x) / radar_width;
	    send_y = (World.height * radar_y) / radar_height;
	    Send_radar(conn, send_x, send_y, p->size);
	}
    }
    else {
	unsigned char buf[3*256];
	int buf_index = 0;
	int fast_count = 0;

	if (num_radar > 256) {
	    num_radar = 256;
	}
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    if (radar_y >= 1024) {
		continue;
	    }
	    buf[buf_index++] = (unsigned char)(radar_x);
	    buf[buf_index++] = (unsigned char)(radar_y & 0xFF);
	    buf[buf_index] = (unsigned char)((radar_y >> 2) & 0xC0);
	    if (p->size & 0x80) {
		buf[buf_index] |= (unsigned char)(0x20);
	    }
	    buf[buf_index] |= (unsigned char)(p->size & 0x07);
	    buf_index++;
	    fast_count++;
	}
	if (fast_count > 0) {
	    Send_fastradar(conn, buf, fast_count);
	}
    }

    free(radar_shuffle);
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
			lock_id = NO_ID,
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
    if (BIT(pl->lock.tagged, LOCK_PLAYER) && BIT(pl->used, HAS_COMPASS)) {
	lock_id = pl->lock.pl_id;
	lock_ind = GetInd[lock_id];

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee || OWNS_TANK(ind, lock_ind) || TEAM(ind, lock_ind) || ALLIANCE(ind, lock_ind))
#endif
	    && BIT(Players[lock_ind]->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
		|| click_inview(&cv,
				Players[lock_ind]->pos.cx,
				Players[lock_ind]->pos.cy))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = (int)Wrap_findDir((int)(Players[lock_ind]->pos.px - pl->pos.px),
				    (int)(Players[lock_ind]->pos.py - pl->pos.py));
	    lock_dist = (int)pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (frame_loops % 8) < 4);
    else if (BIT(pl->used, HAS_AUTOPILOT))
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

    if (BIT(pl->used, HAS_EMERGENCY_THRUST))
	Send_thrusttime(conn,
			pl->emergency_thrust_left,
			EMERGENCY_THRUST_TIME);
    if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
	Send_shieldtime(conn,
			pl->emergency_shield_left >> TIME_BITS,
			EMERGENCY_SHIELD_TIME >> TIME_BITS);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0) {
	Send_destruct(conn, pl->count >> TIME_BITS);
    }
    if (BIT(pl->used, HAS_PHASING_DEVICE))
	Send_phasingtime(conn,
			 (int)pl->phasing_left,
			 PHASING_TIME);
    if (ShutdownServer != -1) {
	Send_shutdown(conn, ShutdownServer, ShutdownDelay);
    }

    if (round_delay_send > 0) {
	Send_rounddelay(conn, round_delay, roundDelaySeconds * FPS);
    }

    return 1;
}

static void Frame_map(int conn, int ind)
{
    player		*pl = Players[ind];
    int			i,
			k,
			conn_bit = (1 << conn);
    const int		fuel_packet_size = 5;
    const int		cannon_packet_size = 5;
    const int		target_packet_size = 7;
    const int		wormhole_packet_size = 5;
    int			bytes_left = 2000;
    int			max_packet;
    int			packet_count;

    packet_count = 0;
    max_packet = MAX(5, bytes_left / target_packet_size);
    i = MAX(0, pl->last_target_update);
    for (k = 0; k < World.NumTargets; k++) {
	target_t *targ;
	if (++i >= World.NumTargets) {
	    i = 0;
	}
	targ = &World.targets[i];
	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& click_inview(&cv, targ->pos.cx, targ->pos.cy))) {
	    Send_target(conn, i, targ->dead_time >> TIME_BITS, targ->damage);
	    pl->last_target_update = i;
	    bytes_left -= target_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / cannon_packet_size);
    i = MAX(0, pl->last_cannon_update);
    for (k = 0; k < World.NumCannons; k++) {
	if (++i >= World.NumCannons) {
	    i = 0;
	}
	if (click_inview(&cv, World.cannon[i].pos.cx, World.cannon[i].pos.cy)) {
	    if (BIT(World.cannon[i].conn_mask, conn_bit) == 0) {
		Send_cannon(conn, i, World.cannon[i].dead_time >> TIME_BITS);
		pl->last_cannon_update = i;
		bytes_left -= max_packet * cannon_packet_size;
		if (++packet_count >= max_packet) {
		    break;
		}
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / fuel_packet_size);
    i = MAX(0, pl->last_fuel_update);
    for (k = 0; k < World.NumFuels; k++) {
	if (++i >= World.NumFuels) {
	    i = 0;
	}
	if (BIT(World.fuel[i].conn_mask, conn_bit) == 0) {
#if 0 /* old block based stuff */
	    if (World.block[World.fuel[i].blk_pos.x]
			   [World.fuel[i].blk_pos.y] == FUEL) {
		if (block_inview(&bv,
				 World.fuel[i].blk_pos.x,
				 World.fuel[i].blk_pos.y)) {
		    Send_fuel(conn, i, (int) World.fuel[i].fuel);
		    pl->last_fuel_update = i;
		    bytes_left -= max_packet * fuel_packet_size;
		    if (++packet_count >= max_packet) {
			break;
		    }
		}
	    }
#else
	    if ((CENTER_XCLICK(World.fuel[i].pos.cx - pl->pos.cx) <
		 (view_width << CLICK_SHIFT) + BLOCK_CLICKS) &&
		(CENTER_YCLICK(World.fuel[i].pos.cy - pl->pos.cy) <
		 (view_height << CLICK_SHIFT) + BLOCK_CLICKS)) {
		Send_fuel(conn, i, (int) World.fuel[i].fuel);
		pl->last_fuel_update = i;
		bytes_left -= max_packet * fuel_packet_size;
		if (++packet_count >= max_packet) {
		    break;
		}
	    }
#endif
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / wormhole_packet_size);
    i = MAX(0, pl->last_wormhole_update);
    for (k = 0; k < World.NumWormholes; k++) {
	wormhole_t *worm;
	if (++i >= World.NumWormholes) {
	    i = 0;
	}
	worm = &World.wormHoles[i];
	if (wormholeVisible
	    && worm->temporary
	    && (worm->type == WORM_IN
		|| worm->type == WORM_NORMAL)
	    && click_inview(&cv, worm->pos.cx, worm->pos.cy)) {
	    /* This is really a stupid bug: he first converts
	       the perfect blocksizes to pixels which the
	       client is perfectly capable of doing itself.
	       Then he sends the pixels in signed shorts.
	       This will fail on big maps. */
#if 0
	    int	x = (worm->pos.x * BLOCK_SZ) + BLOCK_SZ / 2,
		y = (worm->pos.y * BLOCK_SZ) + BLOCK_SZ / 2;
#endif
	    Send_wormhole(conn, worm->pos.cx, worm->pos.cy);
	    pl->last_wormhole_update = i;
	    bytes_left -= max_packet * wormhole_packet_size;
	    if (++packet_count >= max_packet) {
		break;
	    }
	}
    }
}


static void Frame_shuffle_objects(void)
{
    int				i;
    size_t			memsize;

    num_object_shuffle = MIN(NumObjs, MAX_VISIBLE_OBJECTS);

    if (max_object_shuffle < num_object_shuffle) {
	if (object_shuffle_ptr != NULL) {
	    free(object_shuffle_ptr);
	}
	max_object_shuffle = num_object_shuffle;
	memsize = max_object_shuffle * sizeof(shuffle_t);
	object_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (object_shuffle_ptr == NULL) {
	    max_object_shuffle = 0;
	}
    }

    if (max_object_shuffle < num_object_shuffle) {
	num_object_shuffle = max_object_shuffle;
    }

    for (i = 0; i < num_object_shuffle; i++) {
	object_shuffle_ptr[i] = i;
    }
    /* permute. Not perfect distribution but probably doesn't matter here */
    for (i = num_object_shuffle - 1; i >= 0; --i) {
	if (object_shuffle_ptr[i] == i) {
	    int j = (int)(rfrac() * i);
	    shuffle_t tmp = object_shuffle_ptr[j];
	    object_shuffle_ptr[j] = object_shuffle_ptr[i];
	    object_shuffle_ptr[i] = tmp;
	}
    }
}

static void Frame_shuffle_players(void)
{
    int				i;
    size_t			memsize;

    num_player_shuffle = MIN(NumPlayers, MAX_SHUFFLE_INDEX);

    if (max_player_shuffle < num_player_shuffle) {
	if (player_shuffle_ptr != NULL) {
	    free(player_shuffle_ptr);
	}
	max_player_shuffle = num_player_shuffle;
	memsize = max_player_shuffle * sizeof(shuffle_t);
	player_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (player_shuffle_ptr == NULL) {
	    max_player_shuffle = 0;
	}
    }

    if (max_player_shuffle < num_player_shuffle) {
	num_player_shuffle = max_player_shuffle;
    }

    for (i = 0; i < num_player_shuffle; i++) {
	player_shuffle_ptr[i] = i;
    }
    /* permute. */
    for (i = 0; i < num_player_shuffle; i++) {
	int j = (int)(rfrac() * (num_player_shuffle - i) + i);
	shuffle_t tmp = player_shuffle_ptr[j];
	player_shuffle_ptr[j] = player_shuffle_ptr[i];
	player_shuffle_ptr[i] = tmp;
    }
}


static void Frame_shuffle(void)
{
    if (last_frame_shuffle != frame_loops) {
	last_frame_shuffle = frame_loops;
	Frame_shuffle_objects();
	Frame_shuffle_players();
    }
}

static void Frame_shots(int conn, int ind)
{
    player			*pl = Players[ind];
    int				cx, cy;
    int				lcx, lcy, ldir;
    int				i, k, color;
    int				fuzz = 0, teamshot, len;
    int				obj_count;
    object			*shot;
    object			**obj_list;
    int				hori_blocks, vert_blocks;

    hori_blocks = (view_width + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    vert_blocks = (view_height + (BLOCK_SZ - 1)) / (2 * BLOCK_SZ);
    Cell_get_objects(OBJ_X_IN_BLOCKS(pl), OBJ_Y_IN_BLOCKS(pl),
		     MAX(hori_blocks, vert_blocks), num_object_shuffle,
		     &obj_list,
		     &obj_count);
    for (k = 0; k < num_object_shuffle; k++) {
	i = object_shuffle_ptr[k];
	if (i >= obj_count) {
	    continue;
	}
	shot = obj_list[i];
	cx = shot->pos.cx;
	cy = shot->pos.cy;

	if (shot->type != OBJ_PULSE) {
	    if (!click_inview(&cv, cx, cy))
		continue;
	} else {
	    pulseobject *pulse = PULSE_PTR(shot);

	    /* this is ugly, but seems to work */
	    if (click_inview(&cv, cx, cy)) {
		lcx = cx;
		lcy = cy;
		ldir = MOD2(pulse->dir + RES/2, RES);
	    } else {
		lcx = WRAP_XCLICK(cx - tcos(pulse->dir) * pulse->len * CLICK);
		lcy = WRAP_YCLICK(cy - tsin(pulse->dir) * pulse->len * CLICK);
		ldir = pulse->dir;
		if (!click_inview(&cv, lcx, lcy))
		    continue;
	    }
	}
	if ((color = shot->color) == BLACK) {
	    xpprintf("black %d,%d\n", shot->type, shot->id);
	    color = WHITE;
	}
	switch (shot->type) {
	case OBJ_SPARK:
	case OBJ_DEBRIS:
	    if ((fuzz >>= 7) < 0x40) {
#if 0
		fuzz = randomMT(); /* old,  but ng wants: */
#else
		if (conn < World.NumBases)  /* if not pl-> */
		    fuzz = randomMT();
		else
		    fuzz = 0;
#endif
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
		    if (color == BLUE)
			color = (int)shot->life / 2;
		    else
			color = (int)shot->life / 4;
		} else {
		    if (color == BLUE)
			color = (int)shot->life / 4;
		    else
			color = (int)shot->life / 8;
		}
		if (color >= debris_colors)
		    color = debris_colors - 1;
	    }

	    debris_store(shot->pos.cx - cv.world.cx,
			 shot->pos.cy - cv.world.cy,
			 color);
	    break;

	case OBJ_WRECKAGE:
	    if (spark_rand != 0 || wreckageCollisionMayKill) {
		wireobject *wreck = WIRE_PTR(shot);
		Send_wreckage(conn, cx, cy, (u_byte)wreck->info,
			      wreck->size, wreck->rotation);
	    }
	    break;

	case OBJ_ASTEROID: {
		wireobject *ast = WIRE_PTR(shot);
		Send_asteroid(conn, cx, cy,
			      (u_byte)ast->info, ast->size, ast->rotation);
	    }
	    break;

	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    if (Team_immune(shot->id, pl->id)
		|| (shot->id != NO_ID
		    && BIT(Players[GetInd[shot->id]]->status, PAUSE))
		|| (shot->id == NO_ID
		    && BIT(World.rules->mode, TEAM_PLAY)
		    && shot->team == pl->team)) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->id == pl->id
		&& selfImmunity) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (frame_loops & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else {
		teamshot = 0;
	    }

	    fastshot_store(shot->pos.cx - cv.world.cx,
			   shot->pos.cy - cv.world.cy,
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(conn, cx, cy, len, shot->missile_dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, cx, cy, len, shot->missile_dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, cx, cy, len, shot->missile_dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, cx, cy, shot->id);
	    break;
	case OBJ_MINE:
	    {
		int id = 0;
		int laid_by_team = 0;
		int confused = 0;
		mineobject *mine = MINE_PTR(shot);

		/* calculate whether ownership of mine can be determined */
		if (identifyMines
		    && (Wrap_length(pl->pos.cx - mine->pos.cx,
				    pl->pos.cy - mine->pos.cy) / CLICK
			< (SHIP_SZ + MINE_SENSE_BASE_RANGE
			   + pl->item[ITEM_SENSOR] * MINE_SENSE_RANGE_FACTOR))) {
		    id = mine->id;
		    if (id == NO_ID)
			id = EXPIRED_MINE_ID;
		    if (BIT(mine->status, CONFUSED))
			confused = 1;
		}
		if (mine->id != NO_ID
		    && BIT(Players[GetInd[mine->id]]->status, PAUSE)) {
		    laid_by_team = 1;
		} else {
		    laid_by_team = (Team_immune(mine->id, pl->id)
				    || (BIT(mine->status, OWNERIMMUNE)
					&& mine->owner == pl->id));
		    if (confused) {
			id = 0;
			laid_by_team = (rfrac() < 0.5f);
		    }
		}
		Send_mine(conn, cx, cy, laid_by_team, id);
	    }
	    break;

	case OBJ_ITEM:
	    {
		int item_type = shot->info;

		if (BIT(shot->status, RANDOM_ITEM)) {
		    item_type = Choose_random_item();
		}

		Send_item(conn, cx, cy, item_type);
	    }
	    break;

	case OBJ_PULSE:
	    {
		pulseobject *pulse = PULSE_PTR(shot);

		if (Team_immune(pulse->id, pl->id)) {
		    color = BLUE;
		} else if (pulse->id == pl->id
			   && selfImmunity) {
		    color = BLUE;
		} else {
		    color = RED;
		}
		Send_laser(conn, color, lcx, lcy, (int)pulse->len, ldir);
	    }
	break;
	default:
	    warn("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(int conn, int ind)
{
    player			*pl = Players[ind],
				*pl_i;
    int				i, k;

    for (i = 0; i < NumEcms; i++) {
	ecm_t *ecm = Ecms[i];
	Send_ecm(conn, ecm->pos.cx, ecm->pos.cy, (int)ecm->size);
    }
    for (i = 0; i < NumTransporters; i++) {
	trans_t *trans = Transporters[i];
	player 	*victim = Players[GetInd[trans->target]],
		*pl = (trans->id == NO_ID ? NULL : Players[GetInd[trans->id]]);
	int 	cx = (pl ? pl->pos.cx : trans->pos.cx),
		cy = (pl ? pl->pos.cy : trans->pos.cy);
	Send_trans(conn, victim->pos.cx, victim->pos.cy, cx, cy);
    }
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = World.cannon + i;
	if (cannon->tractor_count > 0) {
	    player *t = Players[GetInd[cannon->tractor_target]];
	    if (click_inview(&cv, t->pos.cx, t->pos.cy)) {
		int j;
		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   t->pos.cx + t->ship->pts[j][t->dir].cx,
				   t->pos.cy + t->ship->pts[j][t->dir].cy,
				   cannon->pos.cx,
				   cannon->pos.cy, 1);
		}
	    }
	}
    }

    for (k = 0; k < num_player_shuffle; k++) {
	i = player_shuffle_ptr[k];
	pl_i = Players[i];
	if (!BIT(pl_i->status, PLAYING|PAUSE)) {
	    continue;
	}
	if (BIT(pl_i->status, GAME_OVER)) {
	    continue;
	}
	if (!click_inview(&cv, pl_i->pos.cx, pl_i->pos.cy)) {
	    continue;
	}
	if (BIT(pl_i->status, PAUSE)) {
	    Send_paused(conn,
			pl_i->pos.cx,
			pl_i->pos.cy,
			pl_i->count >> TIME_BITS);
	    continue;
	}

	/* Don't transmit information if fighter is invisible */
	if (pl->visibility[i].canSee
	    || i == ind
	    || TEAM(i, ind)
	    || ALLIANCE(i, ind)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(conn,
		      pl_i->pos.cx,
		      pl_i->pos.cy,
		      pl_i->id,
		      pl_i->dir,
		      BIT(pl_i->used, HAS_SHIELD) != 0,
#if 1
		      BIT(pl_i->used, HAS_CLOAKING_DEVICE) != 0,
#else /* kps - ng wants this, why? */
		      (BIT(pl_i->used, OBJ_CLOAKING_DEVICE) != 0
		       || BIT(pl_i->used, OBJ_PHASING_DEVICE) != 0),
#endif
		      BIT(pl_i->used, HAS_EMERGENCY_SHIELD) != 0,
		      BIT(pl_i->used, HAS_PHASING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_DEFLECTOR) != 0
	    );
	}
	if (BIT(pl_i->used, HAS_REFUEL)) {
	    if (click_inview(&cv,
			     World.fuel[pl_i->fs].pos.cx,
			     World.fuel[pl_i->fs].pos.cy)) {
		Send_refuel(conn,
			    World.fuel[pl_i->fs].pos.cx,
			    World.fuel[pl_i->fs].pos.cy,
			    pl_i->pos.cx,
			    pl_i->pos.cy);
	    }
	}
	if (BIT(pl_i->used, HAS_REPAIR)) {
	    int cx = World.targets[pl_i->repair_target].pos.cx,
		cy = World.targets[pl_i->repair_target].pos.cy;
	    if (click_inview(&cv, cx, cy)) {
		/* same packet as refuel */
		Send_refuel(conn, pl_i->pos.cx, pl_i->pos.cy, cx, cy);
	    }
	}
	if (BIT(pl_i->used, HAS_TRACTOR_BEAM)) {
	    player *t = Players[GetInd[pl_i->lock.pl_id]];
	    if (click_inview(&cv, t->pos.cx, t->pos.cy)) {
		int j;

		for (j = 0; j < 3; j++) {
		    Send_connector(conn,
				   t->pos.cx + t->ship->pts[j][t->dir].cx,
				   t->pos.cy + t->ship->pts[j][t->dir].cy,
				   pl_i->pos.cx,
				   pl_i->pos.cy, 1);
		}
	    }
	}

	if (pl_i->ball != NULL
	    && click_inview(&cv, pl_i->ball->pos.cx, pl_i->ball->pos.cy)) {
	    Send_connector(conn,
			   pl_i->ball->pos.cx,
			   pl_i->ball->pos.cy,
			   pl_i->pos.cx,
			   pl_i->pos.cy, 0);
	}
    }
}

static void Frame_radar(int conn, int ind)
{
    int			i, k, mask, shownuke, size;
    player		*pl = Players[ind];
    object		*shot;
    int			cx, cy;

    Frame_radar_buffer_reset();

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
    if (asteroidsOnRadar)
	mask |= OBJ_ASTEROID;

    if (mask) {
	for (i = 0; i < NumObjs; i++) {
	    shot = Obj[i];
	    if (! BIT(shot->type, mask))
		continue;

	    shownuke = (nukesOnRadar && (shot)->mods.nuclear);
	    if (shownuke && (frame_loops & 2)) {
		size = 3;
	    } else {
		size = 0;
	    }

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (frame_loops % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		size = 2;
	    } else if (BIT(shot->type, OBJ_ASTEROID)) {
		size = WIRE_PTR(shot)->size + 1;
		size |= 0x80;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (frame_loops & 1)
		    continue;
	    }

	    cx = shot->pos.cx;
	    cy = shot->pos.cy;
	    if (Wrap_length(pl->pos.cx - cx,
			    pl->pos.cy - cy) <= pl->sensor_range * CLICK) {
		Frame_radar_buffer_add(cx, cy, size);
	    }
	}
    }
#endif

    if (playersOnRadar
	|| BIT(World.rules->mode, TEAM_PLAY)
	|| NumPseudoPlayers > 0
	|| NumAlliances > 0) {
	for (k = 0; k < num_player_shuffle; k++) {
	    i = player_shuffle_ptr[k];
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necessarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams or alliances if;
	     *			no playersOnRadar or if not visible
	     */
	    if (Players[i]->conn == conn
		|| BIT(Players[i]->status, PLAYING|PAUSE|GAME_OVER) != PLAYING
		|| (!TEAM(i, ind)
		    && !ALLIANCE(ind, i)
		    && !OWNS_TANK(ind, i)
		    && (!playersOnRadar || !pl->visibility[i].canSee))) {
		continue;
	    }
	    cx = Players[i]->pos.cx;
	    cy = Players[i]->pos.cy;
	    if (BIT(World.rules->mode, LIMITED_VISIBILITY)
		&& Wrap_length(pl->pos.cx - cx,
			       pl->pos.cy - cy) > pl->sensor_range * CLICK) {
		continue;
	    }
	    if (BIT(pl->used, HAS_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd[pl->lock.pl_id] == i
		&& frame_loops % 5 >= 3) {
		continue;
	    }
	    size = 3;
	    if (TEAM(i, ind) || ALLIANCE(ind, i) || OWNS_TANK(ind, i)) {
		size |= 0x80;
	    }
	    Frame_radar_buffer_add(cx, cy, size);
	}
    }

    Frame_radar_buffer_send(conn);
}

static void Frame_lose_item_state(int ind)
{
    player		*pl = Players[ind];

    if (pl->lose_item_state != 0) {
	Send_loseitem(pl->lose_item, pl->conn);
	if (pl->lose_item_state == 1)
	    pl->lose_item_state = -5;
	if (pl->lose_item_state < 0)
	    pl->lose_item_state++;
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

    view_cwidth = view_width * CLICK;
    view_cheight = view_height * CLICK;
    cv.world.cx = pl->pos.cx - view_cwidth / 2;	/* Scroll */
    cv.world.cy = pl->pos.cy - view_cheight / 2;
    cv.realWorld = cv.world;
    if (BIT (World.rules->mode, WRAP_PLAY)) {
	if (cv.world.cx < 0 && cv.world.cx + view_cwidth < World.cwidth) {
	    cv.world.cx += World.cwidth;
	}
	else if (cv.world.cx > 0
		 && cv.world.cx + view_cwidth >= World.cwidth) {
	    cv.realWorld.cx -= World.cwidth;
	}
	if (cv.world.cy < 0 && cv.world.cy + view_cheight < World.cheight) {
	    cv.world.cy += World.cheight;
	}
	else if (cv.world.cy > 0
		 && cv.world.cy + view_cheight >= World.cheight) {
	    cv.realWorld.cy -= World.cheight;
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

    frame_loops++;
    frame_time += timeStep2;

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

    for (i = 0; i < observerStart + NumObservers; i++) {
	if (i >= num_player_shuffle && i < observerStart)
	    continue;
	pl = Players[i];
	conn = pl->conn;
	if (conn == NOT_CONNECTED) {
	    continue;
	}
	playback = (pl->rectype == 1);
	if (BIT(pl->status, PAUSE|GAME_OVER)
	    && pl->rectype != 2
	    && !pl->isowner) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     * Owner always gets full framerate even if paused.
	     * With fullFramerate on, everyone gets full framerate.
	     */
	    if (!fullFramerate) {
		if (teamZeroPausing && pl->team == 0) {
		    if (frame_loops & 0x07)
			continue;
		} else if (BIT(pl->status, PAUSE)) {
		    if (frame_loops & 0x03)
			continue;
		} else {
		    if (frame_loops & 0x01)
			continue;
		}
	    }
	}

	/*
	 * Reduce frame rate to player's own rate.
	 */
	if (pl->player_fps < FPS && !ignoreMaxFPS) {
	    int divisor = (FPS - 1) / pl->player_fps + 1;
	    /* Even combined with above pause check gives at least every
	     * (4 * divisor)th frame. */
	    if (frame_loops % divisor)
 		continue;
	}

	if (Send_start_of_frame(conn) == -1) {
	    continue;
	}
	if (newTimeLeft != oldTimeLeft) {
	    Send_time_left(conn, newTimeLeft);
	} else if (maxRoundTime > 0 && roundtime >= 0) {
	    Send_time_left(conn, (roundtime + FPS - 1) / FPS);
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
	if (BIT(pl->lock.tagged, LOCK_PLAYER)
	    && (BIT(pl->status, (GAME_OVER|PLAYING)) == (GAME_OVER|PLAYING)
		|| BIT(pl->status, PAUSE))) {
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
	    Send_damaged(conn, Players[ind]->damaged >> TIME_BITS);
	} else {
	    Frame_parameters(conn, ind);
	    if (Frame_status(conn, ind) <= 0) {
		continue;
	    }
	    Frame_map(conn, ind);
	    Frame_shots(conn, ind);
	    Frame_ships(conn, ind);
	    Frame_radar(conn, ind);
	    Frame_lose_item_state(i);
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
	warn("Max message len exceed (%d,%s)", i, message);
#endif
	strlcpy(tmp, message, MSG_LEN);
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
	warn("Max message len exceed (%d,%s)", i, message);
#endif
	strlcpy(tmp, message, MSG_LEN);
	msg = tmp;
    } else {
	msg = message;
    }
    if (pl->conn != NOT_CONNECTED) {
	Send_message(pl->conn, msg);
    }
    else if (IS_ROBOT_PTR(pl)) {
	Robot_message(GetInd[pl->id], msg);
    }
}

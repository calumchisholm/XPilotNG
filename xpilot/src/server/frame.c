/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xpserver.h"

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

typedef struct {
    unsigned char	x, y;
} debris_t;

typedef struct {
    short		x, y, size;
} radar_t;


long			frame_loops = 1;
long			frame_loops_slow = 1;
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
 * Note - I've changed the block_inview calls to clpos_inview calls,
 * which means that the center of a block has to be visible to be
 * in view.
 */
static inline bool clpos_inview(click_visibility_t *v, clpos pos)
{
    clpos wpos = v->world, rwpos = v->realWorld;

    if (!((pos.cx > wpos.cx && pos.cx < wpos.cx + view_cwidth)
	  || (pos.cx > rwpos.cx && pos.cx < rwpos.cx + view_cwidth)))
	return false;
    if (!((pos.cy > wpos.cy && pos.cy < wpos.cy + view_cheight)
	  || (pos.cy > rwpos.cy && pos.cy < rwpos.cy + view_cheight)))
	return false;
    return true;
}

#define DEBRIS_STORE(xd,yd,color,offset) \
    int			i;						  \
    if (xd < 0)								  \
	xd += World.width;						  \
    if (yd < 0)								  \
	yd += World.height;						  \
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
    if (num_ >= 255)							  \
	return;								  \
    if (num_ >= max_) {							  \
	if (num_ == 0)							  \
	    ptr_ = (debris_t *) malloc((max_ = 16) * sizeof(*ptr_));	  \
	else								  \
	    ptr_ = (debris_t *) realloc(ptr_, (max_ += max_) * sizeof(*ptr_)); \
	if (ptr_ == NULL) {						  \
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

static void fastshot_end(connection_t *conn)
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

static void debris_end(connection_t *conn)
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

static void Frame_radar_buffer_send(connection_t *conn)
{
    int			i;
    int			dest;
    int			tmp;
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

    if (num_radar > MIN(256, MAX_SHUFFLE_INDEX))
	num_radar = MIN(256, MAX_SHUFFLE_INDEX);
    shuffle_bufsize = (num_radar * sizeof(shuffle_t));
    radar_shuffle = (shuffle_t *) malloc(shuffle_bufsize);
    if (radar_shuffle == (shuffle_t *) NULL)
	return;
    for (i = 0; i < num_radar; i++)
	radar_shuffle[i] = i;

    if (conn->rectype != 2) {
	/* permute. */
	for (i = 0; i < num_radar; i++) {
	    dest = (int)(rfrac() * (num_radar - i)) + i;
	    tmp = radar_shuffle[i];
	    radar_shuffle[i] = radar_shuffle[dest];
	    radar_shuffle[dest] = tmp;
	}
    }

    if (!FEATURE(conn, F_FASTRADAR)) {
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
	unsigned fast_count = 0;

	if (num_radar > 256)
	    num_radar = 256;
	for (i = 0; i < num_radar; i++) {
	    p = &radar_ptr[radar_shuffle[i]];
	    radar_x = (radar_width * p->x) / World.width;
	    radar_y = (radar_height * p->y) / World.height;
	    if (radar_y >= 1024)
		continue;
	    buf[buf_index++] = (unsigned char)(radar_x);
	    buf[buf_index++] = (unsigned char)(radar_y & 0xFF);
	    buf[buf_index] = (unsigned char)((radar_y >> 2) & 0xC0);
	    if (p->size & 0x80)
		buf[buf_index] |= (unsigned char)(0x20);
	    buf[buf_index] |= (unsigned char)(p->size & 0x07);
	    buf_index++;
	    fast_count++;
	}
	if (fast_count > 0)
	    Send_fastradar(conn, buf, fast_count);
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

static int Frame_status(connection_t *conn, player *pl)
{
    static char		mods[MAX_CHARS];
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
	player *lock_pl = Player_by_id(pl->lock.pl_id);

	lock_id = pl->lock.pl_id;
	lock_ind = GetInd(lock_id);

	if ((!BIT(World.rules->mode, LIMITED_VISIBILITY)
	     || pl->lock.distance <= pl->sensor_range)
#ifndef SHOW_CLOAKERS_RANGE
	    && (pl->visibility[lock_ind].canSee
		|| OWNS_TANK(pl, lock_pl)
		|| TEAM(pl, lock_pl)
		|| ALLIANCE(pl, lock_pl))
#endif
	    && BIT(lock_pl->status, PLAYING|GAME_OVER) == PLAYING
	    && (playersOnRadar
		|| clpos_inview(&cv, lock_pl->pos))
	    && pl->lock.distance != 0) {
	    SET_BIT(pl->lock.tagged, LOCK_VISIBLE);
	    lock_dir = Wrap_cfindDir(lock_pl->pos.cx - pl->pos.cx,
				     lock_pl->pos.cy - pl->pos.cy);
	    lock_dist = (int)pl->lock.distance;
	}
    }

    if (BIT(pl->status, HOVERPAUSE))
	showautopilot = (pl->count <= 0 || (frame_loops_slow % 8) < 4);
    else if (BIT(pl->used, HAS_AUTOPILOT))
	showautopilot = (frame_loops_slow % 8) < 4;
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
		  Player_by_id(Get_player_id(conn))->status,
		  mods);
    if (n <= 0)
	return 0;

    if (BIT(pl->used, HAS_EMERGENCY_THRUST))
	Send_thrusttime(conn,
			(int) pl->emergency_thrust_left,
			EMERGENCY_THRUST_TIME);
    if (BIT(pl->used, HAS_EMERGENCY_SHIELD))
	Send_shieldtime(conn,
			(int) pl->emergency_shield_left,
			EMERGENCY_SHIELD_TIME);
    if (BIT(pl->status, SELF_DESTRUCT) && pl->count > 0)
	Send_destruct(conn, (int) pl->count);
    if (BIT(pl->used, HAS_PHASING_DEVICE))
	Send_phasingtime(conn,
			 (int) pl->phasing_left,
			 PHASING_TIME);
    if (ShutdownServer != -1)
	Send_shutdown(conn, ShutdownServer, ShutdownDelay);
    if (round_delay_send > 0)
	Send_rounddelay(conn, round_delay, roundDelaySeconds * FPS);

    return 1;
}

static void Frame_map(connection_t *conn, player *pl)
{
    int			i,
			k,
			conn_bit = (1 << conn->ind);
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
	if (++i >= World.NumTargets)
	    i = 0;
	targ = Targets(i);
	if (BIT(targ->update_mask, conn_bit)
	    || (BIT(targ->conn_mask, conn_bit) == 0
		&& clpos_inview(&cv, targ->pos))) {
	    Send_target(conn, i, (int)targ->dead_time, targ->damage);
	    pl->last_target_update = i;
	    bytes_left -= target_packet_size;
	    if (++packet_count >= max_packet)
		break;
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / cannon_packet_size);
    i = MAX(0, pl->last_cannon_update);
    for (k = 0; k < World.NumCannons; k++) {
	cannon_t *cannon;

	if (++i >= World.NumCannons)
	    i = 0;
	cannon = Cannons(i);
	if (clpos_inview(&cv, cannon->pos)) {
	    if (BIT(cannon->conn_mask, conn_bit) == 0) {
		Send_cannon(conn, i, (int)cannon->dead_time);
		pl->last_cannon_update = i;
		bytes_left -= max_packet * cannon_packet_size;
		if (++packet_count >= max_packet)
		    break;
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / fuel_packet_size);
    i = MAX(0, pl->last_fuel_update);
    for (k = 0; k < World.NumFuels; k++) {
	fuel_t *fs;
	if (++i >= World.NumFuels)
	    i = 0;

	fs = Fuels(i);
	if (BIT(fs->conn_mask, conn_bit) == 0) {
	    if ((CENTER_XCLICK(fs->pos.cx - pl->pos.cx) <
		 (view_width << CLICK_SHIFT) + BLOCK_CLICKS) &&
		(CENTER_YCLICK(fs->pos.cy - pl->pos.cy) <
		 (view_height << CLICK_SHIFT) + BLOCK_CLICKS)) {
		Send_fuel(conn, i, fs->fuel);
		pl->last_fuel_update = i;
		bytes_left -= max_packet * fuel_packet_size;
		if (++packet_count >= max_packet)
		    break;
	    }
	}
    }

    packet_count = 0;
    max_packet = MAX(5, bytes_left / wormhole_packet_size);
    i = MAX(0, pl->last_wormhole_update);
    for (k = 0; k < World.NumWormholes; k++) {
	wormhole_t *worm;
	if (++i >= World.NumWormholes)
	    i = 0;
	worm = Wormholes(i);
	if (wormholeVisible
	    && worm->temporary
	    && (worm->type == WORM_IN
		|| worm->type == WORM_NORMAL)
	    && clpos_inview(&cv, worm->pos)) {
	    Send_wormhole(conn, worm->pos.cx, worm->pos.cy);
	    pl->last_wormhole_update = i;
	    bytes_left -= max_packet * wormhole_packet_size;
	    if (++packet_count >= max_packet)
		break;
	}
    }
}


static void Frame_shuffle_objects(void)
{
    int				i;
    size_t			memsize;

    num_object_shuffle = MIN(NumObjs, MAX_VISIBLE_OBJECTS);

    if (max_object_shuffle < num_object_shuffle) {
	if (object_shuffle_ptr != NULL)
	    free(object_shuffle_ptr);
	max_object_shuffle = num_object_shuffle;
	memsize = max_object_shuffle * sizeof(shuffle_t);
	object_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (object_shuffle_ptr == NULL)
	    max_object_shuffle = 0;
    }

    if (max_object_shuffle < num_object_shuffle)
	num_object_shuffle = max_object_shuffle;

    for (i = 0; i < num_object_shuffle; i++)
	object_shuffle_ptr[i] = i;

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
	if (player_shuffle_ptr != NULL)
	    free(player_shuffle_ptr);
	max_player_shuffle = num_player_shuffle;
	memsize = max_player_shuffle * sizeof(shuffle_t);
	player_shuffle_ptr = (shuffle_t *) malloc(memsize);
	if (player_shuffle_ptr == NULL)
	    max_player_shuffle = 0;
    }

    if (max_player_shuffle < num_player_shuffle)
	num_player_shuffle = max_player_shuffle;

    for (i = 0; i < num_player_shuffle; i++)
	player_shuffle_ptr[i] = i;

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

static void Frame_shots(connection_t *conn, player *pl)
{
    clpos			pos;
    int				ldir = 0;
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
	if (i >= obj_count)
	    continue;
	shot = obj_list[i];
	pos = shot->pos;

	if (shot->type != OBJ_PULSE) {
	    if (!clpos_inview(&cv, shot->pos))
		continue;
	} else {
	    pulseobject *pulse = PULSE_PTR(shot);

	    /* check if either end of laser pulse is in view */
	    if (clpos_inview(&cv, pos))
		ldir = MOD2(pulse->dir + RES/2, RES);
	    else {
		pos.cx = WRAP_XCLICK(pos.cx
				     - tcos(pulse->dir) * pulse->len * CLICK);
		pos.cy = WRAP_YCLICK(pos.cy
				     - tsin(pulse->dir) * pulse->len * CLICK);
		ldir = pulse->dir;
		if (!clpos_inview(&cv, pos))
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
		if (conn->rectype != 2)
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
		Send_wreckage(conn, pos.cx, pos.cy, (u_byte)wreck->info,
			      wreck->size, wreck->rotation);
	    }
	    break;

	case OBJ_ASTEROID: {
		wireobject *ast = WIRE_PTR(shot);
		Send_asteroid(conn, pos.cx, pos.cy,
			      (u_byte)ast->info, ast->size, ast->rotation);
	    }
	    break;

	case OBJ_SHOT:
	case OBJ_CANNON_SHOT:
	    if (Team_immune(shot->id, pl->id)
		|| (shot->id != NO_ID
		    && BIT(Player_by_id(shot->id)->status, PAUSE))
		|| (shot->id == NO_ID
		    && BIT(World.rules->mode, TEAM_PLAY)
		    && shot->team == pl->team)) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->id == pl->id
		&& selfImmunity) {
		color = BLUE;
		teamshot = DEBRIS_TYPES;
	    } else if (shot->mods.nuclear && (frame_loops_slow & 2)) {
		color = RED;
		teamshot = DEBRIS_TYPES;
	    } else
		teamshot = 0;

	    fastshot_store(shot->pos.cx - cv.world.cx,
			   shot->pos.cy - cv.world.cy,
			   color, teamshot);
	    break;

	case OBJ_TORPEDO:
	    len =(distinguishMissiles ? TORPEDO_LEN : MISSILE_LEN);
	    Send_missile(conn, pos.cx, pos.cy, len, shot->missile_dir);
	    break;
	case OBJ_SMART_SHOT:
	    len =(distinguishMissiles ? SMART_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, pos.cx, pos.cy, len, shot->missile_dir);
	    break;
	case OBJ_HEAT_SHOT:
	    len =(distinguishMissiles ? HEAT_SHOT_LEN : MISSILE_LEN);
	    Send_missile(conn, pos.cx, pos.cy, len, shot->missile_dir);
	    break;
	case OBJ_BALL:
	    Send_ball(conn, pos.cx, pos.cy, shot->id);
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
		    && BIT(Player_by_id(mine->id)->status, PAUSE)) {
		    laid_by_team = 1;
		} else {
		    laid_by_team = (Team_immune(mine->id, pl->id)
				    || (BIT(mine->status, OWNERIMMUNE)
					&& mine->owner == pl->id));
		    if (confused) {
			id = 0;
			laid_by_team = (rfrac() < 0.5);
		    }
		}
		Send_mine(conn, pos.cx, pos.cy, laid_by_team, id);
	    }
	    break;

	case OBJ_ITEM:
	    {
		int item_type = shot->info;

		if (BIT(shot->status, RANDOM_ITEM))
		    item_type = Choose_random_item();

		Send_item(conn, pos.cx, pos.cy, item_type);
	    }
	    break;

	case OBJ_PULSE:
	    {
		pulseobject *pulse = PULSE_PTR(shot);

		if (Team_immune(pulse->id, pl->id))
		    color = BLUE;
		else if (pulse->id == pl->id && selfImmunity)
		    color = BLUE;
		else
		    color = RED;
		Send_laser(conn, color, pos.cx, pos.cy, (int)pulse->len, ldir);
	    }
	break;
	default:
	    warn("Frame_shots: Shot type %d not defined.", shot->type);
	    break;
	}
    }
}

static void Frame_ships(connection_t *conn, player *pl)
{
    int				i, k;

    for (i = 0; i < NumEcms; i++) {
	ecm_t *ecm = Ecms[i];
	Send_ecm(conn, ecm->pos.cx, ecm->pos.cy, (int)ecm->size);
    }
    for (i = 0; i < NumTransporters; i++) {
	trans_t *trans = Transporters[i];
	player 	*victim = trans->victim,
		*tpl = (trans->id == NO_ID ? NULL : Player_by_id(trans->id));
	int 	cx = (tpl ? tpl->pos.cx : trans->pos.cx),
		cy = (tpl ? tpl->pos.cy : trans->pos.cy);
	Send_trans(conn, victim->pos.cx, victim->pos.cy, cx, cy);
    }
    for (i = 0; i < World.NumCannons; i++) {
	cannon_t *cannon = Cannons(i);
	if (cannon->tractor_count > 0) {
	    player *t = cannon->tractor_target_pl;
	    if (clpos_inview(&cv, t->pos)) {
		int j;
		for (j = 0; j < 3; j++) {
		    clpos pts = Ship_get_point_clpos(t->ship, j, t->dir);
		    Send_connector(conn,
				   t->pos.cx + pts.cx,
				   t->pos.cy + pts.cy,
				   cannon->pos.cx,
				   cannon->pos.cy, 1);
		}
	    }
	}
    }

    for (k = 0; k < num_player_shuffle; k++) {
	player *pl_i;

	i = player_shuffle_ptr[k];
	pl_i = Players(i);
	if (BIT(pl_i->status, GAME_OVER))
	    continue;
	if (!BIT(pl_i->status, PLAYING) || BIT(pl_i->status, PAUSE)) {
	    if (pl_i->home_base == NULL)
		continue;
	    if (!clpos_inview(&cv, pl_i->home_base->pos))
		continue;
	    if (BIT(pl_i->status, PAUSE))
		Send_paused(conn, pl_i->home_base->pos.cx,
			    pl_i->home_base->pos.cy, (int)pl_i->count);
	    else
		Send_appearing(conn, pl_i->home_base->pos.cx,
			       pl_i->home_base->pos.cy,
			       pl_i->id, (int)(pl_i->count * 10));
	    continue;
	}
	if (!clpos_inview(&cv, pl_i->pos))
	    continue;

	/* Don't transmit information if fighter is invisible */
	if (pl->visibility[i].canSee
	    || pl_i->id == pl->id
	    || TEAM(pl_i, pl)
	    || ALLIANCE(pl_i, pl)) {
	    /*
	     * Transmit ship information
	     */
	    Send_ship(conn,
		      pl_i->pos.cx,
		      pl_i->pos.cy,
		      pl_i->id,
		      pl_i->dir,
		      BIT(pl_i->used, HAS_SHIELD) != 0,
		      BIT(pl_i->used, HAS_CLOAKING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_EMERGENCY_SHIELD) != 0,
		      BIT(pl_i->used, HAS_PHASING_DEVICE) != 0,
		      BIT(pl_i->used, HAS_DEFLECTOR) != 0
	    );
	}

	if (BIT(pl_i->used, HAS_REFUEL)) {
	    fuel_t *fs = Fuels(pl_i->fs);
	    if (clpos_inview(&cv, fs->pos))
		Send_refuel(conn, fs->pos.cx, fs->pos.cy,
			    pl_i->pos.cx, pl_i->pos.cy);
	}

	if (BIT(pl_i->used, HAS_REPAIR)) {
	    target_t *targ = Targets(pl_i->repair_target);
	    if (clpos_inview(&cv, targ->pos))
		/* same packet as refuel */
		Send_refuel(conn, pl_i->pos.cx, pl_i->pos.cy,
			    targ->pos.cx, targ->pos.cy);
	}

	if (BIT(pl_i->used, HAS_TRACTOR_BEAM)) {
	    player *t = Player_by_id(pl_i->lock.pl_id);
	    if (clpos_inview(&cv, t->pos)) {
		int j;

		for (j = 0; j < 3; j++) {
		    clpos pts = Ship_get_point_clpos(t->ship, j, t->dir);
		    Send_connector(conn,
				   t->pos.cx + pts.cx,
				   t->pos.cy + pts.cy,
				   pl_i->pos.cx,
				   pl_i->pos.cy, 1);
		}
	    }
	}

	if (pl_i->ball != NULL
	    && clpos_inview(&cv, pl_i->ball->pos)) {
	    Send_connector(conn,
			   pl_i->ball->pos.cx,
			   pl_i->ball->pos.cy,
			   pl_i->pos.cx,
			   pl_i->pos.cy, 0);
	}
    }
}

static void Frame_radar(connection_t *conn, player *pl)
{
    int			i, k, mask, shownuke, size;
    object		*shot;
    int			cx, cy;

    Frame_radar_buffer_reset();

#ifndef NO_SMART_MIS_RADAR
    if (nukesOnRadar)
	mask = OBJ_SMART_SHOT|OBJ_TORPEDO|OBJ_HEAT_SHOT|OBJ_MINE;
    else {
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
	    if (shownuke && (frame_loops_slow & 2))
		size = 3;
	    else
		size = 0;

	    if (BIT(shot->type, OBJ_MINE)) {
		if (!minesOnRadar && !shownuke)
		    continue;
		if (frame_loops_slow % 8 >= 6)
		    continue;
	    } else if (BIT(shot->type, OBJ_BALL)) {
		size = 2;
	    } else if (BIT(shot->type, OBJ_ASTEROID)) {
		size = WIRE_PTR(shot)->size + 1;
		size |= 0x80;
	    } else {
		if (!missilesOnRadar && !shownuke)
		    continue;
		if (frame_loops_slow & 1)
		    continue;
	    }

	    cx = shot->pos.cx;
	    cy = shot->pos.cy;
	    if (Wrap_length(pl->pos.cx - cx,
			    pl->pos.cy - cy) <= pl->sensor_range * CLICK)
		Frame_radar_buffer_add(cx, cy, size);
	}
    }
#endif

    if (playersOnRadar
	|| BIT(World.rules->mode, TEAM_PLAY)
	|| NumPseudoPlayers > 0
	|| NumAlliances > 0) {
	for (k = 0; k < num_player_shuffle; k++) {
	    player *pl_i;
	    i = player_shuffle_ptr[k];
	    pl_i = Players(i);
	    /*
	     * Don't show on the radar:
	     *		Ourselves (not necessarily same as who we watch).
	     *		People who are not playing.
	     *		People in other teams or alliances if;
	     *			no playersOnRadar or if not visible
	     */
	    if (pl_i->conn == conn
		|| !Player_is_active(pl_i) /* kps - active / playing ??? */
		|| (!TEAM(pl_i, pl)
		    && !ALLIANCE(pl, pl_i)
		    && !OWNS_TANK(pl, pl_i)
		    && (!playersOnRadar || !pl->visibility[i].canSee))) {
		continue;
	    }
	    cx = pl_i->pos.cx;
	    cy = pl_i->pos.cy;
	    if (BIT(World.rules->mode, LIMITED_VISIBILITY)
		&& Wrap_length(pl->pos.cx - cx,
			       pl->pos.cy - cy) > pl->sensor_range * CLICK)
		continue;
	    if (BIT(pl->used, HAS_COMPASS)
		&& BIT(pl->lock.tagged, LOCK_PLAYER)
		&& GetInd(pl->lock.pl_id) == i
		&& frame_loops_slow % 5 >= 3) {
		continue;
	    }
	    size = 3;
	    if (TEAM(pl_i, pl) || ALLIANCE(pl, pl_i) || OWNS_TANK(pl, pl_i))
		size |= 0x80;
	    Frame_radar_buffer_add(cx, cy, size);
	}
    }

    Frame_radar_buffer_send(conn);
}

static void Frame_lose_item_state(player *pl)
{
    if (pl->lose_item_state != 0) {
	Send_loseitem(pl->conn, pl->lose_item);
	if (pl->lose_item_state == 1)
	    pl->lose_item_state = -5;
	if (pl->lose_item_state < 0)
	    pl->lose_item_state++;
    }
}

static void Frame_parameters(connection_t *conn, player *pl)
{
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
	if (cv.world.cx < 0 && cv.world.cx + view_cwidth < World.cwidth)
	    cv.world.cx += World.cwidth;
	else if (cv.world.cx > 0 && cv.world.cx + view_cwidth >= World.cwidth)
	    cv.realWorld.cx -= World.cwidth;
	if (cv.world.cy < 0 && cv.world.cy + view_cheight < World.cheight)
	    cv.world.cy += World.cheight;
	else if (cv.world.cy > 0 && cv.world.cy + view_cheight >=World.cheight)
	    cv.realWorld.cy -= World.cheight;
    }
}

void Frame_update(void)
{
    int			i, ind, player_fps;
    connection_t        *conn;
    player		*pl, *pl2;
    time_t		newTimeLeft = 0;
    static time_t	oldTimeLeft;
    static bool		game_over_called = false;
    static double	frame_time2 = 0.0;

    frame_loops++;
    frame_time += timeStep;
    frame_time2 += timeStep;
    if (frame_time2 >= 1.0) {
	frame_time2 -= 1.0;
	frame_loops_slow++;
    }

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
	pl = Players(i);
	conn = pl->conn;
	if (conn == NULL)
	    continue;
	playback = (pl->rectype == 1);
	player_fps = FPS;
	if (BIT(pl->status, PAUSE|GAME_OVER) && pl->rectype != 2) {
	    /*
	     * Lower the frame rate for non-playing players
	     * to reduce network load.
	     */
	    if (BIT(pl->status, PAUSE) && pausedFPS)
		player_fps = pausedFPS;
	    else if (waitingFPS)
		player_fps = waitingFPS;
	}
	player_fps = MIN(player_fps, pl->player_fps);

	/*
	 * Reduce frame rate to player's own rate.
	 */
	if (player_fps < FPS && !ignoreMaxFPS) {
	    int divisor = (FPS - 1) / player_fps + 1;
	    if (frame_loops % divisor)
 		continue;
	}

	if (Send_start_of_frame(conn) == -1)
	    continue;
	if (newTimeLeft != oldTimeLeft)
	    Send_time_left(conn, newTimeLeft);
	else if (maxRoundTime > 0 && roundtime >= 0)
	    Send_time_left(conn, (roundtime + FPS - 1) / FPS);
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
	    ind = GetInd(pl->lock.pl_id);
	} else
	    ind = i;
	if (pl->rectype == 2) {
	    if (BIT(pl->lock.tagged, LOCK_PLAYER))
		ind = GetInd(pl->lock.pl_id);
	    else
		ind = 0;
	}

	pl2 = Players(ind);
	if (pl2->damaged > 0)
	    Send_damaged(conn, (int)pl2->damaged);
	else {
	    Frame_parameters(conn, pl2);
	    if (Frame_status(conn, pl2) <= 0)
		continue;
	    Frame_map(conn, pl2);
	    Frame_shots(conn, pl2);
	    Frame_ships(conn, pl2);
	    Frame_radar(conn, pl2);
	    Frame_lose_item_state(pl);
	    debris_end(conn);
	    fastshot_end(conn);
	}
	sound_play_queued(pl2);
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
	if (!silent)
	    warn("Max message len exceed (%d,%s)", i, message);
	strlcpy(tmp, message, MSG_LEN);
	msg = tmp;
    } else
	msg = message;

    teamcup_log("    %s\n", message);

    if (!rplayback || playback)
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players(i);
	    if (pl->conn != NULL)
		Send_message(pl->conn, msg);
	}
    for (i = 0; i < NumObservers; i++) {
	pl = Players(i + observerStart);
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
	if (!silent)
	    warn("Max message len exceed (%d,%s)", i, message);
	strlcpy(tmp, message, MSG_LEN);
	msg = tmp;
    } else
	msg = message;
    if (pl->conn != NULL)
	Send_message(pl->conn, msg);
    else if (IS_ROBOT_PTR(pl))
	Robot_message(pl, msg);
}

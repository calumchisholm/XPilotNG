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
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#ifndef _WINDOWS
# include <sys/types.h>  /* freebsd for in.h to work */
# include <sys/socket.h> /* freebsd for in.h to work */
# include <netinet/in.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
# include "../common/NT/winNet.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "score.h"
#include "saudio.h"
#include "item.h"
#include "error.h"
#include "walls.h"
#include "click.h"
#include "objpos.h"

#include "const.h"
#include "map.h"
#include "netserver.h"
#include "pack.h"
#include "srecord.h"

#define WALLDIST_MASK	\
	(FILLED_BIT | REC_LU_BIT | REC_LD_BIT | REC_RU_BIT | REC_RD_BIT \
	| FUEL_BIT | CANNON_BIT | TREASURE_BIT | TARGET_BIT \
	| CHECK_BIT | WORMHOLE_BIT)

unsigned SPACE_BLOCKS = (
	SPACE_BIT | BASE_BIT | WORMHOLE_BIT |
	POS_GRAV_BIT | NEG_GRAV_BIT | CWISE_GRAV_BIT | ACWISE_GRAV_BIT |
	UP_GRAV_BIT | DOWN_GRAV_BIT | RIGHT_GRAV_BIT | LEFT_GRAV_BIT |
	DECOR_LU_BIT | DECOR_LD_BIT | DECOR_RU_BIT | DECOR_RD_BIT |
	DECOR_FILLED_BIT | CHECK_BIT | ITEM_CONCENTRATOR_BIT |
	FRICTION_BIT | ASTEROID_CONCENTRATOR_BIT
    );

struct move_parameters mp;
extern DFLOAT wallBounceExplosionMult;
static char msg[MSG_LEN];


/*
 * Two dimensional array giving for each point the distance
 * to the nearest wall.  Measured in blocks times 2.
 */
static unsigned char **walldist;



/*
 * Allocate memory for the two dimensional "walldist" array.
 */
static void Walldist_alloc(void)
{
    int			x;
    unsigned char	*wall_line;
    unsigned char	**wall_ptr;

    walldist = (unsigned char **)malloc(
		World.x * sizeof(unsigned char *) + World.x * World.y);
    if (!walldist) {
	error("No memory for walldist");
	exit(1);
    }
    wall_ptr = walldist;
    wall_line = (unsigned char *)(wall_ptr + World.x);
    for (x = 0; x < World.x; x++) {
	*wall_ptr = wall_line;
	wall_ptr += 1;
	wall_line += World.y;
    }
}

/*
 * Dump the "walldist" array to file as a Portable PixMap.
 * Mainly used for debugging purposes.
 */
static void Walldist_dump(void)
{
#ifdef DEVELOPMENT
    char		name[1024];
    FILE		*fp;
    int			x, y;
    unsigned char	*line;

    if (!getenv("WALLDISTDUMP")) {
	return;
    }

    sprintf(name, "walldist.ppm");
    fp = fopen(name, "w");
    if (!fp) {
	error("%s", name);
	return;
    }
    line = (unsigned char *)malloc(3 * World.x);
    if (!line) {
	error("No memory for walldist dump");
	fclose(fp);
	return;
    }
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", World.x, World.y);
    fprintf(fp, "%d\n", 255);
    for (y = World.y - 1; y >= 0; y--) {
	for (x = 0; x < World.x; x++) {
	    if (walldist[x][y] == 0) {
		line[x * 3 + 0] = 255;
		line[x * 3 + 1] = 0;
		line[x * 3 + 2] = 0;
	    }
	    else if (walldist[x][y] == 2) {
		line[x * 3 + 0] = 0;
		line[x * 3 + 1] = 255;
		line[x * 3 + 2] = 0;
	    }
	    else if (walldist[x][y] == 3) {
		line[x * 3 + 0] = 0;
		line[x * 3 + 1] = 0;
		line[x * 3 + 2] = 255;
	    }
	    else {
		line[x * 3 + 0] = walldist[x][y];
		line[x * 3 + 1] = walldist[x][y];
		line[x * 3 + 2] = walldist[x][y];
	    }
	}
	fwrite(line, World.x, 3, fp);
    }
    free(line);
    fclose(fp);

    printf("Walldist dumped to %s\n", name);
#endif
}

static void Walldist_init(void)
{
    int			x, y, dx, dy, wx, wy;
    int			dist;
    int			mindist;
    int			maxdist = 2 * MIN(World.x, World.y);
    int			newdist;

    typedef struct Qelmt { short x, y; } Qelmt_t;
    Qelmt_t		*q;
    int			qfront = 0, qback = 0;

    if (maxdist > 255) {
	maxdist = 255;
    }
    q = (Qelmt_t *)malloc(World.x * World.y * sizeof(Qelmt_t));
    if (!q) {
	error("No memory for walldist init");
	exit(1);
    }
    for (x = 0; x < World.x; x++) {
	for (y = 0; y < World.y; y++) {
	    if (BIT((1 << World.block[x][y]), WALLDIST_MASK)
		&& (World.block[x][y] != WORMHOLE
		    || World.wormHoles[wormXY(x, y)].type != WORM_OUT)) {
		walldist[x][y] = 0;
		q[qback].x = x;
		q[qback].y = y;
		qback++;
	    } else {
		walldist[x][y] = maxdist;
	    }
	}
    }
    if (!BIT(World.rules->mode, WRAP_PLAY)) {
	for (x = 0; x < World.x; x++) {
	    for (y = 0; y < World.y; y += (!x || x == World.x - 1)
					? 1 : (World.y - (World.y > 1))) {
		if (walldist[x][y] > 1) {
		    walldist[x][y] = 2;
		    q[qback].x = x;
		    q[qback].y = y;
		    qback++;
		}
	    }
	}
    }
    while (qfront != qback) {
	x = q[qfront].x;
	y = q[qfront].y;
	if (++qfront == World.x * World.y) {
	    qfront = 0;
	}
	dist = walldist[x][y];
	mindist = dist + 2;
	if (mindist >= 255) {
	    continue;
	}
	for (dx = -1; dx <= 1; dx++) {
	    if (BIT(World.rules->mode, WRAP_PLAY)
		|| (x + dx >= 0 && x + dx < World.x)) {
		wx = WRAP_XBLOCK(x + dx);
		for (dy = -1; dy <= 1; dy++) {
		    if (BIT(World.rules->mode, WRAP_PLAY)
			|| (y + dy >= 0 && y + dy < World.y)) {
			wy = WRAP_YBLOCK(y + dy);
			if (walldist[wx][wy] > mindist) {
			    newdist = mindist;
			    if (dist == 0) {
				if (World.block[x][y] == REC_LD) {
				    if (dx == +1 && dy == +1) {
					newdist = mindist + 1;
				    }
				}
				else if (World.block[x][y] == REC_RD) {
				    if (dx == -1 && dy == +1) {
					newdist = mindist + 1;
				    }
				}
				else if (World.block[x][y] == REC_LU) {
				    if (dx == +1 && dy == -1) {
					newdist = mindist + 1;
				    }
				}
				else if (World.block[x][y] == REC_RU) {
				    if (dx == -1 && dy == -1) {
					newdist = mindist + 1;
				    }
				}
			    }
			    if (newdist < walldist[wx][wy]) {
				walldist[wx][wy] = newdist;
				q[qback].x = wx;
				q[qback].y = wy;
				if (++qback == World.x * World.y) {
				    qback = 0;
				}
			    }
			}
		    }
		}
	    }
	}
    }
    free(q);
    Walldist_dump();
}

void Walls_init_old(void)
{
    Walldist_alloc();
    Walldist_init();
}

void Treasure_init(void)
{
    int i;
    for (i = 0; i < World.NumTreasures; i++) {
	Make_treasure_ball(i);
    }
}


static void Bounce_edge(move_state_t *ms, move_bounce_t bounce)
{
    if (bounce == BounceLeft) {
	if (ms->mip->edge_bounce) {
	    ms->todo.cx = -ms->todo.cx;
	    ms->vel.x = -ms->vel.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES / 2 - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.cx = 0;
	    ms->vel.x = 0;
	    if (!ms->mip->pl) {
		ms->dir = (ms->vel.y < 0) ? (3*RES/4) : RES/4;
	    }
	}
    }
    else if (bounce == BounceRight) {
	if (ms->mip->edge_bounce) {
	    ms->todo.cx = -ms->todo.cx;
	    ms->vel.x = -ms->vel.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES / 2 - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.cx = 0;
	    ms->vel.x = 0;
	    if (!ms->mip->pl) {
		ms->dir = (ms->vel.y < 0) ? (3*RES/4) : RES/4;
	    }
	}
    }
    else if (bounce == BounceDown) {
	if (ms->mip->edge_bounce) {
	    ms->todo.cy = -ms->todo.cy;
	    ms->vel.y = -ms->vel.y;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.cy = 0;
	    ms->vel.y = 0;
	    if (!ms->mip->pl) {
		ms->dir = (ms->vel.x < 0) ? (RES/2) : 0;
	    }
	}
    }
    else if (bounce == BounceUp) {
	if (ms->mip->edge_bounce) {
	    ms->todo.cy = -ms->todo.cy;
	    ms->vel.y = -ms->vel.y;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES - ms->dir, RES);
	    }
	}
	else {
	    ms->todo.cy = 0;
	    ms->vel.y = 0;
	    if (!ms->mip->pl) {
		ms->dir = (ms->vel.x < 0) ? (RES/2) : 0;
	    }
	}
    }
    ms->bounce = BounceEdge;
}

static void Bounce_wall(move_state_t *ms, move_bounce_t bounce)
{
    if (!ms->mip->wall_bounce) {
	ms->crash = CrashWall;
	return;
    }
    if (bounce == BounceLeft) {
	ms->todo.cx = -ms->todo.cx;
	ms->vel.x = -ms->vel.x;
	if (!ms->mip->pl) {
	    ms->dir = MOD2(RES/2 - ms->dir, RES);
	}
    }
    else if (bounce == BounceRight) {
	ms->todo.cx = -ms->todo.cx;
	ms->vel.x = -ms->vel.x;
	if (!ms->mip->pl) {
	    ms->dir = MOD2(RES/2 - ms->dir, RES);
	}
    }
    else if (bounce == BounceDown) {
	ms->todo.cy = -ms->todo.cy;
	ms->vel.y = -ms->vel.y;
	if (!ms->mip->pl) {
	    ms->dir = MOD2(RES - ms->dir, RES);
	}
    }
    else if (bounce == BounceUp) {
	ms->todo.cy = -ms->todo.cy;
	ms->vel.y = -ms->vel.y;
	if (!ms->mip->pl) {
	    ms->dir = MOD2(RES - ms->dir, RES);
	}
    }
    else {
	clvec t = ms->todo;
	vector v = ms->vel;
	if (bounce == BounceLeftDown) {
	    ms->todo.cx = -t.cy;
	    ms->todo.cy = -t.cx;
	    ms->vel.x = -v.y;
	    ms->vel.y = -v.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(3*RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceLeftUp) {
	    ms->todo.cx = t.cy;
	    ms->todo.cy = t.cx;
	    ms->vel.x = v.y;
	    ms->vel.y = v.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceRightDown) {
	    ms->todo.cx = t.cy;
	    ms->todo.cy = t.cx;
	    ms->vel.x = v.y;
	    ms->vel.y = v.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(RES/4 - ms->dir, RES);
	    }
	}
	else if (bounce == BounceRightUp) {
	    ms->todo.cx = -t.cy;
	    ms->todo.cy = -t.cx;
	    ms->vel.x = -v.y;
	    ms->vel.y = -v.x;
	    if (!ms->mip->pl) {
		ms->dir = MOD2(3*RES/4 - ms->dir, RES);
	    }
	}
    }
    ms->bounce = bounce;
}

/*
 * Move a point through one block and detect
 * wall collisions or bounces within that block.
 * Complications arise when the point starts at
 * the edge of a block.  E.g., if a point is on the edge
 * of a block to which block does it belong to?
 *
 * The caller supplies a set of input parameters and expects
 * the following output:
 *  - the number of pixels moved within this block.  (ms->done)
 *  - the number of pixels that still remain to be traversed. (ms->todo)
 *  - whether a crash happened, in which case no pixels will have been
 *    traversed. (ms->crash)
 *  - some extra optional output parameters depending upon the type
 *    of the crash. (ms->cannon, ms->wormhole, ms->target, ms->treasure)
 *  - whether the point bounced, in which case no pixels will have been
 *    traversed, only a change in direction. (ms->bounce, ms->vel, ms->todo)
 */
static void Move_segment(move_state_t *ms)
{
    int			i;
    int			block_type;	/* type of block we're going through */
    int			inside;		/* inside the block or else on edge */
    int			need_adjust;	/* other param (x or y) needs recalc */
    unsigned		wall_bounce;	/* are we bouncing? what direction? */
    ipos		block;		/* block index */
    ipos		blk2;		/* new block index */
    ivec		sign;		/* sign (-1 or 1) of direction */
    clpos		delta;		/* delta position in clicks */
    clpos		enter;		/* enter block position in clicks */
    clpos		leave;		/* leave block position in clicks */
    clpos		offset;		/* offset within block in clicks */
    clpos		off2;		/* last offset in block in clicks */
    clpos		mid;		/* the mean of (offset+off2)/2 */
    const move_info_t	*const mi = ms->mip;	/* alias */
    int			hole;		/* which wormhole */
    ballobject		*ball;

    /*
     * Fill in default return values.
     */
    ms->crash = NotACrash;
    ms->bounce = NotABounce;
    ms->done.cx = 0;
    ms->done.cy = 0;

    enter = ms->pos;
    if (enter.cx < 0 || enter.cx >= mp.click_width
	|| enter.cy < 0 || enter.cy >= mp.click_height) {

	if (!mi->edge_wrap) {
	    ms->crash = CrashUniverse;
	    return;
	}
	if (enter.cx < 0) {
	    enter.cx += mp.click_width;
	    if (enter.cx < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.cx >= mp.click_width) {
	    enter.cx -= mp.click_width;
	    if (enter.cx >= mp.click_width) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	if (enter.cy < 0) {
	    enter.cy += mp.click_height;
	    if (enter.cy < 0) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	else if (enter.cy >= mp.click_height) {
	    enter.cy -= mp.click_height;
	    if (enter.cy >= mp.click_height) {
		ms->crash = CrashUniverse;
		return;
	    }
	}
	ms->pos = enter;
    }

    sign.x = (ms->vel.x < 0) ? -1 : 1;
    sign.y = (ms->vel.y < 0) ? -1 : 1;
    block.x = enter.cx / BLOCK_CLICKS;
    block.y = enter.cy / BLOCK_CLICKS;
    if (walldist[block.x][block.y] > 2) {
	int maxcl = ((walldist[block.x][block.y] - 2) * BLOCK_CLICKS) >> 1;
	if (maxcl >= sign.x * ms->todo.cx && maxcl >= sign.y * ms->todo.cy) {
	    /* entire movement is possible. */
	    ms->done.cx = ms->todo.cx;
	    ms->done.cy = ms->todo.cy;
	}
	else if (sign.x * ms->todo.cx > sign.y * ms->todo.cy) {
	    /* horizontal movement. */
	    ms->done.cx = sign.x * maxcl;
	    ms->done.cy = ms->todo.cy * maxcl / (sign.x * ms->todo.cx);
	}
	else {
	    /* vertical movement. */
	    ms->done.cx = ms->todo.cx * maxcl / (sign.y * ms->todo.cy);
	    ms->done.cy = sign.y * maxcl;
	}
	ms->todo.cx -= ms->done.cx;
	ms->todo.cy -= ms->done.cy;
	return;
    }

    offset.cx = enter.cx - block.x * BLOCK_CLICKS;
    offset.cy = enter.cy - block.y * BLOCK_CLICKS;
    inside = 1;
    if (offset.cx == 0) {
	inside = 0;
	if (sign.x == -1 && (offset.cx = BLOCK_CLICKS, --block.x < 0)) {
	    if (mi->edge_wrap) {
		block.x += World.x;
	    }
	    else {
		Bounce_edge(ms, BounceLeft);
		return;
	    }
	}
    }
    else if (enter.cx == mp.click_width - 1
	     && !mi->edge_wrap
	     && ms->vel.x > 0) {
	Bounce_edge(ms, BounceRight);
	return;
    }
    if (offset.cy == 0) {
	inside = 0;
	if (sign.y == -1 && (offset.cy = BLOCK_CLICKS, --block.y < 0)) {
	    if (mi->edge_wrap) {
		block.y += World.y;
	    }
	    else {
		Bounce_edge(ms, BounceDown);
		return;
	    }
	}
    }
    else if (enter.cy == mp.click_height - 1
	     && !mi->edge_wrap
	     && ms->vel.y > 0) {
	Bounce_edge(ms, BounceUp);
	return;
    }

    need_adjust = 0;
    if (sign.x == -1) {
	if (offset.cx + ms->todo.cx < 0) {
	    leave.cx = enter.cx - offset.cx;
	    need_adjust = 1;
	}
	else {
	    leave.cx = enter.cx + ms->todo.cx;
	}
    }
    else {
	if (offset.cx + ms->todo.cx > BLOCK_CLICKS) {
	    leave.cx = enter.cx + BLOCK_CLICKS - offset.cx;
	    need_adjust = 1;
	}
	else {
	    leave.cx = enter.cx + ms->todo.cx;
	}
	if (leave.cx == mp.click_width && !mi->edge_wrap) {
	    leave.cx--;
	    need_adjust = 1;
	}
    }
    if (sign.y == -1) {
	if (offset.cy + ms->todo.cy < 0) {
	    leave.cy = enter.cy - offset.cy;
	    need_adjust = 1;
	}
	else {
	    leave.cy = enter.cy + ms->todo.cy;
	}
    }
    else {
	if (offset.cy + ms->todo.cy > BLOCK_CLICKS) {
	    leave.cy = enter.cy + BLOCK_CLICKS - offset.cy;
	    need_adjust = 1;
	}
	else {
	    leave.cy = enter.cy + ms->todo.cy;
	}
	if (leave.cy == mp.click_height && !mi->edge_wrap) {
	    leave.cy--;
	    need_adjust = 1;
	}
    }
    if (need_adjust && ms->todo.cy && ms->todo.cx) {
	double wx = (double)(leave.cx - enter.cx) / ms->todo.cx;
	double wy = (double)(leave.cy - enter.cy) / ms->todo.cy;
	if (wx > wy) {
	    double x = ms->todo.cx * wy;
	    leave.cx = enter.cx + DOUBLE_TO_INT(x);
	}
	else if (wx < wy) {
	    double y = ms->todo.cy * wx;
	    leave.cy = enter.cy + DOUBLE_TO_INT(y);
	}
    }

    delta.cx = leave.cx - enter.cx;
    delta.cy = leave.cy - enter.cy;

    block_type = World.block[block.x][block.y];

    /*
     * We test for several different bouncing directions against the wall.
     * Sometimes there is more than one bounce possible if the point
     * starts at the corner of a block.
     * Therefore we maintain a bit mask for the bouncing possibilities
     * and later we will determine which bounce is appropriate.
     */
    wall_bounce = 0;

    if (!mi->phased) {

    switch (block_type) {

    default:
	break;

    case WORMHOLE:
	if (!mi->wormhole_warps) {
	    break;
	}
	hole = wormXY(block.x, block.y);
	if (World.wormHoles[hole].type == WORM_OUT) {
	    break;
	}
	if (mi->pl) {
	    blk2.x = OBJ_X_IN_BLOCKS(mi->pl);
	    blk2.y = OBJ_Y_IN_BLOCKS(mi->pl);
	    if (mi->pl->warped > 0) {
		if (World.block[blk2.x][blk2.y] == WORMHOLE) {
		    int oldhole = wormXY(blk2.x, blk2.y);
		    if (World.wormHoles[oldhole].type == WORM_NORMAL
			&& mi->pl->wormHoleDest == oldhole) {
			/*
			 * Don't warp again if we are still on the
			 * same wormhole we have just been warped to.
			 */
			break;
		    }
		}
		mi->pl->warped = 0;
	    }
	    if (blk2.x == block.x && blk2.y == block.y) {
		ms->wormhole = hole;
		ms->crash = CrashWormHole;
		return;
	    }
	}
	else {
	    /*
	     * Warp object if this wormhole has ever warped a player.
	     * Warp the object to the same destination as the
	     * player has been warped to.
	     */
	    int last = World.wormHoles[hole].lastdest;
	    if (last >= 0
		&& (World.wormHoles[hole].countdown > 0 || !wormTime)
		&& last < World.NumWormholes
		&& World.wormHoles[last].type != WORM_IN
		&& last != hole
		&& (OBJ_X_IN_BLOCKS(mi->obj) != block.x
		 || OBJ_Y_IN_BLOCKS(mi->obj) != block.y) ) {
		ms->done.cx += (World.wormHoles[last].pos.cx
			       - World.wormHoles[hole].pos.cx);
		ms->done.cy += (World.wormHoles[last].pos.cy
			       - World.wormHoles[hole].pos.cy);
		break;
	    }
	}
	break;

    case CANNON:
	if (!mi->cannon_crashes) {
	    break;
	}
	if (BIT(mi->obj->status, FROMCANNON)
	    && !BIT(World.rules->mode, TEAM_PLAY)) {
	    break;
	}
	for (i = 0; ; i++) {
	    int bx, by;

	    bx = CLICK_TO_BLOCK(World.cannon[i].pos.cx);
	    by = CLICK_TO_BLOCK(World.cannon[i].pos.cy);
	    if (bx == block.x && by == block.y) {
		break;
	    }
	}
	ms->cannon = i;

	if (BIT(World.cannon[i].used, HAS_PHASING_DEVICE)) {
	    break;
	}

	if (BIT(World.rules->mode, TEAM_PLAY)
	    && (teamImmunity
		|| BIT(mi->obj->status, FROMCANNON))
	    && mi->obj->team == World.cannon[i].team) {
	    break;
	}
	{
	    /*
	     * Calculate how far the point can travel in the cannon block
	     * before hitting the cannon.
	     * To reduce duplicate code we first transform all the
	     * different cannon types into one by matrix multiplications.
	     * Later we transform the result back to the real type.
	     */

	    ivec mx, my, dir;
	    clpos mirx, miry, start, end, todo, done, diff, a, b;
	    double d, w;

	    mirx.cx = 0;
	    mirx.cy = 0;
	    miry.cx = 0;
	    miry.cy = 0;
	    switch (World.cannon[i].dir) {
	    case DIR_UP:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = 1;
		break;
	    case DIR_DOWN:
		mx.x = 1; mx.y = 0;
		my.x = 0; my.y = -1;
		miry.cy = BLOCK_CLICKS;
		break;
	    case DIR_RIGHT:
		mx.x = 0; mx.y = 1;
		my.x = -1; my.y = 0;
		miry.cx = BLOCK_CLICKS;
		break;
	    case DIR_LEFT:
		mx.x = 0; mx.y = -1;
		my.x = 1; my.y = 0;
		mirx.cy = BLOCK_CLICKS;
		break;
	    }
	    start.cx = mirx.cx + mx.x * offset.cx + miry.cx + my.x * offset.cy;
	    start.cy = mirx.cy + mx.y * offset.cx + miry.cy + my.y * offset.cy;
	    diff.cx  =          mx.x * delta.cx           + my.x * delta.cy;
	    diff.cy  =          mx.y * delta.cx           + my.y * delta.cy;
	    dir.x   =          mx.x * sign.x            + my.x * sign.y;
	    dir.y   =          mx.y * sign.x            + my.y * sign.y;
	    todo.cx  =          mx.x * ms->todo.cx       + my.x * ms->todo.cy;
	    todo.cy  =          mx.y * ms->todo.cx       + my.y * ms->todo.cy;

	    end.cx = start.cx + diff.cx;
	    end.cy = start.cy + diff.cy;

	    if (start.cx <= BLOCK_CLICKS/2) {
		if (3 * start.cy <= 2 * start.cx) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.cx <= BLOCK_CLICKS/2) {
		    if (3 * end.cy > 2 * end.cx) {
			break;
		    }
		}
	    }
	    else {
		if (3 * start.cy <= 2 * (BLOCK_CLICKS - start.cx)) {
		    ms->crash = CrashCannon;
		    return;
		}
		if (end.cx > BLOCK_CLICKS/2) {
		    if (3 * end.cy > 2 * (BLOCK_CLICKS - end.cx)) {
			break;
		    }
		}
	    }

	    done = diff;

	    /* is direction x-major? */
	    if (dir.x * diff.cx >= dir.y * diff.cy) {
		/* x-major */
		w = (double) todo.cy / todo.cx;
		if (3 * todo.cy != 2 * todo.cx) {
		    d = (3 * start.cy - 2 * start.cx) / (2 - 3 * w);
		    a.cx = DOUBLE_TO_INT(d);
		    a.cy = (int)(a.cx * w);
		    if (dir.x * a.cx < dir.x * done.cx && dir.x * a.cx >= 0) {
			if (start.cy + a.cy <= BLOCK_CLICKS/3) {
			    done = a;
			    if (!(done.cx | done.cy)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
		if (-3 * todo.cy != 2 * todo.cx) {
		    d = (2 * BLOCK_CLICKS - 2 * start.cx - 3 * start.cy) /
			(2 + 3 * w);
		    b.cx = DOUBLE_TO_INT(d);
		    b.cy = (int)(b.cx * w);
		    if (dir.x * b.cx < dir.x * done.cx && dir.x * b.cx >= 0) {
			if (start.cy + b.cy <= BLOCK_CLICKS/3) {
			    done = b;
			    if (!(done.cx | done.cy)) {
				ms->crash = CrashCannon;
				return;
			    }
			}
		    }
		}
	    } else {
		/* y-major */
		w = (double) todo.cx / todo.cy;
		d = (2 * start.cx - 3 * start.cy) / (3 - 2 * w);
		a.cy = DOUBLE_TO_INT(d);
		a.cx = (int)(a.cy * w);
		if (dir.y * a.cy < dir.y * done.cy && dir.y * a.cy >= 0) {
		    if (start.cy + a.cy <= BLOCK_CLICKS/3) {
			done = a;
			if (!(done.cx | done.cy)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
		d = (2 * BLOCK_CLICKS - 2 * start.cx - 3 * start.cy) /
		    (3 + 2 * w);
		b.cy = DOUBLE_TO_INT(d);
		b.cx = (int)(b.cy * w);
		if (dir.y * b.cy < dir.y * done.cy && dir.y * b.cy >= 0) {
		    if (start.cy + b.cy <= BLOCK_CLICKS/3) {
			done = b;
			if (!(done.cx | done.cy)) {
			    ms->crash = CrashCannon;
			    return;
			}
		    }
		}
	    }

	    delta.cx = mx.x * done.cx + mx.y * done.cy;
	    delta.cy = my.x * done.cx + my.y * done.cy;
	}
	break;

    case TREASURE:
	if (block_type == TREASURE) {
	    if (mi->treasure_crashes) {
		/*
		 * Test if the movement is within the upper half of
		 * the treasure, which is the upper half of a circle.
		 * If this is the case then we test if 3 samples
		 * are not hitting the treasure.
		 */
		const DFLOAT r = 0.5f * BLOCK_CLICKS;
		off2.cx = offset.cx + delta.cx;
		off2.cy = offset.cy + delta.cy;
		mid.cx = (offset.cx + off2.cx) / 2;
		mid.cy = (offset.cy + off2.cy) / 2;
		if (offset.cy > r
		    && off2.cy > r
		    && sqr(mid.cx - r) + sqr(mid.cy - r) > sqr(r)
		    && sqr(off2.cx - r) + sqr(off2.cy - r) > sqr(r)
		    && sqr(offset.cx - r) + sqr(offset.cy - r) > sqr(r)) {
		    break;
		}

		for (i = 0; ; i++) {
		    if (World.treasures[i].pos.cx / BLOCK_CLICKS == block.x &&
			World.treasures[i].pos.cy / BLOCK_CLICKS == block.y) {
			break;
		    }
		}
		ms->treasure = i;
		ms->crash = CrashTreasure;

		/*
		 * We handle balls here, because the reaction
		 * depends on which team the treasure and the ball
		 * belong to.
		 */
		if (mi->obj->type != OBJ_BALL) {
		    return;
		}

		ball = BALL_PTR(mi->obj);
		if (ms->treasure == ball->treasure) {
		    /*
		     * Ball has been replaced back in the hoop from whence
		     * it came.  If the player is on the same team as the
		     * hoop, then it should be replaced into the hoop without
		     * exploding and gets the player some points.  Otherwise
		     * nothing interesting happens.
		     */
		    player	*pl = NULL;

		    if (ball->owner != NO_ID)
			pl = Players[GetInd[ball->owner]];

		    if (!BIT(World.rules->mode, TEAM_PLAY)
			|| !pl
			|| (pl->team !=
			    World.treasures[ball->treasure].team)) {
			ball->life = LONG_MAX;
			ms->crash = NotACrash;
			break;
		    }

		    Ball_is_replaced(ball);
		    break;
		}
		if (ball->owner == NO_ID) {
		    ball->life = 0;
		    return;
		}
		if (BIT(World.rules->mode, TEAM_PLAY)
		    && World.treasures[ms->treasure].team ==
		       Players[GetInd[ball->owner]]->team) {
		    Ball_is_destroyed(ball);
		    if (captureTheFlag
			&& !World.treasures[ms->treasure].have
			&& !World.treasures[ms->treasure].empty) {
			strcpy(msg, "Your treasure must be safe before you "
			       "can cash an opponent's!");
			Set_player_message(Players[GetInd[ball->owner]], msg);
		    } else if (Punish_team(GetInd[ball->owner],
					   ball->treasure,
					   ball->pos.cx, ball->pos.cy))
			CLR_BIT(ball->status, RECREATE);
		}
		ball->life = 0;
		return;
	    }
	}
	/*FALLTHROUGH*/

    case TARGET:
	if (block_type == TARGET) {
	    if (mi->target_crashes) {
		ms->target = i = Map_get_itemid(block.x, block.y);

		if (!targetTeamCollision) {
		    int team;
		    if (mi->pl) {
			team = mi->pl->team;
		    }
		    else if (BIT(mi->obj->type, OBJ_BALL)) {
			ballobject *ball = BALL_PTR(mi->obj);
			if (ball->owner != NO_ID) {
			    team = Players[GetInd[ball->owner]]->team;
			} else {
			    team = TEAM_NOT_SET;
			}
		    }
		    else {
			team = mi->obj->team;
		    }
		    if (team == World.targets[i].team) {
			break;
		    }
		}
		if (!mi->pl) {
		    ms->crash = CrashTarget;
		    return;
		}
	    }
	}
	/*FALLTHROUGH*/

    case FUEL:
    case FILLED:
	if (inside) {
	    /* Could happen for targets reappearing and in case of bugs. */
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.cx == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceLeft;
	    }
	}
	else if (offset.cx == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceRight;
	    }
	}
	if (offset.cy == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceDown;
	    }
	}
	else if (offset.cy == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (!(ms->todo.cx | ms->todo.cy)) {
	    /* no bouncing possible and no movement.  OK. */
	    break;
	}
	if (!ms->todo.cx && (offset.cx == 0 || offset.cx == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	if (!ms->todo.cy && (offset.cy == 0 || offset.cy == BLOCK_CLICKS)) {
	    /* tricky */
	    break;
	}
	/* what happened? we should never reach this */
	ms->crash = CrashWall;
	return;

    case REC_LD:
	/* test for bounces first. */
	if (offset.cx == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceLeft;
	    }
	    if (offset.cy == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (offset.cy == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceDown;
	    }
	    if (offset.cx == BLOCK_CLICKS && ms->vel.x + ms->vel.y < 0) {
		wall_bounce |= BounceLeftDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.cx + offset.cy < BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.cx + delta.cx + offset.cy + delta.cy >= BLOCK_CLICKS) {
	    /* movement is entirely within the space part of the block. */
	    break;
	}
	/*
	 * Find out where we bounce exactly
	 * and how far we can move before bouncing.
	 */
	if (sign.x * ms->todo.cx >= sign.y * ms->todo.cy) {
	    double w = (double) ms->todo.cy / ms->todo.cx;
	    delta.cx = (int)((BLOCK_CLICKS - offset.cx - offset.cy) / (1 + w));
	    delta.cy = (int)(delta.cx * w);
	    if (offset.cx + delta.cx + offset.cy + delta.cy < BLOCK_CLICKS) {
		delta.cx++;
		delta.cy = (int)(delta.cx * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cx) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.cx / ms->todo.cy;
	    delta.cy = (int)((BLOCK_CLICKS - offset.cx - offset.cy) / (1 + w));
	    delta.cx = (int)(delta.cy * w);
	    if (offset.cx + delta.cx + offset.cy + delta.cy < BLOCK_CLICKS) {
		delta.cy++;
		delta.cx = (int)(delta.cy * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cy) {
		wall_bounce |= BounceLeftDown;
		break;
	    }
	}
	break;

    case REC_LU:
	if (offset.cx == 0) {
	    if (ms->vel.x > 0) {
		wall_bounce |= BounceLeft;
	    }
	    if (offset.cy == 0 && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (offset.cy == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceUp;
	    }
	    if (offset.cx == BLOCK_CLICKS && ms->vel.x < ms->vel.y) {
		wall_bounce |= BounceLeftUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.cx < offset.cy) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.cx + delta.cx >= offset.cy + delta.cy) {
	    break;
	}
	if (sign.x * ms->todo.cx >= sign.y * ms->todo.cy) {
	    double w = (double) ms->todo.cy / ms->todo.cx;
	    delta.cx = (int)((offset.cy - offset.cx) / (1 - w));
	    delta.cy = (int)(delta.cx * w);
	    if (offset.cx + delta.cx < offset.cy + delta.cy) {
		delta.cx++;
		delta.cy = (int)(delta.cx * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cx) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.cx / ms->todo.cy;
	    delta.cy = (int)((offset.cx - offset.cy) / (1 - w));
	    delta.cx = (int)(delta.cy * w);
	    if (offset.cx + delta.cx < offset.cy + delta.cy) {
		delta.cy--;
		delta.cx = (int)(delta.cy * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cy) {
		wall_bounce |= BounceLeftUp;
		break;
	    }
	}
	break;

    case REC_RD:
	if (offset.cx == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceRight;
	    }
	    if (offset.cy == BLOCK_CLICKS && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (offset.cy == 0) {
	    if (ms->vel.y > 0) {
		wall_bounce |= BounceDown;
	    }
	    if (offset.cx == 0 && ms->vel.x > ms->vel.y) {
		wall_bounce |= BounceRightDown;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.cx > offset.cy) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.cx + delta.cx <= offset.cy + delta.cy) {
	    break;
	}
	if (sign.x * ms->todo.cx >= sign.y * ms->todo.cy) {
	    double w = (double) ms->todo.cy / ms->todo.cx;
	    delta.cx = (int)((offset.cy - offset.cx) / (1 - w));
	    delta.cy = (int)(delta.cx * w);
	    if (offset.cx + delta.cx > offset.cy + delta.cy) {
		delta.cx--;
		delta.cy = (int)(delta.cx * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cx) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.cx / ms->todo.cy;
	    delta.cy = (int)((offset.cx - offset.cy) / (1 - w));
	    delta.cx = (int)(delta.cy * w);
	    if (offset.cx + delta.cx > offset.cy + delta.cy) {
		delta.cy++;
		delta.cx = (int)(delta.cy * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cy) {
		wall_bounce |= BounceRightDown;
		break;
	    }
	}
	break;

    case REC_RU:
	if (offset.cx == BLOCK_CLICKS) {
	    if (ms->vel.x < 0) {
		wall_bounce |= BounceRight;
	    }
	    if (offset.cy == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (offset.cy == BLOCK_CLICKS) {
	    if (ms->vel.y < 0) {
		wall_bounce |= BounceUp;
	    }
	    if (offset.cx == 0 && ms->vel.x + ms->vel.y > 0) {
		wall_bounce |= BounceRightUp;
	    }
	}
	if (wall_bounce) {
	    break;
	}
	if (offset.cx + offset.cy > BLOCK_CLICKS) {
	    ms->crash = CrashWall;
	    return;
	}
	if (offset.cx + delta.cx + offset.cy + delta.cy <= BLOCK_CLICKS) {
	    break;
	}
	if (sign.x * ms->todo.cx >= sign.y * ms->todo.cy) {
	    double w = (double) ms->todo.cy / ms->todo.cx;
	    delta.cx = (int)((BLOCK_CLICKS - offset.cx - offset.cy) / (1 + w));
	    delta.cy = (int)(delta.cx * w);
	    if (offset.cx + delta.cx + offset.cy + delta.cy > BLOCK_CLICKS) {
		delta.cx--;
		delta.cy = (int)(delta.cx * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cx) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	else {
	    double w = (double) ms->todo.cx / ms->todo.cy;
	    delta.cy = (int)((BLOCK_CLICKS - offset.cx - offset.cy) / (1 + w));
	    delta.cx = (int)(delta.cy * w);
	    if (offset.cx + delta.cx + offset.cy + delta.cy > BLOCK_CLICKS) {
		delta.cy--;
		delta.cx = (int)(delta.cy * w);
	    }
	    leave.cx = enter.cx + delta.cx;
	    leave.cy = enter.cy + delta.cy;
	    if (!delta.cy) {
		wall_bounce |= BounceRightUp;
		break;
	    }
	}
	break;
    }

    if (wall_bounce) {
	/*
	 * Bouncing.  As there may be more than one possible bounce
	 * test which bounce is not feasible because of adjacent walls.
	 * If there still is more than one possible then pick one randomly.
	 * Else if it turns out that none is feasible then we must have
	 * been trapped inbetween two blocks.  This happened in the early
	 * stages of this code.
	 */
	int count = 0;
	unsigned bit;
	unsigned save_wall_bounce = wall_bounce;
	unsigned block_mask = FILLED_BIT | FUEL_BIT;

	if (!mi->target_crashes) {
	    block_mask |= TARGET_BIT;
	}
	if (!mi->treasure_crashes) {
	    block_mask |= TREASURE_BIT;
	}
	for (bit = 1; bit <= wall_bounce; bit <<= 1) {
	    if (!(wall_bounce & bit)) {
		continue;
	    }

	    CLR_BIT(wall_bounce, bit);
	    switch (bit) {

	    case BounceLeft:
		blk2.x = block.x - 1;
		if (blk2.x < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x += World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_RD_BIT)) {
		    continue;
		}
		break;

	    case BounceRight:
		blk2.x = block.x + 1;
		if (blk2.x >= World.x) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.x -= World.x;
		}
		blk2.y = block.y;
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_LU_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;

	    case BounceDown:
		blk2.x = block.x;
		blk2.y = block.y - 1;
		if (blk2.y < 0) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y += World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RU_BIT|REC_LU_BIT)) {
		    continue;
		}
		break;

	    case BounceUp:
		blk2.x = block.x;
		blk2.y = block.y + 1;
		if (blk2.y >= World.y) {
		    if (!mi->edge_wrap) {
			continue;
		    }
		    blk2.y -= World.y;
		}
		if (BIT(1 << World.block[blk2.x][blk2.y],
			block_mask|REC_RD_BIT|REC_LD_BIT)) {
		    continue;
		}
		break;
	    }

	    SET_BIT(wall_bounce, bit);
	    count++;
	}

	if (!count) {
	    wall_bounce = save_wall_bounce;
	    switch (wall_bounce) {
	    case BounceLeft|BounceDown:
		wall_bounce = BounceLeftDown;
		break;
	    case BounceLeft|BounceUp:
		wall_bounce = BounceLeftUp;
		break;
	    case BounceRight|BounceDown:
		wall_bounce = BounceRightDown;
		break;
	    case BounceRight|BounceUp:
		wall_bounce = BounceRightUp;
		break;
	    default:
		switch (block_type) {
		case REC_LD:
		    if ((offset.cx == 0) ? (offset.cy == BLOCK_CLICKS)
			: (offset.cx == BLOCK_CLICKS && offset.cy == 0)
			&& ms->vel.x + ms->vel.y >= 0) {
			wall_bounce = 0;
		    }
		    break;
		case REC_LU:
		    if ((offset.cx == 0) ? (offset.cy == 0)
			: (offset.cx == BLOCK_CLICKS && offset.cy == BLOCK_CLICKS)
			&& ms->vel.x >= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RD:
		    if ((offset.cx == 0) ? (offset.cy == 0)
			: (offset.cx == BLOCK_CLICKS && offset.cy == BLOCK_CLICKS)
			&& ms->vel.x <= ms->vel.y) {
			wall_bounce = 0;
		    }
		    break;
		case REC_RU:
		    if ((offset.cx == 0) ? (offset.cy == BLOCK_CLICKS)
			: (offset.cx == BLOCK_CLICKS && offset.cy == 0)
			&& ms->vel.x + ms->vel.y <= 0) {
			wall_bounce = 0;
		    }
		    break;
		}
		if (wall_bounce) {
		    ms->crash = CrashWall;
		    return;
		}
	    }
	}
	else if (count > 1) {
	    /*
	     * More than one bounce possible.
	     * Pick one randomly.
	     */
	    count = (int)(rfrac() * count);
	    for (bit = 1; bit <= wall_bounce; bit <<= 1) {
		if (wall_bounce & bit) {
		    if (count == 0) {
			wall_bounce = bit;
			break;
		    } else {
			count--;
		    }
		}
	    }
	}
    }

    } /* phased */

    if (wall_bounce) {
	Bounce_wall(ms, (move_bounce_t) wall_bounce);
    }
    else {
	ms->done.cx += delta.cx;
	ms->done.cy += delta.cy;
	ms->todo.cx -= delta.cx;
	ms->todo.cy -= delta.cy;
    }
}

#if 0
void Cannon_dies(int ind, player *pl);

static void Cannon_dies_old(move_state_t *ms)
{
    int ind = ms->cannon;
    player *pl = NULL;

    if (!ms->mip->pl) {
	/*
	 * If it was not a player that crashed  into the cannon,
	 * look if it was something fired by a player.
	 */
	if (ms->mip->obj->id != NO_ID) {
	    pl = Players[GetInd[ms->mip->obj->id]];
	}
    } else if (BIT(ms->mip->pl->used, HAS_SHIELD|HAS_EMERGENCY_SHIELD)
	       == (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
	/* if player rammed the cannon with shields up, he gets a kill */
	pl = ms->mip->pl;
    }

    Cannon_dies(ind, pl);
}
#endif


void Object_hits_target(int ind, object *obj, long player_cost);

static void Object_hits_target_old(move_state_t *ms, long player_cost)
{
    Object_hits_target(ms->target, ms->mip->obj, player_cost);
}


void Object_crash(object *obj, struct move *move, int crashtype, int item_id);

static void Object_crash_old(move_state_t *ms)
{
    object *obj = ms->mip->obj;
    int crashtype = ms->crash;
    int item_id = -1;
    struct move move;

    switch (crashtype) {
    default:
	break;
    case CrashWormHole:
	item_id = ms->wormhole;
	break;
    case CrashCannon:
	item_id = ms->cannon;
	break;
    case CrashTarget:
	item_id = ms->target;
	break;
    }

    Object_crash(obj, &move, crashtype, item_id);
}

void Move_object_old(object *obj)
{
    int			nothing_done = 0;
    int			dist;
    move_info_t		mi;
    move_state_t	ms;
    bool		pos_update = false;

    Object_position_remember(obj);

    dist = walldist[OBJ_X_IN_BLOCKS(obj)][OBJ_Y_IN_BLOCKS(obj)];
    if (dist > 2) {
	int max = ((dist - 2) * BLOCK_SZ) >> 1;
	DFLOAT dcx = obj->vel.x * timeStep;
	DFLOAT dcy = obj->vel.y * timeStep;
	if (sqr(max) >= sqr(dcx) + sqr(dcy)) {
	    int cx = obj->pos.cx + FLOAT_TO_CLICK(dcx);
	    int cy = obj->pos.cy + FLOAT_TO_CLICK(dcy);
	    cx = WRAP_XCLICK(cx);
	    cy = WRAP_YCLICK(cy);
	    Object_position_set_clicks(obj, cx, cy);
	    Cell_add_object(obj);
	    return;
	}
    }

    mi.pl = NULL;
    mi.obj = obj;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = BIT(mp.obj_bounce_mask, obj->type);
    mi.cannon_crashes = BIT(mp.obj_cannon_mask, obj->type);
    mi.target_crashes = BIT(mp.obj_target_mask, obj->type);
    mi.treasure_crashes = BIT(mp.obj_treasure_mask, obj->type);
    mi.wormhole_warps = true;
    if (BIT(obj->type, OBJ_BALL) && obj->id != NO_ID) {
	mi.phased = BIT(Players[GetInd[obj->id]]->used, HAS_PHASING_DEVICE);
    } else {
	mi.phased = 0;
    }

    ms.pos.cx = obj->pos.cx;
    ms.pos.cy = obj->pos.cy;
    ms.vel = obj->vel;
    ms.todo.cx = FLOAT_TO_CLICK(ms.vel.x * timeStep);
    ms.todo.cy = FLOAT_TO_CLICK(ms.vel.y * timeStep);
    ms.dir = obj->missile_dir;
    ms.mip = &mi;

    for (;;) {
	Move_segment(&ms);
	if (!(ms.done.cx | ms.done.cy)) {
	    pos_update |= (ms.crash | ms.bounce);
	    if (ms.crash) {
		break;
	    }
	    if (ms.bounce && ms.bounce != BounceEdge) {
		if (obj->type != OBJ_BALL)
		    obj->life = (long)(obj->life * objectWallBounceLifeFactor);
		if (obj->life <= 0) {
		    break;
		}
		/*
		 * Any bouncing sparks are no longer owner immune to give
		 * "reactive" thrust.  This is exactly like ground effect
		 * in the real world.  Very useful for stopping against walls.
		 *
		 * If the FROMBOUNCE bit is set the spark was caused by
		 * the player bouncing of a wall and thus although the spark
		 * should bounce, it is not reactive thrust otherwise wall
		 * bouncing would cause acceleration of the player.
		 */
		if (!BIT(obj->status, FROMBOUNCE) && BIT(obj->type, OBJ_SPARK))
		    CLR_BIT(obj->status, OWNERIMMUNE);
		if (sqr(ms.vel.x) + sqr(ms.vel.y) > sqr(maxObjectWallBounceSpeed)) {
		    obj->life = 0;
		    break;
		}
		ms.vel.x *= objectWallBrakeFactor;
		ms.vel.y *= objectWallBrakeFactor;
		ms.todo.cx = (int)(ms.todo.cx * objectWallBrakeFactor);
		ms.todo.cy = (int)(ms.todo.cy * objectWallBrakeFactor);
	    }
	    if (++nothing_done >= 5) {
		ms.crash = CrashUnknown;
		break;
	    }
	} else {
	    ms.pos.cx += ms.done.cx;
	    ms.pos.cy += ms.done.cy;
	    nothing_done = 0;
	}
	if (!(ms.todo.cx | ms.todo.cy)) {
	    break;
	}
    }
    if (mi.edge_wrap) {
	if (ms.pos.cx < 0) {
	    ms.pos.cx += mp.click_width;
	}
	if (ms.pos.cx >= mp.click_width) {
	    ms.pos.cx -= mp.click_width;
	}
	if (ms.pos.cy < 0) {
	    ms.pos.cy += mp.click_height;
	}
	if (ms.pos.cy >= mp.click_height) {
	    ms.pos.cy -= mp.click_height;
	}
    }
    Object_position_set_clicks(obj, ms.pos.cx, ms.pos.cy);
    obj->vel = ms.vel;
    obj->missile_dir = ms.dir;
    if (ms.crash) {
	Object_crash_old(&ms);
    }
    if (pos_update) {
	Object_position_remember(obj);
    }
    Cell_add_object(obj);
}


void Player_crash(player *pl, struct move *move, int crashtype,
		  int item_id, int pt);

static void Player_crash_old(move_state_t *ms, int pt)
{
    player *pl = ms->mip->pl;
    int crashtype = ms->crash;
    int item_id = -1;
    struct move move;

    switch (crashtype) {
    default:
	break;
    case CrashWormHole:
	item_id = ms->wormhole;
	break;
    case CrashCannon:
	item_id = ms->cannon;
	break;
    case CrashTarget:
	item_id = ms->target;
	break;
    }

    Player_crash(pl, &move, crashtype, item_id, pt);
}



void Move_player_old(int ind)
{
    player		*pl = Players[ind];
    int			nothing_done = 0;
    int			i;
    int			dist;
    move_info_t		mi;
    move_state_t	ms[RES];
    int			worst = 0;
    int			crash;
    int			bounce;
    int			moves_made = 0;
    int			cor_res;
    clpos		pos, pos2;
    clvec		todo;
    clvec		done;
    vector		vel;
    vector		r[RES];
    ivec		sign;		/* sign (-1 or 1) of direction */
    ipos		block;		/* block index */
    bool		pos_update = false;
    DFLOAT		fric;
    DFLOAT		oldvx, oldvy;


    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	if (!BIT(pl->status, KILLED|PAUSE)) {
	    pos.cx = pl->pos.cx + FLOAT_TO_CLICK(pl->vel.x * timeStep);
	    pos.cy = pl->pos.cy + FLOAT_TO_CLICK(pl->vel.y * timeStep);
	    pos.cx = WRAP_XCLICK(pos.cx);
	    pos.cy = WRAP_YCLICK(pos.cy);
	    if (pos.cx != pl->pos.cx || pos.cy != pl->pos.cy) {
		Player_position_remember(pl);
		Player_position_set_clicks(pl, pos.cx, pos.cy);
	    }
	}
	pl->velocity = VECTOR_LENGTH(pl->vel);
	return;
    }

/* Figure out which friction to use. */
    if (BIT(pl->used, HAS_PHASING_DEVICE)) {
	fric = friction;
    }
    else {
	switch (World.block[OBJ_X_IN_BLOCKS(pl)][OBJ_Y_IN_BLOCKS(pl)]) {
	case FRICTION:
	    fric = blockFriction;
	    break;
	default:
	    fric = friction;
	    break;
	}
    }

    cor_res = MOD2(((int)coriolis) * RES / 360, RES);
    oldvx = pl->vel.x;
    oldvy = pl->vel.y;
    pl->vel.x = (1.0f - fric) * (oldvx * tcos(cor_res) + oldvy * tsin(cor_res));
    pl->vel.y = (1.0f - fric) * (oldvy * tcos(cor_res) - oldvx * tsin(cor_res));

    Player_position_remember(pl);

    dist = walldist[OBJ_X_IN_BLOCKS(pl)][OBJ_Y_IN_BLOCKS(pl)];
    if (dist > 3) {
	int max = ((dist - 3) * BLOCK_SZ) >> 1;
	if (max >= pl->velocity) {
	    pos.cx = pl->pos.cx + FLOAT_TO_CLICK(pl->vel.x * timeStep);
	    pos.cy = pl->pos.cy + FLOAT_TO_CLICK(pl->vel.y * timeStep);
	    pos.cx = WRAP_XCLICK(pos.cx);
	    pos.cy = WRAP_YCLICK(pos.cy);
	    Player_position_set_clicks(pl, pos.cx, pos.cy);
	    pl->velocity = VECTOR_LENGTH(pl->vel);
	    return;
	}
    }

    mi.pl = pl;
    mi.obj = (object *) pl;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = true;
    mi.cannon_crashes = true;
    mi.treasure_crashes = true;
    mi.target_crashes = true;
    mi.wormhole_warps = true;
    mi.phased = BIT(pl->used, HAS_PHASING_DEVICE);

    vel = pl->vel;
    todo.cx = FLOAT_TO_CLICK(vel.x * timeStep);
    todo.cy = FLOAT_TO_CLICK(vel.y * timeStep);
    for (i = 0; i < pl->ship->num_points; i++) {
	clpos pt = Ship_get_point_clpos(pl->ship, i, pl->dir);
	ms[i].pos.cx = pl->pos.cx + pt.cx;
	ms[i].pos.cy = pl->pos.cy + pt.cy;
	ms[i].vel = vel;
	ms[i].todo = todo;
	ms[i].dir = pl->dir;
	ms[i].mip = &mi;
	ms[i].target = -1;
    }

    for (;; moves_made++) {
	clpos pt;

	pt = Ship_get_point_clpos(pl->ship, 0, ms[0].dir);
	pos.cx = ms[0].pos.cx - pt.cx;
	pos.cy = ms[0].pos.cy - pt.cy;
	pos.cx = WRAP_XCLICK(pos.cx);
	pos.cy = WRAP_YCLICK(pos.cy);
	block.x = pos.cx / BLOCK_CLICKS;
	block.y = pos.cy / BLOCK_CLICKS;

	if (walldist[block.x][block.y] > 3) {
	    int maxcl = ((walldist[block.x][block.y] - 3) * BLOCK_CLICKS) >> 1;
	    todo = ms[0].todo;
	    sign.x = (todo.cx < 0) ? -1 : 1;
	    sign.y = (todo.cy < 0) ? -1 : 1;
	    if (maxcl >= sign.x * todo.cx && maxcl >= sign.y * todo.cy) {
		/* entire movement is possible. */
		done.cx = todo.cx;
		done.cy = todo.cy;
	    }
	    else if (sign.x * todo.cx > sign.y * todo.cy) {
		/* horizontal movement. */
		done.cx = sign.x * maxcl;
		done.cy = todo.cy * maxcl / (sign.x * todo.cx);
	    }
	    else {
		/* vertical movement. */
		done.cx = todo.cx * maxcl / (sign.y * todo.cy);
		done.cy = sign.y * maxcl;
	    }
	    todo.cx -= done.cx;
	    todo.cy -= done.cy;
	    for (i = 0; i < pl->ship->num_points; i++) {
		ms[i].pos.cx += done.cx;
		ms[i].pos.cy += done.cy;
		ms[i].todo = todo;
		ms[i].crash = NotACrash;
		ms[i].bounce = NotABounce;
		if (mi.edge_wrap) {
		    if (ms[i].pos.cx < 0) {
			ms[i].pos.cx += mp.click_width;
		    }
		    else if (ms[i].pos.cx >= mp.click_width) {
			ms[i].pos.cx -= mp.click_width;
		    }
		    if (ms[i].pos.cy < 0) {
			ms[i].pos.cy += mp.click_height;
		    }
		    else if (ms[i].pos.cy >= mp.click_height) {
			ms[i].pos.cy -= mp.click_height;
		    }
		}
	    }
	    nothing_done = 0;
	    if (!(todo.cx | todo.cy)) {
		break;
	    }
	    else {
		continue;
	    }
	}

	bounce = -1;
	crash = -1;
	for (i = 0; i < pl->ship->num_points; i++) {
	    Move_segment(&ms[i]);
	    pos_update |= (ms[i].crash | ms[i].bounce);
	    if (ms[i].crash) {
		crash = i;
		break;
	    }
	    if (ms[i].bounce) {
		if (bounce == -1) {
		    bounce = i;
		}
		else if (ms[bounce].bounce != BounceEdge
		    && ms[i].bounce == BounceEdge) {
		    bounce = i;
		}
		else if ((ms[bounce].bounce == BounceEdge)
		    == (ms[i].bounce == BounceEdge)) {
		    if ((int)(rfrac() * (pl->ship->num_points - bounce)) == i) {
			bounce = i;
		    }
		}
		worst = bounce;
	    }
	}
	if (crash != -1) {
	    worst = crash;
	    break;
	}
	else if (bounce != -1) {
	    worst = bounce;
	    pl->last_wall_touch = frame_loops;
	    if (ms[worst].bounce != BounceEdge) {
		DFLOAT	speed = VECTOR_LENGTH(ms[worst].vel);
		int	v = (int) speed >> 2;
		int	m = (int) (pl->mass - pl->emptymass * 0.75f);
		DFLOAT	b = 1 - 0.5f * playerWallBrakeFactor;
		long	cost = (long) (b * m * v);
		int	delta_dir,
			abs_delta_dir,
			wall_dir;
		DFLOAT	max_speed = BIT(pl->used, HAS_SHIELD)
				    ? maxShieldedWallBounceSpeed
				    : maxUnshieldedWallBounceSpeed;
		int	max_angle = BIT(pl->used, HAS_SHIELD)
				    ? mp.max_shielded_angle
				    : mp.max_unshielded_angle;

		if (BIT(pl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
		    == (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
		    if (max_speed < 100) {
			max_speed = 100;
		    }
		    max_angle = RES;
		}

		ms[worst].vel.x *= playerWallBrakeFactor;
		ms[worst].vel.y *= playerWallBrakeFactor;
		ms[worst].todo.cx = (int)(ms[worst].todo.cx * playerWallBrakeFactor);
		ms[worst].todo.cy = (int)(ms[worst].todo.cy * playerWallBrakeFactor);

		/* only use armor if neccessary */
		if (speed > max_speed
		    && max_speed < maxShieldedWallBounceSpeed
		    && !BIT(pl->used, HAS_SHIELD)
		    && BIT(pl->have, HAS_ARMOR)) {
		    max_speed = maxShieldedWallBounceSpeed;
		    max_angle = mp.max_shielded_angle;
		    Player_hit_armor(ind);
		}

		if (speed > max_speed) {
		    crash = worst;
		    ms[worst].crash = (ms[worst].target >= 0 ? CrashTarget :
				       CrashWallSpeed);
		    break;
		}

		switch (ms[worst].bounce) {
		case BounceLeft: wall_dir = 4*RES/8; break;
		case BounceRight: wall_dir = 0*RES/8; break;
		case BounceDown: wall_dir = 6*RES/8; break;
		default:
		case BounceUp: wall_dir = 2*RES/8; break;
		case BounceLeftDown: wall_dir = 1*RES/8; break;
		case BounceLeftUp: wall_dir = 7*RES/8; break;
		case BounceRightDown: wall_dir = 3*RES/8; break;
		case BounceRightUp: wall_dir = 5*RES/8; break;
		}
		if (pl->dir >= wall_dir) {
		    delta_dir = (pl->dir - wall_dir <= RES/2)
				? -(pl->dir - wall_dir)
				: (wall_dir + RES - pl->dir);
		} else {
		    delta_dir = (wall_dir - pl->dir <= RES/2)
				? (wall_dir - pl->dir)
				: -(pl->dir + RES - wall_dir);
		}
		abs_delta_dir = ABS(delta_dir);
		/* only use armor if neccessary */
		if (abs_delta_dir > max_angle
		    && max_angle < mp.max_shielded_angle
		    && !BIT(pl->used, HAS_SHIELD)
		    && BIT(pl->have, HAS_ARMOR)) {
		    max_speed = maxShieldedWallBounceSpeed;
		    max_angle = mp.max_shielded_angle;
		    Player_hit_armor(ind);
		}
		if (abs_delta_dir > max_angle) {
		    crash = worst;
		    ms[worst].crash = (ms[worst].target >= 0 ? CrashTarget :
				       CrashWallAngle);
		    break;
		}
		if (abs_delta_dir <= RES/16) {
		    pl->float_dir += (1.0f - playerWallBrakeFactor) * delta_dir;
		    if (pl->float_dir >= RES) {
			pl->float_dir -= RES;
		    }
		    else if (pl->float_dir < 0) {
			pl->float_dir += RES;
		    }
		}

		/*
		 * Small explosion and fuel loss if survived a hit on a wall.
		 * This doesn't affect the player as the explosion is sparks
		 * which don't collide with player.
		 * Clumsy touches (head first) with wall are more costly.
		 */
		cost = (cost * (RES/2 + abs_delta_dir)) / RES;
		if (BIT(pl->used, (HAS_SHIELD|HAS_EMERGENCY_SHIELD))
		    != (HAS_SHIELD|HAS_EMERGENCY_SHIELD)) {
		    Add_fuel(&pl->fuel, (long)(-((cost << FUEL_SCALE_BITS)
					  * wallBounceFuelDrainMult)));
		    Item_damage(ind, wallBounceDestroyItemProb);
		}
		if (!pl->fuel.sum && wallBounceFuelDrainMult != 0) {
		    crash = worst;
		    ms[worst].crash = (ms[worst].target >= 0 ? CrashTarget :
				       CrashWallNoFuel);
		    break;
		}
		if (cost) {
		    int intensity = (int)(cost * wallBounceExplosionMult);
		    Make_debris(
			/* pos.cx, pos.cy   */ pl->pos.cx, pl->pos.cy,
			/* vel.x, vel.y   */ pl->vel.x, pl->vel.y,
			/* owner id       */ pl->id,
			/* owner team	  */ pl->team,
			/* kind           */ OBJ_SPARK,
			/* mass           */ 3.5,
			/* status         */ GRAVITY | OWNERIMMUNE | FROMBOUNCE,
			/* color          */ RED,
			/* radius         */ 1,
			/* num debris     */ (intensity>>1) + ((intensity>>1) * rfrac()),
			/* min,max dir    */ wall_dir - (RES/4), wall_dir + (RES/4),
			/* min,max speed  */ 20, 20 + (intensity>>2),
			/* min,max life   */ 10, 10 + (intensity >> 1)
			);
		    sound_play_sensors(pl->pos.cx, pl->pos.cy,
				       PLAYER_BOUNCED_SOUND);
		    if (ms[worst].target >= 0) {
			cost <<= FUEL_SCALE_BITS;
			cost = (long)(cost * (wallBounceFuelDrainMult / 4.0));
			Object_hits_target_old(&ms[worst], cost);
		    }
		}
	    }
	}
	else {
	    for (i = 0; i < pl->ship->num_points; i++) {
		r[i].x = (vel.x) ? (DFLOAT) ms[i].todo.cx / vel.x : 0;
		r[i].y = (vel.y) ? (DFLOAT) ms[i].todo.cy / vel.y : 0;
		r[i].x = ABS(r[i].x);
		r[i].y = ABS(r[i].y);
	    }
	    worst = 0;
	    for (i = 1; i < pl->ship->num_points; i++) {
		if (r[i].x > r[worst].x || r[i].y > r[worst].y) {
		    worst = i;
		}
	    }
	}

	if (!(ms[worst].done.cx | ms[worst].done.cy)) {
	    if (++nothing_done >= 5) {
		ms[worst].crash = CrashUnknown;
		break;
	    }
	} else {
	    nothing_done = 0;
	    ms[worst].pos.cx += ms[worst].done.cx;
	    ms[worst].pos.cy += ms[worst].done.cy;
	}
	if (!(ms[worst].todo.cx | ms[worst].todo.cy)) {
	    break;
	}

	vel = ms[worst].vel;
	for (i = 0; i < pl->ship->num_points; i++) {
	    if (i != worst) {
		ms[i].pos.cx += ms[worst].done.cx;
		ms[i].pos.cy += ms[worst].done.cy;
		ms[i].vel = vel;
		ms[i].todo = ms[worst].todo;
		ms[i].dir = ms[worst].dir;
	    }
	}
    }

    pos2 = Ship_get_point_clpos(pl->ship, worst, pl->dir);
    pos.cx = ms[worst].pos.cx - pos2.cx;
    pos.cy = ms[worst].pos.cy - pos2.cy;
    pos.cx = WRAP_XCLICK(pos.cx);
    pos.cy = WRAP_YCLICK(pos.cy);
    Player_position_set_clicks(pl, pos.cx, pos.cy);
    pl->vel = ms[worst].vel;
    pl->velocity = VECTOR_LENGTH(pl->vel);

    if (ms[worst].crash) {
	Player_crash_old(&ms[worst], worst /*, false*/);
    }
    if (pos_update) {
	Player_position_remember(pl);
    }
}

void Turn_player_old(int ind)
{
    player		*pl = Players[ind];
    int			i;
    move_info_t		mi;
    move_state_t	ms[RES];
    int			dir;
    int			new_dir = MOD2((int)(pl->float_dir + 0.5f), RES);
    int			sign;
    int			crash = -1;
    int			nothing_done = 0;
    int			turns_done = 0;
    int			blocked = 0;
    clpos		pos;
    vector		salt;

    if (new_dir == pl->dir) {
	return;
    }
    if (BIT(pl->status, PLAYING|PAUSE|GAME_OVER|KILLED) != PLAYING) {
	pl->dir = new_dir;
	return;
    }

    if (walldist[OBJ_X_IN_BLOCKS(pl)][OBJ_Y_IN_BLOCKS(pl)] > 2) {
	pl->dir = new_dir;
	return;
    }

    mi.pl = pl;
    mi.obj = (object *) pl;
    mi.edge_wrap = BIT(World.rules->mode, WRAP_PLAY);
    mi.edge_bounce = edgeBounce;
    mi.wall_bounce = true;
    mi.cannon_crashes = true;
    mi.treasure_crashes = true;
    mi.target_crashes = true;
    mi.wormhole_warps = false;
    mi.phased = BIT(pl->used, HAS_PHASING_DEVICE);

    if (new_dir > pl->dir) {
	sign = (new_dir - pl->dir <= RES + pl->dir - new_dir) ? 1 : -1;
    }
    else {
	sign = (pl->dir - new_dir <= RES + new_dir - pl->dir) ? -1 : 1;
    }

#if 0
    salt.x = (pl->vel.x > 0) ? 0.1f : (pl->vel.x < 0) ? -0.1f : 0;
    salt.y = (pl->vel.y > 0) ? 0.1f : (pl->vel.y < 0) ? -0.1f : 0;
#else
    salt.x = (pl->vel.x > 0) ? 1e-6f : (pl->vel.x < 0) ? -1e-6f : 0;
    salt.y = (pl->vel.y > 0) ? 1e-6f : (pl->vel.y < 0) ? -1e-6f : 0;
#endif

    pos.cx = pl->pos.cx;
    pos.cy = pl->pos.cy;
    for (; pl->dir != new_dir; turns_done++) {
	dir = MOD2(pl->dir + sign, RES);
	if (!mi.edge_wrap) {
	    clpos pt;
	    if (pos.cx <= 22 * CLICK) {
		for (i = 0; i < pl->ship->num_points; i++) {
		    pt = Ship_get_point_clpos(pl->ship, i, dir);
		    if (pos.cx + pt.cx < 0) {
			pos.cx = -pt.cx;
		    }
		}
	    }
	    if (pos.cx >= mp.click_width - 22 * CLICK) {
		for (i = 0; i < pl->ship->num_points; i++) {
		    pt = Ship_get_point_clpos(pl->ship, i, dir);
		    if (pos.cx + pt.cx >= mp.click_width) {
			pos.cx = mp.click_width - 1 - pt.cx;
		    }
		}
	    }
	    if (pos.cy <= 22 * CLICK) {
		for (i = 0; i < pl->ship->num_points; i++) {
		    pt = Ship_get_point_clpos(pl->ship, i, dir);
		    if (pos.cy + pt.cy < 0) {
			pos.cy = -pt.cy;
		    }
		}
	    }
	    if (pos.cy >= mp.click_height - 22 * CLICK) {
		for (i = 0; i < pl->ship->num_points; i++) {
		    pt = Ship_get_point_clpos(pl->ship, i, dir);
		    if (pos.cy + pt.cy >= mp.click_height) {
			pos.cy = mp.click_height - 1 - pt.cy;
		    }
		}
	    }
	    if (pos.cx != pl->pos.cx || pos.cy != pl->pos.cy) {
		Player_position_set_clicks(pl, pos.cx, pos.cy);
	    }
	}

	for (i = 0; i < pl->ship->num_points; i++) {
	    clpos pt;

	    pt = Ship_get_point_clpos(pl->ship, i, dir);
	    ms[i].mip = &mi;
	    ms[i].pos.cx = pos.cx + pt.cx;
	    ms[i].pos.cy = pos.cy + pt.cy;
	    ms[i].todo.cx = pos.cx + pt.cx - ms[i].pos.cx;
	    ms[i].todo.cy = pos.cy + pt.cy - ms[i].pos.cy;
	    ms[i].vel.x = ms[i].todo.cx + salt.x;
	    ms[i].vel.y = ms[i].todo.cy + salt.y;
	    ms[i].target = -1;

	    do {
		Move_segment(&ms[i]);
		if (ms[i].crash | ms[i].bounce) {
		    if (ms[i].crash) {
			if (ms[i].crash != CrashUniverse) {
			    crash = i;
			}
			blocked = 1;
			break;
		    }
		    if (ms[i].bounce != BounceEdge) {
			blocked = 1;
			break;
		    }
		    if (++nothing_done >= 5) {
			ms[i].crash = CrashUnknown;
			crash = i;
			blocked = 1;
			break;
		    }
		}
		else if (ms[i].done.cx | ms[i].done.cy) {
		    ms[i].pos.cx += ms[i].done.cx;
		    ms[i].pos.cy += ms[i].done.cy;
		    nothing_done = 0;
		}
	    } while (ms[i].todo.cx | ms[i].todo.cy);
	    if (blocked) {
		break;
	    }
	}
	if (blocked) {
	    break;
	}
	pl->dir = dir;
    }

    if (blocked) {
	if (!maraTurnqueue)
	    pl->float_dir = (DFLOAT) pl->dir;
	pl->last_wall_touch = frame_loops;
    }

    if (crash != -1) {
	Player_crash_old(&ms[crash], crash /*, true*/);
    }

}

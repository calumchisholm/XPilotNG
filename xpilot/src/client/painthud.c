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
#include <sys/types.h>

#ifndef _WINDOWS
# include <unistd.h>
# include <X11/Xlib.h>
# include <X11/Xos.h>
#endif

#ifdef _WINDOWS
# include "NT/winX.h"
# include "NT/winClient.h"
#endif

#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "bit.h"
#include "types.h"
#include "keys.h"
#include "rules.h"
#include "setup.h"
#include "texture.h"
#include "paint.h"
#include "paintdata.h"
#include "paintmacros.h"
#include "record.h"
#include "xinit.h"
#include "protoclient.h"
#include "bitmaps.h"
#include "commonproto.h"
#include "clientrank.h"

char painthud_version[] = VERSION;


extern setup_t		*Setup;
extern int		RadarHeight;
extern score_object_t	score_objects[MAX_SCORE_OBJECTS];
extern int		score_object;
extern XGCValues	gcv;

int	hudColor;		/* Color index for HUD drawing */
int	hudHLineColor;		/* Color index for horiz. HUD line drawing */
int	hudVLineColor;		/* Color index for vert. HUD line drawing */
int	hudItemsColor;		/* Color index for HUD items drawing */
int	hudRadarEnemyColor;	/* Color index for enemy hudradar dots */
int	hudRadarOtherColor;	/* Color index for other hudradar dots */
int	hudRadarDotSize;	/* Size for hudradar dot drawing */
DFLOAT	hudRadarScale;		/* Scale for hudradar drawing */
DFLOAT	hudRadarMapScale;		/* Scale for mapradar drawing */
DFLOAT	hudRadarLimit;		/* Hudradar dots are not drawn if closer to
				   your ship than this factor of visible
				   range */
int	hudSize;		/* Size for HUD drawing */
int	hudLockColor;		/* Color index for lock on HUD drawing */
int	fuelGaugeColor;		/* Color index for fuel gauge drawing */
int	dirPtrColor;		/* Color index for dirptr drawing */
int	msgScanBallColor;	/* Color index for ball message scan drawing */
int	msgScanCoverColor;	/* Color index for ball message scan drawing */
int	messagesColor;		/* Color index for messages */
int	oldMessagesColor;	/* Color index for old messages */
DFLOAT	scoreObjectTime;	/* How long to flash score objects */
int	baseWarningType;	/* Which type of base warning you prefer */
int	baseWarningFrames;	/* Duration of base warning */

radar_t	*old_radar_ptr;
int	old_num_radar, old_max_radar;

static const int meterColor1 = RED;  /* Color index for meter drawing */
static const int meterColor2 = BLUE; /* Color index for meter border drawing */
static const int meterWidth = 60;
static const int meterHeight = 10;

extern bool ball_shout;
extern bool need_cover;

message_t	*TalkMsg[MAX_MSGS], *GameMsg[MAX_MSGS];
/* store incoming messages while a cut is pending */
message_t	*TalkMsg_pending[MAX_MSGS], *GameMsg_pending[MAX_MSGS];
/* history of the talk window */
char		*HistoryMsg[MAX_HIST_MSGS];

#ifndef _WINDOWS
/* selection in talk- or draw-window */
extern selection_t selection;
extern void Delete_pending_messages(void);
#endif



/*
 * Draw a meter of some kind on screen.
 * When the x-offset is specified as a negative value then
 * the meter is drawn relative to the right side of the screen,
 * otherwise from the normal left side.
 */
static void Paint_meter(int xoff, int y, const char *title, int val, int max)
{
    const int	mw1_4 = meterWidth/4,
		mw2_4 = meterWidth/2,
		mw3_4 = 3*meterWidth/4,
		mw4_4 = meterWidth,
		BORDER = 5;
    int		x, xstr;

    if (xoff >= 0) {
	x = xoff;
        xstr = WINSCALE(x + meterWidth) + BORDER;
    } else {
	x = ext_view_width - (meterWidth - xoff);
        xstr = WINSCALE(x) - (BORDER + XTextWidth(gameFont, title, strlen(title)));
    }
    if (1 || !texturedObjects) {
	Rectangle_add(meterColor1,
		      x+2, y+2,
		      (int)(((meterWidth-3)*val)/(max?max:1)), meterHeight-3);
	SET_FG(colors[meterColor2].pixel);
	rd.drawRectangle(dpy, p_draw, gc,
		       WINSCALE(x), WINSCALE(y),
		       WINSCALE(meterWidth), WINSCALE(meterHeight));
	Erase_4point(WINSCALE(x), WINSCALE(y),
		     WINSCALE(meterWidth), WINSCALE(meterHeight));

	/* Paint scale levels(?) */
	Segment_add(meterColor2, x,       y-4,	x,       y+meterHeight+4);
	Segment_add(meterColor2, x+mw4_4, y-4,	x+mw4_4, y+meterHeight+4);
	Segment_add(meterColor2, x+mw2_4, y-3,	x+mw2_4, y+meterHeight+3);
	Segment_add(meterColor2, x+mw1_4, y-1,	x+mw1_4, y+meterHeight+1);
	Segment_add(meterColor2, x+mw3_4, y-1,	x+mw3_4, y+meterHeight+1);
    } else {
	/*int width = WINSCALE((int)(((meterWidth-3)*val)/(max?max:1)));*/

	printf("TODO: implement paint meter\n");
	/*PaintMeter(p_draw, BM_METER,
	  WINSCALE(x), WINSCALE(y),
	  WINSCALE(meterWidth), WINSCALE(11),
	  width);*/
        SET_FG(colors[meterColor2].pixel);
    }

    rd.drawString(dpy, p_draw, gc,
                  (xstr), WINSCALE(y)+(gameFont->ascent+meterHeight)/2,
		  title, strlen(title));
    Erase_rectangle(xstr,
                    WINSCALE(y)+(gameFont->ascent+meterHeight)/2
                         - gameFont->ascent - 1,
		    XTextWidth(gameFont, title, strlen(title)) + 2,
		    gameFont->ascent + gameFont->descent + 1);
}


static int wrap(int *xp, int *yp)
{
    int			x = *xp, y = *yp;

    if (x < world.x || x > world.x + ext_view_width) {
	if (x < realWorld.x || x > realWorld.x + ext_view_width) {
	    return 0;
	}
	*xp += world.x - realWorld.x;
    }
    if (y < world.y || y > world.y + ext_view_height) {
	if (y < realWorld.y || y > realWorld.y + ext_view_height) {
	    return 0;
	}
	*yp += world.y - realWorld.y;
    }
    return 1;
}


void Paint_score_objects(void)
{
    int		i, x, y;

    if (!scoreObjectColor)
	return;

    for (i = 0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj = &score_objects[i];
	if (sobj->life_time > 0) {
	    if (loopsSlow % 3) {
		x = sobj->x * BLOCK_SZ + BLOCK_SZ/2;
		y = sobj->y * BLOCK_SZ + BLOCK_SZ/2;
		if (wrap(&x, &y)) {
		    SET_FG(colors[scoreObjectColor].pixel);
		    x = WINSCALE(X(x)) - sobj->msg_width / 2,
		    y = WINSCALE(Y(y)) + gameFont->ascent / 2,
		    rd.drawString(dpy, p_draw, gc,
				x, y,
				sobj->msg,
				sobj->msg_len);
		    Erase_rectangle(x - 1, y - gameFont->ascent,
				    sobj->msg_width + 2,
				    gameFont->ascent + gameFont->descent);
		}
	    }
	    sobj->life_time -= timePerFrame;
	    if (sobj->life_time <= 0.0) {
		sobj->life_time = 0.0;
		sobj->hud_msg_len = 0;
	    }
	}
    }
}


void Paint_meters(void)
{
    int y = 20;

    if (BIT(instruments, SHOW_FUEL_METER))
	Paint_meter(-10, y += 20, "Fuel", (int)fuelSum, (int)fuelMax);
    if (BIT(instruments, SHOW_POWER_METER) || control_count)
	Paint_meter(-10, y += 20, "Power", (int)displayedPower, (int)MAX_PLAYER_POWER);
    if (BIT(instruments, SHOW_TURNSPEED_METER) || control_count)
	Paint_meter(-10, y += 20, "Turnspeed",
		    (int)displayedTurnspeed, (int)MAX_PLAYER_TURNSPEED);
    if (control_count > 0)
	control_count--;
    if (BIT(instruments, SHOW_PACKET_SIZE_METER))
	Paint_meter(-10, y += 20, "Packet",
		   (packet_size >= 4096) ? 4096 : packet_size, 4096);
    if (BIT(instruments, SHOW_PACKET_LOSS_METER))
	Paint_meter(-10, y += 20, "Loss", packet_loss, FPS);
    if (BIT(instruments, SHOW_PACKET_DROP_METER))
	Paint_meter(-10, y += 20, "Drop", packet_drop, FPS);
    if (BIT(instruments, SHOW_PACKET_LAG_METER))
	Paint_meter(-10, y += 20, "Lag", MIN(packet_lag, 1 * FPS), 1 * FPS);

    if (thrusttime >= 0 && thrusttimemax > 0)
	Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3,
		    "Thrust Left",
		    (thrusttime >= thrusttimemax ? thrusttimemax : thrusttime),
		    thrusttimemax);

    if (shieldtime >= 0 && shieldtimemax > 0)
	Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 20,
		    "Shields Left",
		    (shieldtime >= shieldtimemax ? shieldtimemax : shieldtime),
		    shieldtimemax);

    if (phasingtime >= 0 && phasingtimemax > 0)
	Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 40,
		    "Phasing left",
		    (phasingtime >= phasingtimemax ? phasingtimemax : phasingtime),
		    phasingtimemax);

    if (destruct > 0)
	Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 60,
		   "Self destructing", destruct, 150);

    if (shutdown_count >= 0)
	Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 80,
		   "SHUTDOWN", shutdown_count, shutdown_delay);
}


static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
    const int	BORDER = 2;
    int		x, y;
    other_t	*target;
    char	str[50];
    static int	mapdiag = 0;

    if (mapdiag == 0)
	mapdiag = (int)LENGTH(Setup->width, Setup->height);

    /*
     * Display direction arrow and miscellaneous target information.
     */
    if ((target = Other_by_id(lock_id)) == NULL)
	return;

    FIND_NAME_WIDTH(target);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(hud_pos_x) - target->name_width / 2,
		WINSCALE(hud_pos_y - hudSize + HUD_OFFSET - BORDER )
			- gameFont->descent ,
		target->id_string, target->name_len);
    Erase_rectangle(WINSCALE(hud_pos_x) - target->name_width / 2 - 1,
		    WINSCALE(hud_pos_y - hudSize + HUD_OFFSET - BORDER )
			- gameFont->descent - gameFont->ascent ,
		    target->name_width + 2,
		    gameFont->ascent + gameFont->descent);

    /* lives left is a better info than distance in team games MM */
    if (BIT(Setup->mode, LIMITED_LIVES)) {
	sprintf(str, "%03d", target->life);
    } else {
	sprintf(str, "%03d", lock_dist / BLOCK_SZ);
    }

    if (BIT(Setup->mode, LIMITED_LIVES) || lock_dist !=0) {

 	if (BIT(Setup->mode, LIMITED_LIVES) && target->life == 0)
	    SET_FG(colors[RED].pixel);
	else
	    SET_FG(colors[hudColor].pixel);

	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x + hudSize - HUD_OFFSET + BORDER),
		    WINSCALE(hud_pos_y - hudSize + HUD_OFFSET - BORDER)
					 - gameFont->descent,
		    str, 3);
	Erase_rectangle(WINSCALE(hud_pos_x + hudSize - HUD_OFFSET
			 + BORDER) - 1,
			WINSCALE(hud_pos_y - hudSize + HUD_OFFSET - BORDER )
			 - gameFont->descent - gameFont->ascent ,
			XTextWidth(gameFont, str, 3) + 2,
			gameFont->ascent + gameFont->descent);
    }
    SET_FG(colors[hudColor].pixel);

    if (lock_dist != 0 && hudLockColor) {

	if (lock_dist > WARNING_DISTANCE || (loopsSlow & 1)) {
	    int size = MIN(mapdiag / lock_dist, 10);

	    if (size == 0) {
		size = 1;
	    }
	    if (self != NULL
		&& ((self->team == target->team && BIT(Setup->mode, TEAM_PLAY))
		|| (self->alliance != ' ' && self->alliance == target->alliance))) {
		Arc_add(hudColor,
			(int)(hud_pos_x + MIN_HUD_SIZE * 0.6 * tcos(lock_dir)
			      - size * 0.5),
			(int)(hud_pos_y - MIN_HUD_SIZE * 0.6 * tsin(lock_dir)
			      - size * 0.5),
			size, size, 0, 64*360);
	    } else {
		SET_FG(colors[hudLockColor].pixel);
		x = (int)(hud_pos_x + MIN_HUD_SIZE * 0.6 * tcos(lock_dir)
			  - size * 0.5),
		y = (int)(hud_pos_y - MIN_HUD_SIZE * 0.6 * tsin(lock_dir)
			  - size * 0.5),
		rd.fillArc(dpy, p_draw, gc,
			 WINSCALE(x), WINSCALE(y),
			 WINSCALE(size), WINSCALE(size), 0, 64*360);
		Erase_rectangle(WINSCALE(x), WINSCALE(y),
				 WINSCALE(size), WINSCALE(size));
		SET_FG(colors[hudColor].pixel);
	    }
	}
    }
}

void Paint_hudradar(DFLOAT hrscale, DFLOAT xlimit, DFLOAT ylimit, int sz)
{
    int i, x, y;
    int hrw = hrscale * 256;
    int hrh = hrscale * RadarHeight;
    DFLOAT xf = (DFLOAT) hrw / (DFLOAT) Setup->width;
    DFLOAT yf = (DFLOAT) hrh / (DFLOAT) Setup->height;

    for (i = 0; i < num_radar; i++) {
	x = radar_ptr[i].x * hrscale
	    - (world.x + ext_view_width / 2) * xf;
	y = radar_ptr[i].y * hrscale
	    - (world.y + ext_view_height / 2) * yf;

	if (x < -hrw / 2)
	    x += hrw;
	else if (x > hrw / 2)
	    x -= hrw;

	if (y < -hrh / 2)
	    y += hrh;
	else if (y > hrh / 2)
	    y -= hrh;

	if (!((x <= xlimit) && (x >= -xlimit)
	      && (y <= ylimit) && (y >= -ylimit))) {

 	    x = x + ext_view_width / 2 - sz / 2;
 	    y = -y + ext_view_height / 2 - sz / 2;

	    if (radar_ptr[i].color == WHITE) {
		if (hudRadarEnemyColor >= 1)
		    Arc_add(hudRadarEnemyColor, x, y, sz, sz, 0, 64 * 360);
	    } else {
		if (hudRadarOtherColor >= 1)
		    Arc_add(hudRadarOtherColor, x, y, sz, sz, 0, 64 * 360);
	    }
	}
    }
}


void Paint_HUD_items(int hud_pos_x, int hud_pos_y)
{
    const int		BORDER = 3;
    char		str[50];
    int			vert_pos, horiz_pos;
    int			i, j, maxWidth = -1,
			rect_x, rect_y, rect_width, rect_height;
    static int		vertSpacing = -1;

    SET_FG(colors[hudItemsColor].pixel);

    /* Special itemtypes */
    if (vertSpacing < 0)
	vertSpacing = MAX(ITEM_SIZE, gameFont->ascent + gameFont->descent) + 1;
    /* find the scaled location, then work in pixels */
    vert_pos = WINSCALE(hud_pos_y - hudSize+HUD_OFFSET + BORDER);
    horiz_pos = WINSCALE(hud_pos_x - hudSize+HUD_OFFSET - BORDER);
    rect_width = 0;
    rect_height = 0;
    rect_x = horiz_pos;
    rect_y = vert_pos;

    for (i = 0; i < NUM_ITEMS; i++) {
	int num = numItems[i];

	if (i == ITEM_FUEL)
	    continue;

	if (num != lastNumItems[i]) {
	    numItemsTime[i] = (int)(showItemsTime * (float)FPS);
	    lastNumItems[i] = num;
	}
	if (numItemsTime[i]-- <= 0) {
	    numItemsTime[i] = 0;
	    num = -1;
	}

	if (num >= 0) {
	    int len, width;

	    /* Paint item symbol */
	    Paint_item_symbol((u_byte)i, p_draw, gc,
			horiz_pos - ITEM_SIZE,
			vert_pos,
			ITEM_HUD);

	    if (i == lose_item) {
		if (lose_item_active != 0) {
		    if (lose_item_active < 0) {
			lose_item_active++;
		    }
		    rd.drawRectangle(dpy, p_draw, gc,
				horiz_pos-ITEM_SIZE-2,
				vert_pos-2, ITEM_SIZE+2, ITEM_SIZE+2);
		}
	    }

	    /* Paint item count */
	    sprintf(str, "%d", num);
	    len = strlen(str);
	    width = XTextWidth(gameFont, str, len);
	    rd.drawString(dpy, p_draw, gc,
			horiz_pos - ITEM_SIZE - BORDER - width,
			vert_pos + ITEM_SIZE/2 + gameFont->ascent/2,
			str, len);

	    maxWidth = MAX(maxWidth, width + BORDER + ITEM_SIZE);
	    vert_pos += vertSpacing;

	    if (vert_pos+vertSpacing > WINSCALE(hud_pos_y+hudSize-HUD_OFFSET-BORDER)) {
		rect_width += maxWidth + 2*BORDER;
		rect_height = MAX(rect_height, vert_pos - rect_y);
		horiz_pos -= maxWidth + 2*BORDER;
		vert_pos = WINSCALE(hud_pos_y - hudSize+HUD_OFFSET + BORDER);
		maxWidth = -1;
	    }
	}
    }
    if (maxWidth != -1) {
	rect_width += maxWidth + BORDER;
    }
    if (rect_width > 0) {
	if (rect_height == 0) {
	    rect_height = vert_pos - rect_y;
	}
	rect_x -= rect_width;
	Erase_rectangle(rect_x - 1, rect_y - 4,
			rect_width + 2, rect_height + 8);
    }

}

void Paint_HUD(void)
{
    const int		BORDER = 3;
    char		str[50];
    int			hud_pos_x, hud_pos_y, size;
    int			did_fuel = 0;
    int			i, j, modlen = 0;
    static char		autopilot[] = "Autopilot";

    /*
     * Show speed pointer
     */
    if (ptr_move_fact != 0.0
	&& selfVisible
	&& (vel.x != 0 || vel.y != 0)) {
	Segment_add(hudColor,
		    ext_view_width / 2,
		    ext_view_height / 2,
		    (int)(ext_view_width / 2 - ptr_move_fact*vel.x),
		    (int)(ext_view_height / 2 + ptr_move_fact*vel.y));
    }

    if (selfVisible && dirPtrColor) {
	Segment_add(dirPtrColor,
		    (int) (ext_view_width / 2 +
			   (100 - 15) * tcos(heading)),
		    (int) (ext_view_height / 2 -
			   (100 - 15) * tsin(heading)),
		    (int) (ext_view_width / 2 + 100 * tcos(heading)),
		    (int) (ext_view_height / 2 - 100 * tsin(heading)));
    }

    if (hudRadarEnemyColor || hudRadarOtherColor) {
	hudRadarMapScale = (DFLOAT) Setup->width / (DFLOAT) 256;
	Paint_hudradar(
	    hudRadarScale,
	    (int)(hudRadarLimit * (active_view_width / 2)
		  * hudRadarScale / hudRadarMapScale),
	    (int)(hudRadarLimit * (active_view_width / 2)
		  * hudRadarScale / hudRadarMapScale),
	    hudRadarDotSize);

	if (BIT(hackedInstruments, MAP_RADAR))
	    Paint_hudradar(hudRadarMapScale,
			   active_view_width / 2,
			   active_view_height / 2,
			   SHIP_SZ);
    }

    /* message scan hack by mara*/
    if (BIT(hackedInstruments, BALL_MSG_SCAN)) {
	if (ball_shout) {
	    Arc_add(msgScanBallColor, ext_view_width / 2 - 5,
		    ext_view_height / 2 - 5, 10, 10, 0, 64 * 360);
	}
	if (need_cover) {
	    Arc_add(msgScanCoverColor, ext_view_width / 2 - 4,
		    ext_view_height / 2 - 4, 8, 8, 0, 64 * 360);
	}
    }

    /* kps - remove */
    if (!hudColor) {
	return;
    }

    /*
     * Display the HUD
     */
    hud_pos_x = (int)(ext_view_width / 2 - hud_move_fact*vel.x);
    hud_pos_y = (int)(ext_view_height / 2 + hud_move_fact*vel.y);

    /* HUD frame */
    gcv.line_style = LineOnOffDash;
    XChangeGC(dpy, gc, GCLineStyle | GCDashOffset, &gcv);

    if (hudHLineColor) {
	SET_FG(colors[hudHLineColor].pixel);
	rd.drawLine(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x - hudSize),
		    WINSCALE(hud_pos_y - hudSize + HUD_OFFSET),
		    WINSCALE(hud_pos_x + hudSize),
		    WINSCALE(hud_pos_y - hudSize + HUD_OFFSET));
	Erase_segment(0,
		      WINSCALE(hud_pos_x - hudSize),
		      WINSCALE(hud_pos_y - hudSize + HUD_OFFSET),
		      WINSCALE(hud_pos_x + hudSize),
		      WINSCALE(hud_pos_y - hudSize + HUD_OFFSET));
	rd.drawLine(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x - hudSize),
		    WINSCALE(hud_pos_y + hudSize - HUD_OFFSET),
		    WINSCALE(hud_pos_x + hudSize),
		    WINSCALE(hud_pos_y + hudSize - HUD_OFFSET));
	Erase_segment(0,
		      WINSCALE(hud_pos_x - hudSize),
		      WINSCALE(hud_pos_y + hudSize - HUD_OFFSET),
		      WINSCALE(hud_pos_x + hudSize),
		      WINSCALE(hud_pos_y + hudSize - HUD_OFFSET));
    }
    if (hudVLineColor) {
	SET_FG(colors[hudVLineColor].pixel);
	rd.drawLine(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x -hudSize + HUD_OFFSET),
		    WINSCALE(hud_pos_y -hudSize),
		    WINSCALE(hud_pos_x -hudSize + HUD_OFFSET),
		    WINSCALE(hud_pos_y +hudSize));
	Erase_segment(0,
		      WINSCALE(hud_pos_x - hudSize + HUD_OFFSET),
		      WINSCALE(hud_pos_y - hudSize),
		      WINSCALE(hud_pos_x - hudSize + HUD_OFFSET),
		      WINSCALE(hud_pos_y + hudSize));
	rd.drawLine(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x + hudSize - HUD_OFFSET),
		    WINSCALE(hud_pos_y - hudSize),
		    WINSCALE(hud_pos_x + hudSize - HUD_OFFSET),
		    WINSCALE(hud_pos_y + hudSize));
	Erase_segment(0,
		      WINSCALE(hud_pos_x + hudSize - HUD_OFFSET),
		      WINSCALE(hud_pos_y - hudSize),
		      WINSCALE(hud_pos_x + hudSize - HUD_OFFSET),
		      WINSCALE(hud_pos_y + hudSize));
    }
    gcv.line_style = LineSolid;
    XChangeGC(dpy, gc, GCLineStyle, &gcv);
    SET_FG(colors[hudColor].pixel);

    if (hudItemsColor)
	Paint_HUD_items(hud_pos_x, hud_pos_y);
    SET_FG(colors[hudColor].pixel);

    /* Fuel notify, HUD meter on */
    if (fuelTime > 0.0 || fuelSum < fuelLevel3) {
	did_fuel = 1;
	sprintf(str, "%04d", (int)fuelSum);
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x + hudSize-HUD_OFFSET+BORDER),
		    WINSCALE(hud_pos_y + hudSize-HUD_OFFSET+BORDER)
				+ gameFont->ascent,
		    str, strlen(str));
	Erase_rectangle(WINSCALE(hud_pos_x + hudSize-HUD_OFFSET+BORDER) - 1,
			WINSCALE(hud_pos_y + hudSize-HUD_OFFSET+BORDER) ,
			XTextWidth(gameFont, str, strlen(str)) + 2,
			gameFont->ascent + gameFont->descent);
	if (numItems[ITEM_TANK]) {
	    if (fuelCurrent == 0)
		strcpy(str,"M ");
	    else
		sprintf(str, "T%d", fuelCurrent);
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(hud_pos_x + hudSize-HUD_OFFSET + BORDER),
			WINSCALE(hud_pos_y + hudSize-HUD_OFFSET + BORDER)
			+ gameFont->descent + 2*gameFont->ascent,
			str, strlen(str));
	    Erase_rectangle(WINSCALE(hud_pos_x + hudSize-HUD_OFFSET + BORDER)
				 - 1,
			    WINSCALE(hud_pos_y + hudSize-HUD_OFFSET + BORDER)
				+ gameFont->descent + gameFont->ascent,
			    XTextWidth(gameFont, str, strlen(str)) + 2,
			    gameFont->ascent + gameFont->descent);
	}
    }

    /* Update the lock display */
    Paint_lock(hud_pos_x, hud_pos_y);

    /* Draw last score on hud if it is an message attached to it */
    for (i=0, j=0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj
	    = &score_objects[(i+score_object)%MAX_SCORE_OBJECTS];
	if (sobj->hud_msg_len > 0) {
	    if (j == 0 &&
		sobj->hud_msg_width > WINSCALE(2*hudSize-HUD_OFFSET*2) &&
	        (did_fuel || hudVLineColor))
			++j;
	    rd.drawString(dpy, p_draw, gc,
			WINSCALE(hud_pos_x) - sobj->hud_msg_width/2,
			WINSCALE(hud_pos_y + hudSize-HUD_OFFSET + BORDER)
			+ gameFont->ascent
			+ j * (gameFont->ascent + gameFont->descent),
			sobj->hud_msg, sobj->hud_msg_len);
	    Erase_rectangle(WINSCALE(hud_pos_x) - sobj->hud_msg_width/2 - 1,
			    WINSCALE(hud_pos_y + hudSize-HUD_OFFSET + BORDER)
				+ j * (gameFont->ascent + gameFont->descent),
			    sobj->hud_msg_width + 2,
			    gameFont->ascent + gameFont->descent);
	    j++;
	}
    }

    if (time_left > 0) {
	sprintf(str, "%3d:%02d", (int)(time_left / 60), (int)(time_left % 60));
	size = XTextWidth(gameFont, str, strlen(str));
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x - hudSize+HUD_OFFSET - BORDER) - size,
		    WINSCALE(hud_pos_y - hudSize+HUD_OFFSET - BORDER)
			- gameFont->descent,
		    str, strlen(str));
	Erase_rectangle(WINSCALE(hud_pos_x - hudSize+HUD_OFFSET - BORDER)
			    - size - 1,
			WINSCALE(hud_pos_y - hudSize+HUD_OFFSET - BORDER)
			    - gameFont->ascent - gameFont->descent,
			size + 2,
			gameFont->ascent + gameFont->descent);
    }

    /* Update the modifiers */
    modlen = strlen(mods);
    rd.drawString(dpy, p_draw, gc,
		WINSCALE(hud_pos_x - hudSize+HUD_OFFSET-BORDER)
		    - XTextWidth(gameFont, mods, modlen),
		WINSCALE(hud_pos_y + hudSize-HUD_OFFSET+BORDER)
		    + gameFont->ascent,
		mods, strlen(mods));

    Erase_rectangle(WINSCALE(hud_pos_x - hudSize + HUD_OFFSET - BORDER)
			- XTextWidth(gameFont, mods, modlen) - 1,
		    WINSCALE(hud_pos_y + hudSize - HUD_OFFSET + BORDER) ,
			XTextWidth(gameFont, mods, modlen) + 1,
		    gameFont->ascent + gameFont->descent);

    if (autopilotLight) {
	int text_width = XTextWidth(gameFont, autopilot, sizeof(autopilot)-1);
	rd.drawString(dpy, p_draw, gc,
		    WINSCALE(hud_pos_x) - text_width/2,
		    WINSCALE(hud_pos_y - hudSize+HUD_OFFSET - BORDER)
				 - gameFont->descent * 2 - gameFont->ascent,
		    autopilot, sizeof(autopilot)-1);

	Erase_rectangle(WINSCALE(hud_pos_x) - text_width/2,
			WINSCALE(hud_pos_y - hudSize+HUD_OFFSET - BORDER)
			    - gameFont->descent * 2 - gameFont->ascent * 2,
			text_width + 2,
			gameFont->ascent + gameFont->descent);
    }

    if (fuelTime > 0.0) {
	fuelTime -= timePerFrame;
	if (fuelTime <= 0.0)
	    fuelTime = 0.0;
    }

    /* draw fuel gauge */
    if (fuelGaugeColor &&
	((fuelTime > 0.0)
	 || (fuelSum < fuelLevel3
	     && ((fuelSum < fuelLevel1 && (loopsSlow % 4) < 2)
		 || (fuelSum < fuelLevel2
		     && fuelSum > fuelLevel1
		     && (loopsSlow % 8) < 4)
		 || (fuelSum > fuelLevel2))))) {

	SET_FG(colors[fuelGaugeColor].pixel);
	rd.drawRectangle(dpy, p_draw, gc,
			 WINSCALE(hud_pos_x + hudSize - HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) - 1,
			 WINSCALE(hud_pos_y - hudSize + HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) - 1,
			 WINSCALE(HUD_OFFSET - (2*FUEL_GAUGE_OFFSET)) + 3,
			 WINSCALE(HUD_FUEL_GAUGE_SIZE) + 3);
	Erase_4point(WINSCALE(hud_pos_x + hudSize - HUD_OFFSET
			      + FUEL_GAUGE_OFFSET) - 1,
		     WINSCALE(hud_pos_y - hudSize + HUD_OFFSET
			      + FUEL_GAUGE_OFFSET) - 1,
		     WINSCALE(HUD_OFFSET - (2*FUEL_GAUGE_OFFSET)) + 3,
		     WINSCALE(HUD_FUEL_GAUGE_SIZE) + 3);

	size = (HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax;
	rd.fillRectangle(dpy, p_draw, gc,
			 WINSCALE(hud_pos_x + hudSize - HUD_OFFSET
				  + FUEL_GAUGE_OFFSET) + 1,
			 WINSCALE(hud_pos_y - hudSize + HUD_OFFSET
				  + FUEL_GAUGE_OFFSET + HUD_FUEL_GAUGE_SIZE
				  - size) + 1,
			 WINSCALE(HUD_OFFSET - (2*FUEL_GAUGE_OFFSET)),
			 WINSCALE(size));
	Erase_rectangle(WINSCALE(hud_pos_x + hudSize - HUD_OFFSET
				 + FUEL_GAUGE_OFFSET),
			WINSCALE(hud_pos_y - hudSize + HUD_OFFSET
				 + FUEL_GAUGE_OFFSET + HUD_FUEL_GAUGE_SIZE
				 - size),
			HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 1, size + 1);
    }
}


void Paint_messages(void)
{
    int		i, x, y, top_y, bot_y, width, len;
    const int	BORDER = 10,
		SPACING = messageFont->ascent+messageFont->descent+1;
    message_t	*msg;
    int		msg_color;
    int		last_msg_index = 0;

    top_y = BORDER + messageFont->ascent;
    bot_y = WINSCALE(ext_view_height) - messageFont->descent - BORDER;

    /* get number of player messages */
    if (selectionAndHistory) {
	while (last_msg_index < maxMessages
		&& TalkMsg[last_msg_index]->len != 0) {
	    last_msg_index++;
	}
	last_msg_index--; /* make it an index */
    }

    for (i = (BIT(instruments, SHOW_REVERSE_SCROLL) ? 2 * maxMessages - 1 : 0);
	 (BIT(instruments, SHOW_REVERSE_SCROLL) ? i >= 0 : i < 2 * maxMessages);
	 i += (BIT(instruments, SHOW_REVERSE_SCROLL) ? -1 : 1)) {
	if (i < maxMessages) {
	    msg = TalkMsg[i];
	} else {
	    msg = GameMsg[i - maxMessages];
	}
	if (msg->len == 0)
	    continue;

	/*
	 * while there is something emphasized, freeze the life time counter
	 * of a message if it is not drawn `flashed' (red) anymore
	 */
	if (
	    msg->lifeTime > MSG_FLASH_TIME
#ifndef _WINDOWS
	    || !selectionAndHistory
	    || (selection.draw.state != SEL_PENDING
		&& selection.draw.state != SEL_EMPHASIZED)
#endif
	    ) {

	    if ((msg->lifeTime -= timePerFrame) <= 0.0) {
		msg->txt[0] = '\0';
		msg->len = 0;
		msg->lifeTime = 0.0;
		continue;
	    }
	}
#ifdef _WINDOWS
	else if ((msg->lifeTime -= timePerFrame) <= 0.0) {
		msg->txt[0] = '\0';
		msg->len = 0;
		msg->lifeTime = 0.0;
		continue;
	    }
#endif

	if (i < maxMessages) {
	    x = BORDER;
	    y = top_y;
	    top_y += SPACING;
	} else {
	    if (!BIT(instruments, SHOW_MESSAGES)) {
		continue;
	    }
	    x = BORDER;
	    y = bot_y;
	    bot_y -= SPACING;
	}
	len = (int)(charsPerSecond * (MSG_LIFE_TIME - msg->lifeTime));
	len = MIN(msg->len, len);
	if (msg->lifeTime > MSG_FLASH_TIME) {
	    msg_color = messagesColor;
	}
	else {
	    msg_color = oldMessagesColor;
	}

#ifndef _WINDOWS
	/*
	 * it's an emphasized talk message
	 */
	if (selectionAndHistory && selection.draw.state == SEL_EMPHASIZED
	    && i < maxMessages
	    && TALK_MSG_SCREENPOS(last_msg_index,i) >= selection.draw.y1
	    && TALK_MSG_SCREENPOS(last_msg_index,i) <= selection.draw.y2) {

	    /*
	     * three strings (ptr), where they begin (xoff) and their
	     * length (l):
	     *   1st is an umemph. string to the left of a selection,
	     *   2nd an emphasized part itself,
	     *   3rd an unemph. part to the right of a selection.
	     * set the according variables if a part exists.
	     * e.g: a selection of several lines `stopping' somewhere in
	     *   the middle of a line -> ptr2,ptr3 are needed to draw
	     *   this line
	     */
	    char	*ptr  = NULL;
	    int		xoff  = 0, l = 0;
	    char	*ptr2 = NULL;
	    int		xoff2 = 0, l2 = 0;
	    char	*ptr3 = NULL;
	    int		xoff3 = 0, l3 = 0;

	    if (TALK_MSG_SCREENPOS(last_msg_index,i) > selection.draw.y1
		 && TALK_MSG_SCREENPOS(last_msg_index,i) < selection.draw.y2) {
		    /* all emphasized on this line */
		    /*xxxxxxxxx*/
		ptr2 = msg->txt;
		l2 = len;
		xoff2 = 0;
	    } else if (TALK_MSG_SCREENPOS(last_msg_index,i) == selection.draw.y1) {
		    /* first/only line */
		    /*___xxx[___]*/
		ptr = msg->txt;
		xoff = 0;
		if ( len < selection.draw.x1) {
		    l = len;
		} else {
			/* at least two parts */
			/*___xxx[___]*/
			/*    ^      */
		    l = selection.draw.x1;
		    ptr2 = &(msg->txt[selection.draw.x1]);
		    xoff2 = XTextWidth(messageFont, msg->txt, selection.draw.x1);

		    if (TALK_MSG_SCREENPOS(last_msg_index,i) < selection.draw.y2) {
			    /* first line */
			    /*___xxxxxx*/
			    /*     ^   */
			l2 = len - selection.draw.x1;
		    } else {
			    /* only line */
			    /*___xxx___*/
			if (len <= selection.draw.x2) {
				/*___xxx___*/
				/*    ^    */
			    l2 = len - selection.draw.x1;
			} else {
				/*___xxx___*/
				/*       ^ */
			    l2 = selection.draw.x2 - selection.draw.x1 + 1;
			    ptr3 = &(msg->txt[selection.draw.x2 + 1]);
			    xoff3 = XTextWidth(messageFont, msg->txt, selection.draw.x2 + 1);
			    l3 = len - selection.draw.x2 - 1;
			}
		    } /* only line */
		} /* at least two parts */
	    } else {
		    /* last line */
		    /*xxxxxx[___]*/
		ptr2 = msg->txt;
		xoff2 = 0;
		if (len <= selection.draw.x2 + 1) {
			/* all blue */
			/*xxxxxx[___]*/
			/*  ^        */
		    l2 = len;
		} else {
			/*xxxxxx___*/
			/*       ^ */
		    l2 = selection.draw.x2 + 1;
		    ptr3 = &(msg->txt[selection.draw.x2 + 1]);
		    xoff3 = XTextWidth(messageFont, msg->txt, selection.draw.x2 + 1);
		    l3 = len - selection.draw.x2 - 1;
		}
	    } /* last line */


	    if (ptr) {
		XSetForeground(dpy, messageGC, colors[msg_color].pixel);
		rd.drawString(dpy, p_draw, messageGC, x + xoff, y, ptr, l);
	    }
	    if (ptr2) {
		XSetForeground(dpy, messageGC, colors[DRAW_EMPHASIZED].pixel);
		rd.drawString(dpy, p_draw, messageGC, x + xoff2, y, ptr2, l2);
	    }
	    if (ptr3) {
		XSetForeground(dpy, messageGC, colors[msg_color].pixel);
		rd.drawString(dpy, p_draw, messageGC, x + xoff3, y, ptr3, l3);
	    }

	} else /* not emphasized */
#endif
	{
	    XSetForeground(dpy, messageGC, colors[msg_color].pixel);
	    rd.drawString(dpy, p_draw, messageGC, x, y, msg->txt, len);
	}

	if (len < msg->len) {
	    width = XTextWidth(messageFont, msg->txt, len);
	} else {
	    width = msg->pixelLen;
	}
	Erase_rectangle((x) - 1,
			(y) - messageFont->ascent ,
			width + 2,
			messageFont->ascent + messageFont->descent);
    }
}


void Paint_recording(void)
{
    int			w = -1;
    int			x, y;
    char		buf[32];
    int			len;
    DFLOAT		mb;

    if (!recording || (loopsSlow % 16) < 8)
	return;

    SET_FG(colors[RED].pixel);
    mb = ((DFLOAT)Record_size()) / 1e6;
    sprintf(buf, "REC %.1f MB", mb);
    len = strlen(buf);
    w = XTextWidth(gameFont, buf, len);
    x = WINSCALE(ext_view_width) - 10 - w;
    y = 10 + gameFont->ascent;
    XDrawString(dpy, p_draw, gc, x, y, buf, len);
    Erase_rectangle( x - 1, WINSCALE(10),
			 w+2, gameFont->ascent + gameFont->descent);
}


void Paint_client_fps(void)
{
    int			w = -1;
    int			x, y;
    char		buf[32];
    int			len;

    /*if (!showFPS)
     * return;
     */

    SET_FG(colors[BLUE].pixel);
    sprintf(buf, "FPS: %d", clientFPS);
    len = strlen(buf);
    w = XTextWidth(gameFont, buf, len);
    x = WINSCALE(ext_view_width) - 10 - w;
    y = 200 + gameFont->ascent;
    rd.drawString(dpy, p_draw, gc, x, y, buf, len);
    Erase_rectangle( x - 1,  WINSCALE(200),
			 w+2, gameFont->ascent + gameFont->descent);
}

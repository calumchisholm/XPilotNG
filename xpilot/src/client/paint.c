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

#include "xpclient_x11.h"

char paint_version[] = VERSION;

/*
 * Globals.
 */
ipos	world;
ipos	realWorld;

XFontStruct* gameFont;		/* The fonts used in the game */
XFontStruct* messageFont;
XFontStruct* scoreListFont;
XFontStruct* buttonFont;
XFontStruct* textFont;
XFontStruct* talkFont;
XFontStruct* motdFont;
char	gameFontName[FONT_LEN];	/* The fonts used in the game */
char	messageFontName[FONT_LEN];
char	scoreListFontName[FONT_LEN];
char	buttonFontName[FONT_LEN];
char	textFontName[FONT_LEN];
char	talkFontName[FONT_LEN];
char	motdFontName[FONT_LEN];

Display		*dpy;		/* Display of player (pointer) */
Display		*kdpy;		/* Keyboard display */
short		about_page;	/* Which page is the player on? */

GC		gameGC;		/* GC for the game area */
GC		messageGC;	/* GC for messages in the game area */
GC		radarGC;	/* GC for the radar */
GC		buttonGC;	/* GC for the buttons */
GC		scoreListGC;	/* GC for the player list */
GC		textGC;		/* GC for the info text */
GC		talkGC;		/* GC for the message window */
GC		motdGC;		/* GC for the motd text */
XGCValues	gcv;

Window	topWindow;		/* Top-level window (topshell) */
Window	drawWindow;		/* Main play window */
Window	keyboardWindow;		/* Keyboard window */
#ifdef _WINDOWS
/*
 * Windows needs some dummy windows (size 0,0)
 * so we can store the active fonts.
 */
				/* supports 1 active font per window */
Window	textWindow;		/* for the GC into the config window */
Window	msgWindow;		/* for messages into the playfield */
Window	buttonWindow;		/* to calculate size of buttons */
#endif

Pixmap	drawPixmap;		/* Saved pixmap for the drawing */
				/* area (monochromes use this) */
Window	playersWindow;		/* Player list window */
				/* monochromes) */
Window	aboutWindow;
Window	about_close_b;		/* About window's close button */
Window	about_next_b;		/* About window's next button */
Window	about_prev_b;		/* About window's previous button */
Window	keys_close_b;		/* Help window's close button */
Window	talkWindow;
XColor	colors[MAX_COLORS];
Colormap	colormap;	/* Private colormap */
int	maxColors;		/* Max. number of colors to use */
bool	gotFocus;
bool	players_exposed;
short	ext_view_width;		/* Width of extended visible area */
short	ext_view_height;	/* Height of extended visible area */
int	active_view_width;	/* Width of active map area displayed. */
int	active_view_height;	/* Height of active map area displayed. */
int	ext_view_x_offset;	/* Offset ext_view_width */
int	ext_view_y_offset;	/* Offset ext_view_height */

bool	titleFlip;		/* Do special title bar flipping? */
int	shieldDrawMode = -1;	/* Either LineOnOffDash or LineSolid */

int		maxKeyDefs;
keydefs_t	*keyDefs = NULL;

long		loops = 0;
unsigned long	loopsSlow = 0;	/* Proceeds slower than loops */
static double   time_counter = 0.0;
double          timePerFrame = 0.0;

int	cacheShips = 0;		/* cache some ship bitmaps every frame */

int	clockColor;		/* Clock color index */
int	scoreColor;		/* Score list color index */
int	scoreSelfColor;		/* Score list own score color index */
int	scoreInactiveColor;	/* Score list inactive player color index */
int	scoreInactiveSelfColor;	/* Score list inactive self color index */
int	scoreOwnTeamColor;	/* Score list own team color index */
int	scoreEnemyTeamColor;	/* Score list enemy team color index */
int	scoreObjectColor;	/* Color index for map score objects */

int	zeroLivesColor;		/* Color to associate with 0 lives */
int	oneLifeColor;		/* Color to associate with 1 life */
int	twoLivesColor;		/* Color to associate with 2 lives */
int	manyLivesColor;		/* Color to associate with >2 lives */

static void Paint_clock(bool redraw);

void Game_over_action(u_byte status)
{
    static u_byte old_status = 0;

    if (BIT(old_status, GAME_OVER) && !BIT(status, GAME_OVER)
	&& !BIT(status,PAUSE))
	XMapRaised(dpy, topWindow);

    /* GAME_OVER -> PLAYING */
    if (BIT(old_status, PLAYING|PAUSE|GAME_OVER) != PLAYING) {
	if (BIT(status, PLAYING|PAUSE|GAME_OVER) == PLAYING)
	    Reset_shields();
    }

    old_status = status;
}

int Check_view_dimensions(void)
{
    int			width_wanted, height_wanted;
    int			srv_width, srv_height;

    width_wanted = (int)(draw_width * scaleFactor + 0.5);
    height_wanted = (int)(draw_height * scaleFactor + 0.5);

    srv_width = width_wanted;
    srv_height = height_wanted;
    LIMIT(srv_height, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    LIMIT(srv_width, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    if (server_display.view_width != srv_width ||
	server_display.view_height != srv_height) {
	if (Send_display(srv_width, 
			 srv_height, 
			 spark_rand, 
			 num_spark_colors))
	    return -1;
    }
    active_view_width = server_display.view_width;
    active_view_height = server_display.view_height;
    ext_view_x_offset = 0;
    ext_view_y_offset = 0;
    if (width_wanted > active_view_width) {
	ext_view_width = width_wanted;
	ext_view_x_offset = (width_wanted - active_view_width) / 2;
    } else {
	ext_view_width = active_view_width;
    }
    if (height_wanted > active_view_height) {
	ext_view_height = height_wanted;
	ext_view_y_offset = (height_wanted - active_view_height) / 2;
    } else {
	ext_view_height = active_view_height;
    }
    return 0;
}


void Paint_frame(void)
{
    static long		scroll_i = 0;
    static int		prev_damaged = 0;
    static int		prev_prev_damaged = 0;

#ifdef _WINDOWS
    /* give any outgoing data a head start to the server */
    /* send anything to the server before returning to Windows */
    Net_flush();
#endif

    Check_view_dimensions();

    world.x = FOOpos.x - (ext_view_width / 2);
    world.y = FOOpos.y - (ext_view_height / 2);
    realWorld = world;
    if (BIT(Setup->mode, WRAP_PLAY)) {
	if (world.x < 0 && world.x + ext_view_width < Setup->width)
	    world.x += Setup->width;
	else if (world.x > 0 && world.x + ext_view_width >= Setup->width)
	    realWorld.x -= Setup->width;
	if (world.y < 0 && world.y + ext_view_height < Setup->height)
	    world.y += Setup->height;
	else if (world.y > 0 && world.y + ext_view_height >= Setup->height)
	    realWorld.y -= Setup->height;
    }

    if (start_loops != end_loops)
	warn("Start neq. End (%ld,%ld,%ld)", start_loops, end_loops, loops);
    loops = end_loops;


    /*
     * If time() changed from previous value, assume one second has passed.
     */
    if (newSecond) {
	/* kps - improve */
	recordFPS = clientFPS;
	timePerFrame = 1.0 / clientFPS;

	/* TODO: move this somewhere else */
	/* check once per second if we are playing */
	if (newSecond && self && !strchr("PW", self->mychar))
	    played_this_round = true;
    }

    /*
     * Instead of using loops to determining if things are drawn this frame,
     * loopsSlow should be used. We don't want things to be drawn too fast
     * at high fps.
     */
    time_counter += timePerFrame;
    if (time_counter >= 1.0 / 12) {
	loopsSlow++;
	time_counter -= (1.0 / 12);
    }

    /*
     * Switch between two different window titles.
     */
    if (titleFlip && ((loopsSlow % TITLE_DELAY) == 0)) {
	scroll_i = !scroll_i;
	if (scroll_i)
	    XStoreName(dpy, topWindow, COPYRIGHT);
	else
	    XStoreName(dpy, topWindow, TITLE);
    }

    /* This seems to have a bug (in Windows) 'cause last frame we ended
       with an XSetForeground(white) confusing SET_FG */
    SET_FG(colors[BLACK].pixel);

#ifdef _WINDOWS
    drawPixmap = draw;		/* let's try this */
    XSetForeground(dpy, gameGC, colors[BLACK].pixel);
    XFillRectangle(dpy, drawPixmap, gameGC, 0, 0, draw_width, draw_height);
#endif

    rd.newFrame();


    /*
     * Do we really need to draw all this if the player is damaged?
     */
    if (damaged <= 0) {
	if (prev_damaged || prev_prev_damaged) {
	    /* clean up ecm damage */
	    SET_FG(colors[BLACK].pixel);
	    XFillRectangle(dpy, drawWindow, gameGC,
			   0, 0, draw_width, draw_height);
	}

	Arc_start();

	Rectangle_start();
	Segment_start();

	Paint_world();

	Segment_end();
	Rectangle_end();

	Rectangle_start();
	Segment_start();

	if (oldServer) {
	    Paint_vfuel();
	    Paint_vdecor();
	    Paint_vcannon();
	    Paint_vbase();
	} else
	    Paint_objects();

	Paint_shots();

	Rectangle_end();
	Segment_end();

	Rectangle_start();
	Segment_start();

	Paint_ships();
	Paint_meters();
	Paint_HUD();
	Paint_recording();
	Paint_client_fps();

	Rectangle_end();
	Segment_end();

	Arc_end();

	Paint_messages();
	Paint_radar();
	Paint_score_objects();
    }
    else {
	/* Damaged. */

	XSetFunction(dpy, gameGC, GXxor);
	SET_FG(colors[BLACK].pixel ^ colors[BLUE].pixel);
	XFillRectangle(dpy, drawWindow, gameGC, 0, 0, draw_width, draw_height);
	XSetFunction(dpy, gameGC, GXcopy);
	SET_FG(colors[BLACK].pixel);
    }
    prev_prev_damaged = prev_damaged;
    prev_damaged = damaged;

    rd.endFrame();

    if (radar_exposures == 1)
	Paint_world_radar();

    /*
     * Now switch planes and clear the screen.
     */
    if (radarPixmap != radarWindow && radar_exposures > 0) {
	if (BIT(instruments, SHOW_SLIDING_RADAR) == 0
	    || BIT(Setup->mode, WRAP_PLAY) == 0) {
#ifndef _WINDOWS
	    XCopyArea(dpy, radarPixmap, radarWindow, gameGC,
		      0, 0, 256, RadarHeight, 0, 0);
#else
	    WinXBltPixToWin(radarPixmap, radarWindow,
			    0, 0, 256, RadarHeight, 0, 0);
#endif
	} else {

	    int x, y, w, h;
	    float xp, yp, xo, yo;

	    xp = (float) (FOOpos.x * 256) / Setup->width;
	    yp = (float) (FOOpos.y * RadarHeight) / Setup->height;
	    xo = (float) 256 / 2;
	    yo = (float) RadarHeight / 2;
	    if (xo <= xp)
		x = (int) (xp - xo + 0.5);
	    else
		x = (int) (256 + xp - xo + 0.5);

	    if (yo <= yp)
		y = (int) (yp - yo + 0.5);
	    else
		y = (int) (RadarHeight + yp - yo + 0.5);

	    y = RadarHeight - y - 1;
	    w = 256 - x;
	    h = RadarHeight - y;

#ifndef _WINDOWS
	    XCopyArea(dpy, radarPixmap, radarWindow, gameGC,
		      0, 0, x, y, w, h);
	    XCopyArea(dpy, radarPixmap, radarWindow, gameGC,
		      x, 0, w, y, 0, h);
	    XCopyArea(dpy, radarPixmap, radarWindow, gameGC,
		      0, y, x, h, w, 0);
	    XCopyArea(dpy, radarPixmap, radarWindow, gameGC,
		      x, y, w, h, 0, 0);
#else
	    Paint_world_radar();
#endif
	}
    }
    else if (radar_exposures > 2)
	Paint_world_radar();

#ifndef _WINDOWS
    if (dbuf_state->type == PIXMAP_COPY)
	XCopyArea(dpy, drawPixmap, drawWindow, gameGC,
		  0, 0, draw_width, draw_height, 0, 0);

    dbuff_switch(dbuf_state);

    if (dbuf_state->type == COLOR_SWITCH) {
	XSetPlaneMask(dpy, gameGC, dbuf_state->drawing_planes);
	XSetPlaneMask(dpy, messageGC, dbuf_state->drawing_planes);
    }
#endif

    if (!damaged) {
	/* Prepare invisible buffer for next frame by clearing. */

	/*
	 * DBE's XdbeBackground switch option is
	 * probably faster than XFillRectangle.
	 */
#ifndef _WINDOWS
	if (dbuf_state->multibuffer_type != MULTIBUFFER_DBE) {
	    SET_FG(colors[BLACK].pixel);
	    XFillRectangle(dpy, drawPixmap, gameGC,
			   0, 0, draw_width, draw_height);
	}
#endif
    }

#ifndef _WINDOWS
    if (talk_mapped == true) {
	static bool toggle;
	static long last_toggled;

	if (loops >= last_toggled + FPS / 2 || loops < last_toggled) {
	    toggle = (toggle == false) ? true : false;
	    last_toggled = loops;
	}
	Talk_cursor(toggle);
    }
#endif

#ifdef _WINDOWS
    Paint_score_table();
    PaintWinClient();
#endif

    Paint_clock(false);

    XFlush(dpy);
}


#define SCORE_BORDER		6


static void Paint_score_background(int thisLine)
{
    if (fullColor &&
	Bitmap_get(playersWindow, BM_SCORE_BG, 0) != NULL &&
	Bitmap_get(playersWindow, BM_LOGO, 0) != NULL) {
	int bgh, lh;

	XSetForeground(dpy, scoreListGC, colors[BLACK].pixel);

	bgh = pixmaps[BM_SCORE_BG].height;
	lh = pixmaps[BM_LOGO].height;

	IFWINDOWS( XFillRectangle(dpy, playersWindow, scoreListGC,
				  0, 0, players_width, players_height) );

	Bitmap_paint(playersWindow, BM_SCORE_BG, 0, 0, 0);
	if (players_height > bgh + lh)
	    XFillRectangle(dpy, playersWindow, scoreListGC,
			   0, bgh,
			   players_width, players_height - (bgh + lh));
	Bitmap_paint(playersWindow, BM_LOGO, 0, players_height - lh, 0);
	XFlush(dpy);
    } else {
	XSetForeground(dpy, scoreListGC, colors[windowColor].pixel);
	XFillRectangle(dpy, playersWindow, scoreListGC,
		       0, 0, players_width, players_height);
	XFlush(dpy);
    }
}


void Paint_score_start(void)
{
    char	headingStr[MSG_LEN];
    static int thisLine;

    thisLine = SCORE_BORDER + scoreListFont->ascent;

    if (showRealName)
	strlcpy(headingStr, "NICK=USER@HOST", sizeof(headingStr));
    else if (BIT(Setup->mode, TEAM_PLAY))
	strlcpy(headingStr, "     SCORE   NAME     LIFE", sizeof(headingStr));
    else {
	strlcpy(headingStr, "  ", sizeof(headingStr));
	if (BIT(Setup->mode, TIMING))
	    strcat(headingStr, "LAP ");
	strlcpy(headingStr, " AL ", sizeof(headingStr));
	strcat(headingStr, "  SCORE  ");
	if (BIT(Setup->mode, LIMITED_LIVES))
	    strlcat(headingStr, "LIFE", sizeof(headingStr));
	strlcat(headingStr, " NAME", sizeof(headingStr));
    }
    Paint_score_background(thisLine);

    ShadowDrawString(dpy, playersWindow, scoreListGC,
		     SCORE_BORDER, thisLine,
		     headingStr,
		     colors[scoreColor].pixel,
		     colors[BLACK].pixel);

    gcv.line_style = LineSolid;
    XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
    XDrawLine(dpy, playersWindow, scoreListGC,
	      SCORE_BORDER, thisLine,
	      players_width - SCORE_BORDER, thisLine);

    gcv.line_style = LineOnOffDash;
    XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);

    Paint_clock(true);
}


void Paint_score_entry(int entry_num, other_t* other, bool is_team)
{
    static char		raceStr[8], teamStr[4], lifeStr[8], label[MSG_LEN];
    static int		lineSpacing = -1, firstLine;
    int			thisLine, color;
    char		scoreStr[16];

    /*
     * First time we're here, set up miscellaneous strings for
     * efficiency and calculate some other constants.
     */
    if (lineSpacing == -1) {
	memset(raceStr, '\0', sizeof raceStr);
	memset(teamStr, '\0', sizeof teamStr);
	memset(lifeStr, '\0', sizeof lifeStr);
	teamStr[1] = ' ';
	raceStr[2] = ' ';

	lineSpacing
	    = scoreListFont->ascent + scoreListFont->descent + 3;
	firstLine
	    = 2*SCORE_BORDER + scoreListFont->ascent + lineSpacing;
    }
    thisLine = firstLine + lineSpacing * entry_num;

    /*
     * Setup the status line
     */
    if (showRealName)
	sprintf(label, "%s=%s@%s", other->name, other->real, other->host);
    else {
	other_t*	war = Other_by_id(other->war_id);

	if (BIT(Setup->mode, TIMING)) {
	    raceStr[0] = ' ';
	    raceStr[1] = ' ';
	    if ((other->mychar == ' ' || other->mychar == 'R')
		&& other->round + other->check > 0) {
		if (other->round > 99)
		    sprintf(raceStr, "%3d", other->round);
		else
		    sprintf(raceStr, "%d.%c",
			    other->round, other->check + 'a');
	    }
	}
	if (BIT(Setup->mode, TEAM_PLAY))
	    teamStr[0] = other->team + '0';
	else
	    sprintf(teamStr, "%c", other->alliance);

	if (BIT(Setup->mode, LIMITED_LIVES))
	    sprintf(lifeStr, " %3d", other->life);

	if (Using_score_decimals())
	    sprintf(scoreStr, "%*.*f",
		    9 - showScoreDecimals, showScoreDecimals,
		    other->score);
	else
	    sprintf(scoreStr, "%6d", (int) rint(other->score));

	if (BIT(Setup->mode, TEAM_PLAY))
	    sprintf(label, "%c %s  %-18s%s",
		    other->mychar, scoreStr, other->name, lifeStr);
	else {
	    sprintf(label, "%c %s%s%s%s  %s",
		    other->mychar, raceStr, teamStr,
		    scoreStr, lifeStr,
		    other->name);
	    if (war) {
		if (strlen(label) + strlen(war->name) + 5 < sizeof(label))
		    sprintf(label + strlen(label), " (%s)", war->name);
	    }
	}
    }

    /*
     * Draw the line
     * e94_msu eKthHacks
     */
    if (!is_team && strchr("DPW", other->mychar)) {
	if (other->id == self->id)
	    color = scoreInactiveSelfColor;
	else
	    color = scoreInactiveColor;

	XSetForeground(dpy, scoreListGC, colors[color].pixel);
	XDrawString(dpy, playersWindow, scoreListGC,
		    SCORE_BORDER, thisLine,
		    label, strlen(label));
    } else {
	if (!is_team) {
	    if (other->id == self->id)
		color = scoreSelfColor;
	    else
		color = scoreColor;
	} else {
	    color = Team_color(other->team);
	    if (!color) {
		if (other->team == self->team)
		    color = scoreOwnTeamColor;
		else
		    color = scoreEnemyTeamColor;
	    }
	}

	ShadowDrawString(dpy, playersWindow, scoreListGC, SCORE_BORDER,
			 thisLine, label,
			 colors[color].pixel,
			 colors[BLACK].pixel);
    }

    /*
     * Underline the teams
     */
    if (is_team) {
	color = (windowColor != BLUE ? BLUE : BLACK);
	XSetForeground(dpy, scoreListGC, colors[color].pixel);
	gcv.line_style = LineSolid;
	XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
	XDrawLine(dpy, playersWindow, scoreListGC,
		  SCORE_BORDER, thisLine,
		  players_width - SCORE_BORDER, thisLine);
	gcv.line_style = LineOnOffDash;
	XChangeGC(dpy, scoreListGC, GCLineStyle, &gcv);
    }
}


static void Print_roundend_messages(other_t **order)
{
    static char		hackbuf[MSG_LEN];
    static char		hackbuf2[MSG_LEN];
    static char		kdratio[16];
    static char		killsperround[16];
    char		*s;
    int			i;
    other_t		*other;

    roundend = false;

    if (killratio_totalkills == 0)
	sprintf(kdratio, "0");
    else if (killratio_totaldeaths == 0)
	sprintf(kdratio, "infinite");
    else
	sprintf(kdratio, "%.2f",
		(double)killratio_totalkills / killratio_totaldeaths);

    if (rounds_played == 0)
	sprintf(killsperround, "0");
    else
	sprintf(killsperround, "%.2f",
		(double)killratio_totalkills / rounds_played);

    sprintf(hackbuf, "Kill ratio - Round: %d/%d Total: %d/%d (%s) "
	    "Rounds played: %d  Avg.kills/round: %s",
	    killratio_kills, killratio_deaths,
	    killratio_totalkills, killratio_totaldeaths, kdratio,
	    rounds_played, killsperround);

    killratio_kills = 0;
    killratio_deaths = 0;
    Add_message(hackbuf);

    sprintf(hackbuf, "Ballstats - Cash/Repl/Team/Lost: %d/%d/%d/%d",
	    ballstats_cashes, ballstats_replaces,
	    ballstats_teamcashes, ballstats_lostballs);
    Add_message(hackbuf);

    s = hackbuf;
    s += sprintf(s, "Points - ");
    /*
     * Scores are nice to see e.g. in cup recordings.
     */
    for (i = 0; i < num_others; i++) {
	other = order[i];
	if (other->mychar == 'P')
	    continue;

	if (Using_score_decimals()) {
	    sprintf(hackbuf2, "%s: %.*f ", other->name,
		    showScoreDecimals, other->score);
	    if ((s - hackbuf) + strlen(hackbuf2) > MSG_LEN) {
		Add_message(hackbuf);
		s = hackbuf;
	    }
	    s += sprintf(s, "%s", hackbuf2);
	} else {
	    sprintf(hackbuf2, "%s: %d ", other->name,
		    (int) rint(other->score));
	    if ((s - hackbuf) + strlen(hackbuf2) > MSG_LEN) {
		Add_message(hackbuf);
		s = hackbuf;
	    }
	    s += sprintf(s,"%s",hackbuf2);
	}
    }
    Add_message(hackbuf);
}

struct team_score {
    double	score;
    int		life;
    int		playing;
};


static void Determine_team_order(struct team_score *team_order[],
				 struct team_score team[])
{
    int i, j, k;

    num_playing_teams = 0;
    for (i = 0; i < MAX_TEAMS; i++) {
	if (team[i].playing) {
	    for (j = 0; j < num_playing_teams; j++) {
		if (team[i].score > team_order[j]->score
		    || (team[i].score == team_order[j]->score
			&& ((BIT(Setup->mode, LIMITED_LIVES))
			    ? (team[i].life > team_order[j]->life)
			    : (team[i].life < team_order[j]->life)))) {
		    for (k = i; k > j; k--)
			team_order[k] = team_order[k - 1];
		    break;
		}
	    }
	    team_order[j] = &team[i];
	    num_playing_teams++;
	}
    }
}

static void Determine_order(other_t **order, struct team_score team[])
{
    other_t		*other;
    int			i, j, k;

    for (i = 0; i < num_others; i++) {
	other = &Others[i];
	if (BIT(Setup->mode, TIMING)) {
	    /*
	     * Sort the score table on position in race.
	     * Put paused and waiting players last as well as tanks.
	     */
	    if (strchr("PTW", other->mychar))
		j = i;
	    else {
		for (j = 0; j < i; j++) {
		    if (order[j]->timing < other->timing)
			break;
		    if (strchr("PTW", order[j]->mychar))
			break;
		    if (order[j]->timing == other->timing) {
			if (order[j]->timing_loops > other->timing_loops)
			    break;
		    }
		}
	    }
	}
	else {
	    for (j = 0; j < i; j++) {
		if (order[j]->score < other->score)
		    break;
	    }
	}
	for (k = i; k > j; k--)
	    order[k] = order[k - 1];
	order[j] = other;

	if (BIT(Setup->mode, TEAM_PLAY|TIMING) == TEAM_PLAY) {
	    switch (other->mychar) {
	    case 'P':
	    case 'W':
	    case 'T':
		break;
	    case ' ':
	    case 'R':
		if (BIT(Setup->mode, LIMITED_LIVES))
		    team[other->team].life += other->life + 1;
		else
		    team[other->team].life += other->life;
		/*FALLTHROUGH*/
	    default:
		team[other->team].playing++;
		team[other->team].score += other->score;
		break;
	    }
	}
    }
    return;
}

#define TEAM_PAUSEHACK 100

static int Team_heading(int entrynum, int teamnum,
			int teamlives, double teamscore)
{
    other_t tmp;
    tmp.id = -1;
    tmp.team = teamnum;
    tmp.war_id = -1;
    tmp.name_width = 0;
    tmp.ship = NULL;
    if (teamnum != TEAM_PAUSEHACK)
	sprintf(tmp.name, "TEAM %d", tmp.team);
    else
	sprintf(tmp.name, "Pause Wusses");
    strcpy(tmp.real, tmp.name);
    strcpy(tmp.host, "");
#if 0
    if (BIT(Setup->mode, LIMITED_LIVES) && teamlives == 0)
	tmp.mychar = 'D';
    else
	tmp.mychar = ' ';
#else
    tmp.mychar = ' ';
#endif
    tmp.score = teamscore;
    tmp.life = teamlives;

    Paint_score_entry(entrynum++, &tmp, true);
    return entrynum;
}

static int Team_score_table(int entrynum, int teamnum,
			    struct team_score team, other_t **order)
{
    other_t *other;
    int i, j;
    bool drawn = false;

    for (i = 0; i < num_others; i++) {
	other = order[i];

	if (teamnum == TEAM_PAUSEHACK) {
	    if (other->mychar != 'P')
		continue;
	} else {
	    if (other->team != teamnum || other->mychar == 'P')
		continue;
	}

	if (!drawn)
	    entrynum = Team_heading(entrynum, teamnum, team.life, team.score);
	j = other - Others;
	Paint_score_entry(entrynum++, other, false);
	drawn = true;
    }

    if (drawn)
	entrynum += 1;
    return entrynum;
}


void Paint_score_table(void)
{

    struct team_score	team[MAX_TEAMS],
			pausers,
			*team_order[MAX_TEAMS];
    other_t		*other,
			**order;
    int			i, j, entrynum = 0;

    if (!scoresChanged || !players_exposed)
	return;

    if (num_others < 1) {
	Paint_score_start();
	scoresChanged = false;
	return;
    }

    if ((order = (other_t **)malloc(num_others * sizeof(other_t *))) == NULL) {
	error("No memory for score");
	return;
    }
    if (BIT(Setup->mode, TEAM_PLAY|TIMING) == TEAM_PLAY) {
	memset(&team[0], 0, sizeof team);
	memset(&pausers, 0, sizeof pausers);
    }
    Determine_order(order, team);
    Paint_score_start();
    if (!(BIT(Setup->mode, TEAM_PLAY|TIMING) == TEAM_PLAY)) {
	for (i = 0; i < num_others; i++) {
	    other = order[i];
	    j = other - Others;
	    Paint_score_entry(i, other, false);
	}
    } else {
	Determine_team_order(team_order, team);

	/* add an empty line */
	entrynum++;
	for (i = 0; i < MAX_TEAMS; i++)
	    entrynum = Team_score_table(entrynum, i, team[i], order);
	/* paint pausers */
	entrynum = Team_score_table(entrynum, TEAM_PAUSEHACK, pausers, order);
#if 0
	for (i = 0; i < num_playing_teams; i++) {
	    entrynum = Team_heading(entrynum,
				    team_order[i] - &team[0],
				    team_order[i]->life,
				    team_order[i]->score);
	}
#endif
    }

    if (roundend)
	Print_roundend_messages(order);

    free(order);

    IFWINDOWS( MarkPlayersForRedraw() );

    scoresChanged = false;
}


static void Paint_clock(bool redraw)
{
    int			second,
			minute,
			hour,
			height = scoreListFont->ascent + scoreListFont->descent
				+ 3,
			border = 3;
    struct tm		*m;
    char		buf[16];
    static int		width;

    if (!clockColor) {
	if (width != 0) {
	    XSetForeground(dpy, scoreListGC, colors[windowColor].pixel);
	    /* kps - Vato does not want this to be done on windows */
	    IFNWINDOWS(XFillRectangle(dpy, playersWindow, scoreListGC,
				      256 - (width + 2 * border), 0,
				      width + 2 * border, height));
	    width = 0;
	}
	return;
    }

    if (!redraw && !newSecond)
	return;

    m = localtime(&currentTime);
    second = m->tm_sec;
    minute = m->tm_min;
    hour = m->tm_hour;
    /*warn("drawing clock at %02d:%02d:%02d", hour, minute, second);*/

    if (!BIT(instruments, SHOW_CLOCK_AMPM_FORMAT))
	sprintf(buf, "%02d:%02d" /*":%02d"*/, hour, minute /*, second*/);
    else {
	char tmpchar = 'A';
	/* strftime(buf, sizeof(buf), "%l:%M%p", m); */
	if (hour > 12){
	    tmpchar = 'P';
	    hour %= 12;
	}
	sprintf(buf, "%2d:%02d%cM", hour, minute, tmpchar);
    }
    width = XTextWidth(scoreListFont, buf, (int)strlen(buf));
    XSetForeground(dpy, scoreListGC, colors[windowColor].pixel);
    /* kps - Vato does not want this to be done on windows */ 
    IFNWINDOWS(XFillRectangle(dpy, playersWindow, scoreListGC,
			      256 - (width + 2 * border), 0,
			      width + 2 * border, height));
    ShadowDrawString(dpy, playersWindow, scoreListGC,
		     256 - (width + border),
		     scoreListFont->ascent + 4,
		     buf,
		     colors[clockColor].pixel,
		     colors[BLACK].pixel);
}


void ShadowDrawString(Display *display, Window w, GC gc,
		      int x, int y, const char* str,
		      unsigned long fg, unsigned long bg)
{
    XSetForeground(display, gc, bg);
    XDrawString(display, w, gc, x+1, y+1, str, strlen(str));
    x--; y--;
    XSetForeground(display, gc, fg);
    XDrawString(display, w, gc, x, y, str, strlen(str));
}

void Play_beep(void)
{
    XBell(dpy, 0);
    XFlush(dpy);
}

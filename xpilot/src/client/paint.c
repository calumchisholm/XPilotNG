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

#include "xpclient.h"

char paint_version[] = VERSION;

/*
 * Globals.
 */
bool roundend = false;
int killratio_kills = 0;
int killratio_deaths = 0;
int killratio_totalkills = 0;
int killratio_totaldeaths = 0;
int ballstats_cashes = 0;
int ballstats_replaces = 0;
int ballstats_teamcashes = 0;
int ballstats_lostballs = 0;
bool played_this_round = false;
int rounds_played = 0;

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
Window	msgWindow;		/* for meesages into the playfield */
Window	buttonWindow;		/* to calculate size of buttons */
#endif

Pixmap	p_draw;			/* Saved pixmap for the drawing */
				/* area (monochromes use this) */
Window	playersWindow;		/* Player list window */
				/* monochromes) */
int	maxMessages;		/* Max. number of messages to display */
int	messagesToStdout;	/* Send messages to standard output */
Window	about_w;		/* About window */
Window	about_close_b;		/* About window's close button */
Window	about_next_b;		/* About window's next button */
Window	about_prev_b;		/* About window's previous button */
Window	keys_close_b;		/* Help window's close button */
Window	talk_w;			/* Talk window */
XColor	colors[MAX_COLORS];	/* Colors */
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
char	modBankStr[NUM_MODBANKS][MAX_CHARS];	/* modifier banks */
char	*texturePath = NULL;		/* Path list of texture directories */
bool	useErase;		/* use Erase hack for slow X */

int		maxKeyDefs;
keydefs_t	*keyDefs = NULL;

other_t		*self;		/* player info */

long		loops = 0;

unsigned long	loopsSlow = 0;	/* Proceeds slower than loops */
int		clientFPS = 1;	/* How many fps we actually paint */
static time_t	old_time = 0;	/* Previous value of time */
time_t		currentTime;	/* Current value of time() */
bool		newSecond = false; /* True if time() incremented this frame */
static int	frame_count = 0;/* Used to estimate client fps */
DFLOAT		timePerFrame = 0.0;/* How much real time proceeds per frame */
static DFLOAT	time_counter = 0.0;

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

    if (start_loops != end_loops)
	warn("Start neq. End (%ld,%ld,%ld)", start_loops, end_loops, loops);
    loops = end_loops;

    frame_count++;

    currentTime = time(NULL);
    if (currentTime != old_time) {
	old_time = currentTime;
	newSecond = true;
    } else
	newSecond = false;

    /*
     * If time() changed from previous value, assume one second has passed.
     */
    if (newSecond) {
	clientFPS = frame_count;
	/* kps - improve */
	recordFPS = clientFPS;
	frame_count = 0;
	/*
	 * I've changed some places that used FPS (from setup) in client
	 * to use clientFPS, to allow client to adapt to server changing
	 * FPS dynamically ("/set fps 100" command on server).
	 */
	if (clientFPS <= 0)
	    clientFPS = 1;
	timePerFrame = 1.0 / clientFPS;

	/* check once per second if we are playing */
	if (self && !strchr("PW", self->mychar))
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
    p_draw = draw;		/* let's try this */
    XSetForeground(dpy, gameGC, colors[BLACK].pixel);
    XFillRectangle(dpy, p_draw, gameGC, 0, 0, draw_width, draw_height);
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

	Erase_start();

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
    if (p_radar != radarWindow && radar_exposures > 0) {
	if (BIT(instruments, SHOW_SLIDING_RADAR) == 0
	    || BIT(Setup->mode, WRAP_PLAY) == 0) {
#ifndef _WINDOWS
	    XCopyArea(dpy, p_radar, radarWindow, gameGC,
		      0, 0, 256, RadarHeight, 0, 0);
#else
	    WinXBltPixToWin(p_radar, radarWindow,
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
	    XCopyArea(dpy, p_radar, radarWindow, gameGC,
		      0, 0, x, y, w, h);
	    XCopyArea(dpy, p_radar, radarWindow, gameGC,
		      x, 0, w, y, 0, h);
	    XCopyArea(dpy, p_radar, radarWindow, gameGC,
		      0, y, x, h, w, 0);
	    XCopyArea(dpy, p_radar, radarWindow, gameGC,
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
	XCopyArea(dpy, p_draw, drawWindow, gameGC,
		  0, 0, draw_width, draw_height, 0, 0);

    dbuff_switch(dbuf_state);

    if (dbuf_state->type == COLOR_SWITCH) {
	XSetPlaneMask(dpy, gameGC, dbuf_state->drawing_planes);
	XSetPlaneMask(dpy, messageGC, dbuf_state->drawing_planes);
    }
#endif

    if (!damaged) {
	/* Prepare invisible buffer for next frame by clearing. */
	if (useErase)
	    Erase_end();
	else {
	    /*
	     * DBE's XdbeBackground switch option is
	     * probably faster than XFillRectangle.
	     */
#ifndef _WINDOWS
	    if (dbuf_state->multibuffer_type != MULTIBUFFER_DBE) {
		SET_FG(colors[BLACK].pixel);
		XFillRectangle(dpy, p_draw, gameGC,
			       0, 0, draw_width, draw_height);
	    }
#endif
	}
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
    Client_score_table();
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
	if (BIT(Setup->mode, TIMING)) {
	    if (version >= 0x3261)
		strcat(headingStr, "LAP ");
	}
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
	    if (version >= 0x3261) {
		if ((other->mychar == ' ' || other->mychar == 'R')
		    && other->round + other->check > 0) {
		    if (other->round > 99)
			sprintf(raceStr, "%3d", other->round);
		    else
			sprintf(raceStr, "%d.%c",
				other->round, other->check + 'a');
		}
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
    width = XTextWidth(scoreListFont, buf, strlen(buf));
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

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
/*#define TIME_CODE*/
#ifdef TIME_CODE
    #include <sys/time.h>
#endif

#include "xpclient.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_gfxPrimitives.h"
#include "sdlpaint.h"
#include "images.h"
#include "console.h"
#include "radar.h"
#include "sdlwindow.h"
#include "text.h"
#include "glwidgets.h"

#define SCORE_BORDER 5

char sdlpaint_version[] = VERSION;

/*
 * Globals.
 */
double	        hudRadarLimit;		/* Limit for hudradar drawing */

static double       time_counter = 0.0;
static TTF_Font     *scoreListFont;
static char         *scoreListFontName = "VeraMoBd.ttf";
static sdl_window_t scoreListWin;
static SDL_Rect     scoreEntryRect; /* Bounds for the last painted score entry */
static guiarea_t    *scoreListArea;
static bool         scoreListMoving;
static widget_list_t *ScoreList_widgetItem;
static guiarea_t    *window_guiarea;

static widget_list_t *ConfMenu;
static GLWidget *testWidget[3];
static int testValue;
static int testMinValue;
static int testMaxValue;
static double testValue2;
static bool testValue3;
static double testMinValue2;
static double testMaxValue2;

void select_button(Uint8 button,Uint8 state,Uint16 x,Uint16 y, void *data)
{
    if (state == SDL_PRESSED) {

	if (button == 1) {
	    select_bounds = malloc(sizeof(irec));
	    if ( !select_bounds )
	    	error("Can't malloc select_bounds!");
	    select_bounds->x = x;
	    select_bounds->y = y;
	    select_bounds->w = 0;
	    select_bounds->h = 0;
	}
    }
    
    if (state == SDL_RELEASED) {
	if ((button == 1) && select_bounds) {
	    free(select_bounds);
	    select_bounds = NULL;
	}
    }
}

void select_move(Sint16 xrel,Sint16 yrel,Uint16 x,Uint16 y, void *data)
{
    if(select_bounds) {
    	select_bounds->w += xrel;
    	select_bounds->h += yrel;
    }
}

/* function to reset our viewport after a window resize */
int Resize_Window( int width, int height )
{
    extern int videoFlags;
    
    draw_width = width;
    draw_height = height;
    window_guiarea->bounds.w = width;
    window_guiarea->bounds.h = height;
    
    if (!SDL_SetVideoMode( width,
			   height,
			   draw_depth, 
			   videoFlags ))
	return -1;
    

    //    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );

    glLoadIdentity( );

    gluOrtho2D(0, draw_width, 0, draw_height);
    
    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );
    
    /* Reset The View */
    glLoadIdentity( );

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )draw_width, ( GLint )draw_height );
    return 0;
}

static void Scorelist_button(Uint8 button, Uint8 state, Uint16 x, Uint16 y, void *data)
{
    if (state == SDL_PRESSED) {
    	if (button == 1)
	    scoreListMoving = true;
    }
    
    if (state == SDL_RELEASED) {
    	if (button == 1)
	    scoreListMoving = false;
    }
}

static void Scorelist_move(Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data)
{
    if (scoreListMoving) {
	scoreListWin.x += xrel;
	scoreListWin.y += yrel;
	scoreListArea->bounds.x = scoreListWin.x;
	scoreListArea->bounds.y = scoreListWin.y;
    }
}


static void Scorelist_cleanup(void)
{
    TTF_CloseFont(scoreListFont);
    sdl_window_destroy(&scoreListWin);
    unregister_guiarea(scoreListArea);
}

static void Scorelist_reg(widget_list_t *LI)
{
    scoreListArea = register_guiarea(*(SDL_Rect *)(LI->GuiRegData), Scorelist_button, NULL, Scorelist_move, NULL, NULL, NULL);
}

static void Scorelist_paint(widget_list_t *LI)
{
    if (scoresChanged) {
	/* This is the easiest way to track if
	 * the height of the score window should be changed */
	int y = scoreEntryRect.y;
        Paint_score_table();
	if (y != scoreEntryRect.y) {
	    sdl_window_resize(&scoreListWin, scoreListWin.w,
			      scoreEntryRect.y + scoreEntryRect.h
			      + 2 * SCORE_BORDER);
	    /* Unfortunately the resize loses the surface
	     * so I have to repaint it */
	    scoresChanged = 1;
	    Paint_score_table();
	    scoreListArea->bounds.w = scoreListWin.w;
	    scoreListArea->bounds.h = scoreListWin.h;
	}
	sdl_window_refresh(&scoreListWin);
    }
    glColor4ub(0, 0x20, 0, 0x90);
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    glVertex2i(scoreListWin.x, scoreListWin.y + scoreListWin.h + 2);    
    glVertex2i(scoreListWin.x, scoreListWin.y);
    glVertex2i(scoreListWin.x + scoreListWin.w, scoreListWin.y);
    glVertex2i(scoreListWin.x + scoreListWin.w, 
               scoreListWin.y + scoreListWin.h + 2);
    glEnd();
    sdl_window_paint(&scoreListWin);
    glBegin(GL_LINE_LOOP);
    glColor4ub(0, 0, 0, 0xff);
    glVertex2i(scoreListWin.x, scoreListWin.y + scoreListWin.h + 2);    
    glColor4ub(0, 0x90, 0x00, 0xff);
    glVertex2i(scoreListWin.x, scoreListWin.y);
    glColor4ub(0, 0, 0, 0xff);
    glVertex2i(scoreListWin.x + scoreListWin.w, scoreListWin.y);
    glColor4ub(0, 0x90, 0x00, 0xff);
    glVertex2i(scoreListWin.x + scoreListWin.w, 
               scoreListWin.y + scoreListWin.h + 2);
    glEnd();
}

static int Scorelist_init(void)
{
    static SDL_Rect r = { 10, 240, 200, 100 };
    scoreListFont = TTF_OpenFont(scoreListFontName, 11);
    if (scoreListFont == NULL) {
	error("opening font %s failed", scoreListFontName);
	return -1;
    }
    if (sdl_window_init(&scoreListWin, r.x, r.y, r.w, r.h)) {
	error("failed to init scorelist window");
	return -1;
    }
    ScoreList_widgetItem = AppendListItem(MainList,Scorelist_paint,NULL,Scorelist_reg,&r,NULL,NULL);
    return 0;
}
#ifndef _WINDOWS
int InitConfMenu(void)
{
    SDL_Rect bounds;
    if (!(ConfMenu=MakeWidgetList(NULL,NULL,NULL,NULL,NULL,NULL))) {
	error("ConfMenu widget-list initialization failed");
	return 1;
    }
    testValue = 999;
    testMinValue = 0;
    testMaxValue = 998;
    testValue2 = 9.99;
    testMinValue2 = 0.0;
    testMaxValue2 = 9.98;
    testWidget[0] = Init_IntChooserWidget(ConfMenu, &gamefont, "testValue1", &testValue, &testMinValue, &testMaxValue, NULL, NULL);
    testWidget[1] = Init_DoubleChooserWidget(ConfMenu, &gamefont, "testValue2", &testValue2, &testMinValue2, &testMaxValue2, NULL, NULL);
    testWidget[2] = Init_BoolChooserWidget(ConfMenu, &gamefont, "showTexturedWalls", &(instruments.showTexturedWalls), NULL, NULL);

    bounds.x = 500;
    bounds.y = 0;
    bounds.w = testWidget[0]->bounds.w;
    bounds.h = testWidget[0]->bounds.h+10;
    SetBounds_GLWidget(testWidget[0],&bounds);
    bounds.x = 500;
    bounds.y = testWidget[0]->bounds.h+1;
    bounds.w = testWidget[1]->bounds.w;
    bounds.h = testWidget[1]->bounds.h;
    SetBounds_GLWidget(testWidget[1],&bounds);
    bounds.x = 500;
    bounds.y = testWidget[1]->bounds.y+testWidget[1]->bounds.h+1;
    bounds.w = testWidget[2]->bounds.w;
    bounds.h = testWidget[2]->bounds.h;
    SetBounds_GLWidget(testWidget[2],&bounds);
   
    AddListGuiAreas(ConfMenu);
    return 0;
}

void CloseConfMenu(void)
{
    if (!ConfMenu) return;
    DelListGuiAreas(ConfMenu);
    CleanList(ConfMenu);
    Close_Widget(testWidget[0]);
    Close_Widget(testWidget[1]);
}
#endif
bool Set_scaleFactor(xp_option_t *opt, double val)
{
    scaleFactor = val;
    scale = 1 / val;
    return true;
}

bool Set_altScaleFactor(xp_option_t *opt, double val)
{
    scaleFactor_s = val;
    return true;
}

int Paint_init(void)
{
    extern bool players_exposed; /* paint.c */
    int i;
	SDL_Rect bounds;

    if (TTF_Init()) {
	error("SDL_ttf initialization failed: %s", SDL_GetError());
	return -1;
    }
    xpprintf("SDL_ttf initialized\n");

    if (Init_wreckage() == -1)
	return -1;
    
    if (Init_asteroids() == -1)
	return -1;

    if (Images_init() == -1) 
	return -1;

    select_bounds = NULL;
    bounds.x = bounds.y = 0;
	bounds.w = draw_width;
	bounds.h = draw_height;
    window_guiarea = register_guiarea(bounds,select_button,NULL,select_move,NULL,NULL,NULL);

    for (i=0;i<MAX_SCORE_OBJECTS;++i)
    	score_object_texs[i].texture = 0;
    for (i=0;i<MAX_METERS;++i)
    	meter_texs[i].texture = 0;
    for (i=0;i<2*MAX_MSGS;++i)
    	message_texs[i].texture = 0;
    
    if (!(MainList=MakeWidgetList(NULL,NULL,NULL,NULL,NULL,NULL))) {
	error("Main widget-list initialization failed");
	return -1;
    }
    if (Scorelist_init() == -1)
	return -1;

    scoresChanged = 1;
    players_exposed = true;
    
    return 0;
}

void Paint_cleanup(void)
{
    int i;
    /*CloseConfMenu();*/
    Scorelist_cleanup();
    Images_cleanup();
    TTF_Quit();
    clean_guiarea_list();

    for (i=0;i<MAX_SCORE_OBJECTS;++i)
    	if (score_object_texs[i].texture) free_string_texture(&score_object_texs[i]);
    for (i=0;i<MAX_METERS;++i)
    	if (meter_texs[i].texture) free_string_texture(&meter_texs[i]);
    for (i=0;i<2*MAX_MSGS;++i)
    	if (message_texs[i].texture) free_string_texture(&message_texs[i]);
}

/* kps - can we rather use Check_view_dimensions in paint.c ? */
#if 0
int Check_view_dimensions(void)
{
    int			width_wanted, height_wanted;
    int			srv_width, srv_height;

    /* TODO: once the scaling is implemented these should be computed
     * somehow. For the moment they equal to draw_*
    width_wanted = (int)(draw_width * scaleFactor + 0.5);
    height_wanted = (int)(draw_height * scaleFactor + 0.5);
    */
    width_wanted = draw_width / scale;
    height_wanted = draw_height / scale;

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
#endif

/* This one works best for things that are fixed in position
 * since they won't appear to move relative to eachother
 */
void setupPaint_stationary(void)
{
    if (paintSetupMode & STATIONARY_MODE) return;
    paintSetupMode = STATIONARY_MODE;
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(rint(-world.x * scale), rint(-world.y * scale), 0);
    glScalef(scale, scale, 0);
}

/* This one works best for things that move, since they don't get
 * painted differently depending on map position
 */
void setupPaint_moving(void)
{
    if (paintSetupMode & MOVING_MODE) return;
    paintSetupMode = MOVING_MODE;
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-world.x * scale, -world.y * scale, 0);
    glScalef(scale, scale, 0);
}

void setupPaint_HUD(void)
{
    if (paintSetupMode & HUD_MODE) return;
    paintSetupMode = HUD_MODE;
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, draw_width, draw_height, 0);
}

void Paint_frame(void)
{
    Check_view_dimensions();
#ifdef TIME_CODE
    static struct timeval timed[17][2];
    int i;
    int measuretime = 100;    
#endif
    
    world.x = selfPos.x - (ext_view_width / 2);
    world.y = selfPos.y - (ext_view_height / 2);
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

    if (damaged <= 0) {
#ifdef TIME_CODE
	gettimeofday(&timed[0][1],NULL);
#endif
    	/*glClear(GL_COLOR_BUFFER_BIT);*/
	/* on my machine this seems about 10 times faster
	 * with seemingly the same result
	 */
	set_alphacolor(blackRGBA);
	glBegin(GL_QUADS);
	    glVertex2i(0,0);
	    glVertex2i(draw_width,0);
	    glVertex2i(draw_width,draw_height);
	    glVertex2i(0,draw_height);
	glEnd();

#ifdef TIME_CODE
	gettimeofday(&timed[1][1],NULL);
#endif
	glEnable(GL_BLEND);
    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* This one works best for things that are fixed in position
	 * since they won't appear to move relative to eachother
	 */
    	setupPaint_stationary();
	
    	Paint_world();
#ifdef TIME_CODE
	gettimeofday(&timed[2][1],NULL);
#endif

	if (oldServer) {
	    Paint_vfuel();
	    Paint_vdecor();
	    Paint_vcannon();
	    Paint_vbase();
	} else
	    Paint_objects();

#ifdef TIME_CODE
	gettimeofday(&timed[3][1],NULL);
#endif

    	Paint_score_objects();
	
#ifdef TIME_CODE
	gettimeofday(&timed[4][1],NULL);
#endif
	
	Paint_shots();
#ifdef TIME_CODE
	gettimeofday(&timed[5][1],NULL);
#endif
	setupPaint_moving();
	Paint_ships();
#ifdef TIME_CODE
	gettimeofday(&timed[6][1],NULL);
#endif

	setupPaint_HUD();

#ifdef TIME_CODE
	gettimeofday(&timed[7][1],NULL);
#endif
    	Paint_meters();
#ifdef TIME_CODE
	gettimeofday(&timed[8][1],NULL);
#endif
    	Paint_HUD();
#ifdef TIME_CODE
	gettimeofday(&timed[9][1],NULL);
#endif
    	Paint_client_fps();
#ifdef TIME_CODE
	gettimeofday(&timed[10][1],NULL);
#endif

	Paint_messages();       
#ifdef TIME_CODE
	gettimeofday(&timed[11][1],NULL);
#endif
	Console_paint();
#ifdef TIME_CODE
	gettimeofday(&timed[12][1],NULL);
#endif
	Paint_select();
#ifdef TIME_CODE
	gettimeofday(&timed[13][1],NULL);
#endif
	if (MainList)
	    DrawWidgetList(MainList);
#ifdef TIME_CODE
	gettimeofday(&timed[14][1],NULL);
#endif
	/*if (ConfMenu)
	    DrawWidgetList(ConfMenu);
    	else if (InitConfMenu()) error("ConfMenu initialization failed");*/
    
#ifdef TIME_CODE
    	gettimeofday(&timed[15][1],NULL);
#endif
	
	
	glPopMatrix();
    }
    
    SDL_GL_SwapBuffers();
#ifdef TIME_CODE
    gettimeofday(&timed[16][1],NULL);

    for (i=16;i>0;--i)
    	timed[i][1].tv_usec = timed[i][1].tv_usec - timed[i-1][1].tv_usec
    			    + ((timed[i][1].tv_sec - timed[i-1][1].tv_sec)*1000000);
    for (i=0;i<17;++i)
    	timed[i][0].tv_usec += timed[i][1].tv_usec;
    
    if (!(loops%measuretime)) {
    	xpprintf("----------Paint_foo times----------\n");
    	for (i=1;i<17;++i)
    	    xpprintf("<%li\t%2i>\n",timed[i][0].tv_usec,i);
    	for (i=1;i<16;++i)
    	    timed[16][0].tv_usec += timed[i][0].tv_usec;
    	xpprintf("**[ total %li µs over %i frames ]**\n",timed[16][0].tv_usec,measuretime);
    for (i=0;i<17;++i)
    	timed[i][0].tv_usec = 0;
    	
    }
#endif
}

void Paint_score_start(void)
{
    char	headingStr[MSG_LEN];
    SDL_Surface *header;
	SDL_Color fg;

    if (showUserName)
	strlcpy(headingStr, "NICK=USER@HOST", sizeof(headingStr));
    else if (BIT(Setup->mode, TEAM_PLAY))
	strlcpy(headingStr, "  SCORE NAME           LIFE", sizeof(headingStr));
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
	
    fg.r = (scoreColorRGBA >> 24) & 255;
	fg.g = (scoreColorRGBA >> 16) & 255;
	fg.b = (scoreColorRGBA >> 8) & 255;
	fg.unused = scoreColorRGBA & 255;
    SDL_FillRect(scoreListWin.surface, NULL, 0);
    header = TTF_RenderText_Blended(scoreListFont, headingStr, fg);
    if (header == NULL) {
	error("scorelist header rendering failed: %s", SDL_GetError());
	return;
    }
    scoreEntryRect.x = scoreEntryRect.y = SCORE_BORDER;
    SDL_SetAlpha(header, 0, 0);
    SDL_BlitSurface(header, NULL, scoreListWin.surface, &scoreEntryRect);
    lineRGBA(scoreListWin.surface, SCORE_BORDER,
	     scoreEntryRect.y + header->h + 2,
	     scoreListWin.w - SCORE_BORDER,
	     scoreEntryRect.y + header->h + 2,
	     0, 128, 0, 255);
    SDL_FreeSurface(header);
}

void Paint_score_entry(int entry_num, other_t *other, bool is_team)
{
    static char		raceStr[8], teamStr[4], lifeStr[8], label[MSG_LEN];
    static int		lineSpacing = -1, firstLine;
    char		scoreStr[16];
    SDL_Surface         *line;
	SDL_Color fg;
    int     	    	color;

    /*
     * First time we're here, set up miscellaneous strings for
     * efficiency and calculate some other constants.
     */
    if (lineSpacing == -1) {
	memset(raceStr, 0, sizeof raceStr);
	memset(teamStr, 0, sizeof teamStr);
	memset(lifeStr, 0, sizeof lifeStr);
	teamStr[1] = ' ';
	raceStr[2] = ' ';

	lineSpacing = TTF_FontLineSkip(scoreListFont) + 1;
	firstLine = 2*SCORE_BORDER + lineSpacing;
    }
    scoreEntryRect.y = firstLine + lineSpacing * entry_num;

    /*
     * Setup the status line
     */
    if (showUserName)
	sprintf(label, "%s=%s@%s",
		other->nick_name, other->user_name, other->host_name);
    else {
	other_t *war = Other_by_id(other->war_id);

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
		    7 - showScoreDecimals, showScoreDecimals,
		    other->score);
	else {
	    int sc = rint(other->score);
	    sprintf(scoreStr, "%6d", sc);
	}

	if (BIT(Setup->mode, TEAM_PLAY))
	    sprintf(label, "%c%s %-15s%s",
		    other->mychar, scoreStr, other->nick_name, lifeStr);
	else {
	    sprintf(label, "%c %s%s%s%s  %s",
		    other->mychar, raceStr, teamStr,
		    scoreStr, lifeStr,
		    other->nick_name);
	    if (war) {
		if (strlen(label) + strlen(war->nick_name) + 5 < sizeof(label))
		    sprintf(label + strlen(label), " (%s)", war->nick_name);
	    }
	}
    }

    /*
     * Draw the line
     * e94_msu eKthHacks
     */
    if (!is_team && strchr("DPW", other->mychar)) {
	if (other->id == self->id)
	    color = scoreInactiveSelfColorRGBA;
	else
	    color = scoreInactiveColorRGBA;
    } else {
	if (!is_team) {
	    if (other->id == self->id)
		color = scoreSelfColorRGBA;
	    else
		color = scoreColorRGBA;
	} else {
	    color = Team_color(other->team);
	    if (!color) {
		if (other->team == self->team)
		    color = scoreOwnTeamColorRGBA;
		else
		    color = scoreEnemyTeamColorRGBA;
	    }
	}
    }
    fg.r = (color >> 24) & 255;
	fg.g = (color >> 16) & 255;
	fg.b = (color >> 8) & 255;
	fg.unused = color & 255;
    line = TTF_RenderText_Blended(scoreListFont, label, fg);
    if (line == NULL) {
	error("scorelist rendering failed: %s", SDL_GetError());
	return;
    }
    SDL_SetAlpha(line, 0, 0);
    SDL_BlitSurface(line, NULL, scoreListWin.surface, &scoreEntryRect);
    scoreEntryRect.h = line->h;

    /*
     * Underline the teams
     */
    if (is_team) {
	lineRGBA(scoreListWin.surface, scoreEntryRect.x, 
		 scoreEntryRect.y + line->h - 1,
		 scoreEntryRect.x + scoreEntryRect.w,
		 scoreEntryRect.y + line->h - 1,
		 fg.r, fg.g, fg.b, 255);
    }

    SDL_FreeSurface(line);
}


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

#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WINDOWS
#ifndef	_WINSOCKAPI_
#include <winsock.h>
#endif

#ifndef	_WINX_H_
#include "NT/winX.h"
#endif
#endif


#ifndef DRAW_H
/* need shipobj */
#include "draw.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif


#define SHOW_SLIDING_RADAR	(1L << 8)
#define SHOW_MESSAGES		(1L << 15)

#define SHOW_OUTLINE_WORLD	(1L << 17)
#define SHOW_FILLED_WORLD	(1L << 18)
#define SHOW_TEXTURED_WALLS	(1L << 19)
#define SHOW_DECOR		(1L << 20)
#define SHOW_OUTLINE_DECOR	(1L << 21)
#define SHOW_FILLED_DECOR	(1L << 22)
#define SHOW_TEXTURED_DECOR	(1L << 23)
#define SHOW_CLOCK_AMPM_FORMAT	(1L << 24)

#define SHOW_REVERSE_SCROLL	(1L << 26)

#define SHOW_SHIP_ID            (1L << 29)

/* hacked instruments */
#define SHOW_SHIP_SHAPES	(1L << 2)
#define SHOW_MY_SHIP_SHAPE	(1L << 3)
#define BALL_MSG_SCAN		(1L << 6)
#define CLIENT_RANKER		(1L << 7)
#define MAP_RADAR		(1L << 12)
#define SHOW_LIVES_BY_SHIP	(1L << 13)
#define TREAT_ZERO_SPECIAL	(1L << 14)
#define SHOW_EXTRA_BASE_INFO	(1L << 15)


#define PACKET_LOSS		0
#define PACKET_DROP		1
#define PACKET_DRAW		2

#define MAX_SCORE_OBJECTS	10

#define MAX_SPARK_SIZE		8
#define MIN_SPARK_SIZE		1
#define MAX_MAP_POINT_SIZE	8
#define MIN_MAP_POINT_SIZE	0
#define MAX_SHOT_SIZE		8
#define MIN_SHOT_SIZE		1
#define MAX_TEAMSHOT_SIZE	8
#define MIN_TEAMSHOT_SIZE	1

#define MIN_SHOW_ITEMS_TIME	0.0
#define MAX_SHOW_ITEMS_TIME	300.0

#define MIN_SCALEFACTOR		0.2
#define MAX_SCALEFACTOR		8.0

#if 0
#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->name);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->name,	\
					 (other)->name_len);		\
    }
#endif /* 0 */

#define FIND_NAME_WIDTH(other)						\
    if ((other)->name_width == 0) {					\
	(other)->name_len = strlen((other)->id_string);			\
	(other)->name_width = 2 + XTextWidth(gameFont, (other)->id_string,\
					 (other)->name_len);		\
    }


typedef struct {
    DFLOAT	ratio;
    short	id;
    short	team;
    DFLOAT	score;
    short	check;
    short	round;
    short	timing;
    long	timing_loops;
    short	life;
    short	mychar;
    short	alliance;
    short	war_id;
    short	name_width;	/* In pixels */
    short	name_len;	/* In bytes */
    shipobj	*ship;
    char	name[MAX_CHARS];
    char	real[MAX_CHARS];
    char	host[MAX_CHARS];
    char	id_string[MAX_CHARS];
    short	ignorelevel;
} other_t;

typedef struct {
    int		pos;		/* Block index */
    long	fuel;		/* Amount of fuel available */
    irec	bounds;		/* Location on map */
} fuelstation_t;

typedef struct {
    int		pos;		/* Block index */
    short	id,		/* Id of owner or -1 */
		team;		/* Team this base belongs to */
    irec	bounds;		/* Location on map */
    int		type;		/* orientation */
    int		deathtime;	/* For base warning */
} homebase_t;

typedef struct {
    int		pos;		/* Block index */
    short	dead_time,	/* Frames inactive */
		dot;		/* Draw dot if inactive */
} cannontime_t;

typedef struct {
    int		pos;		/* Block index */
    short	dead_time;	/* Frames inactive */
    unsigned short	damage;		/* Damage to target */
} target_t;

typedef struct {
    int		pos;		/* Block index */
    irec	bounds;		/* Location on map */
} checkpoint_t;



typedef struct {
    int width;			/* Line width, -1 means no line */
    int color;			/* Line color */
    int rgb;			/* RGB values corresponding to color */
    int style;			/* LineSolid, LineOnOffDash, LineDoubleDash */
} edge_style_t;

typedef struct {
    int color;			/* The color if drawn in filled mode */
    int rgb;			/* RGB values corresponding to color */
    int texture;		/* The texture if drawn in texture mode */
    int flags;			/* Flags about this style (see draw.h) */
    int def_edge_style;		/* The default style for edges */
} polygon_style_t;

typedef struct {
    ipos *points;		/* points[0] is absolute, rest are relative */
    int num_points;		/* number of points */
    irec bounds;		/* bounding box for the polygon */
    int *edge_styles;		/* optional array of indexes to edge_styles */
    int style;			/* index to polygon_styles array */
} xp_polygon_t;



/*#define SCORE_OBJECT_COUNT	100*/
typedef struct {
    DFLOAT	score,
		life_time;
    int		x,
		y,
		hud_msg_len,
		hud_msg_width,
		msg_width,
		msg_len;
    char	msg[10],
		hud_msg[MAX_CHARS+10];
} score_object_t;


/*
 * is a selection pending (in progress), done, drawn emphasized?
 */
#define SEL_NONE       (1 << 0)
#define SEL_PENDING    (1 << 1)
#define SEL_SELECTED   (1 << 2)
#define SEL_EMPHASIZED (1 << 3)

/*
 * a selection (text, string indices, state,...)
 */
typedef struct {
    /* a selection in the talk window */
    struct {
        bool    state;	/* current state of the selection */
        int     x1;	/* string indices */
        int     x2;
        bool    incl_nl;/* include a `\n'? */
    } talk ;
    /* a selection in the draw window */
    struct {
        bool    state;
        int     x1;	/* string indices (for TalkMsg[].txt) */
        int     x2;	/* they are modified when the emphasized area */
        int     y1;	/* is scrolled down by new messages coming in */
        int     y2;
    } draw;
    char	*txt;   /* allocated when needed */
    int		txt_size;	/* size of txt buffer */
    int		len;
    /* when a message `jumps' from talk window to the player messages: */
    bool	keep_emphasizing;
} selection_t;


extern int      oldServer; /* Compatibility mode for old block-based servers */
extern ipos	pos;
extern ipos	vel;
extern ipos	world;
extern ipos	realWorld;
extern short	heading;
extern short	nextCheckPoint;
extern u_byte	numItems[NUM_ITEMS];
extern u_byte	lastNumItems[NUM_ITEMS];
extern int	numItemsTime[NUM_ITEMS];
extern DFLOAT	showItemsTime;
extern short	autopilotLight;
extern int	showScoreDecimals;


extern short	lock_id;		/* Id of player locked onto */
extern short	lock_dir;		/* Direction of lock */
extern short	lock_dist;		/* Distance to player locked onto */

extern other_t*	self;			/* Player info */
extern short	selfVisible;		/* Are we alive and playing? */
extern short	damaged;		/* Damaged by ECM */
extern short	destruct;		/* If self destructing */
extern short	shutdown_delay;
extern short	shutdown_count;
extern short	thrusttime;
extern short	thrusttimemax;
extern short	shieldtime;
extern short	shieldtimemax;
extern short	phasingtime;
extern short	phasingtimemax;

extern int		roundDelay;
extern int		roundDelayMax;

extern int	RadarWidth;
extern int	RadarHeight;
extern int	map_point_distance;	/* spacing of navigation points */
extern int	map_point_size;		/* size of navigation points */
extern int	spark_size;		/* size of sparks and debris */
extern int	shot_size;		/* size of shot */
extern int	teamshot_size;		/* size of team shot */
extern bool	showNastyShots;		/* show original flavor shots or the new "nasty shots" */
extern DFLOAT	controlTime;		/* Display control for how long? */
extern u_byte	spark_rand;		/* Sparkling effect */
extern u_byte	old_spark_rand;		/* previous value of spark_rand */

extern long	fuelSum;		/* Sum of fuel in all tanks */
extern long	fuelMax;		/* How much fuel can you take? */
extern short	fuelCurrent;		/* Number of currently used tank */
extern short	numTanks;		/* Number of tanks */
extern DFLOAT	fuelTime;		/* Display fuel for how long? */
extern int	fuelLevel1;		/* Fuel critical level */
extern int	fuelLevel2;		/* Fuel warning level */
extern int	fuelLevel3;		/* Fuel notify level */

extern char	*shipShape;		/* Shape of player's ship */
extern DFLOAT	power;			/* Force of thrust */
extern DFLOAT	power_s;		/* Saved power fiks */
extern DFLOAT	turnspeed;		/* How fast player acc-turns */
extern DFLOAT	turnspeed_s;		/* Saved turnspeed */
extern DFLOAT	turnresistance;		/* How much is lost in % */
extern DFLOAT	turnresistance_s;	/* Saved (see above) */
extern DFLOAT	displayedPower;		/* What the server is sending us */
extern DFLOAT	displayedTurnspeed;	/* What the server is sending us */
extern DFLOAT	displayedTurnresistance;/* What the server is sending us */
extern DFLOAT	spark_prob;		/* Sparkling effect configurable */
extern int	charsPerSecond;		/* Message output speed (config) */

extern DFLOAT	hud_move_fact;		/* scale the hud-movement (speed) */
extern DFLOAT	ptr_move_fact;		/* scale the speed pointer length */
extern char	mods[MAX_CHARS];	/* Current modifiers in effect */
extern long	instruments;		/* Instruments on screen (bitmask) */
extern long	hackedInstruments;	/* Hacked instruments on screen (bitmask) */
extern int	packet_size;		/* Current frame update packet size */
extern int	packet_loss;		/* lost packets per second */
extern int	packet_drop;		/* dropped packets per second */
extern int	packet_lag;		/* approximate lag in frames */
extern char	*packet_measure;	/* packet measurement in a second */
extern long	packet_loop;		/* start of measurement */

extern bool	showRealName;		/* Show realname instead of nickname */
extern char	name[MAX_CHARS];	/* Nick-name of player */
extern char	realname[MAX_CHARS];	/* Real name of player */
extern char	servername[MAX_CHARS];	/* Name of server connecting to */
extern unsigned	version;		/* Version of the server */
extern int	scoresChanged;
extern int	toggle_shield;		/* Are shields toggled by a press? */
extern int	shields;		/* When shields are considered up */
extern int	auto_shield;            /* drops shield for fire */
extern int	initialPointerControl;	/* Start by using mouse for control? */
extern int	pointerControl;		/* current state of mouse ship flying */
extern bool	useErase;		/* use the Erase hack for slow X */

extern int	maxFPS;			/* Client's own FPS */
extern int 	oldMaxFPS;

extern int	clientPortStart;	/* First UDP port for clients */
extern int	clientPortEnd;		/* Last one (these are for firewalls) */

extern byte	lose_item;		/* flag and index to drop item */
extern int	lose_item_active;	/* one of the lose keys is pressed */

#ifdef SOUND
extern char 	sounds[MAX_CHARS];	/* audio mappings */
extern char 	audioServer[MAX_CHARS];	/* audio server */
extern int 	maxVolume;		/* maximum volume (in percent) */
#endif /* SOUND */

extern int	maxLinesInHistory;	/* number of lines to save in history */
#define MAX_HIST_MSGS	128		/* maximum */

/* mapdata accessible to outside world */

extern fuelstation_t *fuels;
extern int num_fuels;

extern homebase_t *bases;
extern int num_bases;

extern checkpoint_t *checks;
extern int num_checks;

extern xp_polygon_t *polygons;
extern int num_polygons, max_polygons;

extern edge_style_t *edge_styles;
extern int num_edge_styles, max_edge_styles;

extern polygon_style_t *polygon_styles;
extern int num_polygon_styles, max_polygon_styles;

extern other_t *Others;
extern int	num_others, max_others;

extern int num_playing_teams;

int Fuel_by_pos(int x, int y);
int Target_alive(int x, int y, int *damage);
int Target_by_index(int ind, int *xp, int *yp, int *dead_time, int *damage);
int Handle_fuel(int ind, int fuel);
int Cannon_dead_time_by_pos(int x, int y, int *dot);
int Handle_cannon(int ind, int dead_time);
int Handle_target(int num, int dead_time, int damage);
int Base_info_by_pos(int x, int y, int *id, int *team);
int Handle_base(int id, int ind);
int Check_pos_by_index(int ind, int *xp, int *yp);
int Check_index_by_pos(int x, int y);
other_t *Other_by_id(int id);
other_t *Other_by_name(char *name); /* Only used by message scan hack */
shipobj *Ship_by_id(int id);
int Handle_leave(int id);
int Handle_player(int id, int team, int mychar, char *player_name,
		  char *real_name, char *host_name, char *shape,
		  int myself);
int Handle_score(int id, DFLOAT score, int life, int mychar, int alliance);
int Handle_score_object(DFLOAT score, int x, int y, char *msg);
int Handle_team_score(int team, DFLOAT score);
int Handle_timing(int id, int check, int round);
int Handle_war(int robot_id, int killer_id);
int Handle_seek(int programmer_id, int robot_id, int sought_id);
void Map_dots(void);
void Map_restore(int startx, int starty, int width, int height);
void Map_blue(int startx, int starty, int width, int height);
bool Using_score_decimals(void);
void Client_score_table(void);
int Client_init(char *server, unsigned server_version);
int Client_setup(void);
void Client_cleanup(void);
int Client_start(void);
int Client_fps_request(void);
int Client_power(void);
int Client_fd(void);
int Client_input(int);
void Client_flush(void);
void Client_sync(void);
int Client_wrap_mode(void);
void Reset_shields(void);
void Set_toggle_shield(int on);
void Set_auto_shield(int on);

#ifdef XlibSpecificationRelease
void Key_event(XEvent *event);
#endif
#ifndef _WINDOWS
int x_event(int);
#else
int win_xevent(XEvent event);
void MarkPlayersForRedraw(void);
#endif

int Key_init(void);
int Key_update(void);
int Check_client_fps(void);

#ifdef	SOUND
extern	void audioEvents();
#endif

#endif


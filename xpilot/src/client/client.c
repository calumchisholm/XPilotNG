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

char client_version[] = VERSION;


bool			is_server = false;	/* used in common code */
char	*talk_fast_msgs[TALK_FAST_NR_OF_MSGS];	/* talk macros */

int			scoresChanged = 0;
int			RadarHeight = 0;
int			RadarWidth = 256;	/* must always be 256! */

int     oldServer;
ipos	pos;
ipos	vel;
ipos	world;
ipos	realWorld;
short	heading;
short	nextCheckPoint;

u_byte	numItems[NUM_ITEMS];	/* Count of currently owned items */
u_byte	lastNumItems[NUM_ITEMS];/* Last item count shown */
int	numItemsTime[NUM_ITEMS];/* Number of frames to show this item count */
DFLOAT	showItemsTime;		/* How long to show changed item count for */

short	autopilotLight;

int	showScoreDecimals;

short	lock_id;		/* Id of player locked onto */
short	lock_dir;		/* Direction of lock */
short	lock_dist;		/* Distance to player locked onto */

short	selfVisible;		/* Are we alive and playing? */
short	damaged;		/* Damaged by ECM */
short	destruct;		/* If self destructing */
short	shutdown_delay;
short	shutdown_count;
short	thrusttime;
short	thrusttimemax;
short	shieldtime;
short	shieldtimemax;
short	phasingtime;
short	phasingtimemax;

int		roundDelay;			/* != 0 means we're in a delay */
int		roundDelayMax;		/* (not yet) used for graph of time remaining in delay */

int	map_point_distance;	/* spacing of navigation points */
int	map_point_size;		/* size of navigation points */
int	spark_size;		/* size of debris and spark */
int	shot_size;		/* size of shot */
int	teamshot_size;		/* size of team shot */
bool	showNastyShots = false;		/* show original flavor shots or the new "nasty shots" */
DFLOAT	controlTime;		/* Display control for how long? */
u_byte	spark_rand;		/* Sparkling effect */
u_byte	old_spark_rand;		/* previous value of spark_rand */

long	fuelSum;			/* Sum of fuel in all tanks */
long	fuelMax;			/* How much fuel can you take? */
short	fuelCurrent;			/* Number of currently used tank */
short	numTanks;			/* Number of tanks */
DFLOAT	fuelTime;			/* Display fuel for how long? */
int	fuelLevel1;			/* Fuel critical level */
int	fuelLevel2;			/* Fuel warning level */
int	fuelLevel3;			/* Fuel notify level */

char	*shipShape;		/* Shape of player's ship */
DFLOAT	power;			/* Force of thrust */
DFLOAT	power_s;		/* Saved power fiks */
DFLOAT	turnspeed;		/* How fast player acc-turns */
DFLOAT	turnspeed_s;		/* Saved turnspeed */
DFLOAT	turnresistance;		/* How much is lost in % */
DFLOAT	turnresistance_s;	/* Saved (see above) */
DFLOAT	displayedPower;		/* What the server is sending us */
DFLOAT	displayedTurnspeed;	/* What the server is sending us */
DFLOAT	displayedTurnresistance;/* What the server is sending us */
DFLOAT	spark_prob;		/* Sparkling effect user configurable */
int     charsPerSecond;         /* Message output speed (configurable) */

DFLOAT	hud_move_fact;		/* scale the hud-movement (speed) */
DFLOAT	ptr_move_fact;		/* scale the speed pointer length */
long	instruments;		/* Instruments on screen (bitmask) */
long	hackedInstruments;	/* Hacked instruments on screen (bitmask) */
char	mods[MAX_CHARS];	/* Current modifiers in effect */
int	packet_size;		/* Current frame update packet size */
int	packet_loss;		/* lost packets per second */
int	packet_drop;		/* dropped packets per second */
int	packet_lag;		/* approximate lag in frames */
char	*packet_measure;	/* packet measurement in a second */
long	packet_loop;		/* start of measurement */

bool	showRealName = false;	/* Show realname instead of nick name */
char	name[MAX_CHARS];	/* Nick-name of player */
char	realname[MAX_CHARS];	/* Real name of player */
char	servername[MAX_CHARS];	/* Name of server connecting to */
unsigned	version;	/* Version of the server */
int     toggle_shield;          /* Are shields toggled by a press? */
int     shields = 1;            /* When shields are considered up */

int     auto_shield = 1;        /* shield drops for fire */

int	maxFPS;			/* Client's own FPS */
int	oldMaxFPS;

int	clientPortStart = 0;	/* First UDP port for clients */
int	clientPortEnd = 0;	/* Last one (these are for firewalls) */

byte	lose_item;		/* index for dropping owned item */
int	lose_item_active;	/* one of the lose keys is pressed */

DFLOAT scaleFactor;
DFLOAT scaleFactor_s;

#ifdef SOUND
char 	sounds[MAX_CHARS];	/* audio mappings */
char 	audioServer[MAX_CHARS];	/* audio server */
int 	maxVolume;		/* maximum volume (in percent) */
#endif /* SOUND */

other_t		*Others = 0;
int		num_others = 0,
		max_others = 0;

static DFLOAT		teamscores[MAX_TEAMS];

fuelstation_t	*fuels = 0;
int		num_fuels = 0;

homebase_t	*bases = 0;
int		num_bases = 0;

checkpoint_t	*checks = 0;
int		num_checks = 0;

xp_polygon_t	*polygons = 0;
int		num_polygons = 0, max_polygons = 0;

edge_style_t	*edge_styles = 0;
int		num_edge_styles = 0, max_edge_styles = 0;

polygon_style_t	*polygon_styles = 0;
int		num_polygon_styles = 0, max_polygon_styles = 0;

static cannontime_t	*cannons = 0;
static int		num_cannons = 0;

static target_t		*targets = 0;
static int		num_targets = 0;

/*static checkpoint_t	checks[MAX_CHECKPOINT];*/

score_object_t		score_objects[MAX_SCORE_OBJECTS];
int			score_object = 0;

int		num_playing_teams = 0;

#ifndef  _WINDOWS
/* provide cut&paste and message history */
extern	selection_t	selection;
static	char		*HistoryBlock = NULL;
extern	char		*HistoryMsg[MAX_HIST_MSGS];
#endif
bool			selectionAndHistory = false;
int			maxLinesInHistory;

static fuelstation_t *Fuelstation_by_pos(int x, int y)
{
    int			i, lo, hi, pos;

    lo = 0;
    hi = num_fuels - 1;
    pos = x * Setup->y + y;
    while (lo < hi) {
	i = (lo + hi) >> 1;
	if (pos > fuels[i].pos) {
	    lo = i + 1;
	} else {
	    hi = i;
	}
    }
    if (lo == hi && pos == fuels[lo].pos) {
	return &fuels[lo];
    }
    errno = 0;
    error("No fuelstation at (%d,%d)", x, y);
    return NULL;
}

int Fuel_by_pos(int x, int y)
{
    fuelstation_t	*fuelp;

    if ((fuelp = Fuelstation_by_pos(x, y)) == NULL) {
	return 0;
    }
    return fuelp->fuel;
}

int Target_by_index(int ind, int *xp, int *yp, int *dead_time, int *damage)
{
    if (ind < 0 || ind >= num_targets) {
	return -1;
    }
    *xp = targets[ind].pos / Setup->y;
    *yp = targets[ind].pos % Setup->y;
    *dead_time = targets[ind].dead_time;
    *damage = targets[ind].damage;
    return 0;
}

int Target_alive(int x, int y, int *damage)
{
    int 		i, lo, hi, pos;

    lo = 0;
    hi = num_targets - 1;
    pos = x * Setup->y + y;
    while (lo < hi) {
	i = (lo + hi) >> 1;
	if (pos > targets[i].pos) {
	    lo = i + 1;
	} else {
	    hi = i;
	}
    }
    if (lo == hi && pos == targets[lo].pos) {
	*damage = targets[lo].damage;
	return targets[lo].dead_time;
    }
    errno = 0;
    error("No targets at (%d,%d)", x, y);
    return -1;
}

int Handle_fuel(int ind, int fuel)
{
    if (ind < 0 || ind >= num_fuels) {
	errno = 0;
	error("Bad fuelstation index (%d)", ind);
	return -1;
    }
    fuels[ind].fuel = fuel;
    return 0;
}

static cannontime_t *Cannon_by_pos(int x, int y)
{
    int			i, lo, hi, pos;

    lo = 0;
    hi = num_cannons - 1;
    pos = x * Setup->y + y;
    while (lo < hi) {
	i = (lo + hi) >> 1;
	if (pos > cannons[i].pos) {
	    lo = i + 1;
	} else {
	    hi = i;
	}
    }
    if (lo == hi && pos == cannons[lo].pos) {
	return &cannons[lo];
    }
    errno = 0;
    error("No cannon at (%d,%d)", x, y);
    return NULL;
}

int Cannon_dead_time_by_pos(int x, int y, int *dot)
{
    cannontime_t	*cannonp;

    if ((cannonp = Cannon_by_pos(x, y)) == NULL) {
	return -1;
    }
    *dot = cannonp->dot;
    return cannonp->dead_time;
}

int Handle_cannon(int ind, int dead_time)
{
    if (ind < 0 || ind >= num_cannons) {
	errno = 0;
	error("Bad cannon index (%d)", ind);
	return 0;
    }
    cannons[ind].dead_time = dead_time;
    return 0;
}

int Handle_target(int num, int dead_time, int damage)
{
    if (num < 0 || num >= num_targets) {
	errno = 0;
	error("Bad target index (%d)", num);
	return 0;
    }
    if (dead_time == 0
	&& (damage < 1
	|| damage > TARGET_DAMAGE)) {
	printf ("BUG target %d, dead %d, damage %d\n", num, dead_time, damage);
    }
    if (targets[num].dead_time > 0 && dead_time == 0) {
	int pos = targets[num].pos;
	Paint_radar_block(pos / Setup->y, pos % Setup->y, targetRadarColor);
    }
    else if (targets[num].dead_time == 0 && dead_time > 0) {
	int pos = targets[num].pos;
	Paint_radar_block(pos / Setup->y, pos % Setup->y, BLACK);
    }

    targets[num].dead_time = dead_time;
    targets[num].damage = damage;

    return 0;
}

static homebase_t *Homebase_by_pos(int x, int y)
{
    int			i, lo, hi, pos;

    lo = 0;
    hi = num_bases - 1;
    pos = x * Setup->y + y;
    while (lo < hi) {
	i = (lo + hi) >> 1;
	if (pos > bases[i].pos) {
	    lo = i + 1;
	} else {
	    hi = i;
	}
    }
    if (lo == hi && pos == bases[lo].pos) {
	return &bases[lo];
    }
    errno = 0;
    error("No homebase at (%d,%d)", x, y);
    return NULL;
}

int Base_info_by_pos(int x, int y, int *idp, int *teamp)
{
    homebase_t	*basep;

    if ((basep = Homebase_by_pos(x, y)) == NULL) {
	return -1;
    }
    *idp = basep->id;
    *teamp = basep->team;
    return 0;
}

int Handle_base(int id, int ind)
{
    int		i;

    if (ind < 0 || ind >= num_bases) {
	errno = 0;
	error("Bad homebase index (%d)", ind);
	return -1;
    }
    for (i = 0; i < num_bases; i++) {
	if (bases[i].id == id)
	    bases[i].id = -1;
    }
    bases[ind].id = id;

    return 0;
}

int Check_pos_by_index(int ind, int *xp, int *yp)
{
    if (ind < 0 || ind >= num_checks) {
	errno = 0;
	error("Bad checkpoint index (%d)", ind);
	*xp = 0;
	*yp = 0;
	return -1;
    }
    *xp = checks[ind].pos / Setup->y;
    *yp = checks[ind].pos % Setup->y;
    return 0;
}

int Check_index_by_pos(int x, int y)
{
    int			i, pos;

    pos = x * Setup->y + y;
    for (i = 0; i < num_checks; i++) {
	if (pos == checks[i].pos) {
	    return i;
	}
    }
    errno = 0;
    error("Can't find checkpoint (%d,%d)", x, y);
    return 0;
}

/*
 * Convert a `space' map block into a dot.
 */
static void Map_make_dot(unsigned char *data)
{
    if (*data == SETUP_SPACE) {
	*data = SETUP_SPACE_DOT;
    }
    else if (*data == SETUP_DECOR_FILLED) {
	*data = SETUP_DECOR_DOT_FILLED;
    }
    else if (*data == SETUP_DECOR_RU) {
	*data = SETUP_DECOR_DOT_RU;
    }
    else if (*data == SETUP_DECOR_RD) {
	*data = SETUP_DECOR_DOT_RD;
    }
    else if (*data == SETUP_DECOR_LU) {
	*data = SETUP_DECOR_DOT_LU;
    }
    else if (*data == SETUP_DECOR_LD) {
	*data = SETUP_DECOR_DOT_LD;
    }
}

/*
 * Optimize the drawing of all blue space dots by converting
 * certain map objects into a specialised form of their type.
 */
void Map_dots(void)
{
    int			i,
			x,
			y,
			start;
    unsigned char	dot[256];

    /*
     * Lookup table to recognize dots.
     */
    memset(dot, 0, sizeof dot);
    dot[SETUP_SPACE_DOT] = 1;
    dot[SETUP_DECOR_DOT_FILLED] = 1;
    dot[SETUP_DECOR_DOT_RU] = 1;
    dot[SETUP_DECOR_DOT_RD] = 1;
    dot[SETUP_DECOR_DOT_LU] = 1;
    dot[SETUP_DECOR_DOT_LD] = 1;

    /*
     * Restore the map to unoptimized form.
     */
    for (i = Setup->x * Setup->y; i-- > 0; ) {
	if (dot[Setup->map_data[i]]) {
	    if (Setup->map_data[i] == SETUP_SPACE_DOT) {
		Setup->map_data[i] = SETUP_SPACE;
	    }
	    else if (Setup->map_data[i] == SETUP_DECOR_DOT_FILLED) {
		Setup->map_data[i] = SETUP_DECOR_FILLED;
	    }
	    else if (Setup->map_data[i] == SETUP_DECOR_DOT_RU) {
		Setup->map_data[i] = SETUP_DECOR_RU;
	    }
	    else if (Setup->map_data[i] == SETUP_DECOR_DOT_RD) {
		Setup->map_data[i] = SETUP_DECOR_RD;
	    }
	    else if (Setup->map_data[i] == SETUP_DECOR_DOT_LU) {
		Setup->map_data[i] = SETUP_DECOR_LU;
	    }
	    else if (Setup->map_data[i] == SETUP_DECOR_DOT_LD) {
		Setup->map_data[i] = SETUP_DECOR_LD;
	    }
	}
    }

    /*
     * Lookup table to test for map data which can be turned into a dot.
     */
    memset(dot, 0, sizeof dot);
    dot[SETUP_SPACE] = 1;
    if (!BIT(instruments, SHOW_DECOR)) {
	dot[SETUP_DECOR_FILLED] = 1;
	dot[SETUP_DECOR_RU] = 1;
	dot[SETUP_DECOR_RD] = 1;
	dot[SETUP_DECOR_LU] = 1;
	dot[SETUP_DECOR_LD] = 1;
    }

    /*
     * Optimize.
     */
    if (map_point_size > 0) {
	if (BIT(Setup->mode, WRAP_PLAY)) {
	    for (x = 0; x < Setup->x; x++) {
		if (dot[Setup->map_data[x * Setup->y]]) {
		    Map_make_dot(&Setup->map_data[x * Setup->y]);
		}
	    }
	    for (y = 0; y < Setup->y; y++) {
		if (dot[Setup->map_data[y]]) {
		    Map_make_dot(&Setup->map_data[y]);
		}
	    }
	    start = map_point_distance;
	} else {
	    start = 0;
	}
	if (map_point_distance > 0) {
	    for (x = start; x < Setup->x; x += map_point_distance) {
		for (y = start; y < Setup->y; y += map_point_distance) {
		    if (dot[Setup->map_data[x * Setup->y + y]]) {
			Map_make_dot(&Setup->map_data[x * Setup->y + y]);
		    }
		}
	    }
	}
	for (i = 0; i < num_cannons; i++) {
	    x = cannons[i].pos / Setup->y;
	    y = cannons[i].pos % Setup->y;
	    if ((x == 0 || y == 0) && BIT(Setup->mode, WRAP_PLAY)) {
		cannons[i].dot = 1;
	    }
	    else if (map_point_distance > 0
		&& x % map_point_distance == 0
		&& y % map_point_distance == 0) {
		cannons[i].dot = 1;
	    } else {
		cannons[i].dot = 0;
	    }
	}
    }
}

/*
 * Optimize the drawing of all blue map objects by converting
 * their map type to a bitmask with bits for each blue segment.
 */
void Map_restore(int startx, int starty, int width, int height)
{
    int			i, j,
			x, y,
			map_index,
			type;

    /*
     * Restore an optimized map to its original unoptimized state.
     */
    x = startx;
    for (i = 0; i < width; i++, x++) {
	if (x < 0) {
	    x += Setup->x;
	}
	else if (x >= Setup->x) {
	    x -= Setup->x;
	}

	y = starty;
	for (j = 0; j < height; j++, y++) {
	    if (y < 0) {
		y += Setup->y;
	    }
	    else if (y >= Setup->y) {
		y -= Setup->y;
	    }

	    map_index = x * Setup->y + y;

	    type = Setup->map_data[map_index];
	    if ((type & BLUE_BIT) == 0) {
		if (type == SETUP_FILLED_NO_DRAW) {
		    Setup->map_data[map_index] = SETUP_FILLED;
		}
	    }
	    else if ((type & BLUE_FUEL) == BLUE_FUEL) {
		Setup->map_data[map_index] = SETUP_FUEL;
	    }
	    else if (type & BLUE_OPEN) {
		if (type & BLUE_BELOW) {
		    Setup->map_data[map_index] = SETUP_REC_RD;
		} else {
		    Setup->map_data[map_index] = SETUP_REC_LU;
		}
	    }
	    else if (type & BLUE_CLOSED) {
		if (type & BLUE_BELOW) {
		    Setup->map_data[map_index] = SETUP_REC_LD;
		} else {
		    Setup->map_data[map_index] = SETUP_REC_RU;
		}
	    } else {
		Setup->map_data[map_index] = SETUP_FILLED;
	    }
	}
    }
}

void Map_blue(int startx, int starty, int width, int height)
{
    int			i, j,
			x, y,
			map_index,
			type,
			newtype;
    unsigned char	blue[256];
    const long		outline_mask = SHOW_OUTLINE_WORLD
				     | SHOW_FILLED_WORLD
				     | SHOW_TEXTURED_WALLS;

    /*
     * Optimize the map for blue.
     */
    memset(blue, 0, sizeof blue);
    blue[SETUP_FILLED] = BLUE_LEFT | BLUE_UP | BLUE_RIGHT | BLUE_DOWN;
    blue[SETUP_FILLED_NO_DRAW] = blue[SETUP_FILLED];
    blue[SETUP_FUEL] = blue[SETUP_FILLED];
    blue[SETUP_REC_RU] = BLUE_RIGHT | BLUE_UP;
    blue[SETUP_REC_RD] = BLUE_RIGHT | BLUE_DOWN;
    blue[SETUP_REC_LU] = BLUE_LEFT | BLUE_UP;
    blue[SETUP_REC_LD] = BLUE_LEFT | BLUE_DOWN;
    blue[BLUE_BIT|BLUE_OPEN] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_LEFT] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_UP] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_LEFT|BLUE_UP] =
	blue[SETUP_REC_LU];
    blue[BLUE_BIT|BLUE_OPEN|BLUE_BELOW] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_BELOW|BLUE_RIGHT] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_BELOW|BLUE_DOWN] =
    blue[BLUE_BIT|BLUE_OPEN|BLUE_BELOW|BLUE_RIGHT|BLUE_DOWN] =
	blue[SETUP_REC_RD];
    blue[BLUE_BIT|BLUE_CLOSED] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_RIGHT] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_UP] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_RIGHT|BLUE_UP] =
	blue[SETUP_REC_RU];
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_BELOW] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_BELOW|BLUE_LEFT] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_BELOW|BLUE_DOWN] =
    blue[BLUE_BIT|BLUE_CLOSED|BLUE_BELOW|BLUE_LEFT|BLUE_DOWN] =
	blue[SETUP_REC_LD];
    for (i = BLUE_BIT; i < sizeof blue; i++) {
	if ((i & BLUE_FUEL) == BLUE_FUEL
	    || (i & (BLUE_OPEN|BLUE_CLOSED)) == 0) {
	    blue[i] = blue[SETUP_FILLED];
	}
    }

    x = startx;
    for (i = 0; i < width; i++, x++) {
	if (x < 0) {
	    x += Setup->x;
	}
	else if (x >= Setup->x) {
	    x -= Setup->x;
	}

	y = starty;
	for (j = 0; j < height; j++, y++) {
	    if (y < 0) {
		y += Setup->y;
	    }
	    else if (y >= Setup->y) {
		y -= Setup->y;
	    }

	    map_index = x * Setup->y + y;

	    type = Setup->map_data[map_index];
	    newtype = 0;
	    switch (type) {
	    case SETUP_FILLED:
	    case SETUP_FILLED_NO_DRAW:
	    case SETUP_FUEL:
		newtype = BLUE_BIT;
		if (type == SETUP_FUEL) {
		    newtype |= BLUE_FUEL;
		}
		if ((x == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
			& BLUE_RIGHT))
		    newtype |= BLUE_LEFT;
		if ((y == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
			& BLUE_UP))
		    newtype |= BLUE_DOWN;
		if (!BIT(instruments, outline_mask)
		    || ((x == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[y]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
			    & BLUE_LEFT)))
		    newtype |= BLUE_RIGHT;
		if (!BIT(instruments, outline_mask)
		    || ((y == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[x * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
			    & BLUE_DOWN)))
		    newtype |= BLUE_UP;
		break;

	    case SETUP_REC_LU:
		newtype = BLUE_BIT | BLUE_OPEN;
		if (x == 0
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
			& BLUE_RIGHT))
		    newtype |= BLUE_LEFT;
		if (!BIT(instruments, outline_mask)
		    || ((y == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[x * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
			    & BLUE_DOWN)))
		    newtype |= BLUE_UP;
		break;

	    case SETUP_REC_RU:
		newtype = BLUE_BIT | BLUE_CLOSED;
		if (!BIT(instruments, outline_mask)
		    || ((x == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[y]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
			    & BLUE_LEFT)))
		    newtype |= BLUE_RIGHT;
		if (!BIT(instruments, outline_mask)
		    || ((y == Setup->y - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[x * Setup->y]]
				& BLUE_DOWN))
			: !(blue[Setup->map_data[x * Setup->y + (y + 1)]]
			    & BLUE_DOWN)))
		    newtype |= BLUE_UP;
		break;

	    case SETUP_REC_LD:
		newtype = BLUE_BIT | BLUE_BELOW | BLUE_CLOSED;
		if ((x == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[(Setup->x - 1) * Setup->y + y]]
			    & BLUE_RIGHT))
		    : !(blue[Setup->map_data[(x - 1) * Setup->y + y]]
			& BLUE_RIGHT))
		    newtype |= BLUE_LEFT;
		if ((y == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
			& BLUE_UP))
		    newtype |= BLUE_DOWN;
		break;

	    case SETUP_REC_RD:
		newtype = BLUE_BIT | BLUE_BELOW | BLUE_OPEN;
		if (!BIT(instruments, outline_mask)
		    || ((x == Setup->x - 1)
			? (!BIT(Setup->mode, WRAP_PLAY)
			   || !(blue[Setup->map_data[y]]
				& BLUE_LEFT))
			: !(blue[Setup->map_data[(x + 1) * Setup->y + y]]
			    & BLUE_LEFT)))
		    newtype |= BLUE_RIGHT;
		if ((y == 0)
		    ? (!BIT(Setup->mode, WRAP_PLAY) ||
			!(blue[Setup->map_data[x * Setup->y + Setup->y - 1]]
			    & BLUE_UP))
		    : !(blue[Setup->map_data[x * Setup->y + (y - 1)]]
			& BLUE_UP))
		    newtype |= BLUE_DOWN;
		break;

	    default:
		continue;
	    }
	    if (newtype != 0) {
		if (newtype == BLUE_BIT) {
		    newtype = SETUP_FILLED_NO_DRAW;
		}
		Setup->map_data[map_index] = newtype;
	    }
	}
    }
}

static int Map_init(void)
{
    int			i,
			max,
			type;
    u_byte		types[256];

    num_fuels = 0;
    num_bases = 0;
    num_cannons = 0;
    num_targets = 0;
    fuels = NULL;
    bases = NULL;
    cannons = NULL;
    targets = NULL;
    memset(types, 0, sizeof types);
    types[SETUP_FUEL] = 1;
    types[SETUP_CANNON_UP] = 2;
    types[SETUP_CANNON_RIGHT] = 2;
    types[SETUP_CANNON_DOWN] = 2;
    types[SETUP_CANNON_LEFT] = 2;
    for (i = SETUP_TARGET; i < SETUP_TARGET + 10; i++) {
	types[i] = 3;
    }
    for (i = SETUP_BASE_LOWEST; i <= SETUP_BASE_HIGHEST; i++) {
	types[i] = 4;
    }
    max = Setup->x * Setup->y;
    for (i = 0; i < max; i++) {
	switch (types[Setup->map_data[i]]) {
	case 1: num_fuels++; break;
	case 2: num_cannons++; break;
	case 3: num_targets++; break;
	case 4: num_bases++; break;
	}
    }
    if (num_bases != 0) {
	bases = (homebase_t *) malloc(num_bases * sizeof(homebase_t));
	if (bases == NULL) {
	    error("No memory for Map bases (%d)", num_bases);
	    return -1;
	}
	num_bases = 0;
    }
    if (num_fuels != 0) {
	fuels = (fuelstation_t *) malloc(num_fuels * sizeof(fuelstation_t));
	if (fuels == NULL) {
	    error("No memory for Map fuels (%d)", num_fuels);
	    return -1;
	}
	num_fuels = 0;
    }
    if (num_targets != 0) {
	targets = (target_t *) malloc(num_targets * sizeof(target_t));
	if (targets == NULL) {
	    error("No memory for Map targets (%d)", num_targets);
	    return -1;
	}
	num_targets = 0;
    }
    if (num_cannons != 0) {
	cannons = (cannontime_t *) malloc(num_cannons * sizeof(cannontime_t));
	if (cannons == NULL) {
	    error("No memory for Map cannons (%d)", num_cannons);
	    return -1;
	}
	num_cannons = 0;
    }
    for (i = 0; i < num_checks; i++) {
	types[SETUP_CHECK + i] = 5;
    }
    for (i = 0; i < max; i++) {
	type = Setup->map_data[i];
	switch (types[type]) {
	case 1:
	    fuels[num_fuels].pos = i;
	    fuels[num_fuels].fuel = MAX_STATION_FUEL;
	    num_fuels++;
	    break;
	case 2:
	    cannons[num_cannons].pos = i;
	    cannons[num_cannons].dead_time = 0;
	    cannons[num_cannons].dot = 0;
	    num_cannons++;
	    break;
	case 3:
	    targets[num_targets].pos = i;
	    targets[num_targets].dead_time = 0;
	    targets[num_targets].damage = TARGET_DAMAGE;
	    num_targets++;
	    break;
	case 4:
	    bases[num_bases].pos = i;
	    bases[num_bases].id = -1;
	    bases[num_bases].team = type % 10;
	    bases[num_bases].type = type - (type % 10);
	    bases[num_bases].deathtime = -10000; /* kps hack */
	    num_bases++;
	    Setup->map_data[i] = type - (type % 10);
	    break;
	case 5:
	    checks[type - SETUP_CHECK].pos = i;
	    Setup->map_data[i] = SETUP_CHECK;
	    break;
	}
    }
    return 0;
}

static int Map_cleanup(void)
{
    if (num_bases > 0) {
	if (bases != NULL) {
	    free(bases);
	    bases = NULL;
	}
	num_bases = 0;
    }
    if (num_fuels > 0) {
	if (fuels != NULL) {
	    free(fuels);
	    fuels = NULL;
	}
	num_fuels = 0;
    }
    if (num_targets > 0) {
	if (targets != NULL) {
	    free(targets);
	    targets = NULL;
	}
	num_targets = 0;
    }
    if (num_cannons > 0) {
	if (cannons != NULL) {
	    free(cannons);
	    cannons = NULL;
	}
	num_cannons = 0;
	}
    return 0;
}

other_t *Other_by_id(int id)
{
    int			i;

    if (id != -1) {
	for (i = 0; i < num_others; i++) {
	    if (Others[i].id == id)
		return &Others[i];
	}
    }
    return NULL;
}

/* Only used by message scan hack */
other_t *Other_by_name(char *name)
{
    int i;

    if (name == NULL)
	return NULL;

    for (i = 0; i < num_others; i++) {
	if (!strcmp(name, Others[i].name))
	    return &Others[i];
    }
    return NULL;
}

shipobj *Ship_by_id(int id)
{
    other_t		*other;

    if ((other = Other_by_id(id)) == NULL)
	return Parse_shape_str(NULL);
    return other->ship;
}

int Handle_leave(int id)
{
    other_t		*other;
    int			i;
    char		msg[MSG_LEN];

    if ((other = Other_by_id(id)) != NULL) {
	if (other == self) {
	    warn("Self left?!");
	    self = NULL;
	}
	Free_ship_shape(other->ship);
	other->ship = NULL;
	/*
	 * Silent about tanks and robots.
	 */
	if (other->mychar != 'T' && other->mychar != 'R') {
	    sprintf(msg, "%s left this world.", other->name);
	    Add_message(msg);
	}
	num_others--;
	while (other < &Others[num_others]) {
	    *other = other[1];
	    other++;
	}
	scoresChanged = 1;
    }
    for (i = 0; i < num_others; i++) {
	other = &Others[i];
	if (other->war_id == id) {
	    other->war_id = -1;
	    scoresChanged = 1;
	}
    }
    return 0;
}

int Handle_player(int id, int player_team, int mychar, char *player_name,
		  char *real_name, char *host_name, char *shape,
		  int myself)
{
    other_t		*other;

    if ((BIT(Setup->mode, TEAM_PLAY) && player_team < 0)
		|| player_team >= MAX_TEAMS) {
	warn("Illegal team %d for received player, setting to 0");
	player_team = 0;
    }
    if ((other = Other_by_id(id)) == NULL) {
	if (num_others >= max_others) {
	    max_others += 5;
	    if (num_others == 0)
		Others = malloc(max_others * sizeof(other_t));
	    else
		Others = realloc(Others, max_others * sizeof(other_t));
	    if (Others == NULL)
		fatal("Not enough memory for player info");
	    if (self != NULL) {
		/*
		 * We've made `self' the first member of Others[].
		 */
		self = &Others[0];
	    }
	}
	other = &Others[num_others++];
    }
    if (self == NULL
	&& (myself || (version < 0x4F10 && strcmp(name, player_name) == 0))) {
	if (other != &Others[0]) {
	    /*
	     * Make `self' the first member of Others[].
	     */
	    *other = Others[0];
	    other = &Others[0];
	}
	self = other;
	team = player_team;
    }
    other->id = id;
    other->team = player_team;
    other->score = 0;
    other->round = 0;
    other->check = 0;
    other->timing = 0;
    other->life = 0;
    other->mychar = mychar;
    other->war_id = -1;
    other->name_width = 0;
    strlcpy(other->name, player_name, sizeof(other->name));
    if (BIT(instruments, SHOW_SHIP_ID))
	sprintf(other->id_string, "%d", id);
    else
	strlcpy(other->id_string, player_name, sizeof(other->id_string));
    strlcpy(other->real, real_name, sizeof(other->real));
    strlcpy(other->host, host_name, sizeof(other->host));
    scoresChanged = 1;
    other->ship = Convert_shape_str(shape);
    other->ignorelevel = 0;
    Calculate_shield_radius(other->ship);

    return 0;
}

int Handle_team(int id, int pl_team)
{
    other_t *other;

    other = Other_by_id(id);
    if (other == NULL) {
	warn("Received packet to change team for nonexistent id %d", id);
	return 0;
    }
    if ((BIT(Setup->mode, TEAM_PLAY) && pl_team < 0) || pl_team >= MAX_TEAMS) {
	warn("Illegal team %d received for player id %d", pl_team, id);
	return 0;
    }
    other->team = pl_team;
    if (other == self)
	team = pl_team;
    return 0;
}

int Handle_war(int robot_id, int killer_id)
{
    other_t		*robot,
			*killer;
    char		msg[MSG_LEN];

    if ((robot = Other_by_id(robot_id)) == NULL) {
	errno = 0;
	error("Can't update war for non-existing player (%d,%d)",
	      robot_id, killer_id);
	return 0;
    }
    if (killer_id == -1) {
	/*
	 * Robot is no longer in war mode.
	 */
	robot->war_id = -1;
	return 0;
    }
    if ((killer = Other_by_id(killer_id)) == NULL) {
	errno = 0;
	error("Can't update war against non-existing player (%d,%d)",
	      robot_id, killer_id);
	return 0;
    }
    robot->war_id = killer_id;
    sprintf(msg, "%s declares war on %s.", robot->name, killer->name);
    Add_message(msg);
    scoresChanged = 1;

    return 0;
}

int Handle_seek(int programmer_id, int robot_id, int sought_id)
{
    other_t		*programmer,
			*robot,
			*sought;
    char		msg[MSG_LEN];

    if ((programmer = Other_by_id(programmer_id)) == NULL
	|| (robot = Other_by_id(robot_id)) == NULL
	|| (sought = Other_by_id(sought_id)) == NULL) {
	errno = 0;
	error("Bad player seek (%d,%d,%d)",
	      programmer_id, robot_id, sought_id);
	return 0;
    }
    robot->war_id = sought_id;
    sprintf(msg, "%s has programmed %s to seek %s.",
	    programmer->name, robot->name, sought->name);
    Add_message(msg);
    scoresChanged = 1;


    return 0;
}

int Handle_score(int id, DFLOAT score, int life, int mychar, int alliance)
{
    other_t		*other;

    if ((other = Other_by_id(id)) == NULL) {
#ifndef _WINDOWS
	errno = 0;
	error("Can't update score for non-existing player %d,%.2f,%d",
	      id, score, life);
#endif
	return 0;
    }
    else if (other->score != score
	|| other->life != life
	|| other->mychar != mychar
	|| other->alliance != alliance) {
	other->score = score;
	other->life = life;
	other->mychar = mychar;
	other->alliance = alliance;
	scoresChanged = 1;
    }

    return 0;
}

int Handle_team_score(int team, DFLOAT score)
{
    if (teamscores[team] != score) {
	teamscores[team] = score;
	scoresChanged = 1;
    }

    return 0;
}

int Handle_timing(int id, int check, int round)
{
    other_t		*other;

    if ((other = Other_by_id(id)) == NULL) {
	errno = 0;
	error("Can't update timing for non-existing player %d,%d,%d", id, check, round);
	return 0;
    }
    else if (other->check != check
	|| other->round != round) {
	other->check = check;
	other->round = round;
	other->timing = round * num_checks + check;
	other->timing_loops = last_loops;
	scoresChanged = 1;
    }

    return 0;
}

int Handle_score_object(DFLOAT score, int x, int y, char *msg)
{
    score_object_t*	sobj = &score_objects[score_object];

    sobj->score = score;
    sobj->x = x;
    sobj->y = y;
    sobj->life_time = scoreObjectTime;

    /* Initialize sobj->hud_msg (is shown on the HUD) */
    if (msg[0] != '\0') {
	if (Using_score_decimals()) {
	    sprintf(sobj->hud_msg, "%s %.*f", msg, showScoreDecimals, score);
	} else {
	    sprintf(sobj->hud_msg, "%s %d", msg, (int) rint(score));
	}
	sobj->hud_msg_len = strlen(sobj->hud_msg);
	sobj->hud_msg_width = XTextWidth(gameFont,
					 sobj->hud_msg, sobj->hud_msg_len);
    } else
	sobj->hud_msg_len = 0;

    /* Initialize sobj->msg data (is shown on game area) */
    if (Using_score_decimals()) {
	sprintf(sobj->msg, "%.*f", showScoreDecimals, score);
    } else {
	sprintf(sobj->msg, "%d", (int) rint(score));
    }
    sobj->msg_len = strlen(sobj->msg);
    sobj->msg_width = XTextWidth(gameFont, sobj->msg, sobj->msg_len);

    /* Update global index variable */
    score_object = (score_object + 1) % MAX_SCORE_OBJECTS;

    return 0;
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
		(DFLOAT)killratio_totalkills / killratio_totaldeaths);

    if (rounds_played == 0)
	sprintf(killsperround, "0");
    else
	sprintf(killsperround, "%.2f",
		(DFLOAT)killratio_totalkills / rounds_played);

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

bool Using_score_decimals(void)
{
    if (showScoreDecimals > 0 && version >= 0x4500
	&& (version < 0x4F09 || version >= 0x4F11))
	return true;
    return false;
}

struct team_score {
    DFLOAT	score;
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
		    for (k = i; k > j; k--) {
			team_order[k] = team_order[k - 1];
		    }
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
	    /*if (version >= 0x4500) {
	      team[other->team].score = teamscores[other->team];
	      }*/

	}
    }
    return;
}

#define TEAM_PAUSEHACK 100

static int Team_heading(int entrynum, int teamnum,
			int teamlives, DFLOAT teamscore)
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
    if (BIT(Setup->mode, LIMITED_LIVES) && teamlives == 0) {
	tmp.mychar = 'D';
    } else {
	tmp.mychar = ' ';
    }
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


void Client_score_table(void)
{
    struct team_score	team[MAX_TEAMS],
			pausers,
			*team_order[MAX_TEAMS];
    other_t		*other,
			**order;
    int			i, j, entrynum = 0;

    if (scoresChanged == 0)
	return;

    if (players_exposed == false)
	return;

    if (num_others < 1) {
	Paint_score_start();
	scoresChanged = 0;
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

    scoresChanged = 0;
}

#ifndef _WINDOWS
static int Alloc_history(void)
{
    char	*hist_ptr;
    int		i;

    /* maxLinesInHistory is a runtime constant */
    if ((hist_ptr = (char *)malloc(maxLinesInHistory * MAX_CHARS)) == NULL) {
	error("No memory for history");
	return -1;
    }
    HistoryBlock	= hist_ptr;

    for (i = 0; i < maxLinesInHistory; i++) {
	HistoryMsg[i]	= hist_ptr;
	hist_ptr[0]	= '\0';
	hist_ptr	+= MAX_CHARS;
    }
    return 0;
}

static void Free_selectionAndHistory(void)
{
    if (HistoryBlock) {
	free(HistoryBlock);
	HistoryBlock = NULL;
    }
    if (selection.txt) {
	free(selection.txt);
	selection.txt = NULL;
    }
}
#else
static int Alloc_history(void)
{
    return 0;
}

static void Free_selectionAndHistory(void)
{
}
#endif

int Client_init(char *server, unsigned server_version)
{
    version = server_version;
    if (server_version < 0x4F09)
	oldServer = 1;
    else
	oldServer = 0;

    Make_table();
    Init_scale_array();

    if ( Init_wreckage() == -1 ) {
	return -1;
    }

    if (Init_asteroids() == -1) {
	return -1;
    }

    if (Bitmap_add_std_objects() == -1) {
	return -1;
    }

    if (Bitmap_add_std_textures() == -1) {
	return -1;
    }

    strlcpy(servername, server, sizeof(servername));

    return 0;
}

int Client_setup(void)
{
    if (oldServer) {
	if (Map_init() == -1) {
	    return -1;
	}
	Map_dots();
	Map_restore(0, 0, Setup->x, Setup->y);
	Map_blue(0, 0, Setup->x, Setup->y);
	/* No one wants this on old-style maps anyway, so turn it off.
	 * I do, so turn it on.
	 * This allows people to turn it on in their .xpilotrc for new maps
	 * without affecting old ones. It's still possible to turn in on
	 * from the config menu during play for old maps.
	 * -- But doesn't seem to work anyway if turned on? Well who cares */
	CLR_BIT(instruments, SHOW_TEXTURED_WALLS);
    }

    /* kps - the above one is used in the standard code  */
    /*RadarHeight = (RadarWidth * Setup->y) / Setup->x;*/
    RadarHeight = (RadarWidth * Setup->height) / Setup->width;

    if (Init_playing_windows() == -1) {
	return -1;
    }
    if (Alloc_msgs() == -1) {
	return -1;
    }
    if (Alloc_history() == -1) {
	return -1;
    }

    /* Old servers can't deal with 0.0 turnresistance, so swap to
     * the alternate bank, and hope there's something better there. */
    /* HACK: Hanging Gardens runs an old server (version code 0x4101)
     * which happens to have the turnresistance patch. */
    if (turnresistance == 0.0 && version < 0x4200 && version != 0x4101)
    {
	DFLOAT tmp;
#define SWAP(a,b) (tmp = (a), (a) = (b), (b) = tmp)
	SWAP(power, power_s);
	SWAP(turnspeed, turnspeed_s);
	SWAP(turnresistance, turnresistance_s);
#undef SWAP
	controlTime = CONTROL_TIME;
	Add_message("Old server can't handle turnResistance=0.0; "
		    "swapping to alternate settings [*Client message*]");
    }

    return 0;
}

int Client_fps_request(void)
{
    LIMIT(maxFPS, 1, 200);
    oldMaxFPS = maxFPS;
    return Send_fps_request(maxFPS);
}

int Client_power(void)
{
    int		i;

    if (Send_power(power) == -1
	|| Send_power_s(power_s) == -1
	|| Send_turnspeed(turnspeed) == -1
	|| Send_turnspeed_s(turnspeed_s) == -1
	|| Send_turnresistance(turnresistance) == -1
	|| Send_turnresistance_s(turnresistance_s) == -1
	|| Send_display() == -1
	|| Startup_server_motd() == -1) {
	return -1;
    }
    for (i = 0; i < NUM_MODBANKS; i++) {
	if (Send_modifier_bank(i) == -1) {
	    return -1;
	}
    }

    return 0;
}

int Client_start(void)
{
    Key_init();

    return 0;
}

void Client_cleanup(void)
{
    int		i;

    Quit();
    Free_selectionAndHistory();
    if (max_others > 0) {
	for (i = 0; i < num_others; i++) {
	    other_t* other = &Others[i];
	    Free_ship_shape(other->ship);
	}
	free(Others);
	num_others = 0;
	max_others = 0;
    }
    Map_cleanup();
}

int Client_fd(void)
{
    return ConnectionNumber(dpy);
}

int Client_input(int new_input)
{
#ifndef _WINDOWS
    return x_event(new_input);
#else
    return 0;
#endif
}
void Client_flush(void)
{
    XFlush(dpy);
}

void Client_sync(void)
{
    XSync(dpy, False);
}

int Client_wrap_mode(void)
{
    return (BIT(Setup->mode, WRAP_PLAY) != 0);
}

int Check_client_fps(void)
{
    if (oldMaxFPS != maxFPS) {
	LIMIT(maxFPS, 1, 200);
	oldMaxFPS = maxFPS;
	return Send_fps_request(maxFPS);
    }
    return 0;
}

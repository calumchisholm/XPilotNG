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

char guiobjects_version[] = VERSION;

static bool texturedShips = false; /* Turned this off because the images drawn
				    * don't match the actual shipshape used
				    * for wall collisions by the server. */

int selfLWColor;
int enemyLWColor;
int teamLWColor;
int shipNameColor;
int baseNameColor;
int mineNameColor;
int ballColor;
int connColor;
int teamShotColor;
int shipShapesHackColor;
int team0Color;
int team1Color;
int team2Color;
int team3Color;
int team4Color;
int team5Color;
int team6Color;
int team7Color;
int team8Color;
int team9Color;

void Gui_paint_ball(int x, int y)
{
    x = X(x);
    y = Y(y);

    if (!texturedObjects)
	Arc_add(ballColor, x - BALL_RADIUS, y - BALL_RADIUS,
		2 * BALL_RADIUS, 2 * BALL_RADIUS, 0, 64 * 360);
    else
	Bitmap_paint(drawPixmap, BM_BALL, WINSCALE(x - BALL_RADIUS),
		     WINSCALE(y - BALL_RADIUS), 0);
}


void Gui_paint_ball_connector(int x_1, int y_1, int x_2, int y_2)
{
    x_2 = X(x_2);
    y_2 = Y(y_2);
    x_1 = X(x_1);
    y_1 = Y(y_1);
    Segment_add(connColor, x_1, y_1, x_2, y_2);
}

static void Gui_paint_mine_name(int x, int y, char *name)
{
    int		name_len, name_width;

    if (!name)
	return;

    SET_FG(colors[mineNameColor].pixel);

    name_len = strlen(name);
    name_width = XTextWidth(gameFont, name, name_len);

    if (name != NULL)
	rd.drawString(dpy, drawPixmap, gameGC,
		    WINSCALE(x) - name_width / 2,
		    WINSCALE(y + 4) + gameFont->ascent + 1,
		    name, name_len);
}

void Gui_paint_mine(int x, int y, int teammine, char *name)
{
    if (!texturedObjects) {
	static double	lastScaleFactor;
	static XPoint	mine_points[21];
	static XPoint	world_mine_points[21] = {
	    { 0, 0 },
	    { 1, 0 },
	    { 0, -1 },
	    { 4, 0 },
	    { 0, -1 },
	    { 6, 0 },
	    { 0, 1 },
	    { 4, 0 },
	    { 0, 1 },
	    { 1, 0 },
	    { 0, 2 },
	    { -1, 0 },
	    { 0, 1 },
	    { -4, 0 },
	    { 0, 1 },
	    { -6, 0 },
	    { 0, -1 },
	    { -4, 0 },
	    { 0, -1 },
	    { -1, 0 },
	    { 0, -2 }
	};

	if (lastScaleFactor != scaleFactor) {
	    int			i;
	    lastScaleFactor = scaleFactor;
	    for (i = 1; i < 21; ++i) {
		mine_points[i].x = WINSCALE(world_mine_points[i].x);
		mine_points[i].y = WINSCALE(world_mine_points[i].y);
	    }
	}

	x = X(x);
	y = Y(y);
	mine_points[0].x = WINSCALE(x - 8);
	mine_points[0].y = WINSCALE(y - 1);
	if (teammine == 0) {
	    SET_FG(colors[BLUE].pixel);
	    rd.fillRectangle(dpy, drawPixmap, gameGC,
			WINSCALE(x - 7), WINSCALE(y - 2),
			WINSCALE(15), WINSCALE(5));
	}

	SET_FG(colors[WHITE].pixel);
	rd.drawLines(dpy, drawPixmap, gameGC,
		   mine_points, 21, CoordModePrevious);

	if (name)
	    Gui_paint_mine_name(x, y, name);
    }
    else {
	x = X(x);
	y = Y(y);
	if (teammine == 0) {
	    SET_FG(colors[BLUE].pixel);
	    Bitmap_paint(drawPixmap, BM_MINE_OTHER, WINSCALE(x - 10),
			 WINSCALE(y - 7), 0);
	}
	else {
	    SET_FG(colors[WHITE].pixel);
	    Bitmap_paint(drawPixmap, BM_MINE_TEAM, WINSCALE(x - 10),
			 WINSCALE(y - 7), 0);
	}

	if (name)
	    Gui_paint_mine_name(x, y, name);
    }
}


void Gui_paint_spark(int color, int x, int y)
{
    color = spark_color[color];

    Rectangle_add(color,
		  x - spark_size/2,
		  y - spark_size/2,
		  spark_size, spark_size);

}


void Gui_paint_wreck(int x, int y, bool deadly, int wtype, int rot, int size)
{
    int color, cnt, tx, ty;
    static XPoint points[NUM_WRECKAGE_POINTS+2];

    for (cnt = 0; cnt < NUM_WRECKAGE_POINTS; cnt++) {
	tx = (int)(wreckageShapes[wtype][cnt][rot].x * size) >> 8;
	ty = (int)(wreckageShapes[wtype][cnt][rot].y * size) >> 8;

	points[cnt].x = WINSCALE(X(x + tx));
	points[cnt].y = WINSCALE(Y(y + ty));

    }
    points[cnt++] = points[0];

    color = (deadly) ? WHITE: RED;
    SET_FG(colors[color].pixel);
    rd.drawLines(dpy, drawPixmap, gameGC, points, cnt, 0);
}


void Gui_paint_asteroid(int x, int y, int type, int rot, int size)
{
    int cnt, tx, ty;
    static XPoint points[NUM_ASTEROID_POINTS+2];

    for (cnt = 0; cnt < NUM_ASTEROID_POINTS; cnt++) {
	tx = (int)(asteroidShapes[type][cnt][rot].x * size * 1.4);
	ty = (int)(asteroidShapes[type][cnt][rot].y * size * 1.4);

	points[cnt].x = WINSCALE(X(x + tx));
	points[cnt].y = WINSCALE(Y(y + ty));
    }
    points[cnt++] = points[0];

    SET_FG(colors[WHITE].pixel);
    rd.drawLines(dpy, drawPixmap, gameGC, points, cnt, 0);
}


static void Gui_paint_nastyshot(int color, int x, int y, int size)
{
    int z = size;

    if (rfrac() < 0.5f) {
	Segment_add(color,
		    x - z, y - z,
		    x + z, y + z);
	Segment_add(color,
		    x + z, y - z,
		    x - z, y + z);
    } else {
	Segment_add(color,
		    x - z, y,
		    x  + z, y);
	Segment_add(color,
		    x, y - z,
		    x, y + z);
    }
}


void Gui_paint_fastshot(int color, int x, int y)
{
    /* this is for those pesky invisible shots */
    if (color == 0)
	return;

    if (!texturedObjects) {
        int z = shot_size/2;

	if (showNastyShots)
	    Gui_paint_nastyshot(color, x, y, z);
	else
	    Rectangle_add(color,
			  x - z,
			  y - z,
			  shot_size, shot_size);
    }
    else {
	int s_size = (shot_size > 8) ? 8 : shot_size ;
	int z = s_size / 2;
	Bitmap_paint(drawPixmap, BM_BULLET, WINSCALE(x) - z,
		     WINSCALE(y) - z, s_size - 1);
    }
}

void Gui_paint_teamshot(int x, int y)
{
    if (teamShotColor == 0)
	return;

    if (!texturedObjects)
	Gui_paint_nastyshot(teamShotColor, x, y, shot_size/2);
    else {
	int s_size = (teamshot_size > 8) ? 8 : shot_size ;
	int z = s_size / 2;
	Bitmap_paint(drawPixmap, BM_BULLET_OWN, WINSCALE(x) - z,
		     WINSCALE(y) - z, s_size - 1);
    }
}


void Gui_paint_missiles_begin(void)
{
    SET_FG(colors[WHITE].pixel);
    XSetLineAttributes(dpy, gameGC, 4,
		       LineSolid, CapButt, JoinMiter);
}


void Gui_paint_missiles_end(void)
{
    XSetLineAttributes(dpy, gameGC, 0,
		       LineSolid, CapButt, JoinMiter);
}


void Gui_paint_missile(int x, int y, int len, int dir)
{
   int		x_1, x_2, y_1, y_2;

    x_1 = X(x);
    y_1 = Y(y);
    x_2 = (int)(x_1 - tcos(dir) * len);
    y_2 = (int)(y_1 + tsin(dir) * len);
    rd.drawLine(dpy, drawPixmap, gameGC,
		WINSCALE(x_1), WINSCALE(y_1), WINSCALE(x_2), WINSCALE(y_2));
}


void Gui_paint_lasers_begin(void)
{
    XSetLineAttributes(dpy, gameGC, 3,
		       LineSolid, CapButt, JoinMiter);
}


void Gui_paint_lasers_end(void)
{
    XSetLineAttributes(dpy, gameGC, 0,
		       LineSolid, CapButt, JoinMiter);
}


void Gui_paint_laser(int color, int x_1, int y_1, int len, int dir)
{
    int		x_2, y_2;

    x_2 = (int)(x_1 + len * tcos(dir));
    y_2 = (int)(y_1 + len * tsin(dir));
    if ((unsigned)(color) >= NUM_COLORS)
	color = WHITE;
    SET_FG(colors[color].pixel);
    rd.drawLine(dpy, drawPixmap, gameGC,
		WINSCALE(X(x_1)), WINSCALE(Y(y_1)),
		WINSCALE(X(x_2)), WINSCALE(Y(y_2)));
}


void Gui_paint_paused(int x, int y, int count)
{
    if (!texturedObjects) {
	int		x_0, y_0;
	static int	pauseCharWidth = -1;
	const int	half_pause_size = 3*BLOCK_SZ/7;

	if (pauseCharWidth < 0)
	    pauseCharWidth = XTextWidth(gameFont, "P", 1);

	SET_FG(colors[BLUE].pixel);
	x_0 = X(x - half_pause_size);
	y_0 = Y(y + half_pause_size);
	rd.fillRectangle(dpy, drawPixmap, gameGC,
			 WINSCALE(x_0), WINSCALE(y_0),
			 WINSCALE(2*half_pause_size+1),
			 WINSCALE(2*half_pause_size+1));
	if (count <= 0 || loopsSlow % 10 >= 5) {
	    SET_FG(colors[WHITE].pixel);
	    rd.drawRectangle(dpy, drawPixmap, gameGC,
			     WINSCALE(x_0 - 1),
			     WINSCALE(y_0 - 1),
			     WINSCALE(2*(half_pause_size+1)),
			     WINSCALE(2*(half_pause_size+1)));
	    rd.drawString(dpy, drawPixmap, gameGC,
			  WINSCALE(X(x)) - pauseCharWidth/2,
			  WINSCALE(Y(y-1)) + gameFont->ascent/2,
			  "P", 1);
	}
    } else
	Bitmap_paint(drawPixmap, BM_PAUSED, WINSCALE(X(x - BLOCK_SZ / 2)),
		     WINSCALE(Y(y + BLOCK_SZ / 2)),
		     (count <= 0 || loopsSlow % 10 >= 5) ? 1 : 0);
}


/* Create better graphics for this. */
void Gui_paint_appearing(int x, int y, int id, int count)
{
    const int hsize = 3 * BLOCK_SZ / 7;

    /* Make a note we are doing the base warning */
    if (version >= 0x4F12) {
	homebase_t *base = Homebase_by_id(id);
	/* hack */
	if (base != NULL)
	    base->deathtime = loops + count;
    }

#if 1
    SET_FG(colors[RED].pixel);
    rd.fillRectangle(dpy, drawPixmap, gameGC,
		     SCALEX(x - hsize),
		     SCALEY(y - hsize + (int)(count / 180. * hsize + 1)),
		     WINSCALE(2 * hsize + 1),
		     WINSCALE((int)(count / 180. * hsize + 1)));
#else
    SET_FG(colors[RED].pixel);
    rd.fillRectangle(dpy, drawPixmap, gameGC, SCALEX(x - hsize), SCALEY(y + hsize),
		     WINSCALE(2 * hsize + 1), WINSCALE(2 * hsize + 1));
    SET_FG(colors[WHITE].pixel);
    count = 360 - count;
    if (count < 0)
	count = 0;
    rd.fillRectangle(dpy, drawPixmap, gameGC, SCALEX(x - hsize), SCALEY(y + hsize),
		     WINSCALE(2 * hsize + 1),
		     WINSCALE((int)(count / 180. * hsize + 1)));
#endif
}


void Gui_paint_ecm(int x, int y, int size)
{
    Arc_add(WHITE,
	    X(x - size / 2),
	    Y(y + size / 2),
	    size, size, 0, 64 * 360);
}


void Gui_paint_refuel(int x_0, int y_0, int x_1, int y_1)
{
    if (!texturedObjects) {
	rd.drawLine(dpy, drawPixmap, gameGC,
		    WINSCALE(X(x_0)), WINSCALE(Y(y_0)),
		    WINSCALE(X(x_1)), WINSCALE(Y(y_1)));
    }
    else {
	int size = WINSCALE(8);
	double dx, dy;
	int i;
	int fuel[16] = { 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 2, 1, 2, 3, 3, 2 };

	x_0 = WINSCALE(X(x_0));
	y_0 = WINSCALE(Y(y_0));
	x_1 = WINSCALE(X(x_1));
	y_1 = WINSCALE(Y(y_1));
	dx = (double)(x_1 - x_0) / 16;
	dy = (double)(y_1 - y_0) / 16;
	for (i = 0; i < 16; i++) {
	    Bitmap_paint(drawPixmap, BM_REFUEL,
			 (int)(x_0 + (dx * i) - size / 2),
			 (int)(y_0 + (dy * i) - size / 2),
			 fuel[(loops + 16 - i) % 16]);
	}
    }
}


void Gui_paint_connector(int x_0, int y_0, int x_1, int y_1, int tractor)
{
    if (tractor)
	rd.setDashes(dpy, gameGC, 0, cdashes, NUM_CDASHES);
    else
	rd.setDashes(dpy, gameGC, 0, dashes, NUM_DASHES);

    rd.drawLine(dpy, drawPixmap, gameGC,
	      WINSCALE(X(x_0)), WINSCALE(Y(y_0)),
	      WINSCALE(X(x_1)), WINSCALE(Y(y_1)));
    if (tractor)
	rd.setDashes(dpy, gameGC, 0, dashes, NUM_DASHES);
}


void Gui_paint_transporter(int x_0, int y_0, int x_1, int y_1)
{
    rd.drawLine(dpy, drawPixmap, gameGC,
		WINSCALE(X(x_0)), WINSCALE(Y(y_0)),
		WINSCALE(X(x_1)), WINSCALE(Y(y_1)));
}


void Gui_paint_all_connectors_begin(void)
{
    unsigned long	mask;

    SET_FG(colors[connColor].pixel);
    if (gcv.line_style != LineOnOffDash) {
	gcv.line_style = LineOnOffDash;
	mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
	mask |= GCDashOffset;
#endif
	XChangeGC(dpy, gameGC, mask, &gcv);
    }

}


void Gui_paint_ships_begin(void)
{
    gcv.dash_offset = WINSCALE(DASHES_LENGTH - (loops % DASHES_LENGTH));
}


void Gui_paint_ships_end(void)
{
   unsigned long	mask;
   if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	mask = GCLineStyle;
	XChangeGC(dpy, gameGC, mask, &gcv);
    }
    gcv.dash_offset = 0;
}


static void Gui_paint_rounddelay(int x, int y)
{
    char s[12];
    int	 t, text_width;

    sprintf(s, "%d", roundDelay / FPS);
    t = strlen(s);
    SET_FG(colors[WHITE].pixel);
    text_width = XTextWidth(gameFont, s, t);
    rd.drawString(dpy, drawPixmap, gameGC,
		  WINSCALE(X(x)) - text_width / 2,
		  WINSCALE(Y(y)) + gameFont->ascent/2,
		  s, t);
}


/*  Here starts the paint functions for ships  (MM) */
static void Gui_paint_ship_name(int x, int y, other_t *other)
{
    FIND_NAME_WIDTH(other);
    if (shipNameColor) {
	int color = Life_color(other);
	if (!color)
	    color = shipNameColor;

	SET_FG(colors[color].pixel);
	rd.drawString(dpy, drawPixmap, gameGC,
		      WINSCALE(X(x)) - other->name_width / 2,
		      WINSCALE(Y(y) + 16) + gameFont->ascent,
		      other->id_string, other->name_len);
    } else
	SET_FG(colors[BLUE].pixel);

    if (BIT(instruments, SHOW_LIVES_BY_SHIP)
	&& BIT(Setup->mode, LIMITED_LIVES)) {
	char keff[4] = "";

	sprintf(keff, "%03d", other->life);
	if (other->life < 1)
	    SET_FG(colors[WHITE].pixel);
	rd.drawString(dpy, drawPixmap, gameGC,
		      WINSCALE(X(x) + SHIP_SZ),
		      WINSCALE(Y(y) - SHIP_SZ) + gameFont->ascent,
		      &keff[2], 1);
    }
}


static int Gui_is_my_tank(other_t *other)
{
    char	tank_name[MAX_NAME_LEN];

    if (self == NULL
	|| other == NULL
	|| other->mychar != 'T'
	|| (BIT(Setup->mode, TEAM_PLAY)
	&& self->team != other->team)) {
	    return 0;
    }

    if (strlcpy(tank_name, self->name, MAX_NAME_LEN) < MAX_NAME_LEN)
	strlcat(tank_name, "'s tank", MAX_NAME_LEN);

    if (strcmp(tank_name, other->name))
	return 0;

    return 1;
}

/*
 * Assume MAX_TEAMS is 10
 */
int Team_color(int team)
{
    switch (team) {
    case 0:	return team0Color;
    case 1:	return team1Color;
    case 2:	return team2Color;
    case 3:	return team3Color;
    case 4:	return team4Color;
    case 5:	return team5Color;
    case 6:	return team6Color;
    case 7:	return team7Color;
    case 8:	return team8Color;
    case 9:	return team9Color;
    default:    break;
    }
    return 0;
}

int Life_color(other_t *other)
{
    int color = 0;/* default is 'no special color' */

    if (other
	&& (other->mychar == ' ' || other->mychar == 'R')
	&& BIT(Setup->mode, LIMITED_LIVES)) {
		color = Life_color_by_life(other->life);
    }
    return color;
}

int Life_color_by_life(int life) /* This function doesn't check stuff */
{
  int color;
	
	if (life > 2)
	    color = manyLivesColor;
	else if (life == 2)
	    color = twoLivesColor;
	else if (life == 1)
	    color = oneLifeColor;
	else /* we catch all */
	    color = zeroLivesColor;
  return color;
}

static int Gui_calculate_ship_color(int id, other_t *other)
{
    int ship_color = WHITE;

#ifndef NO_BLUE_TEAM
    if (BIT(Setup->mode, TEAM_PLAY)
	&& eyesId != id
	&& other != NULL
	&& eyeTeam == other->team) {
	/* Paint teammates and allies ships with last life in teamLWColor */
	if (BIT(Setup->mode, LIMITED_LIVES)
	    && (other->life == 0))
	    ship_color = teamLWColor;
	else
	    ship_color = BLUE;
    }

    if (eyes != NULL
	&& eyesId != id
	&& other != NULL
	&& eyes->alliance != ' '
	&& eyes->alliance == other->alliance) {
	/* Paint teammates and allies ships with last life in teamLWColor */
	if (BIT(Setup->mode, LIMITED_LIVES)
	    && (other->life == 0))
	    ship_color = teamLWColor;
	else
	    ship_color = BLUE;
    }

    if (Gui_is_my_tank(other))
	ship_color = BLUE;
#endif
    if (roundDelay > 0 && ship_color == WHITE)
	ship_color = RED;

    /* Check for team color */
    if (other && BIT(Setup->mode, TEAM_PLAY)) {
	int team_color = Team_color(other->team);
	if (team_color)
	    return team_color;
    }

    /* Vato color hack start, edited by mara & kps */
    if (BIT(Setup->mode, LIMITED_LIVES)) {
	/* Paint your ship in selfLWColor when on last life */
	if (eyes != NULL
	    && eyes->id == id
	    && eyes->life == 0) {
	    ship_color = selfLWColor;
	}

	/* Paint enemy ships with last life in enemyLWColor */
	if (eyes != NULL
	    && eyes->id != id
	    && other != NULL
	    && eyeTeam != other->team
	    && other->life == 0) {
	    ship_color = enemyLWColor;
	}
    }
    /* Vato color hack end */

    return ship_color;
}


static void Gui_paint_marking_lights(int id, int x, int y,
				     shipshape_t *ship, int dir)
{
    int lcnt;

    if (((loopsSlow + id) & 0xF) == 0) {
	for (lcnt = 0; lcnt < ship->num_l_light; lcnt++) {
	    position l_light = Ship_get_l_light_position(ship, lcnt, dir);
	    Rectangle_add(RED,
			  X(x + l_light.x) - 2,
			  Y(y + l_light.y) - 2,
			  6, 6);
	    Segment_add(RED,
			X(x + l_light.x)-8,
			Y(y + l_light.y),
			X(x + l_light.x)+8,
			Y(y + l_light.y));
	    Segment_add(RED,
			X(x + l_light.x),
			Y(y + l_light.y)-8,
			X(x + l_light.x),
			Y(y + l_light.y)+8);
	}
    } else if (((loopsSlow + id) & 0xF) == 2) {
	for (lcnt = 0; lcnt < ship->num_r_light; lcnt++) {
	    int rightLightColor = maxColors > 4 ? 4 : BLUE;
	    position r_light = Ship_get_r_light_position(ship, lcnt, dir);
	    Rectangle_add(rightLightColor,
			  X(x + r_light.x)-2,
			  Y(y + r_light.y)-2,
			  6, 6);
	    Segment_add(rightLightColor,
			X(x + r_light.x)-8,
			Y(y + r_light.y),
			X(x + r_light.x)+8,
			Y(y + r_light.y));
	    Segment_add(rightLightColor,
			X(x + r_light.x),
			Y(y + r_light.y)-8,
			X(x + r_light.x),
			Y(y + r_light.y)+8);
	}
    }
}


static void Gui_paint_shields_deflectors(int x, int y, int radius, int shield,
				  int deflector, int eshield, int ship_color)
{
    int		e_radius = radius + 4;
    int		half_radius = radius >> 1;
    int		half_e_radius = e_radius >> 1;
    int		scolor = -1;
    int		ecolor = -1;

    if (shield)
	scolor = ship_color;
    if (deflector)
	ecolor = loopsSlow & 0x02 ? RED : BLUE;
    if (eshield && shield) {
	if (ecolor != -1) {
	    scolor = ecolor;
	    ecolor = ship_color;
	} else
	    scolor = ecolor = ship_color;
    }

    if (ecolor != -1) {		/* outer shield */
	SET_FG(colors[ecolor].pixel);
	rd.drawArc(dpy, drawPixmap, gameGC,
		   WINSCALE(X(x - half_e_radius)),
		   WINSCALE(Y(y + half_e_radius)),
		   WINSCALE(e_radius), WINSCALE(e_radius),
		   0, 64 * 360);
    }
    if (scolor != -1) {
	SET_FG(colors[scolor].pixel);
	rd.drawArc(dpy, drawPixmap, gameGC,
		   WINSCALE(X(x - half_radius)),
		   WINSCALE(Y(y + half_radius)),
		   WINSCALE(radius), WINSCALE(radius),
		   0, 64 * 360);
    }
}

static void Set_drawstyle_dashed(int ship_color, int cloak);

static void Gui_paint_ship_cloaked(int ship_color, XPoint *points,
				   int point_count)
{
    Set_drawstyle_dashed(ship_color, 1);
    rd.drawLines(dpy, drawPixmap, gameGC, points, point_count, 0);
}

static void Gui_paint_ship_phased(int ship_color, XPoint *points,
				  int point_count)
{
    Gui_paint_ship_cloaked(ship_color, points, point_count);
}

static void generic_paint_ship(int x, int y, int ang, int ship)
{
    Bitmap_paint(drawPixmap, ship, WINSCALE(X(x) - 16), WINSCALE(Y(y) - 16), ang);
}


static void Gui_paint_ship_uncloaked(int id, XPoint *points,
				     int ship_color, int point_count)
{
    if (gcv.line_style != LineSolid) {
	gcv.line_style = LineSolid;
	XChangeGC(dpy, gameGC, GCLineStyle, &gcv);
    }
    SET_FG(colors[ship_color].pixel);
    rd.drawLines(dpy, drawPixmap, gameGC, points, point_count, 0);

    if (lock_id == id && id != -1 && lock_dist != 0)
	rd.fillPolygon(dpy, drawPixmap, gameGC,
		       points, point_count,
		       Complex, CoordModeOrigin);
}


static void Set_drawstyle_dashed(int ship_color, int cloak)
{
    int mask;
    if (gcv.line_style != LineOnOffDash) {
	gcv.line_style = LineOnOffDash;
	mask = GCLineStyle;
#ifndef NO_ROTATING_DASHES
	mask |= GCDashOffset;
#endif
	XChangeGC(dpy, gameGC, mask, &gcv);
    }
    SET_FG(colors[ship_color].pixel);
}


static int set_shipshape(int world_x, int world_y,
		  int dir, shipshape_t *ship, XPoint *points)
{
    int			cnt;
    register position	ship_point_pos;
    register XPoint	*xpts = points;
    int			window_x;
    int			window_y;

    for (cnt = 0; cnt < ship->num_points; cnt++) {
	ship_point_pos = Ship_get_point_position(ship, cnt, dir);
	window_x = X(world_x + ship_point_pos.x);
	window_y = Y(world_y + ship_point_pos.y);
	xpts->x = WINSCALE(window_x);
	xpts->y = WINSCALE(window_y);
	xpts++;
    }
    points[cnt++] = points[0];

    return cnt;
}


void Gui_paint_ship(int x, int y, int dir, int id, int cloak, int phased,
		    int shield, int deflector, int eshield)
{
    int			cnt, ship_color;
    other_t		*other;
    shipshape_t		*ship;
    XPoint		points[64];
    int			ship_shape;

    ship = Ship_by_id(id);
    other = Other_by_id(id);
    ship_color = WHITE;

    /* mara attempts similar behaviour to the kth ss hack */
    if ((!BIT(instruments, SHOW_SHIP_SHAPES))
	&& (self != NULL)
	&& (self->id != id))
	cnt = set_shipshape(x, y, dir, Default_ship(), points);
    else if ((!BIT(instruments, SHOW_MY_SHIP_SHAPE))
	       && (self != NULL)
	       && (self->id == id))
	cnt = set_shipshape(x, y, dir, Default_ship(), points);
    else
	cnt = set_shipshape(x, y, dir, ship, points);

    /*
     * Determine if the name of the player should be drawn below
     * his/her ship.
     */
    if (self != NULL
	&& self->id != id
	&& other != NULL)
	Gui_paint_ship_name(x, y, other);

    if (roundDelay > 0 && roundDelay % FPS < FPS/2) {
	Gui_paint_rounddelay(x, y);
	return;
    }

    ship_color = Gui_calculate_ship_color(id, other);

    if (cloak == 0 && phased == 0) {
	if (!texturedObjects || !texturedShips) {
	    Gui_paint_ship_uncloaked(id, points, ship_color, cnt);
	    /* shipshapeshack by Mara */
	    if (shipShapesHackColor >= 1) {
		Segment_add(ship_color /*shipShapesHackColor*/,
			    (X(x + SHIP_SZ * tcos(dir))),
			    (Y(y + SHIP_SZ * tsin(dir))),
			    (X(x + (SHIP_SZ + 12) * tcos(dir))),
			    (Y(y + (SHIP_SZ + 12) * tsin(dir))));
		Arc_add(ship_color /*shipShapesHackColor*/,
			X(x - SHIP_SZ), Y(y + SHIP_SZ),
			2 * SHIP_SZ, 2 * SHIP_SZ,
			0, 64 * 360);
	    }
	} else {
	    if (ship_color == BLUE)
		ship_shape = BM_SHIP_FRIEND;
	    else if (self != NULL && self->id != id)
		ship_shape = BM_SHIP_ENEMY;
	    else
		ship_shape = BM_SHIP_SELF;

	    generic_paint_ship(x, y, dir, ship_shape);
	}

    }

    if (phased)
	Gui_paint_ship_phased(ship_color, points, cnt);
    else if (cloak)
	Gui_paint_ship_cloaked(ship_color, points, cnt);

    if (markingLights)
        Gui_paint_marking_lights(id, x, y, ship, dir);

    if (shield || deflector) {
        Set_drawstyle_dashed(ship_color, cloak);
	Gui_paint_shields_deflectors(x, y, ship->shield_radius,
				    shield, deflector,
				    eshield, ship_color);
    }
}

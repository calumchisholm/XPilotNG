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

#include "xpcommon.h"

char shipshape_version[] = VERSION;

static int	Get_shape_keyword(char *keyw);
static int	shape2wire(char *ship_shape_str, shipshape_t *ship);
static void	Rotate_ship(shipshape_t *ship);

bool	debugShapeParsing = false;
bool	verboseShapeParsing = false;
bool	shapeLimits = true;
extern bool is_server;

/* kps - tmp hack */
shapepos_t *Shape_get_points(shape_t *s, int dir)
{
    int i;

    /* kps - optimize if cashed_dir == dir */
    for (i = 0; i < s->num_points; i++)
	s->cashed_pts[i] = s->pts[i][dir];

    return s->cashed_pts;
}

void Rotate_point(shapepos_t pt[RES])
{
    int			i;

    if (is_server) {
	for (i = 1; i < RES; i++) {
	    pt[i].clk.cx
		= (tcos(i) * pt[0].clk.cx - tsin(i) * pt[0].clk.cy) + .5;
	    pt[i].clk.cy
		= (tsin(i) * pt[0].clk.cx + tcos(i) * pt[0].clk.cy) + .5;
	}
    } else {
	for (i = 1; i < RES; i++) {
	    pt[i].pxl.x = (tcos(i) * pt[0].pxl.x - tsin(i) * pt[0].pxl.y) + .5;
	    pt[i].pxl.y = (tsin(i) * pt[0].pxl.x + tcos(i) * pt[0].pxl.y) + .5;
	}
    }
}

void Rotate_position(position_t pt[RES])
{
    int			i;

    for (i = 1; i < RES; i++) {
	pt[i].x = (tcos(i) * pt[0].x - tsin(i) * pt[0].y) + .5;
	pt[i].y = (tsin(i) * pt[0].x + tcos(i) * pt[0].y) + .5;
    }
}

void Rotate_ship(shipshape_t *ship)
{
    int			i;

    for (i = 0; i < ship->num_points; i++)
	Rotate_point(&ship->pts[i][0]);
    Rotate_point(&ship->engine[0]);
    Rotate_point(&ship->m_gun[0]);
    for (i = 0; i < ship->num_l_gun; i++)
	Rotate_point(&ship->l_gun[i][0]);
    for (i = 0; i < ship->num_r_gun; i++)
	Rotate_point(&ship->r_gun[i][0]);
    for (i = 0; i < ship->num_l_rgun; i++)
	Rotate_point(&ship->l_rgun[i][0]);
    for (i = 0; i < ship->num_r_rgun; i++)
	Rotate_point(&ship->r_rgun[i][0]);
    for (i = 0; i < ship->num_l_light; i++)
	Rotate_point(&ship->l_light[i][0]);
    for (i = 0; i < ship->num_r_light; i++)
	Rotate_point(&ship->r_light[i][0]);
    for (i = 0; i < ship->num_m_rack; i++)
	Rotate_point(&ship->m_rack[i][0]);
}


/*
 * Return a pointer to a default ship.
 * This function should always succeed,
 * therefore no malloc()ed memory is used.
 */
shipshape_t *Default_ship(void)
{
    static shipshape_t	sh;
    static shapepos_t	pts[6][RES];

    if (!sh.num_points) {
	ipos_t pos;

	sh.num_points = 3;

	sh.pts[0] = &pts[0][0];
	pos.x = 14;
	pos.y = 0;
	Ship_set_point(&sh, 0, pos);

	sh.pts[1] = &pts[1][0];
	pos.x = -8;
	pos.y = 8;
	Ship_set_point(&sh, 1, pos);

	sh.pts[2] = &pts[2][0];
	pos.x = -8;
	pos.y = -8;
	Ship_set_point(&sh, 2, pos);

	pos.x = -8;
	pos.y = 0;
	Ship_set_engine(&sh, pos);

	pos.x = 14;
	pos.y = 0;
	Ship_set_m_gun(&sh, pos);

	sh.num_l_light = 1;
	sh.l_light[0] = &pts[3][0];
	pos.x = -8;
	pos.y = 8;
	Ship_set_l_light(&sh, 0, pos);

	sh.num_r_light = 1;
	sh.r_light[0] = &pts[4][0];
	pos.x = -8;
	pos.y = -8;
	Ship_set_r_light(&sh, 0, pos);

	sh.num_m_rack = 1;
	sh.m_rack[0] = &pts[5][0];
	pos.x = 14;
	pos.y = 0;
	Ship_set_m_rack(&sh, 0, pos);

	sh.num_l_gun = sh.num_r_gun = sh.num_l_rgun = sh.num_r_rgun = 0;

	Make_table();

	Rotate_ship(&sh);
    }

    return &sh;
}

static int shape2wire(char *ship_shape_str, shipshape_t *ship)
{
/*
 * Macros to simplify limit-checking for ship points.
 * Until XPilot goes C++.
 */
#define GRID_PT(x,y)	grid.pt[(x)+15][(y)+15]
#define GRID_ADD(x,y)	(GRID_PT(x, y) = 2, \
			 grid.chk[grid.todo][0] = (x) + 15, \
			 grid.chk[grid.todo][1] = (y) + 15, \
			 grid.todo++)
#define GRID_GET(x,y)	((x) = (int)grid.chk[grid.done][0] - 15, \
			 (y) = (int)grid.chk[grid.done][1] - 15, \
			 grid.done++)
#define GRID_CHK(x,y)	(GRID_PT(x, y) == 2)
#define GRID_READY()	(grid.done >= grid.todo)
#define GRID_RESET()	(memset(grid.pt, 0, sizeof grid.pt), \
			 grid.done = 0, \
			 grid.todo = 0)

    struct grid_t {
	int		todo, done;
	unsigned char	pt[32][32];
	unsigned char	chk[32*32][2];
    } grid;

    int 		i, j, x, y, dx, dy, max, shape_version = 0;
    ipos_t 		pt[MAX_SHIP_PTS2], in, engine, m_gun,
			l_light[MAX_LIGHT_PTS],
			r_light[MAX_LIGHT_PTS],
			l_gun[MAX_GUN_PTS],
			r_gun[MAX_GUN_PTS],
			l_rgun[MAX_GUN_PTS],
			r_rgun[MAX_GUN_PTS],
			m_rack[MAX_RACK_PTS];
    bool		mainGunSet = false, engineSet = false;
    char		*str, *teststr;
    char 		keyw[20], buf[MSG_LEN];

    memset(ship, 0, sizeof(shipshape_t));

    if (debugShapeParsing)
	warn("parsing shape: %s", ship_shape_str);

    for (str = ship_shape_str; (str = strchr(str, '(' )) != NULL; ) {

	str++;

	if (shape_version == 0) {
	    if (isdigit(*str)) {
		if (verboseShapeParsing)
		    warn("Ship shape is in obsolete 3.1 format.");
		return -1;
	    } else
		shape_version = 0x3200;
	}

	for (i = 0; (keyw[i] = str[i]) != '\0'; i++) {
	    if (i == sizeof(keyw) - 1) {
		keyw[i] = '\0';
		break;
	    }
	    if (keyw[i] == ':') {
		keyw[i + 1] = '\0';
		break;
	    }
	}
	if (str[i] != ':') {
	    if (verboseShapeParsing)
		warn("Missing colon in ship shape: %s", keyw);
	    continue;
	}
	for (teststr = &buf[++i]; (buf[i] = str[i]) != '\0'; i++) {
	    if (buf[i] == ')' ) {
		buf[i++] = '\0';
		break;
	    }
	}
	str += i;

	switch (Get_shape_keyword(keyw)) {

	case 0:		/* Keyword is 'shape' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing ship shape coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_points >= MAX_SHIP_PTS) {
		    if (verboseShapeParsing)
			warn("Too many ship shape coordinates");
		} else {
		    pt[ship->num_points++] = in;
		    if (debugShapeParsing)
			warn("ship point at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 1:		/* Keyword is 'mainGun' */
	    if (mainGunSet) {
		if (verboseShapeParsing)
		    warn("Ship shape keyword \"%s\" multiple defined", keyw);
		break;
	    }
	    while (*teststr == ' ') teststr++;
	    if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		if (verboseShapeParsing)
		    warn("Missing main gun coordinate in: \"%s\"", teststr);
	    } else {
		m_gun = in;
		mainGunSet = true;
		if (debugShapeParsing)
		    warn("main gun at %d,%d", in.x, in.y);
	    }
	    break;

	case 2:		/* Keyword is 'leftGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing left gun coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_l_gun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing)
			warn("Too many left gun coordinates");
		} else {
		    l_gun[ship->num_l_gun++] = in;
		    if (debugShapeParsing)
			warn("left gun at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 3:		/* Keyword is 'rightGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing right gun coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_r_gun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing)
			warn("Too many right gun coordinates");
		} else {
		    r_gun[ship->num_r_gun++] = in;
		    if (debugShapeParsing)
			warn("right gun at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 4:		/* Keyword is 'leftLight' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing left light coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_l_light >= MAX_LIGHT_PTS) {
		    if (verboseShapeParsing)
			warn("Too many left light coordinates");
		} else {
		    l_light[ship->num_l_light++] = in;
		    if (debugShapeParsing)
			warn("left light at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 5:		/* Keyword is 'rightLight' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing right light coordinate in: \"%s\"",
			       teststr);
		    break;
		}
		if (ship->num_r_light >= MAX_LIGHT_PTS) {
		    if (verboseShapeParsing)
			warn("Too many right light coordinates");
		} else {
		    r_light[ship->num_r_light++] = in;
		    if (debugShapeParsing)
			warn("right light at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 6:		/* Keyword is 'engine' */
	    if (engineSet) {
		if (verboseShapeParsing)
		    warn("Ship shape keyword \"%s\" multiple defined", keyw);
		break;
	    }
	    while (*teststr == ' ') teststr++;
	    if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		if (verboseShapeParsing)
		    warn("Missing engine coordinate in: \"%s\"", teststr);
	    } else {
		engine = in;
		engineSet = true;
		if (debugShapeParsing)
		    warn("engine at %d,%d", in.x, in.y);
	    }
	    break;

	case 7:		/* Keyword is 'missileRack' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing) {
			warn("Missing missile rack coordinate in: \"%s\"",
			     teststr);
		    }
		    break;
		}
		if (ship->num_m_rack >= MAX_RACK_PTS) {
		    if (verboseShapeParsing)
			warn("Too many missile rack coordinates");
		} else {
		    m_rack[ship->num_m_rack++] = in;
		    if (debugShapeParsing)
			warn("missile rack at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 8:		/* Keyword is 'name' */
#ifdef	_NAMEDSHIPS
	    ship->name = xp_strdup(teststr);
	    /* ship->name[strlen(ship->name)-1] = '\0'; */
#endif
	    break;

	case 9:		/* Keyword is 'author' */
#ifdef	_NAMEDSHIPS
	    ship->author = xp_strdup(teststr);
	    /* ship->author[strlen(ship->author)-1] = '\0'; */
#endif
	    break;

	case 10:		/* Keyword is 'leftRearGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing left rear gun coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_l_rgun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing)
			warn("Too many left rear gun coordinates");
		} else {
		    l_rgun[ship->num_l_rgun++] = in;
		    if (debugShapeParsing)
			warn("left rear gun at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	case 11:		/* Keyword is 'rightRearGun' */
	    while (teststr) {
		while (*teststr == ' ') teststr++;
		if (sscanf(teststr, "%d,%d", &in.x, &in.y) != 2) {
		    if (verboseShapeParsing)
			warn("Missing right rear gun coordinate in: \"%s\"",
			     teststr);
		    break;
		}
		if (ship->num_r_rgun >= MAX_GUN_PTS) {
		    if (verboseShapeParsing)
			warn("Too many right rear gun coordinates");
		} else {
		    r_rgun[ship->num_r_rgun++] = in;
		    if (debugShapeParsing)
			warn("right rear gun at %d,%d", in.x, in.y);
		}
		teststr = strchr(teststr, ' ');
	    }
	    break;

	default:
	    if (verboseShapeParsing)
		warn("Invalid ship shape keyword: \"%s\"", keyw);
	    /* the good thing about this format is that we can just ignore
	     * this.  it is likely to be a new extension we don't know
	     * about yet. */
	    break;
	}
    }

    /* Check for some things being set, and give them defaults if not */
    if (ship->num_points < 3) {
	if (verboseShapeParsing)
	    warn("not enough ship points defined");
	return -1;
    }

    /* If no main gun set, put at foremost point */
    if (!mainGunSet) {
	max = 0;
	for (i = 1; i < ship->num_points; i++) {
	    if (pt[i].x > pt[max].x
		|| (pt[i].x == pt[max].x
		    && ABS(pt[i].y) < ABS(pt[max].y)))
		max = i;
	}
	m_gun = pt[max];
	mainGunSet = true;
    }

    /* If no left light set, put at leftmost point */
    if (!ship->num_l_light) {
	max = 0;
	for (i = 1; i < ship->num_points; i++) {
	    if (pt[i].y > pt[max].y
		|| (pt[i].y == pt[max].y
		    && pt[i].x <= pt[max].x))
		max = i;
	}
	l_light[0] = pt[max];
	ship->num_l_light = 1;
    }

    /* If no right light set, put at rightmost point */
    if (!ship->num_r_light) {
	max = 0;
	for (i = 1; i < ship->num_points; i++) {
	    if (pt[i].y < pt[max].y
		|| (pt[i].y == pt[max].y
		    && pt[i].x <= pt[max].x))
		max = i;
	}
	r_light[0] = pt[max];
	ship->num_r_light = 1;
    }

    /* If no engine position, put at rear of ship */
    if (!engineSet) {
	max = 0;
	for (i = 1; i < ship->num_points; i++) {
	    if (pt[i].x < pt[max].x)
		max = i;
	}
	/* this may lay outside of ship. */
	engine.x = pt[max].x;
	engine.y = 0;
	engineSet = true;
    }

    /* If no missile racks, put at main gun position*/
    if (!ship->num_m_rack) {
	m_rack[0] = m_gun;
	ship->num_m_rack = 1;
    }

    if (shapeLimits) {
	const int	isLow = -8, isHi = 8, isLeft = 8, isRight = -8,
			minLow = 1, minHi = 1, minLeft = 1, minRight = 1,
			horMax = 15, verMax = 15, horMin = -15, verMin = -15,
			minCount = 3, minSize = 22 + 16;
	int		low = 0, hi = 0, left = 0, right = 0,
			count = 0, change,
			lowest = 0, highest = 0,
			leftmost = 0, rightmost = 0;
	int		invalid = 0;
	const int	checkWidthAgainstLongestAxis = 1;

	max = 0;
	for (i = 0; i < ship->num_points; i++) {
	    x = pt[i].x;
	    y = pt[i].y;
	    change = 0;
	    if (y >= isLeft) {
		change++, left++;
		if (y > leftmost) leftmost = y;
	    }
	    if (y <= isRight) {
		change++, right++;
		if (y < rightmost) rightmost = y;
	    }
	    if (x <= isLow) {
		change++, low++;
		if (x < lowest) lowest = x;
	    }
	    if (x >= isHi) {
		change++, hi++;
		if (x > highest) highest = x;
	    }
	    if (change) count++;
	    if (y > horMax || y < horMin) max++;
	    if (x > verMax || x < verMin) max++;
	}
	if (low < minLow
	    || hi < minHi
	    || left < minLeft
	    || right < minRight
	    || count < minCount) {
	    if (verboseShapeParsing)
		warn("Ship shape does not meet size requirements "
		     "(%d,%d,%d,%d,%d)", low, hi, left, right, count);
	    return -1;
	}
	if (max) {
	    if (verboseShapeParsing)
		warn("Ship shape exceeds size maxima.");
	    return -1;
	}
	if (leftmost - rightmost + highest - lowest < minSize) {
	    if (verboseShapeParsing)
		warn("Ship shape is not big enough.\n"
		     "The ship's width and height added together should\n"
		     "be at least %d.", minSize);
	    return -1;
	}

	if (checkWidthAgainstLongestAxis) {
	    /*
	     * For making sure the ship is the right width!
	     */
	    int pair[2];
	    int dist = 0, tmpDist = 0;
	    double vec[2], width, dTmp;
	    const int minWidth = 12;

	    /*
	     * Loop over all the points and find the two furthest apart
	     */
	    for (i = 0; i < ship->num_points; i++) {
		for (j = i + 1; j < ship->num_points; j++) {
		    /*
		     * Compare the points if they are not the same ones.
		     * Get this distance -- doesn't matter about sqrting
		     * it since only size is important.
		     */
		    if ((tmpDist = ((pt[i].x - pt[j].x) * (pt[i].x - pt[j].x) +
				    (pt[i].y - pt[j].y) * (pt[i].y - pt[j].y)))
			> dist) {
			/*
			 * Set new separation thingy.
			 */
			dist = tmpDist;
			pair[0] = i;
			pair[1] = j;
		    }
		}
	    }

	    /*
	     * Now we know the vector that is _|_ to the one above
	     * is simply found by (x,y) -> (y,-x) => dot-prod = 0
	     */
	    vec[0] = (double)(pt[pair[1]].y - pt[pair[0]].y);
	    vec[1] = (double)(pt[pair[0]].x - pt[pair[1]].x);

	    /*
	     * Normalise
	     */
	    dTmp = LENGTH(vec[0], vec[1]);
	    vec[0] /= dTmp;
	    vec[1] /= dTmp;

	    /*
	     * Now check the width _|_ to the ship main line.
	     */
	    for (i = 0, width = dTmp = 0.0; i < ship->num_points; i++) {
		for (j = i + 1; j < ship->num_points; j++) {
		    /*
		     * Check the line if the points are not the same ones
		     */
		    width = fabs(vec[0] * (double)(pt[i].x - pt[j].x) +
				 vec[1] * (double)(pt[i].y - pt[j].y));
		    if (width > dTmp)
			dTmp = width;
		}
	    }

	    /*
	     * And make sure it is nice and far away
	     */
	    if (((int)dTmp) < minWidth) {
		if (verboseShapeParsing)
		    warn("Ship shape is not big enough.\n"
			 "The ship's width should be at least %d.\n"
			 "Player's is %d", minWidth, (int)dTmp);
		return -1;
	    }
	}

	/*
	 * Check that none of the special points are outside the
	 * shape defined by the normal points.
	 * First the shape is drawn on a grid.  Then all grid points
	 * on the outside of the shape are marked.  Thusly for each
	 * special point can be determined if it is outside the shape.
	 */
	GRID_RESET();

	/* Draw the ship outline first. */
	for (i = 0; i < ship->num_points; i++) {
	    j = i + 1;
	    if (j == ship->num_points) j = 0;

	    GRID_PT(pt[i].x, pt[i].y) = 1;

	    dx = pt[j].x - pt[i].x;
	    dy = pt[j].y - pt[i].y;
	    if (ABS(dx) >= ABS(dy)) {
		if (dx > 0) {
		    for (x = pt[i].x + 1; x < pt[j].x; x++) {
			y = pt[i].y + (dy * (x - pt[i].x)) / dx;
			GRID_PT(x, y) = 1;
		    }
		} else {
		    for (x = pt[j].x + 1; x < pt[i].x; x++) {
			y = pt[j].y + (dy * (x - pt[j].x)) / dx;
			GRID_PT(x, y) = 1;
		    }
		}
	    } else {
		if (dy > 0) {
		    for (y = pt[i].y + 1; y < pt[j].y; y++) {
			x = pt[i].x + (dx * (y - pt[i].y)) / dy;
			GRID_PT(x, y) = 1;
		    }
		} else {
		    for (y = pt[j].y + 1; y < pt[i].y; y++) {
			x = pt[j].x + (dx * (y - pt[j].y)) / dy;
			GRID_PT(x, y) = 1;
		    }
		}
	    }
	}

	/* Check the borders of the grid for blank points. */
	for (y = -15; y <= 15; y++) {
	    for (x = -15; x <= 15; x += (y == -15 || y == 15) ? 1 : 2*15) {
		if (GRID_PT(x, y) == 0)
		    GRID_ADD(x, y);
	    }
	}

	/* Check from the borders of the grid to the centre. */
	while (!GRID_READY()) {
	    GRID_GET(x, y);
	    if (x <  15 && GRID_PT(x + 1, y) == 0) GRID_ADD(x + 1, y);
	    if (x > -15 && GRID_PT(x - 1, y) == 0) GRID_ADD(x - 1, y);
	    if (y <  15 && GRID_PT(x, y + 1) == 0) GRID_ADD(x, y + 1);
	    if (y > -15 && GRID_PT(x, y - 1) == 0) GRID_ADD(x, y - 1);
	}

	/*
	 * Note that for the engine, old format shapes may well have the
	 * engine position outside the ship, so this check not used for those.
	 */

	if (GRID_CHK(m_gun.x, m_gun.y)) {
	    if (verboseShapeParsing)
		warn("Main gun outside ship");
	    invalid++;
	}
	for (i = 0; i < ship->num_l_gun; i++) {
	    if (GRID_CHK(l_gun[i].x, l_gun[i].y)) {
		if (verboseShapeParsing)
		    warn("Left gun %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_r_gun; i++) {
	    if (GRID_CHK(r_gun[i].x, r_gun[i].y)) {
		if (verboseShapeParsing)
		    warn("Right gun %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_l_rgun; i++) {
	    if (GRID_CHK(l_rgun[i].x, l_rgun[i].y)) {
		if (verboseShapeParsing)
		    warn("Left rear gun %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_r_rgun; i++) {
	    if (GRID_CHK(r_rgun[i].x, r_rgun[i].y)) {
		if (verboseShapeParsing)
		    warn("Right rear gun %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_m_rack; i++) {
	    if (GRID_CHK(m_rack[i].x, m_rack[i].y)) {
		if (verboseShapeParsing)
		    warn("Missile rack %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_l_light; i++) {
	    if (GRID_CHK(l_light[i].x, l_light[i].y)) {
		if (verboseShapeParsing)
		    warn("Left light %d outside ship", i);
		invalid++;
	    }
	}
	for (i = 0; i < ship->num_r_light; i++) {
	    if (GRID_CHK(r_light[i].x, r_light[i].y)) {
		if (verboseShapeParsing)
		    warn("Right light %d outside ship", i);
		invalid++;
	    }
	}
	if (GRID_CHK(engine.x, engine.y)) {
	    if (verboseShapeParsing)
		warn("Engine outside of ship");
	    invalid++;
	}

	if (debugShapeParsing) {
	    for (i = -15; i <= 15; i++) {
		for (j = -15; j <= 15; j++) {
		    switch (GRID_PT(j, i)) {
		    case 0: putchar(' '); break;
		    case 1: putchar('*'); break;
		    case 2: putchar('.'); break;
		    default: putchar('?'); break;
		    }
		}
		putchar('\n');
	    }
	}

	if (invalid)
	    return -1;
    }

    ship->num_orig_points = ship->num_points;

    /*MARA evil hack*/
    /* always do SSHACK on server, it seems to work */
    if (is_server) {
	pt[ship->num_points] = pt[0];
	for (i = 1; i < ship->num_points; i++)
	    pt[i + ship->num_points] = pt[ship->num_points - i];
	ship->num_points = ship->num_points * 2;
    }
    /*MARA evil hack*/

    i = sizeof(shapepos_t) * RES;
    if (!(ship->pts[0] = malloc((size_t)ship->num_points * i))
	|| (ship->num_l_gun
	    && !(ship->l_gun[0] = malloc((size_t)ship->num_l_gun * i)))
	|| (ship->num_r_gun
	    && !(ship->r_gun[0] = malloc((size_t)ship->num_r_gun * i)))
	|| (ship->num_l_rgun
	    && !(ship->l_rgun[0] = malloc((size_t)ship->num_l_rgun * i)))
	|| (ship->num_r_rgun
	    && !(ship->r_rgun[0] = malloc((size_t)ship->num_r_rgun * i)))
	|| (ship->num_l_light
	    && !(ship->l_light[0] = malloc((size_t)ship->num_l_light * i)))
	|| (ship->num_r_light
	    && !(ship->r_light[0] = malloc((size_t)ship->num_r_light * i)))
	|| (ship->num_m_rack
	    && !(ship->m_rack[0] = malloc((size_t)ship->num_m_rack * i)))) {
	error("Not enough memory for ship shape");
	XFREE(ship->pts[0]);
	XFREE(ship->l_gun[0]);
	XFREE(ship->r_gun[0]);
	XFREE(ship->l_rgun[0]);
	XFREE(ship->r_rgun[0]);
	XFREE(ship->l_light[0]);
	XFREE(ship->r_light[0]);
	XFREE(ship->m_rack[0]);
	return -1;
    }

    for (i = 1; i < ship->num_points; i++)
	ship->pts[i] = &ship->pts[i - 1][RES];

    for (i = 1; i < ship->num_l_gun; i++)
	ship->l_gun[i] = &ship->l_gun[i - 1][RES];

    for (i = 1; i < ship->num_r_gun; i++)
	ship->r_gun[i] = &ship->r_gun[i - 1][RES];

    for (i = 1; i < ship->num_l_rgun; i++)
	ship->l_rgun[i] = &ship->l_rgun[i - 1][RES];

    for (i = 1; i < ship->num_r_rgun; i++)
	ship->r_rgun[i] = &ship->r_rgun[i - 1][RES];

    for (i = 1; i < ship->num_l_light; i++)
	ship->l_light[i] = &ship->l_light[i - 1][RES];

    for (i = 1; i < ship->num_r_light; i++)
	ship->r_light[i] = &ship->r_light[i - 1][RES];

    for (i = 1; i < ship->num_m_rack; i++)
	ship->m_rack[i] = &ship->m_rack[i - 1][RES];


    for (i = 0; i < ship->num_points; i++)
	Ship_set_point(ship, i, pt[i]);

    if (engineSet)
	Ship_set_engine(ship, engine);

    if (mainGunSet)
	Ship_set_m_gun(ship, m_gun);

    for (i = 0; i < ship->num_l_gun; i++)
	Ship_set_l_gun(ship, i, l_gun[i]);

    for (i = 0; i < ship->num_r_gun; i++)
	Ship_set_r_gun(ship, i, r_gun[i]);

    for (i = 0; i < ship->num_l_rgun; i++)
	Ship_set_l_rgun(ship, i, l_rgun[i]);

    for (i = 0; i < ship->num_r_rgun; i++)
	Ship_set_r_rgun(ship, i, r_rgun[i]);

    for (i = 0; i < ship->num_l_light; i++)
	Ship_set_l_light(ship, i, l_light[i]);

    for (i = 0; i < ship->num_r_light; i++)
	Ship_set_r_light(ship, i, r_light[i]);

    for (i = 0; i < ship->num_m_rack; i++)
	Ship_set_m_rack(ship, i, m_rack[i]);

    Rotate_ship(ship);

    return 0;
}

static shipshape_t *do_parse_shape(char *str)
{
    shipshape_t		*ship;

    if (!str || !*str) {
	if (debugShapeParsing)
	    warn("shape str not set");
	return Default_ship();
    }
    if (!(ship = malloc(sizeof(*ship)))) {
	error("No mem for ship shape");
	return Default_ship();
    }
    if (shape2wire(str, ship) != 0) {
	free(ship);
	if (debugShapeParsing)
	    warn("shape2wire failed");
	return Default_ship();
    }
    if (debugShapeParsing)
	warn("shape2wire succeeded");

    return(ship);
}

void Free_ship_shape(shipshape_t *ship)
{
    if (ship != NULL && ship != Default_ship()) {
	if (ship->num_points > 0)  XFREE(ship->pts[0]);
	if (ship->num_l_gun > 0)   XFREE(ship->l_gun[0]);
	if (ship->num_r_gun > 0)   XFREE(ship->r_gun[0]);
	if (ship->num_l_rgun > 0)  XFREE(ship->l_rgun[0]);
	if (ship->num_r_rgun > 0)  XFREE(ship->r_rgun[0]);
	if (ship->num_l_light > 0) XFREE(ship->l_light[0]);
	if (ship->num_r_light > 0) XFREE(ship->r_light[0]);
	if (ship->num_m_rack > 0)  XFREE(ship->m_rack[0]);
#ifdef	_NAMEDSHIPS
	if (ship->name) free(ship->name);
	if (ship->author) free(ship->author);
#endif
	free(ship);
    }
}

shipshape_t *Parse_shape_str(char *str)
{
    if (is_server)
	verboseShapeParsing = debugShapeParsing;
    else
	verboseShapeParsing = true;
    shapeLimits = true;
    return do_parse_shape(str);
}

shipshape_t *Convert_shape_str(char *str)
{
    verboseShapeParsing = debugShapeParsing;
    shapeLimits = debugShapeParsing;
    return do_parse_shape(str);
}

/*
 * Returns 0 if ships is not valid, 1 if valid.
 */
int Validate_shape_str(char *str)
{
    shipshape_t		*ship;

    verboseShapeParsing = true;
    shapeLimits = true;
    ship = do_parse_shape(str);
    Free_ship_shape(ship);
    return (ship && ship != Default_ship());
}

void Convert_ship_2_string(shipshape_t *ship, char *buf, char *ext,
			   unsigned shape_version)
{
    char tmp[MSG_LEN];
    int i, buflen = 0, extlen, tmplen;

    ext[extlen = 0] = '\0';

    if (shape_version >= 0x3200) {
	position_t engine, m_gun;
	strcpy(buf, "(SH:");
	buflen = strlen(&buf[0]);
	for (i = 0; i < ship->num_orig_points && i < MAX_SHIP_PTS; i++) {
	    position_t pt = Ship_get_point_position(ship, i, 0);
	    sprintf(&buf[buflen], " %d,%d", (int)pt.x, (int)pt.y);
	    buflen += strlen(&buf[buflen]);
	}
	engine = Ship_get_engine_position(ship, 0);
	m_gun = Ship_get_m_gun_position(ship, 0);
	sprintf(&buf[buflen], ")(EN: %d,%d)(MG: %d,%d)",
		(int)engine.x, (int)engine.y,
		(int)m_gun.x, (int)m_gun.y);
	buflen += strlen(&buf[buflen]);

	/*
	 * If the calculations are correct then only from here on
	 * there is danger for overflowing the MSG_LEN size
	 * of the buffer.  Therefore first copy a new pair of
	 * parentheses into a temporary buffer and when the closing
	 * parenthesis is reached then determine if there is enough
	 * room in the main buffer or else copy it into the extended
	 * buffer.  This scheme allows cooperation with versions which
	 * didn't had the extended buffer yet for which the extended
	 * buffer will simply be discarded.
	 */
	if (ship->num_l_gun > 0) {
	    strcpy(&tmp[0], "(LG:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_l_gun && i < MAX_GUN_PTS; i++) {
		position_t l_gun = Ship_get_l_gun_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)l_gun.x, (int)l_gun.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_r_gun > 0) {
	    strcpy(&tmp[0], "(RG:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_r_gun && i < MAX_GUN_PTS; i++) {
		position_t r_gun = Ship_get_r_gun_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)r_gun.x, (int)r_gun.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_l_rgun > 0) {
	    strcpy(&tmp[0], "(LR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_l_rgun && i < MAX_GUN_PTS; i++) {
		position_t l_rgun = Ship_get_l_rgun_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)l_rgun.x, (int)l_rgun.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_r_rgun > 0) {
	    strcpy(&tmp[0], "(RR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_r_rgun && i < MAX_GUN_PTS; i++) {
		position_t r_rgun = Ship_get_r_rgun_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)r_rgun.x, (int)r_rgun.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_l_light > 0) {
	    strcpy(&tmp[0], "(LL:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_l_light && i < MAX_LIGHT_PTS; i++) {
		position_t l_light = Ship_get_l_light_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)l_light.x, (int)l_light.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_r_light > 0) {
	    strcpy(&tmp[0], "(RL:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_r_light && i < MAX_LIGHT_PTS; i++) {
		position_t r_light = Ship_get_r_light_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)r_light.x, (int)r_light.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
	if (ship->num_m_rack > 0) {
	    strcpy(&tmp[0], "(MR:");
	    tmplen = strlen(&tmp[0]);
	    for (i = 0; i < ship->num_m_rack && i < MAX_RACK_PTS; i++) {
		position_t m_rack = Ship_get_m_rack_position(ship, i, 0);
		sprintf(&tmp[tmplen], " %d,%d",
			(int)m_rack.x, (int)m_rack.y);
		tmplen += strlen(&tmp[tmplen]);
	    }
	    strcpy(&tmp[tmplen], ")");
	    tmplen++;
	    if (buflen + tmplen < MSG_LEN) {
		strcpy(&buf[buflen], tmp);
		buflen += tmplen;
	    } else if (extlen + tmplen < MSG_LEN) {
		strcpy(&ext[extlen], tmp);
		extlen += tmplen;
	    }
	}
    } else
	buf[0] = '\0';

    if (buflen >= MSG_LEN || extlen >= MSG_LEN)
	warn("BUG: convert ship: buffer overflow (%d,%d)", buflen, extlen);

    if (debugShapeParsing)
	warn("ship 2 str: %s %s", buf, ext);
}


static int Get_shape_keyword(char *keyw)
{
#define NUM_SHAPE_KEYS	12

    static char		shape_keys[NUM_SHAPE_KEYS][16] = {
			    "shape:",
			    "mainGun:",
			    "leftGun:",
			    "rightGun:",
			    "leftLight:",
			    "rightLight:",
			    "engine:",
			    "missileRack:",
			    "name:",
			    "author:",
			    "leftRearGun:",
			    "rightRearGun:",
			};
    static char		abbrev_keys[NUM_SHAPE_KEYS][4] = {
			    "SH:",
			    "MG:",
			    "LG:",
			    "RG:",
			    "LL:",
			    "RL:",
			    "EN:",
			    "MR:",
			    "NM:",
			    "AU:",
			    "LR:",
			    "RR:",
			};
    int			i;

    /* non-abbreviated keywords start with a lower case letter. */
    if (islower(*keyw)) {
	for (i = 0; i < NUM_SHAPE_KEYS; i++) {
	    if (!strcmp(keyw, shape_keys[i]))
		break;
	}
    }
    /* abbreviated keywords start with an upper case letter. */
    else if (isupper(*keyw)) {
	for (i = 0; i < NUM_SHAPE_KEYS; i++) {
	    if (!strcmp(keyw, abbrev_keys[i]))
		break;
	}
    }
    /* dunno what this is. */
    else
	i = -1;

    return(i);
}

void Calculate_shield_radius(shipshape_t *ship)
{
    int			i;
    int			radius2, max_radius = 0;

    for (i = 0; i < ship->num_points; i++) {
	position_t pti = Ship_get_point_position(ship, i, 0);
	radius2 = (int)(sqr(pti.x) + sqr(pti.y));
	if (radius2 > max_radius)
	    max_radius = radius2;
    }
    max_radius = (int)(2.0 * sqrt((double)max_radius));
    ship->shield_radius = (max_radius + 2 <= 34)
			? 34
			: (max_radius + 2 - (max_radius & 1));
}

shapepos_t Ship_get_point(shipshape_t *ship, int i, int dir)
{
    return ship->pts[i][dir];
}
shapepos_t Ship_get_engine(shipshape_t *ship, int dir)
{
    return ship->engine[dir];
}
shapepos_t Ship_get_m_gun(shipshape_t *ship, int dir)
{
    return ship->m_gun[dir];
}
shapepos_t Ship_get_l_gun(shipshape_t *ship, int gun, int dir)
{
    return ship->l_gun[gun][dir];
}
shapepos_t Ship_get_r_gun(shipshape_t *ship, int gun, int dir)
{
    return ship->r_gun[gun][dir];
}
shapepos_t Ship_get_l_rgun(shipshape_t *ship, int gun, int dir)
{
    return ship->l_rgun[gun][dir];
}
shapepos_t Ship_get_r_rgun(shipshape_t *ship, int gun, int dir)
{
    return ship->r_rgun[gun][dir];
}
shapepos_t Ship_get_l_light(shipshape_t *ship, int l, int dir)
{
    return ship->l_light[l][dir];
}
shapepos_t Ship_get_r_light(shipshape_t *ship, int l, int dir)
{
    return ship->r_light[l][dir];
}
shapepos_t Ship_get_m_rack(shipshape_t *ship, int rack, int dir)
{
    return ship->m_rack[rack][dir];
}


void Ship_set_point(shipshape_t *ship, int i, ipos_t pos)
{
    ship->pts[i][0] = ipos2shapepos(pos);
}

void Ship_set_engine(shipshape_t *ship, ipos_t pos)
{
    ship->engine[0] = ipos2shapepos(pos);
}

void Ship_set_m_gun(shipshape_t *ship, ipos_t pos)
{
    ship->m_gun[0] = ipos2shapepos(pos);
}

void Ship_set_l_gun(shipshape_t *ship, int i, ipos_t pos)
{
    ship->l_gun[i][0] = ipos2shapepos(pos);
}

void Ship_set_r_gun(shipshape_t *ship, int i, ipos_t pos)
{
    ship->r_gun[i][0] = ipos2shapepos(pos);
}

void Ship_set_l_rgun(shipshape_t *ship, int i, ipos_t pos)
{
    ship->l_rgun[i][0] = ipos2shapepos(pos);
}

void Ship_set_r_rgun(shipshape_t *ship, int i, ipos_t pos)
{
    ship->r_rgun[i][0] = ipos2shapepos(pos);
}

void Ship_set_l_light(shipshape_t *ship, int i, ipos_t pos)
{
    ship->l_light[i][0] = ipos2shapepos(pos);
}

void Ship_set_r_light(shipshape_t *ship, int i, ipos_t pos)
{
    ship->r_light[i][0] = ipos2shapepos(pos);
}

void Ship_set_m_rack(shipshape_t *ship, int i, ipos_t pos)
{
    ship->m_rack[i][0] = ipos2shapepos(pos);
}

shapepos_t ipos2shapepos(ipos_t pos)
{
    shapepos_t pt;

    if (is_server) {
	pt.clk.cx = PIXEL_TO_CLICK(pos.x);
	pt.clk.cy = PIXEL_TO_CLICK(pos.y);
    } else {
	pt.pxl.x = pos.x;
	pt.pxl.y = pos.y;
    }

    return pt;
}

position_t shapepos2position(shapepos_t pt)
{
    if (is_server) {
	position_t pos;
	pos.x = CLICK_TO_FLOAT(pt.clk.cx);
	pos.y = CLICK_TO_FLOAT(pt.clk.cy);
	return pos;
    }
    /* client */
    return pt.pxl;
}

position_t Ship_get_point_position(shipshape_t *ship, int i, int dir)
{
    return shapepos2position(Ship_get_point(ship, i, dir));
}
position_t Ship_get_engine_position(shipshape_t *ship, int dir)
{
    return shapepos2position(Ship_get_engine(ship, dir));
}
position_t Ship_get_m_gun_position(shipshape_t *ship, int dir)
{
    return shapepos2position(Ship_get_m_gun(ship, dir));
}
position_t Ship_get_l_gun_position(shipshape_t *ship, int gun, int dir)
{
    return shapepos2position(Ship_get_l_gun(ship, gun, dir));
}
position_t Ship_get_r_gun_position(shipshape_t *ship, int gun, int dir)
{
    return shapepos2position(Ship_get_r_gun(ship, gun, dir));
}
position_t Ship_get_l_rgun_position(shipshape_t *ship, int gun, int dir)
{
    return shapepos2position(Ship_get_l_rgun(ship, gun, dir));
}
position_t Ship_get_r_rgun_position(shipshape_t *ship, int gun, int dir)
{
    return shapepos2position(Ship_get_r_rgun(ship, gun, dir));
}
position_t Ship_get_l_light_position(shipshape_t *ship, int l, int dir)
{
    return shapepos2position(Ship_get_l_light(ship, l, dir));
}
position_t Ship_get_r_light_position(shipshape_t *ship, int l, int dir)
{
    return shapepos2position(Ship_get_r_light(ship, l, dir));
}
position_t Ship_get_m_rack_position(shipshape_t *ship, int rack, int dir)
{
    return shapepos2position(Ship_get_m_rack(ship, rack, dir));
}

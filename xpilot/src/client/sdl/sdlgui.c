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

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"
#include "sdlpaint.h"
#include "images.h"
#include "text.h"

int null    = 0x00000000;
int black   = 0x000000ff;
int white   = 0xffffffff;
int blue    = 0x0000ffff;
int red     = 0xff0000ff;
int green   = 0x00ff00ff;

int wallColor		    = 0x0000ffff;
int hudColor		    = 0xff000088;
int connColor	    	    = 0xff880088;
int scoreObjectColor 	    = 0x00ff0088;
int fuelColor 	    	    = 0xff000088;
int messagesColor   	    = 0xff00ff88;
int oldMessagesColor   	    = 0xff00ff44;
int msgScanBallColor   	    = 0xff000088;
int msgScanSafeColor   	    = 0x00ff0088;
int msgScanCoverColor  	    = 0xaaaa0088;
int msgScanPopColor   	    = 0x0000ff88;

int hudSize 	    	= 250;
static int meterWidth	= 60;
static int meterHeight	= 10;
int meterBorderColor   	    = 0x0000ffff;
int fuelMeterColor   	    = 0x0000ffff;
int powerMeterColor   	    = 0x0000ffff;
int turnSpeedMeterColor     = 0x0000ffff;
int packetSizeMeterColor    = 0x0000ffff;
int packetLossMeterColor    = 0x0000ffff;
int packetDropMeterColor    = 0x0000ffff;
int packetLagMeterColor     = 0x0000ffff;
int temporaryMeterColor     = 0x0000ffff;

int dirPtrColor   	    = 0x0000ffff;
int hudHLineColor   	    = 0x0000ffff;
int hudVLineColor   	    = 0x0000ffff;
int hudItemsColor   	    = 0x0000ffff;
int fuelGaugeColor   	    = 0x0000ffff;
int selfLWColor   	    = 0xff0000ff;
int teamLWColor   	    = 0xff00ffff;
int enemyLWColor   	    = 0xffff00ff;
int team0Color   	    = 0x00000000;
int team1Color   	    = 0x00000000;
int team2Color   	    = 0x00000000;
int team3Color   	    = 0x00000000;
int team4Color   	    = 0x00000000;
int team5Color   	    = 0x00000000;
int team6Color   	    = 0x00000000;
int team7Color   	    = 0x00000000;
int team8Color   	    = 0x00000000;
int team9Color   	    = 0x00000000;


/*static void set_color(int color)
{
    glColor3ub((color >> 16) & 255,
	       (color >> 8) & 255,
	       color & 255);
}*/
/* better to use alpha everywhere, less confusion */
static void set_alphacolor(int color)
{
    glColor4ub((color >> 24) & 255,
    	       (color >> 16) & 255,
	       (color >> 8) & 255,
	       color & 255);
}

int GL_X(int x) {
    return (x - world.x)*scale;
}
int GL_Y(int y) {
    return (y - world.y)*scale;
}

/* remove this later maybe? to tedious for me to edit them all away now */
void Segment_add(int color, int x_1, int y_1, int x_2, int y_2)
{
    set_alphacolor(color);
    glBegin( GL_LINE );
    	glVertex2i(x_1,y_1);
	glVertex2i(x_2,y_2);
    glEnd();
}

void Circle(int color,
	    int x, int y,
	    int radius)
{
    int resolution = 64;
    float i;
    set_alphacolor(color);
    glBegin( GL_LINE_LOOP );
    	/* Silly resolution */
    	for (i = 0.0; i < resolution; i=i+TABLE_SIZE/resolution)
    	    glVertex2i(x + tcos((int)i)*radius,y + tsin((int)i)*radius);
    glEnd();
}

/* Map painting */

void Gui_paint_cannon(int x, int y, int type)
{
    switch (type) {
    case SETUP_CANNON_UP:
        Image_paint(IMG_CANNON_DOWN, x, y, 0);
        break;
    case SETUP_CANNON_DOWN:
        Image_paint(IMG_CANNON_UP, x, y + 1, 0);
        break;
    case SETUP_CANNON_LEFT:
        Image_paint(IMG_CANNON_RIGHT, x, y, 0);
        break;
    case SETUP_CANNON_RIGHT:
        Image_paint(IMG_CANNON_LEFT, x - 1, y, 0);
        break;
    default:
        errno = 0;
        error("Bad cannon dir.");
        return;
    }
}

void Gui_paint_fuel(int x, int y, double fuel)
{
#define FUEL_BORDER 3

    int size, frame;
    irec area;
    image_t *img;

    img = Image_get(IMG_FUEL);
    if (img == NULL) return;

    /* x + x * y will give a pseudo random number,
     * so different fuelcells will not be displayed with the same
     * image-frame. */
    frame = ABS(loopsSlow + x + x * y) % (img->num_frames * 2);

    /* the animation is played from image 0-15 then back again
     * from image 15-0 */
    if (frame >= img->num_frames)
	frame = (2 * img->num_frames - 1) - frame;

    size = (BLOCK_SZ - 2 * FUEL_BORDER) * fuel / MAX_STATION_FUEL;

    Image_paint(IMG_FUELCELL, x, y, 0);

    area.x = 0;
    area.y = 0;
    area.w = BLOCK_SZ - 2 * FUEL_BORDER;
    area.h = size;
    Image_paint_area(IMG_FUEL, 
		     x + FUEL_BORDER, 
		     y + FUEL_BORDER, 
		     frame, 
		     &area);
}

void Gui_paint_base(int x, int y, int id, int team, int type)
{
    switch (type) {
    case SETUP_BASE_UP:
        Image_paint(IMG_BASE_DOWN, x, y, 0);
        break;
    case SETUP_BASE_DOWN:
        Image_paint(IMG_BASE_UP, x, y + 1, 0);
        break;
    case SETUP_BASE_LEFT:
        Image_paint(IMG_BASE_RIGHT, x, y, 0);
        break;
    case SETUP_BASE_RIGHT:
        Image_paint(IMG_BASE_LEFT, x - 1, y, 0);
        break;
    default:
        errno = 0;
        error("Bad base dir.");
        return;
    }    
}

void Gui_paint_decor(int x, int y, int xi, int yi, int type,
		     bool last, bool more_y)
{
}

void Gui_paint_border(int x, int y, int xi, int yi)
{
    set_alphacolor(wallColor);
    glBegin(GL_LINE);
    	glVertex2i(x, y);
    	glVertex2i(xi, yi);
    glEnd();
}

void Gui_paint_visible_border(int x, int y, int xi, int yi)
{
    set_alphacolor(hudColor);
    glBegin(GL_LINE_LOOP);
    	glVertex2i(x, y);
    	glVertex2i(x, yi);
    	glVertex2i(xi, yi);
    	glVertex2i(xi, y);
    glEnd();
}

void Gui_paint_hudradar_limit(int x, int y, int xi, int yi)
{
    set_alphacolor(blue);
    glBegin(GL_LINE_LOOP);
    	glVertex2i(x, y);
    	glVertex2i(x, yi);
    	glVertex2i(xi, yi);
    	glVertex2i(xi, y);
    glEnd();
}

void Gui_paint_setup_check(int x, int y, bool isNext)
{
}

void Gui_paint_setup_acwise_grav(int x, int y)
{
}

void Gui_paint_setup_cwise_grav(int x, int y)
{
}

void Gui_paint_setup_pos_grav(int x, int y)
{
}

void Gui_paint_setup_neg_grav(int x, int y)
{
}

void Gui_paint_setup_up_grav(int x, int y)
{
}

void Gui_paint_setup_down_grav(int x, int y)
{
}

void Gui_paint_setup_right_grav(int x, int y)
{
}

void Gui_paint_setup_left_grav(int x, int y)
{
}

void Gui_paint_setup_worm(int x, int y)
{
}

void Gui_paint_setup_item_concentrator(int x, int y)
{
}

void Gui_paint_setup_asteroid_concentrator(int x, int y)
{
}

void Gui_paint_decor_dot(int x, int y, int size)
{
}

void Gui_paint_setup_target(int x, int y, int team, double damage, bool own)
{
}

void Gui_paint_setup_treasure(int x, int y, int team, bool own)
{
    Image_paint(own ? IMG_HOLDER_FRIEND : IMG_HOLDER_ENEMY, x, y, 0);
}

void Gui_paint_walls(int x, int y, int type)
{
    set_alphacolor(wallColor);
    glBegin(GL_LINES);


    if (type & BLUE_LEFT) {
	glVertex2i(x, y);
	glVertex2i(x, y + BLOCK_SZ);
    }
    if (type & BLUE_DOWN) {
	glVertex2i(x, y);
	glVertex2i(x + BLOCK_SZ, y);
    }
    if (type & BLUE_RIGHT) {
	glVertex2i(x + BLOCK_SZ, y);
	glVertex2i(x + BLOCK_SZ, y + BLOCK_SZ);
    }
    if (type & BLUE_UP) {
	glVertex2i(x, y + BLOCK_SZ);
	glVertex2i(x + BLOCK_SZ, y + BLOCK_SZ);
    }
    if ((type & BLUE_FUEL) == BLUE_FUEL) {
    } else if (type & BLUE_OPEN) {
	glVertex2i(x, y);
	glVertex2i(x + BLOCK_SZ, y + BLOCK_SZ);
    } else if (type & BLUE_CLOSED) {
	glVertex2i(x, y + BLOCK_SZ);
	glVertex2i(x + BLOCK_SZ, y);
    }
    glEnd();
}

void Gui_paint_filled_slice(int bl, int tl, int tr, int br, int y)
{
    set_alphacolor(wallColor);
    glBegin(GL_QUADS);
    glVertex2i(bl, y);
    glVertex2i(tl, y + BLOCK_SZ);
    glVertex2i(tr, y + BLOCK_SZ);
    glVertex2i(br, y);
    glEnd();
}

void Gui_paint_polygon(int i, int xoff, int yoff)
{
    xp_polygon_t    polygon;
    polygon_style_t p_style;
    edge_style_t    e_style;
    
    int             x, y, j;

    polygon = polygons[i];
    p_style = polygon_styles[polygon.style];
    e_style = edge_styles[p_style.def_edge_style];

    if (BIT(p_style.flags, STYLE_INVISIBLE)) return;
    
    set_alphacolor((e_style.rgb << 8) | 255);

    /* maybe this should be done when changing scale?
     * not sure that would help when using GL though.. (mara)
     */
    x = xoff * Setup->width;
    y = yoff * Setup->height;

    glBegin(GL_LINE_LOOP);
    
    for (j = 0; j < polygon.num_points; j++) {
        x += polygon.points[j].x;
        y += polygon.points[j].y;
	glVertex2i(x,y);
    }

    glEnd();
}


/* Object painting */


void Gui_paint_item_object(int type, int x, int y)
{
}

void Gui_paint_ball(int x, int y)
{
    Image_paint(IMG_BALL, x - BALL_RADIUS, y - BALL_RADIUS, 0);
}

void Gui_paint_ball_connector(int x_1, int y_1, int x_2, int y_2)
{
    set_alphacolor(connColor);
    glBegin(GL_LINES);
    glVertex2i(x_1, y_1);
    glVertex2i(x_2, y_2);
    glEnd();
}

void Gui_paint_mine(int x, int y, int teammine, char *name)
{
}

void Gui_paint_spark(int color, int x, int y)
{
    /*
    Image_paint(IMG_SPARKS, 
		x + world.x, 
		world.y + ext_view_height - y, 
		color);
    */
    glColor3ub(255 * (color + 1) / 8, 
	       255 * color * color / 64,
	       0);
    glBegin(GL_POINTS);
    glVertex2i(x + world.x, world.y + ext_view_height - y);
    glEnd();
}

void Gui_paint_wreck(int x, int y, bool deadly, int wtype, int rot, int size)
{
}

void Gui_paint_asteroid(int x, int y, int type, int rot, int size)
{
}


/*
 * It seems that currently the screen coordinates are calculated already
 * in paintobjects.c. This should be changed.
 */
void Gui_paint_fastshot(int color, int x, int y)
{
    /* not sure why i need that 7 to make it right */
    /* that 2 seems to be the size/2 of the bullet */
    Image_paint(IMG_BULLET, 
		x + world.x - 1, 
		world.y - 7 + ext_view_height - y, 
		3);
}

void Gui_paint_teamshot(int x, int y)
{
    Image_paint(IMG_BULLET_OWN, 
		x + world.x - 1, 
		world.y - 7 + ext_view_height - y, 
		3);
}

void Gui_paint_missiles_begin(void)
{
}

void Gui_paint_missiles_end(void)
{
}

void Gui_paint_missile(int x, int y, int len, int dir)
{
}

void Gui_paint_lasers_begin(void)
{
}

void Gui_paint_lasers_end(void)
{
}

void Gui_paint_laser(int color, int x_1, int y_1, int len, int dir)
{
}

void Gui_paint_paused(int x, int y, int count)
{
    Image_paint(IMG_PAUSED, 
		x - BLOCK_SZ / 2,
		y - BLOCK_SZ / 2,
		(count <= 0 || loopsSlow % 10 >= 5) ? 1 : 0);
}

void Gui_paint_appearing(int x, int y, int id, int count)
{
}

void Gui_paint_ecm(int x, int y, int size)
{
}

void Gui_paint_refuel(int x_0, int y_0, int x_1, int y_1)
{
    /*if (texturedObjects) return;*/
    set_alphacolor(fuelColor);
    glLineStipple(3, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glVertex2i(x_0, y_0);
    glVertex2i(x_1, y_1);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

void Gui_paint_connector(int x_0, int y_0, int x_1, int y_1, int tractor)
{
    set_alphacolor(connColor);
    glLineStipple(tractor ? 2 : 4, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glVertex2i(x_0, y_0);
    glVertex2i(x_1, y_1);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

void Gui_paint_transporter(int x_0, int y_0, int x_1, int y_1)
{
}

void Gui_paint_all_connectors_begin(void)
{
}

void Gui_paint_ships_begin(void)
{
}

void Gui_paint_ships_end(void)
{
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

static int Gui_calculate_ship_color(int id, other_t *other)
{
    int ship_color = white;

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
	    ship_color = blue;
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
	    ship_color = blue;
    }

    if (Gui_is_my_tank(other))
	ship_color = blue;
#endif
    if (roundDelay > 0 && ship_color == white)
	ship_color = red;

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

void Gui_paint_ship(int x, int y, int dir, int id, int cloak, int phased,
		    int shield, int deflector, int eshield)
{
    int i;
    shipshape_t *ship;
    shapepos    point;
    other_t 	*other;

    ship = Ship_by_id(id);
    other = Other_by_id(id);
    
    set_alphacolor(Gui_calculate_ship_color(id,other));
    
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < ship->num_points; i++) {
	point = Ship_get_point(ship, i, dir);
	glVertex2i(x + point.pxl.x, y + point.pxl.y);
    }
    glEnd();
}

static int wrap(int *xp, int *yp)
{
    int			x = *xp, y = *yp;
    int returnval =1;

    if (x < world.x || x > world.x + ext_view_width) {
	if (x < realWorld.x || x > realWorld.x + ext_view_width)
	    returnval = 0;
	*xp += world.x - realWorld.x;
    }
    if (y < world.y || y > world.y + ext_view_height) {
	if (y < realWorld.y || y > realWorld.y + ext_view_height)
	    returnval = 0;
	*yp += world.y - realWorld.y;
    }
    return returnval;
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
	    	/* approximate font width to h =( */
		x = sobj->x * BLOCK_SZ + BLOCK_SZ/2;
		y = sobj->y * BLOCK_SZ + BLOCK_SZ/2;
  		if (wrap(&x, &y)) {
		    x = (x - world.x)*scale;
		    y = (y - world.y)*scale;
    	    	    set_alphacolor(scoreObjectColor);
		    fontprint(&gamefont,CENTER,CENTER,x,y,sobj->msg);
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

void Paint_client_fps(void)
{
    int			x, y;
    char		buf[32];
    int			len;
    fontbounds dummy;

    if (!hudColor)
	return;

    set_alphacolor(hudColor);
    sprintf(buf, "FPS: %d", clientFPS);
    len = strlen(buf);
    x = draw_width - 20;
    y = draw_height - 200;
    dummy = fontprint(&gamefont,RIGHT,DOWN,x,y,buf);
}

static void Paint_meter(int xoff, int y, const char *title, int val, int max,
			int meter_color)
{
    const int	mw1_4 = meterWidth/4,
		mw2_4 = meterWidth/2,
		mw3_4 = 3*meterWidth/4,
		mw4_4 = meterWidth,
		BORDER = 5;
    int		x, xstr;
    int x_alignment;

    if (xoff >= 0) {
	x = xoff;
        xstr = x + meterWidth + BORDER;
	x_alignment = LEFT;
    } else {
	x = draw_width - (meterWidth - xoff);
        xstr = x - BORDER;
	x_alignment = RIGHT;
    }

    set_alphacolor(meter_color);
    glBegin( GL_POLYGON );
    	glVertex2i(x+2,y+2);
    	glVertex2i(x+2,y+2+meterHeight-3);
    	glVertex2i(x+2+(int)(((meterWidth-3)*val)/(max?max:1)),y+2+meterHeight-3);
    	glVertex2i(x+2+(int)(((meterWidth-3)*val)/(max?max:1)),y+2);
    glEnd();
    
    

    /* meterBorderColor = 0 obviously means no meter borders are drawn */
    if (meterBorderColor) {
	int color = meterBorderColor;

	set_alphacolor(color);
	glBegin( GL_LINE_LOOP );
	    glVertex2i(x,y);
	    glVertex2i(x,y + meterHeight);
	    glVertex2i(x + meterWidth,y + meterHeight);
	    glVertex2i(x + meterWidth,y);
	glEnd();

	glBegin( GL_LINES );
	    glVertex2i(x,       y-4);glVertex2i(x,       y+meterHeight+4);
	    glVertex2i(x+mw4_4, y-4);glVertex2i(x+mw4_4, y+meterHeight+4);
	    glVertex2i(x+mw2_4, y-3);glVertex2i(x+mw2_4, y+meterHeight+3);
	    glVertex2i(x+mw1_4, y-1);glVertex2i(x+mw1_4, y+meterHeight+1);
	    glVertex2i(x+mw3_4, y-1);glVertex2i(x+mw3_4, y+meterHeight+1);
	glEnd();
    }

    if (!meterBorderColor)
	set_alphacolor(meter_color);

    fontprint(&gamefont,x_alignment,CENTER,xstr,draw_height - y - meterHeight/2,title);
}

void Paint_meters(void)
{
    int y = 20, color;

    if (fuelMeterColor)
	Paint_meter(-10, y += 20, "Fuel",
		    (int)fuelSum, (int)fuelMax, fuelMeterColor);

    if (powerMeterColor)
	color = powerMeterColor;
    else if (controlTime > 0.0)
	color = temporaryMeterColor;
    else
	color = 0;

    if (color)
	Paint_meter(-10, y += 20, "Power",
		    (int)displayedPower, (int)MAX_PLAYER_POWER, color);

    if (turnSpeedMeterColor)
	color = turnSpeedMeterColor;
    else if (controlTime > 0.0)
	color = temporaryMeterColor;
    else
	color = 0;

    if (color)
	Paint_meter(-10, y += 20, "Turnspeed",
		    (int)displayedTurnspeed, (int)MAX_PLAYER_TURNSPEED, color);

    if (controlTime > 0.0) {
	controlTime -= timePerFrame;
	if (controlTime <= 0.0)
	    controlTime = 0.0;
    }

    if (packetSizeMeterColor)
	Paint_meter(-10, y += 20, "Packet",
		   (packet_size >= 4096) ? 4096 : packet_size, 4096,
		    packetSizeMeterColor);
    if (packetLossMeterColor)
	Paint_meter(-10, y += 20, "Loss", packet_loss, FPS,
		    packetLossMeterColor);
    if (packetDropMeterColor)
	Paint_meter(-10, y += 20, "Drop", packet_drop, FPS,
		    packetDropMeterColor);
    if (packetLagMeterColor)
	Paint_meter(-10, y += 20, "Lag", MIN(packet_lag, 1 * FPS), 1 * FPS,
		    packetLagMeterColor);

    if (temporaryMeterColor) {
	if (thrusttime >= 0 && thrusttimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3,
			"Thrust Left",
			(thrusttime >= thrusttimemax
			 ? thrusttimemax : thrusttime),
			thrusttimemax, temporaryMeterColor);

	if (shieldtime >= 0 && shieldtimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 20,
			"Shields Left",
			(shieldtime >= shieldtimemax
			 ? shieldtimemax : shieldtime),
			shieldtimemax, temporaryMeterColor);

	if (phasingtime >= 0 && phasingtimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 40,
			"Phasing left",
			(phasingtime >= phasingtimemax
			 ? phasingtimemax : phasingtime),
			phasingtimemax, temporaryMeterColor);

	if (destruct > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 60,
			"Self destructing", destruct, 150,
			temporaryMeterColor);

	if (shutdown_count >= 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 80,
			"SHUTDOWN", shutdown_count, shutdown_delay,
			temporaryMeterColor);
    }
}

static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
}

static void Paint_hudradar(double hrscale, double xlimit, double ylimit, int sz)
{
}

static void Paint_HUD_items(int hud_pos_x, int hud_pos_y)
{
}

void Paint_HUD(void)
{
    const int		BORDER = 3;
    char		str[50];
    int			hud_pos_x, hud_pos_y, size;
    int			did_fuel = 0;
    int			i, j, modlen = 0;
    static char		autopilot[] = "Autopilot";
    int tempx,tempy,tempw,temph;
    fontbounds dummy;

    /*
     * Show speed pointer
     */
    if (ptr_move_fact != 0.0
	&& selfVisible
	&& (FOOvel.x != 0 || FOOvel.y != 0))
	Segment_add(hudColor,
		    ext_view_width / 2,
		    ext_view_height / 2,
		    (int)(ext_view_width / 2 - ptr_move_fact * FOOvel.x),
		    (int)(ext_view_height / 2 + ptr_move_fact * FOOvel.y));

    if (selfVisible && dirPtrColor) {
	Segment_add(dirPtrColor,
		    (int) (draw_width / 2 +
			   (100 - 15) * tcos(heading)),
		    (int) (draw_height / 2 -
			   (100 - 15) * tsin(heading)),
		    (int) (draw_width / 2 + 100 * tcos(heading)),
		    (int) (draw_height / 2 - 100 * tsin(heading)));
    }
    
    /* TODO */
    /* This should be done in a nicer way now (using radar.c maybe) */
    /*
    if (hudRadarEnemyColor || hudRadarOtherColor) {
	hudRadarMapScale = (double) Setup->width / (double) 256;
	Paint_hudradar(
	    hudRadarScale,
	    (int)(hudRadarLimit * (active_view_width / 2)
		  * hudRadarScale / hudRadarMapScale),
	    (int)(hudRadarLimit * (active_view_width / 2)
		  * hudRadarScale / hudRadarMapScale),
	    hudRadarDotSize);

	if (BIT(instruments, MAP_RADAR))
	    Paint_hudradar(hudRadarMapScale,
			   active_view_width / 2,
			   active_view_height / 2,
			   SHIP_SZ);
    }
    */

    /* message scan hack by mara*/
    if (BIT(instruments, BALL_MSG_SCAN)) {
	if (ball_shout && msgScanBallColor)
	    Circle(msgScanBallColor, draw_width / 2,
		    draw_height / 2, 8*scale);
	if (need_cover && msgScanCoverColor)
	    Circle(msgScanCoverColor, draw_width / 2,
		    draw_height / 2, 6*scale);
    }

    /*
     * Display the HUD
     */
    hud_pos_x = (int)(draw_width / 2 - hud_move_fact * FOOvel.x);
    hud_pos_y = (int)(draw_height / 2 + hud_move_fact * FOOvel.y);

    /* HUD frame */
    glLineStipple(4, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    if (hudHLineColor) {
    	set_alphacolor(hudHLineColor);
    	glBegin(GL_LINES);
    	    glVertex2i(hud_pos_x - hudSize,hud_pos_y - hudSize + HUD_OFFSET);
	    glVertex2i(hud_pos_x + hudSize,hud_pos_y - hudSize + HUD_OFFSET);
    	    
	    glVertex2i(hud_pos_x - hudSize,hud_pos_y + hudSize - HUD_OFFSET);
	    glVertex2i(hud_pos_x + hudSize,hud_pos_y + hudSize - HUD_OFFSET);
    	glEnd();
    }
    if (hudVLineColor) {
    	set_alphacolor(hudVLineColor);
    	glBegin(GL_LINES);
    	    glVertex2i(hud_pos_x - hudSize + HUD_OFFSET,hud_pos_y - hudSize);
	    glVertex2i(hud_pos_x - hudSize + HUD_OFFSET,hud_pos_y + hudSize);
    	    
	    glVertex2i(hud_pos_x + hudSize - HUD_OFFSET,hud_pos_y - hudSize);
	    glVertex2i(hud_pos_x + hudSize - HUD_OFFSET,hud_pos_y + hudSize);
    	glEnd();
    }
    glDisable(GL_LINE_STIPPLE);

    if (hudItemsColor)
	Paint_HUD_items(hud_pos_x, hud_pos_y);

    /* Fuel notify, HUD meter on */
    if (hudColor && (fuelTime > 0.0 || fuelSum < fuelLevel3)) {
	set_alphacolor(hudColor);
	did_fuel = 1;
	sprintf(str, "%04d", (int)fuelSum);
	/* TODO fix this */
	fontprint(&gamefont,LEFT,DOWN,
	    	hud_pos_x + hudSize-HUD_OFFSET+BORDER,
		hud_pos_y - (hudSize-HUD_OFFSET+BORDER),
		str);
	if (numItems[ITEM_TANK]) {
	    if (fuelCurrent == 0)
		strcpy(str,"M ");
	    else
		sprintf(str, "T%d", fuelCurrent);
	    /* TODO fix this */
	    fontprint(&gamefont,LEFT,DOWN,
	    	    hud_pos_x + hudSize-HUD_OFFSET + BORDER,
		    hud_pos_y - hudSize-HUD_OFFSET + BORDER,
		    str);
	}
    }

    /* Update the lock display */
    Paint_lock(hud_pos_x, hud_pos_y);

    /* Draw last score on hud if it is an message attached to it */
    if (hudColor) {
	set_alphacolor(hudColor);
	
    	/* TODO: rewrite this to exploit the fact fontprint handles newlines */
	for (i = 0, j = 0; i < MAX_SCORE_OBJECTS; i++) {
	    score_object_t*	sobj = &score_objects[(i+score_object)%MAX_SCORE_OBJECTS];
	    if (sobj->hud_msg_len > 0) {
	    	dummy = fontprintsize(&gamefont,sobj->hud_msg);
		if (sobj->hud_msg_width == -1)
		    sobj->hud_msg_width = dummy.width;
		if (j == 0 &&
		    sobj->hud_msg_width > 2*hudSize-HUD_OFFSET*2 &&
		    (did_fuel || hudVLineColor))
		    ++j;
		fontprint(&gamefont,CENTER,DOWN,
		    	hud_pos_x,
			hud_pos_y - (hudSize-HUD_OFFSET + BORDER + j * dummy.height),
			sobj->hud_msg);
		j++;
	    }
	}

	if (time_left > 0) {
	    sprintf(str, "%3d:%02d",
		    (int)(time_left / 60), (int)(time_left % 60));
	    fontprint(&gamefont,RIGHT,DOWN,
		    hud_pos_x - hudSize+HUD_OFFSET - BORDER,
		    hud_pos_y + hudSize+HUD_OFFSET + BORDER,
		    str);
	}

	/* Update the modifiers */
	modlen = strlen(mods);
	fontprint(&gamefont,RIGHT,UP,
		hud_pos_x - hudSize+HUD_OFFSET-BORDER,
		hud_pos_y - hudSize+HUD_OFFSET-BORDER,
		mods);

	if (autopilotLight) {
	    dummy = fontprintsize(&gamefont,autopilot);
	    fontprint(&gamefont,CENTER,DOWN,
			  hud_pos_x,
			  hud_pos_y + hudSize+HUD_OFFSET + BORDER
			  + dummy.height*2,
			  autopilot);
	}
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

	set_alphacolor(fuelGaugeColor);
	tempx = hud_pos_x + hudSize - HUD_OFFSET + FUEL_GAUGE_OFFSET - 1;
	tempy = hud_pos_y - hudSize + HUD_OFFSET + FUEL_GAUGE_OFFSET - 1;
	tempw = HUD_OFFSET - (2*FUEL_GAUGE_OFFSET) + 3;
	temph = HUD_FUEL_GAUGE_SIZE + 3;
	glBegin(GL_LINE_LOOP);
	    glVertex2i(tempx,tempy);
	    glVertex2i(tempx,tempy+temph);
	    glVertex2i(tempx+tempw,tempy+temph);
	    glVertex2i(tempx+tempw,tempy);
	glEnd();

	size = (HUD_FUEL_GAUGE_SIZE * fuelSum) / fuelMax;
	tempx = hud_pos_x + hudSize - HUD_OFFSET + FUEL_GAUGE_OFFSET + 1;
    	tempy = hud_pos_y - hudSize + HUD_OFFSET + FUEL_GAUGE_OFFSET + HUD_FUEL_GAUGE_SIZE - size + 1;
    	tempw = HUD_OFFSET - (2*FUEL_GAUGE_OFFSET);
    	temph = size;
	glBegin(GL_POLYGON);
	    glVertex2i(tempx,tempy);
	    glVertex2i(tempx,tempy+temph);
	    glVertex2i(tempx+tempw,tempy+temph);
	    glVertex2i(tempx+tempw,tempy);
	glEnd();
    }
}

void Paint_messages(void)
{
    int		i, x, y, top_y, bot_y, width, len;
    const int	BORDER = 10,
		SPACING = messagefont.h*1.3+1;
    const int	BORDERx_bot = BORDER, BORDERx_top = 200 + BORDER;
    message_t	*msg;
    int		last_msg_index = 0, msg_color;

    top_y = draw_height - messagefont.h - BORDER;
    bot_y = BORDER;

    /* get number of player messages */
    if (selectionAndHistory) {
	while (last_msg_index < maxMessages
		&& TalkMsg[last_msg_index]->len != 0)
	    last_msg_index++;
	last_msg_index--; /* make it an index */
    }

    for (i = (BIT(instruments, SHOW_REVERSE_SCROLL) ? 2 * maxMessages - 1 : 0);
	 (BIT(instruments, SHOW_REVERSE_SCROLL)
	  ? i >= 0 : i < 2 * maxMessages);
	 i += (BIT(instruments, SHOW_REVERSE_SCROLL) ? -1 : 1)) {
	if (i < maxMessages)
	    msg = TalkMsg[i];
	else
	    msg = GameMsg[i - maxMessages];
	if (msg->len == 0)
	    continue;

	/*
	 * While there is something emphasized, freeze the life time counter
	 * of a message if it is not drawn "flashed" (not in oldMessagesColor)
	 * anymore.
	 */
	if (msg->lifeTime > MSG_FLASH_TIME
	    || !selectionAndHistory
	    || (selection.draw.state != SEL_PENDING
		&& selection.draw.state != SEL_EMPHASIZED)) {
	    if ((msg->lifeTime -= timePerFrame) <= 0.0) {
		msg->txt[0] = '\0';
		msg->len = 0;
		msg->lifeTime = 0.0;
		continue;
	    }
	}

	if (msg->lifeTime <= MSG_FLASH_TIME)
	    msg_color = oldMessagesColor;
	else {
	    /* If paused, don't bother to paint messages in mscScan* colors. */
	    if (self && strchr("P", self->mychar))
		msg_color = messagesColor;
	    else {
		switch (msg->bmsinfo) {
		case BmsBall:	msg_color = msgScanBallColor;	break;
		case BmsSafe:	msg_color = msgScanSafeColor;	break;
		case BmsCover:	msg_color = msgScanCoverColor;	break;
		case BmsPop:	msg_color = msgScanPopColor;	break;
		default:	msg_color = messagesColor;	break;
		}
	    }
	}

	if (msg_color == 0)
	    continue;

	if (i < maxMessages) {
	    x = BORDERx_top;
	    y = top_y;
	    top_y -= SPACING;
	} else {
	    if (!BIT(instruments, SHOW_MESSAGES))
		continue;
	    x = BORDERx_bot;
	    y = bot_y;
	    bot_y += SPACING;
	}
	len = (int)(charsPerSecond * (MSG_LIFE_TIME - msg->lifeTime));
	len = MIN(msg->len, len);

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
	    } else if (TALK_MSG_SCREENPOS(last_msg_index,i)
		       == selection.draw.y1) {
		    /* first/only line */
		    /*___xxx[___]*/
		ptr = msg->txt;
		xoff = 0;
		if ( len < selection.draw.x1)
		    l = len;
		else {
			/* at least two parts */
			/*___xxx[___]*/
			/*    ^      */
		    l = selection.draw.x1;
		    ptr2 = &(msg->txt[selection.draw.x1]);
		    /*xoff2 = XTextWidth(messageFont, msg->txt,
				       selection.draw.x1);*/
		    xoff2 = messagefont.w*len;/*this is not accurate*/

		    if (TALK_MSG_SCREENPOS(last_msg_index,i)
			< selection.draw.y2) {
			    /* first line */
			    /*___xxxxxx*/
			    /*     ^   */
			l2 = len - selection.draw.x1;
		    } else {
			    /* only line */
			    /*___xxx___*/
			if (len <= selection.draw.x2)
				/*___xxx___*/
				/*    ^    */
			    l2 = len - selection.draw.x1;
			else {
				/*___xxx___*/
				/*       ^ */
			    l2 = selection.draw.x2 - selection.draw.x1 + 1;
			    ptr3 = &(msg->txt[selection.draw.x2 + 1]);
			    /*xoff3 = XTextWidth(messageFont, msg->txt,
					       selection.draw.x2 + 1);*/
			    xoff3 = messagefont.w*len;/*this is not accurate*/
			    l3 = len - selection.draw.x2 - 1;
			}
		    } /* only line */
		} /* at least two parts */
	    } else {
		    /* last line */
		    /*xxxxxx[___]*/
		ptr2 = msg->txt;
		xoff2 = 0;
		if (len <= selection.draw.x2 + 1)
			/* all blue */
			/*xxxxxx[___]*/
			/*  ^        */
		    l2 = len;
		else {
			/*xxxxxx___*/
			/*       ^ */
		    l2 = selection.draw.x2 + 1;
		    ptr3 = &(msg->txt[selection.draw.x2 + 1]);
		    /*xoff3 = XTextWidth(messageFont, msg->txt,
				       selection.draw.x2 + 1);*/
		    xoff3 = messagefont.w*len;/*this is not accurate*/
		    l3 = len - selection.draw.x2 - 1;
		}
	    } /* last line */


	    if (ptr) {
		set_alphacolor(msg_color);
		fontprint(&gamefont,LEFT,CENTER,x,y,ptr);
	    }
	    if (ptr2) {
		set_alphacolor(white);
		fontprint(&gamefont,LEFT,CENTER,x,y,ptr2);
	    }
	    if (ptr3) {
		set_alphacolor(msg_color);
		fontprint(&gamefont,LEFT,CENTER,x,y,ptr2);
	    }

	} else /* not emphasized */

	{
	    set_alphacolor(msg_color);
	    fontprint(&gamefont,LEFT,CENTER,x,y,msg->txt);
	}

	width = messagefont.w*MIN(len, msg->len); /*this is not accurate*/
    }
}


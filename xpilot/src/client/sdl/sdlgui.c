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

int wallColor = 0xff;
extern unsigned long	loopsSlow;	        /* Proceeds slower than loops */
extern ipos world;
extern int active_view_height;

static void set_color(int color)
{
    glColor3ub((color >> 16) & 255,
	       (color >> 8) & 255,
	       color & 255);
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
}

void Gui_paint_visible_border(int x, int y, int xi, int yi)
{
}

void Gui_paint_hudradar_limit(int x, int y, int xi, int yi)
{
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
    set_color(wallColor);
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
    set_color(wallColor);
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
    polygon_style_t style;
    int             x, y, j;

    polygon = polygons[i];
    style = polygon_styles[polygon.style];

    if (BIT(style.flags, STYLE_INVISIBLE)) return;
    set_color(style.rgb);

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
    set_color(0xffffff);
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
    Image_paint(IMG_BULLET, 
		x + world.x, 
		active_view_height + world.y - y - 5, 
		3);
}

void Gui_paint_teamshot(int x, int y)
{
    Image_paint(IMG_BULLET_OWN, 
		x + world.x, 
		active_view_height + world.y - y, 
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
}

void Gui_paint_connector(int x_0, int y_0, int x_1, int y_1, int tractor)
{
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

void Gui_paint_ship(int x, int y, int dir, int id, int cloak, int phased,
		    int shield, int deflector, int eshield)
{
    int i;
    shipshape_t *ship;
    shapepos    point;

    ship = Ship_by_id(id);
    set_color(0xffffff);
    glBegin(GL_LINE_LOOP);
    for (i = 0; i < ship->num_points; i++) {
	point = Ship_get_point(ship, i, dir);
	glVertex2i(x + point.pxl.x, y + point.pxl.y);
    }
    glEnd();
}

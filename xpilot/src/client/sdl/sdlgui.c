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

int nullRGBA    = 0x00000000;
int blackRGBA   = 0x000000ff;
int whiteRGBA   = 0xffffffff;
int blueRGBA    = 0x0000ffff;
int redRGBA     = 0xff0000ff;
int greenRGBA   = 0x00ff00ff;

int wallColorRGBA   	    	= 0x0000ffff;
int hudColorRGBA    	    	= 0xff000088;
int connColorRGBA   	    	= 0x00ff0088;
int scoreObjectColorRGBA    	= 0x00ff0088;
int fuelColorRGBA   	    	= 0xffffff44;
int messagesColorRGBA	    	= 0x00aaaa88;
int oldmessagesColorRGBA    	= 0x00888888;
int msgScanBallColorRGBA    	= 0xff000088;
int msgScanSafeColorRGBA    	= 0x00ff0088;
int msgScanCoverColorRGBA   	= 0x4e7cff88;
int msgScanPopColorRGBA     	= 0xffbb1188;

int meterBorderColorRGBA    	= 0x0000ff55;
int fuelMeterColorRGBA	    	= 0xff000055;
int powerMeterColorRGBA     	= 0xff000055;
int turnSpeedMeterColorRGBA     = 0xff000055;
int packetSizeMeterColorRGBA    = 0xff000055;
int packetLossMeterColorRGBA    = 0xff000055;
int packetDropMeterColorRGBA    = 0xff000055;
int packetLagMeterColorRGBA     = 0xff000055;
int temporaryMeterColorRGBA     = 0xff000055;

int dirPtrColorRGBA 	    	= 0x0000ff22;
int hudHLineColorRGBA	    	= 0x0000ff44;
int hudVLineColorRGBA	    	= 0x0000ff44;
int hudItemsColorRGBA	    	= 0x0000ff44;
int fuelGaugeColorRGBA	    	= 0x0000ff44;
int selfLWColorRGBA   	    	= 0xff0000ff;
int teamLWColorRGBA   	    	= 0xff00ffff;
int enemyLWColorRGBA   	    	= 0xffff00ff;
int team0ColorRGBA   	    	= 0x00000000;
int team1ColorRGBA  	    	= 0x00000000;
int team2ColorRGBA   	    	= 0x00000000;
int team3ColorRGBA   	    	= 0x00000000;
int team4ColorRGBA   	    	= 0x00000000;
int team5ColorRGBA   	    	= 0x00000000;
int team6ColorRGBA  	    	= 0x00000000;
int team7ColorRGBA  	    	= 0x00000000;
int team8ColorRGBA   	    	= 0x00000000;
int team9ColorRGBA   	    	= 0x00000000;
int shipNameColorRGBA  	    	= 0x0000ff88;
int baseNameColorRGBA  	    	= 0x0000ff88;
int manyLivesColorRGBA	    	= 0x666666aa;
int twoLivesColorRGBA  	    	= 0x008800aa;
int oneLifeColorRGBA  	    	= 0xaaaa00aa;
int zeroLivesColorRGBA	    	= 0xff0000aa;

int hudRadarEnemyColorRGBA  	= 0xff000088;
int hudRadarOtherColorRGBA  	= 0x0000ff88;

int scoreInactiveSelfColorRGBA	= 0x88008888;
int scoreInactiveColorRGBA	= 0x0000aa88;
int scoreSelfColorRGBA	    	= 0xffff00ff;
int scoreColorRGBA	    	= 0x888800ff;
int scoreOwnTeamColorRGBA	= 0x0000ffff;
int scoreEnemyTeamColorRGBA	= 0xff0000ff;

int selectionColorRGBA	    	= 0xff0000ff;

int hudSize 	    	= 250;
static int meterWidth	= 60;
static int meterHeight	= 10;
double hudRadarScale = 2.0;
float hudRadarMapScale;
int hudRadarDotSize = 6;
int baseWarningType = 1;

static GLuint polyListBase = 0;
static GLuint polyEdgeListBase = 0;

int Gui_init(void);
void Gui_cleanup(void);

/* better to use alpha everywhere, less confusion */
void set_alphacolor(int color)
{
    glColor4ub((color >> 24) & 255,
    	       (color >> 16) & 255,
	       (color >> 8) & 255,
	       color & 255);
}

static GLubyte get_alpha(int color)
{
    return (color & 255);
}

int GL_X(int x) {
    return (int)((x - world.x)*scale);
}
int GL_Y(int y) {
    return (int)((y - world.y)*scale);
}

/* remove this later maybe? to tedious for me to edit them all away now */
void Segment_add(int color, int x_1, int y_1, int x_2, int y_2)
{
    set_alphacolor(color);
    glBegin( GL_LINE_LOOP );
    	glVertex2i(x_1,y_1);
	glVertex2i(x_2,y_2);
    glEnd();
}

void Circle(int color,
	    int x, int y,
	    int radius, int filled)
{
    float i,resolution = 16;
    set_alphacolor(color);
    if (filled)
    	glBegin( GL_POLYGON );
    else
    	glBegin( GL_LINE_LOOP );
    	/* Silly resolution */
    	for (i = 0.0f; i < TABLE_SIZE; i=i+((float)TABLE_SIZE)/resolution)
    	    glVertex2f((x + tcos((int)i)*radius),(y + tsin((int)i)*radius));
    glEnd();
}

static void vertex_callback(ipos *p, image_t *texture)
{
    if (texture != NULL) {
	glTexCoord2f(p->x / (GLfloat)texture->frame_width,
		     p->y / (GLfloat)texture->height);
    }
    glVertex2i(p->x, p->y);
}

static void tessellate_polygon(GLUtriangulatorObj *tess, int i)
{
    int j;
    int x,y;
    
    xp_polygon_t polygon;
    polygon_style_t p_style;
    image_t *texture = NULL;
    GLdouble v[3] = { 0, 0, 0 };
    ipos p[MAX_VERTICES];

    polygon = polygons[i];
    p_style = polygon_styles[polygon.style];
    p[0].x = p[0].y = 0;

    if (BIT(p_style.flags, STYLE_TEXTURED))
	texture = Image_get_texture(p_style.texture);

    glNewList(polyListBase + i,  GL_COMPILE);
    gluTessBeginPolygon(tess, texture);
    gluTessVertex(tess, v, &p[0]);
    for (j = 1; j < polygon.num_points; j++) {
	v[0] = p[j].x = p[j-1].x + polygon.points[j].x;
	v[1] = p[j].y = p[j-1].y + polygon.points[j].y;
	gluTessVertex(tess, v, &p[j]);
    }
    gluTessEndPolygon(tess);
    glEndList();
    glNewList(polyEdgeListBase + i,  GL_COMPILE);
    glBegin(GL_LINE_LOOP);
    x = y = 0;
    glVertex2i(x, y);
    for (j = 1; j < polygon.num_points; j++) {
	x += polygon.points[j].x;
	y += polygon.points[j].y;
	glVertex2i(x, y);
    }
    glEnd();
    glEndList();
}

int Gui_init(void)
{
    int i;
    GLUtriangulatorObj *tess;

    if (num_polygons == 0) return 0;

    polyListBase = glGenLists(num_polygons);
    polyEdgeListBase = glGenLists(num_polygons);
    if ((!polyListBase)||(!polyEdgeListBase)) {
	error("failed to generate display lists");
	return -1;
    }

    tess = gluNewTess();
    if (tess == NULL) {
	error("failed to create tessellation object");
	return -1;
    }

    gluTessCallback(tess, GLU_TESS_BEGIN, (_GLUfuncptr)glBegin);
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (_GLUfuncptr)vertex_callback);
    gluTessCallback(tess, GLU_TESS_END, (_GLUfuncptr)glEnd);

    for (i = 0; i < num_polygons; i++) {
	tessellate_polygon(tess, i);
    }

    gluDeleteTess(tess);
    return 0;
}

void Gui_cleanup(void)
{
    if (polyListBase)
	glDeleteLists(polyListBase, num_polygons);
}

/* Map painting */

void Gui_paint_cannon(int x, int y, int type)
{
    switch (type) {
    case SETUP_CANNON_UP:
        Image_paint(IMG_CANNON_DOWN, x, y, 0, whiteRGBA);
        break;
    case SETUP_CANNON_DOWN:
        Image_paint(IMG_CANNON_UP, x, y + 1, 0, whiteRGBA);
        break;
    case SETUP_CANNON_LEFT:
        Image_paint(IMG_CANNON_RIGHT, x, y, 0, whiteRGBA);
        break;
    case SETUP_CANNON_RIGHT:
        Image_paint(IMG_CANNON_LEFT, x - 1, y, 0, whiteRGBA);
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

    Image_paint(IMG_FUELCELL, x, y, 0, fuelColorRGBA);

    area.x = 0;
    area.y = (BLOCK_SZ - 2 * FUEL_BORDER) * (1 - fuel / MAX_STATION_FUEL);
    area.w = BLOCK_SZ - 2 * FUEL_BORDER;
    area.h = size;
    Image_paint_area(IMG_FUEL,
		     x + FUEL_BORDER,
		     y + FUEL_BORDER,
		     frame,
		     &area,
		     fuelColorRGBA);
}

void Gui_paint_base(int x, int y, int id, int team, int type)
{
    switch (type) {
    case SETUP_BASE_UP:
        Image_paint(IMG_BASE_DOWN, x, y, 0, whiteRGBA);
        break;
    case SETUP_BASE_DOWN:
        Image_paint(IMG_BASE_UP, x, y + 1, 0, whiteRGBA);
        break;
    case SETUP_BASE_LEFT:
        Image_paint(IMG_BASE_RIGHT, x, y, 0, whiteRGBA);
        break;
    case SETUP_BASE_RIGHT:
        Image_paint(IMG_BASE_LEFT, x - 1, y, 0, whiteRGBA);
        break;
    default:
        errno = 0;
        error("Bad base dir.");
        return;
    }

    int color;
    homebase_t *base = NULL;
    other_t *other;
    bool do_basewarning = false;

    if (!(other = Other_by_id(id))) return;

    if (baseNameColorRGBA) {
	if (!(color = Life_color(other)))
	    color = baseNameColorRGBA;
    } else
	color = whiteRGBA;

    x = x + BLOCK_SZ / 2;
    y = y + BLOCK_SZ / 2;

    base = Homebase_by_id(id);
    if (base != NULL) {
	/*
	 * Hacks to support Mara's base warning on new servers and
	 * the red "meter" basewarning on old servers.
	 */
	if (loops < base->appeartime)
	    do_basewarning = true;

	if (version < 0x4F12 && do_basewarning) {
	    if (baseWarningType & 1) {
		/* We assume the ship will appear after 3 seconds. */
		int count = 360 * (base->appeartime - loops) / (3 * clientFPS);
		LIMIT(count, 0, 360);
		/* red box basewarning */
		if (count > 0 && (baseWarningType & 1))
		    Gui_paint_appearing(x, y,
					id, count);
	    }
	}
    }
    /* Mara's flashy basewarning */
    if (do_basewarning && (baseWarningType & 2)) {
	if (loopsSlow & 1) {
	    if (color != whiteRGBA)
		color = whiteRGBA;
	    else
		color = redRGBA;
	}
    }

    color |= 0x000000ff;
    switch (type) {
    case SETUP_BASE_UP:
	mapprint(&mapfont,color,CENTER,DOWN ,(x)    	    	,(y - BLOCK_SZ / 2),other->name);
        break;
    case SETUP_BASE_DOWN:
	mapprint(&mapfont,color,CENTER,UP   ,(x)    	    	,(y + BLOCK_SZ / 1.5),other->name);
        break;
    case SETUP_BASE_LEFT:
	mapprint(&mapfont,color,RIGHT,UP    ,(x + BLOCK_SZ / 2) ,(y),other->name);
        break;
    case SETUP_BASE_RIGHT:
	mapprint(&mapfont,color,LEFT,UP     ,(x - BLOCK_SZ / 2) ,(y),other->name);
        break;
    default:
        errno = 0;
        error("Bad base dir.");
    }
}

void Gui_paint_decor(int x, int y, int xi, int yi, int type,
		     bool last, bool more_y)
{
}

void Gui_paint_border(int x, int y, int xi, int yi)
{
    set_alphacolor(wallColorRGBA);
    glBegin(GL_LINE);
    	glVertex2i(x, y);
    	glVertex2i(xi, yi);
    glEnd();
}

void Gui_paint_visible_border(int x, int y, int xi, int yi)
{
    setupPaint_moving();
    set_alphacolor(hudColorRGBA);
    glBegin(GL_LINE_LOOP);
    	glVertex2i(x, y);
    	glVertex2i(x, yi);
    	glVertex2i(xi, yi);
    	glVertex2i(xi, y);
    glEnd();
    setupPaint_stationary();
}

void Gui_paint_hudradar_limit(int x, int y, int xi, int yi)
{
    set_alphacolor(blueRGBA);
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
    Image_paint(own ? IMG_HOLDER_FRIEND : IMG_HOLDER_ENEMY, x, y, 0, whiteRGBA);
}

void Gui_paint_walls(int x, int y, int type)
{
    set_alphacolor(wallColorRGBA);
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
    set_alphacolor(wallColorRGBA);
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

    polygon = polygons[i];
    p_style = polygon_styles[polygon.style];
    e_style = edge_styles[p_style.def_edge_style];

    if (BIT(p_style.flags, STYLE_INVISIBLE)) return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

#if 1
    /* This ugly mess appears to stop most of the walls' wobbling...*/
    /* TODO: sort this mess out */
    int xtrim = 0;
    if (scale != 1.0)
    	if (ABS(world.x-polygon.points[0].x)<(Setup->width/2))
    	    xtrim = ceil(scale)*(world.x<(polygon.points[0].x+1));
    	else
    	    xtrim = 1;
	
    int ytrim = 0;
    if (scale != 1.0)
    	if (ABS(world.y-polygon.points[0].y)<(Setup->height/2))
    	    ytrim = -(world.y>(polygon.points[0].y));
    	else
    	    ytrim = -(int)scale;
    	
    glTranslatef((int)(( xoff * Setup->width + polygon.points[0].x + ((int)((-world.x)*scale)/scale) + xtrim ) * scale),
		 (int)(( yoff * Setup->height + polygon.points[0].y + ((int)((-world.y)*scale)/scale) + ytrim ) * scale),
	0);
    glScalef(scale, scale, 0);
#elif 0
    /* This makes the textures flicker */
    glScalef(scale, scale, 0);
    glTranslatef(xoff * Setup->width + polygon.points[0].x - world.x,
		 yoff * Setup->height + polygon.points[0].y - world.y,
		 0);
#else
    /* This makes the walls wobble */
    glTranslatef((int)((xoff * Setup->width + polygon.points[0].x
			- world.x) * scale),
		 (int)((yoff * Setup->height + polygon.points[0].y
			- world.y) * scale),
	0);
    glScalef(scale, scale, 0);
#endif
    
    if ((instruments.showTexturedWalls || instruments.showFilledWorld) &&
	    BIT(p_style.flags, STYLE_TEXTURED | STYLE_FILLED)) {
	if (BIT(p_style.flags, STYLE_TEXTURED)
	        && instruments.showTexturedWalls) {
	    Image_use_texture(p_style.texture);
	    glCallList(polyListBase + i);
	    Image_no_texture();
	} else {
	    set_alphacolor((p_style.rgb << 8) | 0xff);
	    glCallList(polyListBase + i);
	}
    }
/* might be useful when trying to fix wall-wobbling */
#if 0    
    mapprint(&mapfont,whiteRGBA,LEFT,DOWN,0,0,"%i %i %i %i %i",i,polygon.points[0].x,world.x,polygon.points[0].y,world.y);
#endif
    set_alphacolor((e_style.rgb << 8) | 0xff);
    glLineWidth(e_style.width * scale);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);/* this is about half the cost of drawing objects...*/
    glCallList(polyEdgeListBase + i);
    glDisable(GL_LINE_SMOOTH);
    /*glDisable(GL_BLEND);*/
    glLineWidth(1);
    glPopMatrix();
}


/* Object painting */


void Gui_paint_item_object(int type, int x, int y)
{
}

void Gui_paint_ball(int x, int y)
{
    Image_paint(IMG_BALL, x - BALL_RADIUS, y - BALL_RADIUS, 0, whiteRGBA);
}

void Gui_paint_ball_connector(int x_1, int y_1, int x_2, int y_2)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    set_alphacolor(connColorRGBA);
    glBegin(GL_LINES);
    glVertex2i(x_1, y_1);
    glVertex2i(x_2, y_2);
    glEnd();
    /*glDisable(GL_BLEND);*/
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
		x + world.x - 3,
		world.y - 6 + ext_view_height - y,
		5, whiteRGBA);
}

void Gui_paint_teamshot(int x, int y)
{
    Image_paint(IMG_BULLET_OWN,
		x + world.x - 3,
		world.y - 6 + ext_view_height - y,
		5, whiteRGBA);
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
		(count <= 0 || loopsSlow % 10 >= 5) ? 1 : 0, whiteRGBA);
}

void Gui_paint_appearing(int x, int y, int id, int count)
{
    const unsigned hsize = 3 * BLOCK_SZ / 7;
    int minx,miny,maxx,maxy;
    int color;
    other_t *other = Other_by_id(id);

    /* Make a note we are doing the base warning */
    if (version >= 0x4F12) {
	homebase_t *base = Homebase_by_id(id);
	if (base != NULL)
	    base->appeartime = loops + (count * clientFPS) / 120;
    }

    minx = x - (int)hsize;
    miny = y - (int)hsize;
    maxx = minx + 2 * hsize + 1;
    maxy = miny + (unsigned)(count / 180. * hsize + 1);

    color = Life_color(other);
    set_alphacolor((color)?color:redRGBA);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_QUADS);
    	glVertex2i(minx , miny);
    	glVertex2i(maxx , miny);
    	glVertex2i(maxx , maxy);
    	glVertex2i(minx , maxy);
    glEnd();
}

void Gui_paint_ecm(int x, int y, int size)
{
}

void Gui_paint_refuel(int x_0, int y_0, int x_1, int y_1)
{
    /*if (texturedObjects) return;*/
    int stipple = 4;
    /*int off = 2*stipple;
    float tmpx,tmpy,len;

    tmpx = x_0 - x_1;
    tmpy = y_0 - y_1;
    len = sqrt(tmpx*tmpx + tmpy*tmpy);
    tmpx = (off -1-loops % (off))*tmpx/len;
    tmpy = (off -1-loops % (off))*tmpy/len;*/

    set_alphacolor(fuelColorRGBA);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineStipple(stipple, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    /*glVertex2i(x_1 + tmpx, y_1 + tmpy);*/
    glVertex2i(x_0, y_0);
    glVertex2i(x_1, y_1);
    /*glVertex2i(x_1 + tmpx, y_1 + tmpy);*/
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

void Gui_paint_connector(int x_0, int y_0, int x_1, int y_1, int tractor)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    set_alphacolor(connColorRGBA);
    glLineStipple(tractor ? 2 : 4, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    glVertex2i(x_0, y_0);
    glVertex2i(x_1, y_1);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    /*glDisable(GL_BLEND);*/
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
    case 0:	return team0ColorRGBA;
    case 1:	return team1ColorRGBA;
    case 2:	return team2ColorRGBA;
    case 3:	return team3ColorRGBA;
    case 4:	return team4ColorRGBA;
    case 5:	return team5ColorRGBA;
    case 6:	return team6ColorRGBA;
    case 7:	return team7ColorRGBA;
    case 8:	return team8ColorRGBA;
    case 9:	return team9ColorRGBA;
    default:    break;
    }
    return 0;
}

int Life_color_by_life(int life)
{
    int color;

    if (life > 2)
	color = manyLivesColorRGBA;
    else if (life == 2)
	color = twoLivesColorRGBA;
    else if (life == 1)
	color = oneLifeColorRGBA;
    else /* we catch all */
	color = zeroLivesColorRGBA;
    return color;
}

int Life_color(other_t *other)
{
    int color = 0; /* default is 'no special color' */

    if (other
	&& (other->mychar == ' ' || other->mychar == 'R')
	&& BIT(Setup->mode, LIMITED_LIVES))
	color = Life_color_by_life(other->life);
    return color;
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
    int ship_color = whiteRGBA;

#ifndef NO_BLUE_TEAM
    if (BIT(Setup->mode, TEAM_PLAY)
	&& eyesId != id
	&& other != NULL
	&& eyeTeam == other->team) {
	/* Paint teammates and allies ships with last life in teamLWColorRGBA */
	if (BIT(Setup->mode, LIMITED_LIVES)
	    && (other->life == 0))
	    ship_color = teamLWColorRGBA;
	else
	    ship_color = blueRGBA;
    }

    if (eyes != NULL
	&& eyesId != id
	&& other != NULL
	&& eyes->alliance != ' '
	&& eyes->alliance == other->alliance) {
	/* Paint teammates and allies ships with last life in teamLWColorRGBA */
	if (BIT(Setup->mode, LIMITED_LIVES)
	    && (other->life == 0))
	    ship_color = teamLWColorRGBA;
	else
	    ship_color = blueRGBA;
    }

    if (Gui_is_my_tank(other))
	ship_color = blueRGBA;
#endif
    if (roundDelay > 0 && ship_color == whiteRGBA)
	ship_color = redRGBA;

    /* Check for team color */
    if (other && BIT(Setup->mode, TEAM_PLAY)) {
	int team_color = Team_color(other->team);
	if (team_color)
	    return team_color;
    }

    /* Vato color hack start, edited by mara & kps */
    if (BIT(Setup->mode, LIMITED_LIVES)) {
	/* Paint your ship in selfLWColorRGBA when on last life */
	if (eyes != NULL
	    && eyes->id == id
	    && eyes->life == 0) {
	    ship_color = selfLWColorRGBA;
	}

	/* Paint enemy ships with last life in enemyLWColorRGBA */
	if (eyes != NULL
	    && eyes->id != id
	    && other != NULL
	    && eyeTeam != other->team
	    && other->life == 0) {
	    ship_color = enemyLWColorRGBA;
	}
    }
    /* Vato color hack end */

    return ship_color;
}

static void Gui_paint_ship_name(int x, int y, other_t *other)
{
    int color = Life_color(other);

    /* TODO : do all name painting together, so we don't need
     * all theese setupPaint<foo> calls
     */
    if (shipNameColorRGBA) {
	if (!color)
	    color = shipNameColorRGBA;

	mapprint(&mapfont, color, CENTER, DOWN,x,y - SHIP_SZ,"%s",other->id_string);
    } else
	color = blueRGBA;

    if (instruments.showLivesByShip
	&& BIT(Setup->mode, LIMITED_LIVES)) {
	if (other->life < 1)
	    color = whiteRGBA;

	mapprint(&mapfont, color, LEFT, CENTER,x + SHIP_SZ,y,"%d", other->life);
    }
}

void Gui_paint_ship(int x, int y, int dir, int id, int cloak, int phased,
		    int shield, int deflector, int eshield)
{
    int i;
    shipshape_t *ship;
    shapepos    point;
    other_t 	*other;

    ship = Ship_by_id(id);
    if (!(other = Other_by_id(id))) return;
    
    if (shield) {
	Image_paint(IMG_SHIELD, x - 27, y - 27, 0, blueRGBA - 128);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    set_alphacolor(Gui_calculate_ship_color(id,other));

    glBegin(GL_LINE_LOOP);
    for (i = 0; i < ship->num_points; i++) {
	point = Ship_get_point(ship, i, dir);
	glVertex2i(x + point.pxl.x, y + point.pxl.y);
    }
    glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    if (self != NULL
    	&& self->id != id
    	&& other != NULL)
    	    Gui_paint_ship_name(x,y,other);
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

    if (!get_alpha(scoreObjectColorRGBA))
	return;

    for (i = 0; i < MAX_SCORE_OBJECTS; i++) {
	score_object_t*	sobj = &score_objects[i];
	if (sobj->life_time > 0) {
	    if (loopsSlow % 3) {
	    	/* approximate font width to h =( */
		x = sobj->x * BLOCK_SZ + BLOCK_SZ/2;
		y = sobj->y * BLOCK_SZ + BLOCK_SZ/2;
  		if (wrap(&x, &y)) {
		    /*mapprint(&mapfont,scoreObjectColorRGBA,CENTER,CENTER,x,y,sobj->msg);*/
		    if (!score_object_texs[i].texture)
		    	draw_text(&mapfont, scoreObjectColorRGBA
			    	    ,CENTER,CENTER, x, y, sobj->msg, true
				    , &score_object_texs[i],false);
		    else disp_text(&score_object_texs[i],scoreObjectColorRGBA,CENTER,CENTER,x,y,false);
		}
	    }
	    sobj->life_time -= timePerFrame;
	    if (sobj->life_time <= 0.0) {
		sobj->life_time = 0.0;
		sobj->hud_msg_len = 0;
	    }
	} else {
	    if (score_object_texs[i].texture) free_string_texture(&score_object_texs[i]);
	}
    }
}

void Paint_select(void)
{
    if(!select_bounds) return;
    set_alphacolor(selectionColorRGBA);
    glBegin(GL_LINE_LOOP);
    	glVertex2i(select_bounds->x 	    	    	,select_bounds->y);
    	glVertex2i(select_bounds->x + select_bounds->w	,select_bounds->y);
    	glVertex2i(select_bounds->x + select_bounds->w	,select_bounds->y + select_bounds->h);
    	glVertex2i(select_bounds->x 	    	    	,select_bounds->y + select_bounds->h);
    glEnd();
}

void Paint_client_fps(void)
{
    int			x, y;

    if (!hudColorRGBA)
	return;

    x = draw_width - 20;
    /* Better make sure it's below the meters */
    y = draw_height - 9*((20>gamefont.h)?20:gamefont.h);
;
    HUDprint(&gamefont,hudColorRGBA,RIGHT,DOWN,x,y,"FPS: %d",clientFPS);
}

static void Paint_meter(int xoff, int y, string_tex_t *tex, int val, int max,
			int meter_color)
{
    const int	mw1_4 = meterWidth/4,
		mw2_4 = meterWidth/2,
		mw3_4 = 3*meterWidth/4,
		mw4_4 = meterWidth,
		BORDER = 5;
    int		x, xstr;
    int x_alignment;
    int color;

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
    	glVertex2i(x,y);
    	glVertex2i(x,y+2+meterHeight-3);
    	glVertex2i(x+(int)(((meterWidth)*val)/(max?max:1)),y+2+meterHeight-3);
    	glVertex2i(x+(int)(((meterWidth)*val)/(max?max:1)),y);
    glEnd();



    /* meterBorderColorRGBA = 0 obviously means no meter borders are drawn */
    if (meterBorderColorRGBA) {
    	color = meterBorderColorRGBA;

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

    if (!meterBorderColorRGBA)
	color = meter_color;

    /*HUDprint(&gamefont,color,x_alignment,UP,xstr,draw_height - y - meterHeight,title);*/
    disp_text(tex,color,x_alignment,UP,xstr,draw_height - y - meterHeight,true);
}

void Paint_meters(void)
{
    int spacing = (20>gamefont.h)?20:gamefont.h;
    int y = spacing, color;
    static bool setup_texs = true;
    
    if (setup_texs) {
    	render_text(&gamefont,"Fuel"	    	, &meter_texs[0]);
    	render_text(&gamefont,"Power"	    	, &meter_texs[1]);
     	render_text(&gamefont,"Turnspeed"   	, &meter_texs[2]);
   	render_text(&gamefont,"Packet"	   	, &meter_texs[3]);
    	render_text(&gamefont,"Loss"	    	, &meter_texs[4]);
    	render_text(&gamefont,"Drop"	    	, &meter_texs[5]);
    	render_text(&gamefont,"Lag" 	    	, &meter_texs[6]);
    	render_text(&gamefont,"Thrust Left" 	, &meter_texs[7]);
    	render_text(&gamefont,"Shields Left"	, &meter_texs[8]);
    	render_text(&gamefont,"Phasing left"	, &meter_texs[9]);
    	render_text(&gamefont,"Self destructing", &meter_texs[10]);
    	render_text(&gamefont,"SHUTDOWN"    	, &meter_texs[11]);
	setup_texs = false;
    }
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (fuelMeterColorRGBA)
	Paint_meter(-10, y += spacing, &meter_texs[0],
		    (int)fuelSum, (int)fuelMax, fuelMeterColorRGBA);

    if (powerMeterColorRGBA)
	color = powerMeterColorRGBA;
    else if (controlTime > 0.0)
	color = temporaryMeterColorRGBA;
    else
	color = 0;

    if (color)
	Paint_meter(-10, y += spacing, &meter_texs[1],
		    (int)displayedPower, (int)MAX_PLAYER_POWER, color);

    if (turnSpeedMeterColorRGBA)
	color = turnSpeedMeterColorRGBA;
    else if (controlTime > 0.0)
	color = temporaryMeterColorRGBA;
    else
	color = 0;

    if (color)
	Paint_meter(-10, y += spacing, &meter_texs[2],
		    (int)displayedTurnspeed, (int)MAX_PLAYER_TURNSPEED, color);

    if (controlTime > 0.0) {
	controlTime -= timePerFrame;
	if (controlTime <= 0.0)
	    controlTime = 0.0;
    }

    if (packetSizeMeterColorRGBA)
	Paint_meter(-10, y += spacing, &meter_texs[3],
		   (packet_size >= 4096) ? 4096 : packet_size, 4096,
		    packetSizeMeterColorRGBA);
    if (packetLossMeterColorRGBA)
	Paint_meter(-10, y += spacing, &meter_texs[4], packet_loss, FPS,
		    packetLossMeterColorRGBA);
    if (packetDropMeterColorRGBA)
	Paint_meter(-10, y += spacing, &meter_texs[5], packet_drop, FPS,
		    packetDropMeterColorRGBA);
    if (packetLagMeterColorRGBA)
	Paint_meter(-10, y += spacing, &meter_texs[6], MIN(packet_lag, 1 * FPS), 1 * FPS,
		    packetLagMeterColorRGBA);

    if (temporaryMeterColorRGBA) {
	if (thrusttime >= 0 && thrusttimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3,
			&meter_texs[7],
			(thrusttime >= thrusttimemax
			 ? thrusttimemax : thrusttime),
			thrusttimemax, temporaryMeterColorRGBA);

	if (shieldtime >= 0 && shieldtimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + spacing,
			&meter_texs[8],
			(shieldtime >= shieldtimemax
			 ? shieldtimemax : shieldtime),
			shieldtimemax, temporaryMeterColorRGBA);

	if (phasingtime >= 0 && phasingtimemax > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 2*spacing,
			&meter_texs[9],
			(phasingtime >= phasingtimemax
			 ? phasingtimemax : phasingtime),
			phasingtimemax, temporaryMeterColorRGBA);

	if (destruct > 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 3*spacing,
			&meter_texs[10], destruct, 150,
			temporaryMeterColorRGBA);

	if (shutdown_count >= 0)
	    Paint_meter((ext_view_width-300)/2 -32, 2*ext_view_height/3 + 4*spacing,
			&meter_texs[11], shutdown_count, shutdown_delay,
			temporaryMeterColorRGBA);
    }
    glDisable(GL_BLEND);
}

static void Paint_lock(int hud_pos_x, int hud_pos_y)
{
}

static void Paint_hudradar(double hrscale, double xlimit, double ylimit, int sz)
{
    int i, x, y;
    int hrw = hrscale * 256;
    int hrh = hrscale * RadarHeight;
    double xf = (double) hrw / (double) Setup->width;
    double yf = (double) hrh / (double) Setup->height;

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

 	    x = x + draw_width / 2;
 	    y = -y + draw_height / 2;

	    if (radar_ptr[i].type == normal) {
		if (hudRadarEnemyColorRGBA)
		    Circle(hudRadarEnemyColorRGBA, x, y, sz, 1);
	    } else {
		if (hudRadarOtherColorRGBA)
		    Circle(hudRadarOtherColorRGBA, x, y, sz, 1);
	    }
	}
    }
}

static void Paint_HUD_items(int hud_pos_x, int hud_pos_y)
{
}

typedef char hud_text_t[50];

void Paint_HUD(void)
{
    const int		BORDER = 3;
    char		str[50];
    int			hud_pos_x, hud_pos_y, size;
    int			did_fuel = 0;
    int			i, j, tex_index, modlen = 0;
    static char		autopilot[] = "Autopilot";
    int tempx,tempy,tempw,temph;
    static hud_text_t 	hud_texts[MAX_HUD_TEXS+MAX_SCORE_OBJECTS];
    
bool newcode = true;

    fontbounds dummy;
    hudRadarLimit = 0.050;
    tex_index = 0;
    glEnable(GL_BLEND);
    /*
     * Show speed pointer
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    if (ptr_move_fact != 0.0
	&& selfVisible
	&& (selfVel.x != 0 || selfVel.y != 0))
	Segment_add(hudColorRGBA,
		    draw_width / 2,
		    draw_height / 2,
		    (int)(draw_width / 2 - ptr_move_fact * selfVel.x),
		    (int)(draw_height / 2 + ptr_move_fact * selfVel.y));

    if (selfVisible && dirPtrColorRGBA) {
	Segment_add(dirPtrColorRGBA,
		    (int) (draw_width / 2 +
			   (100 - 15) * tcos(heading)),
		    (int) (draw_height / 2 -
			   (100 - 15) * tsin(heading)),
		    (int) (draw_width / 2 + 100 * tcos(heading)),
		    (int) (draw_height / 2 - 100 * tsin(heading)));
    }

    /* TODO */
    /* This should be done in a nicer way now (using radar.c maybe) */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (hudRadarEnemyColorRGBA || hudRadarOtherColorRGBA) {
	hudRadarMapScale = (double) Setup->width / (double) 256;
	Paint_hudradar(
	    hudRadarScale,
	    (int)(hudRadarLimit * (ext_view_width / 2)
		  * hudRadarScale / hudRadarMapScale),
	    (int)(hudRadarLimit * (ext_view_height / 2)
		  * hudRadarScale / hudRadarMapScale),
	    hudRadarDotSize);

	/*if (BIT(instruments, MAP_RADAR))*/
	    Paint_hudradar(hudRadarMapScale*scale,
    	    	    	   (active_view_width / 2)*scale,
			   (active_view_height / 2)*scale,
			   SHIP_SZ);
    }


    glDisable(GL_BLEND);
    /* message scan hack by mara*/
    if (instruments.useBallMessageScan) {
	if (ball_shout && msgScanBallColorRGBA)
	    Circle(msgScanBallColorRGBA, draw_width / 2,
		    draw_height / 2, 8*scale,0);
	if (need_cover && msgScanCoverColorRGBA)
	    Circle(msgScanCoverColorRGBA, draw_width / 2,
		    draw_height / 2, 6*scale,0);
    }
    glEnable(GL_BLEND);

    /*
     * Display the HUD
     */
    hud_pos_x = (int)(draw_width / 2 - hud_move_fact * selfVel.x);
    hud_pos_y = (int)(draw_height / 2 + hud_move_fact * selfVel.y);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    /* HUD frame */
    glLineStipple(4, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    if (hudHLineColorRGBA) {
    	set_alphacolor(hudHLineColorRGBA);
    	glBegin(GL_LINES);
    	    glVertex2i(hud_pos_x - hudSize,hud_pos_y - hudSize + HUD_OFFSET);
	    glVertex2i(hud_pos_x + hudSize,hud_pos_y - hudSize + HUD_OFFSET);

	    glVertex2i(hud_pos_x - hudSize,hud_pos_y + hudSize - HUD_OFFSET);
	    glVertex2i(hud_pos_x + hudSize,hud_pos_y + hudSize - HUD_OFFSET);
    	glEnd();
    }
    if (hudVLineColorRGBA) {
    	set_alphacolor(hudVLineColorRGBA);
    	glBegin(GL_LINES);
    	    glVertex2i(hud_pos_x - hudSize + HUD_OFFSET,hud_pos_y - hudSize);
	    glVertex2i(hud_pos_x - hudSize + HUD_OFFSET,hud_pos_y + hudSize);

	    glVertex2i(hud_pos_x + hudSize - HUD_OFFSET,hud_pos_y - hudSize);
	    glVertex2i(hud_pos_x + hudSize - HUD_OFFSET,hud_pos_y + hudSize);
    	glEnd();
    }
    glDisable(GL_LINE_STIPPLE);

    if (hudItemsColorRGBA)
	Paint_HUD_items(hud_pos_x, hud_pos_y);

    /* Fuel notify, HUD meter on */
    if (hudColorRGBA && (fuelTime > 0.0 || fuelSum < fuelLevel3)) {
	did_fuel = 1;
	/* TODO fix this */
	if (newcode) {
	sprintf(str, "%04d", (int)fuelSum);
	tex_index=0;
	if (strcmp(str,hud_texts[tex_index])!=0) {
    	    if (HUD_texs[tex_index].texture)
	    	free_string_texture(&HUD_texs[tex_index]);    	    
	    strlcpy(hud_texts[tex_index],str,50);
	}
	if (!HUD_texs[tex_index].texture)
	    render_text(&gamefont, str, &HUD_texs[tex_index]);
	disp_text(  &HUD_texs[tex_index],hudColorRGBA,LEFT,DOWN
	    	    ,hud_pos_x + hudSize-HUD_OFFSET+BORDER
		    ,hud_pos_y - (hudSize-HUD_OFFSET+BORDER)
		    ,true   );
	} else 
	    HUDprint(&gamefont,hudColorRGBA,LEFT,DOWN,
	    	hud_pos_x + hudSize-HUD_OFFSET+BORDER,
		hud_pos_y - (hudSize-HUD_OFFSET+BORDER),
		"%04d", (int)fuelSum);

	if (numItems[ITEM_TANK]) {
	    if (fuelCurrent == 0)
		strcpy(str,"M ");
	    else
		sprintf(str, "T%d", fuelCurrent);
	    
	    if (newcode) {
	    tex_index=1;
	    if (strcmp(str,hud_texts[tex_index])!=0) {
    	    	if (HUD_texs[tex_index].texture)
		    free_string_texture(&HUD_texs[tex_index]);    	    
    	    	strlcpy(hud_texts[tex_index],str,50);
	    }
	    if (!HUD_texs[tex_index].texture)
	    	render_text(&gamefont, str, &HUD_texs[tex_index]);
	    disp_text(  &HUD_texs[tex_index],hudColorRGBA,LEFT,DOWN
	    	    ,hud_pos_x + hudSize-HUD_OFFSET + BORDER
		    ,hud_pos_y - hudSize-HUD_OFFSET + BORDER
		    ,true   );
	    
	    /* TODO fix this */
	    } else
	    	HUDprint(&gamefont,hudColorRGBA,LEFT,DOWN,
	    	    hud_pos_x + hudSize-HUD_OFFSET + BORDER,
		    hud_pos_y - hudSize-HUD_OFFSET + BORDER,
		    str);
	}
    }

    /* Update the lock display */
    Paint_lock(hud_pos_x, hud_pos_y);

    /* Draw last score on hud if it is an message attached to it */
    if (hudColorRGBA) {
	for (i = 0, j = 0; i < MAX_SCORE_OBJECTS; i++) {
	    score_object_t*	sobj = &score_objects[(i+score_object)%MAX_SCORE_OBJECTS];
	    if (sobj->hud_msg_len > 0) {
	    	dummy = printsize(&gamefont,sobj->hud_msg);
		if (sobj->hud_msg_width == -1)
		    sobj->hud_msg_width = dummy.width;
		if (j == 0 &&
		    sobj->hud_msg_width > 2*hudSize-HUD_OFFSET*2 &&
		    (did_fuel || hudVLineColorRGBA))
		    ++j;
	    	
	    	if (newcode) {
		tex_index=MAX_HUD_TEXS+i;
		if (strcmp(sobj->hud_msg,hud_texts[tex_index])!=0) {
    	    	    if (HUD_texs[tex_index].texture)
		    	free_string_texture(&HUD_texs[tex_index]);    	    
    	    	    strlcpy(hud_texts[tex_index],sobj->hud_msg,50);
	    	}
	    	if (!HUD_texs[tex_index].texture)
	    	    render_text(&gamefont, sobj->hud_msg, &HUD_texs[tex_index]);
	    	
		disp_text(  &HUD_texs[tex_index],hudColorRGBA,CENTER,DOWN
	    	    ,hud_pos_x
		    ,hud_pos_y - (hudSize-HUD_OFFSET + BORDER + j * HUD_texs[tex_index].height)
		    ,true   );
		} else
		    HUDprint(&gamefont,hudColorRGBA,CENTER,DOWN,
		    	hud_pos_x,
			hud_pos_y - (hudSize-HUD_OFFSET + BORDER + j * dummy.height),
			sobj->hud_msg);
		j++;
	    }
	}

	if (time_left > 0) {
    	    if (newcode) {
	    sprintf(str, "%3d:%02d", (int)(time_left / 60), (int)(time_left % 60));
	    tex_index=3;
	    if (strcmp(str,hud_texts[tex_index])!=0) {
    	    	if (HUD_texs[tex_index].texture)
		    free_string_texture(&HUD_texs[tex_index]);    	    
    	    	strlcpy(hud_texts[tex_index],str,50);
	    }
	    if (!HUD_texs[tex_index].texture)
	    	render_text(&gamefont, str, &HUD_texs[tex_index]);
	    disp_text(  &HUD_texs[tex_index],hudColorRGBA,RIGHT,DOWN
	    	    ,hud_pos_x - hudSize+HUD_OFFSET - BORDER
		    ,hud_pos_y + hudSize+HUD_OFFSET + BORDER
		    ,true   );
	    } else
	    	HUDprint(&gamefont,hudColorRGBA,RIGHT,DOWN,
		    hud_pos_x - hudSize+HUD_OFFSET - BORDER,
		    hud_pos_y + hudSize+HUD_OFFSET + BORDER,
		    "%3d:%02d",
		    (int)(time_left / 60), (int)(time_left % 60));
	}

	/* Update the modifiers */
	modlen = strlen(mods);
    	if (newcode) {
	tex_index=4;
	if (strcmp(mods,hud_texts[tex_index])!=0) {
    	    if (HUD_texs[tex_index].texture)
	    	free_string_texture(&HUD_texs[tex_index]);		
    	    strlcpy(hud_texts[tex_index],mods,50);
	}
	if (!HUD_texs[tex_index].texture)
	    render_text(&gamefont, mods, &HUD_texs[tex_index]);
	disp_text(  &HUD_texs[tex_index],hudColorRGBA,RIGHT,UP
		,hud_pos_x - hudSize+HUD_OFFSET-BORDER
	    	,hud_pos_y - hudSize+HUD_OFFSET-BORDER
	    	,true	);
	} else
	    HUDprint(&gamefont,hudColorRGBA,RIGHT,UP,
		hud_pos_x - hudSize+HUD_OFFSET-BORDER,
		hud_pos_y - hudSize+HUD_OFFSET-BORDER,
		mods);

	if (autopilotLight) {
    	    if (newcode) {
	    tex_index=5;
	    if (strcmp(autopilot,hud_texts[tex_index])!=0) {
    	    	if (HUD_texs[tex_index].texture)
		    free_string_texture(&HUD_texs[tex_index]);    	    
    	    	strlcpy(hud_texts[tex_index],autopilot,50);
	    }
	    if (!HUD_texs[tex_index].texture)
	    	render_text(&gamefont, autopilot, &HUD_texs[tex_index]);
	    disp_text(  &HUD_texs[tex_index],hudColorRGBA,RIGHT,DOWN
	    	    ,hud_pos_x
		    ,hud_pos_y + hudSize+HUD_OFFSET + BORDER + HUD_texs[tex_index].height*2
		    ,true   );
	    } else {
	    	dummy = printsize(&gamefont,autopilot);
	    	HUDprint(&gamefont,hudColorRGBA,CENTER,DOWN,
			  hud_pos_x,
			  hud_pos_y + hudSize+HUD_OFFSET + BORDER
			  + dummy.height*2,
			  autopilot);
	    }
	}
    }

    if (fuelTime > 0.0) {
	fuelTime -= timePerFrame;
	if (fuelTime <= 0.0)
	    fuelTime = 0.0;
    }

    /* draw fuel gauge */
    if (fuelGaugeColorRGBA &&
	((fuelTime > 0.0)
	 || (fuelSum < fuelLevel3
	     && ((fuelSum < fuelLevel1 && (loopsSlow % 4) < 2)
		 || (fuelSum < fuelLevel2
		     && fuelSum > fuelLevel1
		     && (loopsSlow % 8) < 4)
		 || (fuelSum > fuelLevel2))))) {

	set_alphacolor(fuelGaugeColorRGBA);
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
    glDisable(GL_BLEND);
}

typedef char msg_txt_t[MSG_LEN];

void Paint_messages(void)
{
    int		i, j, x, y, top_y, bot_y, width, len, i_2;
    const int	BORDER = 10,
		SPACING = messagefont.linespacing;
    const int	BORDERx_bot = BORDER, BORDERx_top = 200 + BORDER;
    message_t	*msg;
    int		last_msg_index = 0, msg_color;

    static msg_txt_t talk_texts[MAX_MSGS];
    static msg_txt_t game_texts[MAX_MSGS];
    static bool first_time = true;
    int offset;    

bool newcode = true;
    
    if (first_time) {
    	for (j = 0 ; j < MAX_MSGS ; ++j) {
	    strlcpy(talk_texts[i],"\0",MSG_LEN);
	    strlcpy(game_texts[i],"\0",MSG_LEN);
    	}
    	first_time = false;
    }
    

/* TODO: find a nicer solution for this, its a bit of a hack ;)
 * couldn't think of one that didn't include changing client.h
 * and messages.c
 */
if (newcode) {
    
    offset = 0;
    for (j = maxMessages-1 ; j >= 0 ; --j) {
    	if (!strlen(TalkMsg[j]->txt)) {
	    strlcpy(talk_texts[j],"\0",MSG_LEN);
	    if (message_texs[j].texture)
	    	free_string_texture(&message_texs[j]);
	} else {
	    bool found_it = false;
	    for (i = offset; i <= j; ++i) {
		if ( (found_it = (strcmp(TalkMsg[j]->txt,talk_texts[j-i])==0))) {
		    if (!i) break;
		    if (j + i >= maxMessages)	
			if (message_texs[j].texture)
	    	    	    free_string_texture(&message_texs[j]);
		    strlcpy(talk_texts[j],talk_texts[j-i],MSG_LEN);
		    message_texs[j]=message_texs[j-i];
		    break;
		}
	    }
		
    	    offset = i;
	    
    	    if (!found_it) {
	    	strlcpy(talk_texts[j],TalkMsg[j]->txt,MSG_LEN);
	    	message_texs[j].texture=0;
	    }
	}	
    }
    
    offset = 0;
    for (j = maxMessages-1 ; j >= 0 ; --j) {
    	if (!strlen(GameMsg[j]->txt)) {
	    strlcpy(game_texts[j],"\0",MSG_LEN);
	    if (message_texs[j+MAX_MSGS].texture)
	    	free_string_texture(&message_texs[j+MAX_MSGS]);
	} else {
	    bool found_it = false;
	    for (i = offset; i <= j; ++i) {
		if ( (found_it = (strcmp(GameMsg[j]->txt,game_texts[j-i])==0))) {
		    if (!i) break;
		    if (j + i >= maxMessages)	
			if (message_texs[j+MAX_MSGS].texture)
	    	    	    free_string_texture(&message_texs[j+MAX_MSGS]);
		    strlcpy(game_texts[j],game_texts[j-i],MSG_LEN);
		    message_texs[j+MAX_MSGS]=message_texs[j-i+MAX_MSGS];
		    break;
		}
	    }
		
    	    offset = i;
	    
    	    if (!found_it) {
	    	strlcpy(game_texts[j],GameMsg[j]->txt,MSG_LEN);
	    	message_texs[j+MAX_MSGS].texture=0;
	    }
	}	
    }
}
    	
    top_y = draw_height - messagefont.linespacing;
    bot_y = messagefont.linespacing;

    /* get number of player messages */
    if (selectionAndHistory) {
	while (last_msg_index < maxMessages
		&& TalkMsg[last_msg_index]->len != 0)
	    last_msg_index++;
	last_msg_index--; /* make it an index */
    }

    for (i = (instruments.showReverseScroll ? 2 * maxMessages - 1 : 0);
	 (instruments.showReverseScroll ? i >= 0 : i < 2 * maxMessages);
	 i += (instruments.showReverseScroll ? -1 : 1)) {
	if (i < maxMessages)
	    msg = TalkMsg[i];
	else
	    msg = GameMsg[i - maxMessages];
	if (msg->len == 0)
	    continue;

	/*
	 * While there is something emphasized, freeze the life time counter
	 * of a message if it is not drawn "flashed" (not in oldmessagesColorRGBA)
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
	    msg_color = oldmessagesColorRGBA;
	else {
	    /* If paused, don't bother to paint messages in mscScan* colors. */
	    if (self && strchr("P", self->mychar))
		msg_color = messagesColorRGBA;
	    else {
		switch (msg->bmsinfo) {
		case BmsBall:	msg_color = msgScanBallColorRGBA;	break;
		case BmsSafe:	msg_color = msgScanSafeColorRGBA;	break;
		case BmsCover:	msg_color = msgScanCoverColorRGBA;	break;
		case BmsPop:	msg_color = msgScanPopColorRGBA;	break;
		default:	msg_color = messagesColorRGBA;	break;
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
	    if (!instruments.showMessages)
		continue;
	    x = BORDERx_bot;
	    y = bot_y;
	    bot_y += SPACING;
	}

	i_2=(i<maxMessages)?i:(MAX_MSGS+i-maxMessages);
	
	len = (int)(charsPerSecond * (MSG_LIFE_TIME - msg->lifeTime));
	
	/* TODO: make sure this works! */
	/* new message? */
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
		    /*xoff2 = XTextWidth(messageFont, msg->txt, selection.draw.x1);*/
		    xoff2 = nprintsize(&messagefont,l,msg->txt).width;/*this is not accurate*/

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
			    xoff3 = nprintsize(&messagefont,selection.draw.x2 + 1,msg->txt).width;/*this is not accurate*/
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
		    xoff3 = nprintsize(&messagefont,selection.draw.x2 + 1,msg->txt).width*len;/*this is not accurate*/
		    l3 = len - selection.draw.x2 - 1;
		}
	    } /* last line */

    	    /* TODO: make it possible to actually divide this per character
	     * meanwhile just ballpark guess to a fraction of the width... ;)
	     */
	    xpprintf("DO WE EVER GET HERE???\n");
	    if (newcode)
	    	if (!message_texs[i_2].texture) render_text(&gamefont,msg->txt,&message_texs[i_2]);
	    if (ptr) {
		if (!newcode)
		    HUDnprint(&gamefont,msg_color,LEFT,CENTER,x,y,l,ptr);
	    	else {
		    disp_text_fraq(&message_texs[i_2],msg_color,LEFT,CENTER,x,y
	    	    	    	,(float)( (int)&ptr-(int)&msg->txt  	)/msg->len
	    	    	    	,(float)( (int)&ptr-(int)&msg->txt + l	)/msg->len
	    	    	    	,0.0f
	    	    	    	,1.0f
		    	    	,true);
		}
	    }
	    if (ptr2) {
		if (!newcode)
		    HUDnprint(&gamefont,whiteRGBA,LEFT,CENTER,x,y,l2,ptr2);
	    	else {
		    disp_text_fraq(&message_texs[i_2],msg_color,LEFT,CENTER,x,y
	    	    	    	,(float)( (int)&ptr2-(int)&msg->txt 	    )/msg->len
	    	    	    	,(float)( (int)&ptr2-(int)&msg->txt + l2    )/msg->len
	    	    	    	,0.0f
	    	    	    	,1.0f
		    	    	,true);
		}
	    }
	    if (ptr3) {
		if (!newcode)
		    HUDnprint(&gamefont,msg_color,LEFT,CENTER,x,y,l3,ptr2);
	    	else {
		    disp_text_fraq(&message_texs[i_2],msg_color,LEFT,CENTER,x,y
	    	    	    	,(float)( (int)&ptr3-(int)&msg->txt 	    )/msg->len
	    	    	    	,(float)( (int)&ptr3-(int)&msg->txt + l3    )/msg->len
	    	    	    	,0.0f
	    	    	    	,1.0f
		    	    	,true);
		}
	    }

	} else {
    	    if (!newcode)
    	    	HUDnprint(&gamefont,msg_color,LEFT,CENTER,x,y,len,msg->txt);
	    else {
		if (!message_texs[i_2].texture) render_text(&gamefont,msg->txt,&message_texs[i_2]);
	    	disp_text_fraq(&message_texs[i_2],msg_color,LEFT,CENTER,x,y
	    	    	    ,0.0f
	    	    	    ,(float)len/msg->len
	    	    	    ,0.0f
	    	    	    ,1.0f
			    ,true);
	    }
	}

    	if (!newcode)
	    width = nprintsize(&messagefont,MIN(len, msg->len),msg->txt).width; /*this is not accurate*/
	else
	    width = message_texs[i_2].width;
    }
}

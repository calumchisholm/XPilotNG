/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifdef _WINDOWS
#include "../common/NT/winX.h"
#include "NT/winbitmap.h"
#include "NT/winClient.h"
#endif
#ifndef _WINDOWS
#include <stdlib.h>
#include <X11/Xlib.h>
#endif

#include "bitmaps.h"
#include "gfx2d.h"
#include "xpmread.h"

#include "error.h"
#include "const.h"
#include "paint.h"
#include "paintdata.h"


xp_pixmap_t object_pixmaps[] = {
    { "holder1.ppm"	    , 1},
    { "holder2.ppm"	    , 1},
    { "ball.ppm"	    , 1},
    { "ship_red.ppm"	    , 128},
    { "ship_blue.ppm"	    , 128},
    { "ship_red2.ppm"	    , 128},
    { "bullet.ppm"	    , -8},
    { "bullet_blue.ppm"    ,  -8},
    { "base_down.ppm"	    , 1},
    { "base_left.ppm"	    , 1},
    { "base_up.ppm"	    , 1},
    { "base_right.ppm"	    , 1},
    { "fuelcell.ppm"	    , 1},
    { "fuel2.ppm"	    , -16},
    { "allitems.ppm"	    , -30},
    { "cannon_down.ppm"    , 1},
    { "cannon_left.ppm"    , 1},
    { "cannon_up.ppm"	    , 1},
    { "cannon_right.ppm"   , 1},
    { "sparks.ppm"	    , -8},
    { "paused.ppm"	    , -2},
    { "wall_top.ppm"	    , 1},
    { "wall_left.ppm"	    , 1},
    { "wall_bottom.ppm"    , 1},
    { "wall_right.ppm"	    , 1},
    { "wall_ul.ppm"	    , 1},
    { "wall_ur.ppm"	    , 1},
    { "wall_dl.ppm"	    , 1},
    { "wall_dr.ppm"	    , 1},
    { "wall_fi.ppm"	    , 1},
    { "wall_url.ppm"	    , 1},
    { "wall_ull.ppm"	    , 1},
    { "clouds.ppm"	    , 1},
    { "logo.ppm"	    , 1},
    { "refuel.ppm"	    , -4},
    { "wormhole.ppm"	    , 8},
    { "mine_team.ppm"	    , 1},
    { "mine_other.ppm"	    , 1},
    { "concentrator.ppm"    , 32},
    { "plus.ppm"	    , 1},
    { "minus.ppm"	    , 1},
    { "checkpoint.ppm"	    , -2},
    { "meter.ppm"	    , -2},
};

xp_pixmap_t *pixmaps = 0;
int num_pixmaps = 0, max_pixmaps = 0;


static void Bitmap_create (Drawable d, xp_pixmap_t *pixmap, int bmp);
static void Bitmap_create_begin (Drawable d, xp_pixmap_t *pm, int bmp);
static void Bitmap_create_end (Drawable d);
static void Bitmap_picture_copy (xp_pixmap_t *xp_pixmap, int image);
static void Bitmap_picture_scale (xp_pixmap_t *xp_pixmap, int image);
static void Bitmap_set_pixel(xp_pixmap_t *, int, int, int, RGB_COLOR);


/**
 * Adds the standard object bitmaps (aka. block bitmaps) specified
 * in the object_pixmaps array into global pixmaps array.
 */
int Add_object_bitmaps (void)
{
    int i;
    xp_pixmap_t pixmap;
    for (i = 0; i < NUM_OBJECT_BITMAPS; i++) {
        pixmap = object_pixmaps[i];
        pixmap.scalable = 1;
        STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    }
    return 0;
}


/**
 * Defines the standard texture bitmaps specified in the standard_textures
 * array into global pixmaps array.
 */
int Add_default_textures (void)
{
    xp_pixmap_t pixmap;
    pixmap.filename="rock4.xpm";
    pixmap.count=1;
    pixmap.scalable=false;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    /* this is for decor */
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);

    pixmap.filename="ball.xpm";
    pixmap.count=1;
    pixmap.scalable=false;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);

    return 0;
}


/**
 * Adds a new bitmap needed by the current map into global pixmaps.
 * Returns the index of the newly added bitmap in the array.
 */
int Add_bitmap (char *filename, int count, bool scalable)
{
    xp_pixmap_t pixmap;
    pixmap.count = count;
    pixmap.scalable = scalable;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    return num_pixmaps;
}


/*
 * Purpose: initialize the bitmaps, currently i call it after
 * item bitmaps has been created.
 * i hacked the rotations member, it's really #images, but it can also
 * be negative.
 * if rotations > 0 then rotate it (resolution = #images)
 * else it's a handdrawn animation (#images = -rotations) or similar images
 * collected in same ppm for convenience for editing purposes (items).
 *
 * return 0 on success.
 * return -1 on error.
 */
int Bitmaps_init (void)
{
    int i, j, count;
    static int initialized = 0;
    if (initialized) return (initialized == 2) ? 0 : -1;
    else initialized = 1;

    for (i = 0; i < num_pixmaps; i++) {

        count = ABS(pixmaps[i].count);

        if (!(pixmaps[i].bitmaps = malloc(count * sizeof(xp_bitmap_t)))) {
            error("not enough memory for bitmaps");
            return -1;
        }

        for (j = 0; j < count; j++)
            pixmaps[i].bitmaps[j].bitmap =
                pixmaps[i].bitmaps[j].mask = None;

        if (Picture_init
            (&pixmaps[i].picture,
             pixmaps[i].filename,
             pixmaps[i].count) == -1)
            return -1;

        pixmaps[i].width = pixmaps[i].picture.width;
        pixmaps[i].height = pixmaps[i].picture.height;
    }

    initialized = 2;
    return 0;
}


int Bitmaps_create (Drawable d)
{
    int i,j;
    for (i = 0; i < num_pixmaps; i++) {
        for (j = 0; j < ABS(pixmaps[i].count); j++) {
            if (pixmaps[i].scalable) {
                pixmaps[i].width = WINSCALE(pixmaps[i].picture.width);
                pixmaps[i].height = WINSCALE(pixmaps[i].picture.height);
            }
            Bitmap_create(d, &pixmaps[i], j);
        }
    }
    return 0;
}


/*
 * Purpose: create a device/OS dependent bitmap.
 * The windows version need to create and lock a device context.
 * I got no clue what the unix version needs before and after drawing the
 * picture to the pixmap.
 * (the windows version just need the Drawable as parameter, the unix version
 * might need more)
 */
static void Bitmap_create (Drawable d, xp_pixmap_t *pixmap, int bmp)
{
    Bitmap_create_begin(d, pixmap, bmp);

    if (pixmap->height == pixmap->picture.height &&
        pixmap->width == pixmap->picture.width) {
	/* exactly same size as original */
	Bitmap_picture_copy(pixmap, bmp);
    } else {
	Bitmap_picture_scale(pixmap, bmp);
    }

    Bitmap_create_end(d);
}




/*
 * Purpose: Take a device independent picture and create a
 * device/os dependent image.
 * This is only used in the scalefactor 1.0 special case.
 *
 * Actually this function could be killed, but it's very fast
 * and it uses the intended original image.
 */
static void Bitmap_picture_copy (xp_pixmap_t *xp_pixmap, int image)
{
    int		x, y;
    RGB_COLOR	color;

    for (y=0; y < xp_pixmap->height; y++) {
	for (x=0; x < xp_pixmap->width; x++) {
	    color = Picture_get_pixel(&(xp_pixmap->picture), image, x, y);
	    Bitmap_set_pixel(xp_pixmap, image, x, y, color);
	}
    }

    /* copy bounding box from original picture. */
    xp_pixmap->bitmaps[image].bbox = xp_pixmap->picture.bbox[image];
}


/*
 * Purpose: Take a device independent picture and create a
 * scaled device/os dependent image.
 * This is for some of us the general case.
 * The trick is for each pixel in the target image
 * to find the area it responds to in the original image, and then
 * find an average of the colors in this area.
 */
static void Bitmap_picture_scale (xp_pixmap_t *xp_pixmap, int image)
{
    int		x, y;
    RGB_COLOR	color;
    double	x_scaled, y_scaled;
    double      dx_scaled, dy_scaled;
    double	orig_height, orig_width;
    int		height, width;

    orig_height = xp_pixmap->picture.height;
    orig_width = xp_pixmap->picture.width;
    height = xp_pixmap->height;
    width = xp_pixmap->width;

    dx_scaled = orig_width  / width;
    dy_scaled = orig_height / height;
    y_scaled = 0;

    for (y = 0; y < height; y++) {
	x_scaled = 0;
	for (x=0; x < width; x++) {
	    color =
                Picture_get_pixel_area
                (&(xp_pixmap->picture), image,
                 x_scaled, y_scaled, dx_scaled, dy_scaled);

	    Bitmap_set_pixel(xp_pixmap, image, x, y, color);
	    x_scaled += dx_scaled;
	}
	y_scaled += dy_scaled;
    }

    /* scale bounding box as well. */
    {
	bbox_t	*src = &xp_pixmap->picture.bbox[image];
	bbox_t	*dst = &xp_pixmap->bitmaps[image].bbox;

	dst->xmin = (int)((width * src->xmin) / orig_width);
	dst->ymin = (int)((height * src->ymin) / orig_height);
	dst->xmax = (int)(((width * src->xmax) + (orig_width - 1 )) /
                          orig_width);
	dst->ymax = (int)(((height * src->ymax) + (orig_height - 1 )) /
                          orig_height);
    }
}


#ifndef _WINDOWS
/*
 * Maybe move this part to a sperate file.
 */

#include "paintdata.h"

extern int		dispDepth;
extern unsigned long	(*RGB)(unsigned char r, unsigned char g, unsigned char b);
static GC maskGC;


/*
 *    Purpose: to allocate and prepare a pixmap for drawing.
 *   this might be inlined in block_bitmap_create instead?
 */
static void Bitmap_create_begin(Drawable d, xp_pixmap_t *pm, int bmp)
{
    Drawable pixmap;

    if (pm->bitmaps[bmp].bitmap) {
	XFreePixmap(dpy, pm->bitmaps[bmp].bitmap);
	pm->bitmaps[bmp].bitmap = None;
    }
    if (pm->bitmaps[bmp].mask) {
	XFreePixmap(dpy, pm->bitmaps[bmp].mask);
	pm->bitmaps[bmp].mask = None;
    }

    if (!(pixmap = XCreatePixmap(dpy, d, pm->width, pm->height, dispDepth))) {
	error("Could not create pixmap");
	exit(1);
    }

    pm->bitmaps[bmp].bitmap = pixmap;

    if (!(pixmap = XCreatePixmap(dpy, d, pm->width, pm->height, 1))) {
	error("Could not create mask pixmap");
	exit(1);
    }
    pm->bitmaps[bmp].mask = pixmap;

    if (!maskGC) {
	XGCValues	xgc;
	unsigned long	values;

	xgc.line_width = 0;
	xgc.line_style = LineSolid;
	xgc.cap_style = CapButt;
	xgc.join_style = JoinMiter;
	xgc.graphics_exposures = False;
	values =
	    GCLineWidth|GCLineStyle|GCCapStyle|GCJoinStyle|GCGraphicsExposures;
	maskGC = XCreateGC(dpy, pixmap, values, &xgc);
    }
}

/*
 * Purpose: to deallocate resources needed during creation of a bitmap.
 */
static void Bitmap_create_end(Drawable d)
{
}

/*
 * Purpose: set 1 pixel in the device/OS dependent bitmap.
 */
static void Bitmap_set_pixel(xp_pixmap_t * xp_pixmap,
			    int bmp, int x, int y,
			    RGB_COLOR color)
{
    unsigned long	pixel;
    unsigned char	r, g, b;

    r = RED_VALUE(color);
    g = GREEN_VALUE(color);
    b = BLUE_VALUE(color);
    pixel = (RGB)(r, g, b);
    SET_FG(pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[bmp].bitmap, gc, x, y);

    pixel = (color) ? 1 : 0;
    XSetForeground(dpy, maskGC, pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[bmp].mask, maskGC, x, y);
}


/*
 * Purpose: Paint a the bitmap specified with img and bmp
 * so that only the pixels inside the bounding box are
 * painted.
 */
void Bitmap_paint(Drawable d, int img, int x, int y, int bmp)
{
    bbox_t *box = &pixmaps[img].bitmaps[bmp].bbox;
    irec   area;

    area.x = box->xmin;
    area.y = box->ymin;
    area.w = box->xmax + 1 - box->xmin;
    area.h = box->ymax + 1 - box->ymin;

    Bitmap_paint_area(d, img, x + area.x, y + area.y, bmp, &area);
}


/*
 * Purpose: Paint an area r of a bitmap specified with img and bmp in
 * a device dependent manner.
 */
void Bitmap_paint_area(Drawable d, int img, int x, int y, int bmp, irec *r)
{
    xp_bitmap_t	*bit = &pixmaps[img].bitmaps[bmp];

    XSetClipOrigin(dpy, gc, x - r->x, y - r->y);
    XSetClipMask(dpy, gc, bit->mask);
    XCopyArea(dpy, bit->bitmap, d, gc, r->x, r->y, r->w, r->h, x, y);
    XSetClipMask(dpy, gc, None);
}


#endif

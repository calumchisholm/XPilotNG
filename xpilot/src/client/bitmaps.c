/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
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

char bitmaps_version[] = VERSION;

/* this gets rid of missing initializer warnings */
#define XP_PIXMAP_INITIALIZER(f, c) { f,c,0,0,0,false,NULL,{0,0,0,NULL,NULL} }

xp_pixmap_t object_pixmaps[] = {
    XP_PIXMAP_INITIALIZER("holder1.ppm", 1),
    XP_PIXMAP_INITIALIZER("holder2.ppm", 1),
    XP_PIXMAP_INITIALIZER("ball.ppm", 1),
    XP_PIXMAP_INITIALIZER("ship_red.ppm", 128),
    XP_PIXMAP_INITIALIZER("ship_blue.ppm", 128),
    XP_PIXMAP_INITIALIZER("ship_red2.ppm", 128),
    XP_PIXMAP_INITIALIZER("bullet.ppm", -8),
    XP_PIXMAP_INITIALIZER("bullet_blue.ppm", -8),
    XP_PIXMAP_INITIALIZER("base_down.ppm", 1),
    XP_PIXMAP_INITIALIZER("base_left.ppm", 1),
    XP_PIXMAP_INITIALIZER("base_up.ppm", 1),
    XP_PIXMAP_INITIALIZER("base_right.ppm", 1),
    XP_PIXMAP_INITIALIZER("fuelcell.ppm", 1),
    XP_PIXMAP_INITIALIZER("fuel2.ppm", -16),
    XP_PIXMAP_INITIALIZER("allitems.ppm", -30),
    XP_PIXMAP_INITIALIZER("cannon_down.ppm", 1),
    XP_PIXMAP_INITIALIZER("cannon_left.ppm", 1),
    XP_PIXMAP_INITIALIZER("cannon_up.ppm", 1),
    XP_PIXMAP_INITIALIZER("cannon_right.ppm", 1),
    XP_PIXMAP_INITIALIZER("sparks.ppm", -8),
    XP_PIXMAP_INITIALIZER("paused.ppm", -2),
    XP_PIXMAP_INITIALIZER("wall_top.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_left.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_bottom.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_right.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_ul.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_ur.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_dl.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_dr.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_fi.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_url.ppm", 1),
    XP_PIXMAP_INITIALIZER("wall_ull.ppm", 1),
    XP_PIXMAP_INITIALIZER("clouds.ppm", 1),
    XP_PIXMAP_INITIALIZER("logo.ppm", 1),
    XP_PIXMAP_INITIALIZER("refuel.ppm", -4),
    XP_PIXMAP_INITIALIZER("wormhole.ppm", 8),
    XP_PIXMAP_INITIALIZER("mine_team.ppm", 1),
    XP_PIXMAP_INITIALIZER("mine_other.ppm", 1),
    XP_PIXMAP_INITIALIZER("concentrator.ppm", 32),
    XP_PIXMAP_INITIALIZER("plus.ppm", 1),
    XP_PIXMAP_INITIALIZER("minus.ppm", 1),
    XP_PIXMAP_INITIALIZER("checkpoint.ppm", -2),
    XP_PIXMAP_INITIALIZER("meter.ppm", -2),
    XP_PIXMAP_INITIALIZER("asteroidconcentrator.ppm", 32),
};

xp_pixmap_t *pixmaps = 0;
int num_pixmaps = 0, max_pixmaps = 0;


static int Bitmap_init(int img);
static void Bitmap_picture_copy(xp_pixmap_t * xp_pixmap, int image);
static void Bitmap_picture_scale(xp_pixmap_t * xp_pixmap, int image);

static int Bitmap_create_begin(Drawable d, xp_pixmap_t * pm, int bmp);
static int Bitmap_create_end(Drawable d);
static void Bitmap_set_pixel(xp_pixmap_t *, int, int, int, RGB_COLOR);


/**
 * Adds the standard object bitmaps (aka. block bitmaps) specified
 * in the object_pixmaps array into global pixmaps array.
 */
int Bitmap_add_std_objects(void)
{
    int i;
    xp_pixmap_t pixmap;
    for (i = 0; i < NUM_OBJECT_BITMAPS; i++) {
	pixmap = object_pixmaps[i];
	pixmap.scalable = (i == BM_LOGO
			   || i == BM_SCORE_BG) ? false : true;
	pixmap.state = BMS_UNINITIALIZED;
	STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    }
    return 0;
}


/**
 * Defines the standard texture bitmaps specified in the standard_textures
 * array into global pixmaps array.
 */
int Bitmap_add_std_textures(void)
{
    xp_pixmap_t pixmap;
    pixmap.filename = "rock4.xpm";
    pixmap.count = 1;
    pixmap.scalable = false;
    pixmap.state = BMS_UNINITIALIZED;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    /* this is for decor */
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);

    pixmap.filename = "ball.xpm";
    pixmap.count = 1;
    pixmap.scalable = false;
    pixmap.state = BMS_UNINITIALIZED;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);

    return 0;
}


/**
 * Adds a new bitmap needed by the current map into global pixmaps.
 * Returns the index of the newly added bitmap in the array.
 */
int Bitmap_add(char *filename, int count, bool scalable)
{
    xp_pixmap_t pixmap;
    pixmap.filename = xp_strdup(filename);
    pixmap.count = count;
    pixmap.scalable = scalable;
    pixmap.state = BMS_UNINITIALIZED;
    STORE(xp_pixmap_t, pixmaps, num_pixmaps, max_pixmaps, pixmap);
    return num_pixmaps - 1;
}


/**
 * Creates the Pixmaps needed for the given image.
 */
int Bitmap_create(Drawable d, int img)
{
    int j;
    xp_pixmap_t *pix = &pixmaps[img];

    if (pix->state == BMS_UNINITIALIZED)
	Bitmap_init(img);
    if (pix->state != BMS_INITIALIZED)
	return -1;


    for (j = 0; j < ABS(pix->count); j++) {
	if (pix->scalable) {
	    pix->width = UWINSCALE(pix->picture.width);
	    pix->height = UWINSCALE(pix->picture.height);
	}

	if (Bitmap_create_begin(d, pix, j) == -1) {
	    pix->state = BMS_ERROR;
	    return -1;
	}

	if (pix->height == pix->picture.height &&
	    pix->width == pix->picture.width) {
	    Bitmap_picture_copy(pix, j);
	} else
	    Bitmap_picture_scale(pix, j);

	if (Bitmap_create_end(d) == -1) {
	    pix->state = BMS_ERROR;
	    return -1;
	}
    }

    pix->state = BMS_READY;

    return 0;
}


/**
 * Causes all scalable bitmaps to be rescaled (recreated actually)
 * next time needed.
 */
void Bitmap_update_scale(void)
{
    /* This should do the trick.
     * All "good" scalable bitmaps are marked as initialized
     * causing the next Bitmap_get to recreate the bitmap using
     * the current scale factor. Bitmap_create should take care of
     * releasing the device pixmaps no longer needed. */

    int i;
    for (i = 0; i < num_pixmaps; i++)
	if (pixmaps[i].state == BMS_READY && pixmaps[i].scalable)
	    pixmaps[i].state = BMS_INITIALIZED;
}


/**
 * Gets a pointer to the bitmap specified with img and bmp.
 * Ensures that the bitmap returned has been initialized and created
 * properly. Returns NULL if the specified bitmap is not in appropriate
 * state.
 */
xp_bitmap_t *Bitmap_get(Drawable d, int img, int bmp)
{
    if (!fullColor || img < 0 || img >= num_pixmaps)
	return NULL;

    if (pixmaps[img].state != BMS_READY) {
	if (Bitmap_create(d, img) == -1)
	    return NULL;
    }

    return &pixmaps[img].bitmaps[bmp];
}


/**
 * Loads and initializes the given image.
 */
static int Bitmap_init(int img)
{
    int j, count;

    count = ABS(pixmaps[img].count);

    if (!(pixmaps[img].bitmaps = malloc(count * sizeof(xp_bitmap_t)))) {
	error("not enough memory for bitmaps");
	pixmaps[img].state = BMS_ERROR;
	return -1;
    }

    for (j = 0; j < count; j++)
	pixmaps[img].bitmaps[j].bitmap =
	    pixmaps[img].bitmaps[j].mask = None;

    if (Picture_init
	(&pixmaps[img].picture,
	 pixmaps[img].filename, pixmaps[img].count) == -1) {
	pixmaps[img].state = BMS_ERROR;
	return -1;
    }

    pixmaps[img].width = pixmaps[img].picture.width;
    pixmaps[img].height = pixmaps[img].picture.height;
    pixmaps[img].state = BMS_INITIALIZED;

    return 0;
}


/*
 * Purpose: Take a device independent picture and create a
 * device/os dependent image.
 * This is only used in the scalefactor 1.0 special case.
 *
 * Actually this function could be killed, but it's very fast
 * and it uses the intended original image.
 */
static void Bitmap_picture_copy(xp_pixmap_t * xp_pixmap, int image)
{
    int x, y;
    RGB_COLOR color;

    for (y = 0; y < (int)xp_pixmap->height; y++) {
	for (x = 0; x < (int)xp_pixmap->width; x++) {
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
static void Bitmap_picture_scale(xp_pixmap_t * xp_pixmap, int image)
{
    int x, y;
    RGB_COLOR color;
    double x_scaled, y_scaled;
    double dx_scaled, dy_scaled;
    double orig_height, orig_width;
    int height, width;

    orig_height = xp_pixmap->picture.height;
    orig_width = xp_pixmap->picture.width;
    height = xp_pixmap->height;
    width = xp_pixmap->width;

    dx_scaled = orig_width / width;
    dy_scaled = orig_height / height;
    y_scaled = 0;

    for (y = 0; y < height; y++) {
	x_scaled = 0;
	for (x = 0; x < width; x++) {
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
	bbox_t *src = &xp_pixmap->picture.bbox[image];
	bbox_t *dst = &xp_pixmap->bitmaps[image].bbox;

	dst->xmin = (int) ((width * src->xmin) / orig_width);
	dst->ymin = (int) ((height * src->ymin) / orig_height);
	dst->xmax = (int) (((width * src->xmax) + (orig_width - 1)) /
			   orig_width);
	dst->ymax = (int) (((height * src->ymax) + (orig_height - 1)) /
			   orig_height);
    }
}


/*
 * Purpose: Paint a the bitmap specified with img and bmp
 * so that only the pixels inside the bounding box are
 * painted.
 */
void Bitmap_paint(Drawable d, int img, int x, int y, int bmp)
{
    xp_bitmap_t *bit;
    bbox_t *box;
    irec area;

    if ((bit = Bitmap_get(d, img, bmp)) == NULL)
	return;
    box = &bit->bbox;

    area.x = box->xmin;
    area.y = box->ymin;
    area.w = box->xmax + 1 - box->xmin;
    area.h = box->ymax + 1 - box->ymin;

    Bitmap_paint_area(d, bit, x + area.x, y + area.y, &area);
}


#ifndef _WINDOWS
/*
 * Maybe move this part to a sperate file.
 */

extern unsigned long (*RGB) (unsigned char r, unsigned char g,
			     unsigned char b);
static GC maskGC;


/**
 * Allocates and prepares a pixmap for drawing in a platform
 * dependent (UNIX) way.
 */
static int Bitmap_create_begin(Drawable d, xp_pixmap_t * pm, int bmp)
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
	return -1;
    }
    pm->bitmaps[bmp].bitmap = pixmap;

    if (!(pixmap = XCreatePixmap(dpy, d, pm->width, pm->height, 1))) {
	error("Could not create mask pixmap");
	return -1;
    }
    pm->bitmaps[bmp].mask = pixmap;

    if (!maskGC) {
	XGCValues xgc;
	unsigned long values;

	xgc.line_width = 0;
	xgc.line_style = LineSolid;
	xgc.cap_style = CapButt;
	xgc.join_style = JoinMiter;
	xgc.graphics_exposures = False;
	values =
	    GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle |
	    GCGraphicsExposures;
	maskGC = XCreateGC(dpy, pixmap, values, &xgc);
    }

    return 0;
}

/**
 * Deallocates resources needed when creating and drawing a pixmap.
 */
static int Bitmap_create_end(Drawable d)
{
    (void)d;
    return 0;
}


/*
 * Purpose: set 1 pixel in the device/OS dependent bitmap.
 */
static void Bitmap_set_pixel(xp_pixmap_t * xp_pixmap,
			     int bmp, int x, int y, RGB_COLOR color)
{
    unsigned long pixel;
    unsigned char r, g, b;

    r = RED_VALUE(color);
    g = GREEN_VALUE(color);
    b = BLUE_VALUE(color);
    pixel = (RGB) (r, g, b);
    SET_FG(pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[bmp].bitmap, gameGC, x, y);

    pixel = (color) ? 1 : 0;
    XSetForeground(dpy, maskGC, pixel);
    XDrawPoint(dpy, xp_pixmap->bitmaps[bmp].mask, maskGC, x, y);
}


/**
 * Purpose: Paint an area r of xp_bitmap bit in a device dependent manner.
 */
void Bitmap_paint_area(Drawable d, xp_bitmap_t * bit, int x, int y,
		       irec * r)
{
    XSetClipOrigin(dpy, gameGC, x - r->x, y - r->y);
    XSetClipMask(dpy, gameGC, bit->mask);
    XCopyArea(dpy, bit->bitmap, d, gameGC, r->x, r->y, r->w, r->h, x, y);
    XSetClipMask(dpy, gameGC, None);
}

#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"
#include "sdlpaint.h"
#include "SDL_gfxPrimitives.h"
#include "radar.h"

/* kps - had to add prefix so that these would not conflict with options */
color_t wallRadarColorValue = 0xa0;
color_t targetRadarColorValue = 0xa0;
color_t decorRadarColorValue = 0xff0000;
color_t bgRadarColorValue = 0xa00000ff;

static SDL_Rect    radar_bounds;      /* radar position and dimensions */
static SDL_Surface *radar_surface;     /* offscreen image with walls */
static GLuint      radar_texture;     /* above as an OpenGL texture */
static guiarea_t *guiarea;
bool moving;

#define RGBA(RGB) \
    ((RGB) & 0xff000000 ? (RGB) & 0xff000000 : 0xff000000 \
     | ((RGB) & 0xff0000) >> 16 \
     | ((RGB) & 0x00ff00) \
     | ((RGB) & 0x0000ff) << 16)

static int pow2_ceil(int t)
{
    int r;
    for (r = 1; r < t; r <<= 1);
    return r;
}

static void Radar_paint_border(void)
{
    glBegin(GL_LINE_LOOP);
    glColor4ub(0, 0, 0, 0xff);
    glVertex2i(radar_bounds.x, radar_bounds.y + radar_bounds.h);
    glColor4ub(0, 0x00, 0x90, 0xff);
    glVertex2i(radar_bounds.x, radar_bounds.y);
    glColor4ub(0, 0, 0, 0xff);
    glVertex2i(radar_bounds.x + radar_bounds.w, radar_bounds.y);
    glColor4ub(0, 0x00, 0x90, 0xff);
    glVertex2i(radar_bounds.x + radar_bounds.w,
	       radar_bounds.y + radar_bounds.h);
    glEnd();
}

/*
 * Paints a block in the radar to the given position using the
 * given color. This one doesn't do any locking on the surface.
 */
static void Radar_paint_block(SDL_Surface *s, int xi, int yi, color_t color)
{
    SDL_Rect block;
    block.x = xi * radar_bounds.w / Setup->x;
    block.y = radar_bounds.h - (yi + 1) * radar_bounds.h / Setup->y;
    block.w = (xi + 1) * radar_bounds.w / Setup->x - block.x;
    block.h = radar_bounds.h - yi * radar_bounds.h / Setup->y - block.y;

    SDL_FillRect(s, &block, RGBA(color));
}

/*
 * Paints an image of the world on the radar surface when the map
 * is a block map.
 */
static void Radar_paint_world_blocks(SDL_Surface *s)
{
    double damage;
    int i, xi, yi, type, color;
    color_t bcolor[256]; /* color of a block indexed by block type */

    /*
     * Calculate an array which returns the color to use
     * for drawing when indexed with a map block type.
     */
    memset(bcolor, 0, sizeof bcolor);
    bcolor[SETUP_FILLED] =
	bcolor[SETUP_FILLED_NO_DRAW] =
	bcolor[SETUP_REC_LU] =
	bcolor[SETUP_REC_RU] =
	bcolor[SETUP_REC_LD] =
	bcolor[SETUP_REC_RD] =
	bcolor[SETUP_FUEL] = wallRadarColorValue;
    for (i = 0; i < 10; i++)
	bcolor[SETUP_TARGET+i] = targetRadarColorValue;
    for (i = BLUE_BIT; i < 256; i++)
	bcolor[i] = wallRadarColorValue;
    if (instruments.showDecor) {
	bcolor[SETUP_DECOR_FILLED] =
	    bcolor[SETUP_DECOR_LU] =
	    bcolor[SETUP_DECOR_RU] =
	    bcolor[SETUP_DECOR_LD] =
	    bcolor[SETUP_DECOR_RD] = decorRadarColorValue;
    }

    if (SDL_MUSTLOCK(s)) SDL_LockSurface(s);
    SDL_FillRect(s, NULL, RGBA(bgRadarColorValue));

    /* Scan the map and paint the blocks */
    for (xi = 0; xi < Setup->x; xi++) {
        for (yi = 0; yi < Setup->y; yi++) {

            type = Setup->map_data[xi * Setup->y + yi];

            if (type >= SETUP_TARGET
                && type < SETUP_TARGET + 10
                && !Target_alive(xi, yi, &damage))
                type = SETUP_SPACE;

            color = bcolor[type];
            if (color & 0xffffff)
                Radar_paint_block(s, xi, yi, color);
        }
    }

    if (SDL_MUSTLOCK(s)) SDL_UnlockSurface(s);

}

static void Compute_bounds(ipos *min, ipos *max, const irec *b)
{
    min->x = (0 - (b->x + b->w)) / Setup->width;
    if (0 > b->x + b->w) min->x++;
    max->x = (0 + Setup->width - b->x) / Setup->width;
    if (0 + Setup->width < b->x) max->x--;
    min->y = (0 - (b->y + b->h)) / Setup->height;
    if (0 > b->y + b->h) min->y++;
    max->y = (0 + Setup->height - b->y) / Setup->height;
    if (0 + Setup->height < b->y) max->y--;
}

/*
 * Paints an image of the world on the radar surface when the map
 * is a polygon map.
 */
static void Radar_paint_world_polygons(SDL_Surface *s)
{
    int i, j, xoff, yoff;
    ipos min, max;
    Sint16 vx[MAX_VERTICES], vy[MAX_VERTICES];
    color_t color;

    if (SDL_MUSTLOCK(s)) SDL_LockSurface(s);
    SDL_FillRect(s, NULL, RGBA(bgRadarColorValue));

    for (i = 0; i < num_polygons; i++) {

	if (BIT(polygon_styles[polygons[i].style].flags,
		STYLE_INVISIBLE_RADAR)) continue;
	Compute_bounds(&min, &max, &polygons[i].bounds);

	for (xoff = min.x; xoff <= max.x; xoff++) {
	    for (yoff = min.y; yoff <= max.y; yoff++) {

		int x = polygons[i].points[0].x + xoff * Setup->width;
		int y = -polygons[i].points[0].y + (1-yoff) * Setup->height;
		vx[0] = (x * radar_bounds.w) / Setup->width;
		vy[0] = (y * radar_bounds.h) / Setup->height;

		for (j = 1; j < polygons[i].num_points; j++) {
		    x += polygons[i].points[j].x;
		    y -= polygons[i].points[j].y;
		    vx[j]= (x * radar_bounds.w) / Setup->width;
		    vy[j] = (y * radar_bounds.h) / Setup->height;
		}

		color = polygon_styles[polygons[i].style].rgb;
		filledPolygonRGBA(s, vx, vy, j,
				  (color >> 16) & 0xff,
				  (color >> 8) & 0xff,
				  color & 0xff,
				  0xff);
	    }
	}
    }

    if (SDL_MUSTLOCK(s)) SDL_UnlockSurface(s);
}

/*
 * Paints objects (ships, etc.) visible in the radar.
 */
static void Radar_paint_objects(void)
{
    int	i, x, y, s;
    float fx, fy, sx, sy;
    SDL_Rect rb = radar_bounds;

    if (instruments.showSlidingRadar) {
	sx = selfPos.x * rb.w / Setup->width;
	sy = selfPos.y * rb.h / Setup->height;
    }

    for (i = 0; i < num_radar; i++) {
        fx = radar_ptr[i].x * rb.w / RadarWidth;
        fy = radar_ptr[i].y * rb.h / RadarHeight;
        s = radar_ptr[i].size;

	if (instruments.showSlidingRadar) {
            fx = fx - sx;
            fy = fy - sy;
	    if (fx < -rb.w/2)
		fx += rb.w;
	    else if (fx > rb.w/2)
		fx -= rb.w;
	    if (fy < -rb.h/2)
		fy += rb.h;
	    else if (fy > rb.h/2)
		fy -= rb.h;
	    fx += rb.w/2;
	    fy += rb.h/2;
        }

	x = rb.x + (fx + 0.5) - s/2;
	y = rb.y + rb.h - (fy + 0.5) - s/2;

	if (radar_ptr[i].type == friend) glColor3ub(0, 0xff, 0);
	else glColor3ub(0xff, 0xff, 0xff);

	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x + s, y);
	glVertex2i(x + s, y + s);
	glVertex2i(x, y + s);
	glEnd();
    }

    glEnd();

    if (num_radar)
	RELEASE(radar_ptr, num_radar, max_radar);
}

/*
 * Paints player's ship and direction.
 */
static void Radar_paint_self(float xf, float yf)
{
}

/*
 * Paints checkpoints.
 */
static void Radar_paint_checkpoints(float xf, float yf)
{
}

void button(Uint8 button,Uint8 state,Uint16 x,Uint16 y)
{
    if (state == SDL_PRESSED) {
    	if (button == 1)
	    moving = true;
    }
    
    if (state == SDL_RELEASED) {
    	if (button == 1)
	    moving = false;
    }
}

void move(Sint16 xrel,Sint16 yrel,Uint16 x,Uint16 y)
{
    if (moving) {
    	radar_bounds.x += xrel;
    	radar_bounds.y += yrel;
	guiarea->bounds.x = radar_bounds.x;
	guiarea->bounds.y = radar_bounds.y;
    }
}

/*
 * The radar is drawn so that first the walls are painted to an offscreen
 * SDL surface. This surface is then converted into an OpenGL texture.
 * For each frame OpenGL is used to paint rectangles with this walls
 * texture and on top of that the radar objects.
 */
int Radar_init(void)
{
    radar_bounds.x = 10;
    radar_bounds.y = 10;
    radar_bounds.w = 200;
    radar_bounds.h = 200 * RadarHeight / RadarWidth;

    radar_surface =
	SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
                             pow2_ceil(radar_bounds.w),
			     pow2_ceil(radar_bounds.h), 32,
                             RMASK, GMASK, BMASK, AMASK);
    if (!radar_surface) {
        error("Could not create radar surface: %s", SDL_GetError());
        return -1;
    }

    if (oldServer) Radar_paint_world_blocks(radar_surface);
    else Radar_paint_world_polygons(radar_surface);

    glGenTextures(1, &radar_texture);
    glBindTexture(GL_TEXTURE_2D, radar_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		 radar_surface->w, radar_surface->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 radar_surface->pixels);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST);
		    
    guiarea = register_guiarea(radar_bounds,button,move);
    
    return 0;
}

void Radar_cleanup(void)
{
    glDeleteTextures(1, &radar_texture);
    SDL_FreeSurface(radar_surface);
}

static void Radar_blit_world(SDL_Rect *sr, SDL_Rect *dr)
{
    float tx1, ty1, tx2, ty2;

    tx1 = (float)sr->x / radar_surface->w;
    ty1 = (float)sr->y / radar_surface->h;
    tx2 = ((float)sr->x + sr->w) / radar_surface->w;
    ty2 = ((float)sr->y + sr->h) / radar_surface->h;

    glBindTexture(GL_TEXTURE_2D, radar_texture);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(255, 255, 255, 255);

    glBegin(GL_QUADS);
    glTexCoord2f(tx1, ty1); glVertex2i(dr->x, dr->y);
    glTexCoord2f(tx2, ty1); glVertex2i(dr->x + dr->w, dr->y);
    glTexCoord2f(tx2, ty2); glVertex2i(dr->x + dr->w, dr->y + dr->h);
    glTexCoord2f(tx1, ty2); glVertex2i(dr->x, dr->y + dr->h);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

/*
 * Paints the radar surface and objects to the screen.
 */
void Radar_paint(void)
{
    const float	xf = (float)radar_bounds.w / (float)Setup->width;
    const float yf = (float)radar_bounds.h / (float)Setup->height;

    if (instruments.showSlidingRadar) {

        int x, y, w, h;
        float xp, yp, xo, yo;
        SDL_Rect sr, dr;

        xp = (float) (selfPos.x * radar_bounds.w) / Setup->width;
        yp = (float) (selfPos.y * radar_bounds.h) / Setup->height;
        xo = (float) radar_bounds.w / 2;
        yo = (float) radar_bounds.h / 2;
        if (xo <= xp)
            x = (int) (xp - xo + 0.5);
	else
            x = (int) (radar_bounds.w + xp - xo + 0.5);
        if (yo <= yp)
            y = (int) (yp - yo + 0.5);
	else
            y = (int) (radar_bounds.h + yp - yo + 0.5);
	/* CB fixed radar bug   y = radar_bounds.h - y - 1; */
	y = radar_bounds.h - y;
        w = radar_bounds.w - x;
        h = radar_bounds.h - y;

        sr.x = 0; sr.y = 0; sr.w = x; sr.h = y;
        dr.x = w + radar_bounds.x; dr.y = h + radar_bounds.y;
        dr.w = x; dr.h = y;
        Radar_blit_world(&sr, &dr);

        sr.x = x; sr.y = 0; sr.w = w; sr.h = y;
        dr.x = 0 + radar_bounds.x; dr.y = h + radar_bounds.y;
        dr.w = w; dr.h = y;
        Radar_blit_world(&sr, &dr);

        sr.x = 0; sr.y = y; sr.w = x; sr.h = h;
        dr.x = w + radar_bounds.x; dr.y = 0 + radar_bounds.y;
        dr.w = x; dr.h = h;
        Radar_blit_world(&sr, &dr);

        sr.x = x; sr.y = y; sr.w = w; sr.h = h;
        dr.x = 0 + radar_bounds.x; dr.y = 0 + radar_bounds.y;
        dr.w = w; dr.h = h;
        Radar_blit_world(&sr, &dr);
    } else {
	SDL_Rect sr;
	sr.x = sr.y = 0;
	sr.w = radar_bounds.w; sr.h = radar_bounds.h;
	Radar_blit_world(&sr, &radar_bounds);
    }

    Radar_paint_checkpoints(xf, yf);
    Radar_paint_objects();
    Radar_paint_self(xf, yf);
    Radar_paint_border();
}

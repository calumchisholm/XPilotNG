#ifndef SDLPAINT_H
#define SDLPAINT_H

#define MAX_VERTICES 10000

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff
#else
#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif

typedef unsigned int color_t;

extern int              draw_depth;
extern double           scale;              /* The opengl scale factor */

/* code to set the viewport size */
int Resize_Window( int width, int height );


#define STATIONARY_MODE 1
#define MOVING_MODE 	2
#define HUD_MODE    	4
int paintSetupMode;
void setupPaint_stationary(void);
void setupPaint_moving(void);
void setupPaint_HUD(void);

/* helper function in sdlgui.c, some may be removed soon */
extern int GL_X(int x);
extern int GL_Y(int y);
extern void Segment_add(int color, int x_1, int y_1, int x_2, int y_2);
extern void Circle(int color, int x, int y, int radius, int filled);

#endif

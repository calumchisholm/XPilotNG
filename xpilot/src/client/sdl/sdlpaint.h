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

extern int              draw_width;
extern int              draw_height;
extern int              draw_depth;
extern int              num_spark_colors;
extern float            scale;              /* The scale factor */

/* code to set the viewport size */
int Resize_Window( int width, int height );

#endif

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
extern bool             played_this_round;

extern ipos	        world;
extern ipos	        realWorld;
extern short	        ext_view_width;	    /* Width of extended visible area */
extern short	        ext_view_height;    /* Height of extended visible area */
extern int	        active_view_width;  /* Width of active map area displayed. */
extern int	        active_view_height; /* Height of active map area displayed. */
extern int	        ext_view_x_offset;  /* Offset ext_view_width */
extern int	        ext_view_y_offset;  /* Offset ext_view_height */
extern long		loops;
extern unsigned long	loopsSlow;	    /* Proceeds slower than loops */
extern double           timePerFrame;
extern double	        hudRadarLimit;	    /* Limit for hudradar drawing */
extern float            scale;              /* The scale factor */

/* from the generic code ( paintmap.c and paintobjects.c ) */

extern void Paint_world(void);
extern void Paint_vfuel(void);
extern void Paint_vdecor(void);
extern void Paint_vcannon(void);
extern void Paint_vbase(void);
extern void Paint_objects(void);
extern void Paint_shots(void);
extern void Paint_ships(void);

#endif

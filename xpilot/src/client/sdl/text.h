#ifndef TEXT_H
#define TEXT_H

#include <GL/gl.h>
#include <GL/glu.h>

#include "xpclient.h"
#include "sdlpaint.h"

#define LEFT 0
#define DOWN 0
#define CENTER 1
#define RIGHT 2
#define UP 2

#define NUMCHARS 256

typedef struct {
    GLfloat MinX;
    GLfloat MinY;
    GLfloat MaxX;
    GLfloat MaxY;
} texcoord_t;    

typedef struct {
    GLuint textures[NUMCHARS]; /* texture indexes for the characters */
    GLuint W[NUMCHARS]; /* holds paint width fr each character */
    GLuint list_base; /* start of the texture list for this font */
    GLuint h; /* char height */
    GLuint linespacing; /* proper line spacing according to FT */
} font_data;

int renderstyle;
enum {
	RENDER_LATIN1,
	RENDER_UTF8,
	RENDER_UNICODE
} rendertype;

/* The init function will create a font of
 * of the height h from the file fname.
 */
int fontinit(font_data *ft_font, const char * fname, unsigned int size);

/* Free all the resources assosiated with the font.*/
void fontclean(font_data *ft_font);

typedef struct {
    	float width;
	float height;
} fontbounds;

/* Calcs the bounding width,height for the text if it were printed
 * to screen with given font
 */
fontbounds printsize(font_data *ft_font, int length, const char *fmt, ...);

/* 
 * NOTE: passing color 0x00000000 causes the painting to *not* set color,
 * it does *not* mean the text will be drawn with color 0x00000000, you
 * should check for that before calling this function.
 */
/* length = 0 means that the whole text will be painted.
 */ 
void HUDnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt, ...);
void mapnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt,...);

font_data gamefont;
font_data messagefont;
font_data mapfont;


#endif

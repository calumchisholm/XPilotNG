#ifndef TEXT_H
#define TEXT_H

/*OpenGL Headers*/
#include <GL/gl.h>
#include <GL/glu.h>

#include "xpclient.h"

/* This holds all of the information related to any
 * freetype font that we want to create.
 */ 
typedef struct {
	float h;    	    /* Holds the height of the font.*/
	float w;    	    /* Holds the width of the font.*/
	GLuint * textures;  /* Holds the texture id's */
	GLuint list_base;   /* Holds the first display list id */
} font_data;

/* The init function will create a font of
 * of the height h from the file fname.
 */
int fontinit(font_data *ft_font, const char * fname, unsigned int w/*this is mostly ballpark I guess*/, unsigned int h);

/* Free all the resources assosiated with the font.*/
void fontclean(font_data *ft_font);

typedef struct {
    	float width;
	float height;
} fontbounds;

/* The flagship function of the library - this thing will print
 * out text at window coordinates x,y, using the font ft_font.
 * The current modelview matrix will also be applied to the text.
 */ 
fontbounds fontprint(font_data *ft_font, float x, float y, const char *fmt, ...) ;

font_data gamefont;
font_data messagefont;


#endif
